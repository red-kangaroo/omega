#include "glob.h"
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

//----------------------------------------------------------------------

static void init_world(void);

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
citysite CitySiteList[NUMCITYSITES];

uint32_t GameStatus = 0;	// Game Status bit vector
uint8_t ScreenLength = 0;	// How large is level window
struct player Player;		// the player
struct level* Country = NULL;	// The countryside
struct level* City = NULL;	// The city of Rampart
struct level* TempLevel = NULL;	// Place holder
struct level* Level = NULL;	// Pointer to current Level
struct level* Dungeon = NULL;	// Pointer to current Dungeon
uint8_t Villagenum = 0;		// Current Village number
int8_t ScreenOffset = 0;	// Offset of displayed screen to level
uint8_t MaxDungeonLevels = 0;	// Deepest level allowed in dungeon
int8_t Current_Dungeon = -1;	// What is Dungeon now
EEnvironment Current_Environment = E_CITY;	// Which environment are we in
EEnvironment Last_Environment = E_COUNTRYSIDE;	// Which environment were we in
const int8_t Dirs[2][9] = {	// 9 xy directions
    { 1, 1, -1, -1, 1, -1, 0, 0, 0 },
    { 1, -1, 1, -1, 0, 0, 1, -1, 0 }
};
chtype Cmd = 's';		// last player command
uint16_t Command_Duration = 0;	// how long does current command take
uint8_t Arena_Opponent = 0;	// case label of opponent in l_arena()
uint8_t Arena_Victory = 0;	// did player win in arena?
uint8_t Imprisonment = 0;	// amount of time spent in jail
uint8_t Precipitation = 0;	// Hours of rain, snow, etc
int8_t Lunarity = 0;		// Effect of the moon on character
uint8_t Phase = 0;		// Phase of the moon
uint16_t Date = 0;		// Starting date
uint16_t Pawndate = 0;		// Pawn Shop item generation date
vector<object> Pawnitems;

// items in pawn shop
int8_t SymbolUseHour = -1;		// holy symbol use marker
int8_t ViewHour = -1;		// crystal ball use marker
int8_t ZapHour = -1;		// staff of enchantment use marker
int8_t HelmHour = -1;		// helm of teleportation use marker
uint8_t Constriction = 0;	// Dragonlord Attack State
bool Blessing = false;		// Altar Blessing State
int16_t LastDay = -1;		// DPW date of dole
int8_t RitualHour = -1;		// last use of ritual magic
int8_t RitualRoom = -1;		// last room of ritual magic
uint8_t Lawstone = 0;		// magic stone counter
uint8_t Chaostone = 0;		// magic stone counter
uint8_t Mindstone = 0;		// magic stone counter
uint8_t Searchnum = 1;		// number of times to search on 's'
EVerbosity Verbosity = VERBOSE;	// verbosity level
uint32_t Time = 0;		// turn number
uint8_t Tick = 0;		// 10 a turn; action coordinator
char Stringbuffer[STRING_BUFFER_SIZE][80];	// last strings printed
uint32_t Gymcredit = 0;		// credit at rampart gym
uint8_t Spellsleft = 0;		// research allowance at college
uint16_t StarGemUse = 0;	// last date of star gem use
uint16_t HiMagicUse = 0;	// last date of high magic use
uint8_t HiMagic = 0;		// current level for l_throne
uint32_t Balance = 0;		// bank account
uint32_t FixedPoints = 0;	// points are frozen after adepthood
uint8_t LastTownLocX = 0;	// previous position in village or city
uint8_t LastTownLocY = 0;	// previous position in village or city
uint8_t LastCountryLocX = 0;	// previous position in countryside
uint8_t LastCountryLocY = 0;	// previous position in countryside
char Password[8];		// autoteller password
char Str1[STRING_LEN], Str2[STRING_LEN], Str3[STRING_LEN], Str4[STRING_LEN]; // Some string space, random uses

vector<object> Condoitems;	// Items in condo

// high score names, levels, behavior
const char Archmage[] = "Oz";
const char Champion[] = "Spartacus";
const char Chaoslord[] = "Arioch";
const char Commandant[] = "Sonders";
const char Duke[] = "Cormorant";
const char Hidescrip[] = "Killed by a pun";
const char Hiscorer[] = "Zippy";
const char Justiciar[] = "Morgon";
const char Lawlord[] = "Ariel";
const char Priest[NUMRELIGIONS][12] = { "", "Gallow", "Kharst", "Allana", "Dara", "Thistleman", "Humperdinck" };
const uint16_t Priestbehavior[NUMRELIGIONS] = { 0, 2711, 2712, 2931, 2932, 2933, 5718 };
const uint8_t Priestlevel[NUMRELIGIONS] = { 0, 10, 10, 10, 10, 10, 10 };
const char Prime[] = "Blackskull";
const char Shadowlord[] = "Shadowspawn";

// New globals which used to be statics
bool twiddle = false;
bool saved = false;
bool onewithchaos = false;
uint8_t club_hinthour = 0;
uint16_t winnings = 0;
uint8_t tavern_hinthour;

uint8_t deepest [E_MAX + 1];
uint32_t level_seed [E_MAX + 1];	// random number seed that generated level

static void signalexit (int sig);

void initrand (int environment, int level)
{
    if (environment == E_RESTORE)
	srandrand();
    else if (environment >= 0)
	srand (level_seed[environment] + level);
}

int main (void)
{
    // always catch ^c and other fatal signals
    #define S(s) __sigmask(s)
    constexpr sigset_t sigset_Quit =
	S(SIGHUP)|S(SIGINT)|S(SIGPIPE)|S(SIGQUIT)|S(SIGTERM)|
	S(SIGPWR)|S(SIGILL)|S(SIGABRT)|S(SIGBUS)|S(SIGFPE)|
	S(SIGSYS)|S(SIGSEGV)|S(SIGALRM)|S(SIGXCPU);
    #undef S
    for (uint32_t i = NSIG; --i;)
	if (sigismember(&sigset_Quit,i))
	    signal (i, signalexit);

    // all kinds of initialization
    srandrand();
    initgraf();

    for (unsigned count = 0; count < STRING_BUFFER_SIZE; count++)
	strcpy (Stringbuffer[count], "<nothing>");

    // Try to restore game, if any
    bool continuing = restore_game();

    // monsters initialized in restore_game if game is being restored
    // items initialized in restore_game if game is being restored
    if (!continuing) {
	omega_title();
	Date = random_range (360);
	Phase = random_range (24);
	Password[0] = 0;
	initplayer();
	init_world();
	mprint ("'?' for help or commandlist, 'Q' to quit.");
    } else
	mprint ("Your adventure continues....");

    timeprint();
    calc_melee();
    if (Current_Environment != E_COUNTRYSIDE)
	showroom (Level->site(Player.x,Player.y).roomnumber);
    else
	terrain_check (false);

    screencheck (Player.y);

    // game cycle
    if (!continuing)
	time_clock (true);
    while (true) {
	if (Current_Environment == E_COUNTRYSIDE)
	    p_country_process();
	else
	    time_clock (false);
    }
}

static void signalexit (int sig)
{
    if (sig == SIGINT)
	return (quit());
    endgraf();
    psignal (sig, "[S] Fatal error");
    exit (-(char)sig);
}

// Start up game with new dungeons; start with player in city
static void init_world (void)
{
    TempLevel = Dungeon = NULL;
    for (int env = 0; env <= E_MAX; env++)
	level_seed[env] = rand();
    Level = Country = new level;
    clear_level (Country);
    load_country();
    for (unsigned i = 0; i < ArraySize(CitySiteList); i++)
	CitySiteList[i].known = false;
    for (unsigned i = 0; i < ArraySize(ObjectAttrs); ++i)
	ObjectAttrs[i] = Objects[i].uniqueness;
    Level = City = new level;
    clear_level (Level);
    load_city();
    Player.x = Level->lastx;
    Player.y = Level->lasty;
    Level = City;
    Current_Environment = E_CITY;
    print1 ("You pass through the massive gates of Rampart, the city.");
}

// This function coordinates monsters and player actions, as well as
// random events. Each tick is a second. There are therefore 60 ticks to
// the minute and 60 minutes to the hour.
void time_clock (int reset)
{
    if (++Tick > 60) {
	Tick = 0;
	minute_status_check();	// see about some player statuses each minute
	if (++Time % 10 == 0)
	    tenminute_check();
    }
    if (reset)
	Tick = (Player.click = 0);

    int env = Current_Environment;
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
    if (Current_Environment != E_COUNTRYSIDE) {
	foreach (m, Level->mlist) {
	    if (m->hp <= 0)
		--(m = Level->mlist.erase(m));
	    if (Tick == m->click) {
		m->click = (m->click + m->speed) % 60;
		m_pulse (m);
	    }
	}
    }
}
