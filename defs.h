#pragma once
#include "config.h"
#include <curses.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

enum { CASHVALUE = -2, ABORT = -1, };
enum {
    MAXWIDTH = 64,		// moderately arbitrary but probably cannot be easily changed
    MAXLENGTH = 64,
    SMALLSCREENLENGTH = 16,
    MAXITEMS = 16,		// number of slots in inventory. Cannot be changed without work
    MAXPACK = 26,		// number of slots in pack. Should be <= 26.
    PAWNITEMS = 20,		// number of items in pawn shop. Should be <= 26
    STRING_BUFFER_SIZE = 25,	// number of lines back strings are recalled
    REROLLS = 31,		// number of rerolls allowed +1
    MAXROOMS = 48,
    MAXCONNECTIONS = 4,
    STRING_LEN = 100,
    MAXLEVELS = 21,		// Arbitrary. Max of the levels in the dungeons
    ASTRALLEVELS = 5,		// levels in each dungeon
    SEWERLEVELS = 18,
    CASTLELEVELS = 16,
    CAVELEVELS = 10,
    VOLCANOLEVELS = 20,
    NOCITYMOVE = 666,		// cannot use M command on site with this aux value
    NUMOPTIONS = 11,		// total number of player options
    NUMTFOPTIONS = 9,		// number of options with TRUE/FALSE values
    VERBOSITY_LEVEL = 10,	// The slot number of the two options not in Player.options
    SEARCH_DURATION = 11
};

// Verbosity levels
enum { TERSE, MEDIUM, VERBOSE };

// Overall Game Progress Vector Bits
// Long had BETTER have at least 32 bits....
enum {
    SPOKE_TO_DRUID	= (1<<0),
    COMPLETED_CAVES	= (1<<1),
    COMPLETED_SEWERS	= (1<<2),
    COMPLETED_CASTLE	= (1<<3),
    COMPLETED_ASTRAL	= (1<<4),
    COMPLETED_VOLCANO	= (1<<5),
    KILLED_DRAGONLORD	= (1<<6),
    KILLED_EATER	= (1<<7),
    KILLED_LAWBRINGER	= (1<<8),
    COMPLETED_CHALLENGE	= (1<<9),
    SOLD_CONDO		= (1<<10),
    FAST_MOVE		= (1<<11),
    SKIP_PLAYER		= (1<<12),
    SKIP_MONSTERS	= (1<<13),
    MOUNTED		= (1<<14),
    SUPPRESS_PRINTING	= (1<<15),
    LOST		= (1<<16),
    ARENA_MODE		= (1<<17),
    CHEATED		= (1<<18),
    BANK_BROKEN		= (1<<19),
    CLUB_MEMBER		= (1<<20),
    PREPARED_VOID	= (1<<21),
    DESTROYED_ORDER	= (1<<22),
    GAVE_STARGEM	= (1<<23),
    ATTACKED_ORACLE	= (1<<24),
    UNDEAD_GUARDS	= (1<<25),
};

enum {
    // non-existant environments for the random number seeding routine
    E_RESTORE = -2,
    E_RANDOM,
    // general environment types
    E_NEVER_NEVER_LAND,
    E_COUNTRYSIDE,
    E_CITY,
    E_VILLAGE,
    E_TACTICAL_MAP,
    E_SEWERS,
    E_CASTLE,
    E_CAVES,
    E_VOLCANO,
    E_ASTRAL,
    E_ARENA,
    E_HOVEL,
    E_MANSION,
    E_HOUSE,
    E_DLAIR,
    E_ABYSS,
    E_STARPEAK,
    E_MAGIC_ISLE,
    E_TEMPLE,
    E_CIRCLE,
    E_COURT,
    E_MAX = E_COURT
};

// player game status
enum { DEAD = 1, QUIT, WIN, BIGWIN };

// as in attack low, block high, etc.
// These values may be added to the ones above to get things like
// block high, cut low, etc. CLEVER is only used by some monsters
// to cheat with....
enum { LOW = 1, CENTER, HIGH, CLEVER };
// weapon types
enum { CUTTING = 1, THRUSTING, STRIKING, MISSILE };

// random aux constants
// aux field for private residences in city
enum { UNLOCKED, BURGLED = 2, LOCKED };
// bow and crossbow object aux fields
enum { UNLOADED, LOADED };
// alignment used randomly throughout
enum { LAWFUL = 1, CHAOTIC, NEUTRAL };

// spells
enum ESpell {
    S_MON_DET, S_OBJ_DET, S_MISSILE, S_FIREBOLT, S_TELEPORT,
    S_LBALL, S_SLEEP, S_DISRUPT, S_DISINTEGRATE, S_POLYMORPH,
    S_HEAL, S_DISPEL, S_IDENTIFY, S_BREATHE, S_INVISIBLE,
    S_WARP, S_ENCHANT, S_BLESS, S_RESTORE, S_CURE,
    S_TRUESIGHT, S_HELLFIRE, S_KNOWLEDGE, S_HERO, S_RETURN,
    S_DESECRATE, S_HASTE, S_SUMMON, S_SANCTUARY, S_ACCURACY,
    S_RITUAL, S_FEAR, S_APPORT, S_SHADOWFORM, S_ALERT,
    S_REGENERATE, S_SANCTIFY, S_CLAIRVOYANCE, S_DRAIN, S_LEVITATE,
    S_WISH, NUMSPELLS
};

// ranks in guilds, etc
enum { LEGION, ARENA, COLLEGE, THIEVES, ORDER, CIRCLE, NOBILITY, PRIESTHOOD, ADEPT, NUMRANKS };
enum { NOT_IN_LEGION,	LEGIONAIRE, CENTURION, FORCE_LEADER, COLONEL, COMMANDANT };
enum { NOT_IN_ARENA,	TRAINEE, BESTIARIUS, RETIARIUS, GLADIATOR, CHAMPION };
enum { NOT_IN_COLLEGE,	NOVICE, STUDENT, PRECEPTOR, MAGE, ARCHMAGE };
enum { NOT_A_THIEF,	TMEMBER, ATHIEF, THIEF, TMASTER, SHADOWLORD };
enum { NOT_IN_ORDER,	GALLANT, GUARDIAN, CHEVALIER, PALADIN, JUSTICIAR };
enum { NOT_IN_CIRCLE,	INITIATE, ENCHANTER, SORCEROR, HIGHSORCEROR, PRIME };
enum { NOT_NOBILITY,	COMMONER, ESQUIRE, KNIGHT, LORD, DUKE };
enum { NOT_A_BELIEVER,	LAY, ACOLYTE, PRIEST, SPRIEST, HIGHPRIEST };
enum { NOT_A_RELIGION,	ODIN, SET, ATHENA, HECATE, DRUID, DESTINY };

// MONSTER STATUS/ABILITY BITS
enum {
    AWAKE	= (1<<0),
    MOBILE 	= (1<<1),
    HOSTILE	= (1<<2),
    WANDERING	= (1<<4),
    HUNGRY	= (1<<5),
    GREEDY	= (1<<6),
    NEEDY	= (1<<7),
    ONLYSWIM	= (1<<8),
    FLYING	= (1<<9),
    INTANGIBLE	= (1<<10),
    M_INVISIBLE	= (1<<11),
    SWIMMING	= (1<<12),
    POISONOUS	= (1<<13),
    EDIBLE	= (1<<14)
};

// PLAYER STATUS INDICES
enum {
    ACCURACY, BLINDED, SLOWED, DISPLACED, SLEPT,
    DISEASED, POISONED, HASTED, BREATHING, INVISIBLE,
    REGENERATING, VULNERABLE, BERSERK, IMMOBILE, ALERT,
    AFRAID, HERO, LEVITATING, ACCURATE, TRUESIGHT,
    SHADOWFORM, ILLUMINATION, DEFLECTION, PROTECTION, RETURNING,
    // PROTECTION is deviant -- indicates protective value, not duration
    // RETURNING is somewhat deviant--how many turns 'til RETURN spell goes off
    NUMSTATI
};

// player immunity indices
// also monster immunity bits (2^n)
// also damage types
enum {
    EVERYTHING=-1,
    UNSTOPPABLE, NORMAL_DAMAGE, FLAME, COLD, ELECTRICITY,
    POISON, ACID, FEAR, SLEEP, NEGENERGY,
    OTHER_MAGIC, THEFT, GAZE, INFECTION, NUMIMMUNITIES
};

// location lstatus bits
enum {
    SEEN	= (1<<0),
    LIT		= (1<<1),
    SECRET	= (1<<2),
    STOPS	= (1<<3),
    CHANGED	= (1<<4)
};

// room string id for use in roomname()
enum ERoomName {
    RS_EMPTY,
    RS_WALLSPACE,
    RS_CORRIDOR,
    RS_CAVERN,
    RS_GOBLINKING,
    RS_DRAGONLORD,
    RS_PONDS,
    RS_OCEAN,
    RS_WYRM,
    RS_ADEPT,
    RS_DESTINY,
    RS_ODIN,
    RS_SET,
    RS_ATHENA,
    RS_HECATE,
    RS_DRUID,
    RS_COUNTRYSIDE,
    RS_ARENA,
    RS_SEWER_DUCT,
    RS_DRAINED_SEWER,
    RS_DROWNED_SEWER,
    RS_KITCHEN,
    RS_BEDROOM,
    RS_BATHROOM,
    RS_DININGROOM,
    RS_SECRETPASSAGE,
    RS_CLOSET,
    RS_LOWERASTRAL,
    RS_EARTHPLANE,
    RS_WATERPLANE,
    RS_AIRPLANE,
    RS_FIREPLANE,
    RS_HIGHASTRAL,
    RS_VOLCANO,
    RS_STARPEAK,
    RS_MAGIC_ISLE,
    RS_CIRCLE,
    RS_ZORCH,
    RS_COURT,
    RS_ROOMBASE,
    RS_GARDEROBE = RS_ROOMBASE,
    RS_DUNGEON_CELL,
    RS_TILED_CHAMBER,
    RS_CRYSTAL_CAVERN,
    RS_MASTER_BEDROOM,
    RS_STOREROOM,
    RS_CHARRED_ROOM,
    RS_MARBLE_HALL,
    RS_EERIE_CAVE,
    RS_TREASURE_CHAMBER,
    RS_SMOKED_ROOM,
    RS_APARTMENT,
    RS_ANTECHAMBER,
    RS_HAREM,
    RS_MULTIPURPOSE,
    RS_STALACTITES,
    RS_GREENHOUSE,
    RS_WATERCLOSET,
    RS_STUDY,
    RS_LIVING_ROOM,
    RS_DEN,
    RS_ABATOIR,
    RS_BOUDOIR,
    RS_STAR_CHAMBER,
    RS_MANMADE_CAVERN,
    RS_SEWER_CONTROL_ROOM,
    RS_HIGH_MAGIC_SHRINE,
    RS_MAGIC_LABORATORY,
    RS_PENTAGRAM_ROOM,
    RS_OMEGA_ROOM,
    RS_ROOMLAST,
    NUMROOMNAMES = RS_ROOMBASE-RS_ROOMLAST
};

enum {
    CLR_BLACK_BROWN		= COLOR_PAIR(5),
    CLR_BLACK_RED		= COLOR_PAIR(3),
    CLR_BLACK_WHITE		= COLOR_PAIR(4),
    CLR_BLUE_BLACK		= COLOR_PAIR(6),
    CLR_BLUE_WHITE		= COLOR_PAIR(7),
    CLR_BRIGHT_WHITE_BLACK	= COLOR_PAIR(17)|A_BOLD,
    CLR_BRIGHT_WHITE_BLUE	= COLOR_PAIR(18)|A_BOLD,
    CLR_BRIGHT_WHITE_RED	= COLOR_PAIR(19)|A_BOLD,
    CLR_BROWN_BLACK		= COLOR_PAIR(21),
    CLR_BROWN_RED		= COLOR_PAIR(23),
    CLR_BROWN_WHITE		= COLOR_PAIR(24),
    CLR_CYAN_BLACK		= COLOR_PAIR(8),
    CLR_GREEN_BLACK		= COLOR_PAIR(9),
    CLR_GREEN_BROWN		= COLOR_PAIR(12),
    CLR_GREEN_RED		= COLOR_PAIR(11),
    CLR_GREY_BLACK		= COLOR_PAIR(1)|A_BOLD,
    CLR_GREY_BROWN		= COLOR_PAIR(5)|A_BOLD,
    CLR_GREY_GREEN		= COLOR_PAIR(2)|A_BOLD,
    CLR_GREY_RED		= COLOR_PAIR(3)|A_BOLD,
    CLR_GREY_WHITE		= COLOR_PAIR(4)|A_BOLD,
    CLR_LIGHT_BLUE_BLACK	= COLOR_PAIR(6)|A_BOLD,
    CLR_LIGHT_BLUE_WHITE	= COLOR_PAIR(7)|A_BOLD,
    CLR_LIGHT_GREEN_BLACK	= COLOR_PAIR(9)|A_BOLD,
    CLR_LIGHT_GREEN_BLUE	= COLOR_PAIR(10)|A_BOLD,
    CLR_LIGHT_PURPLE_BLACK	= COLOR_PAIR(13)|A_BOLD,
    CLR_LIGHT_RED_BLACK		= COLOR_PAIR(15)|A_BOLD,
    CLR_LIGHT_RED_WHITE		= COLOR_PAIR(16)|A_BOLD,
    CLR_PURPLE_BLACK		= COLOR_PAIR(13),
    CLR_PURPLE_WHITE		= COLOR_PAIR(14),
    CLR_RED_BLACK		= COLOR_PAIR(15),
    CLR_RED_WHITE		= COLOR_PAIR(16),
    CLR_WHITE_BLACK		= COLOR_PAIR(17),
    CLR_WHITE_BLUE		= COLOR_PAIR(18),
    CLR_WHITE_BROWN		= COLOR_PAIR(20),
    CLR_WHITE_RED		= COLOR_PAIR(19),
    CLR_YELLOW_BLACK		= COLOR_PAIR(21)|A_BOLD,
    CLR_YELLOW_BLUE		= COLOR_PAIR(22)|A_BOLD,
    CLR_YELLOW_BROWN		= COLOR_PAIR(25)|A_BOLD,
    CLR_YELLOW_WHITE		= COLOR_PAIR(24)|A_BOLD
};

// objects, locations, and terrain; characters to draw
enum {
    NULL_ITEM,
    SPACE		= (' ' | CLR_WHITE_BLACK),
    WALL		= ('#' | CLR_GREY_BLACK),
    PORTCULLIS		= ('7' | CLR_WHITE_BLACK),
    OPEN_DOOR		= ('|' | CLR_BROWN_BLACK),
    CLOSED_DOOR		= ('-' | CLR_BROWN_BLACK),
    WHIRLWIND		= ('6' | CLR_LIGHT_BLUE_BLACK),
    ABYSS		= ('0' | CLR_BLACK_BROWN),
    VOID_CHAR		= (' ' | CLR_WHITE_BLACK),
    LAVA		= ('`' | CLR_RED_BLACK),
    HEDGE		= ('\"'| CLR_GREEN_BLACK),
    WATER		= ('~' | CLR_BLUE_BLACK),
    FIRE		= (';' | CLR_LIGHT_RED_BLACK),
    TRAP		= ('^' | CLR_RED_BLACK),
    LIFT		= ('_' | CLR_BRIGHT_WHITE_BLACK),
    STAIRS_UP		= ('<' | CLR_WHITE_BLACK),
    STAIRS_DOWN		= ('>' | CLR_WHITE_BLACK),
    FLOOR		= ('.' | CLR_BROWN_BLACK),
    PLAYER		= ('@' | CLR_WHITE_BLACK),
    CORPSE		= ('+' | CLR_RED_BLACK),
    STATUE		= ('1' | CLR_GREY_BLACK),
    RUBBLE		= ('4' | CLR_GREY_BLACK),
    ALTAR		= ('8' | CLR_LIGHT_BLUE_BLACK),
    CASH		= ('$' | CLR_YELLOW_BLACK),
    PILE		= ('*' | CLR_BRIGHT_WHITE_BLACK),
    FOOD		= ('%' | CLR_BROWN_BLACK),
    WEAPON		= (')' | CLR_GREY_BLACK),
    MISSILEWEAPON	= ('(' | CLR_BROWN_BLACK),
    SCROLL		= ('?' | CLR_YELLOW_BLACK),
    POTION		= ('!' | CLR_LIGHT_GREEN_BLACK),
    ARMOR		= (']' | CLR_GREY_BLACK),
    SHIELD		= ('[' | CLR_BROWN_BLACK),
    CLOAK		= ('}' | CLR_CYAN_BLACK),
    BOOTS		= ('{' | CLR_BROWN_BLACK),
    STICK		= ('/' | CLR_BROWN_BLACK),
    RING		= ('=' | CLR_YELLOW_BLACK),
    THING		= ('\\'| CLR_WHITE_BLACK),
    ARTIFACT		= ('&' | CLR_YELLOW_BLACK),
    PLAINS		= ('-' | CLR_LIGHT_GREEN_BLACK),
    TUNDRA		= ('_' | CLR_GREY_BLACK),
    ROAD		= ('.' | CLR_BROWN_BLACK),
    MOUNTAINS		= ('^' | CLR_GREY_BLACK),
    PASS		= ('v' | CLR_BROWN_BLACK),
    RIVER		= ('~' | CLR_BLUE_BLACK),
    CITY		= ('O' | CLR_WHITE_BLACK),
    VILLAGE		= ('o' | CLR_WHITE_BLACK),
    FOREST		= ('(' | CLR_LIGHT_GREEN_BLACK),
    JUNGLE		= (')' | CLR_GREEN_BLACK),
    SWAMP		= ('=' | CLR_GREEN_BLACK),
    VOLCANO		= ('!' | CLR_RED_BLACK),
    CASTLE		= ('%' | CLR_GREY_BLACK),
    TEMPLE		= ('X' | CLR_BROWN_BLACK),
    CAVES		= ('*' | CLR_BLACK_BROWN),
    DESERT		= ('\"'| CLR_YELLOW_BLACK),
    CHAOS_SEA		= ('+' | CLR_LIGHT_PURPLE_BLACK),
    STARPEAK		= ('|' | CLR_LIGHT_BLUE_BLACK),
    DRAGONLAIR		= ('$' | CLR_BROWN_BLACK),
    MAGIC_ISLE		= ('&' | CLR_PURPLE_BLACK),
    CHAIR		= ('5' | CLR_BROWN_BLACK),
    SAFE		= ('3' | CLR_GREY_BLACK),
    FURNITURE		= ('2' | CLR_BROWN_BLACK),
    BED			= ('9' | CLR_CYAN_BLACK)
};

// Player.options bits
enum {
    BELLICOSE	= (1<<0),
    JUMPMOVE	= (1<<1),
    RUNSTOP	= (1<<2),
    PICKUP	= (1<<3),
    CONFIRM 	= (1<<4),
    TOPINV 	= (1<<5),
    PACKADD 	= (1<<6),
    COMPRESS_OPTION= (1<<7),
    SHOW_COLOUR	= (1<<8)
};

// running sum of itemtypes, for indexing into Objects array
// This has to be changed whenever an item is added
enum {
    THINGID,
    THING_DOOR_OPENER		= THINGID,
    THING_LOCKPICK		= THINGID+2,
    THING_SALT_WATER		= THINGID+6,
    THING_KEY,
    THING_TORCH,
    THING_JUSTICIAR_BADGE	= THINGID+16,
    THING_DART_TRAP_COMPONENT,
    THING_ACID_TRAP_COMPONENT,
    THING_SNARE_TRAP_COMPONENT,
    THING_FIRE_TRAP_COMPONENT,
    THING_TELEPORT_TRAP_COMPONENT,
    THING_SLEEP_TRAP_COMPONENT,
    THING_DISINTEGRATE_TRAP_COMPONENT,
    THING_ABYSS_TRAP_COMPONENT,
    THING_MANADRAIN_TRAP_COMPONENT,
    NUMTHINGS	= 26,

    FOODID	= THINGID+NUMTHINGS,
    FOOD_RATION			= FOODID,
    FOOD_LEMBAS			= FOODID+1,
    FOOD_GRAIN			= FOODID+15,
    NUMFOODS	= 16,

    SCROLLID	= FOODID+NUMFOODS,
    SCROLL_BLANK		= SCROLLID,
    SCROLL_SPELLS		= SCROLLID+1,
    SCROLL_TELEPORT		= SCROLLID+5,
    SCROLL_TRUESIGHT		= SCROLLID+19,
    SCROLL_SUMMON		= SCROLLID+21,
    SCROLL_LIGHT		= SCROLLID+22,
    NUMSCROLLS	= 24,

    POTIONID	= SCROLLID+NUMSCROLLS,
    POTION_OF_ALERTNESS		= POTIONID+5,
    POTION_OF_SPEED		= POTIONID+6,
    POTION_OF_INVISIBILITY	= POTIONID+10,
    POTION_OF_LEVITATION	= POTIONID+16,
    POTION_OF_CURING		= POTIONID+17,
    NUMPOTIONS	= 18,

    WEAPONID	= POTIONID+NUMPOTIONS,
    WEAPON_DAGGER		= WEAPONID,
    WEAPON_SHORT_SWORD		= WEAPONID+1,
    WEAPON_GREAT_SWORD		= WEAPONID+5,
    WEAPON_ESTOC		= WEAPONID+8,
    WEAPON_GREAT_AXE		= WEAPONID+12,
    WEAPON_MACE			= WEAPONID+13,
    WEAPON_CLUB			= WEAPONID+17,
    WEAPON_QUARTERSTAFF		= WEAPONID+18,
    WEAPON_SPEAR		= WEAPONID+19,
    WEAPON_HALBERD		= WEAPONID+20,
    WEAPON_TRIDENT		= WEAPONID+21,
    WEAPON_LIGHTSABRE		= WEAPONID+23,
    WEAPON_DEMONBLADE		= WEAPONID+24,
    WEAPON_MACE_OF_DISRUPTION	= WEAPONID+25,
    WEAPON_LONGBOW		= WEAPONID+26,
    WEAPON_CROSSBOW		= WEAPONID+27,
    WEAPON_ARROW		= WEAPONID+28,
    WEAPON_BOLT			= WEAPONID+29,
    WEAPON_VORPAL_SWORD		= WEAPONID+31,
    WEAPON_DESECRATOR		= WEAPONID+32,
    WEAPON_FIRESTAR		= WEAPONID+33,
    WEAPON_DEFENDER		= WEAPONID+34,
    WEAPON_VICTRIX		= WEAPONID+35,
    WEAPON_GOBLIN_HEWER		= WEAPONID+36,
    WEAPON_GIANT_CLUB		= WEAPONID+38,
    WEAPON_SCYTHE_OF_DEATH	= WEAPONID+39,
    NUMWEAPONS	= 41,

    ARMORID	= WEAPONID+NUMWEAPONS,
    ARMOR_SOFT_LEATHER		= ARMORID+1,
    ARMOR_MITHRIL_PLATE		= ARMORID+11,
    ARMOR_DRAGONSCALE		= ARMORID+12,
    NUMARMOR	= 17,

    SHIELDID	= ARMORID+NUMARMOR,
    SHIELD_NORMAL		= SHIELDID+2,
    SHIELD_OF_DEFLECTION	= SHIELDID+7,
    NUMSHIELDS	= 8,

    CLOAKID	= SHIELDID+NUMSHIELDS,
    CLOAK_OF_WOOL		= CLOAKID,
    CLOAK_OF_NEGIMMUNITY,
    CLOAK_OF_INVISIBILITY,
    CLOAK_OF_SKILL,
    CLOAK_OF_PROTECTION,
    CLOAK_OF_DISPLACEMENT,
    CLOAK_OF_TRUESIGHT,
    NUMCLOAKS	= 7,

    BOOTID	= CLOAKID+NUMCLOAKS,
    NUMBOOTS	= 7,

    RINGID	= BOOTID+NUMBOOTS,
    RING_OF_TRUESIGHT		= RINGID,
    RING_OF_STRENGTH		= RINGID+2,
    RING_OF_GAZE_IMMUNITY	= RINGID+3,
    RING_OF_FIRE_RESISTANCE	= RINGID+4,
    RING_OF_POISON_RESISTANCE	= RINGID+6,
    NUMRINGS	= 10,

    STICKID	= RINGID+NUMRINGS,
    WAND_OF_BALL_LIGHTNING	= STICKID+3,
    ROD_OF_SUMMONING		= STICKID+4,
    WAND_OF_FIREBALLS		= STICKID+7,
    SNOWBALL_STICK		= STICKID+10,
    STAFF_OF_MISSILES		= STICKID+11,
    STAFF_OF_DISPELLING		= STICKID+13,
    STAFF_OF_HEALING		= STICKID+14,
    NUMSTICKS	= 17,

    ARTIFACTID	= STICKID+NUMSTICKS,
    ORB_OF_MASTERY		= ARTIFACTID,
    ORB_OF_FIRE,
    ORB_OF_WATER,
    ORB_OF_EARTH,
    ORB_OF_AIR,
    ORB_BURNT_OUT,
    CRYSTAL_BALL,
    HOLY_HAND_GRENADE,
    AMULET_OF_YENDOR,
    KEY_OF_KOLWYNIA,
    POTION_OF_DEATH,
    STAFF_OF_ENCHANTMENT,
    TELEPORTATION_HELM,
    POTION_OF_LIFE,
    HOLY_SYMBOL_OF_ODIN,
    HOLY_SYMBOL_OF_SET,
    HOLY_SYMBOL_OF_ATHENA,
    HOLY_SYMBOL_OF_HECATE,
    HOLY_SYMBOL_OF_DRUIDISM,
    HOLY_SYMBOL_OF_DESTINY,
    JUGGERNAUT_OF_KARNAK,
    STAR_GEM,
    SCEPTRE_OF_HIGH_MAGIC,
    AMULET_OF_THE_PLAINS,
    NUMARTIFACTS	= 24,

    CASHID	= ARTIFACTID+NUMARTIFACTS,
    CORPSEID,	// Corpse's aux field is monster id
    TOTALITEMS,
    NO_THING = uint8_t(-1)
};

// describing unique items and monsters
enum EUniqueness { COMMON, UNIQUE_UNMADE, UNIQUE_MADE, UNIQUE_TAKEN };
enum EObjectAttr { OBJECT_KNOWN = (1<<2), OBJECT_UNIQUENESS = OBJECT_KNOWN-1 };

// general item function id's
enum {
    // note some of these functions are for other types of items too
    I_NO_OP, I_NOTHING,
    // scroll functions
    I_BLESS = 101, I_ACQUIRE, I_ENCHANT, I_TELEPORT, I_WISH,
    I_CLAIRVOYANCE, I_DISPLACE, I_ID, I_FIREFLASH, I_SPELLS,
    I_JANE_T, I_ALERT, I_FLUX, I_CHARGE, I_WARP, I_KNOWLEDGE,
    I_LAW, I_HINT, I_HERO, I_TRUESIGHT, I_ILLUMINATE,
    I_DEFLECT,
    // potion functions
    I_HEAL = 201, I_OBJDET, I_MONDET, I_SLEEP_SELF, I_RESTORE,
    I_NEUTRALIZE_POISON, I_SPEED, I_AZOTH, I_REGENERATE, I_INVISIBLE,
    I_BREATHING, I_FEAR_RESIST, I_AUGMENT, I_CHAOS, I_ACCURACY,
    I_LEVITATION, I_CURE,
    // stick functions
    I_FIREBOLT = 301, I_LBOLT, I_MISSILE, I_SLEEP_OTHER, I_FIREBALL,
    I_LBALL, I_SUMMON, I_HIDE, I_DISRUPT, I_DISINTEGRATE,
    I_SNOWBALL, I_APPORT, I_DISPEL, I_POLYMORPH, I_FEAR,
    // food functions
    I_FOOD = 401, I_LEMBAS, I_STIM, I_POW, I_IMMUNE,
    I_POISON_FOOD, I_CORPSE, I_PEPPER_FOOD, I_CANNIBAL, I_INEDIBLE,
    // boots functions
    I_PERM_SPEED = 501, I_PERM_HERO, I_PERM_LEVITATE, I_PERM_AGILITY,
    I_BOOTS_JUMPING, I_BOOTS_7LEAGUE,
    // cloak functions
    I_PERM_DISPLACE = 601, I_PERM_NEGIMMUNE, I_PERM_INVISIBLE, I_PERM_ACCURACY,
    I_PERM_PROTECTION, I_PERM_TRUESIGHT,
    // ring functions
    I_PERM_VISION = 701, I_PERM_BURDEN, I_PERM_STRENGTH, I_PERM_GAZE_IMMUNE,
    I_PERM_FIRE_RESIST, I_PERM_POISON_RESIST, I_PERM_REGENERATE, I_PERM_KNOWLEDGE,
    // armor functions
    I_PERM_ENERGY_RESIST = 801, I_PERM_BREATHING, I_PERM_FEAR_RESIST, I_NORMAL_ARMOR,
    // artifact functions
    I_ORBFIRE = 901, I_ORBWATER, I_ORBEARTH, I_ORBAIR, I_ORBMASTERY,
    I_ORBDEAD, I_CRYSTAL, I_ANTIOCH, I_KOLWYNIA, I_DEATH,
    I_ENCHANTMENT, I_HELM, I_LIFE, I_JUGGERNAUT, I_SYMBOL,
    I_STARGEM, I_SCEPTRE, I_PLANES,
    // weapons functions
    I_NORMAL_WEAPON = 1001, I_LIGHTSABRE, I_DEMONBLADE, I_MACE_DISRUPT, I_TANGLE,
    I_ARROW, I_BOLT, I_VORPAL, I_DESECRATE, I_FIRESTAR,
    I_DEFEND, I_VICTRIX, I_EMPIRE, I_SCYTHE, I_ACIDWHIP,
    // thing functions
    I_PICK = 1101, I_KEY, I_SHOVEL, I_EXCAVATOR, I_PERM_ILLUMINATE,
    I_TRAP, I_RAISE_PORTCULLIS,
    // shield functions
    I_PERM_DEFLECT = 1201,
    I_NORMAL_SHIELD
};

// monster function ids
// Its conceivable for a function of one type to be called when another
// would usually occur. A monster's special function may be an extra move,
// for example.
enum {
    M_NO_OP = -1,
    // talk functions
    M_TALK_STUPID = 101,
    M_TALK_SILENT,
    M_TALK_HUNGRY,
    M_TALK_GREEDY,
    M_TALK_TITTER,
    M_TALK_MAN,
    M_TALK_ROBOT,
    M_TALK_EVIL,
    M_TALK_BURBLE,
    M_TALK_SLITHY,
    M_TALK_MIMSY,
    M_TALK_SEDUCTOR,
    M_TALK_MP,
    M_TALK_IM,
    M_TALK_GUARD,
    M_TALK_GHOST,
    M_TALK_HINT,
    M_TALK_BEG,
    M_TALK_EF,
    M_TALK_GF,
    M_TALK_MORGON,
    M_TALK_LB,
    M_TALK_DEMONLOVER,
    M_TALK_ASSASSIN,
    M_TALK_NINJA,
    M_TALK_THIEF,
    M_TALK_MERCHANT,
    M_TALK_HORSE,
    M_TALK_PARROT,
    M_TALK_ANIMAL,
    M_TALK_HYENA,
    M_TALK_SERVANT,
    M_TALK_SCREAM,
    M_TALK_DRUID,
    M_TALK_ARCHMAGE,
    M_TALK_PRIME,
    // ability functions
    M_SP_SURPRISE = 201,
    M_SP_MP,
    M_SP_THIEF,
    M_SP_AGGRAVATE,
    M_SP_POISON_CLOUD,
    M_SP_HUGE,
    M_SP_SUMMON,
    M_SP_ILLUSION,
    M_SP_FLUTTER,
    M_SP_ESCAPE,
    M_SP_SPELL,
    M_SP_EXPLODE,
    M_SP_DEMON,
    M_SP_ACID_CLOUD,
    M_SP_WHIRL,
    M_SP_GHOST,
    M_SP_WHISTLEBLOWER,
    M_SP_EATER,
    M_SP_LAWBRINGER,
    M_SP_DRAGONLORD,
    M_SP_DE,
    M_SP_DEMONLOVER,
    M_SP_SEDUCTOR,
    M_SP_MASTER,
    M_SP_WYRM,
    M_SP_BLACKOUT,
    M_SP_BOG,
    M_SP_MERCHANT,
    M_SP_WERE,
    M_SP_SERVANT,
    M_SP_AV,
    M_SP_LW,
    M_SP_SWARM,
    M_SP_ANGEL,
    M_SP_MB,
    M_SP_MIRROR,
    M_SP_RAISE,
    M_SP_DEATH,
    M_SP_COURT,
    M_SP_LAIR,
    M_SP_PRIME,
    // rangestrike functions
    M_STRIKE_MISSILE = 301,
    M_STRIKE_FBOLT,
    M_STRIKE_LBALL,
    M_STRIKE_FBALL,
    M_STRIKE_BLIND,
    M_STRIKE_SNOWBALL,
    M_STRIKE_MASTER,
    M_STRIKE_SONIC,
    // combat functions
    M_MELEE_NORMAL = 401,
    M_MELEE_FIRE,
    M_MELEE_DRAGON,
    M_MELEE_MP,
    M_MELEE_ELEC,
    M_MELEE_POISON,
    M_MELEE_NG,
    M_MELEE_SUCCUBUS,
    M_MELEE_SPIRIT,
    M_MELEE_DISEASE,
    M_MELEE_SLEEP,
    M_MELEE_COLD,
    M_MELEE_MASTER,
    M_MELEE_GRAPPLE,
    M_MELEE_DEATH,
    // movement functions
    M_MOVE_NORMAL = 501,
    M_MOVE_FLUTTER,
    M_MOVE_TELEPORT,
    M_MOVE_FOLLOW,
    M_MOVE_RANDOM,
    M_MOVE_SMART,
    M_MOVE_SPIRIT,
    M_MOVE_SCAREDY,
    M_MOVE_CONFUSED,
    M_MOVE_ANIMAL,
    M_MOVE_LEASH
};

// Some monster ID's : (Those that are explicitly named in code)
// Actually, there are still many magic constants floating around.
// Eventually I'll get around to making each monster's id a constant....
// done, thanks to David Given.
//
// MLx -> index to Monsters starting for level x
// MLx -> number of monsters of level x or less
// NML_x -> number of monsters of level x
// NML-X must be changed whenever a monster is added
// This whole thing MUST be repaired.  Monster levels must
// be represented elsewhere.
//
enum {
    RANDOM = -1,

    ML0, HORNET = ML0, MEND_PRIEST, ITIN_MERCH, GUARD, NPC,
    SHEEP, MERCHANT, ZERO_NPC, HISCORE_NPC,

    ML1, GRUNT = ML1, TSETSE, FNORD, SEWER_RAT, AGGRAVATOR,
    BLIPPER, GOBLIN, PHANTASTICON, ROBOT, GEEK,
    BOROGROVE, QUAIL, BADGER, HAWK, DEER,
    CAMEL, ANTEATER, BUNNY, TROUT, BASS,
    PARROT, HYENA,

    ML2, APPR_NINJA = ML2, NIGHT_GAUNT, SNEAK_THIEF, EYE, TOVE,
    NASTY, GHOST, ENCHANTOR, MURK, GOBLIN_CHIEF,
    WOLF, ANT, ELEPHANT, HORSE,

    ML3, SALAMANDER = ML3, CATOBLEPAS, L_FDEMON, ACID_CLOUD, PHANTOM,
    GOBLIN_KING, PTERODACTYL, GOBLIN_SHAMAN, LION, BRIGAND,
    BEAR, MAMBA, MANOWAR, WEREHUMAN, THOUGHTFORM,

    ML4, MANTICORE = ML4, TASMANIAN, AUTO_MINOR, DENEBIAN, JUBJUB,
    HAUNT, INCUBUS, SATYR, CROC, TORPOR,
    DOBERMAN, FUZZY, SERV_LAW, SERV_CHAOS, SWARM,
    BAN_SIDHE, GRUE, GENIN,

    ML5, DRAGONETTE = ML5, TESLA, WYVERN, CATEAGLE, FROST_DEMON,
    SPECTRE, NECROMANCER, SHADOW, BOGTHING, ASTRAL_VAMP,
    LAVA_WORM, MANABURST, OUTER_DEMON, MIRRORSHADE,

    ML6, FIRE_ELEM = ML6, AIR_ELEM, WATER_ELEM, EARTH_ELEM, BANDERSNATCH,
    LICHE, TRITON, MAST_THIEF, TRICER, RAKSHASA,
    DEMON_SERP, ANGEL, CHUNIN,

    ML7, BEHEMOTH = ML7, NAZGUL, UNICORN, ROUS, ILL_FIEND,
    GREAT_WYRM, FLAME_DEV, LURKER, SANDMAN, MIRRORMAST,
    ELDER_GRUE, LOATHLY, ZOMBIE, RICOCHET, INNER_DEMON,

    ML8, GOOD_FAIRY = ML8, BAD_FAIRY, AUTO_MAJOR, DRAGON, JABBERWOCK,
    FDEMON_L, TIGERSHARK, JONIN, SHADOW_SLAY, MIL_PRIEST,
    COMA, HIGH_ANGEL,

    ML9, JOTUN = ML9, INVIS_SLAY, KING_WYV, DEATHSTAR, THAUMATURGIST,
    VAMP_LORD, ARCHANGEL, DEMON_PRINCE,

    ML10, DEATH = ML10, EATER, LAWBRINGER, DRAGON_LORD, DEMON_EMP,
    LORD_EARTH, LORD_AIR, LORD_WATER, LORD_FIRE, ELEM_MASTER,

    NUMMONSTERS,

    NML_0 = ML1-ML0, NML_1 = ML2-ML1, NML_2 = ML3-ML2,
    NML_3 = ML4-ML3, NML_4 = ML5-ML4, NML_5 = ML6-ML5,
    NML_6 = ML7-ML6, NML_7 = ML8-ML9, NML_8 = ML9-ML8,
    NML_9 = ML10-ML9, NML_10 = NUMMONSTERS-ML10
};

// location functions
enum {
    L_NO_OP, L_LIFT, L_BALANCESTONE, L_FIRE, L_WHIRLWIND,
    L_VOIDSTONE, L_WARNING, L_ARENA_EXIT, L_HOUSE_EXIT, L_SAFE
};

// city level shop and guild functions
// following are those in CitySiteList
enum {
    CITYSITEBASE = 10,
    L_CHARITY, L_ARMORER, L_CLUB, L_GYM, L_THIEVES_GUILD,
    L_COLLEGE, L_HEALER, L_CASINO, L_TAVERN, L_MERC_GUILD,
    L_ALCHEMIST, L_SORCERORS, L_CASTLE, L_ARENA, L_DPW,
    L_LIBRARY, L_PAWN_SHOP, L_BANK, L_CONDO, L_ORACLE,
    L_ORDER, L_DINER, L_COMMANDANT, L_CRAP, L_TEMPLE,
    L_COUNTRYSIDE, L_BROTHEL, L_JAIL, L_TEMPLE_WARNING, L_LAWSTONE,
    L_CHAOSTONE,
    // final abyss sites ignore levitation
    L_EARTH_STATION, L_FIRE_STATION, L_WATER_STATION, L_AIR_STATION, L_VOID_STATION,
    L_VOID, L_VOICE1, L_VOICE2, L_VOICE3, L_SACRIFICESTONE,
    // circle hq sites
    L_TOME1, L_TOME2, L_ENTER_CIRCLE, L_CIRCLE_LIBRARY,
    // other site functions
    L_DRUID, L_ALTAR, L_GARDEN, L_ADEPT, L_SEWER,
    L_OMEGA, L_CARTOGRAPHER, L_STABLES, L_COMMONS, L_GRANARY,
    L_MAZE, L_HOVEL, L_HOUSE, L_MANSION, L_OCCUPIED_HOUSE,
    L_TACTICAL_EXIT, L_VAULT, L_CEMETARY, L_THRONE, L_ESCALATOR,
    L_ENTER_COURT, L_TRIFID, L_FINAL_ABYSS, L_RAISE_PORTCULLIS, L_MINDSTONE,
    // random sites
    L_CHAOS, L_WATER, L_LAVA, L_MAGIC_POOL, L_PORTCULLIS_TRAP,
    L_DROP_EVERY_PORTCULLIS = L_PORTCULLIS_TRAP+2, L_PORTCULLIS,
    // traps
    L_TRAP_DART, L_TRAP_PIT, L_TRAP_DOOR, L_TRAP_SNARE, L_TRAP_BLADE,
    L_TRAP_FIRE, L_TRAP_TELEPORT, L_TRAP_DISINTEGRATE, L_TRAP_SLEEP_GAS, L_TRAP_ACID,
    L_TRAP_MANADRAIN, L_TRAP_ABYSS, L_TRAP_SIREN,
    // more random sites
    L_STATUE_WAKE, L_STATUE_RANDOM, L_HEDGE, L_RUBBLE, L_ABYSS,
    // Computed limits
    NUMCITYSITES = L_JAIL-CITYSITEBASE,
    LEVITATION_AVOIDANCE = L_CHAOS-1,	// above LEVITATION_AVOIDANCE, no effect if player is levitating
    TRAP_BASE = L_TRAP_DART,
    NUMTRAPS = L_TRAP_SIREN-TRAP_BASE+1
};

// player possession slots
// slot 0 should not be filled when out of inventory_control()
enum {
    O_UP_IN_AIR, O_READY_HAND, O_WEAPON_HAND,
    O_LEFT_SHOULDER, O_RIGHT_SHOULDER,
    O_BELT1, O_BELT2, O_BELT3, O_SHIELD, O_ARMOR,
    O_BOOTS, O_CLOAK, O_RING1, O_RING2, O_RING3, O_RING4
};

// structure definitions

struct spell { uint8_t powerdrain; };

struct object {
    uint8_t id;
    uint8_t level;
    uint16_t weight;
    int8_t plus;
    uint8_t fragility;
    int16_t charge;
    uint8_t dmg;
    uint8_t hit;
    int16_t aux;
    uint16_t number;
    uint16_t basevalue;
    uint8_t used;
    int8_t blessing;
    uint8_t type;
    uint8_t uniqueness;
    int16_t usef;
    chtype objchar;
    const char* objstr;
    const char* truename;
    const char* cursestr;
};

struct monster_data {
    uint8_t id;
    uint8_t level;
    uint16_t hp;
    uint8_t hit;
    uint8_t ac;
    uint8_t dmg;
    uint8_t sense;
    uint8_t wakeup;
    uint8_t speed;
    uint8_t sleep;
    uint8_t treasure;
    uint16_t xpv;
    uint16_t corpseweight;
    uint16_t corpsevalue;
    uint8_t transformid;
    uint8_t startthing;
    uint8_t uniqueness;
    int16_t talkf;
    int16_t movef;
    int16_t meleef;
    int16_t strikef;
    int16_t specialf;
    uint32_t status;
    uint32_t immunity;
    chtype monchar;
    const char* monstring;
    const char* corpsestr;
    const char* meleestr;
};

struct monster : public monster_data {
    int attacked;
    int aux1;
    int aux2;
    int click;
    int x;
    int y;
    struct objectlist *possessions;
public:
    inline monster& operator= (const monster_data& v)	{ *implicit_cast<monster_data*>(this) = v; possessions = NULL; return (*this); }
};

struct monsterlist {
    struct monster *m;
    struct monsterlist *next;
};

struct player {
    uint8_t	str;
    uint8_t	con;
    uint8_t	dex;
    uint8_t	agi;
    uint8_t	iq;
    uint8_t	pow;
    uint8_t	maxstr;
    uint8_t	maxcon;
    uint8_t	maxdex;
    uint8_t	maxagi;
    uint8_t	maxiq;
    uint8_t	maxpow;
    uint32_t	xp;
    int16_t	level;
    int16_t	hp;
    uint16_t	maxhp;
    int16_t	hit;
    int16_t	dmg;
    int16_t	absorption;
    uint16_t	click;
    int16_t	defense;
    int16_t	food;
    int16_t	alignment;
    uint16_t	mana;
    uint16_t	maxmana;
    uint32_t	cash;
    int		sx, sy;			// sanctuary coordinates
    int		x, y;			// current player coordinates
    unsigned	packptr;
    uint32_t	options;
    uint16_t	itemweight;
    uint16_t	maxweight;
    uint8_t	speed;
    uint8_t	patron;
    char	preference;
    uint16_t	immunity[NUMIMMUNITIES];
    uint16_t	status[NUMSTATI];
    int8_t	rank[NUMRANKS];
    uint16_t	guildxp[NUMRANKS];
    char	name[32];
    char	meleestr[64];
    struct object* possessions[MAXITEMS];
    struct object* pack[MAXPACK];
};

struct objectlist {
    struct object *thing;
    struct objectlist *next;
};

// terrain locations
struct terrain {
    chtype base_terrain_type;
    chtype current_terrain_type;
    char aux;
    char status;
};

// dungeon locations
struct location {
    char p_locf;		// function executed when moved on
    unsigned char lstatus;	// seen,stopsrun,lit,secret,
    char roomnumber;		// so room can be named
    chtype locchar;		// terrain type
    chtype showchar;		// char instantaneously drawn for site
    int aux;			// signifies various things
    unsigned char buildaux;	// used in constructing level
    struct objectlist *things;
    struct monster *creature;
};

struct level {
    char depth;			// which level is this
    struct level *next;		// pointer to next level in dungeon
    struct location site[MAXWIDTH][MAXLENGTH];	// dungeon data
    char generated;		// has the level been made (visited) yet?
    char numrooms;		// number of rooms on level
    char tunnelled;		// amount of tunnelling done on this level
    struct monsterlist *mlist;	// List of monsters on level
    int environment;		// where kind of level is this?
    int last_visited;		// time player was last on this level
};

// random typedefs

typedef struct monsterlist mltype;
typedef mltype *pml;

typedef struct monster montype;
typedef montype *pmt;

typedef struct location loctype;
typedef loctype *plc;

typedef struct level levtype;
typedef levtype *plv;

typedef struct object objtype;
typedef objtype *pob;

typedef struct objectlist oltype;
typedef oltype *pol;
