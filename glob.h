#include "defs.h"
#include "extern.h"

extern const struct monster_data Monsters[NUMMONSTERS];	// one of each monster
extern struct spell Spells[NUMSPELLS + 1];	// one of each spell
extern uint64_t SpellKnown;			// bit set for known spells
extern struct object Objects[TOTALITEMS];	// one of each item
extern uint8_t ObjectAttrs[TOTALITEMS];		// attributes such as known and uniqueness
extern int CitySiteList[NUMCITYSITES][3];	// locations of city sites [0] - found, [1] - x, [2] - y
extern struct player Player;			// the player
extern int LENGTH;				// level y dimension
extern int WIDTH;				// level x dimension
extern long GameStatus;				// Game Status bit vector
extern int ScreenLength;			// How large is level window
extern struct terrain Country[MAXWIDTH][MAXLENGTH];	// The countryside
extern struct level *City;			// The city of Rampart
extern struct level *TempLevel;			// Place holder
extern struct level *Dungeon;			// Pointer to current Dungeon
extern struct level *Level;			// Pointer to current Level
extern int Current_Dungeon;			// What is Dungeon now (an E_ constant)
extern int Villagenum;				// Current Village number
extern int ScreenOffset;			// Offset of displayed screen to level
extern int MaxDungeonLevels;			// Deepest level allowed in dungeon
extern int Current_Environment;			// Which environment are we in (an E_ constant)
extern int Last_Environment;			// Which environment were we in last (an E_ constant)
extern int Dirs[2][9];				// 9 xy directions
extern char Cmd;				// last player command
extern int Command_Duration;			// how long does current command take
extern struct monster *Arena_Monster;		// Opponent in arena
extern int Arena_Opponent;			// case label of opponent in l_arena()
extern int Arena_Victory;			// did player win in arena?
extern int Imprisonment;			// amount of time spent in jail
extern int Precipitation;			// Hours of rain, snow, etc
extern int Phase;				// Phase of the moon
extern int Lunarity;				// How player is affected by moon
extern int Date;				// day of the year
extern int Pawndate;				// Pawn Shop item generation date
extern pob Pawnitems[PAWNITEMS];		// items in pawn shop
extern int ViewHour;				// crystal ball use marker
extern int ZapHour;				// staff of enchantment use marker
extern int HelmHour;				// helm of teleportation use marker
extern int SymbolUseHour;			// holy symbol use marker
extern int Constriction;			// Dragonlord Attack State
extern int Blessing;				// Altar Blessing State
extern int LastDay;				// DPW date of dole
extern int RitualHour;				// last use of ritual magic
extern int RitualRoom;				// last room use of ritual magic
extern int Lawstone;				// magic stone counter
extern int Chaostone;				// magic stone counter
extern int Mindstone;				// magic stone counter
extern int Searchnum;				// number of times to search on 's'
extern int Verbosity;				// verbosity level
extern int Behavior;				// NPC behavior, if entered
extern char Seed;				// random seed
extern long Time;				// turn number
extern int Tick;				// current second in minute; action coordinator
extern char Stringbuffer[STRING_BUFFER_SIZE][80];	// the last printed strings
extern long Gymcredit;				// credit at rampart gym
extern int Spellsleft;				// research allowance at college
extern int StarGemUse;				// last date of star gem use
extern int HiMagicUse;				// last date of high magic use
extern int HiMagic;				// current level for l_throne
extern long Balance;				// bank account
extern long FixedPoints;			// points are frozen after adepthood
extern int LastCountryLocX;			// previous position in countryside
extern int LastCountryLocY;			// previous position in countryside
extern int LastTownLocX;			// previous position in village or city
extern int LastTownLocY;			// previous position in village or city
extern char Password[64];			// autoteller password
extern pol Condoitems;				// items in condo
extern char Str1[100], Str2[100], Str3[100], Str4[100];	// Some string space, random uses

// high score names, levels, behavior
extern int Shadowlordbehavior, Archmagebehavior, Primebehavior, Justiciarbehavior;
extern int Commandantbehavior, Chaoslordbehavior, Lawlordbehavior;
extern int Championbehavior, Priestbehavior[7], Hibehavior, Dukebehavior;
extern char Shadowlord[80], Archmage[80], Prime[80], Commandant[80], Duke[80];
extern char Champion[80], Priest[7][80], Hiscorer[80], Hidescrip[80];
extern char Chaoslord[80], Lawlord[80], Justiciar[80];
extern int Shadowlordlevel, Archmagelevel, Primelevel, Commandantlevel, Dukelevel;
extern int Championlevel, Priestlevel[7], Hilevel, Justiciarlevel;
extern long Hiscore;
extern int Chaoslordlevel, Lawlordlevel, Chaos, Law;

// New globals which used to be statics
extern int twiddle;
extern int saved;
extern int onewithchaos;
extern int club_hinthour;
extern int winnings;
extern int tavern_hinthour;
extern int scroll_ids[30];
extern int potion_ids[30];
extern int stick_ids[30];
extern int ring_ids[30];
extern int cloak_ids[30];
extern int boot_ids[30];

extern int deepest[E_MAX + 1];
extern int level_seed[E_MAX + 1];

namespace {

static constexpr inline int pow2 (int n) { return (1 << n); }

static inline bool loc_statusp (int x, int y, unsigned stat)	{ return (Level->site[x][y].lstatus & stat); }
static inline void lset (int x, int y, unsigned stat)		{ Level->site[x][y].lstatus |= stat; }
static inline void lreset (int x, int y, unsigned stat)		{ Level->site[x][y].lstatus &= ~stat; }

static inline bool c_statusp (int x, int y, unsigned stat)	{ return (Country[x][y].status & stat); }
static inline void c_set (int x, int y, unsigned stat)		{ Country[x][y].status |= stat; }
static inline void c_reset(int x, int y, unsigned stat)		{ Country[x][y].status &= ~stat; }

static inline bool m_statusp (const monster_data* m,unsigned s)	{ return (m->status & s); }
static inline void m_status_set (monster* m, unsigned s)	{ m->status |= s; }
static inline void m_status_reset (monster* m, unsigned s)	{ m->status &= ~s; }
static inline bool m_immunityp (monster* m, unsigned s)		{ return (m->immunity & pow2(s)); }

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
static inline void learn_object (object* o)			{ learn_object (o->id); }
static inline void forget_object (object* o)			{ forget_object (o->id); }
static inline uint8_t object_uniqueness (const object* o)	{ return (object_uniqueness (o->id)); }
static inline void set_object_uniqueness (object* o, EUniqueness u)	{ set_object_uniqueness (o->id, u); }

} // namespace
