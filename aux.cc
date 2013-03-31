#include "glob.h"

//----------------------------------------------------------------------

static void player_miss (struct monster *m, EDamageType dtype);
static void p_fumble (EDamageType dtype);
static void drop_weapon(void);
static void break_weapon(void);
static void movecursor(int *x, int *y, int dx, int dy);
static void gain_level(void);
static const char* levelname (unsigned level) PURE;
static unsigned expval (unsigned plevel);
static void tacplayer(struct monster *m);
static int player_hit(int hitmod, int hitloc, struct monster *m);
static void indoors_random_event(void);
static void outdoors_random_event(void);
static void showknownsites(unsigned first, unsigned last);
static void destroy_order(void);
static void default_maneuvers(void);
static void fight_monster (monster *m);

//----------------------------------------------------------------------

// check to see if too much tunneling has been done in this level
void level::tunnelcheck (void)
{
    if ((!depth && Current_Environment != E_DLAIR) || Current_Environment == E_ASTRAL)
	return;
    ++tunnelled;
    if (tunnelled > height / 4)
	mprint ("Dust and stone fragments fall on you from overhead.");
    if (tunnelled > height / 2)
	mprint ("You hear groaning and creaking noises.");
    if (tunnelled > 3 * height / 4)
	mprint ("The floor trembles and you hear a loud grinding screech.");
    if (tunnelled > height) {
	mprint ("With a scream of tortured stone, the entire dungeon caves in!!!");
	gain_experience (5000);
	if (Player.status[SHADOWFORM]) {
	    change_environment (E_COUNTRYSIDE);
	    c_set (Player.x, Player.y, SEEN);
	    c_set (Player.x, Player.y, SECRET);
	    c_set (Player.x, Player.y, CHANGED);
	    print1 ("In your shadowy state, you float back up to the surface.");
	    return;
	}
	mprint ("You are flattened into an unpleasant jellylike substance.");
	p_death ("dungeon cave-in");
    }
}

// displays a room's name
void showroom (int i)
{
    char namebuf [128];
    namebuf[0] = 0;
    if (Current_Environment-E_CITY <= E_HOUSE-E_CITY) {
	static const char enames[] = // E_CITY-E_HOUSE
	    "The City of Rampart\0"
	    "The Village of \0"
	    "The Tactical Map \0"
	    "The Sewers: \0"
	    "The Archmage's Castle: \0"
	    "The Goblin Caves: \0"
	    "The Volcano: \0"
	    "The Astral Plane: \0"
	    "The Rampart Arena\0"
	    "A hovel: \0"
	    "A luxurious mansion: \0"
	    "A house: ";
	strcpy (namebuf, zstrn (enames, Current_Environment-E_CITY, E_HOUSE-E_CITY));
    }
    if (Current_Environment == E_VILLAGE) {
	static const char villageNames[] =
	    "Star View\0" "Woodmere\0" "Stormwatch\0"
	    "Thaumaris\0" "Skorch\0" "Whorfen";
	strcat (namebuf, zstrn(villageNames,Villagenum,6));
    }
    if (Current_Environment == Current_Dungeon) {
	char lnamebuf[64];
	snprintf (ArrayBlock(lnamebuf), "Level %hhd (%s)", Level->depth, roomname(i));
	strcat (namebuf, lnamebuf);
    } else if (!namebuf[0] || Current_Environment == E_HOVEL || Current_Environment == E_MANSION || Current_Environment == E_HOUSE)
	strcat (namebuf, roomname (i));
    locprint (namebuf);
}

bool player::on_sanctuary (void) const
{
    return ((x == sx && y == sy)
	    || (Level->site(x,y).locchar == ALTAR
		&& Level->site(x,y).aux == patron));
}

// check a move attempt, maybe attack something, return true if ok to move.
// x y is the proposed place to move to
bool p_moveable (int x, int y)
{
    setgamestatus (SKIP_MONSTERS);
    const auto& l = Level->site(x,y);
    if (!inbounds (x, y))
	return (false);
    else if (Player.status[SHADOWFORM]) {
	if (l.p_locf == L_CHAOS || l.p_locf == L_ABYSS || l.p_locf == L_VOID)
	    return confirmation();
    } else if (loc_statusp (x, y, SECRET)) {
	if (!gamestatusp (FAST_MOVE))
	    print3 ("Ouch!");
	return (false);
    } else if (Level->creature(x,y)) {
	if (!gamestatusp (FAST_MOVE)) {
	    fight_monster (Level->creature(x,y));
	    resetgamestatus (SKIP_MONSTERS);
	}
	return (false);
    } else if (l.locchar == WALL || l.locchar == STATUE || l.locchar == PORTCULLIS || l.locchar == CLOSED_DOOR
	       || (gamestatusp (FAST_MOVE)
		   && (l.locchar == HEDGE || l.locchar == LAVA || l.locchar == ABYSS
		       || l.locchar == VOID_CHAR || l.locchar == FIRE || l.locchar == WHIRLWIND
		       || l.locchar == WATER || l.locchar == LIFT || l.locchar == TRAP))) {
	if (!gamestatusp (FAST_MOVE))
	    print3 ("Ouch!");
	return (false);
    } else if (optionp (CONFIRM)) {
	if (l.locchar == HEDGE || l.locchar == LAVA || l.locchar == FIRE || l.locchar == WHIRLWIND
	    || l.locchar == ABYSS || l.locchar == VOID_CHAR || l.locchar == WATER
	    || l.locchar == RUBBLE || l.locchar == LIFT || l.locchar == TRAP) {
	    // horses WILL go into water...
	    if (gamestatusp (MOUNTED)) {
		if (l.locchar != WATER || l.p_locf != L_WATER) {
		    print1 ("You can't convince your steed to continue.");
		    setgamestatus (SKIP_MONSTERS);
		    return (false);
		}
		return (true);
	    } else if (!confirmation()) {
		setgamestatus (SKIP_MONSTERS);
		return (false);
	    }
	}
    }
    resetgamestatus (SKIP_MONSTERS);
    return (true);
}

// check a move attempt in the countryside
bool p_country_moveable (int x, int y)
{
    if (!inbounds (x, y))
	return (false);
    else if (optionp(CONFIRM) &&
		(Country->site(x,y).showchar() == CHAOS_SEA ||
		 Country->site(x,y).showchar() == MOUNTAINS))
	return (confirmation());
    return (true);
}

// search once particular spot
void searchat (int x, int y)
{
    if (inbounds (x, y) && (random_range(3) || Player.status[ALERT])) {
	auto& l = Level->site(x,y);
	if (loc_statusp (x, y, SECRET)) {
	    lreset (x, y, SECRET);
	    lset (x, y, CHANGED);
	    if ((l.locchar == OPEN_DOOR) || (l.locchar == CLOSED_DOOR)) {
		mprint ("You find a secret door!");
		for (unsigned i = 0; i <= 8; i++) {
		    lset (x + Dirs[0][i], y + Dirs[1][i], STOPS);
		    lset (x + Dirs[0][i], y + Dirs[1][i], CHANGED);
		}
	    } else
		mprint ("You find a secret passage!");
	    drawvision (Player.x, Player.y);
	}
	if (l.p_locf >= TRAP_BASE && l.locchar != TRAP && l.p_locf <= TRAP_BASE + NUMTRAPS) {
	    l.locchar = TRAP;
	    lset (x, y, CHANGED);
	    mprint ("You find a trap!");
	    drawvision (Player.x, Player.y);
	    resetgamestatus (FAST_MOVE);
	}
    }
}

void calc_melee (void) { Player.calc_melee(); }

// This is to be called whenever anything might change player performance in
// melee, such as changing weapon, statistics, etc.
void player::calc_melee (void)
{
    calc_weight();

    absorption = status[PROTECTION];
    maxweight = str*agi*10;
    defense = 2*statmod(agi) + level/2;
    hit = level + statmod(dex) + 1;
    dmg = statmod(str) + 3;
    if (status[ACCURATE])
	hit += 20;
    if (status[HERO]) {
	hit += dex;
	dmg += str;
	defense += agi;
    }

    speed = 5 - min (4, (statmod (agi) / 2));
    if (status[HASTED] || status[HERO])
	speed /= 2;
    if (status[SLOWED])
	speed *= 2;
    speed -= min (speed, 4-4*itemweight/maxweight);
    speed = max (1, min (25, speed));

    if (gamestatusp (MOUNTED)) {
	speed = 3;
	hit += 10;
	dmg += 10;
    }

    // weapon
    // have to check for used since it could be a 2h weapon just carried in one hand
    if (has_possession(O_WEAPON_HAND)) {
	if (possessions[O_WEAPON_HAND].used && (possessions[O_WEAPON_HAND].objchar == WEAPON || possessions[O_WEAPON_HAND].objchar == MISSILEWEAPON)) {
	    hit += possessions[O_WEAPON_HAND].hit + possessions[O_WEAPON_HAND].plus;
	    dmg += possessions[O_WEAPON_HAND].dmg + possessions[O_WEAPON_HAND].plus;
	}
    }

    // shield or defensive weapon
    if (has_possession(O_SHIELD))
	defense += possessions[O_SHIELD].aux + possessions[O_SHIELD].plus;

    // armor
    if (has_possession(O_ARMOR)) {
	absorption += possessions[O_ARMOR].dmg;
	defense += possessions[O_ARMOR].plus - possessions[O_ARMOR].aux;
    }

    if (strlen(meleestr) > maneuvers()*2)
	default_maneuvers();
    comwinprint();
    showflags();
    dataprint();
}

// player attacks monster m
static void fight_monster (struct monster *m)
{
    int hitmod = 0;
    int reallyfight = true;

    if (Player.status[AFRAID]) {
	print3 ("You are much too afraid to fight!");
	reallyfight = false;
    } else if (Player.on_sanctuary()) {
	print3 ("You restrain yourself from desecrating this holy place.");
	reallyfight = false;
    } else if (Player.status[SHADOWFORM]) {
	print3 ("Your attack has no effect in your shadowy state.");
	reallyfight = false;
    } else if (Player.status[BERSERK] < 1 && !m_statusp (m, HOSTILE)) {
	if (!optionp (BELLICOSE))
	    reallyfight = confirmation();
    }
    if (reallyfight) {
	if (Lunarity == 1)
	    hitmod += Player.level;
	else if (Lunarity == -1)
	    hitmod -= (Player.level / 2);

	if (!m->attacked)
	    Player.alignment -= 2;	// chaotic action
	m_status_reset (*m, ASLEEP);
	m_status_set (*m, HOSTILE);
	m->attacked = true;
	Player.hit += hitmod;
	tacplayer (m);
	Player.hit -= hitmod;
    }
}

// Attempt to break an object o
int damage_item (pob o)
{
    // special case -- break star gem
    if (o->id == STAR_GEM) {
	print1 ("The Star Gem shatters into a million glistening shards....");
	if (Current_Environment == E_STARPEAK) {
	    if (!gamestatusp (KILLED_LAWBRINGER))
		print2 ("You hear an agonizing scream of anguish and despair.");
	    morewait();
	    print1 ("A raging torrent of energy escapes in an explosion of magic!");
	    print2 ("The energy flows to the apex of Star Peak where there is");
	    morewait();
	    clearmsg();
	    print1 ("an enormous explosion!");
	    morewait();
	    annihilate (1);
	    print3 ("You seem to gain strength in the chaotic glare of magic!");
	    Player.str = max (Player.str, Player.maxstr + 5);
	    Player.pow = max (Player.pow, Player.maxpow + 5);
	    Player.alignment -= 200;
	    Player.remove_possession (o, 1);
	} else {
	    morewait();
	    print1 ("The shards coalesce back together again, and vanish");
	    print2 ("with a muted giggle.");
	    Player.remove_possession (o, 1);
	    set_object_uniqueness (o, UNIQUE_UNMADE);
	    // WDT HACK: the above is correct only if UNIQUE_UNMADE means that
	    // the artifact hasn't been generated yet.  (Clearly, Omega is a
	    // little buggy in that regard with respect to artifacts in general
	    // -- it's almost trivial to force two identical artefacts to be
	    // generated right now.)
	}
	return 1;
    } else {
	if (o->fragility < random_range (30)) {
	    if (o->objchar == STICK) {
		snprintf (ArrayBlock(Str1), "Your %s explodes!", (o->blessing >= 0 ? o->truename : o->cursestr));
		print1 (Str1);
		morewait();
		if (o->charge < 1)
		    nprint1 (" Fzzz... Out of Power... Oh well...");
		else {
		    nprint1 (" Ka-Blamm!!!");
		    // general case. Some sticks will eventually do special things
		    morewait();
		    manastorm (Player.x, Player.y, o->charge * o->level * 10);
		    Player.remove_possession (o, 1);
		}
		return 1;
	    } else if ((o->blessing > 0) && (o->level > random_range (10))) {
		snprintf (ArrayBlock(Str1), "Your %s glows strongly", itemid(o));
		print1 (Str1);
		return 0;
	    } else if ((o->blessing < -1) && (o->level > random_range (10))) {
		snprintf (ArrayBlock(Str1), "You hear an evil giggle from your %s", itemid(o));
		print1 (Str1);
		return 0;
	    } else if (o->plus > 0) {
		snprintf (ArrayBlock(Str1), "Your %s glows and then fades", itemid(o));
		print1 (Str1);
		o->plus--;
		return 0;
	    } else {
		if (o->blessing > 0)
		    print1 ("You hear a faint despairing cry!");
		else if (o->blessing < 0)
		    print1 ("You hear an agonized scream!");
		snprintf (ArrayBlock(Str1), "Your %s shatters in a thousand lost fragments!", itemid(o));
		print2 (Str1);
		morewait();
		Player.remove_possession (o, 1);
		return 1;
	    }
	}
	return 0;
    }
}

// do dmg points of damage of type dtype, from source fromstring
void p_damage (int dmg, EDamageType dtype, const char* fromstring)
{
    if (Player.immune_to (dtype))
	mprint ("You resist the effects!");
    else {
	if (gamestatusp (FAST_MOVE)) {
	    drawvision (Player.x, Player.y);
	    resetgamestatus (FAST_MOVE);
	}
	if (dtype == NORMAL_DAMAGE)
	    dmg = max (1, (dmg - Player.absorption));
	if ((Player.hp -= dmg) < 1)
	    p_death (fromstring);
    }
    dataprint();
}

// game over, you lose!
void p_death (const char* fromstring)
{
    Player.hp = -1;
    print3 ("You died!");
    morewait();
    display_death (fromstring);
    endgraf();
    exit (0);
}

// hooray
void p_win (void)
{
    morewait();
    clearmsg();
    print1 ("You won!");
    morewait();
    display_win();
    endgraf();
    exit (0);
}

// move the cursor around, like for firing a wand, sets x and y to target
void setspot (int *x, int *y)
{
    mprint ("Targeting.... ? for help");
    curs_set (1);
    omshowcursor (*x, *y);
    for (;;) {
	char c = lgetc();
	if (c == '.')
	    break;
	else if (c == KEY_ESCAPE) {
	    *x = *y = ABORT;
	    break;
	} else if (c == '?') {
	    clearmsg();
	    mprint ("Use vi keys or numeric keypad to move cursor to target.");
	    mprint ("Hit the '.' key when done, or ESCAPE to abort.");
	}
	static const struct { char k1,k2; int8_t dx,dy; } c_targkeymap[] = {
	    { 'h','4', -1, 0 },
	    { 'j','2',  0, 1 },
	    { 'k','8',  0,-1 },
	    { 'l','6',  1, 0 },
	    { 'b','1', -1, 1 },
	    { 'n','3',  1, 1 },
	    { 'y','7', -1,-1 },
	    { 'u','9',  1,-1 }
	};
	for (unsigned i = 0; i < ArraySize(c_targkeymap); ++i)
	    if (c_targkeymap[i].k1 == c || c_targkeymap[i].k2 == c)
		movecursor (x, y, c_targkeymap[i].dx, c_targkeymap[i].dy);
    }
    curs_set (0);
    screencheck (Player.y);
}

// get a direction: return index into Dirs array corresponding to direction
int getdir (void)
{
    while (1) {
	mprint ("Select direction [hjklyubn, ESCAPE to quit]: ");
	char c = mgetc();
	if (c == KEY_ESCAPE)
	    return (ABORT);
	static const char c_dirkey[] = "3nN9uU1bB7yY6lL4hH2jJ8kK";
	const char* ikey = strchr (c_dirkey, c);
	if (ikey)
	    return ((unsigned)distance(c_dirkey,ikey)/3);
	print3 ("That's not a direction! ");
    }
}

// functions describes monster m's state for examine function
const char* mstatus_string (struct monster *m)
{
    const auto& mstd = Monsters[m->id];
    const char* healthstr = m->monstring;
    if (m->hp < mstd.hp / 3)
	healthstr = "grievously injured ";
    else if (m->hp < mstd.hp / 2)
	healthstr = "severely injured ";
    else if (m->hp < mstd.hp)
	healthstr = "injured ";
    const char* articlestr = getarticle (healthstr);
    const char* fmtstr = "%3$s who is %2$s";
    if (healthstr == m->monstring) {
	healthstr = "";
	fmtstr = "%3$s";
    }
    if (m_statusp (m, M_INVISIBLE) && !Player.status[TRUESIGHT])
	fmtstr = "Some invisible creature";
    else if (m->uniqueness == COMMON)
	fmtstr = (m->level > mstd.level ? "%s%s%s (level %s)" : "%s%s%s");
    else
	articlestr = "";
    const char* levelstr = wordnum (m->level + 1 - mstd.level);
    snprintf (ArrayBlock(Str2), fmtstr, articlestr, healthstr, m->monstring, levelstr);
    return (Str2);
}

// for the examine function
void describe_player (void)
{
    if (Player.hp < (Player.maxhp / 5))
	print1 ("A grievously injured ");
    else if (Player.hp < (Player.maxhp / 2))
	print1 ("A seriously wounded ");
    else if (Player.hp < Player.maxhp)
	print1 ("A somewhat bruised ");
    else
	print1 ("A fit ");

    if (Player.status[SHADOWFORM])
	nprint1 ("shadow");
    else
	nprint1 (levelname (Player.level));
    nprint1 (" named ");
    nprint1 (Player.name);
    if (gamestatusp (MOUNTED))
	nprint1 (" (riding a horse.)");
}

// access to player experience...
// share out experience among guild memberships
void gain_experience (int amount)
{
    Player.xp += amount;
    gain_level();		// actually, check to see if should gain level
    unsigned count = 0;
    for (unsigned i = 0; i < NUMGUILDS; i++)
	if (Player.guildxp[i] > 0)
	    ++count;
    unsigned share = amount / (max (count, 1u));
    for (unsigned i = 0; i < NUMGUILDS; i++)
	if (Player.guildxp[i] > 0)
	    Player.guildxp[i] += share;
}

// try to hit a monster in an adjacent space. If there are none return false.
// Note if you're berserk you get to attack ALL adjacent monsters!
bool goberserk (void)
{
    bool wentberserk = false;
    lstring meleestr (ArrayBlock("lLlClH"));
    Player.meleestr.swap (raw_cast<string>(meleestr));
    for (unsigned i = 0; i < 8; i++) {
	monster* m = Level->creature(Player.x + Dirs[0][i], Player.y + Dirs[1][i]);
	if (m) {
	    wentberserk = true;
	    fight_monster(m);
	    morewait();
	}
    }
    Player.meleestr.swap (raw_cast<string>(meleestr));
    return (wentberserk);
}

// checks current food status of player, every hour, and when food is eaten
void foodcheck (void)
{
    if (Player.food > 48) {
	Player.food = 12;
	print3 ("You vomit up your huge meal.");
    } else if (Player.food == 30)
	print3 ("Time for a smackerel of something.");
    else if (Player.food == 20)
	print3 ("You feel hungry.");
    else if (Player.food == 12)
	print3 ("You are ravenously hungry.");
    else if (Player.food < 4) {
	if (gamestatusp (FAST_MOVE)) {
	    drawvision (Player.x, Player.y);
	    resetgamestatus (FAST_MOVE);
	}
	if (Player.food >= 0)
	    print3 ("You feel weak.");
	else {
	    p_damage (-5 * Player.food, UNSTOPPABLE, "starvation");
	    print3 ("You're starving!");
	}
    }
    showflags();
}

// see whether room should be illuminated
void roomcheck (void)
{
    int roomno = Level->site(Player.x,Player.y).roomnumber;
    if (roomno == RS_CAVERN || roomno == RS_SEWER_DUCT || roomno == RS_KITCHEN
	    || roomno == RS_BATHROOM || roomno == RS_BEDROOM || roomno == RS_DININGROOM
	    || roomno == RS_CLOSET || roomno > RS_ROOMBASE) {
	if (!loc_statusp (Player.x, Player.y, LIT) && !Player.status[BLINDED] && (Player.status[ILLUMINATION] || difficulty() < 6)) {
	    spreadroomlight (Player.x, Player.y, roomno);
	    levelrefresh();
	}
    }
    showroom (roomno);
}

// ask for mercy
void surrender (struct monster *m)
{
    static const char c_SurrenderPlea[] =
	"You grovel at the monster's feet...\0"
	"You cry 'uncle'!\0"
	"You beg for mercy.\0"
	"You yield to the monster.";
    print1 (zstrn (c_SurrenderPlea, random_range(4), 4));
    if (m->id == GUARD) {
	if (m_statusp (m, HOSTILE))
	    monster_talk (m);
	else {
	    print2 ("The guard (bored): Have you broken a law? [yn] ");
	    if (ynq2() == 'n')
		print2 ("Then don't bother me. Scat!");
	    else {
		print2 ("The guard grabs you, and drags you to court.");
		morewait();
		send_to_jail();
	    }
	}
    } else if (m->talkf == M_NO_OP || m->talkf == M_TALK_STUPID)
	print3 ("Your plea is ignored.");
    else {
	morewait();
	print1 ("Your surrender is accepted.");
	if (Player.cash > 0) {
	    Player.cash = 0;
	    nprint1 (" All your gold is taken....");
	}
	int bestitem = ABORT;
	unsigned bestvalue = 0;
	for (unsigned i = 1; i < MAXITEMS; ++i) {
	    if (Player.has_possession(i)) {
		unsigned ival = true_item_value (Player.possessions[i]);
		if (bestvalue < ival) {
		    bestvalue = ival;
		    bestitem = i;
		}
	    }
	}
	if (bestitem != ABORT) {
	    print2 ("You also give away your best item... ");
	    nprint2 (itemid (Player.possessions[bestitem]));
	    morewait();
	    givemonster (*m, Player.possessions[bestitem]);
	    morewait();	// msgs come from givemonster
	    Player.remove_possession (bestitem);
	}
	print2 ("You feel less experienced... ");
	Player.xp = max (0, (int)Player.xp - m->xpv);
	nprint2 ("The monster seems more experienced!");
	m->level = min (10, m->level + 1);
	m->hp += m->level * 20;
	m->hit += m->level;
	m->dmg += m->level;
	m->ac += m->level;
	m->xpv += m->level * 10;
	morewait();
	clearmsg();
	if (m->talkf == M_TALK_EVIL && random_range(10)) {
	    print1 ("It continues to attack you, laughing evilly!");
	    m_status_set (m, HOSTILE);
	    m_status_reset (m, GREEDY);
	} else {
	    print1 ("The monster leaves, chuckling to itself....");
	    m_teleport (m);
	}
    }
    dataprint();
}

// threaten a monster
void threaten (struct monster *m)
{
    static const char c_Threat[] =
	"You demand that your opponent surrender!\0"
	"You threaten to do bodily harm to it\0"
	"You attempt to bluster it into submission\0"
	"You try to cow it with your awesome presence";
    mprint (zstrn (c_Threat, random_range(4), 4));
    morewait();
    if (!m_statusp (m, HOSTILE)) {
	print3 ("You only annoy it with your futile demand.");
	m_status_set (m, HOSTILE);
    } else if ((2*m->level > Player.level && m->hp > Player.dmg) || m->uniqueness != COMMON)
	print1 ("It sneers contemptuously at you.");
    else if (m->talkf != M_TALK_GREEDY && m->talkf != M_TALK_HUNGRY && m->talkf != M_TALK_EVIL
		&& m->talkf != M_TALK_MAN && m->talkf != M_TALK_BEG && m->talkf != M_TALK_THIEF
		&& m->talkf != M_TALK_MERCHANT && m->talkf != M_TALK_IM)
	print1 ("Your demand is ignored");
    else {
	print1 ("It yields to your mercy.");
	Player.alignment += 3;
	print2 ("Kill it, rob it, or free it? [krf] ");
	for (char response;;) {
	    response = (char) mcigetc();
	    if (response == 'k') {
		m_death (m);
		print2 ("You treacherous rogue!");
		Player.alignment -= 13;
		break;
	    } else if (response == 'r') {
		Player.alignment -= 2;
		print2 ("It drops its treasure and flees.");
		m_dropstuff (m);
		Level->mlist.erase (m);
		putspot (m->x, m->y, getspot (m->x, m->y, false));
		break;
	    } else if (response == 'f') {
		Player.alignment += 2;
		print2 ("'If you love something set it free ... '");
		if (!random_range(100)) {
		    morewait();
		    print2 ("'...If it doesn't come back, hunt it down and kill it.'");
		}
		print3 ("It departs with a renewed sense of its own mortality.");
		Level->mlist.erase (m);
		putspot (m->x, m->y, getspot (m->x, m->y, false));
		break;
	    }
	}
    }
}

// name of the player's experience level
static const char* levelname (unsigned level)
{
    if (level < 23) {
	static const char c_Levels[] =
	    "neophyte\0"
	    "beginner\0"
	    "tourist\0"
	    "traveller\0"
	    "wayfarer\0"
	    "peregrinator\0"
	    "wanderer\0"
	    "hunter\0"
	    "scout\0"
	    "trailblazer\0"
	    "discoverer\0"
	    "explorer\0"
	    "senior explorer\0"
	    "ranger\0"
	    "ranger captain\0"
	    "ranger knight\0"
	    "adventurer\0"
	    "experienced adventurer\0"
	    "skilled adventurer\0"
	    "master adventurer\0"
	    "hero\0"
	    "superhero\0"
	    "demigod";
	return (zstrn (c_Levels, level, 23));
    }
    if (level < 100)
	snprintf (ArrayBlock(Str3), "Order %u Master of Omega", level/10-2);
    else
	snprintf (ArrayBlock(Str3), "Ultimate Master of Omega");
    return (Str3);
}

// Player stats like str, agi, etc give modifications to various abilities
// chances to do things, etc. Positive is good, negative bad.
int statmod (int stat)
{
    return ((stat - 10) / 2);
}

// effects of hitting
void p_hit (struct monster *m, int dmg, int dtype)
{
    unsigned dmult = 1;
    const char* hitdesc;
    // chance for critical hit..., 3/10
    switch (random_range (10)) {
	case 0:
	    if (random_range(100) < Player.level) {
		dmult = 1000;
		hitdesc = "annihilate";
	    } else {
		dmult = 5;
		hitdesc = "blast";
	    }
	    break;
	case 1:
	case 2:
	    dmult = 2;
	    hitdesc = "smash";
	    break;
	default:
	    static const char c_DamageDesc[] =
		"damage\0"
		"inflict bodily harm on\0"
		"injure\0"
		"molest\0"
		"hit";
	    hitdesc = zstrn (c_DamageDesc, random_range(14), 5);
	    break;
    }
    if (Lunarity == 1)
	dmult *= 2;
    else if (Lunarity == -1)
	dmult /= 2;
    snprintf (ArrayBlock(Str3), "You %s %s%s.", hitdesc, m->definite_article(), m->monstring);
    if (Verbosity != TERSE)
	mprint (Str3);
    else
	mprint ("You hit it.");
    m_damage (m, dmult * random_range (dmg), dtype);
    if (Verbosity != TERSE && !random_range(10) && m->hp > 0)
	mprint ("It laughs at the injury and fights on!");
}

// and effects of missing
static void player_miss (struct monster *m, EDamageType dtype)
{
    if (!random_range(30))	// fumble 1 in 30
	p_fumble (dtype);
    else if (Verbosity == TERSE)
	mprint ("You missed it.");
    else {
	static const char c_MissDesc[] =
	    "flail lamely at\0"
	    "only amuse\0"
	    "fail to even come close to\0"
	    "totally avoid contact with\0"
	    "miss";
	snprintf (ArrayBlock(Str3), "You %s %s%s.", zstrn(c_MissDesc,random_range(14),5), m->definite_article(), m->monstring);
	mprint (Str3);
    }
}

// oh nooooo, a fumble....
static void p_fumble (EDamageType dtype)
{
    mprint ("Ooops! You fumbled....");
    switch (random_range(10)) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	    drop_weapon();
	    break;
	case 6:
	case 7:
	case 8:
	    break_weapon();
	    break;
	case 9:
	    mprint ("Oh No! You hit yourself!");
	    p_damage (Player.dmg, dtype, "stupidity");
	    break;
    }
}

// try to drop a weapon (from fumbling)
static void drop_weapon (void)
{
    if (!Player.has_possession(O_WEAPON_HAND))
	mprint ("You feel fortunate.");
    else {
	strcpy (Str1, "You dropped your ");
	strcat (Str1, Player.possessions[O_WEAPON_HAND].objstr);
	mprint (Str1);
	morewait();
	p_drop_at (Player.x, Player.y, Player.possessions[O_WEAPON_HAND], 1);
	Player.remove_possession (O_WEAPON_HAND, 1);
    }
}

// try to break a weapon (from fumbling)
static void break_weapon (void)
{
    if (Player.has_possession(O_WEAPON_HAND)) {
	snprintf (ArrayBlock(Str1), "Your %s vibrates in your hand....", itemid (Player.possessions[O_WEAPON_HAND]));
	mprint (Str1);
	damage_item (&Player.possessions[O_WEAPON_HAND]);
	morewait();
    }
}

// handle a h,j,k,l, etc., to change x and y by dx and dy
// for targeting in dungeon
static void movecursor (int *x, int *y, int dx, int dy)
{
    if (inbounds (*x + dx, *y + dy)) {
	*x += dx;
	*y += dy;
	screencheck (*y);
    }
    omshowcursor (*x, *y);
}

// deal with each possible stati -- values are per move
// this function is executed every move
// A value over 1000 indicates a permanent effect
void minute_status_check (void)
{
    if (Player.status[HASTED] > 0 && Player.status[HASTED] < 1000 && !--Player.status[HASTED]) {
	mprint ("The world speeds up.");
	calc_melee();
    }
    if (Player.status[POISONED] > 0) {
	p_damage (3, POISON, "poison");
	if (!--Player.status[POISONED]) {
	    showflags();
	    mprint ("You feel better now.");
	}
    }
    if (Player.immunity[UNSTOPPABLE] > 0) {
	for (unsigned i = 0; i < NUMIMMUNITIES; i++)
	    --Player.immunity[i];
	if (Player.immunity[UNSTOPPABLE] == 1)
	    mprint ("You feel vincible again.");
    }
    if (Player.status[IMMOBILE] > 0 && !--Player.status[IMMOBILE])
	mprint ("You can move again.");
    if (Player.status[SLEPT] > 0 && !--Player.status[SLEPT])
	mprint ("You woke up.");
    if (Player.status[REGENERATING] > 0) {
	if (Player.hp < Player.maxhp && Player.mana > 0) {
	    ++Player.hp;
	    --Player.mana;
	    dataprint();
	}
	if (Player.status[REGENERATING] < 1000 && !--Player.status[REGENERATING])
	    mprint ("You feel less homeostatic.");
    }
    if (Player.status[SLOWED] > 0 && Player.status[SLOWED] < 1000 && !--Player.status[SLOWED]) {
	mprint ("You feel quicker now.");
	calc_melee();
    }
    if (Player.status[RETURNING] > 0) {
	if (!--Player.status[RETURNING])
	    level_return();
	else if (Player.status[RETURNING] == 1)
	    mprint ("Your surroundings start to warp and fade!");
	else if (Player.status[RETURNING] == 5)
	    mprint ("A vortex of mana begins to form around you!");
	else if (Player.status[RETURNING] == 8)
	    mprint ("There is an electric tension in the air!");
	else if (Player.status[RETURNING] == 10)
	    mprint ("Your return spell slowly hums towards activation...");
    }
    if (Player.status[AFRAID] > 0 && Player.status[AFRAID] < 1000 && !--Player.status[AFRAID])
	mprint ("You feel bolder now.");
}

// effect of gamma ray radiation...
void moon_check (void)
{
    // 24 day lunar cycle
    Phase = (Phase + 1) % 24;
    phaseprint();
    Lunarity = 0;
    if ((Player.patron == DRUID && (Phase / 2 == 3 || Phase / 2 == 9))
	    || (Player.alignment > 10 && Phase / 2 == 6)
	    || (Player.alignment < -10 && Phase / 2 == 0)) {
	mprint ("As the moon rises you feel unusually vital!");
	Lunarity = 1;
    } else if ((Player.patron == DRUID && (Phase / 2 == 0 || Phase / 2 == 6))
	    || (Player.alignment > 10 && Phase / 2 == 0)
	    || (Player.alignment < -10 && Phase / 2 == 6)) {
	mprint ("The rise of the moon tokens a strange enervation!");
	Lunarity = -1;
    }
}

// check 1/hour for torch to burn out if used
void torch_check (void)
{
    for (unsigned i = O_READY_HAND; i <= O_WEAPON_HAND; ++i) {
	if (Player.possessions[i].id == THING_TORCH && Player.possessions[i].aux > 0 && !--Player.possessions[i].aux) {
	    mprint ("Your torch goes out!!!");
	    Player.remove_possession (i, 1);
	}
    }
}

// values are in multiples of ten minutes
// values over 1000 indicate a permanent effect
void tenminute_status_check (void)
{
    if (Player.status[SHADOWFORM] > 0 && Player.status[SHADOWFORM] < 1000 && !--Player.status[SHADOWFORM]) {
	--Player.immunity[NORMAL_DAMAGE];
	--Player.immunity[ACID];
	--Player.immunity[THEFT];
	--Player.immunity[INFECTION];
	mprint ("You feel less shadowy now.");
    }
    static const uint8_t stati[] = {
	ILLUMINATION,
	VULNERABLE,
	DEFLECTION,
	LEVITATING,
	INVISIBLE,
	BLINDED,
	TRUESIGHT,
	BERSERK,
	ALERT,
	BREATHING,
	DISPLACED,
	ACCURATE,
	HERO,
	DISEASED
    };
    static const char statStopMsg[] =	// Parallel to stati
	"Your light goes out!\0"
	"You feel less endangered.\0"
	"You feel less well defended.\0"
	"You're no longer walking on air.\0"
	"You feel more opaque now.\0"
	"You can see again.\0"
	"You feel less keen now.\0"
	"You stop foaming at the mouth.\0"
	"You feel less alert now.\0"
	"You feel somewhat congested.\0"
	"You feel a sense of position.\0"
	"The bulls' eyes go away.\0"
	"You feel less than super.\0"
	"You feel better now.";
    const char* stopmsg = statStopMsg;
    for (unsigned i = 0; i < ArraySize(stati); ++i, stopmsg=strnext(stopmsg))
	if (Player.status[stati[i]] > 0 && Player.status[stati[i]] < 1000 && !--Player.status[stati[i]])
	    mprint (stopmsg);
    calc_melee();
    showflags();
    timeprint();
    dataprint();
}

// Increase in level at appropriate experience gain
static void gain_level (void)
{
    if (gamestatusp (SUPPRESS_PRINTING))
	return;
    bool gained = false;
    while (expval (Player.level+1) <= Player.xp) {
	if (!gained)
	    morewait();
	gained = true;
	++Player.level;
	print1 ("You have attained a new experience level!");
	print2 ("You are now ");
	nprint2 (getarticle (levelname (Player.level)));
	nprint2 (levelname (Player.level));
	Player.maxhp += random_range (Player.con)+1;
	Player.hp = max (Player.hp, Player.maxhp);
	Player.maxmana = Player.calcmana();
	Player.mana = max (Player.mana, Player.maxmana);
	morewait();
    }
    if (gained)
	clearmsg();
    calc_melee();
}

// experience requirements
static unsigned expval (unsigned l)
{
    static const unsigned c_LevelXP[] = { 0, 20, 50, 200, 500, 1000, 2000, 3000, 5000, 7000, 10000 };
    constexpr unsigned multbase = ArraySize(c_LevelXP)-2;	// So level 11 is 10000*2, etc
    return (l < ArraySize(c_LevelXP) ? c_LevelXP[l] : 10000*(l-multbase));
}

// If an item is unidentified, it isn't worth much to those who would buy it
unsigned item_value (const object* item)
{
    unsigned value = true_item_value (item);
    if (!object_is_known(item))
	value /= 10;
    if (item->objchar == THING)
	value = 1;
    return (value);
}

// figures value based on item base-value, charge, plus, and blessing
unsigned true_item_value (const object* item)
{
    unsigned value = item->basevalue;
    if (item->objchar != THING) {
	if (item->objchar == STICK)
	    value += value * item->charge / 16;
	if (item->plus > -1)
	    value += value * item->plus / 4;
	else
	    value /= -item->plus;
	if (item->blessing > 0)
	    value *= 2;
    }
    return (value);
}

// kill off player if he isn't got the "breathing" status
void p_drown (void)
{
    if (Player.status[BREATHING] > 0) {
	mprint ("Your breathing is unaffected!");
	return;
    }
    for (unsigned attempts = 3; Player.has_possession(O_ARMOR) || Player.itemweight > Player.maxweight / 2; --attempts) {
	menuclear();
	if (!attempts)
	    p_death ("drowning");
	print3 ("You try to hold your breath...");
	if (attempts <= 2)
	    nprint3 (" You choke...");
	if (attempts <= 1)
	    nprint3 (" Your lungs fill...");
	morewait();
	menuprint ("a: Drop an item.\n");
	menuprint ("b: Bash an item.\n");
	menuprint ("c: Drop your whole pack.\n");
	showmenu();
	switch (menugetc()) {
	    case 'a':
		drop();
		if (Level->site(Player.x,Player.y).p_locf == L_WATER && Level->thing(Player.x,Player.y)) {
		    mprint ("It sinks without a trace.");
		    Level->remove_things (Player.x, Player.y);
		}
		break;
	    case 'b':
		bash_item();
		break;
	    case 'c':
		setgamestatus (SUPPRESS_PRINTING);
		foreach (i, Player.pack)
		    if (Level->site(Player.x,Player.y).p_locf != L_WATER)
			p_drop_at (Player.x, Player.y, *i, i->number);
		Player.pack.clear();
		if (Level->site(Player.x,Player.y).p_locf == L_WATER)
		    mprint ("It sinks without a trace.");
		resetgamestatus (SUPPRESS_PRINTING);
		calc_melee();
		break;
	}
    }
    show_screen();
}

// the effect of some weapon on monster m, with dmgmod a bonus to damage
void weapon_use (int dmgmod, pob weapon, struct monster *m)
{
    if (!weapon)
	return (weapon_bare_hands (dmgmod, m));
    switch (weapon->aux) {
	default:
	case I_NO_OP:		weapon_normal_hit (dmgmod, weapon, m); break;
	case I_ACIDWHIP:	weapon_acidwhip (dmgmod, weapon, m); break;
	case I_TANGLE:		weapon_tangle (dmgmod, weapon, m); break;
	case I_ARROW:		weapon_arrow (dmgmod, weapon, m); break;
	case I_BOLT:		weapon_bolt (dmgmod, weapon, m); break;
	case I_DEMONBLADE:	weapon_demonblade (dmgmod, weapon, m); break;
	case I_LIGHTSABRE:	weapon_lightsabre (dmgmod, weapon, m); break;
	case I_MACE_DISRUPT:	weapon_mace_disrupt (dmgmod, weapon, m); break;
	case I_VORPAL:		weapon_vorpal (dmgmod, weapon, m); break;
	case I_DESECRATE:	weapon_desecrate (dmgmod, weapon, m); break;
	case I_FIRESTAR:	weapon_firestar (dmgmod, weapon, m); break;
	case I_DEFEND:		weapon_defend (dmgmod, weapon, m); break;
	case I_VICTRIX:		weapon_victrix (dmgmod, weapon, m); break;
	case I_SCYTHE:		weapon_scythe (dmgmod, weapon, m); break;
    }
}

// for printing actions in printactions above
const char* actionlocstr (char dir)
{
	 if (dir=='L')	return ("low.");
    else if (dir=='C')	return ("center.");
    else if (dir=='H')	return ("high.");
    else		return ("wildly.");
}

// execute player combat actions versus monster m
static void tacplayer (struct monster* m)
{
    for (const char* attack = Player.meleestr; *attack && m->hp > 0; attack += 2) {
	if (*attack == 't' || *attack == 'T') {
	    if (Verbosity == VERBOSE) {
		if (!Player.has_possession(O_WEAPON_HAND))
		    strcpy (Str1, "You punch ");
		else
		    strcpy (Str1, "You thrust ");
		strcat (Str1, actionlocstr (attack[1]));
		mprint (Str1);
	    }
	    if (player_hit (2 * statmod (Player.dex), attack[1], m))
		weapon_use (0, &Player.possessions[O_WEAPON_HAND], m);
	    else
		player_miss (m, NORMAL_DAMAGE);
	} else if (*attack == 'c' || *attack == 'C') {
	    if (Verbosity == VERBOSE) {
		if (!Player.has_possession(O_WEAPON_HAND))
		    strcpy (Str1, "You punch ");
		else if (Player.possessions[O_WEAPON_HAND].type == CUTTING)
		    strcpy (Str1, "You cut ");
		else if (Player.possessions[O_WEAPON_HAND].type == STRIKING)
		    strcpy (Str1, "You strike ");
		else
		    strcpy (Str1, "You attack ");
		strcat (Str1, actionlocstr (attack[1]));
		mprint (Str1);
	    }
	    if (player_hit (0, attack[1], m))
		weapon_use (2 * statmod (Player.str), &Player.possessions[O_WEAPON_HAND], m);
	    else
		player_miss (m, NORMAL_DAMAGE);
	} else if (*attack == 'l' || *attack == 'L') {
	    strcpy (Str1, "You lunge ");
	    strcat (Str1, actionlocstr (attack[1]));
	    if (Verbosity == VERBOSE)
		mprint (Str1);
	    if (player_hit (Player.level + Player.dex, attack[1], m))
		weapon_use (Player.level, &Player.possessions[O_WEAPON_HAND], m);
	    else
		player_miss (m, NORMAL_DAMAGE);
	}
    }
}

// checks to see if player hits with hitmod vs. monster m at location hitloc
static int player_hit (int hitmod, int hitloc, struct monster *m)
{
    if (m->hp < 1) {
	mprint ("Unfortunately, your opponent is already dead!");
	return (0);
    }
    if (hitloc == 'X')
	hitloc = random_loc();

    transcribe_monster_actions (m);

    bool blocks = false;
    int goodblocks = 0;
    for (const char* attack = m->meleestr; attack[0]; attack += 2) {
	if (attack[0] == 'B' || attack[0] == 'R') {
	    blocks = true;
	    goodblocks += (hitloc == attack[1]);
	}
    }
    if (!blocks)
	goodblocks = -1;
    int hit = hitp (Player.hit + hitmod, m->ac + 10*goodblocks);
    if (!hit && goodblocks > 0 && Verbosity == VERBOSE) {
	if (m->uniqueness == COMMON) {
	    strcpy (Str1, "The ");
	    strcat (Str1, m->monstring);
	} else
	    strcpy (Str1, m->monstring);
	strcat (Str1, " blocks it!");
	mprint (Str1);
    }
    return (hit);
}

// This function is used to undo all items temporarily, should
// always be used in pairs with on being true and false, and may cause
// anomalous stats and item-usage if used indiscriminately
void toggle_item_use (int on)
{
    static bool used [MAXITEMS];
    setgamestatus (SUPPRESS_PRINTING);
    if (on) {
	for (unsigned i = 0; i < MAXITEMS; i++) {
	    used[i] = false;
	    if (Player.has_possession(i) && Player.possessions[i].used) {
		used[i] = true;
		Player.possessions[i].used = false;
		item_use (Player.possessions[i]);
	    }
	}
    } else {
	for (unsigned i = 0; i < MAXITEMS; i++) {
	    if (used[i]) {
		Player.possessions[i].used = true;
		item_use (Player.possessions[i]);
	    }
	}
	calc_melee();
	showflags();
	dataprint();
	timeprint();
    }
    resetgamestatus (SUPPRESS_PRINTING);
}

// Switches context dungeon/countryside/city, etc
void change_environment (EEnvironment new_environment)
{
    int i, emerging = false;

    Player.sx = -1;
    Player.sy = -1;		// reset sanctuary if there was one
    resetgamestatus (LOST);	// in case the player gets lost _on_ a site
    resetgamestatus (FAST_MOVE);

    Last_Environment = Current_Environment;
    if (Last_Environment == E_COUNTRYSIDE) {
	LastCountryLocX = Player.x;
	LastCountryLocY = Player.y;
    }
    if (Level) {
	Level->lastx = Player.x;
	Level->lasty = Player.y;
    }

    // FIXME: Exiting from one level to another should use a level stack
    if ((Last_Environment == E_CITY || Last_Environment == E_VILLAGE)
	    && (new_environment == E_MANSION || new_environment == E_HOUSE
		|| new_environment == E_HOVEL || new_environment == E_SEWERS
		|| new_environment == E_ARENA)) {
	LastTownLocX = Player.x;
	LastTownLocY = Player.y;
    } else if ((Last_Environment == E_MANSION || Last_Environment == E_HOUSE
		|| Last_Environment == E_HOVEL || Last_Environment == E_SEWERS
		|| Last_Environment == E_ARENA)
	       && (new_environment == E_CITY || new_environment == E_VILLAGE)) {
	Player.x = LastTownLocX;
	Player.y = LastTownLocY;
	emerging = true;
    }

    Current_Environment = new_environment;
    switch (new_environment) {
	case E_ARENA:
	    TempLevel = Level;
	    if (ok_to_free (TempLevel)) {
		free_level (TempLevel);
		TempLevel = NULL;
	    }
	    Level = new level;
	    clear_level (Level);
	    setgamestatus (ARENA_MODE);
	    load_arena();
	    Player.x = Level->lastx;
	    Player.y = Level->lasty;
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_ABYSS:
	    TempLevel = Level;
	    if (ok_to_free (TempLevel)) {
		free_level (TempLevel);
		TempLevel = NULL;
	    }
	    Level = new level;
	    clear_level (Level);
	    load_abyss();
	    displayfile (Data_AbyssIntro);
	    lose_all_items();
	    Player.x = Level->lastx;
	    Player.y = Level->lasty;
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_CIRCLE:
	    TempLevel = Level;
	    if (ok_to_free (TempLevel)) {
		free_level (TempLevel);
		TempLevel = NULL;
	    }
	    Level = new level;
	    clear_level (Level);
	    load_circle();
	    if (object_uniqueness(STAR_GEM) == UNIQUE_TAKEN) {
		print1 ("A bemused voice says:");
		print2 ("'Why are you here? You already have the Star Gem!'");
		morewait();
	    } else if (Player.rank[CIRCLE] >= INITIATE) {
		print1 ("You hear the voice of the Prime Sorceror:");
		print2 ("'Congratulations on your attainment of the Circle's Demesne.'");
		morewait();
		print1 ("For the honor of the Circle, you may take the Star Gem");
		print2 ("and destroy it on the acme of Star Peak.");
		morewait();
		print1 ("Beware the foul LawBringer who resides there...");
		print2 ("By the way, some of the members of the Circle seem to");
		morewait();
		print1 ("have become a bit jealous of your success --");
		print2 ("I'd watch out for them too if I were you.");
		morewait();
	    } else if (Player.alignment > 0) {
		print1 ("A mysterious ghostly image materializes in front of you.");
		print2 ("It speaks: 'Greetings, fellow abider in Law. I am called");
		morewait();
		print1 ("The LawBringer. If you wish to advance our cause, obtain");
		print2 ("the mystic Star Gem and return it to me on Star Peak.");
		morewait();
		print1 ("Beware the power of the evil Circle of Sorcerors and the");
		print2 ("forces of Chaos which guard the gem.'");
		morewait();
		print1 ("The strange form fades slowly.");
		morewait();
	    }
	    Player.x = Level->lastx;
	    Player.y = Level->lasty;
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_COURT:
	    TempLevel = Level;
	    if (ok_to_free (TempLevel)) {
		free_level (TempLevel);
		TempLevel = NULL;
	    }
	    Level = new level;
	    clear_level (Level);
	    load_court();
	    Player.x = Level->lastx;
	    Player.y = Level->lasty;
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_MANSION:
	    TempLevel = Level;
	    if (ok_to_free (TempLevel)) {
		free_level (TempLevel);
		TempLevel = NULL;
	    }
	    Level = new level;
	    clear_level (Level);
	    load_house (E_MANSION);
	    Player.x = Level->lastx;
	    Player.y = Level->lasty;
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_HOUSE:
	    TempLevel = Level;
	    if (ok_to_free (TempLevel)) {
		free_level (TempLevel);
		TempLevel = NULL;
	    }
	    Level = new level;
	    clear_level (Level);
	    load_house (E_HOUSE);
	    Player.x = Level->lastx;
	    Player.y = Level->lasty;
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_HOVEL:
	    TempLevel = Level;
	    if (ok_to_free (TempLevel)) {
		free_level (TempLevel);
		TempLevel = NULL;
	    }
	    Level = new level;
	    clear_level (Level);
	    load_house (E_HOVEL);
	    Player.x = Level->lastx;
	    Player.y = Level->lasty;
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_DLAIR:
	    TempLevel = Level;
	    if (ok_to_free (TempLevel)) {
		free_level (TempLevel);
		TempLevel = NULL;
	    }
	    Level = new level;
	    clear_level (Level);
	    load_dlair (gamestatusp (KILLED_DRAGONLORD));
	    Player.x = Level->lastx;
	    Player.y = Level->lasty;
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_STARPEAK:
	    TempLevel = Level;
	    if (ok_to_free (TempLevel)) {
		free_level (TempLevel);
		TempLevel = NULL;
	    }
	    Level = new level;
	    clear_level (Level);
	    load_speak (gamestatusp (KILLED_LAWBRINGER));
	    Player.x = Level->lastx;
	    Player.y = Level->lasty;
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_MAGIC_ISLE:
	    TempLevel = Level;
	    if (ok_to_free (TempLevel)) {
		free_level (TempLevel);
		TempLevel = NULL;
	    }
	    Level = new level;
	    clear_level (Level);
	    load_misle (gamestatusp (KILLED_EATER));
	    Player.x = Level->lastx;
	    Player.y = Level->lasty;
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_TEMPLE:
	    TempLevel = Level;
	    if (ok_to_free (TempLevel)) {
		free_level (TempLevel);
		TempLevel = NULL;
	    }
	    Level = new level;
	    clear_level (Level);
	    load_temple (Country->site(Player.x,Player.y).aux);
	    Player.x = Level->lastx;
	    Player.y = Level->lasty;
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_CITY:
	    if (emerging) {
		print1 ("You emerge onto the street.");
		emerging = false;
	    } else
		print1 ("You pass through the massive gates of Rampart, the city.");
	    Level = City;
	    if (!Level) {
		TempLevel = Level;
		if (ok_to_free (TempLevel)) {
		    free_level (TempLevel);
		    TempLevel = NULL;
		}
		Level = City = new level;
		clear_level (Level);
		load_city();
	    }
	    Player.x = Level->lastx;
	    Player.y = Level->lasty;
	    ScreenOffset = Player.y - (ScreenLength / 2);
	    show_screen();
	    break;
	case E_VILLAGE:
	    if (!emerging || !TempLevel || TempLevel->environment != E_VILLAGE) {
		TempLevel = Level;
		if (ok_to_free (TempLevel)) {
		    free_level (TempLevel);
		    TempLevel = NULL;
		}
		Level = new level;
		clear_level (Level);
		Villagenum = Country->site(Player.x,Player.y).aux;
		load_village (Villagenum);
	    } else
		Level = TempLevel;
	    if (emerging) {
		print1 ("You emerge onto the street.");
		emerging = false;
	    } else
		print1 ("You enter a small rural village.");
	    Player.x = Level->lastx;
	    Player.y = Level->lasty;
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_CAVES:
	    print1 ("You enter a dark cleft in a hillside;");
	    print2 ("You note signs of recent passage in the dirt nearby.");
	    if (gamestatusp (MOUNTED)) {
		morewait();
		print1 ("Seeing as you might not be coming back, you feel compelled");
		print2 ("to let your horse go, rather than keep him hobbled outside.");
		resetgamestatus (MOUNTED);
		calc_melee();
	    }
	    MaxDungeonLevels = CAVELEVELS;
	    if (Current_Dungeon != E_CAVES) {
		free_dungeon();
		Dungeon = NULL;
		Level = NULL;
		Current_Dungeon = E_CAVES;
	    }
	    change_level (0, 1, false);
	    break;
	case E_VOLCANO:
	    print1 ("You pass down through the glowing crater.");
	    if (gamestatusp (MOUNTED)) {
		morewait();
		print1 ("Seeing as you might not be coming back, you feel compelled");
		print2 ("to let your horse go, rather than keep him hobbled outside.");
		resetgamestatus (MOUNTED);
		calc_melee();
	    }
	    MaxDungeonLevels = VOLCANOLEVELS;
	    if (Current_Dungeon != E_VOLCANO) {
		free_dungeon();
		Dungeon = NULL;
		Level = NULL;
		Current_Dungeon = E_VOLCANO;
	    }
	    change_level (0, 1, false);
	    break;
	case E_ASTRAL:
	    print1 ("You are in a weird flickery maze.");
	    if (gamestatusp (MOUNTED)) {
		print2 ("Your horse doesn't seem to have made it....");
		resetgamestatus (MOUNTED);
		calc_melee();
	    }
	    MaxDungeonLevels = ASTRALLEVELS;
	    if (Current_Dungeon != E_ASTRAL) {
		free_dungeon();
		Dungeon = NULL;
		Level = NULL;
		Current_Dungeon = E_ASTRAL;
	    }
	    change_level (0, 1, false);
	    break;
	case E_CASTLE:
	    print1 ("You cross the drawbridge. Strange forms move beneath the water.");
	    if (gamestatusp (MOUNTED)) {
		morewait();
		print1 ("Seeing as you might not be coming back, you feel compelled");
		print2 ("to let your horse go, rather than keep him hobbled outside.");
		resetgamestatus (MOUNTED);
	    }
	    MaxDungeonLevels = CASTLELEVELS;
	    if (Current_Dungeon != E_CASTLE) {
		free_dungeon();
		Dungeon = NULL;
		Level = NULL;
		Current_Dungeon = E_CASTLE;
	    }
	    change_level (0, 1, false);
	    break;
	case E_SEWERS:
	    print1 ("You pry open a manhole and descend into the sewers below.");
	    if (gamestatusp (MOUNTED)) {
		print2 ("You horse waits patiently outside the sewer entrance....");
		dismount_steed();
	    }
	    MaxDungeonLevels = SEWERLEVELS;
	    if (Current_Dungeon != E_SEWERS) {
		free_dungeon();
		Dungeon = NULL;
		Level = NULL;
		Current_Dungeon = E_SEWERS;
	    }
	    change_level (0, 1, false);
	    break;
	case E_COUNTRYSIDE:
	    print1 ("You return to the fresh air of the open countryside.");
	    Level = Country;
	    if (Last_Environment == E_CITY) {
		Player.x = 27;
		Player.y = 19;
	    } else {
		Player.x = LastCountryLocX;
		Player.y = LastCountryLocY;
	    }
	    for (i = 0; i < 9; i++)
		c_set (Player.x + Dirs[0][i], Player.y + Dirs[1][i], SEEN);
	    ScreenOffset = Player.y - (ScreenLength / 2);
	    show_screen();
	    break;
	case E_TACTICAL_MAP:
	    print1 ("You are now on the tactical screen; exit off any side to leave");
	    make_country_screen (Country->site(Player.x,Player.y).showchar());
	    make_country_monsters (Country->site(Player.x,Player.y).showchar());
	    Player.x = Level->width / 2;
	    Player.y = Level->height / 2;
	    while (Level->site(Player.x,Player.y).locchar == WATER) {
		if (Player.y < (int)Level->height / 2 + 5)
		    Player.y++;
		else if (Player.x > (int)Level->width / 2 - 10) {
		    Player.x--;
		    Player.y = Level->height / 2 - 5;
		} else {
		    Level->site(Player.x,Player.y).locchar = FLOOR;
		    Level->site(Player.x,Player.y).p_locf = L_NO_OP;
		}
	    }
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_NEVER_NEVER_LAND:
	default:
	    print1 ("There must be some mistake. You don't look like Peter Pan.");
	    print2 ("(But here you are in Never-Never Land)");
	    ScreenOffset = Player.y - (ScreenLength / 2);
	    show_screen();
	    break;
    }
    setlastxy (Player.x, Player.y);
    if (Current_Environment != E_COUNTRYSIDE)
	showroom (Level->site(Player.x,Player.y).roomnumber);
    else
	terrain_check (false);
}

// check every ten minutes
void tenminute_check (void)
{
    if (!(Time % 60))
	return (hourly_check());
    if (Current_Environment == Current_Dungeon)
	wandercheck();
    minute_status_check();
    tenminute_status_check();
    if (Player.status[DISEASED] < 1 && Player.hp < Player.maxhp)
	Player.hp = min (Player.maxhp, Player.hp + Player.level + 1);
    if (Current_Environment != E_COUNTRYSIDE && Current_Environment != E_ABYSS)
	indoors_random_event();
}

// hourly check is same as ten_minutely check except food is also
// checked, and since time moves in hours out of doors, also
// outdoors_random_event is possible
void hourly_check (void)
{
    --Player.food;
    foodcheck();
    if (!hour()) {	// midnight, a new day
	moon_check();
	++Date;
    }
    torch_check();
    if (Current_Environment == Current_Dungeon)
	wandercheck();
    minute_status_check();
    tenminute_status_check();
    if (Player.status[DISEASED] == 0 && Player.hp < Player.maxhp)
	Player.hp = min (Player.maxhp, Player.hp + Player.level + 1);
    if (Current_Environment != E_COUNTRYSIDE && Current_Environment != E_ABYSS)
	indoors_random_event();
}

static void indoors_random_event (void)
{
    switch (random_range (1000)) {
	case 0: print3 ("You feel an unexplainable elation."); morewait(); break;
	case 1: print3 ("You hear a distant rumbling."); morewait(); break;
	case 2: print3 ("You realize your fly is open."); morewait(); break;
	case 3: print3 ("You have a sudden craving for a pecan twirl."); morewait(); break;
	case 4: print3 ("You discover an itch just where you can't scratch it."); morewait(); break;
	case 5: print3 ("You feel strangely lucky."); morewait(); break;
	case 6:
	    print3 ("A cosmic ray strikes!");
	    p_damage (10, UNSTOPPABLE, "a cosmic ray");
	    morewait();
	    break;
	case 7:
	    print3 ("A mysterious healing flux settles over the level.");
	    foreach (m, Level->mlist)
		if (m->hp > 0)
		    m->hp = Monsters[m->id].hp;
	    Player.hp = max (Player.hp, Player.maxhp);
	    morewait();
	    break;
	case 8:
	    print3 ("You catch your second wind....");
	    Player.hp = max (Player.hp, ++Player.maxhp);
	    Player.mana = max (Player.mana, Player.calcmana());
	    morewait();
	    break;
	case 9:
	    print3 ("You find some spare change in a hidden pocket.");
	    Player.cash += Player.level * Player.level + 1;
	    morewait();
	    break;
	case 10:
	    print3 ("You trip over something hidden in a shadow...");
	    morewait();
	    Level->add_thing (Player.x, Player.y, create_object(difficulty()));
	    pickup();
	    break;
	case 11:
	    print3 ("A mysterious voice echoes all around you....");
	    morewait();
	    hint();
	    morewait();
	    break;
	case 12:
	    if (Balance > 0) {
		print3 ("You get word of the failure of your bank!");
		Balance = 0;
	    } else
		print3 ("You feel lucky.");
	    break;
	case 13:
	    if (Balance > 0) {
		print3 ("You get word of a bank error in your favor!");
		Balance += 5000;
	    } else
		print3 ("You feel unlucky.");
	    break;
    }
    dataprint();
    showflags();
}

static void outdoors_random_event (void)
{
    switch (random_range (300)) {
	case 0:
	    switch (Country->site(Player.x,Player.y).showchar()) {
		case TUNDRA: mprint ("It begins to snow. Heavily."); break;
		case DESERT: mprint ("A sandstorm swirls around you."); break;
		default:
		    if (Date > 75 && Date < 330)
			mprint ("You are drenched by a sudden downpour!");
		    else
			mprint ("It begins to snow. Heavily.");
	    }
	    morewait();
	    mprint ("Due to the inclement weather conditions, you have become lost.");
	    morewait();
	    Precipitation += random_range (12) + 1;
	    setgamestatus (LOST);
	    break;
	case 1:
	    mprint ("You enter a field of brightly colored flowers...");
	    mprint ("Wow, man! These are some pretty poppies...");
	    morewait();
	    mprint ("poppies...");
	    morewait();
	    mprint ("poppies...");
	    morewait();
	    print3 ("You become somewhat disoriented...");
	    setgamestatus (LOST);
	    break;
	case 2:
	    mprint ("You discover a sprig of athelas growing lonely in the wild.");
	    morewait();
	    mprint ("Using your herbalist lore you cook a cake of lembas....");
	    morewait();
	    gain_item (Objects[FOOD_LEMBAS]);
	    break;
	case 3:
	    if (Precipitation > 0) {
		mprint ("You are struck by a bolt of lightning!");
		p_damage (random_range (25), ELECTRICITY, "a lightning strike");
		morewait();
	    } else
		mprint ("You feel static cling");
	    break;
	case 4:
	    mprint ("You find a fast-food establishment.");
	    morewait();
	    l_commandant();
	    break;
	case 5: {
	    mprint ("A weird howling tornado hits from out of the West!");
	    morewait();
	    mprint ("You've been caught in a chaos storm!");
	    morewait();
	    unsigned num = random_range (300);
	    if (num < 10) {
		mprint ("Your cell-structure was disrupted!");
		p_damage (random_range (100), UNSTOPPABLE, "a chaos storm");
		morewait();
	    } else if (num < 20) {
		mprint ("The chaos storm warps your frame!");
		morewait();
		mprint ("Your statistical entropy has been maximized.");
		morewait();
		mprint ("You feel average...");
		morewait();
		toggle_item_use (true);
		Player.str = Player.maxstr = Player.con = Player.maxcon = Player.dex = Player.maxdex
		    = Player.agi = Player.maxagi = Player.iq = Player.maxiq = Player.pow = Player.maxpow
		    = (Player.maxstr + Player.maxcon + Player.maxdex + Player.maxagi + Player.maxiq + Player.maxpow + 12) / 6;
		toggle_item_use (false);
	    } else if (num < 30) {
		mprint ("Your entire body glows with an eerie flickering light.");
		morewait();
		toggle_item_use (true);
		for (unsigned i = 1; i < MAXITEMS; i++) {
		    if (Player.has_possession(i)) {
			++Player.possessions[i].plus;
			if (Player.possessions[i].objchar == STICK)
			    Player.possessions[i].charge += 10;
			Player.possessions[i].blessing += 10;
		    }
		}
		toggle_item_use (false);
		cleanse (1);
		mprint ("You feel filled with energy!");
		morewait();
		Player.maxpow += 5;
		Player.pow += 5;
		Player.mana = Player.maxmana = Player.calcmana() * 5;
		mprint ("You also feel weaker. Paradoxical, no?");
		morewait();
		Player.con -= 5;
		Player.maxcon -= 5;
		if (Player.con < 3)
		    p_death ("congestive heart failure");
	    } else if (num < 40) {
		mprint ("Your entire body glows black.");
		morewait();
		dispel (-1);
		dispel (-1);
		Player.pow -= 10;
		Player.mana = 0;
	    } else if (num < 60) {
		mprint ("The storm deposits you in a strange place....");
		morewait();
		Player.x = random_range (Level->width);
		Player.y = random_range (Level->height);
		screencheck (Player.y);
	    } else if (num < 70) {
		mprint ("A tendril of the storm condenses and falls into your hands.");
		morewait();
		gain_item (make_artifact());
	    } else if (num < 80) {
		if (gamestatusp (MOUNTED)) {
		    mprint ("Your horse screams as he is transformed into an");
		    morewait();
		    mprint ("imaginary unseen dead tortoise.");
		    morewait();
		    mprint ("You are now on foot.");
		    morewait();
		    resetgamestatus (MOUNTED);
		} else {
		    mprint ("You notice you are riding a horse. Odd. Very odd....");
		    morewait();
		    mprint ("Now that's a horse of a different color!");
		    morewait();
		    setgamestatus (MOUNTED);
		}
	    } else if (num < 90) {
		mprint ("You feel imbued with godlike power....");
		morewait();
		wish (1);
	    } else if (num < 100) {
		mprint ("The chaos storm has wiped your memory!");
		morewait();
		mprint ("You feel extraordinarily naive....");
		morewait();
		mprint ("You can't remember a thing! Not even your name.");
		morewait();
		Player.xp = 0;
		Player.level = 0;
		for (unsigned i = 0; i < NUMGUILDS; i++)
		    Player.rank[i] = 0;
		forget_all_spells();
		rename_player();
	    } else {
		mprint ("You survive the chaos storm relatively unscathed.");
		morewait();
		mprint ("It was a learning experience.");
		morewait();
		gain_experience (1000);
	    }
	    } break;
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	    mprint ("An encounter!");
	    morewait();
	    change_environment (E_TACTICAL_MAP);
	    break;
	case 11:
	    mprint ("You find a Traveller's Aid station with maps of the local area.");
	    morewait();
	    if (gamestatusp (LOST)) {
		resetgamestatus (LOST);
		mprint ("You know where you are now.");
	    }
	    for (int i = Player.x - 5; i < Player.x + 6; ++i) {
		for (int j = Player.y - 5; j < Player.y + 6; ++j) {
		    if (inbounds (i, j)) {
			c_set (i, j, SEEN);
			c_set (i, j, CHANGED);
		    }
		}
	    }
	    show_screen();
	    break;
	case 12:
	    if (!gamestatusp (MOUNTED)) {
		mprint ("You develop blisters....");
		p_damage (1, UNSTOPPABLE, "blisters");
	    }
	    break;
	case 13:
	    mprint ("You discover an itch just where you can't scratch it.");
	    break;
	case 14:
	    mprint ("A cosmic ray strikes!");
	    morewait();
	    p_damage (10, UNSTOPPABLE, "a cosmic ray");
	    break;
	case 15:
	    mprint ("You feel strangely lucky.");
	    break;
	case 16:
	    mprint ("The west wind carries with it a weird echoing voice....");
	    morewait();
	    hint();
	    morewait();
	    break;
    }
    dataprint();
    showflags();
}

char getlocation (void)
{
    menuprint (" (enter location [HCL]) ");
    showmenu();
    char c = '\0';
    while (c == '\0') {
	switch ((c = mcigetc())) {
	    case 'h': menuprint (" High."); break;
	    case 'c': menuprint (" Center."); break;
	    case 'l': menuprint (" Low."); break;
	    default: c = '\0'; break;
	}
    }
    showmenu();
    return (c-'a'+'A');
}

// chance for player to resist magic somehow
// hostile_magic ranges in power from 0 (weak) to 10 (strong)
bool magic_resist (unsigned hostile_magic)
{
    if ((Player.rank[COLLEGE] + Player.rank[CIRCLE] > 0) && (Player.level / 2 + urandom_range (20) > hostile_magic + urandom_range (20))) {
	if (Player.mana > hostile_magic * hostile_magic) {
	    mprint ("Thinking fast, you defend youself with a counterspell!");
	    Player.mana -= hostile_magic * hostile_magic;
	    dataprint();
	    return (true);
	}
    }
    if (Player.level / 4 + Player.status[PROTECTION] + urandom_range (20) > hostile_magic + urandom_range (30)) {
	mprint ("You resist the spell!");
	return (true);
    } else
	return (false);
}

void terrain_check (bool takestime)
{
    bool faster = false;
    if (Player.patron == DRUID) {
	faster = true;
	switch (urandom_range (32)) {
	    case 0: print2 ("Along the many paths of nature..."); break;
	    case 1: print2 ("You move swiftly through the wilderness."); break;
	}
    } else if (gamestatusp (MOUNTED)) {
	faster = true;
	switch (urandom_range (32)) {
	    case 0:
	    case 1: print2 ("Clippity Clop."); break;
	    case 2: print2 ("....my spurs go jingle jangle jingle...."); break;
	    case 3: print2 ("....as I go riding merrily along...."); break;
	}
    } else if (Player.has_possession(O_BOOTS) && Player.possessions[O_BOOTS].usef == I_BOOTS_7LEAGUE) {
	takestime = false;
	switch (urandom_range (32)) {
	    case 0: print2 ("Boingg!"); break;
	    case 1: print2 ("Whooosh!"); break;
	    case 2: print2 ("Over hill, over dale...."); break;
	    case 3: print2 ("...able to leap over 7 leagues in a single bound...."); break;
	}
    } else if (Player.status[SHADOWFORM]) {
	faster = true;
	switch (urandom_range (32)) {
	    case 0: print2 ("As swift as a shadow."); break;
	    case 1: print2 ("\"I walk through the trees...\""); break;
	}
    } else {
	switch (urandom_range (32)) {
	    case 0: print2 ("Trudge. Trudge."); break;
	    case 1: print2 ("The road goes ever onward...."); break;
	}
    }
    switch (Country->site(Player.x,Player.y).showchar()) {
	case RIVER:
	    if ((Player.y < 6) && (Player.x > 20))
		locprint ("Star Lake.");
	    else if (Player.y < 41) {
		if (Player.x < 10)
		    locprint ("Aerie River.");
		else
		    locprint ("The Great Flood.");
	    } else if (Player.x < 42)
		locprint ("The Swamp Runs.");
	    else
		locprint ("River Greenshriek.");
	    if (takestime) {
		for (unsigned i = 0; i < 3; ++i) {
		    Time += 60;
		    hourly_check();
		}
	    }
	    break;
	case ROAD:
	    locprint ("A well-maintained road.");
	    if (takestime) {
		Time += 60;
		hourly_check();
	    }
	    break;
	case PLAINS:
	    locprint ("A rippling sea of grass.");
	    if (takestime) {
		Time += 60;
		hourly_check();
		if (!faster) {
		    Time += 60;
		    hourly_check();
		}
	    }
	    break;
	case TUNDRA:
	    locprint ("The Great Northern Wastes.");
	    if (takestime) {
		Time += 60;
		hourly_check();
		if (!faster) {
		    Time += 60;
		    hourly_check();
		}
	    }
	    break;
	case FOREST:
	    if (Player.y < 10)
		locprint ("The Deepwood.");
	    else if (Player.y < 18)
		locprint ("The Forest of Erelon.");
	    else if (Player.y < 46)
		locprint ("The Great Forest.");
	    if (takestime) {
		Time += 60;
		hourly_check();
		if (Player.rank[PRIESTHOOD] == NOT_A_BELIEVER || Player.patron != DRUID) {
		    Time += 60;
		    hourly_check();
		    if (!faster) {
			Time += 60;
			hourly_check();
		    }
		}
	    }
	    break;
	case JUNGLE:
	    locprint ("Greenshriek Jungle.");
	    if (takestime) {
		Time += 60;
		hourly_check();
		Time += 60;
		hourly_check();
		if (!faster) {
		    Time += 60;
		    hourly_check();
		    Time += 60;
		    hourly_check();
		}
	    }
	    break;
	case DESERT:
	    locprint ("The Waste of Time.");
	    if (takestime) {
		Time += 60;
		hourly_check();
		Time += 60;
		hourly_check();
		if (!faster) {
		    Time += 60;
		    hourly_check();
		    Time += 60;
		    hourly_check();
		}
	    }
	    break;
	case MOUNTAINS:
	    if ((Player.y < 9) && (Player.x < 12))
		locprint ("The Magic Mountains");
	    else if ((Player.y < 9) && (Player.y > 2) && (Player.x < 40))
		locprint ("The Peaks of the Fist.");
	    else if (Player.x < 52)
		locprint ("The Rift Mountains.");
	    else
		locprint ("Borderland Mountains.");
	    if (takestime) {
		for (unsigned i = 0; i < 5u+3*!faster; ++i) {
		    Time += 60;
		    hourly_check();
		}
	    }
	    break;
	case PASS:
	    locprint ("A hidden pass.");
	    if (takestime) {
		Time += 60;
		hourly_check();
	    }
	    break;
	case CHAOS_SEA:
	    locprint ("The Sea of Chaos.");
	    if (takestime) {
		Time += 60;
		hourly_check();
	    }
	    mprint ("You have entered the sea of chaos...");
	    morewait();
	    l_chaos();
	    break;
	case SWAMP:
	    locprint ("The Loathly Swamp.");
	    if (takestime) {
		for (unsigned i = 0; i < 6u+2*!faster; ++i) {
		    Time += 60;
		    hourly_check();
		}
	    }
	    break;
	case CITY:
	    if (gamestatusp (LOST)) {
		resetgamestatus (LOST);
		mprint ("Well, I guess you know where you are now....");
	    }
	    locprint ("Outside Rampart, the city.");
	    break;
	case VILLAGE:
	    if (gamestatusp (LOST)) {
		resetgamestatus (LOST);
		mprint ("The village guards let you know where you are....");
	    }
	    locprint ("Outside a small village.");
	    break;
	case CAVES:
	    locprint ("A deserted hillside.");
	    if (takestime) {
		Time += 60;
		hourly_check();
	    }
	    mprint ("You notice a concealed entrance into the hill.");
	    break;
	case CASTLE:
	    locprint ("Near a fortified castle.");
	    if (takestime) {
		Time += 60;
		hourly_check();
	    }
	    mprint ("The castle is hewn from solid granite. The drawbridge is down.");
	    break;
	case TEMPLE:
	    static const char c_TempleDesc[] =
		"\0A rough-hewn granite temple.\0"
		"A black pyramidal temple made of sandstone.\0"
		"A classical marble-columned temple.\0"
		"A temple of ebony adorned with ivory.\0"
		"A temple formed of living trees.\0"
		"A temple of some mysterious blue crystal.";
	    locprint (zstrn (c_TempleDesc, Country->site(Player.x,Player.y).aux, DESTINY));
	    if (takestime) {
		Time += 60;
		hourly_check();
	    }
	    mprint ("You notice an entrance conveniently at hand.");
	    break;
	case MAGIC_ISLE:
	    locprint ("A strange island in the midst of the Sea of Chaos.");
	    if (takestime) {
		Time += 60;
		hourly_check();
	    }
	    mprint ("There is a narrow causeway to the island from here.");
	    break;
	case STARPEAK:
	    locprint ("Star Peak.");
	    if (takestime) {
		Time += 60;
		hourly_check();
	    }
	    mprint ("The top of the mountain seems to glow with a allochroous aura.");
	    break;
	case DRAGONLAIR:
	    locprint ("A rocky chasm.");
	    if (takestime) {
		Time += 60;
		hourly_check();
	    }
	    mprint ("You are at a cave entrance from which you see the glint of gold.");
	    break;
	case VOLCANO:
	    locprint ("HellWell Volcano.");
	    if (takestime) {
		Time += 60;
		hourly_check();
	    }
	    mprint ("A shimmer of heat lightning plays about the crater rim.");
	    break;
	default:
	    locprint ("I haven't any idea where you are!!!");
	    break;
    }
    outdoors_random_event();
}

void countrysearch (void)
{
    Time += 60;
    hourly_check();
    for (int x = Player.x - 1; x < Player.x + 2; ++x) {
	for (int y = Player.y - 1; y < Player.y + 2; ++y) {
	    if (!inbounds (x, y) || !c_statusp (x,y, SECRET))
		continue;
	    clearmsg();
	    mprint ("Your search was fruitful!");
	    c_reset (x, y, SECRET);
	    c_set (x, y, CHANGED);
	    mprint ("You discovered:");
	    mprint (countryid (Country->site(x,y).locchar));
	}
    }
}

const char* countryid (int terrain)
{
    static const uint8_t _terrains[] =  {
	MOUNTAINS & 0xff, PLAINS & 0xff, TUNDRA & 0xff, ROAD & 0xff, PASS & 0xff,
	RIVER & 0xff, CITY & 0xff, VILLAGE & 0xff, FOREST & 0xff, JUNGLE & 0xff,
	SWAMP & 0xff, VOLCANO & 0xff, CASTLE & 0xff, STARPEAK & 0xff, DRAGONLAIR & 0xff,
	MAGIC_ISLE & 0xff, CAVES & 0xff, TEMPLE & 0xff, DESERT & 0xff, CHAOS_SEA & 0xff
    };
    static const char _terrain_names[] =
	"Almost impassable mountains\0"
	"Seemingly endless plains\0"
	"A frosty stretch of tundra\0"
	"A paved highway\0"
	"A secret mountain pass\0"
	"A rolling river\0"
	"The city of Rampart\0"
	"A rural village\0"
	"A verdant forest\0"
	"A densely overgrown jungle\0"
	"A swampy fen\0"
	"A huge active volcano\0"
	"An imposing castle\0"
	"A mysterious mountain.\0"
	"A cavern filled with treasure.\0"
	"An island emanating magic.\0"
	"A hidden cave entrance\0"
	"A neoclassical temple\0"
	"A sere desert\0"
	"The Sea of Chaos\0"
	"I have no idea.\0";

    unsigned i; const uint8_t tval = terrain & 0xff;
    for (i = 0; i < ArraySize(_terrains); ++i)
	if (_terrains[i] == tval)
	    break;
    return (zstrn (_terrain_names, i, ArraySize(_terrains)+1));
}

static const char* sitenames[NUMCITYSITES] = {	// alphabetical listing
    "alchemist", "arena", "armorer", "bank", "brothel",
    "casino", "castle", "city gates", "collegium magii", "condo",
    "department of public works", "diner", "explorers' club", "fast food", "gymnasium",
    "healer", "hospice", "jail", "les crapuleux", "library",
    "mercenary guild", "oracle", "order of paladins", "pawn shop", "sorcerors' guild ",
    "tavern", "temple", "thieves' guild"
};

static int sitenums[ArraySize(sitenames)] = {	// the order matches sitenames[]
    L_ALCHEMIST, L_ARENA, L_ARMORER, L_BANK, L_BROTHEL,
    L_CASINO, L_CASTLE, L_COUNTRYSIDE, L_COLLEGE, L_CONDO,
    L_DPW, L_DINER, L_CLUB, L_COMMANDANT, L_GYM,
    L_HEALER, L_CHARITY, L_JAIL, L_CRAP, L_LIBRARY,
    L_MERC_GUILD, L_ORACLE, L_ORDER, L_PAWN_SHOP, L_SORCERORS,
    L_TAVERN, L_TEMPLE, L_THIEVES_GUILD
};

static void showknownsites (unsigned first, unsigned last)
{
    bool printed = false;
    menuclear();
    menuprint ("\nPossible Sites:\n");
    for (unsigned i = first; i <= last; i++) {
	if (CitySiteList[sitenums[i] - CITYSITEBASE].known) {
	    printed = true;
	    menuprint (sitenames[i]);
	    menuprint ("\n");
	}
    }
    if (!printed)
	menuprint ("\nNo known sites match that prefix!");
    showmenu();
}

int parsecitysite (void)
{
    int first, last, pos;
    char byte, prefix[80];
    int found = 0;
    int f, l;

    first = 0;
    last = NUMCITYSITES - 1;
    pos = 0;
    print2 ("");
    do {
	byte = mgetc();
	if (byte == KEY_BACKSPACE) {
	    if (pos > 0) {
		prefix[--pos] = '\0';
		byte = prefix[pos - 1];
		f = first;
		while (f >= 0 && !strncmp (prefix, sitenames[f], pos)) {
		    if (CitySiteList[sitenums[f] - CITYSITEBASE].known)
			first = f;
		    f--;
		}
		l = last;
		while (l < NUMCITYSITES-1 && !strncmp (prefix, sitenames[l], pos)) {
		    if (CitySiteList[sitenums[l] - CITYSITEBASE].known)
			last = l;
		    l++;
		}
		if (found)
		    found = 0;
		print2 (prefix);
	    }
	    if (pos == 0) {
		first = 0;
		last = NUMCITYSITES - 1;
		found = 0;
		print2 ("");
	    }
	} else if (byte == KEY_ESCAPE) {
	    xredraw();
	    return ABORT;
	} else if (byte == '?')
	    showknownsites (first, last);
	else if (byte != KEY_ENTER) {
	    byte = tolower (byte);
	    if (found)
		continue;
	    f = first;
	    l = last;
	    while (f < NUMCITYSITES && (!CitySiteList[sitenums[f] - CITYSITEBASE].known || (int)strlen (sitenames[f]) < pos || sitenames[f][pos] < byte))
		f++;
	    while (l >= 0 && (!CitySiteList[sitenums[l] - CITYSITEBASE].known || (int)strlen (sitenames[l]) < pos || sitenames[l][pos] > byte))
		--l;
	    if (l < f)
		continue;
	    prefix[pos++] = byte;
	    prefix[pos] = '\0';
	    nprint2 (prefix + pos - 1);
	    first = f;
	    last = l;
	    if (first == last && !found) {	// unique name
		found = 1;
		nprint2 (sitenames[first] + pos);
	    }
	}
    } while (byte != KEY_ENTER);
    xredraw();
    if (found)
	return sitenums[first] - CITYSITEBASE;
    else {
	print3 ("That is an ambiguous abbreviation!");
	return ABORT;
    }
}

// are there hostile monsters within 2 moves?
bool hostilemonstersnear (void)
{
    for (const monster& m : Level->mlist)
	if (m_statusp(m, HOSTILE) && abs_distance(m.x,Player.x) < 3 && abs_distance(m.y,Player.y) < 3)
	    return (true);
    return (false);
}

// random effects from some of stones in villages
// if alignment of stone is alignment of player, gets done sooner
int stonecheck (int alignment)
{
    int *stone, match = false, cycle = false, i;

    if (alignment == 1) {
	stone = &Lawstone;
	match = Player.alignment > 0;
    } else if (alignment == -1) {
	stone = &Chaostone;
	match = Player.alignment < 0;
    } else {
	stone = &Mindstone;
	match = false;
    }
    *stone += random_range (4) + (match ? random_range (4) : 0);
    switch ((*stone)++) {
	case 0:
	case 2:
	case 4:
	case 6:
	case 8:
	case 10:
	case 12:
	case 14:
	case 16:
	case 18:
	case 20:
	case 22:
	case 24:
	case 26:
	case 28:
	case 30:
	case 32:
	case 34:
	case 36:
	case 38:
	case 40:
	    print1 ("The stone glows grey.");
	    print2 ("Not much seems to happen this time.");
	    (*stone)--;
	    break;
	case 1:
	    print1 ("The stone glows black");
	    print2 ("A burden has been removed from your shoulders.....");
	    print3 ("Your pack has disintegrated!");
	    Player.pack.clear();
	    break;
	case 3:
	    print1 ("The stone glows microwave");
	    print2 ("A vortex of antimana spins about you!");
	    morewait();
	    dispel (-1);
	    break;
	case 5:
	    print1 ("The stone glows infrared");
	    print2 ("A portal opens nearby and an obviously confused monster appears!");
	    summon (-1, -1);
	    morewait();
	    break;
	case 7:
	    print1 ("The stone glows brick red");
	    print2 ("A gold piece falls from the heavens into your money pouch!");
	    Player.cash++;
	    break;
	case 9:
	    print1 ("The stone glows cherry red");
	    print2 ("A flush of warmth spreads through your body.");
	    augment (1);
	    break;
	case 11:
	    print1 ("The stone glows orange");
	    print2 ("A flux of energy blasts you!");
	    manastorm (Player.x, Player.y, random_range (Player.maxhp) + 1);
	    break;
	case 13:
	    print1 ("The stone glows lemon yellow");
	    print2 ("You're surrounded by enemies! You begin to foam at the mouth.");
	    Player.status[BERSERK] += 10;
	    break;
	case 15:
	    print1 ("The stone glows yellow");
	    print2 ("Oh no! The DREADED AQUAE MORTIS!");
	    morewait();
	    print2 ("No, wait, it's just your imagination.");
	    break;
	case 17:
	    print1 ("The stone glows chartreuse");
	    print2 ("Your joints stiffen up.");
	    Player.agi -= 3;
	    break;
	case 19:
	    print1 ("The stone glows green");
	    print2 ("You come down with an acute case of Advanced Leprosy.");
	    Player.status[DISEASED] = 1100;
	    Player.hp = 1;
	    Player.dex -= 5;
	    break;
	case 21:
	    print1 ("The stone glows forest green");
	    print2 ("You feel wonderful!");
	    Player.status[HERO] += 10;
	    break;
	case 23:
	    print1 ("The stone glows cyan");
	    print2 ("You feel a strange twisting sensation....");
	    morewait();
	    strategic_teleport (-1);
	    break;
	case 25:
	    print1 ("The stone glows blue");
	    morewait();
	    print1 ("You feel a tingle of an unearthly intuition:");
	    morewait();
	    hint();
	    break;
	case 27:
	    print1 ("The stone glows navy blue");
	    print2 ("A sudden shock of knowledge overcomes you.");
	    morewait();
	    clearmsg();
	    identify (1);
	    knowledge (1);
	    break;
	case 29:
	    print1 ("The stone glows blue-violet");
	    print2 ("You feel forgetful.");
	    for (i = 0; i < NUMSPELLS; i++) {
		if (spell_is_known(ESpell(i))) {
		    forget_spell(ESpell(i));
		    break;
		}
	    }
	    break;
	case 31:
	    print1 ("The stone glows violet");
	    morewait();
	    acquire (0);
	    break;
	case 33:
	    print1 ("The stone glows deep purple");
	    print2 ("You vanish.");
	    Player.status[INVISIBLE] += 10;
	    break;
	case 35:
	    print1 ("The stone glows ultraviolet");
	    print2 ("All your hair rises up on end.... A bolt of lightning hits you!");
	    p_damage (random_range (Player.maxhp), ELECTRICITY, "mystic lightning");
	    break;
	case 37:
	    print1 ("The stone glows roentgen");
	    print2 ("You feel more experienced.");
	    gain_experience ((Player.level + 1) * 250);
	    break;
	case 39:
	    print1 ("The stone glows gamma");
	    print2 ("Your left hand shines silvery, and your right emits a golden aura.");
	    morewait();
	    enchant (1);
	    bless (1);
	    print3 ("Your hands stop glowing.");
	    break;
	case 41:
	case 42:
	case 43:
	case 44:
	case 45:
	case 46:
	case 47:
	case 48:
	case 49:
	    print1 ("The stone glows cosmic!");
	    print2 ("The stone's energy field quiets for a moment...");
	    *stone = 50;
	    cycle = true;
	    break;
	default:
	    print1 ("The stone glows polka-dot (?!?!?!?)");
	    print2 ("You feel a strange twisting sensation....");
	    morewait();
	    *stone = 0;
	    strategic_teleport (-1);
	    break;
    }
    calc_melee();
    return (cycle);
}

void alert_guards (void)
{
    bool foundguard = false;
    foreach (m, Level->mlist) {
	if ((m->id == GUARD || (m->id == HISCORE_NPC && m->aux2 == NPC_JUSTICIAR)) && m->hp > 0) {
	    foundguard = true;
	    m_status_reset (*m, ASLEEP);
	    m_status_set (*m, HOSTILE);
	}
    }
    if (foundguard) {
	mprint ("You hear a whistle and the sound of running feet!");
	if (Current_Environment == E_CITY)
	    Level->site(40,60).p_locf = L_NO_OP;	// pacify_guards restores this
    }
    if (!foundguard && Current_Environment == E_CITY && !gamestatusp (DESTROYED_ORDER)) {
	bool suppress = gamestatusp (SUPPRESS_PRINTING);
	resetgamestatus (SUPPRESS_PRINTING);
	print2 ("The last member of the Order of Paladins dies....");
	morewait();
	gain_experience (1000);
	Player.alignment -= 250;
	if (!gamestatusp (KILLED_LAWBRINGER)) {
	    print1 ("A chime sounds from far away.... The sound grows stronger....");
	    print2 ("Suddenly the great shadowy form of the LawBringer appears over");
	    print3 ("the city. He points his finger at you....");
	    morewait();
	    print1 ("\"Cursed art thou, minion of chaos! May thy strength fail thee");
	    print2 ("in thy hour of need!\" You feel an unearthly shiver as the");
	    print3 ("LawBringer waves his palm across the city skies....");
	    morewait();
	    Player.str /= 2;
	    dataprint();
	    print1 ("You hear a bell tolling, and eerie moans all around you....");
	    print2 ("Suddenly, the image of the LawBringer is gone.");
	    print3 ("You hear a guardsman's whistle in the distance!");
	    morewait();
	    resurrect_guards();
	} else {
	    print1 ("The Order's magical defenses have dropped, and the");
	    print2 ("Legions of Chaos strike....");
	    morewait();
	    print1 ("The city shakes! An earthquake has struck!");
	    print2 ("Cracks open in the street, and a chasm engulfs the Order HQ!");
	    print3 ("Flames lick across the sky and you hear wild laughter....");
	    morewait();
	    gain_experience (5000);
	    destroy_order();
	}
	if (suppress)
	    setgamestatus (SUPPRESS_PRINTING);
    }
}

static void destroy_order (void)
{
    setgamestatus (DESTROYED_ORDER);
    Level->mlist.clear();
    // Disable jail
    for (unsigned i = 35; i < 46; ++i) {
	for (unsigned j = 60; j < 63; ++j) {
	    location& l = Level->site(i,j);
	    if (i == 40 && (j == 60 || j == 61)) {
		l.lstatus &= ~SECRET;
		l.locchar = FLOOR;
		l.p_locf = L_NO_OP;
	    } else {
		l.locchar = RUBBLE;
		l.p_locf = L_RUBBLE;
	    }
	    l.lstatus |= CHANGED;
	    monster& m = make_site_monster (i, j, GHOST);
	    m.monstring = "ghost of a Paladin";
	    m_status_set (m, HOSTILE);
	}
    }
}

unsigned maneuvers (void)
{
    unsigned m = 2 + Player.level / 7;
    if (Player.rank[ARENA])	++m;
    if (Player.status[HASTED])	m *= 2;
    if (Player.status[SLOWED])	m /= 2;
    return (min (8U, max (1U, m)));
}

// for when haste runs out, etc.
static void default_maneuvers (void)
{
    morewait();
    clearmsg();
    print1 ("Warning, resetting your combat options to the default.");
    print2 ("Use the 'F' command to select which options you prefer.");
    morewait();
    const unsigned nManeuvers = maneuvers();
    fill_n ((uint32_t*) Player.meleestr.data(), DivRU(nManeuvers,2), vpack('A','C','B','C'));
    Player.meleestr[nManeuvers*2] = '\0';
}
