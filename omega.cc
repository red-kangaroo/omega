#include "glob.h"
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

//----------------------------------------------------------------------

static int game_restore(int argc, const char* argv[]);
static void init_world(void);
static void fix_phantom(struct monster *m);

//----------------------------------------------------------------------

// most globals originate in omega.c

// Objects and Monsters are allocated and initialized in init.c

// one of each spell
struct spell Spells[NUMSPELLS + 1] = {
{/*S_MON_DET*/ 3}, {/*S_OBJ_DET*/ 3}, {/*S_IDENTIFY*/ 10}, {/*S_FIREBOLT*/ 20}, {/*S_SLEEP*/ 15},
{/*S_LBALL*/ 25}, {/*S_TELEPORT*/ 20}, {/*S_DISRUPT*/ 30}, {/*S_DISINTEGRATE*/ 40}, {/*S_MISSILE*/ 10},
{/*S_HEAL*/ 15}, {/*S_DISPEL*/ 40}, {/*S_BREATHE*/ 20}, {/*S_INVISIBLE*/ 15}, {/*S_WARP*/ 50},
{/*S_ENCHANT*/ 30}, {/*S_BLESS*/ 30}, {/*S_RESTORE*/ 20}, {/*S_CURE*/ 20}, {/*S_TRUESIGHT*/ 20},
{/*S_HELLFIRE*/ 90}, {/*S_KNOWLEDGE*/ 10}, {/*S_HERO*/ 20}, {/*S_RETURN*/ 10}, {/*S_DESECRATE*/ 50},
{/*S_HASTE*/ 15}, {/*S_SUMMON*/ 20}, {/*S_SANCTUARY*/ 75}, {/*S_ACCURACY*/ 20}, {/*S_RITUAL*/ 50},
{/*S_APPORT*/ 15}, {/*S_SHADOWFORM*/ 50}, {/*S_ALERT*/ 15}, {/*S_REGENERATE*/ 20}, {/*S_SANCTIFY*/ 75},
{/*S_CLAIRVOYANCE*/ 10}, {/*S_DRAIN*/ 40}, {/*S_LEVITATE*/ 25}, {/*S_POLYMORPH*/ 30}, {/*S_FEAR*/ 10},
{/*S_WISH*/ 100}, {0}};

uint64_t SpellKnown = 0;
uint8_t ObjectAttrs[TOTALITEMS] = {0};

// locations of city sites [0] - found, [1] - x, [2] - y
int CitySiteList[NUMCITYSITES][3];

// Currently defined in caps since it is now a variable, was a constant
int LENGTH = MAXLENGTH;
int WIDTH = MAXWIDTH;

long GameStatus = 0L;		// Game Status bit vector
int ScreenLength = 0;		// How large is level window
struct player Player;		// the player
struct terrain Country[MAXWIDTH][MAXLENGTH];	// The countryside
struct level *City = NULL;	// The city of Rampart
struct level *TempLevel = NULL;	// Place holder
struct level *Level = NULL;	// Pointer to current Level
struct level *Dungeon = NULL;	// Pointer to current Dungeon
int Villagenum = 0;		// Current Village number
int ScreenOffset = 0;		// Offset of displayed screen to level
int MaxDungeonLevels = 0;	// Deepest level allowed in dungeon
int Current_Dungeon = -1;	// What is Dungeon now
int Current_Environment = E_CITY;	// Which environment are we in
int Last_Environment = E_COUNTRYSIDE;	// Which environment were we in
int Dirs[2][9];			// 9 xy directions
char Cmd = 's';			// last player command
int Command_Duration = 0;	// how long does current command take
struct monster *Arena_Monster = NULL;	// Opponent in arena
int Arena_Opponent = 0;		// case label of opponent in l_arena()
int Arena_Victory = 0;		// did player win in arena?
int Imprisonment = 0;		// amount of time spent in jail
int Precipitation = 0;		// Hours of rain, snow, etc
int Lunarity = 0;		// Effect of the moon on character
int Phase = 0;			// Phase of the moon
int Date = 0;			// Starting date
int Pawndate = 0;		// Pawn Shop item generation date
pob Pawnitems[PAWNITEMS] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

// items in pawn shop
int SymbolUseHour = -1;		// holy symbol use marker
int ViewHour = -1;		// crystal ball use marker
int ZapHour = -1;		// staff of enchantment use marker
int HelmHour = -1;		// helm of teleportation use marker
int Constriction = 0;		// Dragonlord Attack State
int Blessing = FALSE;		// Altar Blessing State
int LastDay = -1;		// DPW date of dole
int RitualHour = -1;		// last use of ritual magic
int RitualRoom = -1;		// last room of ritual magic
int Lawstone = 0;		// magic stone counter
int Chaostone = 0;		// magic stone counter
int Mindstone = 0;		// magic stone counter
int Searchnum = 1;		// number of times to search on 's'
int Behavior;			// Player NPC behavior
int Verbosity = VERBOSE;	// verbosity level
long Time = 0;			// turn number
int Tick = 0;			// 10 a turn; action coordinator
char Stringbuffer[STRING_BUFFER_SIZE][80];	// last strings printed
long Gymcredit = 0;		// credit at rampart gym
int Spellsleft = 0;		// research allowance at college
int StarGemUse = 0;		// last date of star gem use
int HiMagicUse = 0;		// last date of high magic use
int HiMagic = 0;		// current level for l_throne
long Balance = 0;		// bank account
long FixedPoints = 0;		// points are frozen after adepthood
int LastTownLocX = 0;		// previous position in village or city
int LastTownLocY = 0;		// previous position in village or city
int LastCountryLocX = 0;	// previous position in countryside
int LastCountryLocY = 0;	// previous position in countryside
char Password[64];		// autoteller password
char Str1[STRING_LEN], Str2[STRING_LEN], Str3[STRING_LEN], Str4[STRING_LEN]; // Some string space, random uses

pol Condoitems = NULL;		// Items in condo

// high score names, levels, behavior
int Shadowlordbehavior, Archmagebehavior, Primebehavior, Commandantbehavior;
int Championbehavior, Priestbehavior[7], Hibehavior, Dukebehavior;
int Chaoslordbehavior, Lawlordbehavior, Justiciarbehavior;
char Shadowlord[80], Archmage[80], Prime[80], Commandant[80], Duke[80];
char Champion[80], Priest[7][80], Hiscorer[80], Hidescrip[80];
char Chaoslord[80], Lawlord[80], Justiciar[80];
int Shadowlordlevel, Archmagelevel, Primelevel, Commandantlevel, Dukelevel;
int Championlevel, Priestlevel[7], Hilevel, Justiciarlevel;
long Hiscore = 0L;
int Chaoslordlevel = 0, Lawlordlevel = 0, Chaos = 0, Law = 0;

// New globals which used to be statics
int twiddle = FALSE;
int saved = FALSE;
int onewithchaos = FALSE;
int club_hinthour = 0;
int winnings = 0;
int tavern_hinthour;

int deepest[E_MAX + 1];
int level_seed[E_MAX + 1];	// random number seed that generated level

static void signalexit (int sig);

// This may be implementation dependent
// environment is the environment about to be generated, or -1 for the first
// time, or -2 if we want to restore the random number point
void initrand (int environment, int level)
{
    static int store;
    int seed;

    if (environment >= 0)
	store = rand();
    // Pseudo Random Seed
    if (environment == E_RANDOM)
	seed = (int) time (NULL);
    else if (environment == E_RESTORE)
	seed = store;
    else
	seed = level_seed[environment] + 1000 * level;
    srand (seed);
}

static int game_restore (int argc, const char* argv[])
{
    char savestr[80];
    int ok;
    if (argc == 2) {
	strcpy (savestr, argv[1]);
	ok = restore_game (savestr);
	if (!ok) {
	    endgraf();
	    printf ("Try again with the right save file, luser!\n");
	    exit (0);
	}
	unlink (savestr);
	return (TRUE);
    } else
	return (FALSE);
}

int main (int argc, const char* argv[])
{
    int continuing;
    int count;

    // always catch ^c and hang-up signals

    signal (SIGINT, (__sighandler_t) quit);
    signal (SIGHUP, signalsave);
    signal (SIGQUIT, signalexit);
    signal (SIGILL, signalexit);
    signal (SIGTRAP, signalexit);
    signal (SIGFPE, signalexit);
    signal (SIGSEGV, signalexit);
    signal (SIGABRT, signalexit);
    signal (SIGBUS, signalexit);
    signal (SIGSYS, signalexit);

    // if filecheck is 0, some necessary data files are missing
    if (filecheck() == 0)
	exit (0);

    // all kinds of initialization
    initgraf();
    initdirs();
    initrand (E_RANDOM, 0);

    for (count = 0; count < STRING_BUFFER_SIZE; count++)
	strcpy (Stringbuffer[count], "<nothing>");

    omega_title();
    showscores();

    // game restore attempts to restore game if there is an argument
    continuing = game_restore (argc, argv);

    // monsters initialized in game_restore if game is being restored
    // items initialized in game_restore if game is being restored
    if (!continuing) {
	Date = random_range (360);
	Phase = random_range (24);
	strcpy (Password, "");
	initplayer();
	init_world();
	mprint ("'?' for help or commandlist, 'Q' to quit.");
    } else
	mprint ("Your adventure continues....");

    timeprint();
    calc_melee();
    if (Current_Environment != E_COUNTRYSIDE)
	showroom (Level->site[Player.x][Player.y].roomnumber);
    else
	terrain_check (FALSE);

    screencheck (Player.y);

    // game cycle
    if (!continuing)
	time_clock (TRUE);
    while (TRUE) {
	if (Current_Environment == E_COUNTRYSIDE)
	    p_country_process();
	else
	    time_clock (FALSE);
    }
}

static void signalexit (int sig UNUSED)
{
    int reply;
    clearmsg();
    mprint ("Yikes!");
    morewait();
    mprint ("Sorry, caught a core-dump signal.");
    mprint ("Want to try and save the game?");
    reply = ynq();
    if (reply == 'y')
	save (FALSE, TRUE);	// don't compress, force save
    else if (reply == EOF)
	signalsave (0);
    mprint ("Bye!");
    endgraf();
    exit (0);
}

// Start up game with new dungeons; start with player in city
static void init_world (void)
{
    int env, i;

    City = Level = TempLevel = Dungeon = NULL;
    for (env = 0; env <= E_MAX; env++)
	level_seed[env] = rand();
    load_country();
    for (i = 0; i < NUMCITYSITES; i++)
	CitySiteList[i][0] = FALSE;
    load_city (TRUE);
    WIDTH = 64;
    LENGTH = 64;
    Player.x = 62;
    Player.y = 21;
    Level = City;
    Current_Environment = E_CITY;
    print1 ("You pass through the massive gates of Rampart, the city.");
}

// This function coordinates monsters and player actions, as well as
// random events. Each tick is a second. There are therefore 60 ticks to
// the minute and 60 minutes to the hour.
void time_clock (int reset)
{
    int env;
    pml ml, *prev;

    if (++Tick > 60) {
	Tick = 0;
	minute_status_check();	// see about some player statuses each minute
	if (++Time % 10 == 0)
	    tenminute_check();
    }

    if (reset)
	Tick = (Player.click = 0);

    env = Current_Environment;
    while ((Tick == Player.click) && (Current_Environment != E_COUNTRYSIDE) && Current_Environment == env) {
	if (!gamestatusp (SKIP_PLAYER))
	    do {
		resetgamestatus (SKIP_MONSTERS);
		if ((!Player.status[SLEPT]) && (Current_Environment != E_COUNTRYSIDE))
		    p_process();
	    } while (gamestatusp (SKIP_MONSTERS) && (Current_Environment != E_COUNTRYSIDE));
	else
	    resetgamestatus (SKIP_PLAYER);
	Player.click = (Player.click + Command_Duration) % 60;
    }

    // klugy but what the heck. w/o this line, if the player caused
    // a change-environment to the country, the monsters on the old Level
    // will still act, causing all kinds of anomalies and core dumps,
    // intermittently. However, any other environment change will reset
    // Level appropriately, so only have to check for countryside

    if (Current_Environment != E_COUNTRYSIDE) {

	prev = &(Level->mlist);
	ml = *prev;
	while (ml)
	    if (ml->m->hp > 0) {
		// following is a hack until I discover source of phantom monsters
		if (Level->site[ml->m->x][ml->m->y].creature != ml->m)
		    fix_phantom (ml->m);
		if (Tick == ml->m->click) {
		    ml->m->click += ml->m->speed;
		    while (ml->m->click > 60)
			ml->m->click -= 60;
		    m_pulse (ml->m);
		}
		prev = &(ml->next);
		ml = ml->next;
	    } else if (ml->m != Arena_Monster) {
		*prev = ml->next;
		delete ml->m;
		delete ml;
		ml = *prev;
	    } else
		ml = ml->next;
    }
}

// remedies occasional defective monsters
static void fix_phantom (struct monster *m)
{
    if (Level->site[m->x][m->y].creature == NULL) {
	mprint ("You hear a sound like a sigh of relief....");
	Level->site[m->x][m->y].creature = m;
    } else {
	mprint ("You hear a puff of displaced air....");
	findspace (&(m->x), &(m->y), -1);
	Level->site[m->x][m->y].creature = m;
	m_death (m);
    }
}
