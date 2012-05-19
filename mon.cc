#include "glob.h"

//----------------------------------------------------------------------

static void m_aggravate(struct monster *m);
static void m_blind_strike(struct monster *m);
static void m_confused_move(struct monster *m);
static void m_fireball(struct monster *m);
static void m_firebolt(struct monster *m);
static void m_flutter_move(struct monster *m);
static void m_follow_move(struct monster *m);
static void m_hit(struct monster *m, int dtype);
static void m_huge_sounds(struct monster *m);
static void m_illusion(struct monster *m);
static void m_lball(struct monster *m);
static void m_move_animal(struct monster *m);
static void m_move_leash(struct monster *m);
static void m_nbolt(struct monster *m);
static void m_normal_move(struct monster *m);
static void m_random_move(struct monster *m);
static void m_scaredy_move(struct monster *m);
static void m_simple_move(struct monster *m);
static void m_smart_move(struct monster *m);
static void m_snowball(struct monster *m);
static void m_sp_acid_cloud(struct monster *m);
static void m_sp_angel(struct monster *m);
static void m_sp_av(struct monster *m);
static void m_sp_blackout(struct monster *m);
static void m_sp_bogthing(struct monster *m);
static void m_sp_court(struct monster *m);
static void m_sp_demon(struct monster *m);
static void m_sp_demonlover(struct monster *m);
static void m_sp_dragonlord(struct monster *m);
static void m_sp_eater(struct monster *m);
static void m_sp_escape(struct monster *m);
static void m_sp_explode(struct monster *m);
static void m_sp_ghost(struct monster *m);
static void m_sp_lair(struct monster *m);
static void m_sp_lw(struct monster *m);
static void m_sp_mb(struct monster *m);
static void m_sp_merchant(struct monster *m);
static void m_sp_mirror(struct monster *m);
static void m_sp_mp(struct monster *m);
static void m_sp_ng(struct monster *m);
static void m_sp_poison_cloud(struct monster *m);
static void m_sp_prime(struct monster *m);
static void m_sp_raise(struct monster *m);
static void m_sp_seductor(struct monster *m);
static void m_sp_servant(struct monster *m);
static void m_sp_spell(struct monster *m);
static void m_sp_surprise(struct monster *m);
static void m_sp_swarm(struct monster *m);
static void m_sp_were(struct monster *m);
static void m_sp_whistleblower(struct monster *m);
static void m_spirit_move(struct monster *m);
static void m_strike_sonic(struct monster *m);
static void m_summon(struct monster *m);
static void m_talk_animal(struct monster *m);
static void m_talk_archmage(struct monster *m);
static void m_talk_assassin(struct monster *m);
static void m_talk_beg(struct monster *m);
static void m_talk_burble(struct monster *m);
static void m_talk_demonlover(struct monster *m);
static void m_talk_druid(struct monster *m);
static void m_talk_ef(struct monster *m);
static void m_talk_evil(struct monster *m);
static void m_talk_gf(struct monster *m);
static void m_talk_greedy(struct monster *m);
static void m_talk_guard(struct monster *m);
static void m_talk_hint(struct monster *m);
static void m_talk_horse(struct monster *m);
static void m_talk_hungry(struct monster *m);
static void m_talk_hyena(struct monster *m);
static void m_talk_im(struct monster *m);
static void m_talk_man(struct monster *m);
static void m_talk_merchant(struct monster *m);
static void m_talk_mimsy(struct monster *m);
static void m_talk_mp(struct monster *m);
static void m_talk_ninja(struct monster *m);
static void m_talk_parrot(struct monster *m);
static void m_talk_prime(struct monster *m);
static void m_talk_robot(struct monster *m);
static void m_talk_scream(struct monster *m);
static void m_talk_seductor(struct monster *m);
static void m_talk_servant(struct monster *m);
static void m_talk_silent(struct monster *m);
static void m_talk_slithy(struct monster *m);
static void m_talk_stupid(struct monster *m);
static void m_talk_thief(struct monster *m);
static void m_talk_titter(struct monster *m);
static void m_thief_f(struct monster *m);
static void monster_action(struct monster *m, int action);
static int monster_hit(struct monster *m, int hitloc, int bonus);
static void monster_melee(struct monster *m, int hitloc, int bonus);
static void monster_move(struct monster *m);
static void monster_special(struct monster *m);
static void monster_strike(struct monster *m);
static void movemonster(struct monster *m, int newx, int newy);
static void strengthen_death(struct monster *m);
static void tacmonster(struct monster *m);

//----------------------------------------------------------------------

const char* monster::name (void) const
{
    static char namebuf[32] = "The ";
    strcpy (namebuf+strlen("The "), monstring);
    return (uniqueness == COMMON ? namebuf : namebuf+strlen("The "));
}

const char* monster::by_name (void) const
{
    static char namebuf[32] = "a ";
    strcpy (namebuf+strlen("a "), monstring);
    return (uniqueness == COMMON ? namebuf : namebuf+strlen("a "));
}

//----------------------------------------------------------------------

void m_pulse (struct monster *m)
{
    int range = distance (m->x, m->y, Player.x, Player.y);
    if (!(Time % 10))
	if (m->hp < Monsters[m->id].hp)
	    m->hp++;

    if (!m_statusp (m, AWAKE) && range <= m->wakeup) {
	m_status_set (m, AWAKE);
	resetgamestatus (FAST_MOVE);
    }

    bool strike = false;
    if (!m_statusp (m, AWAKE))
	return;
    if (m_statusp (m, WANDERING)) {
	if (m_statusp (m, MOBILE))
	    m_random_move (m);
	if (range <= m->sense && (m_statusp (m, HOSTILE) || m_statusp (m, NEEDY)))
	    m_status_reset (m, WANDERING);
    } else {
	if (m_statusp (m, HOSTILE)) {
	    if (range > 2 && range < m->sense && random_range(2)) {
		if (los_p (m->x, m->y, Player.x, Player.y) && !Player.status[INVISIBLE]) {
		    strike = true;
		    monster_strike (m);
		}
	    }
	}
	if ((m_statusp (m, HOSTILE) || m_statusp (m, NEEDY)) && range > 1 && m_statusp (m, MOBILE) && (!strike || random_range(2)))
	    monster_move (m);
	else if (m_statusp (m, HOSTILE) && range == 1) {
	    resetgamestatus (FAST_MOVE);
	    tacmonster (m);
	}
    }
    // if monster is greedy, picks up treasure it finds
    if (m_statusp(m, GREEDY) && m->hp > 0) {
	foreach (i, Level->things) {
	    if (i->x == m->x && i->y == m->y) {
		m->pickup (*i);
		--(i = Level->things.erase(i));
	    }
	}
    }
    // prevents monsters from casting spells from other side of dungeon
    if (range < max (5, m->level) && m->hp > 0 && random_range(2))
	monster_special (m);
}

// like m_normal_move, but can open doors
static void m_smart_move (struct monster *m)
{
    m_simple_move (m);
}

// not very smart, but not altogether stupid movement
static void m_normal_move (struct monster *m)
{
    m_simple_move (m);
}

// used by both m_normal_move and m_smart_move
static void m_simple_move (struct monster *m)
{
    int dx = sign (Player.x - m->x);
    int dy = sign (Player.y - m->y);

    erase_monster (m);
    if (m->hp < Monsters[m->id].hp / 4) {
	dx = -dx;
	dy = -dy;
	m->movef = M_MOVE_SCAREDY;
	if (!m->possessions.empty()) {
	    mprintf ("%s drops its treasure and flees!", m->name());
	    m_dropstuff (m);
	} else
	    mprintf ("%s flees!", m->name());
	m->speed = min (2, m->speed - 1);
    }
    if ((!m_statusp (m, HOSTILE) && !m_statusp (m, NEEDY)) || (Player.status[INVISIBLE] > 0))
	m_random_move (m);
    else {
	if (m_unblocked (m, m->x + dx, m->y + dy))
	    movemonster (m, m->x + dx, m->y + dy);
	else if (dx == 0) {
	    if (m_unblocked (m, m->x + 1, m->y + dy))
		movemonster (m, m->x + 1, m->y + dy);
	    else if (m_unblocked (m, m->x - 1, m->y + dy))
		movemonster (m, m->x - 1, m->y + dy);
	}

	else if (dy == 0) {
	    if (m_unblocked (m, m->x + dx, m->y + 1))
		movemonster (m, m->x + dx, m->y + 1);
	    else if (m_unblocked (m, m->x + dx, m->y - 1))
		movemonster (m, m->x + dx, m->y - 1);
	}

	else {
	    if (m_unblocked (m, m->x + dx, m->y))
		movemonster (m, m->x + dx, m->y);
	    else if (m_unblocked (m, m->x, m->y + dy))
		movemonster (m, m->x, m->y + dy);
	}
    }
}

static void m_move_animal (struct monster *m)
{
    if (m_statusp (m, HOSTILE))
	m_normal_move (m);
    else
	m_scaredy_move (m);
}

// same as simple move except run in opposite direction
static void m_scaredy_move (struct monster *m)
{
    int dx = -sign (Player.x - m->x);
    int dy = -sign (Player.y - m->y);
    erase_monster (m);
    if (Player.status[INVISIBLE])
	m_random_move (m);
    else {
	if (m_unblocked (m, m->x + dx, m->y + dy))
	    movemonster (m, m->x + dx, m->y + dy);
	else if (dx == 0) {
	    if (m_unblocked (m, m->x + 1, m->y + dy))
		movemonster (m, m->x + 1, m->y + dy);
	    else if (m_unblocked (m, m->x - 1, m->y + dy))
		movemonster (m, m->x - 1, m->y + dy);
	}

	else if (dy == 0) {
	    if (m_unblocked (m, m->x + dx, m->y + 1))
		movemonster (m, m->x + dx, m->y + 1);
	    else if (m_unblocked (m, m->x + dx, m->y - 1))
		movemonster (m, m->x + dx, m->y - 1);
	}

	else {
	    if (m_unblocked (m, m->x + dx, m->y))
		movemonster (m, m->x + dx, m->y);
	    else if (m_unblocked (m, m->x, m->y + dy))
		movemonster (m, m->x, m->y + dy);
	}
    }
}

// for spirits (and earth creatures) who can ignore blockages because
// either they are noncorporeal or they can move through stone
static void m_spirit_move (struct monster *m)
{
    int dx = sign (Player.x - m->x);
    int dy = sign (Player.y - m->y);
    erase_monster (m);
    if (m->hp < Monsters[m->id].hp / 6) {
	dx = -dx;
	dy = -dy;
    }

    if (Player.status[INVISIBLE] > 0 || !m_unblocked (m, m->x + dx, m->y + dy))
	m_random_move (m);
    else
	movemonster (m, m->x + dx, m->y + dy);
}

// fluttery dumb movement
static void m_flutter_move (struct monster *m)
{
    int trange, range = distance (m->x, m->y, Player.x, Player.y);
    int i, tx, ty, nx = m->x, ny = m->y;
    erase_monster (m);
    if (Player.status[INVISIBLE] > 0)
	m_random_move (m);
    else {
	for (i = 0; i < 8; i++) {
	    tx = m->x + Dirs[0][i];
	    ty = m->y + Dirs[1][i];
	    trange = distance (tx, ty, Player.x, Player.y);
	    if (m->hp < Monsters[m->id].hp / 6) {
		if ((trange > range) && m_unblocked (m, tx, ty)) {
		    range = trange;
		    nx = tx;
		    ny = ty;
		}
	    } else if ((trange <= range) && m_unblocked (m, tx, ty)) {
		range = trange;
		nx = tx;
		ny = ty;
	    }
	}
	movemonster (m, nx, ny);
    }
}

static void m_follow_move (struct monster *m)
{
    if (!m_statusp (m, HOSTILE))
	m_normal_move (m);
    else
	m_scaredy_move (m);
}

// allows monsters to fall into pools, revealed traps, etc
static void m_confused_move (struct monster *m)
{
    int i, nx, ny, done = FALSE;
    erase_monster (m);
    for (i = 0; ((i < 8) && (!done)); i++) {
	nx = m->x + random_range (3) - 1;
	ny = m->y + random_range (3) - 1;
	if (unblocked (nx, ny) && ((nx != Player.x) || (ny != Player.y))) {
	    done = TRUE;
	    movemonster (m, nx, ny);
	}
    }
}

static void m_random_move (struct monster *m)
{
    int i, nx, ny, done = FALSE;
    erase_monster (m);
    for (i = 0; ((i < 8) && (!done)); i++) {
	nx = m->x + random_range (3) - 1;
	ny = m->y + random_range (3) - 1;
	if (m_unblocked (m, nx, ny) && ((nx != Player.x) || (ny != Player.y))) {
	    done = TRUE;
	    movemonster (m, nx, ny);
	}
    }
}

// monster removed from play
void m_vanish (struct monster *m)
{
    mprintf ("%s vanishes in the twinkling of an eye!", m->name());
    Level->mlist.erase (m);
    erase_monster (m);
}

// monster still in play
void m_teleport (struct monster *m)
{
    erase_monster (m);
    if (m_statusp (m, AWAKE)) {
	findspace (&(m->x), &(m->y), -1);
	levelrefresh();
    }
}

static void m_move_leash (struct monster *m)
{
    m_simple_move (m);
    if (m->aux1 == 0) {
	m->aux1 = m->x;
	m->aux2 = m->y;
    } else if (distance (m->x, m->y, m->aux1, m->aux2) > 5) {
	monster* lm = Level->creature(m->aux1,m->aux2);
	if (lm) {
	    if (los_p (Player.x, Player.y, m->aux1, m->aux2)) // some other monster is where the chain starts
		mprintf ("%s releases the dog's chain!", lm->name());
	    m->movef = M_MOVE_NORMAL;
	    // otherwise, we'd lose either the dog or the other monster.
	} else if (los_p (Player.x, Player.y, m->x, m->y)) {
	    mprint ("You see the dog jerked back by its chain!");
	    plotspot (m->x, m->y, FALSE);
	} else
	    mprint ("You hear a strangled sort of yelp!");
	m->x = m->aux1;
	m->y = m->aux2;
    }
}

// actually make a move
static void movemonster (struct monster *m, int newx, int newy)
{
    if (Level->creature(newx,newy))
	return;
    m->x = newx;
    m->y = newy;
    m_movefunction (m, Level->site(m->x,m->y).p_locf);
}

// The druid's altar is in the northern forest
static void m_talk_druid (struct monster *m)
{
    if (!m_statusp (m, HOSTILE)) {
	print1 ("The Archdruid raises a hand in greeting.");
	if (!gamestatusp (SPOKE_TO_DRUID)) {
	    setgamestatus (SPOKE_TO_DRUID);
	    morewait();
	    print1 ("The Archdruid congratulates you on reaching his sanctum.");
	    print2 ("You feel competent.");
	    morewait();
	    gain_experience (300);
	    if (Player.patron == DRUID) {
		print1 ("The Archdruid conveys to you the wisdom of nature....");
		print2 ("You feel like a sage.");
		morewait();
		for (int i = 0; i < NUMRANKS; i++)
		    if (Player.guildxp[i] > 0)
			Player.guildxp[i] += 300;
	    }
	}
	mprint ("Do you request a ritual of neutralization? [yn] ");
	if (ynq() == 'y') {
	    if (Phase / 2 == 6 || Phase / 2 == 0) {	// full or new moon
		mprint ("\"Unfortunately, I cannot perform a ritual of balance on");
		if (Phase / 2 == 6)
		    mprint ("this lawful day.\"");
		else
		    mprint ("this chaotic day.\"");
	    } else if (Phase / 2 == 3 || Phase / 2 == 9) {	// half moon
		mprint ("You take part in today's holy celebration of balance...");
		Player.alignment = 0;
		Player.mana = calcmana();
		if (Player.patron == DRUID)
		    gain_experience (200);
		for (int i = 0; i < 6; ++i) {
		    Time += 60;			// if a druid wants to spend 2 days
		    hourly_check();		// celebrating for 1600 xp, why not?
		}
	    } else {
		mprint ("The ArchDruid conducts a sacred rite of balance...");
		if (Player.patron == DRUID) {
		    Player.alignment = 0;
		    Player.mana = calcmana();
		} else
		    Player.alignment -= Player.alignment * max (0, 10 - Player.level) / 10;
		// the higher level the character is, the more set in his/her ways
		Time += 60;
		hourly_check();
	    }
	    dataprint();
	}
    } else {
	mprint ("The ArchDruid looks at you and cries: 'Unclean! Unclean!'");
	disrupt (Player.x, Player.y, 100);
	mprint ("This seems to have satiated his desire for vengeance.");
	mprint ("'Have you learned your lesson?' The ArchDruid asks. [yn] ");
	if (ynq()) {
	    mprint ("'I certainly hope so!' says the ArchDruid.");
	    foreach (lm, Level->mlist)
		m_status_reset (*lm, HOSTILE);
	    m_vanish (m);
	} else {
	    mprint ("'Idiot.' mutters the ArchDruid.");
	    p_damage (500, UNSTOPPABLE, "the ArchDruid's Vengeance");
	}
    }
}

static void m_talk_silent (struct monster *m)
{
    static const char _silent_replies[] =
	"does not reply\0"
	"shrugs silently\0"
	"hold a finger to his mouth\0"
	"glares at you but says nothing";
    mprintf ("%s %s", m->name(), zstrn(_silent_replies,random_range(4),4));
}

static void m_talk_stupid (struct monster *m)
{
    static const char _stupid_replies[] =
	"looks at you with mute incomprehension.\0"
	"growls menacingly and ignores you.\0"
	"does not seem to have heard you.\0"
	"tries to pretend it didn't hear you.";
    mprintf ("%s %s", m->name(), zstrn(_stupid_replies,random_range(4),4));
}

static void m_talk_greedy (struct monster *m)
{
    static const char _greedy_replies[] =
	"Give me a treasure....\0"
	"Stand and deliver, knave!\0"
	"Your money or your life!\0"
	"Yield or Die!";
    mprintf ("%s says: %s", m->name(), zstrn(_greedy_replies,random_range(4),4));
}

static void m_talk_hungry (struct monster *m)
{
    static const char _hungry_replies[] =
	    "says: I hunger, foolish adventurer!\0"
	    "drools menacingly at you.\0"
	    "says: You're invited to be lunch!\0"
	    "says: Feeed Meee!";
    mprintf ("%s %s", m->name(), zstrn(_hungry_replies,random_range(4),4));
}

static void m_talk_guard (struct monster *m)
{
    if (m_statusp (m, HOSTILE)) {
	print1 ("'Surrender in the name of the Law!'");
	print2 ("Do it? [yn] ");
	if (ynq2() == 'y') {
	    Player.alignment++;
	    if (Current_Environment == E_CITY) {
		print1 ("Go directly to jail. Do not pass go, do not collect 200Au.");
		print2 ("You are taken to the city gaol.");
		morewait();
		send_to_jail();
		drawvision (Player.x, Player.y);
	    } else {
		clearmsg();
		print1 ("Mollified, the guard disarms you and sends you away.");
		Player.remove_possession (O_WEAPON_HAND);
		pacify_guards();
	    }
	} else {
	    clearmsg();
	    print1 ("All right, you criminal scum, you asked for it!");
	}
    } else if (Player.rank[ORDER] >= GALLANT)
	print1 ("'Greetings comrade! May you always tread the paths of Law.'");
    else
	print1 ("Move it right along, stranger!");
}

static void m_talk_mp (struct monster *m UNUSED)
{
    mprint ("The mendicant priest asks you to spare some treasure for the needy");
}

static void m_talk_titter (struct monster *m)
{
    mprintf ("%s titters obscenely at you", m->name());
    mprint (Str2);
}

static void m_talk_ninja (struct monster *m UNUSED)
{
    mprint ("The black-garbed figure says apologetically:");
    mprint ("'Situree simasita, wakarimasen.'");
}

static void m_talk_thief (struct monster *m)
{
    if (Player.rank[THIEVES]) {
	if (m->level == ATHIEF)
	    m->monstring = "sneak thief";
	else
	    m->monstring = "master thief";
	print1 ("The cloaked figure makes a gesture which you recognize...");
	print2 ("...the thieves' guild recognition signal!");
	print3 ("'Sorry, mate, thought you were a mark....'");
	morewait();
	m_vanish (m);
    } else
	m_talk_man (m);

}

static void m_talk_assassin (struct monster *m)
{
    m->monstring = "master assassin";
    print1 ("The ominous figure does not reply, but hands you an embossed card:");
    print2 ("'Guild of Assassins Ops are forbidden to converse with targets.'");
}

static void m_talk_im (struct monster *m)
{
    m->monstring = "itinerant merchant";
    if (m->possessions.empty()) {
	mprint ("The merchant says: Alas! I have nothing to sell!");
	return;
    }
    clearmsg();
    object* o = &m->possessions[0];
    unsigned cost = max (10, 4 * true_item_value(o));
    mprintf ("I have a fine %s for only %uAu. Want it? [yn] ", itemid(o), cost);
    if (ynq() != 'y')
	mprint ("Well then, I must be off. Good day.");
    else if (Player.cash < cost) {
	if (Player.alignment > 10) {
	    mprint ("Well, I'll let you have it for what you've got.");
	    Player.cash = 0;
	    gain_item (*o);
	    m->possessions.erase(o);
	} else
	    mprint ("Beat it, you deadbeat!");
    } else {
	mprint ("Here you are. Have a good day.");
	Player.cash -= cost;
	gain_item (*o);
	m->possessions.erase(o);
    }
    m_vanish (m);
}

static void m_talk_man (struct monster *m)
{
    static const char _man_replies[] =
	"asks you for the way home\0"
	"wishes you a pleasant day\0"
	"sneers at you contemptuously\0"
	"smiles and nods\0"
	"tells you a joke";
    mprintf ("%s %s", m->name(), zstrn(_man_replies,random_range(5),5));
}

static void m_talk_evil (struct monster *m)
{
    static const char _evil_replies[] =
	"says: 'THERE CAN BE ONLY ONE!'\0"
	"says: 'Prepare to die, Buckwheat!'\0"
	"says: 'Time to die!'\0"
	"says: 'There will be no mercy'\0"
	"insults your mother-in-law\0"
	"says: 'Kurav tu ando mul!'\0"
	"says: '!va al infierno!'\0"
	"says: 'dame desu, nee'\0"
	"spits on your rug and calls your cat a bastard\0"
	"snickers malevolently and draws a weapon\0"
	"sends 'rm -r *' to your shell!\0"
	"tweaks your nose and cackles evilly\0"
	"thumbs you in the eyes\0"
	"kicks you in the groin";
    mprintf ("%s %s", m->name(), zstrn(_evil_replies,random_range(14),14));
}

static void m_talk_robot (struct monster *m)
{
    static const char _robot_replies[] =
	"exterminate...Exterminate...EXTERMINATE!!!\0"
	"Kill ... Crush ... Destroy\0"
	"Danger -- Danger\0"
	"Yo Mama -- core dumped";
    mprintf ("%s says: %s", m->name(), zstrn(_robot_replies,random_range(4),4));
}

static void m_talk_slithy (struct monster *m UNUSED)
{
    mprint ("It can't talk -- it's too slithy!");
}

static void m_talk_mimsy (struct monster *m UNUSED)
{
    mprint ("It can't talk -- it's too mimsy!");
}

static void m_talk_burble (struct monster *m)
{
    mprintf ("%s burbles hatefully at you", m->name());
}

static void m_talk_beg (struct monster *m)
{
    mprintf ("%s asks you for alms", m->name());
}

static void m_talk_hint (struct monster *m)
{
    if (m_statusp (m, HOSTILE))
	mprintf ("%s only sneers at you", m->name());
    else {
	mprintf ("%s whispers in your ear: ", m->name());
	hint();
	m->talkf = M_TALK_SILENT;
    }
}

static void m_talk_gf (struct monster *m)
{
    mprint ("The good fairy glints: Would you like a wish?");
    if (ynq() == 'y') {
	mprint ("The good fairy glows: Are you sure?");
	if (ynq() == 'y') {
	    mprint ("The good fairy radiates: Really really sure?");
	    if (ynq() == 'y') {
		mprint ("The good fairy beams: I mean, like, sure as sure can be?");
		if (ynq() == 'y') {
		    mprint ("The good fairy dazzles: You don't want a wish, right?");
		    if (ynq() == 'y')
			mprint ("The good fairy laughs: I thought not.");
		    else
			wish (0);
		}
	    }
	}
    }
    mprint ("In a flash of sweet-smelling light, the fairy vanishes....");
    Player.hp = max (Player.hp, Player.maxhp);
    Player.mana = max (Player.mana, calcmana());
    mprint ("You feel mellow.");
    m_vanish (m);
}

static void m_talk_ef (struct monster *m)
{
    mprint ("The evil fairy roils: Eat my pixie dust!");
    mprint ("She waves her black-glowing wand, which screams thinly....");
    m->movef = M_MOVE_SMART;
    m->meleef = M_MELEE_POISON;
    m->specialf = M_SP_THIEF;
    acquire (-1);
    bless (-1);
    sleep_player (m->level / 2);
    summon (-1, -1);
    summon (-1, -1);
    summon (-1, -1);
    summon (-1, -1);
}

static void m_talk_seductor (struct monster *m)
{
    if (Player.preference == 'n')
	mprintf ("%s notices your disinterest and leaves with a pout", m->name());
    else {
	mprintf ("%s beckons seductively... Flee? [yn] ", m->name());
	if (ynq() == 'y')
	    mprint ("You feel stupid.");
	else {
	    strcpy (Str2, "The ");
	    strcat (Str2, m->monstring);
	    strcat (Str2, " shows you a good time....");
	    mprint (Str2);
	    gain_experience (500);
	    ++Player.con;
	}
    }
    m_vanish (m);
}

static void m_talk_demonlover (struct monster *m)
{
    if (Player.preference == 'n') {
	mprintf ("%s notices your disinterest and changes with a snarl...", m->name());
	morewait();
    } else {
	mprintf ("%s beckons seductively... Flee? [yn] ", m->name());
	if (ynq() == 'y')
	    mprint ("You feel fortunate....");
	else {
	    mprintf ("%s shows you a good time....", m->name());
	    morewait();
	    mprint ("You feel your life energies draining...");
	    level_drain (random_range (3) + 1, "a demon's kiss");
	    morewait();
	}
    }
    m->talkf = M_TALK_EVIL;
    m->meleef = M_MELEE_SPIRIT;
    m->specialf = M_SP_DEMON;

    if ((m->monchar & 0xff) == 's') {
	m->monchar = 'I' | CLR_RED_BLACK;
	m->monstring = "incubus";
    } else {
	m->monchar = 'S' | CLR_RED_BLACK;
	m->monstring = "succubus";
    }
    mprintf ("%s laughs insanely.", m->name());
    mprint ("You now notice the fangs, claws, batwings...");
}

static void m_talk_horse (struct monster *m)
{
    if (m_statusp (m, HOSTILE))
	mprint ("The horse neighs angrily at you.");
    else if (m_statusp (m, HUNGRY))
	mprint ("The horse noses curiously at your pack.");
    else if (gamestatusp (MOUNTED))
	mprint ("The horse and your steed don't seem to get along.");
    else if (Current_Environment == Current_Dungeon)
	mprint ("The horse shies; maybe he doesn't like the dungeon air....");
    else {
	mprint ("The horse lets you pat his nose. Want to ride him? [yn] ");
	if (ynq() == 'y') {
	    Level->mlist.erase (m);
	    setgamestatus (MOUNTED);
	    calc_melee();
	    mprint ("You are now equitating!");
	}
    }
}

static void m_talk_hyena (struct monster *m UNUSED)
{
    mprint ("The hyena only laughs at you...");
}

static void m_talk_parrot (struct monster *m UNUSED)
{
    mprint ("Polly wanna cracker?");
}

static void m_talk_servant (struct monster *m)
{
    int target, x = Player.x, y = Player.y;
    if (m->id == SERV_LAW) {
	target = SERV_CHAOS;
	mprint ("The Servant of Law pauses in thought for a moment.");
	mprint ("You are asked: Are there any Servants of Chaos hereabouts? [yn] ");
    } else {
	target = SERV_LAW;
	mprint ("The Servant of Chaos grins mischievously at you.");
	mprint ("You are asked: Are there any Servants of Law hereabouts? [yn] ");
    }
    if (ynq() != 'y')
	mprint ("The servant shrugs and turns away.");
    else {
	print1 ("Show me.");
	show_screen();
	drawmonsters (TRUE);
	setspot (&x, &y);
	monster* om = Level->creature(x,y);
	if (!om || om->id != target)
	    mprint ("Right. Tell me about it. Idiot!");
	else {
	    mprint ("The Servant launches itself towards its enemy.");
	    mprint ("In a blaze of combat, the Servants annihilate each other!");
	    gain_experience (m->xpv);
	    m_death (om);
	    m->x = x; m->y = y;
	    m_death (m);
	}
    }
}

static void m_talk_animal (struct monster *m)
{
    mprintf ("%s shows you a scholarly paper by Dolittle, D. Vet.", m->name());
    mprint ("which demonstrates that animals don't have speech centers");
    mprint ("complex enough to communicate in higher languages.");
    mprint ("It giggles softly to itself and takes back the paper.");
}

static void m_talk_scream (struct monster *m)
{
    mprint ("A thinly echoing scream reaches your ears....");
    morewait();
    mprint ("You feel doomed....");
    morewait();
    mprint ("A bird appears and flies three times widdershins around your head.");
    summon (-1, QUAIL);
    m->talkf = M_TALK_EVIL;
}

static void m_talk_archmage (struct monster *m)
{
    if (m_statusp (m, HOSTILE)) {
	mprint ("The Archmage ignores your attempt at conversation");
	mprint ("and concentrates on his spellcasting....");
    } else if (Current_Environment == E_COURT) {
	mprint ("The Archmage congratulates you on getting this far.");
	mprint ("He invites you to attempt the Throne of High Magic");
	mprint ("but warns you that it is important to wield the Sceptre");
	mprint ("before sitting on the throne.");
	if (Level->site(m->x,m->y).p_locf == L_THRONE) {
	    mprint ("The Archmage smiles and makes an arcane gesture....");
	    m_vanish (m);
	}
    } else {
	mprint ("The Archmage tells you to find him again in his");
	mprint ("Magical Court at the base of his castle in the mountains");
	mprint ("of the far North-East; if you do he will give you some");
	mprint ("important information.");
    }
}

static void m_talk_merchant (struct monster *m)
{
    if (!m_statusp (m, HOSTILE)) {
	if (Current_Environment == E_VILLAGE) {
	    mprint ("The merchant asks you if you want to buy a horse for 250GP.");
	    mprint ("Pay the merchant? [yn] ");
	    if (ynq() == 'y') {
		if (Player.cash < 250)
		    mprint ("The merchant says: 'Come back when you've got the cash!'");
		else {
		    Player.cash -= 250;
		    mprint ("The merchant takes your money and tells you to select");
		    mprint ("any horse you want in the stables.");
		    mprint ("He says: 'You'll want to get to know him before trying to");
		    mprint ("ride him. By the way, food for the horse is not included.'");
		    mprint ("The merchant runs off toward the bank, cackling gleefully.");
		    m_vanish (m);
		}
	    } else
		mprint ("The merchant tells you to stop wasting his time.");
	} else {
	    mprint ("The merchant tells you to visit his stables at his village");
	    mprint ("for a great deal on a horse.");
	}
    } else {
	mprint ("The merchant ignores you and screams:");
	mprint ("'Help! Help! I'm being oppressed!'");
    }
}

static void m_talk_prime (struct monster *m)
{
    if (m_statusp (m, HOSTILE))
	return (m_talk_evil (m));
    if (Current_Environment == E_CIRCLE) {
	print1 ("The Prime nods brusquely at you, removes a gem from his");
	print2 ("sleeve, places it on the floor, and vanishes wordlessly.");
	morewait();
	m_dropstuff (m);
	m_vanish (m);
    } else {
	print1 ("The Prime makes an intricate gesture, which leaves behind");
	print2 ("glowing blue sparks... He winks mischievously at you....");
	if (Player.rank[CIRCLE] >= INITIATE) {
	    morewait();
	    print1 ("The blue sparks strike you! You feel enhanced!");
	    print2 ("You feel more experienced....");
	    Player.pow += Player.rank[CIRCLE];
	    Player.mana += calcmana();
	    gain_experience (1000);
	    m_vanish (m);
	}
    }
}

void m_dropstuff (struct monster *m)
{
    foreach (i, m->possessions) {
	i->x = m->x;
	i->y = m->y;
	Level->things.push_back (*i);
    }
    m->possessions.clear();
}

static void m_hit (struct monster *m, int dtype)
{
    if (Player.status[DISPLACED] > 0 && random_range(2))
	mprint ("The attack was displaced!");
    else
	p_damage (random_range (m->dmg), dtype, m->by_name());
}

// execute monster attacks versus player
static void tacmonster (struct monster *m)
{
    drawvision (Player.x, Player.y);
    transcribe_monster_actions (m);
    for (unsigned i = 0; i < strlen(m->meleestr) && m->hp > 0; i += 2) {
	if (m->meleestr[i] == 'A') {
	    if (Verbosity == VERBOSE)
		mprintf ("%s attacks %s", m->name(), actionlocstr (m->meleestr[i + 1]));
	    monster_melee (m, m->meleestr[i + 1], 0);
	} else if (m->meleestr[i] == 'L') {
	    if (Verbosity == VERBOSE)
		mprintf ("%s lunges %s", m->name(), actionlocstr (m->meleestr[i + 1]));
	    monster_melee (m, m->meleestr[i + 1], m->level);
	}
    }
}

static void monster_melee (struct monster *m, int hitloc, int bonus)
{
    if (player_on_sanctuary()) {
	print1 ("The aegis of your deity protects you!");
	return;
    }
    // It's lawful to wait to be attacked
    if (m->attacked == 0)
	++Player.alignment;
    ++m->attacked;
    if (m->uniqueness == COMMON) {
	strcpy (Str2, "The ");
	strcat (Str2, m->monstring);
    } else
	strcpy (Str2, m->monstring);
    if (monster_hit (m, hitloc, bonus))
	switch (m->meleef) {
	    case M_NO_OP:
		mprintf ("%s touches you.", m->name());
		break;
	    case M_MELEE_NORMAL:
		mprintf ("%s hits you.", m->name());
		m_hit (m, NORMAL_DAMAGE);
		break;
	    case M_MELEE_NG:
		mprintf ("%s hits you.", m->name());
		m_hit (m, NORMAL_DAMAGE);
		if (random_range (5) == 3)
		    m_sp_ng (m);
		break;
	    case M_MELEE_FIRE:
		mprintf ("%s blasts you with fire.", m->name());
		m_hit (m, FLAME);
		break;
	    case M_MELEE_DRAGON:
		mprintf ("%s hits you and blasts you with fire.", m->name());
		m_hit (m, NORMAL_DAMAGE);
		m_hit (m, FLAME);
		break;
	    case M_MELEE_ELEC:
		mprintf ("%s lashes you with electricity.", m->name());
		m_hit (m, ELECTRICITY);
		break;
	    case M_MELEE_COLD:
		mprintf ("%s freezes you with cold.", m->name());
		m_hit (m, ELECTRICITY);
		break;
	    case M_MELEE_POISON:
		mprintf ("%s hits you.", m->name());
		m_hit (m, NORMAL_DAMAGE);
		if (random_range (10) < m->level) {
		    mprint ("You've been poisoned!");
		    p_poison (m->dmg);
		}
		break;
	    case M_MELEE_GRAPPLE:
		mprintf ("%s grabs you.", m->name());
		m_hit (m, NORMAL_DAMAGE);
		Player.status[IMMOBILE]++;
		break;
	    case M_MELEE_SPIRIT:
		mprintf ("%s touches you.", m->name());
		m_hit (m, NORMAL_DAMAGE);
		drain_life (m->level);
		break;
	    case M_MELEE_DISEASE:
		mprintf ("%s hits you.", m->name());
		m_hit (m, NORMAL_DAMAGE);
		if (random_range (10) < m->level) {
		    mprint ("You've been infected!");
		    disease (m->level);
		}
		break;
	    case M_MELEE_SLEEP:
		mprintf ("%s hit you.", m->name());
		m_hit (m, NORMAL_DAMAGE);
		if (random_range (10) < m->level) {
		    mprint ("You feel drowsy");
		    sleep_player (m->level);
		}
		break;
    } else if (random_range(10)) {
	mprintf ("%s missed you.", m->name());
    } else {
	const char* msg = "missed you";
	if (Verbosity == TERSE) {
	    switch (random_range(10)) {
		case 0:
		    msg = "blundered severely";
		    m_damage (m, m->dmg, UNSTOPPABLE);
		    break;
		case 1:
		    msg = "tripped while attacking";
		    m_dropstuff (m);
		    break;
		case 2:
		    msg = "seems seriously confused";
		    m->speed = min (30, m->speed * 2);
		    break;
	    }
	}
	switch (random_range (10)) {
	    case 0: msg = "flailed stupidly at you"; break;
	    case 1: msg = "made you laugh"; break;
	    case 2: msg = "is seriously ashamed"; break;
	    case 3: msg = " made a boo-boo"; break;
	    case 4: msg = "blundered"; break;
	    case 5: msg = "cries out in anger and frustration"; break;
	    case 6: msg = "curses your ancestry"; break;
	    case 7:
		msg = "blundered severely";
		m_damage (m, m->dmg, UNSTOPPABLE);
		break;
	    case 8:
		msg = "tripped while attacking";
		m_dropstuff (m);
		break;
	    case 9:
		msg = "seems seriously confused";
		m->speed = min (30, m->speed * 2);
		break;
	}
	mprintf ("%s %s", m->name(), msg);
    }
}

// checks to see if player hits with hitmod vs. monster m at location hitloc
static int monster_hit (struct monster *m, int hitloc, int bonus)
{
    int blocks = FALSE, goodblocks = 0, hit, riposte = FALSE;
    for (unsigned i = 0; i < strlen (Player.meleestr); i += 2) {
	if (Player.meleestr[i] == 'B' || Player.meleestr[i] == 'R') {
	    blocks = TRUE;
	    if (hitloc == Player.meleestr[i + 1]) {
		goodblocks++;
		if (Player.meleestr[i] == 'R')
		    riposte = TRUE;
	    }
	}
    }
    if (!blocks)
	goodblocks = -1;
    hit = hitp (m->hit + bonus, Player.defense + goodblocks * 10);
    if ((!hit) && (goodblocks > 0)) {
	if (Verbosity == VERBOSE)
	    mprint ("You blocked it!");
	if (riposte) {
	    if (Verbosity != TERSE)
		mprint ("You got a riposte!");
	    if (!hitp (Player.hit, m->ac))
		mprint ("You missed.");
	    else {
		mprint ("You hit!");
		weapon_use (0, &Player.possessions[O_WEAPON_HAND], m);
	    }
	}
    }
    return (hit);
}

// decide monster actions in tactical combat mode
// if monster is skilled, it can try see the player's attacks coming and
// try to block appropriately.
void transcribe_monster_actions (struct monster *m)
{
    static char mmstr[80];

    int p_blocks[3];
    int p_attacks[3];

    for (int i = 0; i < 3; i++)
	p_blocks[i] = p_attacks[i] = 0;

    // Find which area player blocks and attacks least in
    for (unsigned i = 0; i < strlen (Player.meleestr); i += 2) {
	if ((Player.meleestr[i] == 'B') || (Player.meleestr[i] == 'R')) {
	    if (Player.meleestr[i + 1] == 'H')
		p_blocks[0]++;
	    if (Player.meleestr[i + 1] == 'C')
		p_blocks[1]++;
	    if (Player.meleestr[i + 1] == 'L')
		p_blocks[2]++;
	} else if ((Player.meleestr[i] == 'A') || (Player.meleestr[i] == 'L')) {
	    if (Player.meleestr[i + 1] == 'H')
		p_attacks[0]++;
	    if (Player.meleestr[i + 1] == 'C')
		p_attacks[1]++;
	    if (Player.meleestr[i + 1] == 'L')
		p_attacks[2]++;
	}
    }

    m->meleestr = mmstr;
    if (m->id != NPC)
	m->meleestr = Monsters[m->id].meleestr;
    else
	m->meleestr = m_melee_str (m->level/2);

#if 0	// FIXME: melee strings are const
    char attack_loc, block_loc;
    if ((p_blocks[2] <= p_blocks[1]) && (p_blocks[2] <= p_blocks[0]))
	attack_loc = 'L';
    else if ((p_blocks[1] <= p_blocks[2]) && (p_blocks[1] <= p_blocks[0]))
	attack_loc = 'C';	// DG 12/31/98
    else
	attack_loc = 'H';
    if ((p_attacks[2] <= p_attacks[1]) && (p_attacks[2] <= p_attacks[0]))
	block_loc = 'L';
    else if ((p_attacks[1] <= p_attacks[2]) && (p_attacks[1] <= p_attacks[0]))
	block_loc = 'C';
    else
	block_loc = 'H';

    for (unsigned i = 0; i < strlen (m->meleestr); i += 2) {
	if ((m->meleestr[i] == 'A') || (m->meleestr[i] == 'L')) {
	    if (m->meleestr[i + 1] == '?') {
		if (m->level + random_range (30) > Player.level + random_range (20))
		    m->meleestr[i + 1] = attack_loc;
		else
		    m->meleestr[i + 1] = random_loc();
	    } else if (m->meleestr[i + 1] == 'X')
		m->meleestr[i + 1] = random_loc();
	} else if ((m->meleestr[i] == 'B') || (m->meleestr[i] == 'R')) {
	    if (m->meleestr[i + 1] == '?') {
		if (m->level + random_range (30) > Player.level + random_range (20))
		    m->meleestr[i + 1] = block_loc;
		else
		    m->meleestr[i + 1] = random_loc();
	    } else if (m->meleestr[i + 1] == 'X')
		m->meleestr[i + 1] = random_loc();
	}
    }
#endif
}

char random_loc (void)
{
    return ("HCL"[random_range(strlen("HCL"))]);
}

static void m_firebolt (struct monster *m)
{
    fbolt (m->x, m->y, Player.x, Player.y, m->hit, m->dmg);
}

static void m_nbolt (struct monster *m)
{
    nbolt (m->x, m->y, Player.x, Player.y, m->hit, m->dmg);
}

static void m_lball (struct monster *m)
{
    lball (m->x, m->y, Player.x, Player.y, m->dmg);
}

static void m_fireball (struct monster *m)
{
    fball (m->x, m->y, Player.x, Player.y, m->dmg);
}

static void m_snowball (struct monster *m)
{
    snowball (m->x, m->y, Player.x, Player.y, m->dmg);
}

static void m_blind_strike (struct monster *m)
{
    if (Player.status[BLINDED] == 0 && los_p(m->x, m->y, Player.x, Player.y) && distance (m->x, m->y, Player.x, Player.y) < 5) {
	mprintf ("%s gazes at you menacingly", m->name());
	if (!p_immune (GAZE)) {
	    mprint ("You've been blinded!");
	    Player.status[BLINDED] = random_range (4) + 1;
	    levelrefresh();
	} else
	    mprint ("You gaze steadily back....");
    }
}

static void m_strike_sonic (struct monster *m)
{
    mprintf ("%s screams at you!", m->name());
    p_damage (m->dmg, OTHER_MAGIC, "a sonic blast");
}


static void m_sp_mp (struct monster *m)
{
    if (m->attacked && random_range(3)) {
	mprint ("You feel cursed!");
	p_damage (10, UNSTOPPABLE, "a mendicant priest's curse");
	m_vanish (m);
    } else if (!m_statusp (m, NEEDY)) {
	mprint ("The mendicant priest makes a mystical gesture....");
	mprint ("You feel impressed...");
	Player.alignment += 5;
	if (Player.alignment > 20)
	    Player.hp = max (Player.hp, Player.maxhp);
	m_vanish (m);
    }
}

static void m_sp_ng (struct monster *m)
{
    if (distance (m->x, m->y, Player.x, Player.y) >= 2 || random_range(5) || Player.status[VULNERABLE] <= 0)
	return;
    mprint ("The night gaunt grabs you and carries you off!");
    mprint ("Its leathery wings flap and flap, and it giggles insanely.");
    mprint ("It tickles you cunningly to render you incapable of escape.");
    mprint ("Finally, it deposits you in a strange place.");
    p_teleport (0);
}

static void m_sp_poison_cloud (struct monster *m)
{
    if (distance (m->x, m->y, Player.x, Player.y) >= 3)
	return;
    mprint ("A cloud of poison gas surrounds you!");
    if (Player.status[BREATHING] > 0)
	mprint ("You can breathe freely, however.");
    else
	p_poison (7);
}

static void m_sp_explode (struct monster *m)
{
    if (distance (Player.x, Player.y, m->x, m->y) < 2 && m->hp > 0 && m->hp < Monsters[m->id].hp)
	fball (m->x, m->y, m->x, m->y, m->hp);
}

static void m_sp_demon (struct monster *m)
{
    if (random_range (2)) {
	if ((m->id != INCUBUS) &&	// succubi don't give fear
	    los_p (m->x, m->y, Player.x, Player.y) && (random_range (30) > Player.level + 10) && (Player.status[AFRAID] == 0)) {
	    mprint ("You are stricken with fear!");
	    if (!p_immune (FEAR))
		Player.status[AFRAID] += m->level;
	    else
		mprint ("You master your reptile brain and stand fast.");
	} else
	    m_sp_spell (m);
    }
    if ((m->hp < (m->level * 5)) && (m->hp > 1)) {
	mprint ("The demon uses its waning lifeforce to summon help!");
	m->hp = 1;
	static const uint8_t _demons[] = {
	    NIGHT_GAUNT, NIGHT_GAUNT, NIGHT_GAUNT, NIGHT_GAUNT, L_FDEMON,
	    L_FDEMON, FROST_DEMON, OUTER_DEMON, DEMON_SERP, INNER_DEMON
	};
	int mid = _demons[min(ArraySize(_demons),(unsigned)m->level)];
	summon (-1, mid);
	summon (-1, mid);
    }
}

static void m_sp_acid_cloud (struct monster *m)
{
    if (m_statusp (m, HOSTILE) && (distance (m->x, m->y, Player.x, Player.y) < 3))
	acid_cloud();
}

static void m_sp_escape (struct monster *m)
{
    if (m_statusp (m, HOSTILE))
	m_vanish (m);
}

static void m_sp_ghost (struct monster *m)
{
    if (m_statusp (m, HOSTILE)) {
	mprint ("The ghost moans horribly....");
	p_damage (1, FEAR, "a ghost-inspired heart attack");
	mprint ("You've been terrorized!");
	if (!p_immune (FEAR))
	    Player.status[AFRAID] += m->level;
	else
	    mprint ("You master your reptile brain and stand fast.");
    }
}

// random spell cast by monster
static void m_sp_spell (struct monster *m)
{
    if (m_statusp (m, HOSTILE) && los_p (Player.x, Player.y, m->x, m->y)) {
	mprintf ("%s casts a spell...", m->name());
	if (!magic_resist (m->level))
	    switch (random_range (m->level + 7)) {
		case 0:
		    nbolt (m->x, m->y, Player.x, Player.y, m->hit, 10);
		    break;
		case 1:
		    mprint ("It seems stronger...");
		    m->hp += random_range (m->level * m->level);
		    break;
		case 2:
		    haste (-1);
		    break;
		case 3:
		    cure (-1);
		    break;
		case 4:
		    // WDT: I'd like to make this (and "case 5" below) dependant on
		    // the monster's IQ in some way -- dumb but powerful monsters
		    // deserve what they get :).  No rush.
		    if (m_immunityp (m, ELECTRICITY) || distance (m->x, m->y, Player.x, Player.y) > 2)
			lball (m->x, m->y, Player.x, Player.y, 20);
		    else
			lbolt (m->x, m->y, Player.x, Player.y, m->hit, 20);
		    break;
		case 5:
		    if (m_immunityp (m, COLD) || distance (m->x, m->y, Player.x, Player.y) > 2)
			snowball (m->x, m->y, Player.x, Player.y, 30);
		    else
			icebolt (m->x, m->y, Player.x, Player.y, m->hit, 30);
		    break;
		case 6:
		    enchant (-1);
		    break;
		case 7:
		    bless (0 - m->level);
		    break;
		case 8:
		    p_poison (m->level);
		    break;
		case 9:
		    sleep_player (m->level / 2);
		    break;
		case 10:
		    fbolt (m->x, m->y, Player.x, Player.y, m->hit * 3, 50);
		    break;
		case 11:
		    acquire (0 - m->level);
		    break;
		case 12:
		    dispel (-1);
		    break;
		case 13:
		    disrupt (Player.x, Player.y, 50);
		    break;
		case 14:
		    level_drain (m->level, m->by_name());
		    break;
		case 15:
		case 16:
		    disintegrate (Player.x, Player.y);
		    break;
	    }
    }
}

// monsters with this have some way to hide, camouflage, etc until they attack
static void m_sp_surprise (struct monster *m)
{
    if (!m->attacked)
	return;
    if (m_statusp (m, HOSTILE) && !Player.status[TRUESIGHT] && m_statusp (m, M_INVISIBLE)) {
	m->monchar = Monsters[m->id].monchar;
	if (!Player.status[ALERT]) {
	    static const char _surprize[] =
		"You are surprised by a sudden treacherous attack!\0"
		"You are shocked out of your reverie by the scream of battle!\0"
		"Suddenly, from out of the shadows, a surprise attack!\0"
		"A shriek of hatred causes you to momentarily freeze up!";
	    mprint (zstrn(_surprize,random_range(4),4));
	    morewait();
	    setgamestatus (SKIP_PLAYER);
	    m_status_reset (m, M_INVISIBLE);
	} else {
	    mprint ("You alertly sense the presence of an attacker!");
	    m_status_reset (m, M_INVISIBLE);
	}
    }
}

static void m_sp_whistleblower (struct monster *m)
{
    if (m_statusp (m, HOSTILE)) {
	alert_guards();
	m->specialf = M_MELEE_NORMAL;
    }
}

static void m_sp_seductor (struct monster *m)
{
    if (m_statusp (m, HOSTILE)) {
	mprintf ("%s runs away screaming for help....", m->name());
	m_vanish (m);
	summon (-1, -1);
	summon (-1, -1);
	summon (-1, -1);
    } else if (distance (Player.x, Player.y, m->x, m->y) < 2)
	m_talk_seductor (m);

}

static void m_sp_demonlover (struct monster *m)
{
    if (distance (Player.x, Player.y, m->x, m->y) < 2)
	m_talk_demonlover (m);
}

static void m_sp_eater (struct monster *m)
{
    if (Player.rank[COLLEGE])
	m_status_set (m, HOSTILE);
    if (m_statusp (m, HOSTILE))
	if (los_p (m->x, m->y, Player.x, Player.y)) {
	    mprint ("A strange numbing sensation comes over you...");
	    morewait();
	    Player.mana = Player.mana / 2;
	    if (random_range (4))
		enchant (-1);
	    else
		dispel (-1);
	    Player.pow--;
	    if (--Player.pow < 1)
		p_death ("the Eater of Magic");
	}
    if (m->hp < 10) {
	mprint ("The Eater explodes in a burst of mana!");
	manastorm (m->x, m->y, 1000);
    }
}

static void m_sp_dragonlord (struct monster *m)
{
    if (m_statusp (m, HOSTILE)) {
	if (distance (m->x, m->y, Player.x, Player.y) < 2) {
	    if (!Player.status[IMMOBILE]) {
		mprint ("A gust of wind from the Dragonlord's wings knocks you down!");
		p_damage (25, NORMAL_DAMAGE, "a gust of wind");
		setgamestatus (SKIP_PLAYER);
		Player.status[IMMOBILE] += 2;
	    } else if (!Constriction) {
		mprint ("The Dragonlord grabs you with his tail!");
		Constriction = 25;
		Player.status[IMMOBILE] += 1;
	    } else if (random_range (2)) {
		mprint ("The coils squeeze tighter and tighter...");
		p_damage (Constriction, NORMAL_DAMAGE, "the Dragonlord");
		Player.status[IMMOBILE] += 1;
		Constriction *= 2;
	    } else {
		mprint ("The Dragonlord hurls you to the ground!");
		p_damage (2 * Constriction, NORMAL_DAMAGE, "the Dragonlord");
		Constriction = 0;
	    }
	    m_sp_spell (m);
	} else {
	    Constriction = 0;
	    if (view_los_p (m->x, m->y, Player.x, Player.y)) {
		if ((!Player.immunity[FEAR]) && (!Player.status[AFRAID])) {
		    mprint ("You are awestruck at the sight of the Dragonlord.");
		    Player.status[AFRAID] += 5;
		}
		if (random_range (3)) {
		    m_sp_spell (m);
		    m_sp_spell (m);
		}
	    }
	}
    } else if (distance (m->x, m->y, Player.x, Player.y) < 2)
	mprint ("You are extremely impressed at the sight of the Dragonlord.");
}

static void m_sp_blackout (struct monster *m)
{
    if ((distance (m->x, m->y, Player.x, Player.y) < 4) && (Player.status[BLINDED] == 0)) {
	mprint ("The fungus emits a burst of black spores. You've been blinded!");
	if (Player.status[TRUESIGHT] > 0)
	    mprint ("The blindness quickly passes.");
	else
	    Player.status[BLINDED] += 4;
    }
    if (loc_statusp (m->x, m->y, LIT)) {
	mprint ("The fungus chirps.... ");
	mprint ("The area is plunged into darkness.");
	torch_check();
	torch_check();
	torch_check();
	torch_check();
	torch_check();
	torch_check();
	spreadroomdark (m->x, m->y, Level->site(m->x,m->y).roomnumber);
	levelrefresh();
    }
}

static void m_sp_bogthing (struct monster *m)
{
    if (Player.status[IMMOBILE] && (distance (Player.x, Player.y, m->x, m->y) < 2)) {
	if (!Player.status[AFRAID]) {
	    mprint ("As the bogthing touches you, you feel a frisson of terror....");
	    if (Player.immunity[FEAR])
		mprint ("which you shake off.");
	    else
		Player.status[AFRAID] += 2;
	} else {
	    mprint ("The bogthing's touch causes you scream in agony!");
	    p_damage (50, UNSTOPPABLE, "fright");
	    mprint ("Your struggles grow steadily weaker....");
	    if (--Player.con < 3 || --Player.str < 3)
		p_death ("congestive heart failure");
	}
    }
}

static void m_sp_were (struct monster *m)
{
    if (!m_statusp(m, HOSTILE) && Phase != 6)
	return;
    int mid;
    do
	mid = random_range (ML9 - NML_0) + ML1;
    // log npc, 0th level npc, high score npc or were-creature
    while (mid == NPC || mid == ZERO_NPC || mid == HISCORE_NPC || mid == WEREHUMAN || (Monsters[mid].uniqueness != COMMON) || (!m_statusp (&(Monsters[mid]), MOBILE)) || (!m_statusp (&(Monsters[mid]), HOSTILE)));
    m->id = Monsters[mid].id;
    m->hp += Monsters[mid].hp;
    m->status |= Monsters[mid].status;
    m->ac = Monsters[mid].ac;
    m->dmg = Monsters[mid].dmg;
    m->speed = Monsters[mid].speed;
    m->immunity |= Monsters[mid].immunity;
    m->xpv += Monsters[mid].xpv;
    m->corpseweight = Monsters[mid].corpseweight;
    m->monchar = Monsters[mid].monchar;
    m->talkf = Monsters[mid].talkf;
    m->meleef = Monsters[mid].meleef;
    m->strikef = Monsters[mid].strikef;
    m->specialf = Monsters[mid].specialf;
    strcpy (Str1, "were-");
    strcat (Str1, Monsters[mid].monstring);
    strcpy (Str2, "dead were-");
    strcat (Str2, Monsters[mid].monstring);
    m->monstring = strdup (Str1);
    m->corpsestr = strdup (Str2);
    m->immunity += pow2 (NORMAL_DAMAGE);
    if (los_p (m->x, m->y, Player.x, Player.y))
	mprint ("You witness a hideous transformation!");
    else
	mprint ("You hear a distant howl.");
}

static void m_sp_servant (struct monster *m)
{
    if ((m->id == SERV_LAW) && (Player.alignment < 0))
	m_status_set (m, HOSTILE);
    else if ((m->id == SERV_CHAOS) && (Player.alignment > 0))
	m_status_set (m, HOSTILE);
}

static void m_sp_av (struct monster *m)
{
    if (Player.mana > 0) {
	mprint ("You feel a sudden loss of mana!");
	Player.mana -= (max (0, 10 - distance (m->x, m->y, Player.x, Player.y)));
	dataprint();
    }
}

static void m_sp_lw (struct monster *m)
{
    if (random_range (2)) {
	if (Level->site(m->x,m->y).locchar == FLOOR) {
	    Level->site(m->x,m->y).locchar = LAVA;
	    Level->site(m->x,m->y).p_locf = L_LAVA;
	    lset (m->x, m->y, CHANGED);
	} else if (Level->site(m->x,m->y).locchar == WATER) {
	    Level->site(m->x,m->y).locchar = FLOOR;
	    Level->site(m->x,m->y).p_locf = L_NO_OP;
	    lset (m->x, m->y, CHANGED);
	}
    }
}

static void m_sp_angel (struct monster *m)
{
    int hostile = FALSE;
    switch (m->aux1) {
	case ATHENA:
	case ODIN:
	    hostile = (Player.patron == HECATE || Player.patron == SET);
	    break;
	case SET:
	case HECATE:
	    hostile = (Player.patron == ODIN || Player.patron == ATHENA);
	    break;
	case DESTINY:
	    hostile = (Player.patron != DESTINY);
	    break;
    }
    if (hostile)
	m_status_set (m, HOSTILE);
    if (m_statusp (m, HOSTILE)) {
	mprint ("The angel summons a heavenly host!");
	int mid = (m->level > 8 ? HIGH_ANGEL : (m->level > 7 ? ANGEL : PHANTOM));
	summon (-1, mid);
	summon (-1, mid);
	summon (-1, mid);
	m->specialf = M_NO_OP;	// prevent angel from summoning infinitely
    }
}

// Could completely fill up level
static void m_sp_swarm (struct monster *m)
{
    if (!random_range(5)) {
	if (view_los_p (m->x, m->y, Player.x, Player.y))
	    mprint ("The swarm expands!");
	else
	    mprint ("You hear an aggravating humming noise.");
	summon (-1, SWARM);
    }
}

// raise nearby corpses from the dead....
static void m_sp_raise (struct monster *m)
{
    foreach (i, Level->things) {
	if (abs_distance (i->x, m->x) <= 2 && abs_distance(i->y, m->y) <= 2 && i->id == CORPSEID) {
	    mprint ("The Zombie Overlord makes a mystical gesture...");
	    summon (-1, i->charge);
	    --(i = Level->things.erase(i));
	}
    }
}

static void m_sp_mb (struct monster *m)
{
    if (distance (m->x, m->y, Player.x, Player.y) == 1) {
	mprint ("The manaburst explodes!");
	if (m_statusp (m, HOSTILE)) {
	    mprint ("You get blasted!");
	    p_damage (random_range (100), UNSTOPPABLE, "a manaburst");
	    mprint ("You feel cold all over!");
	    Player.pow -= 3;
	    Player.iq--;
	    Player.con--;
	    Player.str -= 2;
	    Player.dex--;
	    Player.agi--;
	    dispel (-1);
	} else {
	    mprint ("You feel toasty warm inside!");
	    Player.pow++;
	    Player.mana = max (Player.mana, calcmana());
	    Player.hp = max (Player.hp, ++Player.maxhp);
	}
	m->hp = 0;
    }
}

static void m_sp_mirror (struct monster *m)
{
    int i, x, y;
    if (view_los_p (m->x, m->y, Player.x, Player.y)) {
	if (random_range (20) + 6 < m->level) {
	    summon (-1, m->id);
	    mprint ("You hear the sound of a mirror shattering!");
	} else {
	    for (i = 0; i < 5; i++) {
		x = m->x + random_range (13) - 6;
		y = m->y + random_range (13) - 6;
		if (inbounds (x, y)) {
		    Level->site(x,y).showchar = m->monchar;
		    putspot (x, y, m->monchar);
		}
	    }
	}
    }
}

static void m_illusion (struct monster *m)
{
    int i = random_range (NUMMONSTERS);
    if (i == NPC || i == HISCORE_NPC || i == ZERO_NPC)
	i = m->id;		// can't imitate NPC
    if (Player.status[TRUESIGHT]) {
	m->monchar = Monsters[m->id].monchar;
	m->monstring = Monsters[m->id].monstring;
    } else if (random_range (5) == 1) {
	m->monchar = Monsters[i].monchar;
	m->monstring = Monsters[i].monstring;
    }
}

static void m_huge_sounds (struct monster *m)
{
    if (m_statusp (m, AWAKE) && (!los_p (m->x, m->y, Player.x, Player.y)) && (random_range (10) == 1))
	mprint ("The dungeon shakes!");
}

static void m_thief_f (struct monster *m)
{
    int i = random_item();
    if (random_range(3) || distance (Player.x, Player.y, m->x, m->y) > 1)
	return;
    if (p_immune (THEFT) || Player.level > m->level*2 + random_range(20)) {
	mprint ("You feel secure.");
	return;
    }
    if (i == ABORT)
	mprint ("You feel fortunate.");
    else if (Player.possessions[i].used || Player.dex < m->level * random_range(10))
	mprint ("You feel a sharp tug.... You hold on!");
    else {
	mprint ("You feel uneasy for a moment.");
	mprintf ("%s suddenly runs away for some reason.", m->name());
	m_teleport (m);
	m->movef = M_MOVE_SCAREDY;
	m->specialf = M_MOVE_SCAREDY;
	m->pickup (Player.possessions[i]);
	Player.remove_possession (i);
    }
}

static void m_summon (struct monster *m)
{
    if (distance (Player.x, Player.y, m->x, m->y) < 2 && !random_range(3)) {
	summon (0, -1);
	summon (0, -1);
    }
}

static void m_aggravate (struct monster *m)
{
    if (m_statusp (m, HOSTILE)) {
	mprintf ("%s emits an irritating humming sound", m->name());
	aggravate();
	m_status_reset (m, HOSTILE);
    }
}

static void m_sp_merchant (struct monster *m)
{
    if (!m_statusp (m, HOSTILE) || Current_Environment != E_VILLAGE)
	return;
    mprint ("The merchant screams: 'Help! Murder! Guards! Help!'");
    mprint ("You hear the sound of police whistles and running feet.");
    foreach (g, Level->mlist) {
	m_status_set (*g, AWAKE);
	m_status_set (*g, HOSTILE);
    }
    m->specialf = M_NO_OP;
}

// The special function of the various people in the court of the archmage
// and the sorcerors' circle
static void m_sp_court (struct monster *m)
{
    if (!m_statusp (m, HOSTILE))
	return;
    mprint ("A storm of spells hits you!");
    foreach (sm, Level->mlist) {
	m_status_set (*sm, HOSTILE);
	m_sp_spell (sm);
	if (sm->specialf == M_SP_COURT)
	    sm->specialf = M_SP_SPELL;
    }
}

// The special function of the dragons in the dragons' lair
static void m_sp_lair (struct monster *m)
{
    if (!m_statusp (m, HOSTILE))
	return;
    mprint ("You notice a number of dragons waking up....");
    mprint ("You are struck by a quantity of firebolts.");
    morewait();
    foreach (d, Level->mlist) {
	if (d->hp > 0 && d->specialf == M_SP_LAIR) {
	    m_status_set (*d, HOSTILE);
	    fbolt (d->x, d->y, Player.x, Player.y, 100, 100);
	    if (d->id == DRAGON_LORD)
		d->specialf = M_SP_DRAGONLORD;
	    else
		d->specialf = M_STRIKE_FBOLT;
	}
    }
}

static void m_sp_prime (struct monster *m)
{
    if (m_statusp (m, HOSTILE)) {
	mprint ("The prime sorceror gestures and a pentacular gate opens!");
	mprint ("You are surrounded by demons!");
	summon (-1, DEMON_PRINCE);
	summon (-1, DEMON_PRINCE);
	summon (-1, DEMON_PRINCE);
	summon (-1, DEMON_PRINCE);
    }
    m->specialf = M_SP_SPELL;
}

void m_damage (struct monster *m, int dmg, int dtype)
{
    m_status_set (m, AWAKE);
    m_status_set (m, HOSTILE);
    if (m_immunityp (m, dtype)) {
	if (los_p (Player.x, Player.y, m->x, m->y))
	    mprintf ("%s ignores the attack!", m->name());
    } else if ((m->hp -= dmg) < 1)
	m_death (m);
}

void m_death (struct monster *m)
{
    m->hp = -1;
    if (los_p (Player.x, Player.y, m->x, m->y)) {
	gain_experience (m->xpv);
	calc_melee();
	mprintf ("%s is dead! ", m->name());
    }
    m_dropstuff (m);
    if (m->id == DEATH) {
	mprint ("Death lies sprawled out on the ground......");
	mprint ("Death laughs ironically and gets back to its feet.");
	mprint ("It gestures and another scythe appears in its hands.");
	static const char _deathTaunts[] =
	    "Death performs a little bow and goes back on guard.\0"
	    "'A hit! A palpable hit!' Death goes back on the attack.\0"
	    "'Ah, if only it could be so simple!' snickers Death.\0"
	    "'You think Death can be slain?  What a jest!' says Death.\0"
	    "'Your point is well taken.' says Death, attacking again.\0"
	    "'Oh, come now, stop delaying the inevitable.' says Death.\0"
	    "'Your destiny ends here with me.' says Death, scythe raised.\0"
	    "'I almost felt that.' says Death, smiling.\0"
	    "'Timeo Mortis?' asks Death quizzically, 'Not me!'\0"
	    "Death sighs theatrically. 'They never learn.'";
	mprint (zstrn (_deathTaunts, random_range(10), 10));
	strengthen_death (m);
    } else {
	if (Current_Environment == E_ARENA && Level->mlist.size() <= 1)
	    Arena_Victory = m->level+1;	// won this round of arena combat
	if (random_range(2) || m->uniqueness != COMMON)
	    drop_at (m->x, m->y, make_corpse(*m));
	plotspot (m->x, m->y, FALSE);
	switch (m->id) {
	    case HISCORE_NPC:
		switch (m->aux2) {
		    case NPC_HIGHSCORE:
			mprint ("You hear a faroff dirge. You feel a sense of triumph.");
			break;
		    case NPC_HIGHPRIEST_ODIN:
		    case NPC_HIGHPRIEST_SET:
		    case NPC_HIGHPRIEST_ATHENA:
		    case NPC_HIGHPRIEST_HECATE:
		    case NPC_HIGHPRIEST_DRUID:
		    case NPC_HIGHPRIEST_DESTINY:
			mprint ("You hear a faroff sound like angels crying....");
			break;
		    case NPC_SHADOWLORD:
			mprint ("A furtive figure dashes out of the shadows, takes a look at");
			mprint ("the corpse, and runs away!");
			break;
		    case NPC_COMMANDANT:
			mprint ("An aide-de-camp approaches, removes the corpse's insignia,");
			mprint ("and departs.");
			break;
		    case NPC_ARCHMAGE:
			mprint ("An odd glow surrounds the corpse, and slowly fades.");
			break;
		    case NPC_PRIME:
			mprint ("A demon materializes, takes a quick look at the corpse,");
			mprint ("and teleports away with a faint popping noise.");
			break;
		    case NPC_CHAMPION:
			mprint ("A sports columnist rushes forward and takes a quick photo");
			mprint ("of the corpse and rushes off muttering about a deadline.");
			break;
		    case NPC_DUKE:
			mprint ("You hear a fanfare in the distance, and feel dismayed.");
			break;
		    case NPC_LORD_OF_CHAOS:
			if (Player.alignment > 10)
			    mprint ("You feel smug.");
			else if (Player.alignment < 10)
			    mprint ("You feel ashamed.");
			break;
		    case NPC_LORD_OF_LAW:
			if (Player.alignment < 10)
			    mprint ("You feel smug.");
			else if (Player.alignment > 10)
			    mprint ("You feel ashamed.");
			break;
		    case NPC_JUSTICIAR:
			// just a tad complicated. Promote a new justiciar if any
			// guards are left in the city, otherwise Destroy the Order!
			Player.alignment -= 100;
			if (gamestatusp (DESTROYED_ORDER)) {
			    mprint ("A Servant of Chaos materializes, grabs the corpse,");
			    mprint ("snickers a bit, and vanishes.");
			} else {
			    object* ibadge = NULL;
			    foreach (i, Level->things)
				if (i->id == THING_JUSTICIAR_BADGE)
				    ibadge = i;
			    mprint ("In the distance you hear a trumpet. A Servant of Law");
			    // promote one of the city guards to be justiciar
			    monster* guard = NULL;
			    foreach (g, City->mlist)
				if (g->id == GUARD && g->hp > 0)
				    guard = g;
			    if (!guard) {
				mprint ("materializes, sheds a tear, and leaves.");
				morewait();
			    } else {
				if (!ibadge)
				    mprint ("materializes, sheds a tear, and leaves.");
				else {
				    mprint ("materializes, sheds a tear, picks up the badge, and leaves.");
				    guard->pickup (*ibadge);
				    Level->things.erase (ibadge);
				}
				mprint ("A new justiciar has been promoted!");
				make_hiscore_npc (*guard, NPC_JUSTICIAR);
				guard->click = (Tick + 1) % 60;
				m_status_reset (*guard, AWAKE);
				m_status_reset (*guard, HOSTILE);
			    }
			    alert_guards(); // will cause order to be destroyed if no guards or justiciar
			}
			break;
		}
		break;
	    case GUARD:
		Player.alignment -= 10;
		if (Current_Environment == E_CITY || Current_Environment == E_VILLAGE)
		    alert_guards();
		break;
	    case GOBLIN_KING:
		if (gamestatusp (SPOKE_TO_ORACLE)) {
		    mprint ("You seem to hear a woman's voice from far off:");
		    mprint ("'Well done! Come to me now....'");
		}
		setgamestatus (COMPLETED_CAVES);
		break;
	    case GREAT_WYRM:
		if (gamestatusp (SPOKE_TO_ORACLE)) {
		    mprint ("A female voice sounds from just behind your ear:");
		    mprint ("'Well fought! I have some new advice for you....'");
		}
		setgamestatus (COMPLETED_SEWERS);
		break;
	    case EATER:
		setgamestatus (KILLED_EATER);
		break;
	    case LAWBRINGER:
		setgamestatus (KILLED_LAWBRINGER);
		break;
	    case DRAGON_LORD:
		setgamestatus (KILLED_DRAGONLORD);
		break;
	    case DEMON_EMP:
		setgamestatus (COMPLETED_VOLCANO);
		if (gamestatusp (SPOKE_TO_ORACLE)) {
		    mprint ("You feel a soft touch on your shoulder...");
		    mprint ("You turn around but there is no one there!");
		    mprint ("You turn back and see a note: 'See me soon.'");
		    mprint ("The note vanishes in a burst of blue fire!");
		}
		break;
	    case ELEM_MASTER:
		if (gamestatusp (SPOKE_TO_ORACLE)) {
		    mprint ("Words appear before you, traced in blue flame!");
		    mprint ("'Return to the Prime Plane via the Circle of Sorcerors....'");
		}
		break;
	}
	switch (m->specialf) {
	    case M_SP_COURT:
	    case M_SP_LAIR:
		m_status_set (m, HOSTILE);
		monster_action (m, m->specialf);
	}
	Level->mlist.erase (m);
    }
}

static void monster_move (struct monster *m)
{
    monster_action (m, m->movef);
}

static void monster_strike (struct monster *m)
{
    if (player_on_sanctuary())
	print1 ("The aegis of your deity protects you!");
    else {
	// It's lawful to wait to be attacked
	if (m->attacked == 0)
	    Player.alignment++;
	m->attacked++;
	monster_action (m, m->strikef);
    }
}

static void monster_special (struct monster *m)
{
    // since many special functions are really attacks, cancel them all if on sanctuary
    if (!player_on_sanctuary())
	monster_action (m, m->specialf);
}

void monster_talk (struct monster *m)
{
    monster_action (m, m->talkf);
}

static void monster_action (struct monster *m, int action)
{
    int meleef;
    if (action >= M_MELEE_NORMAL && action < M_MOVE_NORMAL) {
	// kluge allows multiple attack forms
	if (distance (m->x, m->y, Player.x, Player.y) < 2) {
	    meleef = m->meleef;
	    m->meleef = action;
	    tacmonster (m);
	    m->meleef = meleef;
	}
	return;
    }
    switch (action) {
	case M_NO_OP:		m_no_op (m);		break;
	case M_MOVE_NORMAL:	m_normal_move (m);	break;
	case M_MOVE_FLUTTER:	m_flutter_move (m);	break;
	case M_MOVE_FOLLOW:	m_follow_move (m);	break;
	case M_MOVE_TELEPORT:	m_teleport (m);		break;
	case M_MOVE_RANDOM:	m_random_move (m);	break;
	case M_MOVE_SMART:	m_smart_move (m);	break;
	case M_MOVE_SPIRIT:	m_spirit_move (m);	break;
	case M_MOVE_CONFUSED:	m_confused_move (m);	break;
	case M_MOVE_SCAREDY:	m_scaredy_move (m);	break;
	case M_MOVE_ANIMAL:	m_move_animal (m);	break;
	case M_MOVE_LEASH:	m_move_leash (m);	break;
	case M_STRIKE_MISSILE:	m_nbolt (m);		break;
	case M_STRIKE_FBOLT:	m_firebolt (m);		break;
	case M_STRIKE_LBALL:	m_lball (m);		break;
	case M_STRIKE_FBALL:	m_fireball (m);		break;
	case M_STRIKE_SNOWBALL:	m_snowball (m);		break;
	case M_STRIKE_BLIND:	m_blind_strike (m);	break;
	case M_STRIKE_SONIC:	m_strike_sonic (m);	break;
	case M_TALK_HORSE:	m_talk_horse (m);	break;
	case M_TALK_THIEF:	m_talk_thief (m);	break;
	case M_TALK_STUPID:	m_talk_stupid (m);	break;
	case M_TALK_SILENT:	m_talk_silent (m);	break;
	case M_TALK_HUNGRY:	m_talk_hungry (m);	break;
	case M_TALK_GREEDY:	m_talk_greedy (m);	break;
	case M_TALK_TITTER:	m_talk_titter (m);	break;
	case M_TALK_MP:		m_talk_mp (m);		break;
	case M_TALK_IM:		m_talk_im (m);		break;
	case M_TALK_MAN:	m_talk_man (m);		break;
	case M_TALK_ROBOT:	m_talk_robot (m);	break;
	case M_TALK_EVIL:	m_talk_evil (m);	break;
	case M_TALK_GUARD:	m_talk_guard (m);	break;
	case M_TALK_MIMSY:	m_talk_mimsy (m);	break;
	case M_TALK_SLITHY:	m_talk_slithy (m);	break;
	case M_TALK_BURBLE:	m_talk_burble (m);	break;
	case M_TALK_BEG:	m_talk_beg (m);		break;
	case M_TALK_HINT:	m_talk_hint (m);	break;
	case M_TALK_EF:		m_talk_ef (m);		break;
	case M_TALK_GF:		m_talk_gf (m);		break;
	case M_TALK_SEDUCTOR:	m_talk_seductor (m);	break;
	case M_TALK_DEMONLOVER:	m_talk_demonlover (m);	break;
	case M_TALK_NINJA:	m_talk_ninja (m);	break;
	case M_TALK_ASSASSIN:	m_talk_assassin (m);	break;
	case M_TALK_SERVANT:	m_talk_servant (m);	break;
	case M_TALK_ANIMAL:	m_talk_animal (m);	break;
	case M_TALK_SCREAM:	m_talk_scream (m);	break;
	case M_TALK_PARROT:	m_talk_parrot (m);	break;
	case M_TALK_HYENA:	m_talk_hyena (m);	break;
	case M_TALK_DRUID:	m_talk_druid (m);	break;
	case M_TALK_ARCHMAGE:	m_talk_archmage (m);	break;
	case M_TALK_MERCHANT:	m_talk_merchant (m);	break;
	case M_TALK_PRIME:	m_talk_prime (m);	break;
	case M_SP_BOG:		m_sp_bogthing (m);	break;
	case M_SP_WERE:		m_sp_were (m);		break;
	case M_SP_WHISTLEBLOWER:m_sp_whistleblower (m);	break;
	case M_SP_MERCHANT:	m_sp_merchant (m);	break;
	case M_SP_SURPRISE:	m_sp_surprise (m);	break;
	case M_SP_MP:		m_sp_mp (m);		break;
	case M_SP_THIEF:	m_thief_f (m);		break;
	case M_SP_DEMONLOVER:	m_sp_demonlover (m);	break;
	case M_SP_AGGRAVATE:	m_aggravate (m);	break;
	case M_SP_POISON_CLOUD:	m_sp_poison_cloud (m);	break;
	case M_SP_HUGE:		m_huge_sounds (m);	break;
	case M_SP_SUMMON:	m_summon (m);		break;
	case M_SP_ILLUSION:	m_illusion (m);		break;
	case M_SP_ESCAPE:	m_sp_escape (m);	break;
	case M_SP_FLUTTER:	m_flutter_move (m);	break;
	case M_SP_EXPLODE:	m_sp_explode (m);	break;
	case M_SP_DEMON:	m_sp_demon (m);		break;
	case M_SP_ACID_CLOUD:	m_sp_acid_cloud (m);	break;
	case M_SP_GHOST:	m_sp_ghost (m);		break;
	case M_SP_SPELL:	m_sp_spell (m);		break;
	case M_SP_SEDUCTOR:	m_sp_seductor (m);	break;
	case M_SP_EATER:	m_sp_eater (m);		break;
	case M_SP_DRAGONLORD:	m_sp_dragonlord (m);	break;
	case M_SP_BLACKOUT:	m_sp_blackout (m);	break;
	case M_SP_SWARM:	m_sp_swarm (m);		break;
	case M_SP_ANGEL:	m_sp_angel (m);		break;
	case M_SP_SERVANT:	m_sp_servant (m);	break;
	case M_SP_AV:		m_sp_av (m);		break;
	case M_SP_LW:		m_sp_lw (m);		break;
	case M_SP_MB:		m_sp_mb (m);		break;
	case M_SP_RAISE:	m_sp_raise (m);		break;
	case M_SP_MIRROR:	m_sp_mirror (m);	break;
	case M_SP_COURT:	m_sp_court (m);		break;
	case M_SP_LAIR:		m_sp_lair (m);		break;
	case M_SP_PRIME:	m_sp_prime (m);		break;
    }
}

void m_trap_dart (struct monster *m)
{
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	mprintf ("%s was hit by a dart!", m->name());
	Level->site(m->x,m->y).locchar = TRAP;
	lset (m->x, m->y, CHANGED);
    }
    m_damage (m, difficulty() * 2, NORMAL_DAMAGE);
}

void m_trap_pit (struct monster *m)
{
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	mprintf ("%s fell into a pit!", m->name());
	Level->site(m->x,m->y).locchar = TRAP;
	lset (m->x, m->y, CHANGED);
    }
    if (!m_statusp (m, INTANGIBLE))
	m_status_reset (m, MOBILE);
    m_damage (m, difficulty() * 5, NORMAL_DAMAGE);

}

void m_trap_door (struct monster *m)
{
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	mprintf ("%s fell into a trap door!", m->name());
	Level->site(m->x,m->y).locchar = TRAP;
	lset (m->x, m->y, CHANGED);
    }
    m_vanish (m);
}

void m_trap_abyss (struct monster *m)
{
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	mprintf ("%s fell into the infinite abyss!", m->name());
	Level->site(m->x,m->y).locchar = ABYSS;
	lset (m->x, m->y, CHANGED);
	Level->site(m->x,m->y).p_locf = L_ABYSS;
	lset (m->x, m->y, CHANGED);
    }
    setgamestatus (SUPPRESS_PRINTING);
    m_vanish (m);
    resetgamestatus (SUPPRESS_PRINTING);
}

void m_trap_snare (struct monster *m)
{
    Level->site(m->x,m->y).locchar = TRAP;
    lset (m->x, m->y, CHANGED);
    if (los_p (m->x, m->y, Player.x, Player.y))
	mprintf ("%s was caught in a snare!", m->name());
    if (!m_statusp (m, INTANGIBLE))
	m_status_reset (m, MOBILE);
}

void m_trap_blade (struct monster *m)
{
    Level->site(m->x,m->y).locchar = TRAP;
    lset (m->x, m->y, CHANGED);
    if (los_p (m->x, m->y, Player.x, Player.y))
	mprintf ("%s was hit by a blade trap!", m->name());
    m_damage (m, (difficulty() + 1) * 7 - Player.defense, NORMAL_DAMAGE);
}

void m_trap_fire (struct monster *m)
{
    Level->site(m->x,m->y).locchar = TRAP;
    lset (m->x, m->y, CHANGED);
    if (los_p (m->x, m->y, Player.x, Player.y))
	mprintf ("%s was hit by a fire trap!", m->name());
    m_damage (m, (difficulty() + 1) * 5, FLAME);
}

void m_fire (struct monster *m)
{
    if (los_p (m->x, m->y, Player.x, Player.y))
	mprintf ("%s was blasted by fire!", m->name());
    m_damage (m, random_range (100), FLAME);
}

void m_trap_teleport (struct monster *m)
{
    Level->site(m->x,m->y).locchar = TRAP;
    lset (m->x, m->y, CHANGED);
    if (los_p (m->x, m->y, Player.x, Player.y))
	mprintf ("%s walked into a teleport trap!", m->name());
    m_teleport (m);
}

void m_trap_disintegrate (struct monster *m)
{
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	mprintf ("%s walked into a disintegration trap!", m->name());
	Level->site(m->x,m->y).locchar = TRAP;
	lset (m->x, m->y, CHANGED);
    }
    disintegrate (m->x, m->y);
}

void m_trap_sleepgas (struct monster *m)
{
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	mprintf ("%s walked into a sleepgas trap!", m->name());
	Level->site(m->x,m->y).locchar = TRAP;
	lset (m->x, m->y, CHANGED);
    }
    if (!m_immunityp (m, SLEEP))
	m_status_reset (m, AWAKE);
}

void m_trap_acid (struct monster *m)
{
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	mprintf ("%s walked into an acid bath trap!", m->name());
	Level->site(m->x,m->y).locchar = TRAP;
	lset (m->x, m->y, CHANGED);
    }
    m_damage (m, random_range (difficulty() * difficulty()), ACID);
}

void m_trap_manadrain (struct monster *m)
{
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	mprintf ("%s walked into a manadrain trap!", m->name());
	Level->site(m->x,m->y).locchar = TRAP;
	lset (m->x, m->y, CHANGED);
    }
    if (m->specialf == M_SP_SPELL)
	m->specialf = M_NO_OP;
}

void m_water (struct monster *m)
{
    if ((!m_statusp (m, INTANGIBLE)) && (!m_statusp (m, SWIMMING)) && (!m_statusp (m, ONLYSWIM))) {
	if (los_p (m->x, m->y, Player.x, Player.y))
	    mprintf ("%s drowned!", m->name());
	m_death (m);
    }
}

void m_abyss (struct monster *m)
{
    if (los_p (m->x, m->y, Player.x, Player.y))
	mprintf ("%s fell into the infinite abyss!", m->name());
    m_vanish (m);
}

void m_lava (struct monster *m)
{
    if ((!m_immunityp (m, FLAME)) || ((!m_statusp (m, SWIMMING)) && (!m_statusp (m, ONLYSWIM)))) {
	if (los_p (m->x, m->y, Player.x, Player.y))
	    mprintf ("%s died in a pool of lava!", m->name());
	m_death (m);
    }
}

void m_altar (struct monster *m)
{
    int visible = view_los_p (Player.x, Player.y, m->x, m->y);
    int reaction = 0;
    int altar = Level->site(m->x,m->y).aux;

    if (visible)
	mprintf ("%s walks next to an altar...", m->name());
    if (!m_statusp (m, HOSTILE))
	reaction = 0;
    else if (m->id == HISCORE_NPC && m->aux2 == altar)
	reaction = 1;		// high priest of same deity
    else if ((m->id == ANGEL || m->id == HIGH_ANGEL || m->id == ARCHANGEL) && m->aux1 == altar)
	reaction = 1;		// angel of same deity
    else if (altar == Player.patron)
	reaction = -1;		// friendly deity will zap hostile monster
    else if (((Player.patron == ODIN || Player.patron == ATHENA) && (altar == SET || altar == HECATE)) || ((Player.patron == SET || Player.patron == HECATE) && (altar == ODIN || altar == ATHENA)))
	reaction = 1;		// hostile deity will help hostile monster
    switch (reaction) {
	case -1:
	    if (visible) {
		mprint ("Your deity is angry!");
		mprint ("A bolt of godsfire strikes the monster....");
	    }
	    disrupt (m->x, m->y, Player.rank[PRIESTHOOD] * 50);
	    break;
	case 1:
	    if (visible) {
		mprint ("The deity of the altar smiles on the monster....");
		mprint ("A shaft of light zaps the altar...");
	    }
	    m->hp = Monsters[m->id].hp * 2;
	    break;
	default:
	    if (visible)
		mprint ("but nothing much seems to happen");
	    break;
    }
}

const char* mantype (void)
{
    static const char _typestr[] =
	"janitor\0" "beggar\0" "barbarian\0" "hairdresser\0" "accountant\0"
	"lawyer\0" "indian chief\0" "tinker\0" "tailor\0" "soldier\0"
	"spy\0" "doctor\0" "miner\0" "noble\0" "serf\0"
	"neer-do-well\0" "vendor\0" "dilettante\0" "surveyor\0" "jongleur";
    return (zstrn (_typestr, random_range(20), 20));
}

static void strengthen_death (struct monster *m)
{
    m->xpv += min (10000, m->xpv + 1000);
    m->hit += min (1000, m->hit + 10);
    m->dmg = min (10000, m->dmg * 2);
    m->ac += min (1000, m->ac + 10);
    m->speed = max (m->speed - 1, 1);
    m->movef = M_MOVE_SMART;
    m->hp = min (100000, 100 + m->dmg * 10);
    m->possessions.push_back (Objects[WEAPON_SCYTHE_OF_DEATH]);
}

void m_no_op (struct monster *m UNUSED)
{
}

const char* m_melee_str (unsigned level)
{
    static const char SkilledMeleeStr[33] = "L?R?L?R?L?R?L?R?L?R?L?R?L?R?L?R?";
    return (SkilledMeleeStr+strlen(SkilledMeleeStr)-4*min(level,8U));
}
