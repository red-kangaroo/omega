#include "glob.h"

//----------------------------------------------------------------------

static void player_miss(struct monster *m, int dtype);
static void p_fumble(int dtype);
static void drop_weapon(void);
static void break_weapon(void);
static void movecursor(int *x, int *y, int dx, int dy);
static void gain_level(void);
static long expval(int plevel);
static void tacplayer(struct monster *m);
static int player_hit(int hitmod, int hitloc, struct monster *m);
static void indoors_random_event(void);
static void outdoors_random_event(void);
static void showknownsites(int first, int last);
static void destroy_order(void);
static void default_maneuvers(void);
static void fight_monster (monster *m);

//----------------------------------------------------------------------

// check to see if too much tunneling has been done in this level
void tunnelcheck (void)
{
    if ((Level->depth == 0 && Current_Environment != E_DLAIR) || Current_Environment == E_ASTRAL)
	return;
    Level->tunnelled++;
    if (Level->tunnelled > LENGTH / 4)
	mprint ("Dust and stone fragments fall on you from overhead.");
    if (Level->tunnelled > LENGTH / 2)
	mprint ("You hear groaning and creaking noises.");
    if (Level->tunnelled > 3 * LENGTH / 4)
	mprint ("The floor trembles and you hear a loud grinding screech.");
    if (Level->tunnelled > LENGTH) {
	mprint ("With a scream of tortured stone, the entire dungeon caves in!!!");
	gain_experience (5000);
	if (Player.status[SHADOWFORM]) {
	    change_environment (E_COUNTRYSIDE);
	    switch (Country[Player.x][Player.y].base_terrain_type) {
		case CASTLE:
		case STARPEAK:
		case CAVES:
		case VOLCANO:
		    Country[Player.x][Player.y].current_terrain_type = MOUNTAINS;
		    break;
		case DRAGONLAIR:
		    Country[Player.x][Player.y].current_terrain_type = DESERT;
		    break;
		case MAGIC_ISLE:
		    Country[Player.x][Player.y].current_terrain_type = CHAOS_SEA;
		    break;
	    }
	    Country[Player.x][Player.y].base_terrain_type = Country[Player.x][Player.y].current_terrain_type;
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
    strcpy (Str1, "");
    strcpy (Str2, "");
    switch (Current_Environment) {
	case E_MANSION:
	    strcpy (Str2, "A luxurious mansion: ");
	    break;
	case E_HOUSE:
	    strcpy (Str2, "A house: ");
	    break;
	case E_HOVEL:
	    strcpy (Str2, "A hovel: ");
	    break;
	case E_CITY:
	    strcpy (Str2, "The City of Rampart");
	    break;
	case E_VILLAGE:
	    switch (Villagenum) {
		case 1:
		    strcpy (Str2, "The Village of Star View");
		    break;
		case 2:
		    strcpy (Str2, "The Village of Woodmere");
		    break;
		case 3:
		    strcpy (Str2, "The Village of Stormwatch");
		    break;
		case 4:
		    strcpy (Str2, "The Village of Thaumaris");
		    break;
		case 5:
		    strcpy (Str2, "The Village of Skorch");
		    break;
		case 6:
		    strcpy (Str2, "The Village of Whorfen");
		    break;
	    }
	    break;
	case E_CAVES:
	    strcpy (Str2, "The Goblin Caves: ");
	    break;
	case E_CASTLE:
	    strcpy (Str2, "The Archmage's Castle: ");
	    break;
	case E_ASTRAL:
	    strcpy (Str2, "The Astral Plane: ");
	    break;
	case E_VOLCANO:
	    strcpy (Str2, "The Volcano: ");
	    break;
	case E_SEWERS:
	    strcpy (Str2, "The Sewers: ");
	    break;
	case E_TACTICAL_MAP:
	    strcpy (Str2, "The Tactical Map ");
	    break;
	default:
	    strcpy (Str2, "");
	    break;
    }
    if (Current_Environment == Current_Dungeon) {
	strcpy (Str1, "Level ");
	if (Level->depth < 10) {
	    Str1[6] = Level->depth + '0';
	    Str1[7] = 0;
	} else {
	    Str1[6] = (Level->depth / 10) + '0';
	    Str1[7] = (Level->depth % 10) + '0';
	    Str1[8] = 0;
	}
	strcat (Str1, " (");
	strcat (Str1, roomname (i));
	strcat (Str1, ")");
    } else if (strlen (Str2) == 0 || Current_Environment == E_MANSION || Current_Environment == E_HOUSE || Current_Environment == E_HOVEL)
	strcpy (Str1, roomname (i));
    strcat (Str2, Str1);
    locprint (Str2);
}

int player_on_sanctuary (void)
{
    if ((Player.x == Player.sx) && (Player.y == Player.sy))
	return (TRUE);
    else {
	if (Player.patron) {
	    if ((Level->site[Player.x][Player.y].locchar == ALTAR) && (Level->site[Player.x][Player.y].aux == Player.patron))
		return (TRUE);
	    else
		return (FALSE);
	} else
	    return (FALSE);
    }
}

// check a move attempt, maybe attack something, return TRUE if ok to move.
// x y is the proposed place to move to
int p_moveable (int x, int y)
{
    setgamestatus (SKIP_MONSTERS);
    if (!inbounds (x, y))
	return (FALSE);
    else if (Player.status[SHADOWFORM]) {
	switch (Level->site[x][y].p_locf) {
	    case L_CHAOS:
	    case L_ABYSS:
	    case L_VOID:
		return confirmation();
	    default:
		resetgamestatus (SKIP_MONSTERS);
		return (TRUE);
	}
    } else if (loc_statusp (x, y, SECRET)) {
	if (!gamestatusp (FAST_MOVE))
	    print3 ("Ouch!");
	return (FALSE);
    } else if (Level->creature(x,y)) {
	if (!gamestatusp (FAST_MOVE)) {
	    fight_monster (Level->creature(x,y));
	    resetgamestatus (SKIP_MONSTERS);
	    return (FALSE);
	} else
	    return (FALSE);
    } else if ((Level->site[x][y].locchar == WALL) || (Level->site[x][y].locchar == STATUE) || (Level->site[x][y].locchar == PORTCULLIS) || (Level->site[x][y].locchar == CLOSED_DOOR)
	       || (gamestatusp (FAST_MOVE)
		   && ((Level->site[x][y].locchar == HEDGE) || (Level->site[x][y].locchar == LAVA) || (Level->site[x][y].locchar == ABYSS) || (Level->site[x][y].locchar == VOID_CHAR) || (Level->site[x][y].locchar == FIRE)
		       || (Level->site[x][y].locchar == WHIRLWIND) || (Level->site[x][y].locchar == WATER) || (Level->site[x][y].locchar == LIFT) || (Level->site[x][y].locchar == TRAP)))) {
	if (!gamestatusp (FAST_MOVE))
	    print3 ("Ouch!");
	return (FALSE);
    } else if (optionp (CONFIRM)) {
	if ((Level->site[x][y].locchar == HEDGE) || (Level->site[x][y].locchar == LAVA) || (Level->site[x][y].locchar == FIRE) || (Level->site[x][y].locchar == WHIRLWIND) || (Level->site[x][y].locchar == ABYSS) || (Level->site[x][y].locchar == VOID_CHAR)
	    || (Level->site[x][y].locchar == WATER) || (Level->site[x][y].locchar == RUBBLE) || (Level->site[x][y].locchar == LIFT) || (Level->site[x][y].locchar == TRAP)) {
	    // horses WILL go into water...
	    if (gamestatusp (MOUNTED)) {
		if (Level->site[x][y].locchar != WATER || Level->site[x][y].p_locf != L_WATER) {
		    print1 ("You can't convince your steed to continue.");
		    setgamestatus (SKIP_MONSTERS);
		    return (FALSE);
		} else
		    return (TRUE);
	    } else if (confirmation())
		resetgamestatus (SKIP_MONSTERS);
	    else
		setgamestatus (SKIP_MONSTERS);
	    return (!gamestatusp (SKIP_MONSTERS));
	} else {
	    resetgamestatus (SKIP_MONSTERS);
	    return (TRUE);
	}
    } else {
	resetgamestatus (SKIP_MONSTERS);
	return (TRUE);
    }
}

// check a move attempt in the countryside
int p_country_moveable (int x, int y)
{
    if (!inbounds (x, y))
	return (FALSE);
    else if (optionp (CONFIRM)) {
	if ((Country[x][y].current_terrain_type == CHAOS_SEA) || (Country[x][y].current_terrain_type == MOUNTAINS))
	    return (confirmation());
	else
	    return (TRUE);
    } else
	return (TRUE);
}

// search once particular spot
void searchat (int x, int y)
{
    int i;
    if (inbounds (x, y) && (random_range (3) || Player.status[ALERT])) {
	if (loc_statusp (x, y, SECRET)) {
	    lreset (x, y, SECRET);
	    lset (x, y, CHANGED);
	    if ((Level->site[x][y].locchar == OPEN_DOOR) || (Level->site[x][y].locchar == CLOSED_DOOR)) {
		mprint ("You find a secret door!");
		for (i = 0; i <= 8; i++) {
		    lset (x + Dirs[0][i], y + Dirs[1][i], STOPS);
		    lset (x + Dirs[0][i], y + Dirs[1][i], CHANGED);
		}
	    } else
		mprint ("You find a secret passage!");
	    drawvision (Player.x, Player.y);
	}
	if ((Level->site[x][y].p_locf >= TRAP_BASE) && (Level->site[x][y].locchar != TRAP) && (Level->site[x][y].p_locf <= TRAP_BASE + NUMTRAPS)) {
	    Level->site[x][y].locchar = TRAP;
	    lset (x, y, CHANGED);
	    mprint ("You find a trap!");
	    drawvision (Player.x, Player.y);
	    resetgamestatus (FAST_MOVE);
	}
    }
}

// This is to be called whenever anything might change player performance in
// melee, such as changing weapon, statistics, etc.
void calc_melee (void)
{
    calc_weight();

    Player.maxweight = (Player.str * Player.agi * 10);
    Player.absorption = Player.status[PROTECTION];
    Player.defense = 2 * statmod (Player.agi) + (Player.level / 2);
    Player.hit = Player.level + statmod (Player.dex) + 1;
    Player.dmg = statmod (Player.str) + 3;
    Player.speed = 5 - min (4, (statmod (Player.agi) / 2));
    if (Player.status[HASTED] > 0)
	Player.speed = Player.speed / 2;
    if (Player.status[SLOWED] > 0)
	Player.speed = Player.speed * 2;
    if (Player.itemweight > 0)
	switch (Player.maxweight / Player.itemweight) {
	    case 0:
		Player.speed += 6;
		break;
	    case 1:
		Player.speed += 3;
		break;
	    case 2:
		Player.speed += 2;
		break;
	    case 3:
		Player.speed += 1;
		break;
	}

    if (Player.status[ACCURATE])
	Player.hit += 20;
    if (Player.status[HERO])
	Player.hit += Player.dex;
    if (Player.status[HERO])
	Player.dmg += Player.str;
    if (Player.status[HERO])
	Player.defense += Player.agi;
    if (Player.status[HERO])
	Player.speed = Player.speed / 2;

    Player.speed = max (1, min (25, Player.speed));

    if (gamestatusp (MOUNTED)) {
	Player.speed = 3;
	Player.hit += 10;
	Player.dmg += 10;
    }

    // weapon
    // have to check for used since it could be a 2h weapon just carried in one hand
    if (Player.has_possession(O_WEAPON_HAND))
	if (Player.possessions[O_WEAPON_HAND].used && (Player.possessions[O_WEAPON_HAND].objchar == WEAPON || Player.possessions[O_WEAPON_HAND].objchar == MISSILEWEAPON)) {
	    Player.hit += Player.possessions[O_WEAPON_HAND].hit + Player.possessions[O_WEAPON_HAND].plus;
	    Player.dmg += Player.possessions[O_WEAPON_HAND].dmg + Player.possessions[O_WEAPON_HAND].plus;
	}

    // shield or defensive weapon
    if (Player.has_possession(O_SHIELD)) {
	Player.defense += Player.possessions[O_SHIELD].aux + Player.possessions[O_SHIELD].plus;
    }

    // armor
    if (Player.has_possession(O_ARMOR)) {
	Player.absorption += Player.possessions[O_ARMOR].dmg;
	Player.defense += Player.possessions[O_ARMOR].plus - Player.possessions[O_ARMOR].aux;
    }

    if (strlen (Player.meleestr) > 2U * maneuvers())
	default_maneuvers();
    comwinprint();
    showflags();
    dataprint();
}

// player attacks monster m
static void fight_monster (struct monster *m)
{
    int hitmod = 0;
    int reallyfight = TRUE;

    if (Player.status[AFRAID]) {
	print3 ("You are much too afraid to fight!");
	reallyfight = FALSE;
    } else if (player_on_sanctuary()) {
	print3 ("You restrain yourself from desecrating this holy place.");
	reallyfight = FALSE;
    } else if (Player.status[SHADOWFORM]) {
	print3 ("Your attack has no effect in your shadowy state.");
	reallyfight = FALSE;
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
	m_status_set (*m, AWAKE);
	m_status_set (*m, HOSTILE);
	m->attacked = TRUE;
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
		strcpy (Str1, "Your ");
		strcat (Str1, (o->blessing >= 0 ? o->truename : o->cursestr));
		strcat (Str1, " explodes!");
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
		strcpy (Str1, "Your ");
		strcat (Str1, itemid (o));
		strcat (Str1, " glows strongly.");
		print1 (Str1);
		return 0;
	    } else if ((o->blessing < -1) && (o->level > random_range (10))) {
		strcpy (Str1, "You hear an evil giggle from your ");
		strcat (Str1, itemid (o));
		print1 (Str1);
		return 0;
	    } else if (o->plus > 0) {
		strcpy (Str1, "Your ");
		strcat (Str1, itemid (o));
		strcat (Str1, " glows and then fades.");
		print1 (Str1);
		o->plus--;
		return 0;
	    } else {
		if (o->blessing > 0)
		    print1 ("You hear a faint despairing cry!");
		else if (o->blessing < 0)
		    print1 ("You hear an agonized scream!");
		strcpy (Str1, "Your ");
		strcat (Str1, itemid (o));
		strcat (Str1, " shatters in a thousand lost fragments!");
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
void p_damage (int dmg, int dtype, const char* fromstring)
{
    if (!p_immune (dtype)) {
	if (gamestatusp (FAST_MOVE)) {
	    drawvision (Player.x, Player.y);
	    resetgamestatus (FAST_MOVE);
	}
	if (dtype == NORMAL_DAMAGE)
	    Player.hp -= max (1, (dmg - Player.absorption));
	else
	    Player.hp -= dmg;
	if (Player.hp < 1)
	    p_death (fromstring);
    } else
	mprint ("You resist the effects!");
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

// move the cursor around, like for firing a wand, sets x and y to target
void setspot (int *x, int *y)
{
    char c = ' ';
    mprint ("Targeting.... ? for help");
    curs_set (1);
    omshowcursor (*x, *y);
    while ((c != '.') && (c != KEY_ESCAPE)) {
	c = lgetc();
	switch (c) {
	    case 'h':
	    case '4':
		movecursor (x, y, -1, 0);
		break;
	    case 'j':
	    case '2':
		movecursor (x, y, 0, 1);
		break;
	    case 'k':
	    case '8':
		movecursor (x, y, 0, -1);
		break;
	    case 'l':
	    case '6':
		movecursor (x, y, 1, 0);
		break;
	    case 'b':
	    case '1':
		movecursor (x, y, -1, 1);
		break;
	    case 'n':
	    case '3':
		movecursor (x, y, 1, 1);
		break;
	    case 'y':
	    case '7':
		movecursor (x, y, -1, -1);
		break;
	    case 'u':
	    case '9':
		movecursor (x, y, 1, -1);
		break;
	    case '?':
		clearmsg();
		mprint ("Use vi keys or numeric keypad to move cursor to target.");
		mprint ("Hit the '.' key when done, or ESCAPE to abort.");
		break;
	}
    }
    if (c == KEY_ESCAPE)
	*x = *y = ABORT;
    curs_set (0);
    screencheck (Player.y);
}

// get a direction: return index into Dirs array corresponding to direction
int getdir (void)
{
    while (1) {
	mprint ("Select direction [hjklyubn, ESCAPE to quit]: ");
	switch (mgetc()) {
	    case '4':
	    case 'h':
	    case 'H':
		return (5);
	    case '2':
	    case 'j':
	    case 'J':
		return (6);
	    case '8':
	    case 'k':
	    case 'K':
		return (7);
	    case '6':
	    case 'l':
	    case 'L':
		return (4);
	    case '7':
	    case 'y':
	    case 'Y':
		return (3);
	    case '9':
	    case 'u':
	    case 'U':
		return (1);
	    case '1':
	    case 'b':
	    case 'B':
		return (2);
	    case '3':
	    case 'n':
	    case 'N':
		return (0);
	    case KEY_ESCAPE:
		return (ABORT);
	    default:
		print3 ("That's not a direction! ");
	}
    }
}

// functions describes monster m's state for examine function
const char* mstatus_string (struct monster *m)
{
    if (m_statusp (m, M_INVISIBLE) && !Player.status[TRUESIGHT])
	strcpy (Str2, "Some invisible creature");
    else if (m->uniqueness == COMMON) {
	if (m->hp < Monsters[m->id].hp / 3)
	    strcpy (Str2, "a grievously injured ");
	else if (m->hp < Monsters[m->id].hp / 2)
	    strcpy (Str2, "a severely injured ");
	else if (m->hp < Monsters[m->id].hp)
	    strcpy (Str2, "an injured ");
	else
	    strcpy (Str2, getarticle (m->monstring));
	if (m->level > Monsters[m->id].level) {
	    strcat (Str2, " (level ");
	    strcat (Str2, wordnum (m->level + 1 - Monsters[m->id].level));
	    strcat (Str2, ") ");
	}
	strcat (Str2, m->monstring);
    } else {
	strcpy (Str2, m->monstring);
	if (m->hp < Monsters[m->id].hp / 3)
	    strcat (Str2, " who is grievously injured ");
	else if (m->hp < Monsters[m->id].hp / 2)
	    strcat (Str2, " who is severely injured ");
	else if (m->hp < Monsters[m->id].hp)
	    strcat (Str2, " who is injured ");
    }
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
    int i, count = 0, share;
    Player.xp += (long) amount;
    gain_level();		// actually, check to see if should gain level
    for (i = 0; i < NUMRANKS; i++)
	if (Player.guildxp[i] > 0)
	    count++;
    share = amount / (max (count, 1));
    for (i = 0; i < NUMRANKS; i++)
	if (Player.guildxp[i] > 0)
	    Player.guildxp[i] += share;
}

// try to hit a monster in an adjacent space. If there are none
// return FALSE. Note if you're berserk you get to attack ALL
// adjacent monsters!
int goberserk (void)
{
    int wentberserk = FALSE, i;
    char meleestr[80];
    strcpy (meleestr, Player.meleestr);
    strcpy (Player.meleestr, "lLlClH");
    for (i = 0; i < 8; i++) {
	monster* m = Level->creature(Player.x + Dirs[0][i], Player.y + Dirs[1][i]);
	if (m) {
	    wentberserk = TRUE;
	    fight_monster(m);
	    morewait();
	}
    }
    strcpy (Player.meleestr, meleestr);
    return (wentberserk);
}

// identifies a trap for examine() by its aux value
const char* trapid (int trapno)
{
    switch (trapno) {
	case L_TRAP_SIREN:
	    return ("A siren trap");
	case L_TRAP_DART:
	    return ("A dart trap");
	case L_TRAP_PIT:
	    return ("A pit");
	case L_TRAP_SNARE:
	    return ("A snare");
	case L_TRAP_BLADE:
	    return ("A blade trap");
	case L_TRAP_FIRE:
	    return ("A fire trap");
	case L_TRAP_TELEPORT:
	    return ("A teleport trap");
	case L_TRAP_DISINTEGRATE:
	    return ("A disintegration trap");
	case L_TRAP_DOOR:
	    return ("A trap door");
	case L_TRAP_MANADRAIN:
	    return ("A manadrain trap");
	case L_TRAP_ACID:
	    return ("An acid shower trap");
	case L_TRAP_SLEEP_GAS:
	    return ("A sleep gas trap");
	case L_TRAP_ABYSS:
	    return ("A concealed entrance to the abyss");
	default:
	    return ("A completely inoperative trap.");
    }
}

// checks current food status of player, every hour, and when food is eaten
void foodcheck (void)
{
    if (Player.food > 48) {
	print3 ("You vomit up your huge meal.");
	Player.food = 12;
    } else if (Player.food == 30)
	print3 ("Time for a smackerel of something.");
    else if (Player.food == 20)
	print3 ("You feel hungry.");
    else if (Player.food == 12)
	print3 ("You are ravenously hungry.");
    else if (Player.food == 3) {
	print3 ("You feel weak.");
	if (gamestatusp (FAST_MOVE)) {
	    drawvision (Player.x, Player.y);
	    resetgamestatus (FAST_MOVE);
	}
    } else if (Player.food < 0) {
	if (gamestatusp (FAST_MOVE)) {
	    drawvision (Player.x, Player.y);
	    resetgamestatus (FAST_MOVE);
	}
	print3 ("You're starving!");
	p_damage (-5 * Player.food, UNSTOPPABLE, "starvation");
    }
    showflags();
}

// see whether room should be illuminated
void roomcheck (void)
{
    static int oldroomno = -1;
    static plv oldlevel = NULL;
    int roomno = Level->site[Player.x][Player.y].roomnumber;

    if ((roomno == RS_CAVERN) || (roomno == RS_SEWER_DUCT) || (roomno == RS_KITCHEN) || (roomno == RS_BATHROOM) || (roomno == RS_BEDROOM) || (roomno == RS_DININGROOM) || (roomno == RS_CLOSET) || (roomno > RS_ROOMBASE)) {
	if ((!loc_statusp (Player.x, Player.y, LIT)) && (!Player.status[BLINDED]) && (Player.status[ILLUMINATION] || (difficulty() < 6))) {
	    showroom (Level->site[Player.x][Player.y].roomnumber);
	    spreadroomlight (Player.x, Player.y, roomno);
	    levelrefresh();
	}
    }
    if ((oldroomno != roomno) || (oldlevel != Level)) {
	showroom (roomno);
	oldroomno = roomno;
	oldlevel = Level;
    }
}

// ask for mercy
void surrender (struct monster *m)
{
    int i;
    long bestitem, bestvalue;

    switch (random_range (4)) {
	case 0:
	    print1 ("You grovel at the monster's feet...");
	    break;
	case 1:
	    print1 ("You cry 'uncle'!");
	    break;
	case 2:
	    print1 ("You beg for mercy.");
	    break;
	case 3:
	    print1 ("You yield to the monster.");
	    break;
    }
    if (m->id == GUARD) {
	if (m_statusp (m, HOSTILE))
	    monster_talk (m);
	else {
	    print2 ("The guard (bored): Have you broken a law? [yn] ");
	    if (ynq2() == 'y') {
		print2 ("The guard grabs you, and drags you to court.");
		morewait();
		send_to_jail();
	    } else
		print2 ("Then don't bother me. Scat!");
	}
    } else if ((m->talkf == M_NO_OP) || (m->talkf == M_TALK_STUPID))
	print3 ("Your plea is ignored.");
    else {
	morewait();
	print1 ("Your surrender is accepted.");
	if (Player.cash > 0)
	    nprint1 (" All your gold is taken....");
	Player.cash = 0;
	bestvalue = 0;
	bestitem = ABORT;
	for (i = 1; i < MAXITEMS; i++) {
	    if (Player.has_possession(i)) {
		if (bestvalue < true_item_value (Player.possessions[i])) {
		    bestitem = i;
		    bestvalue = true_item_value (Player.possessions[i]);
		}
	    }
	}
	if (bestitem != ABORT) {
	    print2 ("You also give away your best item... ");
	    nprint2 (itemid (Player.possessions[bestitem]));
	    nprint2 (".");
	    morewait();
	    givemonster (*m, Player.possessions[bestitem]);
	    morewait();	// msgs come from givemonster
	    Player.remove_possession (bestitem);
	}
	print2 ("You feel less experienced... ");
	Player.xp = max (0U, Player.xp - m->xpv);
	nprint2 ("The monster seems more experienced!");
	m->level = (min (10, m->level + 1));
	m->hp += m->level * 20;
	m->hit += m->level;
	m->dmg += m->level;
	m->ac += m->level;
	m->xpv += m->level * 10;
	morewait();
	clearmsg();
	if ((m->talkf == M_TALK_EVIL) && random_range (10)) {
	    print1 ("It continues to attack you, laughing evilly!");
	    m_status_set (m, HOSTILE);
	    m_status_reset (m, GREEDY);
	} else if (m->id == HORNET || m->id == GUARD)
	    print1 ("It continues to attack you. ");
	else {
	    print1 ("The monster leaves, chuckling to itself....");
	    m_teleport (m);
	}
    }
    dataprint();
}

// threaten a monster
void threaten (struct monster *m)
{
    char response;
    switch (random_range (4)) {
	case 0:
	    mprint ("You demand that your opponent surrender!");
	    break;
	case 1:
	    mprint ("You threaten to do bodily harm to it.");
	    break;
	case 2:
	    mprint ("You attempt to bluster it into submission.");
	    break;
	case 3:
	    mprint ("You try to cow it with your awesome presence.");
	    break;
    }
    morewait();
    if (!m_statusp (m, HOSTILE)) {
	print3 ("You only annoy it with your futile demand.");
	m_status_set (m, HOSTILE);
    } else if (((m->level * 2 > Player.level) && (m->hp > Player.dmg)) || (m->uniqueness != COMMON))
	print1 ("It sneers contemptuously at you.");
    else if ((m->talkf != M_TALK_GREEDY) && (m->talkf != M_TALK_HUNGRY) && (m->talkf != M_TALK_EVIL) && (m->talkf != M_TALK_MAN) && (m->talkf != M_TALK_BEG) && (m->talkf != M_TALK_THIEF) && (m->talkf != M_TALK_MERCHANT) && (m->talkf != M_TALK_IM))
	print1 ("Your demand is ignored");
    else {
	print1 ("It yields to your mercy.");
	Player.alignment += 3;
	print2 ("Kill it, rob it, or free it? [krf] ");
	do
	    response = (char) mcigetc();
	while ((response != 'k') && (response != 'r') && (response != 'f'));
	if (response == 'k') {
	    m_death (m);
	    print2 ("You treacherous rogue!");
	    Player.alignment -= 13;
	} else if (response == 'r') {
	    Player.alignment -= 2;
	    print2 ("It drops its treasure and flees.");
	    m_dropstuff (m);
	    Level->mlist.erase (m);
	    putspot (m->x, m->y, getspot (m->x, m->y, FALSE));
	} else {
	    Player.alignment += 2;
	    print2 ("'If you love something set it free ... '");
	    if (random_range (100) == 13) {
		morewait();
		print2 ("'...If it doesn't come back, hunt it down and kill it.'");
	    }
	    print3 ("It departs with a renewed sense of its own mortality.");
	    Level->mlist.erase (m);
	    putspot (m->x, m->y, getspot (m->x, m->y, FALSE));
	}
    }
}

// name of the player's experience level
const char* levelname (int level)
{
    switch (level) {
	case 0:
	    strcpy (Str3, "neophyte");
	    break;
	case 1:
	    strcpy (Str3, "beginner");
	    break;
	case 2:
	    strcpy (Str3, "tourist");
	    break;
	case 3:
	    strcpy (Str3, "traveller");
	    break;
	case 4:
	    strcpy (Str3, "wayfarer");
	    break;
	case 5:
	    strcpy (Str3, "peregrinator");
	    break;
	case 6:
	    strcpy (Str3, "wanderer");
	    break;
	case 7:
	    strcpy (Str3, "hunter");
	    break;
	case 8:
	    strcpy (Str3, "scout");
	    break;
	case 9:
	    strcpy (Str3, "trailblazer");
	    break;
	case 10:
	    strcpy (Str3, "discoverer");
	    break;
	case 11:
	    strcpy (Str3, "explorer");
	    break;
	case 12:
	    strcpy (Str3, "senior explorer");
	    break;
	case 13:
	    strcpy (Str3, "ranger");
	    break;
	case 14:
	    strcpy (Str3, "ranger captain");
	    break;
	case 15:
	    strcpy (Str3, "ranger knight");
	    break;
	case 16:
	    strcpy (Str3, "adventurer");
	    break;
	case 17:
	    strcpy (Str3, "experienced adventurer");
	    break;
	case 18:
	    strcpy (Str3, "skilled adventurer");
	    break;
	case 19:
	    strcpy (Str3, "master adventurer");
	    break;
	case 20:
	    strcpy (Str3, "hero");
	    break;
	case 21:
	    strcpy (Str3, "superhero");
	    break;
	case 22:
	    strcpy (Str3, "demigod");
	    break;
	default:
	    if (level < 100) {
		strcpy (Str3, "Order ");
		Str3[6] = ((level / 10) - 2) + '0';
		Str3[7] = 0;
		strcat (Str3, " Master of Omega");
	    } else
		strcpy (Str3, "Ultimate Master of Omega");
	    break;
    }
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
    int dmult;

    // chance for critical hit..., 3/10
    switch (random_range (10)) {
	case 0:
	    if (random_range (100) < Player.level) {
		strcpy (Str3, "You annihilate ");
		dmult = 1000;
	    } else {
		strcpy (Str3, "You blast ");
		dmult = 5;
	    }
	    break;
	case 1:
	case 2:
	    strcpy (Str3, "You smash ");
	    dmult = 2;
	    break;

	default:
	    dmult = 1;
	    if (random_range (10))
		strcpy (Str3, "You hit ");
	    else
		switch (random_range (4)) {
		    case 0:
			strcpy (Str3, "You damage ");
			break;
		    case 1:
			strcpy (Str3, "You inflict bodily harm on ");
			break;
		    case 2:
			strcpy (Str3, "You injure ");
			break;
		    case 3:
			strcpy (Str3, "You molest ");
			break;
		}
	    break;
    }
    if (Lunarity == 1)
	dmult = dmult * 2;
    else if (Lunarity == -1)
	dmult = dmult / 2;
    if (m->uniqueness == COMMON)
	strcat (Str3, "the ");
    strcat (Str3, m->monstring);
    strcat (Str3, ". ");
    if (Verbosity != TERSE)
	mprint (Str3);
    else
	mprint ("You hit it.");
    m_damage (m, dmult * random_range (dmg), dtype);
    if ((Verbosity != TERSE) && (random_range (10) == 3) && (m->hp > 0))
	mprint ("It laughs at the injury and fights on!");
}

// and effects of missing
static void player_miss (struct monster *m, int dtype)
{
    if (random_range (30) == 1)	// fumble 1 in 30
	p_fumble (dtype);
    else {
	if (Verbosity != TERSE) {
	    if (random_range (10))
		strcpy (Str3, "You miss ");
	    else
		switch (random_range (4)) {
		    case 0:
			strcpy (Str3, "You flail lamely at ");
			break;
		    case 1:
			strcpy (Str3, "You only amuse ");
			break;
		    case 2:
			strcpy (Str3, "You fail to even come close to ");
			break;
		    case 3:
			strcpy (Str3, "You totally avoid contact with ");
			break;
		}
	    if (m->uniqueness == COMMON)
		strcat (Str3, "the ");
	    strcat (Str3, m->monstring);
	    strcat (Str3, ". ");
	    mprint (Str3);
	} else
	    mprint ("You missed it.");
    }
}

// oh nooooo, a fumble....
static void p_fumble (int dtype)
{
    mprint ("Ooops! You fumbled....");
    switch (random_range (10)) {
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
    if (Player.has_possession(O_WEAPON_HAND)) {
	strcpy (Str1, "You dropped your ");
	strcat (Str1, Player.possessions[O_WEAPON_HAND].objstr);
	mprint (Str1);
	morewait();
	p_drop_at (Player.x, Player.y, Player.possessions[O_WEAPON_HAND], 1);
	Player.remove_possession (O_WEAPON_HAND, 1);
    } else
	mprint ("You feel fortunate.");
}

// try to break a weapon (from fumbling)
static void break_weapon (void)
{
    if (Player.has_possession(O_WEAPON_HAND)) {
	strcpy (Str1, "Your ");
	strcat (Str1, itemid (Player.possessions[O_WEAPON_HAND]));
	strcat (Str1, " vibrates in your hand....");
	mprint (Str1);
	damage_item (&Player.possessions[O_WEAPON_HAND]);
	morewait();
    }
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

// is Player immune to damage type dtype
int p_immune (int dtype)
{
    return (Player.immunity[dtype] > 0);
}

// deal with each possible stati -- values are per move
// this function is executed every move
// A value over 1000 indicates a permanent effect
void minute_status_check (void)
{
    int i;

    if (Player.status[HASTED] > 0) {
	if (Player.status[HASTED] < 1000) {
	    Player.status[HASTED]--;
	    if (Player.status[HASTED] == 0) {
		mprint ("The world speeds up.");
		calc_melee();
	    }
	}
    }

    if (Player.status[POISONED] > 0) {
	Player.status[POISONED]--;
	p_damage (3, POISON, "poison");
	if (Player.status[POISONED] == 0) {
	    showflags();
	    mprint ("You feel better now.");
	}
    }

    if (Player.immunity[UNSTOPPABLE] > 0) {
	for (i = 0; i < NUMIMMUNITIES; i++)
	    Player.immunity[i]--;
	if (Player.immunity[UNSTOPPABLE] == 1)
	    mprint ("You feel vincible again.");
    }

    if (Player.status[IMMOBILE] > 0) {
	Player.status[IMMOBILE]--;
	if (Player.status[IMMOBILE] == 0)
	    mprint ("You can move again.");
    }

    if (Player.status[SLEPT] > 0) {
	Player.status[SLEPT]--;
	if (Player.status[SLEPT] == 0) {
	    mprint ("You woke up.");
	}
    }

    if (Player.status[REGENERATING] > 0) {
	if ((Player.hp < Player.maxhp) && (Player.mana > 0)) {
	    Player.hp++;
	    Player.mana--;
	    dataprint();
	}
	if (Player.status[REGENERATING] < 1000) {
	    Player.status[REGENERATING]--;
	    if (Player.status[REGENERATING] == 0) {
		mprint ("You feel less homeostatic.");
	    }
	}
    }

    if (Player.status[SLOWED] > 0) {
	if (Player.status[SLOWED] < 1000) {
	    Player.status[SLOWED]--;
	    if (Player.status[SLOWED] == 0) {
		mprint ("You feel quicker now.");
		calc_melee();
	    }
	}
    }

    if (Player.status[RETURNING] > 0) {
	Player.status[RETURNING]--;
	if (Player.status[RETURNING] == 10)
	    mprint ("Your return spell slowly hums towards activation...");
	else if (Player.status[RETURNING] == 8)
	    mprint ("There is an electric tension in the air!");
	else if (Player.status[RETURNING] == 5)
	    mprint ("A vortex of mana begins to form around you!");
	else if (Player.status[RETURNING] == 1)
	    mprint ("Your surroundings start to warp and fade!");
	if (Player.status[RETURNING] == 0)
	    level_return();
    }

    if (Player.status[AFRAID] > 0) {
	if (Player.status[AFRAID] < 1000) {
	    Player.status[AFRAID]--;
	    if (Player.status[AFRAID] == 0) {
		mprint ("You feel bolder now.");
	    }
	}
    }

}

// effect of gamma ray radiation...
void moon_check (void)
{
    // 24 day lunar cycle
    Phase = (Phase + 1) % 24;
    phaseprint();
    Lunarity = 0;
    if (((Player.patron == DRUID) && ((Phase / 2 == 3) || (Phase / 2 == 9))) || ((Player.alignment > 10) && (Phase / 2 == 6)) || ((Player.alignment < -10) && (Phase / 2 == 0))) {
	mprint ("As the moon rises you feel unusually vital!");
	Lunarity = 1;
    } else if (((Player.patron == DRUID) && ((Phase / 2 == 0) || (Phase / 2 == 6))) || ((Player.alignment > 10) && (Phase / 2 == 0)) || ((Player.alignment < -10) && (Phase / 2 == 6))) {
	mprint ("The rise of the moon tokens a strange enervation!");
	Lunarity = -1;
    }

}

// check 1/hour for torch to burn out if used
void torch_check (void)
{
    for (int i = O_READY_HAND; i <= O_WEAPON_HAND; i++) {
	if (Player.possessions[i].id == THING_TORCH && Player.possessions[i].aux > 0) {
	    if (--Player.possessions[i].aux == 0) {
		mprint ("Your torch goes out!!!");
		Player.remove_possession (i, 1);
	    }
	}
    }
}

// values are in multiples of ten minutes
// values over 1000 indicate a permanent effect
void tenminute_status_check (void)
{
    if ((Player.status[SHADOWFORM] > 0) && (Player.status[SHADOWFORM] < 1000)) {
	Player.status[SHADOWFORM]--;
	if (Player.status[SHADOWFORM] == 0) {
	    Player.immunity[NORMAL_DAMAGE]--;
	    Player.immunity[ACID]--;
	    Player.immunity[THEFT]--;
	    Player.immunity[INFECTION]--;
	    mprint ("You feel less shadowy now.");
	}
    }

    if ((Player.status[ILLUMINATION] > 0) && (Player.status[ILLUMINATION] < 1000)) {
	Player.status[ILLUMINATION]--;
	if (Player.status[ILLUMINATION] == 0) {
	    mprint ("Your light goes out!");
	}
    }

    if ((Player.status[VULNERABLE] > 0) && (Player.status[VULNERABLE] < 1000)) {
	Player.status[VULNERABLE]--;
	if (Player.status[VULNERABLE] == 0)
	    mprint ("You feel less endangered.");
    }

    if ((Player.status[DEFLECTION] > 0) && (Player.status[DEFLECTION] < 1000)) {
	Player.status[DEFLECTION]--;
	if (Player.status[DEFLECTION] == 0)
	    mprint ("You feel less well defended.");
    }

    if ((Player.status[ACCURATE] > 0) && (Player.status[ACCURACY] < 1000)) {
	Player.status[ACCURATE]--;
	if (Player.status[ACCURATE] == 0) {
	    calc_melee();
	    mprint ("The bulls' eyes go away.");
	}
    }
    if ((Player.status[HERO] > 0) && (Player.status[HERO] < 1000)) {
	Player.status[HERO]--;
	if (Player.status[HERO] == 0) {
	    calc_melee();
	    mprint ("You feel less than super.");
	}
    }

    if ((Player.status[LEVITATING] > 0) && (Player.status[LEVITATING] < 1000)) {
	Player.status[LEVITATING]--;
	if (Player.status[LEVITATING] == 0)
	    mprint ("You're no longer walking on air.");
    }

    if (Player.status[DISEASED] > 0) {
	Player.status[DISEASED]--;
	if (Player.status[DISEASED] == 0) {
	    showflags();
	    mprint ("You feel better now.");
	}
    }

    if ((Player.status[INVISIBLE] > 0) && (Player.status[INVISIBLE] < 1000)) {
	Player.status[INVISIBLE]--;
	if (Player.status[INVISIBLE] == 0)
	    mprint ("You feel more opaque now.");
    }

    if ((Player.status[BLINDED] > 0) && (Player.status[BLINDED] < 1000)) {
	Player.status[BLINDED]--;
	if (Player.status[BLINDED] == 0)
	    mprint ("You can see again.");
    }

    if ((Player.status[TRUESIGHT] > 0) && (Player.status[TRUESIGHT] < 1000)) {
	Player.status[TRUESIGHT]--;
	if (Player.status[TRUESIGHT] == 0)
	    mprint ("You feel less keen now.");
    }

    if ((Player.status[BERSERK] > 0) && (Player.status[BERSERK] < 1000)) {
	Player.status[BERSERK]--;
	if (Player.status[BERSERK] == 0)
	    mprint ("You stop foaming at the mouth.");
    }

    if ((Player.status[ALERT] > 0) && (Player.status[ALERT] < 1000)) {
	Player.status[ALERT]--;
	if (Player.status[ALERT] == 0)
	    mprint ("You feel less alert now.");
    }

    if ((Player.status[BREATHING] > 0) && (Player.status[BREATHING] < 1000)) {
	Player.status[BREATHING]--;
	if (Player.status[BREATHING] == 0)
	    mprint ("You feel somewhat congested.");
    }

    if ((Player.status[DISPLACED] > 0) && (Player.status[DISPLACED] < 1000)) {
	Player.status[DISPLACED]--;
	if (Player.status[DISPLACED] == 0)
	    mprint ("You feel a sense of position.");
    }
    timeprint();
    dataprint();
}

// Increase in level at appropriate experience gain
static void gain_level (void)
{
    int gained = FALSE;
    int hp_gain;

    if (gamestatusp (SUPPRESS_PRINTING))
	return;
    while (expval (Player.level + 1) <= Player.xp) {
	if (!gained)
	    morewait();
	gained = TRUE;
	Player.level++;
	print1 ("You have attained a new experience level!");
	print2 ("You are now ");
	nprint2 (getarticle (levelname (Player.level)));
	nprint2 (levelname (Player.level));
	hp_gain = random_range (Player.con) + 1;	// start fix 12/30/98
	if (Player.hp < Player.maxhp)
	    Player.hp += hp_gain * Player.hp / Player.maxhp;
	else if (Player.hp < Player.maxhp + hp_gain)
	    Player.hp = Player.maxhp + hp_gain;
	// else leave current hp alone
	Player.maxhp += hp_gain;
	Player.maxmana = calcmana();
	// If the character was given a bonus, let him keep it.  Otherwise
	// recharge him.
	Player.mana = max (Player.mana, Player.maxmana);	// end fix 12/30/98
	morewait();
    }
    if (gained)
	clearmsg();
    calc_melee();
}

// experience requirements
static long expval (int plevel)
{
    switch (plevel) {
	case 0:
	    return (0L);
	case 1:
	    return (20L);
	case 2:
	    return (50L);
	case 3:
	    return (200L);
	case 4:
	    return (500L);
	case 5:
	    return (1000L);
	case 6:
	    return (2000L);
	case 7:
	    return (3000L);
	case 8:
	    return (5000L);
	case 9:
	    return (7000L);
	case 10:
	    return (10000L);
	default:
	    return ((plevel - 9) * 10000L);
    }
}

// If an item is unidentified, it isn't worth much to those who would buy it
long item_value (const object* item)
{
    if (!object_is_known(item)) {
	if (item->objchar == THING)
	    return (1);
	else
	    return (true_item_value (item) / 10);
    } else
	return (true_item_value (item));
}

// figures value based on item base-value, charge, plus, and blessing
long true_item_value (const object* item)
{
    long value = item->basevalue;
    if (item->objchar == THING)
	return (item->basevalue);
    else {
	if (item->objchar == STICK)
	    value += value * item->charge / 20;
	if (item->plus > -1)
	    value += value * item->plus / 4;
	else
	    value /= -item->plus;
	if (item->blessing > 0)
	    value *= 2;
	return ((long) value);
    }
}

// kill off player if he isn't got the "breathing" status
void p_drown (void)
{
    if (Player.status[BREATHING] > 0) {
	mprint ("Your breathing is unaffected!");
	return;
    }
    for (int attempts = 3; Player.has_possession(O_ARMOR) || Player.itemweight > Player.maxweight / 2; --attempts) {
	menuclear();
	switch (attempts) {
	    case 3: print3 ("You try to hold your breath..."); break;
	    case 2: print3 ("You try to hold your breath... You choke..."); break;
	    case 1: print3 ("You try to hold your breath... You choke... Your lungs fill..."); break;
	    case 0: p_death ("drowning");
	}
	morewait();
	menuprint ("a: Drop an item.\n");
	menuprint ("b: Bash an item.\n");
	menuprint ("c: Drop your whole pack.\n");
	showmenu();
	switch (menugetc()) {
	    case 'a':
		drop();
		if (Level->site[Player.x][Player.y].p_locf == L_WATER && Level->thing(Player.x,Player.y)) {
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
		    if (Level->site[Player.x][Player.y].p_locf != L_WATER)
			p_drop_at (Player.x, Player.y, *i, i->number);
		Player.pack.clear();
		if (Level->site[Player.x][Player.y].p_locf == L_WATER)
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
    int aux = (weapon == NULL ? -2 : weapon->aux);	// bare hands
    switch (aux) {
	case -2:
	    weapon_bare_hands (dmgmod, m);
	    break;
	default:
	case I_NO_OP:
	    weapon_normal_hit (dmgmod, weapon, m);
	    break;
	case I_ACIDWHIP:
	    weapon_acidwhip (dmgmod, weapon, m);
	    break;
	case I_TANGLE:
	    weapon_tangle (dmgmod, weapon, m);
	    break;
	case I_ARROW:
	    weapon_arrow (dmgmod, weapon, m);
	    break;
	case I_BOLT:
	    weapon_bolt (dmgmod, weapon, m);
	    break;
	case I_DEMONBLADE:
	    weapon_demonblade (dmgmod, weapon, m);
	    break;
	case I_LIGHTSABRE:
	    weapon_lightsabre (dmgmod, weapon, m);
	    break;
	case I_MACE_DISRUPT:
	    weapon_mace_disrupt (dmgmod, weapon, m);
	    break;
	case I_VORPAL:
	    weapon_vorpal (dmgmod, weapon, m);
	    break;
	case I_DESECRATE:
	    weapon_desecrate (dmgmod, weapon, m);
	    break;
	case I_FIRESTAR:
	    weapon_firestar (dmgmod, weapon, m);
	    break;
	case I_DEFEND:
	    weapon_defend (dmgmod, weapon, m);
	    break;
	case I_VICTRIX:
	    weapon_victrix (dmgmod, weapon, m);
	    break;
	case I_SCYTHE:
	    weapon_scythe (dmgmod, weapon, m);
	    break;
    }
}

// for printing actions in printactions above
const char* actionlocstr (int dir)
{
    switch (dir) {
	case 'L':
	    strcpy (Str3, "low.");
	    break;
	case 'C':
	    strcpy (Str3, "center.");
	    break;
	case 'H':
	    strcpy (Str3, "high.");
	    break;
	default:
	    strcpy (Str3, "wildly.");
	    break;
    }
    return (Str3);
}

// execute player combat actions versus monster m
static void tacplayer (struct monster *m)
{
    for (unsigned i = 0; i < strlen (Player.meleestr); i += 2) {
	if (m->hp > 0) {
	    switch (Player.meleestr[i]) {
		case 't':
		case 'T':
		    if (!Player.has_possession(O_WEAPON_HAND))
			strcpy (Str1, "You punch ");
		    else
			strcpy (Str1, "You thrust ");
		    strcat (Str1, actionlocstr (Player.meleestr[i + 1]));
		    if (Verbosity == VERBOSE)
			mprint (Str1);
		    if (player_hit (2 * statmod (Player.dex), Player.meleestr[i + 1], m))
			weapon_use (0, &Player.possessions[O_WEAPON_HAND], m);
		    else
			player_miss (m, NORMAL_DAMAGE);
		    break;
		case 'c':
		case 'C':
		    if (!Player.has_possession(O_WEAPON_HAND))
			strcpy (Str1, "You punch ");
		    else if (Player.possessions[O_WEAPON_HAND].type == CUTTING)
			strcpy (Str1, "You cut ");
		    else if (Player.possessions[O_WEAPON_HAND].type == STRIKING)
			strcpy (Str1, "You strike ");
		    else
			strcpy (Str1, "You attack ");
		    strcat (Str1, actionlocstr (Player.meleestr[i + 1]));
		    if (Verbosity == VERBOSE)
			mprint (Str1);
		    if (player_hit (0, Player.meleestr[i + 1], m))
			weapon_use (2 * statmod (Player.str), &Player.possessions[O_WEAPON_HAND], m);
		    else
			player_miss (m, NORMAL_DAMAGE);
		    break;
		case 'l':
		case 'L':
		    strcpy (Str1, "You lunge ");
		    strcat (Str1, actionlocstr (Player.meleestr[i + 1]));
		    if (Verbosity == VERBOSE)
			mprint (Str1);
		    if (player_hit (Player.level + Player.dex, Player.meleestr[i + 1], m))
			weapon_use (Player.level, &Player.possessions[O_WEAPON_HAND], m);
		    else
			player_miss (m, NORMAL_DAMAGE);
		    break;
	    }
	}
    }
}

// checks to see if player hits with hitmod vs. monster m at location hitloc
static int player_hit (int hitmod, int hitloc, struct monster *m)
{
    int blocks = FALSE, goodblocks = 0, hit;
    if (m->hp < 1) {
	mprint ("Unfortunately, your opponent is already dead!");
	return (FALSE);
    } else {
	if (hitloc == 'X')
	    hitloc = random_loc();

	transcribe_monster_actions (m);

	for (unsigned i = 0; i < strlen (m->meleestr); i += 2) {
	    if ((m->meleestr[i] == 'B') || (m->meleestr[i] == 'R')) {
		blocks = TRUE;
		if (hitloc == m->meleestr[i + 1])
		    goodblocks++;
	    }
	}

	if (!blocks)
	    goodblocks = -1;
	hit = hitp (Player.hit + hitmod, m->ac + goodblocks * 10);
	if ((!hit) && (goodblocks > 0)) {
	    if (m->uniqueness == COMMON) {
		strcpy (Str1, "The ");
		strcat (Str1, m->monstring);
	    } else
		strcpy (Str1, m->monstring);
	    strcat (Str1, " blocks it!");
	    if (Verbosity == VERBOSE)
		mprint (Str1);
	}
	return (hit);
    }
}

// This function is used to undo all items temporarily, should
// always be used in pairs with on being TRUE and FALSE, and may cause
// anomalous stats and item-usage if used indiscriminately
void toggle_item_use (int on)
{
    static bool used[MAXITEMS];
    setgamestatus (SUPPRESS_PRINTING);
    if (on) {
	for (int i = 0; i < MAXITEMS; i++) {
	    used[i] = false;
	    if (Player.has_possession(i)) {
		if ((used[i] = Player.possessions[i].used) == true) {
		    Player.possessions[i].used = false;
		    item_use (Player.possessions[i]);
		}
	    }
	}
    } else {
	for (int i = 0; i < MAXITEMS; i++) {
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

void enter_site (int site)
{
    switch (site) {
	case CITY:
	    change_environment (E_CITY);
	    break;
	case VILLAGE:
	    change_environment (E_VILLAGE);
	    break;
	case CAVES:
	    change_environment (E_CAVES);
	    break;
	case CASTLE:
	    change_environment (E_CASTLE);
	    break;
	case VOLCANO:
	    change_environment (E_VOLCANO);
	    break;
	case TEMPLE:
	    change_environment (E_TEMPLE);
	    break;
	case DRAGONLAIR:
	    change_environment (E_DLAIR);
	    break;
	case STARPEAK:
	    change_environment (E_STARPEAK);
	    break;
	case MAGIC_ISLE:
	    change_environment (E_MAGIC_ISLE);
	    break;
	default:
	    print3 ("There's nothing to enter here!");
	    break;
    }
}

// Switches context dungeon/countryside/city, etc
void change_environment (int new_environment)
{
    int i, emerging = FALSE;

    Player.sx = -1;
    Player.sy = -1;		// reset sanctuary if there was one
    resetgamestatus (LOST);	// in case the player gets lost _on_ a site

    resetgamestatus (FAST_MOVE);

    Last_Environment = Current_Environment;
    if (Last_Environment == E_COUNTRYSIDE) {
	LastCountryLocX = Player.x;
	LastCountryLocY = Player.y;
    }
    if (((Last_Environment == E_CITY) || (Last_Environment == E_VILLAGE)) && ((new_environment == E_MANSION) || (new_environment == E_HOUSE) || (new_environment == E_HOVEL) || (new_environment == E_SEWERS) || (new_environment == E_ARENA))) {
	LastTownLocX = Player.x;
	LastTownLocY = Player.y;
    } else if (((Last_Environment == E_MANSION) || (Last_Environment == E_HOUSE) || (Last_Environment == E_HOVEL) || (Last_Environment == E_SEWERS) || (Last_Environment == E_ARENA)) && ((new_environment == E_CITY) || (new_environment == E_VILLAGE))) {
	Player.x = LastTownLocX;
	Player.y = LastTownLocY;
	emerging = TRUE;
    }

    Current_Environment = new_environment;
    switch (new_environment) {
	case E_ARENA:
	    LENGTH = 16;
	    WIDTH = 64;
	    Player.x = 5;
	    Player.y = 7;
	    setgamestatus (ARENA_MODE);
	    load_arena();
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_ABYSS:
	    LENGTH = 16;
	    WIDTH = 64;
	    Player.x = 32;
	    Player.y = 15;
	    load_abyss();
	    abyss_file();
	    lose_all_items();
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_CIRCLE:
	    LENGTH = 16;
	    WIDTH = 64;
	    Player.x = 32;
	    Player.y = 14;
	    load_circle (TRUE);
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
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_COURT:
	    WIDTH = 64;
	    LENGTH = 24;
	    Player.x = 32;
	    Player.y = 2;
	    LastCountryLocX = 6;
	    LastCountryLocY = 1;
	    load_court (TRUE);
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_MANSION:
	    WIDTH = 64;
	    LENGTH = 16;
	    load_house (E_MANSION, TRUE);
	    Player.y = 8;
	    Player.x = 2;
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_HOUSE:
	    WIDTH = 64;
	    LENGTH = 16;
	    load_house (E_HOUSE, TRUE);
	    Player.y = 13;
	    Player.x = 2;
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_HOVEL:
	    WIDTH = 64;
	    LENGTH = 16;
	    load_house (E_HOVEL, TRUE);
	    Player.y = 9;
	    Player.x = 2;
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_DLAIR:
	    WIDTH = 64;
	    LENGTH = 16;
	    Player.y = 9;
	    Player.x = 2;
	    load_dlair (gamestatusp (KILLED_DRAGONLORD), TRUE);
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_STARPEAK:
	    WIDTH = 64;
	    LENGTH = 16;
	    Player.y = 9;
	    Player.x = 2;
	    load_speak (gamestatusp (KILLED_LAWBRINGER), TRUE);
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_MAGIC_ISLE:
	    WIDTH = 64;
	    LENGTH = 16;
	    Player.y = 14;
	    Player.x = 62;
	    load_misle (gamestatusp (KILLED_EATER), TRUE);
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_TEMPLE:
	    WIDTH = 64;
	    LENGTH = 16;
	    load_temple (Country[Player.x][Player.y].aux, TRUE);
	    Player.y = 15;
	    Player.x = 32;
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_CITY:
	    WIDTH = 64;
	    LENGTH = 64;
	    if (emerging) {
		print1 ("You emerge onto the street.");
		emerging = FALSE;
	    } else {
		print1 ("You pass through the massive gates of Rampart, the city.");
		Player.x = 62;
		Player.y = 21;
	    }
	    if (City == NULL)
		load_city (TRUE);
	    Level = City;
	    ScreenOffset = Player.y - (ScreenLength / 2);
	    show_screen();
	    break;
	case E_VILLAGE:
	    WIDTH = 64;
	    LENGTH = 16;
	    if (!emerging) {
		// different villages per different locations
		switch (Country[Player.x][Player.y].aux) {
		    case 1:
			Player.x = 0;
			Player.y = 6;
			Villagenum = 1;
			break;
		    default:
			print3 ("Very strange, a nonexistent village.");
		    case 2:
			Player.x = 39;
			Player.y = 15;
			Villagenum = 2;
			break;
		    case 3:
			Player.x = 63;
			Player.y = 8;
			Villagenum = 3;
			break;
		    case 4:
			Player.x = 32;
			Player.y = 15;
			Villagenum = 4;
			break;
		    case 5:
			Player.x = 2;
			Player.y = 8;
			Villagenum = 5;
			break;
		    case 6:
			Player.x = 2;
			Player.y = 2;
			Villagenum = 6;
			break;
		}
	    }
	    if ((!emerging) || (TempLevel == NULL))
		load_village (Villagenum, TRUE);
	    else if (TempLevel->environment != E_VILLAGE)
		load_village (Villagenum, TRUE);
	    else
		Level = TempLevel;
	    if (emerging) {
		print1 ("You emerge onto the street.");
		emerging = FALSE;
	    } else
		print1 ("You enter a small rural village.");
	    ScreenOffset = 0;
	    show_screen();
	    break;
	case E_CAVES:
	    WIDTH = 64;
	    LENGTH = 64;
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
	    change_level (0, 1, FALSE);
	    break;
	case E_VOLCANO:
	    WIDTH = 64;
	    LENGTH = 64;
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
	    change_level (0, 1, FALSE);
	    break;
	case E_ASTRAL:
	    WIDTH = 64;
	    LENGTH = 64;
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
	    change_level (0, 1, FALSE);
	    break;
	case E_CASTLE:
	    WIDTH = 64;
	    LENGTH = 64;
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
	    change_level (0, 1, FALSE);
	    break;
	case E_SEWERS:
	    WIDTH = 64;
	    LENGTH = 64;
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
	    change_level (0, 1, FALSE);
	    break;
	case E_COUNTRYSIDE:
	    WIDTH = 64;
	    LENGTH = 64;
	    print1 ("You return to the fresh air of the open countryside.");
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
	    WIDTH = 64;
	    LENGTH = 16;
	    print1 ("You are now on the tactical screen; exit off any side to leave");
	    make_country_screen (Country[Player.x][Player.y].current_terrain_type);
	    make_country_monsters (Country[Player.x][Player.y].current_terrain_type);
	    Player.x = WIDTH / 2;
	    Player.y = LENGTH / 2;
	    while (Level->site[Player.x][Player.y].locchar == WATER) {
		if (Player.y < LENGTH / 2 + 5)
		    Player.y++;
		else if (Player.x > WIDTH / 2 - 10) {
		    Player.x--;
		    Player.y = LENGTH / 2 - 5;
		} else {
		    Level->site[Player.x][Player.y].locchar = Level->site[Player.x][Player.y].showchar = FLOOR;
		    Level->site[Player.x][Player.y].p_locf = L_NO_OP;
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
	showroom (Level->site[Player.x][Player.y].roomnumber);
    else
	terrain_check (FALSE);
}

// check every ten minutes
void tenminute_check (void)
{
    if (Time % 60 == 0)
	hourly_check();
    else {
	if (Current_Environment == Current_Dungeon)
	    wandercheck();
	minute_status_check();
	tenminute_status_check();
	if ((Player.status[DISEASED] < 1) && (Player.hp < Player.maxhp))
	    Player.hp = min (Player.maxhp, Player.hp + Player.level + 1);
	if (Current_Environment != E_COUNTRYSIDE && Current_Environment != E_ABYSS)
	    indoors_random_event();
    }
}

// hourly check is same as ten_minutely check except food is also
// checked, and since time moves in hours out of doors, also
// outdoors_random_event is possible
void hourly_check (void)
{
    Player.food--;
    foodcheck();
    if (hour() == 0) {		// midnight, a new day
	moon_check();
	Date++;
    }
    torch_check();
    if (Current_Environment == Current_Dungeon)
	wandercheck();
    minute_status_check();
    tenminute_status_check();
    if ((Player.status[DISEASED] == 0) && (Player.hp < Player.maxhp))
	Player.hp = min (Player.maxhp, Player.hp + Player.level + 1);
    if (Current_Environment != E_COUNTRYSIDE && Current_Environment != E_ABYSS)
	indoors_random_event();
}

static void indoors_random_event (void)
{
    switch (random_range (1000)) {
	case 0:
	    print3 ("You feel an unexplainable elation.");
	    morewait();
	    break;
	case 1:
	    print3 ("You hear a distant rumbling.");
	    morewait();
	    break;
	case 2:
	    print3 ("You realize your fly is open.");
	    morewait();
	    break;
	case 3:
	    print3 ("You have a sudden craving for a pecan twirl.");
	    morewait();
	    break;
	case 4:
	    print3 ("A mysterious healing flux settles over the level.");
	    foreach (m, Level->mlist)
		if (m->hp > 0)
		    m->hp = Monsters[m->id].hp;
	    Player.hp = max (Player.hp, Player.maxhp);
	    morewait();
	    break;
	case 5:
	    print3 ("You discover an itch just where you can't scratch it.");
	    morewait();
	    break;
	case 6:
	    print3 ("A cosmic ray strikes!");
	    p_damage (10, UNSTOPPABLE, "a cosmic ray");
	    morewait();
	    break;
	case 7:
	    print3 ("You catch your second wind....");
	    Player.maxhp++;
	    Player.hp = max (Player.hp, Player.maxhp);
	    Player.mana = max (Player.mana, calcmana());
	    morewait();
	    break;
	case 8:
	    print3 ("You find some spare change in a hidden pocket.");
	    Player.cash += Player.level * Player.level + 1;
	    morewait();
	    break;
	case 9:
	    print3 ("You feel strangely lucky.");
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
    int num, i, j;
    switch (random_range (300)) {
	case 0:
	    switch (Country[Player.x][Player.y].current_terrain_type) {
		case TUNDRA:
		    mprint ("It begins to snow. Heavily.");
		    break;
		case DESERT:
		    mprint ("A sandstorm swirls around you.");
		    break;
		default:
		    if ((Date > 75) && (Date < 330))
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
	case 5:
	    mprint ("A weird howling tornado hits from out of the West!");
	    morewait();
	    mprint ("You've been caught in a chaos storm!");
	    morewait();
	    num = random_range (300);
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
		toggle_item_use (TRUE);
		Player.str = Player.maxstr = Player.con = Player.maxcon = Player.dex = Player.maxdex = Player.agi = Player.maxagi = Player.iq = Player.maxiq = Player.pow = Player.maxpow =
		    ((Player.maxstr + Player.maxcon + Player.maxdex + Player.maxagi + Player.maxiq + Player.maxpow + 12) / 6);
		toggle_item_use (FALSE);
	    } else if (num < 30) {
		mprint ("Your entire body glows with an eerie flickering light.");
		morewait();
		toggle_item_use (TRUE);
		for (i = 1; i < MAXITEMS; i++)
		    if (Player.has_possession(i)) {
			Player.possessions[i].plus++;
			if (Player.possessions[i].objchar == STICK)
			    Player.possessions[i].charge += 10;
			Player.possessions[i].blessing += 10;
		    }
		toggle_item_use (FALSE);
		cleanse (1);
		mprint ("You feel filled with energy!");
		morewait();
		Player.maxpow += 5;
		Player.pow += 5;
		Player.mana = Player.maxmana = calcmana() * 5;
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
		Player.x = random_range (WIDTH);
		Player.y = random_range (LENGTH);
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
		for (i = 0; i < NUMRANKS; i++)
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
	    break;
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
	    for (i = Player.x - 5; i < Player.x + 6; i++)
		for (j = Player.y - 5; j < Player.y + 6; j++)
		    if (inbounds (i, j)) {
			c_set (i, j, SEEN);
			if (Country[i][j].current_terrain_type != Country[i][j].base_terrain_type) {
			    c_set (i, j, CHANGED);
			    Country[i][j].current_terrain_type = Country[i][j].base_terrain_type;
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
    char c = '\0';

    menuprint (" (enter location [HCL]) ");
    showmenu();
    while (c == '\0')
	switch (c = ((char) mcigetc())) {
	    case 'h':
		menuprint (" High.");
		break;
	    case 'c':
		menuprint (" Center.");
		break;
	    case 'l':
		menuprint (" Low.");
		break;
	    default:
		c = '\0';
		break;
	}
    showmenu();
    return (c - 'a' + 'A');
}

// chance for player to resist magic somehow
// hostile_magic ranges in power from 0 (weak) to 10 (strong)
int magic_resist (int hostile_magic)
{
    if ((Player.rank[COLLEGE] + Player.rank[CIRCLE] > 0) && (Player.level / 2 + random_range (20) > hostile_magic + random_range (20))) {
	if (Player.mana > hostile_magic * hostile_magic) {
	    mprint ("Thinking fast, you defend youself with a counterspell!");
	    Player.mana -= hostile_magic * hostile_magic;
	    dataprint();
	    return (TRUE);
	}
    }
    if (Player.level / 4 + Player.status[PROTECTION] + random_range (20) > hostile_magic + random_range (30)) {
	mprint ("You resist the spell!");
	return (TRUE);
    } else
	return (FALSE);
}

void terrain_check (int takestime)
{
    int faster = 0;
    if (Player.patron == DRUID) {
	faster = 1;
	switch (random_range (32)) {
	    case 0: print2 ("Along the many paths of nature..."); break;
	    case 1: print2 ("You move swiftly through the wilderness."); break;
	}
    } else if (gamestatusp (MOUNTED)) {
	faster = 1;
	switch (random_range (32)) {
	    case 0:
	    case 1: print2 ("Clippity Clop."); break;
	    case 2: print2 ("....my spurs go jingle jangle jingle...."); break;
	    case 3: print2 ("....as I go riding merrily along...."); break;
	}
    } else if (Player.has_possession(O_BOOTS) && Player.possessions[O_BOOTS].usef == I_BOOTS_7LEAGUE) {
	takestime = 0;
	switch (random_range (32)) {
	    case 0: print2 ("Boingg!"); break;
	    case 1: print2 ("Whooosh!"); break;
	    case 2: print2 ("Over hill, over dale...."); break;
	    case 3: print2 ("...able to leap over 7 leagues in a single bound...."); break;
	}
    } else if (Player.status[SHADOWFORM]) {
	faster = 1;
	switch (random_range (32)) {
	    case 0: print2 ("As swift as a shadow."); break;
	    case 1: print2 ("\"I walk through the trees...\""); break;
	}
    } else {
	switch (random_range (32)) {
	    case 0: print2 ("Trudge. Trudge."); break;
	    case 1: print2 ("The road goes ever onward...."); break;
	}
    }
    switch (Country[Player.x][Player.y].current_terrain_type) {
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
		Time += 60;
		hourly_check();
		Time += 60;
		hourly_check();
		Time += 60;
		hourly_check();
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
		Time += 60;
		hourly_check();
		Time += 60;
		hourly_check();
		Time += 60;
		hourly_check();
		Time += 60;
		hourly_check();
		Time += 60;
		hourly_check();
		if (!faster) {
		    Time += 60;
		    hourly_check();
		    Time += 60;
		    hourly_check();
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
		Time += 60;
		hourly_check();
		Time += 60;
		hourly_check();
		Time += 60;
		hourly_check();
		Time += 60;
		hourly_check();
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
	    switch (Country[Player.x][Player.y].aux) {
		case ODIN:
		    locprint ("A rough-hewn granite temple.");
		    break;
		case SET:
		    locprint ("A black pyramidal temple made of sandstone.");
		    break;
		case ATHENA:
		    locprint ("A classical marble-columned temple.");
		    break;
		case HECATE:
		    locprint ("A temple of ebony adorned with ivory.");
		    break;
		case DRUID:
		    locprint ("A temple formed of living trees.");
		    break;
		case DESTINY:
		    locprint ("A temple of some mysterious blue crystal.");
		    break;
	    }
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
    int x, y;
    Time += 60;
    hourly_check();
    for (x = Player.x - 1; x < Player.x + 2; x++)
	for (y = Player.y - 1; y < Player.y + 2; y++)
	    if (inbounds (x, y)) {
		if (Country[x][y].current_terrain_type != Country[x][y].base_terrain_type) {
		    clearmsg();
		    mprint ("Your search was fruitful!");
		    Country[x][y].current_terrain_type = Country[x][y].base_terrain_type;
		    c_set (x, y, CHANGED);
		    mprint ("You discovered:");
		    mprint (countryid (Country[x][y].base_terrain_type));
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
    "alchemist", "arena", "armorer", "bank", "brothel", "casino", "castle",
    "city gates", "collegium magii", "condo", "department of public works",
    "diner", "explorers' club", "fast food", "gymnasium", "healer", "hospice",
    "les crapuleux", "library", "mercenary guild", "oracle", "order of paladins",
    "pawn shop", "sorcerors' guild ", "tavern", "temple", "thieves' guild"
};

static int sitenums[NUMCITYSITES] = {	// the order matches sitenames[]
    L_ALCHEMIST, L_ARENA, L_ARMORER, L_BANK, L_BROTHEL, L_CASINO, L_CASTLE,
    L_COUNTRYSIDE, L_COLLEGE, L_CONDO, L_DPW, L_DINER, L_CLUB, L_COMMANDANT,
    L_GYM, L_HEALER, L_CHARITY, L_CRAP, L_LIBRARY, L_MERC_GUILD, L_ORACLE,
    L_ORDER, L_PAWN_SHOP, L_SORCERORS, L_TAVERN, L_TEMPLE, L_THIEVES_GUILD
};

static void showknownsites (int first, int last)
{
    int i, printed = FALSE;

    menuclear();
    menuprint ("\nPossible Sites:\n");
    for (i = first; i <= last; i++)
	if (CitySiteList[sitenums[i] - CITYSITEBASE][0]) {
	    printed = TRUE;
	    menuprint (sitenames[i]);
	    menuprint ("\n");
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
		    if (CitySiteList[sitenums[f] - CITYSITEBASE][0])
			first = f;
		    f--;
		}
		l = last;
		while (l < NUMCITYSITES-1 && !strncmp (prefix, sitenames[l], pos)) {
		    if (CitySiteList[sitenums[l] - CITYSITEBASE][0])
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
	else if (byte != '\n') {
	    if (byte >= 'A' && byte <= 'Z')
		byte += 'a' - 'A';
	    if (found)
		continue;
	    f = first;
	    l = last;
	    while (f < NUMCITYSITES && (!CitySiteList[sitenums[f] - CITYSITEBASE][0] || (int)strlen (sitenames[f]) < pos || sitenames[f][pos] < byte))
		f++;
	    while (l >= 0 && (!CitySiteList[sitenums[l] - CITYSITEBASE][0] || (int)strlen (sitenames[l]) < pos || sitenames[l][pos] > byte))
		l--;
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
    } while (byte != '\n');
    xredraw();
    if (found)
	return sitenums[first] - CITYSITEBASE;
    else {
	print3 ("That is an ambiguous abbreviation!");
	return ABORT;
    }
}

// are there hostile monsters within 2 moves?
int hostilemonstersnear (void)
{
    int hostile = FALSE;
    for (int i = Player.x - 2; ((i < Player.x + 3) && (!hostile)); i++)
	for (int j = Player.y - 2; ((j < Player.y + 3) && (!hostile)); j++)
	    if (inbounds (i, j))
		if (Level->creature(i,j))
		    hostile = m_statusp (Level->creature(i,j), HOSTILE);
    return (hostile);
}

// random effects from some of stones in villages
// if alignment of stone is alignment of player, gets done sooner
int stonecheck (int alignment)
{
    int *stone, match = FALSE, cycle = FALSE, i;

    if (alignment == 1) {
	stone = &Lawstone;
	match = Player.alignment > 0;
    } else if (alignment == -1) {
	stone = &Chaostone;
	match = Player.alignment < 0;
    } else {
	stone = &Mindstone;
	match = FALSE;
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
	    cycle = TRUE;
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
    int foundguard = FALSE;
    foreach (m, Level->mlist) {
	if ((m->id == GUARD || (m->id == HISCORE_NPC && m->aux2 == NPC_JUSTICIAR)) && m->hp > 0) {
	    foundguard = TRUE;
	    m_status_set (*m, AWAKE);
	    m_status_set (*m, HOSTILE);
	}
    }
    if (foundguard) {
	mprint ("You hear a whistle and the sound of running feet!");
	if (Current_Environment == E_CITY)
	    Level->site[40][60].p_locf = L_NO_OP;	// pacify_guards restores this
    }
    if (!foundguard && Current_Environment == E_CITY && !gamestatusp (DESTROYED_ORDER)) {
	int suppress = gamestatusp (SUPPRESS_PRINTING);
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

// can only occur when player is in city, so OK to use Level
static void destroy_order (void)
{
    setgamestatus (DESTROYED_ORDER);
    if (Level != City) {
	print1 ("Zounds! A Serious Mistake!");
	return;
    }
    Level->mlist.clear();
    for (int i = 35; i < 46; i++) {
	for (int j = 60; j < 63; j++) {
	    if (i == 40 && (j == 60 || j == 61)) {
		lreset (i, j, SECRET);
		Level->site[i][j].locchar = FLOOR;
		Level->site[i][j].p_locf = L_NO_OP;
		lset (i, j, CHANGED);
	    } else {
		Level->site[i][j].locchar = RUBBLE;
		Level->site[i][j].p_locf = L_RUBBLE;
		lset (i, j, CHANGED);
	    }
	    monster& m = make_site_monster (i, j, GHOST);
	    m.monstring = "ghost of a Paladin";
	    m_status_set (m, HOSTILE);
	}
    }
}

int maneuvers (void)
{
    int m = 2 + Player.level / 7;
    if (Player.rank[ARENA])
	++m;
    if (Player.status[HASTED])
	m *= 2;
    if (Player.status[SLOWED])
	m /= 2;
    return (min (8, max (1, m)));
}

// for when haste runs out, etc.
static void default_maneuvers (void)
{
    int i;
    morewait();
    clearmsg();
    print1 ("Warning, resetting your combat options to the default.");
    print2 ("Use the 'F' command to select which options you prefer.");
    morewait();
    for (i = 0; i < maneuvers(); i += 2) {
	Player.meleestr[i * 2] = 'A';
	Player.meleestr[(i * 2) + 1] = 'C';
	Player.meleestr[(i + 1) * 2] = 'B';
	Player.meleestr[((i + 1) * 2) + 1] = 'C';
    }
    Player.meleestr[maneuvers() * 2] = 0;
}
