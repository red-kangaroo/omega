#include "defs.h"
#include "extern.h"

extern const struct monster_data Monsters[NUMMONSTERS];	// one of each monster
extern struct spell Spells[NUMSPELLS + 1];	// one of each spell
extern uint64_t SpellKnown;			// bit set for known spells
extern const struct object_data Objects[TOTALITEMS];	// one of each item
extern uint8_t ObjectAttrs[TOTALITEMS];		// attributes such as known and uniqueness
extern const struct object_data NullObject;
extern citysite CitySiteList[NUMCITYSITES];	// locations of city sites
extern struct player Player;			// the player
extern uint32_t GameStatus;			// Game Status bit vector
extern uint8_t ScreenLength;			// How large is level window
extern struct level* Country;			// The countryside
extern struct level* City;			// The city of Rampart
extern struct level* TempLevel;			// Place holder
extern struct level* Dungeon;			// Pointer to current Dungeon
extern struct level* Level;			// Pointer to current Level
extern int8_t Current_Dungeon;			// What is Dungeon now (an E_ constant)
extern uint8_t Villagenum;			// Current Village number
extern int8_t ScreenOffset;			// Offset of displayed screen to level
extern uint8_t MaxDungeonLevels;		// Deepest level allowed in dungeon
extern EEnvironment Current_Environment;	// Which environment are we in (an E_ constant)
extern EEnvironment Last_Environment;		// Which environment were we in last (an E_ constant)
extern const int8_t Dirs[2][9];			// 9 xy directions
extern chtype Cmd;				// last player command
extern uint16_t Command_Duration;		// how long does current command take
extern uint8_t Arena_Opponent;			// case label of opponent in l_arena()
extern uint8_t Arena_Victory;			// did player win in arena?
extern uint8_t Imprisonment;			// amount of time spent in jail
extern uint8_t Precipitation;			// Hours of rain, snow, etc
extern uint8_t Phase;				// Phase of the moon
extern int8_t Lunarity;				// How player is affected by moon
extern uint16_t Date;				// day of the year
extern uint16_t Pawndate;			// Pawn Shop item generation date
extern vector<object> Pawnitems;		// items in pawn shop
extern int8_t ViewHour;				// crystal ball use marker
extern int8_t ZapHour;				// staff of enchantment use marker
extern int8_t HelmHour;				// helm of teleportation use marker
extern int8_t SymbolUseHour;			// holy symbol use marker
extern uint8_t Constriction;			// Dragonlord Attack State
extern bool Blessing;				// Altar Blessing State
extern int16_t LastDay;				// DPW date of dole
extern int8_t RitualHour;			// last use of ritual magic
extern int8_t RitualRoom;			// last room use of ritual magic
extern uint8_t Lawstone;			// magic stone counter
extern uint8_t Chaostone;			// magic stone counter
extern uint8_t Mindstone;			// magic stone counter
extern uint8_t Searchnum;			// number of times to search on 's'
extern EVerbosity Verbosity;			// verbosity level
extern uint32_t Time;				// turn number
extern uint8_t Tick;				// current second in minute; action coordinator
extern char Stringbuffer[STRING_BUFFER_SIZE][80];	// the last printed strings
extern uint32_t Gymcredit;			// credit at rampart gym
extern uint8_t Spellsleft;			// research allowance at college
extern uint16_t StarGemUse;			// last date of star gem use
extern uint16_t HiMagicUse;			// last date of high magic use
extern uint8_t HiMagic;				// current level for l_throne
extern uint32_t Balance;			// bank account
extern uint32_t FixedPoints;			// points are frozen after adepthood
extern uint8_t LastCountryLocX;			// previous position in countryside
extern uint8_t LastCountryLocY;			// previous position in countryside
extern uint8_t LastTownLocX;			// previous position in village or city
extern uint8_t LastTownLocY;			// previous position in village or city
extern char Password[8];			// autoteller password
extern vector<object> Condoitems;		// items in condo
extern char Str1[STRING_LEN], Str2[STRING_LEN], Str3[STRING_LEN], Str4[STRING_LEN];	// Some string space, random uses

// high score names, levels, behavior
const uint16_t Archmagebehavior = 2933;
const uint16_t Archmagelevel = 10;
const uint16_t Championbehavior = 2713;
const uint16_t Championlevel = 10;
const uint16_t Chaos = -100;
const uint16_t Chaoslordbehavior = 2712;
const uint16_t Chaoslordlevel = 10;
const uint16_t Commandantbehavior = 2713;
const uint16_t Commandantlevel = 10;
const uint16_t Dukebehavior = 2718;
const uint16_t Dukelevel = 10;
const uint16_t Hibehavior = 2713;
const uint16_t Hilevel = 0;
const uint16_t Justiciarbehavior = 2718;
const uint16_t Justiciarlevel = 10;
const uint16_t Law = 100;
const uint16_t Lawlordbehavior = 2711;
const uint16_t Lawlordlevel = 10;
const uint16_t Primebehavior = 2932;
const uint16_t Primelevel = 10;
const uint16_t Shadowlordbehavior = 2712;
const uint16_t Shadowlordlevel = 10;
const uint16_t Hiscore = 0;
extern const uint16_t Priestbehavior [NUMRELIGIONS];
extern const char Shadowlord[], Archmage[], Prime[], Commandant[], Duke[];
extern const char Champion[], Priest[NUMRELIGIONS][12], Hiscorer[], Hidescrip[];
extern const char Chaoslord[], Lawlord[], Justiciar[];
extern const uint8_t Priestlevel [NUMRELIGIONS];

// New globals which used to be statics
extern bool twiddle;
extern bool saved;
extern bool onewithchaos;
extern uint8_t club_hinthour;
extern uint16_t winnings;
extern uint8_t tavern_hinthour;

extern uint8_t deepest[E_MAX + 1];
extern uint32_t level_seed[E_MAX + 1];

// Data files
extern const char Help_Overview[];
extern const char Help_Character[];
extern const char Help_Inventory[];
extern const char Help_Movement[];
extern const char Help_Combat[];
extern const char Help_Bugs[];
extern const char Help_Magic[];
extern const char Help_Countryside[];
extern const char Help_Screen[];
extern const char Help_Saving[];
extern const char Help_Options[];
extern const char Help_CityCommands[];
extern const char Help_CountryCommands[];
extern const char Data_AbyssIntro[];
extern const char Data_Intro[];
extern const char Data_Title[];
extern const char Data_ScrollReligion[];
extern const char Data_ScrollRampartGuide[];
extern const char Data_ScrollHiMagick[];
extern const char Data_ScrollAdept[];
extern const char Level_Abyss[];
extern const char Level_Arena[];
extern const char Level_Circle[];
extern const char Level_City[];
extern const char Level_Country[];
extern const char Level_Court[];
extern const char Level_DragonLair[];
extern const char Level_House[];
extern const char Level_Mansion[];
extern const char Level_Hovel[];
extern const char Level_Maze1[];
extern const char Level_Maze2[];
extern const char Level_Maze3[];
extern const char Level_Maze4[];
extern const char Level_MagicIsle[];
extern const char Level_StarPeak[];
extern const char Level_Temple[];
extern const char Level_Village1[];
extern const char Level_Village2[];
extern const char Level_Village3[];
extern const char Level_Village4[];
extern const char Level_Village5[];
extern const char Level_Village6[];

namespace {

static constexpr inline int pow2 (int n) { return (1 << n); }

static inline bool loc_statusp (int x, int y, unsigned stat)	{ return (Level->site(x,y).lstatus & stat); }
static inline void lset (int x, int y, unsigned stat)		{ Level->site(x,y).lstatus |= stat; }
static inline void lreset (int x, int y, unsigned stat)		{ Level->site(x,y).lstatus &= ~stat; }

static inline bool c_statusp (int x, int y, unsigned stat)	{ return (Country->site(x,y).lstatus & stat); }
static inline void c_set (int x, int y, unsigned stat)		{ Country->site(x,y).lstatus |= stat; }
static inline void c_reset(int x, int y, unsigned stat)		{ Country->site(x,y).lstatus &= ~stat; }

static inline bool m_statusp (const monster_data* m,unsigned s)	{ return (m->status & s); }
static inline void m_status_set (monster* m, unsigned s)	{ m->status |= s; }
static inline void m_status_reset (monster* m, unsigned s)	{ m->status &= ~s; }
static inline bool m_immunityp (const monster* m, unsigned s)	{ return (m->immunity & pow2(s)); }
static inline bool m_statusp (const monster_data& m,unsigned s)	{ return (m.status & s); }
static inline void m_status_set (monster& m, unsigned s)	{ m.status |= s; }
static inline void m_status_reset (monster& m, unsigned s)	{ m.status &= ~s; }
static inline bool m_immunityp (const monster& m, unsigned s)	{ return (m.immunity & pow2(s)); }

static inline bool gamestatusp (unsigned flag)			{ return (GameStatus & flag); }
static inline void setgamestatus (unsigned flag)		{ GameStatus |= flag; }
static inline void resetgamestatus (unsigned flag)		{ GameStatus &= ~flag; }

static inline bool optionp (unsigned o)				{ return (Player.options & o); }
static inline void optionset (unsigned o)			{ Player.options |= o; }
static inline void optionreset (unsigned o)			{ Player.options &= ~o; }

static inline bool spell_is_known (ESpell sp)			{ return (SpellKnown & (UINT64_C(1) << sp)); }
static inline void learn_spell (ESpell sp)			{ SpellKnown |= (UINT64_C(1) << sp); }
static inline void forget_spell (ESpell sp)			{ SpellKnown &= ~(UINT64_C(1) << sp); }
static inline void learn_all_spells (void)			{ SpellKnown = UINT64_MAX; }
static inline void forget_all_spells (void)			{ SpellKnown = 0; }

static inline bool object_is_known (unsigned o)			{ return (ObjectAttrs[o] & OBJECT_KNOWN); }
static inline void learn_object (unsigned o)			{ ObjectAttrs[o] |= OBJECT_KNOWN; }
static inline void forget_object (unsigned o)			{ ObjectAttrs[o] &= ~OBJECT_KNOWN; }
static inline uint8_t object_uniqueness (unsigned o)		{ return (EUniqueness(ObjectAttrs[o] & OBJECT_UNIQUENESS)); }
static inline void set_object_uniqueness (unsigned o, EUniqueness u)	{ ObjectAttrs[o] = (ObjectAttrs[o]&OBJECT_KNOWN)|u; }

static inline bool object_is_known (const object* o)		{ return (object_is_known(o->id)); }
static inline void learn_object (const object* o)		{ learn_object (o->id); }
static inline void forget_object (const object* o)		{ forget_object (o->id); }
static inline uint8_t object_uniqueness (const object* o)	{ return (object_uniqueness (o->id)); }
static inline void set_object_uniqueness (const object* o, EUniqueness u) { set_object_uniqueness (o->id, u); }

static inline bool object_is_known (const object& o)		{ return (object_is_known(o.id)); }
static inline void learn_object (const object& o)		{ learn_object (o.id); }
static inline void forget_object (const object& o)		{ forget_object (o.id); }
static inline uint8_t object_uniqueness (const object& o)	{ return (object_uniqueness (o.id)); }
static inline void set_object_uniqueness (const object& o,EUniqueness u) { set_object_uniqueness (o.id, u); }

} // namespace
