// Omega is free software, distributed under the MIT license

#include "defs.h"
#include "glob.h"

//----------------------------------------------------------------------
//{{{ Objects

const object_data NullObject = { NO_THING, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, COMMON, I_NO_OP, NULL_ITEM, nullptr, nullptr, nullptr };

const struct object_data Objects[TOTALITEMS] = {
//     lev    plus frg   dmg    aux     frag     used  type  unique             	    objstr              cursestr
//   id  weight      chrg   hit     value bless             usef         objchar          truename
    { 0, 2,  10, 0, 10, 0, 0, 0,  0,   50, 0, 0, 0, COMMON, I_RAISE_PORTCULLIS, THING, "small grey box with red button", "disposeable garage door opener", "disposeable garage door opener"},
    { 1, 4,  20, 0, 20, 0, 0, 0,  0, 2000, 0, 0, 0, COMMON, I_NO_OP, THING, "some rocks", "uncut diamonds", "uncut diamonds"},
    { 2, 2,  15, 0, 10, 0, 0, 0,  0,   50, 0, 0, 0, COMMON, I_PICK, THING, "twisted piece of metal", "thieve's pick", "thieve's pick"},
    { 3, 1,  10, 0, 15, 0, 0, 0,  0, 1500, 0, 0, 0, COMMON, I_NO_OP, THING, "large green gem", "humongous emerald", "humongous emerald"},
    { 4, 4, 200, 0, 18, 0, 0, 0,  0, 1750, 0, 0, 0, COMMON, I_NO_OP, THING, "gold ingot", "gold ingot", "gold ingot"},
    { 5, 5,   5, 0,  1, 0, 0, 0,  0, 3000, 0, 0, 0, COMMON, I_NO_OP, THING, "old illegible documents", "bearer bonds", "bearer bonds"},
    { 6, 0, 100, 0,  1, 0, 0, 0,  0,    1, 0, 0, 0, COMMON, I_NO_OP, THING, "bucket of salt water", "bucket of salt water", "bucket of salt water"},
    { 7, 2,   3, 0, 10, 0, 0, 0,  0,   20, 0, 0, 0, COMMON, I_KEY, THING, "key", "magic key", "magic key"},
    { 8, 0,  30, 0,  8, 0, 0, 0,  6,   10, 0, 0, 0, COMMON, I_PERM_ILLUMINATE, THING, "torch", "torch", "torch"},
    { 9, 0,  10, 0, 12, 0, 0, 0,  0,    0, 0, 0, 0, COMMON, I_NO_OP, THING, "grot", "grot", "grot"},
    {10, 0,  10, 0, 12, 0, 0, 0,  0,    0, 0, 0, 0, COMMON, I_NO_OP, THING, "grot", "grot", "grot"},
    {11, 0,  10, 0, 12, 0, 0, 0,  0,    0, 0, 0, 0, COMMON, I_NO_OP, THING, "grot", "grot", "grot"},
    {12, 0,  10, 0, 12, 0, 0, 0,  0,    0, 0, 0, 0, COMMON, I_NO_OP, THING, "grot", "grot", "grot"},
    {13, 0,  10, 0, 12, 0, 0, 0,  0,    0, 0, 0, 0, COMMON, I_NO_OP, THING, "grot", "grot", "grot"},
    {14, 0,  10, 0, 12, 0, 0, 0,  0,    0, 0, 0, 0, COMMON, I_NO_OP, THING, "grot", "grot", "grot"},
    {15, 0,  10, 0, 12, 0, 0, 0,  0,    0, 0, 0, 0, COMMON, I_NO_OP, THING, "grot", "grot", "grot"},
    {16, 0,  10, 0, 12, 0, 0, 0,  0,    0, 0, 0, 0, UNIQUE_MADE, I_NO_OP, THING, "Justiciar's Badge", "Justiciar's Badge", "Justiciar's Badge"},
    {17, 0,  10, 0,  3, 0, 0, 0, 89,   30, 0, 0, 0, COMMON, I_TRAP, THING, "spring-loaded tube", "dart-trap component", "dart-trap component"},
    {18, 0,  10, 0,  3, 0, 0, 0, 98,   30, 0, 0, 0, COMMON, I_TRAP, THING, "pressurized cannister", "acid-trap component", "acid-trap component"},
    {19, 0,  10, 0,  3, 0, 0, 0, 92,   30, 0, 0, 0, COMMON, I_TRAP, THING, "coil of fine wire", "snare component", "snare component"},
    {20, 0,  10, 0,  3, 0, 0, 0, 94,   30, 0, 0, 0, COMMON, I_TRAP, THING, "mound of powdered aluminum", "fire-trap component", "fire-trap component"},
    {21, 0,  10, 0,  3, 0, 0, 0, 95,   30, 0, 0, 0, COMMON, I_TRAP, THING, "powerful rune", "teleport-trap component", "teleport-trap component"},
    {22, 0,  10, 0,  3, 0, 0, 0, 97,   30, 0, 0, 0, COMMON, I_TRAP, THING, "pressurized cannister", "sleepgas-trap component", "sleepgas-trap component"},
    {23, 0,  10, 0,  3, 0, 0, 0, 96,   30, 0, 0, 0, COMMON, I_TRAP, THING, "powerful rune", "disintegration-trap component", "disintegration-trap component"},
    {24, 0,  10, 0,  3, 0, 0, 0,100,   30, 0, 0, 0, COMMON, I_TRAP, THING, "dark fabric patch", "abyss-trap component", "abyss-trap component"},
    {25, 0,  10, 0,  3, 0, 0, 0, 99,   30, 0, 0, 0, COMMON, I_TRAP, THING, "powerful rune", "manadrain-trap component", "manadrain-trap component"},
    {26, 0,  20, 0,  0, 0, 0, 0,  8,    2, 0, 0, 0, COMMON, I_FOOD, FOOD, "red and white striped bucket", "food ration", "food ration"},
    {27, 2,  10, 0,  0, 0, 0, 0, 48,  200, 0, 0, 0, COMMON, I_LEMBAS, FOOD, "wafer of lembas", "wafer of lembas", "wafer of lembas"},
    {28, 3,   1, 0,  0, 0, 0, 0,  1,   50, 0, 0, 0, COMMON, I_STIM, FOOD, "small gray tablet", "stimtab", "stimtab"},
    {29, 3,   1, 0,  0, 0, 0, 0,  1,  100, 0, 0, 0, COMMON, I_POW, FOOD, "small gray tablet", "powtab", "powtab"},
    {30, 3,   1, 0,  0, 0, 0, 0, 36,   20, 0, 0, 0, COMMON, I_FOOD, FOOD, "small gray tablet", "nutritab", "nutritab"},
    {31, 3,   1, 0,  0, 0, 0, 0,  1,  500, 0, 0, 0, COMMON, I_IMMUNE, FOOD, "small gray tablet", "immunotab", "immunotab"},
    {32, 0,   1, 0,  0, 0, 0, 0,  2,    1, 0, 0, 0, COMMON, I_FOOD, FOOD, "jelly baby", "jelly baby", "jelly baby"},
    {33, 1,  20, 0,  0, 0, 0, 0, -1,   25, 0, 0, 0, COMMON, I_POISON_FOOD, FOOD, "red and white striped bucket", "poisoned ration", "poisoned ration"},
    {34, 2,   2, 0,  0, 0, 0, 0,  1,  100, 0, 0, 0, COMMON, I_PEPPER_FOOD, FOOD, "withered reddish stringy vegetable", "szechuan pepper", "szechuan pepper"},
    {35, 2,   2, 0,  0, 0, 0, 0,  8,    5, 0, 0, 0, COMMON, I_FOOD, FOOD, "messy red and white disk", "pepperoni pizza pie", "pepperoni pizza pie"},
    {36, 2,   2, 0,  0, 0, 0, 0,  6,    7, 0, 0, 0, COMMON, I_FOOD, FOOD, "brown bag with strange ideograms", "hunan take-out", "hunan take-out"},
    {37, 2,   2, 0,  0, 0, 0, 0,  2,    5, 0, 0, 0, COMMON, I_FOOD, FOOD, "strange foamy cylinders", "twinkies", "twinkies"},
    {38, 2,   2, 0,  0, 0, 0, 0,  3,    3, 0, 0, 0, COMMON, I_FOOD, FOOD, "squiggly orange spirals", "cheese-doodles", "cheese-doodles"},
    {39, 2,   2, 0,  0, 0, 0, 0,  4,    3, 0, 0, 0, COMMON, I_FOOD, FOOD, "sweet nutty-smelling disks", "pecan twirls", "pecan_twirls"},
    {40, 2,   2, 0,  0, 0, 0, 0, 10,   75, 0, 0, 0, COMMON, I_FOOD, FOOD, "pale white strips with green dots", "veal piccata with capers", "veal piccata with capers"},
    {41, 2,   2, 0,  0, 0, 0, 0,  3,    5, 0, 0, 0, COMMON, I_FOOD, FOOD, "a bag of hard brown kernels", "a bag of unmilled grain", "a bag of unmilled grain"},
    {42, 0,   5, 0,  1, 0, 0, 0,  0,    5, 0, 0, 0, COMMON, I_NOTHING, SCROLL, "scroll", "blank scroll", "Antinomy!"},
    {43, 1,   5, 0,  1, 0, 0, 0,  0,  250, 0, 0, 0, COMMON, I_SPELLS, SCROLL, "scroll", "scroll of spells", "scroll of spell loss"},
    {44, 2,   5, 0,  1, 0, 0, 0,  0,  400, 0, 0, 0, COMMON, I_BLESS, SCROLL, "scroll", "scroll of blessing", "scroll of cursing"},
    {45, 7,   5, 0,  1, 0, 0, 0,  0, 5000, 0, 0, 0, COMMON, I_ACQUIRE, SCROLL, "scroll", "scroll of acquirement", "scroll of destruction"},
    {46, 2,   5, 0,  1, 0, 0, 0,  0,  200, 0, 0, 0, COMMON, I_ENCHANT, SCROLL, "scroll", "scroll of enchantment", "scroll of disenchantment"},
    {47, 1,   5, 0,  1, 0, 0, 0,  0,   75, 0, 0, 0, COMMON, I_TELEPORT, SCROLL, "scroll", "scroll of teleportation", "scroll of teleportation"},
    {48, 10,  5, 0,  1, 0, 0, 0,  0,10000, 0, 0, 0, COMMON, I_WISH, SCROLL, "scroll", "scroll of wishing", "scroll of wishing"},
    {49, 1,   5, 0,  1, 0, 0, 0,  0,   25, 0, 0, 0, COMMON, I_CLAIRVOYANCE, SCROLL, "scroll", "scroll of clairvoyance", "scroll of amnesia"},
    {50, 2,   5, 0,  1, 0, 0, 0,  0,   50, 0, 0, 0, COMMON, I_DISPLACE, SCROLL, "scroll", "scroll of displacement", "scroll of vulnerability"},
    {51, 0,   5, 0,  1, 0, 0, 0,  0,   20, 0, 0, 0, COMMON, I_ID, SCROLL, "scroll", "scroll of identification", "scroll of forgetfulness"},
    {52, 0,   5, 0,  1, 0, 0, 0,  0,   10, 0, 0, 0, COMMON, I_HINT, SCROLL, "scroll", "hint sheet", "hint sheet"},
    {53, 2,   5, 0,  1, 0, 0, 0,  0,   50, 0, 0, 0, COMMON, I_JANE_T, SCROLL, "scroll", "Jane's guide to treasure", "Jane's guide to treasure"},
    {54, 1,   5, 0,  1, 0, 0, 0,  0,   50, 0, 0, 0, COMMON, I_ALERT, SCROLL, "scroll", "scroll of alertness", "scroll of drowsiness"},
    {55, 5,   5, 0,  1, 0, 0, 0,  0,  300, 0, 0, 0, COMMON, I_FLUX, SCROLL, "scroll", "scroll of cosmic flux", "scroll of cosmic flux"},
    {56, 2,   5, 0,  1, 0, 0, 0,  0,  100, 0, 0, 0, COMMON, I_CHARGE, SCROLL, "scroll", "scroll of charging", "scroll of draining"},
    {57, 3,   5, 0,  1, 0, 0, 0,  0,  100, 0, 0, 0, COMMON, I_WARP, SCROLL, "scroll", "scroll of the warp", "scroll of the warp"},
    {58, 1,   5, 0,  1, 0, 0, 0,  0,   10, 0, 0, 0, COMMON, I_KNOWLEDGE, SCROLL, "scroll", "scroll of self-knowledge", "scroll of delusion"},
    {59, 2,   5, 0,  1, 0, 0, 0,  0,  100, 0, 0, 0, COMMON, I_LAW, SCROLL, "scroll", "scroll of law", "scroll of law"},
    {60, 5,   5, 0,  1, 0, 0, 0,  0,  200, 0, 0, 0, COMMON, I_HERO, SCROLL, "scroll", "scroll of heroism", "scroll of cowardliness"},
    {61, 2,   5, 0,  1, 0, 0, 0,  0,   50, 0, 0, 0, COMMON, I_TRUESIGHT, SCROLL, "scroll", "scroll of true sight", "scroll of blindness"},
    {62, 3,   5, 0,  1, 0, 0, 0,  0,  250, 0, 0, 0, COMMON, I_DISPEL, SCROLL, "scroll", "scroll of dispelling", "scroll of self-dispelling"},
    {63, 2,   5, 0,  1, 0, 0, 0,  0,  300, 0, 0, 0, COMMON, I_SUMMON, SCROLL, "scroll", "scroll of summoning", "scroll of summoning"},
    {64, 0,   5, 0,  1, 0, 0, 0,  0,   30, 0, 0, 0, COMMON, I_ILLUMINATE, SCROLL, "scroll", "scroll of light", "scroll of darkness"},
    {65, 3,   5, 0,  1, 0, 0, 0,  0,  250, 0, 0, 0, COMMON, I_DEFLECT, SCROLL, "scroll", "scroll of deflection", "scroll of vulnerability"},
    {66, 0,  20, 0,  5, 0, 0, 0,  0,    0, 0, 0, 0, COMMON, I_NO_OP, POTION, "potion", "potion of quenching thirst", "Antinomy!"},
    {67, 2,  20, 0,  5, 0, 0, 0,  0,   40, 0, 0, 0, COMMON, I_HEAL, POTION, "potion", "potion of healing", "potion of disruption"},
    {68, 1,  20, 0,  5, 0, 0, 0,  0,   10, 0, 0, 0, COMMON, I_OBJDET, POTION, "potion", "potion of object detection", "potion of object detection"},
    {69, 1,  20, 0,  5, 0, 0, 0,  0,   10, 0, 0, 0, COMMON, I_MONDET, POTION, "potion", "potion of monster detection", "potion of monster detection"},
    {70, 2,  20, 0,  5, 0, 0, 0,  0,   20, 0, 0, 0, COMMON, I_NEUTRALIZE_POISON, POTION, "potion", "potion of neutralize poison", "potion of poison"},
    {71, 0,  20, 0,  5, 0, 0, 0,  0,  100, 0, 0, 0, COMMON, I_ALERT, POTION, "potion", "potion of alertness", "potion of sleep"},
    {72, 1,  20, 0,  5, 0, 0, 0,  0,   40, 0, 0, 0, COMMON, I_SPEED, POTION, "potion", "potion of speed", "potion of slowness"},
    {73, 3,  20, 0,  5, 0, 0, 0,  0,   50, 0, 0, 0, COMMON, I_RESTORE, POTION, "potion", "potion of restoration", "potion of debilitation"},
    {74, 7,  20,-1,  5, 0, 0, 0,  0, 1500, 0, 0, 0, COMMON, I_AZOTH, POTION, "potion", "Azoth", "mercury"},
    {75, 4,  20, 0,  5, 0, 0, 0,  0,  100, 0, 0, 0, COMMON, I_REGENERATE, POTION, "potion", "potion of regeneration", "potion of great disruption"},
    {76, 2,  20, 0,  5, 0, 0, 0,  0,  100, 0, 0, 0, COMMON, I_INVISIBLE, POTION, "potion", "potion of invisibility", "potion of aggravation"},
    {77, 2,  20, 0,  5, 0, 0, 0,  0,   30, 0, 0, 0, COMMON, I_BREATHING, POTION, "potion", "potion of breathing", "potion of choking"},
    {78, 5,  20, 0,  5, 0, 0, 0,  0,  200, 0, 0, 0, COMMON, I_AUGMENT, POTION, "potion", "potion of augmentation", "potion of debilitation"},
    {79, 2,  20, 0,  5, 0, 0, 0,  0,   50, 0, 0, 0, COMMON, I_FEAR_RESIST, POTION, "potion", "potion of banishing fear", "potion of fear"},
    {80, 3,  20, 0,  5, 0, 0, 0,  0,  100, 0, 0, 0, COMMON, I_CHAOS, POTION, "potion", "potion of chaos", "Antinomy!"},
    {81, 3,  20, 0,  5, 0, 0, 0,  0,  200, 0, 0, 0, COMMON, I_ACCURACY, POTION, "potion", "potion of accuracy", "potion of fuzziness"},
    {82, 1,  20, 0,  5, 0, 0, 0,  0,   50, 0, 0, 0, COMMON, I_LEVITATION, POTION, "potion", "potion of levitation", "potion of levitation"},
    {83, 1,  20, 0,  5, 0, 0, 0,  0,   50, 0, 0, 0, COMMON, I_CURE, POTION, "potion", "potion of curing", "potion of disease"},
    {84, 0,  10, 0, 15, 0, 6,12,  0,    5, 0, 0, THRUSTING, COMMON, I_NORMAL_WEAPON, WEAPON, "dagger", "dagger", "dagger"},
    {85, 1,  25, 0, 15, 0, 8,11,  0,   40, 0, 0, CUTTING, COMMON, I_NORMAL_WEAPON, WEAPON, "short sword", "short sword", "short sword"},
    {86, 2,  40, 0, 15, 0,12, 9,  0,   90, 0, 0, CUTTING, COMMON, I_NORMAL_WEAPON, WEAPON, "broad sword", "broad sword", "broad sword"},
    {87, 3,  50, 0, 15, 0,16, 8,  0,  250, 0, 0, CUTTING, COMMON, I_NORMAL_WEAPON, WEAPON, "bastard sword", "bastard sword", "bastard sword"},
    {88, 4,  50, 0, 15, 0,16,10,  0,  500, 0, 0, CUTTING, COMMON, I_NORMAL_WEAPON, WEAPON, "katana", "katana", "katana"},
    {89, 5,  75, 0, 15, 0,20, 7,  0,  400, 0, 0, CUTTING, COMMON, I_NORMAL_WEAPON, WEAPON, "great-sword", "great-sword", "great-sword"},
    {90, 2,  15, 0, 15, 0, 6,15,  0,   50, 0, 0, THRUSTING, COMMON, I_NORMAL_WEAPON, WEAPON, "epee", "epee", "epee"},
    {91, 3,  20, 0, 15, 0, 8,13,  0,  250, 0, 0, THRUSTING, COMMON, I_NORMAL_WEAPON, WEAPON, "rapier", "rapier", "rapier"},
    {92, 5,  25, 0, 15, 0,11,12,  0,  500, 0, 0, THRUSTING, COMMON, I_NORMAL_WEAPON, WEAPON, "estoc", "estoc", "estoc"},
    {93, 1,  35, 0, 15, 0, 8, 8,  0,   40, 0, 0, CUTTING, COMMON, I_NORMAL_WEAPON, WEAPON, "cutlass", "cutlass", "cutlass"},
    {94, 1,  25, 0, 15, 0, 8,10,  0,   50, 0, 0, CUTTING, COMMON, I_NORMAL_WEAPON, WEAPON, "hand-axe", "hand-axe", "hand-axe"},
    {95, 2,  50, 0, 15, 0,12, 8,  0,  100, 0, 0, CUTTING, COMMON, I_NORMAL_WEAPON, WEAPON, "battle-axe", "battle-axe", "battle-axe"},
    {96, 5, 100, 0, 15, 0,24, 7,  0,  200, 0, 0, CUTTING, COMMON, I_NORMAL_WEAPON, WEAPON, "great-axe", "great-axe", "great-axe"},
    {97, 2,  50, 0, 15, 0,12, 8,  0,   50, 0, 0, STRIKING, COMMON, I_NORMAL_WEAPON, WEAPON, "mace", "mace", "mace"},
    {98, 2,  50, 0, 15, 0,14, 6,  0,   60, 0, 0, STRIKING, COMMON, I_NORMAL_WEAPON, WEAPON, "war-hammer", "war-hammer", "war-hammer"},
    {99, 5,  60, 0, 15, 0,16, 7,  0,  150, 0, 0, STRIKING, COMMON, I_NORMAL_WEAPON, WEAPON, "morning-star", "morning-star", "morning-star"},
    {100, 3, 50, 0, 15, 0,12, 7,  0,   50, 0, 0, STRIKING, COMMON, I_NORMAL_WEAPON, WEAPON, "flail", "flail", "flail"},
    {101, 0, 30, 0, 15, 0, 6,10,  0,    5, 0, 0, STRIKING, COMMON, I_NORMAL_WEAPON, WEAPON, "club", "club", "club"},
    {102, 1, 80, 0, 15, 0, 9,12,  0,   30, 0, 0, STRIKING, COMMON, I_NORMAL_WEAPON, WEAPON, "quarterstaff", "quarterstaff", "quarterstaff"},
    {103, 2, 50, 0, 15, 0,10,10,  0,   50, 0, 0, THRUSTING, COMMON, I_NORMAL_WEAPON, WEAPON, "spear", "spear", "spear"},
    {104, 3,100, 0, 15, 0,16, 6,  0,  100, 0, 0, CUTTING, COMMON, I_NORMAL_WEAPON, WEAPON, "halberd", "halberd", "halberd"},
    {105, 3, 80, 0, 15, 0,12, 7,  0,   75, 0, 0, THRUSTING, COMMON, I_NORMAL_WEAPON, WEAPON, "trident", "trident", "trident"},
    {106, 2, 20, 0, 15, 0, 4, 8,1005, 100, 0, 0, STRIKING, COMMON, I_NORMAL_WEAPON, WEAPON, "whip", "whip", "whip"},
    {107, 9, 20, 0, 15, 0,30,20,1002,2000, 0, 0, THRUSTING, COMMON, I_LIGHTSABRE, WEAPON, "grey metal cylinder", "lightsabre", "lightsabre"},
    {108, 9,500, 8, 15, 0,16, 8,1003,1000, 0, -8, CUTTING, COMMON, I_DEMONBLADE, WEAPON, "bastard sword", "Demonblade", "Demonblade"},
    {109, 7,250, 7, 15, 0,17,10,1004,3000, 0, 7, STRIKING, COMMON, I_MACE_DISRUPT, WEAPON, "mace", "mace of disruption", "mace"},
    {110, 2,100, 0, 15, 0,12,15,  0,  300, 0, 0, MISSILE, COMMON, I_NORMAL_WEAPON, MISSILEWEAPON, "longbow", "longbow", "longbow"},
    {111, 3,150, 0, 15, 0,20,15,  0,  500, 0, 0, MISSILE, COMMON, I_NORMAL_WEAPON, MISSILEWEAPON, "crossbow", "crossbow", "crossbow"},
    {112, 1,  2, 0, 15, 0, 3, 3,1006,   2, 0, 0, MISSILE, COMMON, I_NORMAL_WEAPON, MISSILEWEAPON, "arrow", "arrow", "arrow"},
    {113, 1,  2, 0, 15, 0, 3, 0,1007,   5, 0, 0, MISSILE, COMMON, I_NORMAL_WEAPON, MISSILEWEAPON, "bolt", "bolt", "bolt"},
    {114, 2, 50, 0, 15, 0, 3,10,1005,  50, 0, 0, MISSILE, COMMON, I_NORMAL_WEAPON, MISSILEWEAPON, "bola", "bola", "bola"},
    {115, 7, 40, 5, 15, 0,12, 9,1008,3000, 0, 0, CUTTING, COMMON, I_NORMAL_WEAPON, WEAPON, "broad sword", "vorpal sword", "sword of self-mutilation"},
    {116, 9,100, 7, 15, 0,20, 7,1009,5000, 0, -7, CUTTING, COMMON, I_DESECRATE, WEAPON, "great-sword", "Desecrator", "Desecrator"},
    {117, 6, 60, 7, 15, 0,16, 7,1010,2000, 0, 0, STRIKING, COMMON, I_PERM_FIRE_RESIST, WEAPON, "morning-star", "firestar", "firestar"},
    {118, 7, 50, 7, 15, 0,10,12,1011,3000, 0, 7, THRUSTING, COMMON, I_DEFEND, WEAPON, "estoc", "Defender", "Defender"},
    {119,10, 50,10, 15,0,100,10,1012,5000, 0, 10, THRUSTING, UNIQUE_MADE, I_VICTRIX, WEAPON, "ivory spear", "Victrix", "Victrix"},
    {120, 7,500, 6, 15, 0,32, 6,  0, 2000, 0, 0, CUTTING, UNIQUE_MADE, I_NORMAL_WEAPON, WEAPON, "great-axe", "Goblins' Hewer", "Goblins' Hewer"},
    {121, 6,100,20, 15, 0,30, 10, 0,  500, 0, -10, CUTTING, COMMON, I_NORMAL_WEAPON, WEAPON, "scythe", "scythe of slicing", "scythe of slicing"},
    {122, 5,250, 0, 15, 0,50,10,  0,  100, 0, 0, STRIKING, COMMON, I_NORMAL_WEAPON, WEAPON, "giant club", "giant club", "giant club"},
    {123,10,500,0,0,15,255,255,1014, 1000, 0, 0, CUTTING, UNIQUE_MADE, I_NORMAL_WEAPON, WEAPON, "Scythe of Death", "Scythe of Death", "Scythe of Death"},
    {124, 4, 30, 0, 15, 0,16,16,1015,1000, 0, 0, STRIKING, COMMON, I_NORMAL_WEAPON, WEAPON, "whip", "acid whip", "acid whip"},
    {125, 0,100, 0, 20, 0, 1, 0,  0,    5, 0, 0, 0, COMMON, I_NORMAL_ARMOR, ARMOR, "flak jacket", "flak jacket", "flak jacket"},
    {126, 1,200, 0, 20, 0, 2, 0,  1,   10, 0, 0, 0, COMMON, I_NORMAL_ARMOR, ARMOR, "soft leather armor", "soft leather armor", "soft leather armor"},
    {127, 1,200, 0, 20, 0, 2, 0,  1,   30, 0, 0, 0, COMMON, I_NORMAL_ARMOR, ARMOR, "cuirbouilli", "cuirbouilli", "cuirbouilli"},
    {128, 2,300, 0, 20, 0, 3, 0,  2,  100, 0, 0, 0, COMMON, I_NORMAL_ARMOR, ARMOR, "ring mail", "ring mail", "ring mail"},
    {129, 3,400, 0, 20, 0, 4, 0,  3,  200, 0, 0, 0, COMMON, I_NORMAL_ARMOR, ARMOR, "chain mail", "chain mail", "chain mail"},
    {130, 4,400, 0, 20, 0, 4, 0,  2,  300, 0, 0, 0, COMMON, I_NORMAL_ARMOR, ARMOR, "scale mail", "scale mail", "scale mail"},
    {131, 5,400, 0, 20, 0, 5, 0,  3,  450, 0, 0, 0, COMMON, I_NORMAL_ARMOR, ARMOR, "partial plate mail", "partial plate mail", "partial plate mail"},
    {132, 6,600, 0, 20, 0, 6, 0,  4,  750, 0, 0, 0, COMMON, I_NORMAL_ARMOR, ARMOR, "full plate mail", "full plate mail", "full plate mail"},
    {133, 6,500, 0, 20, 0, 7, 0,  4, 1500, 0, 0, 0, COMMON, I_NORMAL_ARMOR, ARMOR, "plate armor", "plate armor", "plate armor"},
    {134, 6,200, 0, 20, 0, 5, 0,  2, 1500, 0, 0, 0, COMMON, I_NORMAL_ARMOR, ARMOR, "lamellar armor", "lamellar armor", "lamellar armor"},
    {135, 7,200, 0, 20, 0, 5, 0,  2, 2000, 0, 0, 0, COMMON, I_PERM_FEAR_RESIST, ARMOR, "mithril chain mail", "mithril chain mail", "mithril chain mail"},
    {136, 7,300, 0, 20, 0, 8, 0,  3, 3000, 0, 0, 0, COMMON, I_PERM_FEAR_RESIST, ARMOR, "mithril plate armor", "mithril plate armor", "mithril plate armor"},
    {137, 7,500, 0, 20, 0, 6, 0,  3, 3000, 0, 0, 0, COMMON, I_PERM_FIRE_RESIST, ARMOR, "dragonscale armor", "dragonscale armor", "dragonscale armor"},
    {138, 8,100, 9, 20, 0, 7, 0,  2, 1000, 0, 0, 0, COMMON, I_PERM_ENERGY_RESIST, ARMOR, "prismatrix armor", "prismatrix armor", "prismatrix armor"},
    {139, 9,  0, 0, 20, 0,20, 0,  0, 5000, 0, 0, 0, COMMON, I_PERM_BREATHING, ARMOR, "powered combat armor", "powered combat armor", "powered combat armor"},
    {140, 7,  0, 0, 20, 0,10, 0,  0, 3000, 0, 0, 0, COMMON, I_PERM_BREATHING, ARMOR, "powered scout armor", "powered scout armor", "powered scout armor"},
    {141, 3,100, 5, 20, 0, 5, 0,  0, 2000, 0, -9, 0, COMMON, I_PERM_ENERGY_RESIST, ARMOR, "demonhide armor", "demonhide armor", "demonhide armor"},
    {142, 0, 20, 0, 15, 0, 0, 0,  1,   10, 0, 0, 0, COMMON, I_NORMAL_SHIELD, SHIELD, "buckler", "buckler", "buckler"},
    {143, 1, 40, 0, 15, 0, 0, 0,  2,   20, 0, 0, 0, COMMON, I_NORMAL_SHIELD, SHIELD, "small round shield", "small round shield", "small round shield"},
    {144, 2,100, 0, 15, 0, 0, 0,  3,   50, 0, 0, 0, COMMON, I_NORMAL_SHIELD, SHIELD, "large round shield", "large round shield", "large round shield"},
    {145, 2,200, 0, 15, 0, 0, 0,  4,   75, 0, 0, 0, COMMON, I_NORMAL_SHIELD, SHIELD, "heater shield", "heater shield", "heater shield"},
    {146, 3,300, 0, 15, 0, 0, 0,  5,  150, 0, 0, 0, COMMON, I_NORMAL_SHIELD, SHIELD, "hoplon", "hoplon", "hoplon"},
    {147, 4,500, 0, 15, 0, 0, 0,  6,  200, 0, 0, 0, COMMON, I_NORMAL_SHIELD, SHIELD, "tower shield", "tower shield", "tower shield"},
    {148, 6, 20, 0, 15, 0, 0, 0,  7, 1000, 0, 0, 0, COMMON, I_NORMAL_SHIELD, SHIELD, "plasteel shield", "plasteel shield", "plasteel shield"},
    {149, 7,200, 6, 15, 0, 0, 0,  6, 2000, 0, 0, 0, COMMON, I_PERM_DEFLECT, SHIELD, "shield of deflection", "shield of deflection", "shield of deflection"},
    {150, 0, 30, 0,  4, 0, 0, 0,  0,   15, 0, 0, 0, COMMON, I_NO_OP, CLOAK, "cloak", "cloak of wool", "cloak of wool"},
    {151, 3, 30, 0,  4, 0, 0, 0,  0,  500, 0, 0, 0, COMMON, I_PERM_NEGIMMUNE, CLOAK, "cloak", "cloak of negimmunity", "cloak of level drain"},
    {152, 5, 30, 0,  4, 0, 0, 0,  0,  500, 0, 0, 0, COMMON, I_PERM_INVISIBLE, CLOAK, "cloak", "cloak of invisibility", "cloak of invisibility"},
    {153, 5, 30, 0,  4, 0, 0, 0,  0, 1000, 0, 0, 0, COMMON, I_PERM_ACCURACY, CLOAK, "cloak", "cloak of skill", "cloak of skill"},
    {154, 3, 30, 2,  4, 0, 0, 0,  0,  500, 0, 0, 0, COMMON, I_PERM_PROTECTION, CLOAK, "cloak", "cloak of protection", "cloak of damage"},
    {155, 6, 30, 0,  4, 0, 0, 0,  0, 2000, 0, 0, 0, COMMON, I_PERM_DISPLACE, CLOAK, "cloak", "cloak of displacement", "cloak of vulnerability"},
    {156, 3, 30, 0,  4, 0, 0, 0,  0,  500, 0, 0, 0, COMMON, I_PERM_TRUESIGHT, CLOAK, "cloak", "cloak of true sight", "cloak of blindness"},
    {157, 4, 30, 0,  6, 0, 0, 0,  0, 1000, 0, 0, 0, COMMON, I_PERM_SPEED, BOOTS, "boots", "boots of speed", "boots of slowness"},
    {158, 6, 30, 0,  6, 0, 0, 0,  0, 1000, 0, 0, 0, COMMON, I_PERM_HERO, BOOTS, "boots", "boots of heroism", "boots of cowardliness"},
    {159, 3, 30, 0,  6, 0, 0, 0,  0,  500, 0, 0, 0, COMMON, I_PERM_LEVITATE, BOOTS, "boots", "boots of levitation", "boots of levitation"},
    {160, 3, 30, 0,  6, 0, 0, 0,  0,  250, 0, 0, 0, COMMON, I_PERM_AGILITY, BOOTS, "boots", "boots of agility", "boots of clumsiness"},
    {161, 2, 30, 0,  6, 0, 0, 0,  0,  200, 0, 0, 0, COMMON, I_BOOTS_JUMPING, BOOTS, "boots", "jumping boots", "jumping boots"},
    {162, 0, 30, 0,  6, 0, 0, 0,  0,    7, 0, 0, 0, COMMON, I_NO_OP, BOOTS, "boots", "boots of leather", "boots of leather"},
    {163, 7, 30, 0,  6, 0, 0, 0,  0, 2700, 0, 0, 0, COMMON, I_BOOTS_7LEAGUE, BOOTS, "boots", "seven league boots", "umpteen league boots"},
    {164, 3,  1, 0, 10, 0, 0, 0,  0,  400, 0, 0, 0, COMMON, I_PERM_TRUESIGHT, RING, "ring", "ring of truesight", "ring of blindness"},
    {165, 0,  1, 0, 10, 0, 0, 0,  0,    1, 0,-1, 0, COMMON, I_PERM_BURDEN, RING, "ring", "ring of burden", "ring of burden"},
    {166, 1,  1, 0, 10, 0, 0, 0,  0,  100, 0, 0, 0, COMMON, I_PERM_STRENGTH, RING, "ring", "ring of strength", "ring of strength"},
    {167, 2,  1, 0, 10, 0, 0, 0,  0,  100, 0, 0, 0, COMMON, I_PERM_GAZE_IMMUNE, RING, "ring", "ring of gaze immunity", "ring of gaze immunity"},
    {168, 3,  1, 0, 10, 0, 0, 0,  0,  100, 0, 0, 0, COMMON, I_PERM_FIRE_RESIST, RING, "ring", "ring of fire resistance", "ring of fire resistance"},
    {169, 0,  1, 0, 10, 0, 0, 0,  0,   10, 0, 0, 0, COMMON, I_NO_OP, RING, "ring", "ring of brass and glass", "ring of brass and glass"},
    {170, 4,  1, 0, 10, 0, 0, 0,  0,  100, 0, 0, 0, COMMON, I_PERM_POISON_RESIST, RING, "ring", "ring of poison resistance", "ring of poison"},
    {171, 5,  1, 0, 10, 0, 0, 0,  0, 1000, 0, 0, 0, COMMON, I_PERM_REGENERATE, RING, "ring", "ring of regeneration", "ring of regeneration"},
    {172, 0,  1, 0, 10, 0, 0, 0,  0,  100, 0, 0, 0, COMMON, I_PERM_KNOWLEDGE, RING, "ring", "ring of self-knowledge", "ring of delusion"},
    {173, 4,  1, 0, 10, 0, 0, 0,  0,  500, 0, 0, 0, COMMON, I_PERM_PROTECTION, RING, "ring", "ring of protection", "ring of vulnerability"},
    {174, 3, 80, 0, 10, 0, 0, 0,  0,  500, 0, 0, 0, COMMON, I_FIREBOLT, STICK, "staff", "staff of firebolts", "staff of firebolts"},
    {175, 0, 80, 0, 10, 0, 0, 0,  0,   10, 0, 0, 0, COMMON, I_NOTHING, STICK, "stick", "walking stick", "walking stick"},
    {176, 1, 80, 0, 10, 0, 0, 0,  0,  100, 0, 0, 0, COMMON, I_SLEEP_OTHER, STICK, "staff", "staff of sleep", "staff of sleep"},
    {177, 4, 80, 0, 10, 0, 0, 0,  0,  500, 0, 0, 0, COMMON, I_LBALL, STICK, "wand", "wand of ball lightning", "wand of ball lightning"},
    {178, 2, 80, 0, 10, 0, 0, 0,  0,  500, 0, 0, 0, COMMON, I_SUMMON, STICK, "rod", "rod of summoning", "rod of summoning"},
    {179, 1, 80, 0, 10, 0, 0, 0,  0,  100, 0, 0, 0, COMMON, I_HIDE, STICK, "rod", "rod of hiding", "rod of hiding"},
    {180, 3, 80, 0, 10, 0, 0, 0,  0,  500, 0, 0, 0, COMMON, I_LBOLT, STICK, "staff", "staff of lightning bolts", "staff of lightning bolts"},
    {181, 5, 80, 0, 10, 0, 0, 0,  0,  500, 0, 0, 0, COMMON, I_FIREBALL, STICK, "wand", "wand of fireballs", "wand of fireballs"},
    {182, 7, 80, 0, 10, 0, 0, 0,  0, 2000, 0, 0, 0, COMMON, I_DISINTEGRATE, STICK, "rod", "rod of disintegration", "rod of disintegration"},
    {183, 6, 80, 0, 10, 0, 0, 0,  0, 1000, 0, 0, 0, COMMON, I_DISRUPT, STICK, "staff", "staff of disruption", "staff of disruption"},
    {184, 2, 80, 0, 10, 0, 0, 0,  0,  100, 0, 0, 0, COMMON, I_SNOWBALL, STICK, "stick", "snowball stick", "snowball stick"},
    {185, 1, 80, 0, 10, 0, 0, 0,  0,   50, 0, 0, 0, COMMON, I_MISSILE, STICK, "staff", "staff of missiles", "staff of missiles"},
    {186, 2, 80, 0, 10, 0, 0, 0,  0,  200, 0, 0, 0, COMMON, I_APPORT, STICK, "rod", "rod of apportation", "rod of lossage"},
    {187, 6, 80, 0, 10, 0, 0, 0,  0,  750, 0, 0, 0, COMMON, I_DISPEL, STICK, "staff", "staff of dispelling", "staff of self-dispelling"},
    {188, 3, 80, 0, 10, 0, 0, 0,  0,  500, 0, 0, 0, COMMON, I_HEAL, STICK, "staff", "staff of healing", "staff of harming"},
    {189, 3, 80, 0, 10, 0, 0, 0,  0, 1000, 0, 0, 0, COMMON, I_POLYMORPH, STICK, "wand", "wand of polymorph", "wand of stupidity"},
    {190, 2, 80, 0, 10, 0, 0, 0,  0,  500, 0, 0, 0, COMMON, I_FEAR, STICK, "wand", "wand of fear", "wand of fear"},
    {191,10,500, 0, 15, 0, 0, 0,  0,10000, 0, 0, 0, UNIQUE_MADE, I_ORBMASTERY, ARTIFACT, "Mysterious Orb", "Orb of Mastery", "Orb of Mastery"},
    {192, 9, 50, 0, 15, 0, 0, 0,  0, 2000, 0, 0, 0, COMMON, I_ORBFIRE, ARTIFACT, "Mysterious Orb", "Orb of Fire", "Orb of Fire"},
    {193, 8, 50, 0, 15, 0, 0, 0,  0, 2000, 0, 0, 0, COMMON, I_ORBWATER, ARTIFACT, "Mysterious Orb", "Orb of Water", "Orb of Water"},
    {194, 7, 50, 0, 15, 0, 0, 0,  0, 2000, 0, 0, 0, COMMON, I_ORBEARTH, ARTIFACT, "Mysterious Orb", "Orb of Earth", "Orb of Earth"},
    {195, 6, 50, 0, 15, 0, 0, 0,  0, 2000, 0, 0, 0, COMMON, I_ORBAIR, ARTIFACT, "Mysterious Orb", "Orb of Air", "Orb of Air"},
    {196, 1, 25, 0, 15, 0, 0, 0,  0,  100, 0, 0, 0, COMMON, I_ORBDEAD, ARTIFACT, "Black Orb", "Burnt-out Orb", "Burnt-out Orb"},
    {197, 3, 75, 0, 15, 0, 0, 0,  0,  500, 0, 0, 0, COMMON, I_CRYSTAL, ARTIFACT, "Glass Orb", "Crystal Ball", "Ball of Mindlessness"},
    {198, 5, 25, 0, 15, 0, 0, 0,  0, 1000, 0, 0, 0, COMMON, I_ANTIOCH, ARTIFACT, "Metal Pineapple", "Holy Hand-Grenade of Antioch", "Holy Hand-Grenade of Antioch"},
    {199, 1,500, 0, 15, 0, 0, 0,  0,    1, 0, 0, 0, UNIQUE_UNMADE, I_NOTHING, ARTIFACT, "Mystic Amulet", "Amulet of Yendor", "Amulet of Yendor"},
    {200, 7,  1, 0, 15, 0, 0, 0,  0,20000, 0, 0, 0, UNIQUE_MADE, I_KOLWYNIA, ARTIFACT, "Key", "Kolwynia, the Key That Was Lost", "Kolwynia, the Key That was Lost"},
    {201, 3, 10, 0, 15, 0, 0, 0,  0,  500, 0, 0, 0, UNIQUE_UNMADE, I_DEATH, ARTIFACT, "potion", "Potion of Death", "Potion of Death"},
    {202, 5,100, 0, 15, 0, 0, 0,  0, 2000, 0, 0, 0, UNIQUE_UNMADE, I_ENCHANTMENT, ARTIFACT, "Scintillating Staff", "Staff of Enchantment", "Staff of Enchantment"},
    {203, 2,  0, 0, 15, 0, 0, 0,  0, 1500, 0, 0, 0, UNIQUE_UNMADE, I_HELM, ARTIFACT, "Strange Weightless Helm", "Helm of Teleportation", "Helm of Teleportation"},
    {204, 6, 10, 0, 15, 0, 0, 0,  0,  500, 0, 0, 0, UNIQUE_UNMADE, I_LIFE, ARTIFACT, "potion", "Potion of Life", "Potion of Life"},
    {205,10,  5, 0, 15, 0, 0, 0,  1, 5000, 0, 0, 0, UNIQUE_MADE, I_SYMBOL, ARTIFACT, "Silver Gallows", "Holy Symbol of Odin", "Holy Symbol of Odin"},
    {206,10,  5, 0, 15, 0, 0, 0,  2, 5000, 0, 0, 0, UNIQUE_MADE, I_SYMBOL, ARTIFACT, "Jet Scarab", "Holy Symbol of Set", "Holy Symbol of Set"},
    {207,10,  5, 0, 15, 0, 0, 0,  3, 5000, 0, 0, 0, UNIQUE_MADE, I_SYMBOL, ARTIFACT, "Gold Owl Pendant", "Holy Symbol of Athena", "Holy Symbol of Athena"},
    {208,10,  5, 0, 15, 0, 0, 0,  4, 5000, 0, 0, 0, UNIQUE_MADE, I_SYMBOL, ARTIFACT, "Obsidian Crescent", "Holy Symbol of Hecate", "Holy Symbol of Hecate"},
    {209,10,  5, 0, 15, 0, 0, 0,  5, 5000, 0, 0, 0, UNIQUE_MADE, I_SYMBOL, ARTIFACT, "Sprig of Mistletoe", "Holy Symbol of Druidism", "Holy Symbol of Druidism"},
    {210,10,  5, 0, 15, 0, 0, 0,  6, 5000, 0, 0, 0, UNIQUE_MADE, I_SYMBOL, ARTIFACT, "Blue Crystal Pendant", "Holy Symbol of Destiny", "Holy Symbol of Destiny"},
    {211, 6,1000,0, 15, 0, 0, 0,  0, 1000, 0, 0, 0, UNIQUE_UNMADE, I_JUGGERNAUT, ARTIFACT, "Crudely Carved Monolith", "Juggernaut of Karnak", "Juggernaut of Karnak"},
    {212, 7, 10, 0,  0, 0, 0, 0,  0,10000, 0, 0, 0, UNIQUE_MADE, I_STARGEM, ARTIFACT, "Strangely Glowing Crystal", "Star Gem", "Star Gem"},
    {213,10,1000,0, 10, 0, 0, 0,  0, 1000, 0, 0, 0, UNIQUE_MADE, I_SCEPTRE, ARTIFACT, "Extremely Heavy Stick ", "Sceptre of High Magic", "Sceptre of High Magic"},
    {214,10, 10, 0, 10, 0, 0, 0,  0, 1000, 0, 0, 0, UNIQUE_MADE, I_PLANES, ARTIFACT, "Octagonal Copper Amulet ", "Amulet of the Planes", "Amulet of the Planes"},
    {215, 0,  0, 0, 10, 0, 0, 0,  0,    0, 0, 0, 0, COMMON, I_NO_OP, CASH, "money", "money", "money"},
    {216, 0,100, 0, 10, 0, 0, 0,  0,    0, 0, 0, 0, COMMON, I_CORPSE, CORPSE, "a mysterious corpse", "a mysterious corpse", "a mysterious corpse"}
};

//}}}-------------------------------------------------------------------
//{{{ Monsters

const struct monster_data Monsters[NUMMONSTERS] = {
//    id     hp      ac   sense    lev    sleep     xpv     value   thing
//      lev     hit     dmg    wake  speed   treas     weight  transf
    {  0, 0,  1,  1, 10,  2,  4,  1,  1,  50, 0,   1,    1,  1, NO_THING, NO_THING, COMMON, M_TALK_STUPID, M_MOVE_FLUTTER, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | FLYING | POISONOUS, 0, 'h' | CLR_YELLOW_BLACK, "hornet", "dead hornet", "AXAX"},
    {  1, 0, 30,  0,  0,  0, 10, 10,  5, 100, 2,  15, 1000,  0, NO_THING, NO_THING, COMMON, M_TALK_MP, M_MOVE_FOLLOW, M_MELEE_MP, M_NO_OP, M_SP_MP, MOBILE | NEEDY, 0, '@' | CLR_RED_BLACK, "mendicant priest", "dead mendicant priest", "BX"},
    {  2, 0, 15,  0,  0,  0, 10, 10,  7,   0, 3,   5, 1000,  0, NO_THING, NO_THING, COMMON, M_TALK_IM, M_MOVE_FOLLOW, M_NO_OP, M_NO_OP, M_SP_ESCAPE, MOBILE, 0, '@' | CLR_RED_BLACK, "itinerant merchant", "dead itinerant merchant", "BX"},
    {  3, 0,150, 20, 20, 25, 10,  1,  3,  50, 0, 150, 1000,  0, NO_THING, WEAPON_GREAT_SWORD, COMMON, M_TALK_GUARD, M_MOVE_SMART, M_MELEE_NORMAL, M_NO_OP, M_SP_WHISTLEBLOWER, ASLEEP | MOBILE, 0, 'G' | CLR_RED_BLACK, "guardsman", "dead guardsman", "A?A?A?B?B?B?"},
    {  4, 0,  5,  5,  5,  5,  5,  5,  5,   5, 0,   5, 1000,  0, NO_THING, NO_THING, COMMON, M_TALK_MAN, M_MOVE_SMART, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, HOSTILE, 0, '@' | CLR_RED_BLACK, "Log NPC", "dead NPC", "A?B?"},
    {  5, 0,  5,  5,  5,  1,  2,  1,  5,  25, 0,   2,  200, 10, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_RANDOM, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | SWIMMING | EDIBLE, 0, 's' | CLR_WHITE_BLACK, "sheep", "side of mutton", "ALBX"},
    {  6, 0, 16,  6,  6,  1,  2,  1,  6,   0, 0,   6,  200,  0, NO_THING, NO_THING, COMMON, M_TALK_MERCHANT, M_NO_OP, M_MELEE_NORMAL, M_NO_OP, M_SP_MERCHANT, MOBILE, 0, '@' | CLR_RED_BLACK, "merchant", "dead merchant", "A?B?"},
    {  7, 0, 10,  5,  5,  5,  5,  5,  5,  25, 1,   5, 1800,  0, NO_THING, NO_THING, COMMON, M_TALK_MAN, M_MOVE_SMART, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | WANDERING, 0, '@' | CLR_RED_BLACK, "0th level NPC", "dead 0th level NPC", "AXBX"},
    {  8, 0,  5,  5,  5,  5,  5,  5,  5,   5, 0,   5, 1000,  0, NO_THING, NO_THING, UNIQUE_MADE, M_TALK_MAN, M_MOVE_SMART, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, ASLEEP | HOSTILE, 0, '@' | CLR_RED_BLACK, "Hiscore NPC", "dead NPC", "A?B?"},
    {  9, 1, 25, 12,  5, 15,  3,  3,  8, 100, 0,  10,  100,  0, NO_THING, WEAPON_CLUB, COMMON, M_TALK_STUPID, M_MOVE_NORMAL, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | HUNGRY | POISONOUS, pow2 (FLAME) | pow2 (COLD) | pow2 (ELECTRICITY), 'g' | CLR_BROWN_BLACK, "grunt", "dead grunt", "ACAC"},
    { 10, 1,  1, 10,  5,  2, 10, 10,  4,   0, 0,   3,    1,  5, POTION_OF_ALERTNESS, NO_THING, COMMON, M_TALK_STUPID, M_MOVE_FLUTTER, M_MELEE_SLEEP, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | FLYING, 0, 't' | CLR_PURPLE_BLACK, "tse-tse fly", "dead tse-tse fly", "AX"},
    { 11, 1,  1,  0,  0,  0, 10, 10, 10,   0, 1,   5,    2, 50, SCROLL_SUMMON, NO_THING, COMMON, M_TALK_STUPID, M_MOVE_FLUTTER, M_NO_OP, M_NO_OP, M_SP_SUMMON, 0, 0, 'f' | CLR_PURPLE_BLACK, "fnord", "fnord's antennae", ""},
    { 12, 1,  5,  5,  3,  5,  5,  2,  3,  30, 0,   5,   10,  0, NO_THING, NO_THING, COMMON, M_TALK_STUPID, M_MOVE_FLUTTER, M_MELEE_DISEASE, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | HUNGRY | SWIMMING, 0, 'r' | CLR_BROWN_BLACK, "sewer rat", "dead sewer rat", "ALAL"},
    { 13, 1, 12,  0,  0,  0, 10, 10, 10,   0, 0,   1,    1,  0, NO_THING, NO_THING, COMMON, M_TALK_STUPID, M_NO_OP, M_NO_OP, M_NO_OP, M_SP_AGGRAVATE, 0, pow2 (FLAME), 'f' | CLR_CYAN_BLACK, "aggravator fungus", "aggravator fungus spores", ""},
    { 14, 1, 50,  0,  5,  0,  3,  3, 24, 100, 2,  20,   20, 25, SCROLL_TELEPORT, NO_THING, COMMON, M_TALK_STUPID, M_NO_OP, M_NO_OP, M_NO_OP, M_MOVE_TELEPORT, MOBILE, 0, 'b' | CLR_BRIGHT_WHITE_BLACK | A_BLINK, "blipper", "blipper organ", "BX"},
    { 15, 1,  8, 10,  8,  8, 10,  2,  5,   0, 1,  10,  500,  0, NO_THING, WEAPON_DAGGER, COMMON, M_TALK_GREEDY, M_MOVE_SMART, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | GREEDY, 0, 'g' | CLR_GREEN_BLACK, "goblin", "dead goblin", "AXBX"},
    { 16, 1, 10, 10, 10, 10,  4,  4,  9,  50, 1,  25,    5, 10, SCROLL_TRUESIGHT, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_SMART, M_MELEE_NORMAL, M_NO_OP, M_SP_ILLUSION, MOBILE | HOSTILE | POISONOUS, pow2 (SLEEP), 'p' | CLR_RED_BLACK, "phantasticon", "phantasticon's eyes", "AXAXBX"},
    { 17, 1,  5, 10, 10, 15, 10,  3, 12, 100, 0,  15,   25,  0, NO_THING, NO_THING, COMMON, M_TALK_ROBOT, M_MOVE_NORMAL, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE, pow2 (ELECTRICITY) | pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'R' | CLR_GREY_BLACK, "household robot", "household robot battery", "ACBC"},
    { 18, 1,  1,  5,  5,  5, 10, 10,  8,  50, 0,   1,  250,  0, NO_THING, NO_THING, COMMON, M_TALK_STUPID, M_MOVE_SCAREDY, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE, 0, 'g' | CLR_WHITE_BLACK, "pencil-necked geek", "dead pencil-necked geek", "AX"},
    { 19, 1,  6,  8,  5,  6,  4,  3,  4,  50, 0,   6,   30,  0, NO_THING, NO_THING, COMMON, M_TALK_MIMSY, M_MOVE_FLUTTER, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | FLYING | EDIBLE, 0, 'b' | CLR_GREEN_BLACK, "borogrove", "borogrove wings", "AXBX"},
    { 20, 1,  3,  0, 15,  1,  4,  4,  2,  50, 0,   1,   20,  0, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_FLUTTER, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | FLYING | EDIBLE, 0, 'q' | CLR_BROWN_BLACK, "quail", "quail en brochet", ""},
    { 21, 1, 13, 10,  5,  5,  2,  1,  4,  50, 0,   3,   20,  0, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_ANIMAL, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | SWIMMING | EDIBLE, 0, 'b' | CLR_BROWN_BLACK, "badger", "badger ribs", "ALAL"},
    { 22, 1,  7, 15, 10, 10, 10,  5,  2,  50, 0,  10,   20,  0, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_ANIMAL, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | FLYING | EDIBLE, 0, 'h' | CLR_BROWN_BLACK, "hawk", "buffalo style hawk wings", "AH"},
    { 23, 1, 23,  5, 10,  5, 10, 10,  2,  50, 0,   5,  500,  0, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_ANIMAL, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | SWIMMING | EDIBLE, 0, 'd' | CLR_BROWN_BLACK, "deer", "venison", "AXBXBX"},
    { 24, 1, 30, 10,  5, 10,  4,  2,  5,  50, 0,   5,  500,  0, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_ANIMAL, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | HUNGRY | EDIBLE, 0, 'c' | CLR_BROWN_BLACK, "camel", "hump au jus", "AXBX"},
    { 25, 1, 20,  0,  5,  5,  4,  2,  4,  50, 0,   3,  400,  0, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_ANIMAL, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | SWIMMING | EDIBLE, 0, 'a' | CLR_BROWN_BLACK, "anteater", "roast loin of anteater", "AL"},
    { 26, 1,  3,  0,  7,  1,  4,  4,  3,  50, 0,   1,   20,  0, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_ANIMAL, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | SWIMMING | EDIBLE, 0, 'r' | CLR_BROWN_BLACK, "rabbit", "bunnyburgers", "AL"},
    { 27, 1,  3,  0, 18,  1,  4,  4,  2,  50, 0,   1,   20,  0, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_ANIMAL, M_NO_OP, M_NO_OP, M_NO_OP, MOBILE | ONLYSWIM | EDIBLE, 0, 't' | CLR_CYAN_BLACK, "trout", "filet of trout", ""},
    { 28, 1,  1,  0,  0,  0,  4,  4,  4,  50, 0,   1,   20,  0, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_ANIMAL, M_NO_OP, M_NO_OP, M_NO_OP, MOBILE | ONLYSWIM | EDIBLE, 0, 'b' | CLR_CYAN_BLACK, "striped bass", "bass filets", ""},
    { 29, 1,  5, 10, 10, 12, 10,  2,  3,  50, 0,   5,   20,  0, NO_THING, NO_THING, COMMON, M_TALK_PARROT, M_MOVE_FLUTTER, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HUNGRY | FLYING | EDIBLE, 0, 'p' | CLR_PURPLE_BLACK, "parrot", "grilled breast of parrot", "AH"},
    { 30, 1, 10, 10,  5,  5, 10,  2,  4,  20, 0,   8,   20,  0, NO_THING, NO_THING, COMMON, M_TALK_HYENA, M_MOVE_ANIMAL, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | HUNGRY | SWIMMING | EDIBLE, 0, 'h' | CLR_BROWN_BLACK, "hyena", "hyena ribs", "AX"},
    { 31, 2, 20, 16, 10, 15, 10,  3,  3,  50, 1,  40, 1000,  0, NO_THING, WEAPON_SHORT_SWORD, COMMON, M_TALK_NINJA, M_MOVE_SMART, M_MELEE_NORMAL, M_NO_OP, M_SP_SURPRISE, MOBILE | HOSTILE | M_INVISIBLE, 0, 'n' | CLR_GREY_BLACK, "apprentice ninja", "dead ninja", "A?A?B?B?"},
    { 32, 2, 30, 15, 10,  5, 10, 10,  6,   0, 0,  35,  250,100, NO_THING, NO_THING, COMMON, M_TALK_TITTER, M_MOVE_FLUTTER, M_MELEE_NG, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | FLYING | POISONOUS, 0, 'n' | CLR_GREY_BLACK, "night gaunt", "dead night gaunt", "ACAC"},
    { 33, 2, 32,  0, 10,  0, 10, 10,  2,   0, 3,  20, 1000,  0, NO_THING, NO_THING, COMMON, M_TALK_THIEF, M_MOVE_SMART, M_NO_OP, M_NO_OP, M_SP_THIEF, MOBILE | HOSTILE | GREEDY, 0, '@' | CLR_RED_BLACK, "sneak-thief", "dead sneak-thief", "AXB?B?B?"},
    { 34, 2, 25,  0,  0,  0, 10, 10,  5,   0, 0,  10,    1, 20, POTION_OF_LEVITATION, NO_THING, COMMON, M_NO_OP, M_MOVE_RANDOM, M_NO_OP, M_NO_OP, M_SP_EXPLODE, MOBILE | FLYING, pow2 (SLEEP), 'e' | CLR_GREEN_BLACK, "floating eye", "dead floating eye", ""},
    { 35, 2, 30, 10, 10, 10,  6,  3,  4,   0, 0,  25,   10,  0, NO_THING, NO_THING, COMMON, M_TALK_SLITHY, M_MOVE_FLUTTER, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE, 0, 't' | CLR_GREEN_BLACK, "tove", "dead tove", "ALBX"},
    { 36, 2, 32, 15, 10,  5,  5,  3,  8,   0, 1,  30,   50, 30, POTION_OF_INVISIBILITY, NO_THING, COMMON, M_NO_OP, M_MOVE_FLUTTER, M_MELEE_NORMAL, M_STRIKE_MISSILE, M_NO_OP, MOBILE | HOSTILE | M_INVISIBLE, 0, 'n' | CLR_GREY_BLACK, "transparent nasty", "dead transparent nasty", "ACAC"},
    { 37, 2, 32,  0, 10,  0,  5, 10, 20,   0, 0,  30,    1,  0, NO_THING, NO_THING, COMMON, M_TALK_HINT, M_MOVE_SPIRIT, M_NO_OP, M_NO_OP, M_SP_GHOST, MOBILE | WANDERING | INTANGIBLE, pow2 (NORMAL_DAMAGE) | pow2 (FEAR) | pow2 (SLEEP), 'g' | CLR_GREY_BLACK, "ghost", "ghost ectoplasm", ""},
    { 38, 2, 20,  5, 10,  5,  5,  3,  8,   0, 2,  40,    1,100, NO_THING, NO_THING, COMMON, M_TALK_EVIL, M_MOVE_SCAREDY, M_MELEE_NORMAL, M_NO_OP, M_SP_SPELL, MOBILE | HOSTILE, pow2 (COLD), 'e' | CLR_RED_BLACK, "enchanter", "dead enchanter", "AXBXBX"},
    { 39, 2,  7,  0,  0,  0, 10,  3, 12,   0, 0,  40,    1, 10, SCROLL_LIGHT, NO_THING, COMMON, M_NO_OP, M_NO_OP, M_NO_OP, M_NO_OP, M_SP_BLACKOUT, HOSTILE | POISONOUS, 0, 'f' | CLR_GREY_BLACK, "murk fungus", "murk cloud particles", ""},
    { 40, 2, 25, 20, 12, 10, 10,  3,  4,   0, 1,  30,    1,  0, NO_THING, WEAPON_GREAT_AXE, COMMON, M_TALK_EVIL, M_MOVE_SMART, M_MELEE_NORMAL, M_STRIKE_MISSILE, M_NO_OP, MOBILE | HOSTILE | GREEDY, pow2 (FEAR), 'G' | CLR_GREEN_BLACK, "goblin chieftain", "dead goblin chieftain", "A?A?B?"},
    { 41, 2, 40, 15,  5, 15,  8,  3,  4,  10, 0,  35,   50,  0, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_NORMAL, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | HUNGRY | SWIMMING | EDIBLE, pow2 (FEAR) | pow2 (SLEEP), 'w' | CLR_BROWN_BLACK, "wolf", "wolf cutlets", "AXAXAX"},
    { 42, 2, 20, 15, 15, 10,  3,  3,  3,  10, 0,  30,   50,  0, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_NORMAL, M_MELEE_POISON, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | POISONOUS, pow2 (FEAR) | pow2 (SLEEP), 'a' | CLR_RED_BLACK, "giant soldier ant", "ant mandibles", "AXAXAX"},
    { 43, 2,100, 25, 25, 50,  3,  3, 12,  10, 0,  50,  500,  0, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_ANIMAL, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HUNGRY | SWIMMING | EDIBLE, 0, 'e' | CLR_GREY_BLACK, "elephant", "trunk steak", "AX"},
    { 44, 2, 50, 20,  5, 20,  6,  3,  4,  10, 0,  40,  500,  0, NO_THING, NO_THING, COMMON, M_TALK_HORSE, M_MOVE_NORMAL, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HUNGRY | SWIMMING | EDIBLE, 0, 'h' | CLR_BROWN_BLACK, "horse", "steaming horsemeat", "AX"},
    { 45, 3, 35, 15,  5, 30,  4,  1,  4,  50, 0,  75,   50, 50, RING_OF_FIRE_RESISTANCE, NO_THING, COMMON, M_TALK_STUPID, M_MOVE_FLUTTER, M_MELEE_FIRE, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | GREEDY, pow2 (FLAME), 's' | CLR_LIGHT_RED_BLACK, "salamander", "salamander scales", "AXAX"},
    { 46, 3,100, 10, 10, 20,  5,  2,  8,  50, 0,  50,   10, 50, RING_OF_GAZE_IMMUNITY, NO_THING, COMMON, M_TALK_STUPID, M_MOVE_NORMAL, M_MELEE_NORMAL, M_STRIKE_BLIND, M_NO_OP, MOBILE | HOSTILE | HUNGRY | GREEDY, 0, 'C' | CLR_GREEN_BLACK, "catoblepas", "catoblepas' eyes", "AXBX"},
    { 47, 3, 15, 18, 15, 15,  3,  3,  4,   0, 2,  80,   25, 50, SNOWBALL_STICK, NO_THING, COMMON, M_TALK_EVIL, M_MOVE_SMART, M_MELEE_COLD, M_NO_OP, M_SP_DEMON, MOBILE | HOSTILE, pow2 (COLD) | pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'f' | CLR_LIGHT_BLUE_BLACK, "lesser frost demon", "lesser frost demon's heart", "A?A?B?"},
    { 48, 3, 10, 20,  0, 50,  5,  1, 20,   0, 0,  60,   10,  0, NO_THING, NO_THING, COMMON, M_NO_OP, M_MOVE_FOLLOW, M_MELEE_NORMAL, M_NO_OP, M_SP_ACID_CLOUD, MOBILE | FLYING, pow2 (POISON) | pow2 (SLEEP), 'a' | CLR_YELLOW_BLACK, "acid cloud", "acid pool", "AX"},
    { 49, 2,  1, 20,  0,  0,  5,  5, 10, 100, 0,  90,   10,  0, NO_THING, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_SPIRIT, M_MELEE_SPIRIT, M_NO_OP, M_SP_SURPRISE, MOBILE | HOSTILE | FLYING | INTANGIBLE | M_INVISIBLE, pow2 (NORMAL_DAMAGE) | pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'p' | CLR_GREY_BLACK, "phantom", "hardly anything at all", "AX"},
    { 50, 3, 32, 18, 10, 32,  5, 20,  5, 100, 3, 500,  100,  0, NO_THING, WEAPON_GOBLIN_HEWER, UNIQUE_MADE, M_TALK_GREEDY, M_MOVE_SMART, M_MELEE_NORMAL, M_STRIKE_MISSILE, M_NO_OP, MOBILE | HOSTILE | GREEDY, 0, 'K' | CLR_LIGHT_GREEN_BLACK, "The Goblin King", "The Head of the Goblin King", "A?A?B?B?"},
    { 51, 3, 42, 16, 10, 10,  8,  4,  3,   0, 0,  75,  500,  1, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_FLUTTER, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | FLYING | EDIBLE, pow2 (FEAR) | pow2 (SLEEP), 'P' | CLR_PURPLE_BLACK, "pterodactyl", "pterodactyl patee", "AH"},
    { 52, 3, 25, 10, 10, 10,  5,  5,  6,   0, 2,  40,  500,  0, NO_THING, NO_THING, COMMON, M_TALK_EVIL, M_MOVE_SMART, M_MELEE_NORMAL, M_NO_OP, M_SP_SPELL, MOBILE | HOSTILE | GREEDY, 0, 'G' | CLR_GREEN_BLACK, "goblin shaman", "dead goblin shaman", "AXBX"},
    { 53, 3, 60, 20, 10, 25,  5,  1,  3,   0, 0,  50,   50,  0, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_NORMAL, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | HUNGRY | SWIMMING | EDIBLE, 0, 'l' | CLR_YELLOW_BLACK, "lion", "lion souflee", "AXAX"},
    { 54, 3, 30, 15, 10, 15,  5,  1,  5,   0, 2,  75,   50,  0, NO_THING, WEAPON_CROSSBOW, COMMON, M_TALK_GREEDY, M_MOVE_SMART, M_MELEE_NORMAL, M_STRIKE_MISSILE, M_NO_OP, MOBILE | HOSTILE | GREEDY, 0, 'b' | CLR_RED_BLACK, "brigand", "dead brigand", "AXAXBX"},
    { 55, 3, 70, 25, 20, 40,  3,  1, 10,  80, 0,  55,  500,  0, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_ANIMAL, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HUNGRY | SWIMMING | EDIBLE, pow2 (FEAR), 'b' | CLR_BROWN_BLACK, "bear", "bear's sirloin tips", "AXAXBX"},
    { 56, 3, 10, 25, 10, 20,  3,  1,  3,  40, 0,  45,   30,  0, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_NORMAL, M_MELEE_POISON, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | POISONOUS, 0, 'm' | CLR_RED_BLACK, "mamba", "mamba filet", "AXAX"},
    { 57, 3, 40, 15, 10, 30,  3,  1,  7,   0, 1,  40,   20,  0, NO_THING, NO_THING, COMMON, M_NO_OP, M_MOVE_NORMAL, M_MELEE_POISON, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | HUNGRY | ONLYSWIM | POISONOUS, pow2 (POISON), 'M' | CLR_BLUE_BLACK, "man o' war", "deliquescing tentacles", "AXAX"},
    { 58, 3, 40,  5, 10, 10,  5,  2,  5,   0, 1, 100,   20,  0, NO_THING, NO_THING, COMMON, M_TALK_MAN, M_MOVE_NORMAL, M_MELEE_NORMAL, M_NO_OP, M_SP_WERE, MOBILE, 0, '@' | CLR_RED_BLACK, "were-human", "dead were-human", "AXBX"},
    { 59, 3,  4,  0,  0,  1,  5,  2,  5,   0, 1,  50,   20,  0, NO_THING, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_SPIRIT, M_MELEE_SPIRIT, M_NO_OP, M_NO_OP, MOBILE | HOSTILE, pow2 (COLD) | pow2 (ELECTRICITY) | pow2 (POISON) | pow2 (ACID) | pow2 (FEAR) | pow2 (SLEEP) | pow2 (NEGENERGY) | pow2 (OTHER_MAGIC) | pow2 (THEFT) | pow2 (GAZE) | pow2 (INFECTION), 'T' | CLR_GREY_BLACK, "thought-form", "a velleity", "ACACACACAC"},
    { 60, 4,100, 15, 12, 25,  6,  2,  6,  25, 2, 150,  300, 20, STAFF_OF_MISSILES, NO_THING, COMMON, M_TALK_HUNGRY, M_MOVE_NORMAL, M_MELEE_NORMAL, M_STRIKE_MISSILE, M_NO_OP, MOBILE | HOSTILE | HUNGRY | FLYING, 0, 'M' | CLR_YELLOW_BLACK, "manticore", "manticore spikes", "AXAX"},
    { 61, 4, 50, 12, 10, 10, 10, 10,  2,  50, 0, 100,  300, 10, POTION_OF_SPEED, NO_THING, COMMON, M_TALK_HUNGRY, M_MOVE_FLUTTER, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | HUNGRY, 0, 'T' | CLR_BROWN_BLACK, "tasmanian devil", "dead tasmanian devil", "AXAXAXAX"},
    { 62, 4,100, 15, 15, 50,  5,  2,  7, 100, 0, 100,  100,  0, NO_THING, NO_THING, COMMON, M_TALK_ROBOT, M_MOVE_SMART, M_MELEE_NORMAL, M_NO_OP, M_SP_POISON_CLOUD, MOBILE | HOSTILE, pow2 (ELECTRICITY) | pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'a' | CLR_GREY_BLACK, "automatum minor", "automatum minor battery", "ACBC"},
    { 63, 4, 75, 30, 10, 15,  5,  1, 12,  75, 0, 125,   50, 20, POTION_OF_CURING, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_NORMAL, M_MELEE_DISEASE, M_NO_OP, M_SP_SURPRISE, MOBILE | HOSTILE | HUNGRY | M_INVISIBLE | SWIMMING, pow2 (FLAME) | pow2 (ELECTRICITY) | pow2 (POISON), 's' | CLR_GREEN_BLACK, "denebian slime devil", "denebian slime", "AL"},
    { 64, 4, 50, 18, 12, 25,  5,  1,  3,  75, 0, 125,  500,  0, NO_THING, NO_THING, COMMON, M_TALK_BURBLE, M_MOVE_FLUTTER, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | HUNGRY | FLYING | EDIBLE, pow2 (POISON), 'j' | CLR_PURPLE_BLACK, "jub-jub bird", "jube-jubes", "AHAH"},
    { 65, 4, 50, 15,  5, 20,  4,  1,  7,   0, 0, 150,  500,  0, NO_THING, NO_THING, COMMON, M_TALK_EVIL, M_MOVE_NORMAL, M_MELEE_POISON, M_NO_OP, M_SP_SURPRISE, MOBILE | HOSTILE | HUNGRY | M_INVISIBLE | POISONOUS, pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'h' | CLR_GREY_BLACK, "haunt", "dessicated corpse", "ACAC"},
    { 66, 4, 50, 20, 10, 10,  4,  4,  5,   0, 1, 150,    1,200, NO_THING, NO_THING, COMMON, M_TALK_DEMONLOVER, M_MOVE_SMART, M_NO_OP, M_NO_OP, M_SP_DEMONLOVER, MOBILE | HOSTILE | POISONOUS, pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), '!' | CLR_RED_BLACK, "incubus/succubus", "(null)", "(null)"},
    { 67, 4, 50,  0, 10,  0,  4,  4,  6,   0, 0,  50,    1,  0, NO_THING, NO_THING, COMMON, M_TALK_SEDUCTOR, M_MOVE_SMART, M_NO_OP, M_NO_OP, M_SP_SEDUCTOR, MOBILE | NEEDY, 0, '!' | CLR_RED_BLACK, "satyr or nymph", "(null)", "(null)"},
    { 68, 4, 80, 25, 20, 30,  4,  1,  8,  50, 0, 100,  300,100, NO_THING, NO_THING, COMMON, M_NO_OP, M_MOVE_NORMAL, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | HUNGRY | ONLYSWIM, 0, 'c' | CLR_GREEN_BLACK, "giant crocodile", "crocodile hide", "ALALBX"},
    { 69, 4, 40, 30,  0, 10,  4,  4, 12,   0, 0, 120,   30, 50, POTION_OF_ALERTNESS, NO_THING, COMMON, M_TALK_STUPID, M_MOVE_NORMAL, M_MELEE_SLEEP, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | HUNGRY, pow2 (FEAR) | pow2 (SLEEP), 't' | CLR_BROWN_BLACK, "torpor beast", "a valium gland", "AXBX"},
    { 70, 4, 40, 20, 10, 15,  8,  8,  2,   0, 0, 120,  300,  0, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_LEASH, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE, pow2 (FEAR) | pow2 (SLEEP), 'd' | CLR_BROWN_BLACK, "doberman death-hound", "dead doberman death-hound", "LXLXLX"},
    { 71, 4, 40, 10, 10, 10,  8,  8,  1,   0, 0, 100,    0,  0, NO_THING, NO_THING, COMMON, M_NO_OP, M_MOVE_SPIRIT, M_MELEE_COLD, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | INTANGIBLE, pow2 (FLAME) | pow2 (FEAR) | pow2 (SLEEP), 'F' | CLR_GREY_BLACK, "astral fuzzy", "nothing much", "AX"},
    { 72, 4, 30, 40, 40, 20,  8,  8,  5,   0, 2, 120,    0,  0, NO_THING, NO_THING, COMMON, M_TALK_SERVANT, M_MOVE_SMART, M_MELEE_NORMAL, M_STRIKE_MISSILE, M_SP_SERVANT, MOBILE, pow2 (FEAR) | pow2 (SLEEP), 'l' | CLR_BLUE_BLACK, "servant of law", "dead servant", "L?R?"},
    { 73, 4, 40, 25, 20, 10,  8,  8,  5,   0, 2, 120,  500,  0, NO_THING, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_SMART, M_MELEE_NORMAL, M_STRIKE_MISSILE, M_SP_SERVANT, MOBILE, pow2 (FEAR) | pow2 (SLEEP), 'c' | CLR_RED_BLACK, "servant of chaos", "dead servant", "AXAXAXAX?"},
    { 74, 4, 40, 15,  0, 10,  3,  0, 10,   0, 0,  50,    1,  0, NO_THING, NO_THING, COMMON, M_NO_OP, M_MOVE_NORMAL, M_MELEE_NORMAL, M_NO_OP, M_SP_SWARM, MOBILE | HOSTILE | FLYING, pow2 (NORMAL_DAMAGE) | pow2 (FEAR) | pow2 (SLEEP), 's' | CLR_YELLOW_BLACK, "swarm", "dead swarm members", "AX"},
    { 75, 4, 40, 10, 20, 10,  8,  8,  4,   0, 2, 100,   50,  0, NO_THING, NO_THING, COMMON, M_TALK_SCREAM, M_MOVE_NORMAL, M_MELEE_SPIRIT, M_STRIKE_SONIC, M_NO_OP, MOBILE | HOSTILE | FLYING, pow2 (FEAR) | pow2 (SLEEP), 'b' | CLR_GREY_BLACK, "ban sidhe", "dead ban sidhe", "LX"},
    { 76, 4, 50, 20, 20, 20,  3,  1,  6,  60, 3, 100,    1,  0, NO_THING, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_SPIRIT, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | INTANGIBLE, pow2 (NORMAL_DAMAGE), 'e' | CLR_GREEN_BLACK, "etheric grue", "dead etheric grue", "LHAHLH"},
    { 77, 4, 40, 20, 13, 30,  8,  4,  5,   0, 1, 100,  500,100, NO_THING, NO_THING, COMMON, M_TALK_NINJA, M_MOVE_NORMAL, M_MELEE_POISON, M_STRIKE_MISSILE, M_SP_SURPRISE, MOBILE | HOSTILE | M_INVISIBLE, pow2 (POISON) | pow2 (SLEEP), 'n' | CLR_GREY_BLACK, "ninja (genin)", "dead ninja", "A?A?R?R"},
    { 78, 5, 50, 20, 15, 40, 10,  2,  3,  90, 4, 180,  500, 50, RING_OF_FIRE_RESISTANCE, NO_THING, COMMON, M_TALK_GREEDY, M_MOVE_NORMAL, M_MELEE_DRAGON, M_STRIKE_FBOLT, M_NO_OP, MOBILE | HOSTILE | HUNGRY | GREEDY | FLYING | EDIBLE, pow2 (FLAME), 'd' | CLR_LIGHT_RED_BLACK, "dragonette", "dragon steak", "AXAXBXBX"},
    { 79, 5, 50, 15, 10, 40, 10,  2, 12,  40, 0, 150,  100,200, WAND_OF_BALL_LIGHTNING, NO_THING, COMMON, M_TALK_STUPID, M_MOVE_FLUTTER, M_MELEE_ELEC, M_STRIKE_LBALL, M_NO_OP, MOBILE | HOSTILE, pow2 (ELECTRICITY), 't' | CLR_LIGHT_BLUE_BLACK, "tesla monster", "tesla monster whip", "AXAX"},
    { 80, 5,100, 20, 12, 20,  4,  2,  4,  80, 2, 150,  100,150, RING_OF_POISON_RESISTANCE, NO_THING, COMMON, M_TALK_GREEDY, M_MOVE_NORMAL, M_MELEE_POISON, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | GREEDY | FLYING | POISONOUS, pow2 (FLAME), 'W' | CLR_LIGHT_RED_BLACK, "wyvern", "wyvern's sting", "AXAXBXBX"},
    { 81, 5, 50, 15, 10, 20, 10,  2,  2,  50, 0, 250,  400,  0, NO_THING, NO_THING, COMMON, M_TALK_HUNGRY, M_MOVE_FLUTTER, M_MELEE_NORMAL, M_NO_OP, M_SP_FLUTTER, MOBILE | HOSTILE | HUNGRY | FLYING, pow2 (FEAR), 'c' | CLR_BROWN_BLACK, "radeligian cateagle", "dead radeligian cateagle", "AXAXAXAXAX"},
    { 82, 5, 75, 20, 20, 20,  5,  5,  3,  50, 3, 200,   50,200, ROD_OF_SUMMONING, NO_THING, COMMON, M_TALK_EVIL, M_MOVE_SMART, M_MELEE_COLD, M_STRIKE_SNOWBALL, M_SP_DEMON, MOBILE | HOSTILE | POISONOUS, pow2 (COLD) | pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'f' | CLR_WHITE_BLACK, "frost demon", "frost demon's heart", "A?A?B?B?"},
    { 83, 5, 75, 20, 20, 20,  5,  2,  6,  50, 0, 200,   10,200, CLOAK_OF_NEGIMMUNITY, NO_THING, COMMON, M_TALK_EVIL, M_MOVE_SPIRIT, M_MELEE_SPIRIT, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | FLYING, pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'S' | CLR_GREY_BLACK, "spectre", "spectre's veil", "A?A?A?"},
    { 84, 5, 50, 12, 13, 10,  5,  5,  5,   0, 4, 205,   10,500, NO_THING, SCROLL_SPELLS, COMMON, M_TALK_EVIL, M_MOVE_SCAREDY, M_MELEE_NORMAL, M_NO_OP, M_SP_SPELL, MOBILE | HOSTILE, pow2 (FLAME) | pow2 (COLD) | pow2 (ELECTRICITY), 'n' | CLR_RED_BLACK, "necromancer", "dead necromancer", "A?B?B?B?"},
    { 85, 5, 40, 18, 18, 20,  5,  5,  7,  50, 2, 175,    2, 10, SCROLL_LIGHT, NO_THING, COMMON, M_TALK_EVIL, M_MOVE_SPIRIT, M_MELEE_SPIRIT, M_NO_OP, M_SP_BLACKOUT, MOBILE | HOSTILE | INTANGIBLE, pow2 (NORMAL_DAMAGE), 'S' | CLR_GREY_BLACK, "shadow spirit", "shadowstuff", "ACACAC"},
    { 86, 5, 80, 22,  8, 30,  3,  3, 12,  50, 2, 275,   20,  0, NO_THING, NO_THING, COMMON, M_TALK_EVIL, M_MOVE_SMART, M_MELEE_GRAPPLE, M_NO_OP, M_SP_BOG, MOBILE | HOSTILE | ONLYSWIM | POISONOUS, pow2 (NORMAL_DAMAGE), 'B' | CLR_GREEN_BLACK, "bog thing", "swamp slime", "AXAXB?B?"},
    { 87, 5, 40, 25,  0,  0, 20,  3,  6,   0, 2, 175,   20,  0, NO_THING, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_SPIRIT, M_MELEE_SPIRIT, M_NO_OP, M_SP_AV, MOBILE | HOSTILE | INTANGIBLE, pow2 (NORMAL_DAMAGE) | pow2 (FEAR) | pow2 (SLEEP), 'V' | CLR_GREY_BLACK, "astral vampire", "dead astral vampire", "R?R?R?"},
    { 88, 5,100, 40,  0, 60,  3,  1, 10,  75, 0, 175, 2000,  0, NO_THING, NO_THING, COMMON, M_NO_OP, M_MOVE_NORMAL, M_MELEE_FIRE, M_NO_OP, M_SP_LW, MOBILE | HOSTILE | SWIMMING | POISONOUS, pow2 (FLAME), 'W' | CLR_LIGHT_RED_BLACK, "lava worm", "dead lava worm", "AX"},
    { 89, 5,  1,  0,  0,  0, 30, 30, 20,   0, 0, 100,    0,  0, NO_THING, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_RANDOM, M_NO_OP, M_NO_OP, M_SP_MB, MOBILE | HOSTILE | INTANGIBLE, 0, 'm' | CLR_PURPLE_BLACK, "manaburst", "feeling of warmth", "BX"},
    { 90, 5,120, 25, 20, 20,  6,  6,  4,   0, 1, 150, 2000,  0, NO_THING, WEAPON_ESTOC, COMMON, M_TALK_EVIL, M_MOVE_SMART, M_MELEE_FIRE, M_MELEE_POISON, M_SP_DEMON, MOBILE | HOSTILE | POISONOUS, pow2 (FLAME) | pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'd' | CLR_LIGHT_RED_BLACK, "outer circle demon", "dead outer circle demon", "L?R?R?R?"},
    { 91, 5, 40, 15, 10, 30,  8,  8,  6,   0, 1, 100,   20,  0, NO_THING, NO_THING, COMMON, M_NO_OP, M_MOVE_SMART, M_MELEE_SPIRIT, M_NO_OP, M_SP_MIRROR, MOBILE | HOSTILE, pow2 (FLAME) | pow2 (COLD) | pow2 (ELECTRICITY), 'm' | CLR_GREY_BLACK, "mirrorshade", "some broken glass", "AXAXAX"},
    { 92, 6,125, 30,  0, 50, 10,  2,  4, 100, 0, 250,    1,200, WAND_OF_FIREBALLS, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_NORMAL, M_MELEE_FIRE, M_STRIKE_FBALL, M_NO_OP, MOBILE | HOSTILE | FLYING, pow2 (FLAME) | pow2 (POISON) | pow2 (ACID) | pow2 (SLEEP), 'F' | CLR_WHITE_RED, "fire elemental", "essence of fire elemental", "AXAXAXRXRX"},
    { 93, 6,125, 25,  0, 20, 10,  2,  2, 100, 0, 250,    1,200, CLOAK_OF_INVISIBILITY, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_FLUTTER, M_MELEE_NORMAL, M_NO_OP, M_SP_WHIRL, MOBILE | HOSTILE | FLYING | INTANGIBLE, pow2 (ELECTRICITY) | pow2 (POISON) | pow2 (ACID) | pow2 (SLEEP), 'A' | CLR_WHITE_BLUE, "air elemental", "essence of air elemental", "AXAXAXAXBXBX"},
    { 94, 6,100, 15, 10, 30,  5,  1,  6, 100, 0, 250,    1,200, STAFF_OF_HEALING, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_NORMAL, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | ONLYSWIM, pow2 (COLD) | pow2 (SLEEP), 'W' | CLR_WHITE_BLUE, "water elemental", "essence of water elemental", "AXAXBXBX"},
    { 95, 6,200, 20, 25,100,  5,  1, 15, 100, 0, 250,    1,200, CLOAK_OF_PROTECTION, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_SPIRIT, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE, pow2 (POISON) | pow2 (SLEEP), 'E' | CLR_WHITE_BROWN, "earth elemental", "essence of earth elemental", "AXBX"},
    { 96, 6,100, 20, 20, 50,  3,  1,  2,  50, 1, 250, 1000,100, NO_THING, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_FLUTTER, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE, pow2 (SLEEP), 'b' | CLR_YELLOW_BROWN, "bandersnatch", "bandersnatch hide", "AXAXAXBX"},
    { 97, 6,100, 15, 20, 50,  5,  1,  3,  50, 5, 300,  100,1000,NO_THING, NO_THING, COMMON, M_TALK_EVIL, M_MOVE_SMART, M_MELEE_SPIRIT, M_NO_OP, M_SP_SPELL, MOBILE | HOSTILE, pow2 (FLAME) | pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'L' | CLR_BLACK_WHITE, "lich", "lich's skeleton", "A?A?A?B?B?"},
    { 98, 6,100, 20, 20, 30,  5,  3,  5,  30, 3, 300,   10,500, NO_THING, WEAPON_TRIDENT, COMMON, M_TALK_EVIL, M_MOVE_SMART, M_MELEE_NORMAL, M_NO_OP, M_SP_SPELL, MOBILE | HOSTILE | SWIMMING, pow2 (SLEEP), 'T' | CLR_LIGHT_GREEN_BLUE, "triton", "dead triton", "A?A?A?B?B?"},
    { 99, 6,100,  0, 20,  0, 10,  3,  3,   0, 5, 200,   10,  0, NO_THING, THING_LOCKPICK, COMMON, M_TALK_THIEF, M_MOVE_SMART, M_NO_OP, M_NO_OP, M_SP_THIEF, MOBILE | HOSTILE | GREEDY | SWIMMING, 0, '@' | CLR_RED_BLACK, "human", "dead master thief", "A?B?B?B?"},
    {100, 6,200, 50, 50, 80,  3,  1,  8,   0, 0, 300,60000,  0, NO_THING, NO_THING, COMMON, M_TALK_ANIMAL, M_MOVE_NORMAL, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | SWIMMING, pow2 (FEAR) | pow2 (SLEEP), 'T' | CLR_GREEN_BROWN, "triceratops", "dead triceratops", "LX"},
    {101, 6,100, 20, 20, 20, 10,  2,  3,  50, 4, 250,   10,  0, NO_THING, WEAPON_GREAT_SWORD, COMMON, M_TALK_EVIL, M_MOVE_SMART, M_MELEE_NORMAL, M_STRIKE_FBOLT, M_SP_ILLUSION, MOBILE | HOSTILE, pow2 (FLAME) | pow2 (FEAR) | pow2 (SLEEP), 'R' | CLR_YELLOW_WHITE, "rakshasa", "dead rakshasa", "L?L?R?R?"},
    {102, 6,100, 40, 20, 30,  5,  1,  7,  90, 4, 250,   10,500, NO_THING, NO_THING, COMMON, M_TALK_EVIL, M_MOVE_NORMAL, M_MELEE_POISON, M_STRIKE_FBALL, M_SP_DEMON, MOBILE | HOSTILE | SWIMMING, pow2 (FLAME) | pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'S' | CLR_GREEN_RED, "demon serpent", "demon serpent's jewel", "L?L?"},
    {103, 6,120, 30, 20, 20, 10, 10,  4,   0, 1, 250,   10,  0, NO_THING, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_SMART, M_MELEE_NORMAL, M_NO_OP, M_SP_ANGEL, MOBILE | FLYING, pow2 (FEAR) | pow2 (SLEEP), 'a' | CLR_YELLOW_WHITE, "angel", "angel's gown", "L?R?RX"},
    {104, 6,100, 25, 15, 40,  9,  4,  4,   0, 1, 200,  100,100, NO_THING, NO_THING, COMMON, M_TALK_NINJA, M_MOVE_NORMAL, M_MELEE_POISON, M_STRIKE_MISSILE, M_SP_SURPRISE, MOBILE | HOSTILE | M_INVISIBLE, pow2 (POISON) | pow2 (SLEEP), 'n' | CLR_BLACK_WHITE, "ninja (chunin)", "dead ninja", "A?A?R?R?R"},
    {105, 7,300, 30,  5,120,  3,  3, 15, 100, 0, 400,  500, 50, RING_OF_STRENGTH, NO_THING, COMMON, M_TALK_STUPID, M_MOVE_NORMAL, M_MELEE_NORMAL, M_NO_OP, M_SP_HUGE, MOBILE | HOSTILE | SWIMMING, pow2 (ELECTRICITY), 'B' | CLR_BLACK_BROWN, "behemoth", "behemoth's gonads", "AHBXBX"},
    {106, 7,250, 40, 20, 20, 10,  5,  5,   0, 0, 500,  100,2000,NO_THING, WEAPON_DEMONBLADE, COMMON, M_TALK_EVIL, M_MOVE_SPIRIT, M_MELEE_SPIRIT, M_NO_OP, M_SP_SPELL, MOBILE | HOSTILE | FLYING, pow2 (COLD) | pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'N' | CLR_BLACK_WHITE, "nazgul", "nazgul's husk", "A?A?A?B?B?"},
    {107, 7,100,100, 30, 50, 10,  3,  2,  50, 0, 400,  100,500, RING_OF_POISON_RESISTANCE, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_NORMAL, M_MELEE_NORMAL, M_NO_OP, M_MOVE_SCAREDY, MOBILE | SWIMMING, pow2 (POISON) | pow2 (FEAR), 'U' | CLR_BLACK_WHITE, "unicorn", "unicorn's horn", "ACACB?B?B?B?B?B?B?"},
    {108, 7,150, 30, 20, 50, 10,  3,  3,  50, 0, 400,  100,  0, NO_THING, NO_THING, COMMON, M_NO_OP, M_MOVE_NORMAL, M_MELEE_DISEASE, M_NO_OP, M_SP_SURPRISE, MOBILE | HOSTILE | HUNGRY | M_INVISIBLE | SWIMMING, pow2 (FEAR), 'r' | CLR_BLACK_BROWN, "rodent of unusual size", "dead R.O.U.S.", "AXAXAX"},
    {109, 7,100, 20, 25, 30, 10,  2,  5,  50, 3, 400,    0,  0, NO_THING, NO_THING, COMMON, M_NO_OP, M_MOVE_NORMAL, M_MELEE_NORMAL, M_NO_OP, M_SP_ILLUSION, MOBILE | HOSTILE | GREEDY, pow2 (OTHER_MAGIC), 'F' | CLR_BLACK_WHITE, "illusory fiend", "not much of anything", "AXAXAX"},
    {110, 7,250, 40, 30, 20, 10,  1,  5, 100, 8, 500, 1500,2000,NO_THING, WEAPON_DEFENDER, UNIQUE_MADE, M_TALK_EVIL, M_MOVE_NORMAL, M_MELEE_NORMAL, M_MELEE_COLD, M_SP_WYRM, MOBILE | HOSTILE | SWIMMING, pow2 (COLD) | pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'W' | CLR_GREY_RED, "Great Wyrm", "The head of the Great Wyrm", "AXAXAX"},
    {111, 7,150, 30, 30, 20, 10,  1,  1, 100, 8, 350, 1500,  0, NO_THING, WEAPON_FIRESTAR, COMMON, M_NO_OP, M_MOVE_SMART, M_MELEE_FIRE, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | FLYING, pow2 (FLAME) | pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'F' | CLR_GREY_RED, "flame devil", "dead flame devil", "A?A?R?R?R?"},
    {112, 7,150, 50, 10, 20,  1,  0,  1, 100, 4, 400, 1500,  0, NO_THING, NO_THING, COMMON, M_NO_OP, M_NO_OP, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, HOSTILE | HUNGRY, 0, OPEN_DOOR, "lurker at the threshold", "dead lurker", "L?L?L?"},
    {113, 7, 50, 30, 30, 20, 10,  1,  4,   0, 2, 450,  500,  0, NO_THING, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_NORMAL, M_MELEE_SLEEP, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | FLYING, pow2 (NORMAL_DAMAGE) | pow2 (SLEEP), 'S' | CLR_YELLOW_BROWN, "sandman", "mound of sand", "AX"},
    {114, 7, 50, 40, 10, 10, 10, 10,  2,   0, 5, 350,   50,  0, NO_THING, NO_THING, COMMON, M_NO_OP, M_MOVE_SMART, M_MELEE_SLEEP, M_NO_OP, M_SP_MIRROR, MOBILE | HOSTILE | FLYING, pow2 (OTHER_MAGIC), 'm' | CLR_RED_WHITE, "mirror master", "shards of glass", "AXAX"},
    {115, 7,150, 30, 30, 20, 10,  1,  4,  90, 2, 450,  500,  0, NO_THING, NO_THING, COMMON, M_TALK_HUNGRY, M_MOVE_NORMAL, M_MELEE_POISON, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | HUNGRY | FLYING, pow2 (POISON) | pow2 (FEAR), 'G' | CLR_GREY_GREEN, "elder etheric grue", "dead elder etheric grue", "AXAXAXL?B?B?"},
    {116, 7,150, 50, 40, 60,  3,  3,  8,  90, 6, 380,  500,  0, NO_THING, NO_THING, COMMON, M_TALK_HUNGRY, M_MOVE_NORMAL, M_MELEE_POISON, M_NO_OP, M_SP_ACID_CLOUD, MOBILE | HOSTILE | HUNGRY | SWIMMING, pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'S' | CLR_GREY_GREEN, "loathly serpent", "dead loathly serpent", "AXAXAXL?B?B?"},
    {117, 7,100, 10, 30, 20,  8,  3,  3,  20, 2, 460,  500,  0, NO_THING, NO_THING, COMMON, M_TALK_EVIL, M_MOVE_SCAREDY, M_MELEE_SPIRIT, M_NO_OP, M_SP_RAISE, MOBILE | HOSTILE, pow2 (FEAR), 'Z' | CLR_BLACK_WHITE, "zombie overlord", "extremely dead zombie overlord", "L?R?R?R?"},
    {118, 7, 50, 30, 30,  2, 13,  1,  1,  30, 0, 250,   50,  0, NO_THING, NO_THING, COMMON, M_TALK_STUPID, M_MOVE_FLUTTER, M_MELEE_NORMAL, M_NO_OP, M_MOVE_FLUTTER, MOBILE | HOSTILE | FLYING, 0, 'r' | CLR_GREY_BROWN, "ricochet", "dead ricochet", "L?"},
    {119, 7,240, 35, 30, 40,  8,  8,  3,   0, 3, 300, 3000,  0, NO_THING, NO_THING, COMMON, M_TALK_EVIL, M_MOVE_SMART, M_MELEE_FIRE, M_MELEE_SLEEP, M_SP_DEMON, MOBILE | HOSTILE | POISONOUS, pow2 (FLAME) | pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'D' | CLR_GREY_RED, "inner circle demon", "dead inner circle demon", "L?L?R?R?R?RXRXRX"},
    {120, 8,  1,  0,100,  0,  6,  2,  3,   0, 0,  50,  100,  0, NO_THING, NO_THING, COMMON, M_TALK_GF, M_MOVE_RANDOM, M_NO_OP, M_NO_OP, M_NO_OP, MOBILE | FLYING, pow2 (FLAME) | pow2 (COLD) | pow2 (SLEEP), 'f' | CLR_YELLOW_WHITE, "fairy", "good fairy dust", "B?"},
    {121, 8,100, 50, 50, 20, 10, 10,  3,   0, 0, 500,    3,  0, NO_THING, NO_THING, COMMON, M_TALK_EF, M_MOVE_RANDOM, M_NO_OP, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | FLYING | POISONOUS, pow2 (FLAME) | pow2 (COLD) | pow2 (SLEEP), 'f' | CLR_YELLOW_WHITE, "fairy", "evil fairy dust", "B?"},
    {122, 8,500, 30, 50,100,  5,  2,  3, 100, 0, 500, 1000,  0, NO_THING, WEAPON_LIGHTSABRE, COMMON, M_TALK_ROBOT, M_MOVE_SMART, M_MELEE_NORMAL, M_STRIKE_LBALL, M_SP_POISON_CLOUD, MOBILE | HOSTILE, pow2 (ELECTRICITY) | pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'A' | CLR_GREY_WHITE, "automatum major", "automatum major components", "A?A?R?R?"},
    {123, 8,500, 50, 30, 75, 20,  1,  5, 100, 7, 750,  500,1000,ARMOR_DRAGONSCALE, NO_THING, COMMON, M_TALK_GREEDY, M_MOVE_NORMAL, M_MELEE_DRAGON, M_STRIKE_FBOLT, M_NO_OP, MOBILE | HOSTILE | HUNGRY | GREEDY | FLYING, pow2 (FLAME) | pow2 (POISON) | pow2 (SLEEP), 'D' | CLR_GREY_RED, "dragon", "dragon scales", "A?A?A?A?B?B?"},
    {124, 8,500, 40, 25,100,  5,  1,  2,  75, 2, 600,  500,1000,WEAPON_VORPAL_SWORD, NO_THING, COMMON, M_TALK_BURBLE, M_MOVE_FLUTTER, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | HUNGRY, pow2 (POISON) | pow2 (SLEEP), 'J' | CLR_BROWN_RED, "jabberwock", "jabberwock's head", "A?A?A?A?A?"},
    {125, 8,350, 40, 40, 40, 10, 10,  2,  25, 5, 700,  100,500, WEAPON_DEMONBLADE, NO_THING, COMMON, M_TALK_EVIL, M_MOVE_SMART, M_MELEE_COLD, M_STRIKE_SNOWBALL, M_SP_DEMON, MOBILE | HOSTILE | HUNGRY, pow2 (COLD) | pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'F' | CLR_LIGHT_BLUE_WHITE, "frost demon lord", "frost demon lord's sigil", "A?A?A?R?R?R?"},
    {126, 8,350, 35, 30, 40, 10,  4,  2,  25, 0, 700,  100,100, NO_THING, NO_THING, COMMON, M_NO_OP, M_MOVE_NORMAL, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | HUNGRY | ONLYSWIM, pow2 (COLD) | pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'T' | CLR_YELLOW_BLUE, "tigershark", "tigershark pelt", "AXAXAXAX"},
    {127, 8,250, 40, 25, 50, 10,  4,  3,   0, 2, 800,  100,100, NO_THING, NO_THING, COMMON, M_TALK_NINJA, M_MOVE_NORMAL, M_MELEE_POISON, M_STRIKE_MISSILE, M_SP_SURPRISE, MOBILE | HOSTILE | M_INVISIBLE, pow2 (POISON) | pow2 (SLEEP), 'n' | CLR_BLACK_WHITE, "ninja (jonin)", "dead ninja", "A?A?R?R?R?R?R?"},
    {128, 8,250, 30, 40, 30, 10,  4,  3,   0, 3, 500,   10, 75, RING_OF_TRUESIGHT, NO_THING, COMMON, M_TALK_EVIL, M_MOVE_SPIRIT, M_MELEE_POISON, M_SP_SPELL, M_SP_BLACKOUT, MOBILE | HOSTILE | INTANGIBLE | M_INVISIBLE, pow2 (NORMAL_DAMAGE) | pow2 (POISON) | pow2 (SLEEP), 'S' | CLR_BLACK_WHITE, "shadow slayer", "shadow matrix", "A?A?B?B?"},
    {129, 8,250, 30, 30, 30, 10,  4,  6,   0, 3, 700,   10, 75, NO_THING, WEAPON_MACE, COMMON, M_TALK_SILENT, M_MOVE_SMART, M_MELEE_NORMAL, M_STRIKE_MISSILE, M_SP_SPELL, MOBILE | HOSTILE, pow2 (SLEEP), '@' | CLR_RED_BLACK, "militant priest", "dead militant priest", "A?A?A?B?B?"},
    {130, 8,150, 25, 40, 30, 10,  4,  5,   0, 3, 500,   10, 75, NO_THING, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_SMART, M_MELEE_SLEEP, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | M_INVISIBLE, pow2 (SLEEP), 'C' | CLR_GREY_BROWN, "coma beast", "dead coma beast", "AXAXAXBX"},
    {131, 8,250, 50, 40, 30, 10, 10,  3,   0, 3, 500,   10,  0, NO_THING, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_SMART, M_MELEE_NORMAL, M_STRIKE_FBOLT, M_SP_ANGEL, MOBILE | FLYING, pow2 (FLAME) | pow2 (FEAR) | pow2 (SLEEP), 'A' | CLR_YELLOW_WHITE, "high angel", "high angel's robes", "L?R?RXRX"},
    {132, 9,750, 80, 50,200, 10,  2,  8,  90, 5,1000,  500,2000,NO_THING, WEAPON_GIANT_CLUB, COMMON, M_TALK_GREEDY, M_MOVE_NORMAL, M_MELEE_NORMAL, M_STRIKE_MISSILE, M_SP_HUGE, MOBILE | HOSTILE | HUNGRY | GREEDY, pow2 (COLD) | pow2 (FEAR), 'J' | CLR_GREY_BROWN, "jotun", "jotun's head", "AHAHB?"},
    {133, 9,1000,50, 30, 40, 20,  2,  4,  40, 0, 750,  100,200, CLOAK_OF_INVISIBILITY, NO_THING, COMMON, M_TALK_SILENT, M_MOVE_FLUTTER, M_MELEE_NORMAL, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | FLYING | M_INVISIBLE, pow2 (ELECTRICITY), 'i' | CLR_BLACK_WHITE, "invisible slayer", "dead slayer", "AXAXAX"},
    {134, 9,700, 50, 32, 50,  5,  2,  3,  80, 4,1000,  100, 50, RING_OF_POISON_RESISTANCE, NO_THING, COMMON, M_TALK_GREEDY, M_MOVE_NORMAL, M_MELEE_POISON, M_NO_OP, M_NO_OP, MOBILE | HOSTILE | GREEDY | FLYING, pow2 (FLAME) | pow2 (POISON), 'W' | CLR_GREEN_RED, "king wyvern", "king wyvern's sting", "AXAXAXBXBX"},
    {135, 9,500, 30, 50, 30, 10,  2,  4,  50, 0, 500,   40,500, WEAPON_FIRESTAR, NO_THING, COMMON, M_NO_OP, M_MOVE_RANDOM, M_MELEE_NORMAL, M_NO_OP, M_SP_EXPLODE, MOBILE | HOSTILE | HUNGRY | FLYING | POISONOUS, pow2 (FLAME), 'd' | CLR_BLACK_WHITE, "deathstar", "deathstar's eye", "BX"},
    {136, 9,250, 20, 25, 25,  5,  5,  4,  50, 4,1500,   50,3000,NO_THING, NO_THING, COMMON, M_TALK_EVIL, M_MOVE_SMART, M_MELEE_NORMAL, M_SP_SPELL, M_SP_SPELL, MOBILE, pow2 (FLAME) | pow2 (COLD) | pow2 (ELECTRICITY), 'T' | CLR_YELLOW_WHITE, "high thaumaturgist", "dead thaumaturgist", "A?B?B?B?B?"},
    {137, 9,750, 70, 50, 50, 10,  1,  2,  50, 0,1000,    1,200, CLOAK_OF_NEGIMMUNITY, NO_THING, COMMON, M_TALK_EVIL, M_MOVE_SPIRIT, M_MELEE_SPIRIT, M_MELEE_SPIRIT, M_SP_SURPRISE, MOBILE | HOSTILE | FLYING | M_INVISIBLE, pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP), 'V' | CLR_BLACK_RED, "vampire lord", "vampire dust", "A?A?A?B?B?B?"},
    {138, 9,1000,100,50, 50, 20, 20,  2, 100, 5,1500,   15,3000,NO_THING, WEAPON_DEFENDER, COMMON, M_TALK_SILENT, M_MOVE_SMART, M_MELEE_NORMAL, M_MELEE_SLEEP, M_SP_ANGEL, MOBILE | FLYING, pow2 (FLAME) | pow2 (COLD) | pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP) | pow2 (OTHER_MAGIC), 'A' | CLR_YELLOW_WHITE, "archangel", "archangel's diadem", "L?R?R?R?R?R?R?R?R?"},
    {139, 9,800, 80, 40, 80, 12, 12,  2,   0, 5,1200, 4500,3000,NO_THING, WEAPON_DEMONBLADE, COMMON, M_TALK_EVIL, M_MOVE_SMART, M_MELEE_NORMAL, M_MELEE_NORMAL, M_SP_DEMON, MOBILE | HOSTILE | FLYING | POISONOUS, pow2 (FLAME) | pow2 (POISON) | pow2 (FEAR) | pow2 (SLEEP) | pow2 (OTHER_MAGIC), 'D' | CLR_BLACK_RED, "demon prince", "dead demon prince", "LLLCLHRLRCRHRLRCRH"},
    {140,10,100, 10, 10,  1,100,100,100,   0, 0,   0,    0,  1, NO_THING, WEAPON_SCYTHE_OF_DEATH, UNIQUE_MADE, M_TALK_EVIL, M_MOVE_SMART, M_MELEE_DEATH, M_NO_OP, M_SP_DEATH, MOBILE | HOSTILE, pow2 (FEAR) | pow2 (SLEEP) | pow2 (NEGENERGY) | pow2 (OTHER_MAGIC), 'D' | CLR_BLACK_WHITE, "Death", "No way can you be seeing this!", "AX"},
    {141,10,1000, 0, 20,  0,  3,  3,  5,  50, 0,10000,1000,250, STAFF_OF_DISPELLING, NO_THING, UNIQUE_MADE, M_TALK_SILENT, M_MOVE_RANDOM, M_NO_OP, M_NO_OP, M_SP_EATER, MOBILE, pow2 (FLAME) | pow2 (COLD) | pow2 (ELECTRICITY) | pow2 (POISON) | pow2 (ACID) | pow2 (FEAR) | pow2 (SLEEP) | pow2 (NEGENERGY) | pow2 (OTHER_MAGIC) | pow2 (THEFT) | pow2 (GAZE) | pow2 (INFECTION), 'E' | CLR_BLACK_WHITE, "The Eater of Magic", "The Heart of the Eater of Magic", "B?B?B?B?B?"},
    {142,10,500, 50, 50, 50,  5,  3,  5,  50, 0,10000,1000,2000,NO_THING, WEAPON_DEFENDER, UNIQUE_MADE, M_TALK_LB, M_MOVE_SMART, M_MELEE_NORMAL, M_NO_OP, M_SP_LAWBRINGER, MOBILE, pow2 (NORMAL_DAMAGE), 'L' | CLR_BRIGHT_WHITE_BLUE, "The LawBringer", "The LawBringer's Crown", "A?A?B?B?B?B?"},
    {143,10,2000,100,100,100,10,  1,  1, 100,10,10000,1000,2000,ARMOR_DRAGONSCALE, NO_THING, UNIQUE_MADE, M_TALK_EVIL, M_MOVE_NORMAL, M_MELEE_DRAGON, M_STRIKE_FBOLT, M_SP_DRAGONLORD, MOBILE | HOSTILE | FLYING, pow2 (FLAME) | pow2 (COLD) | pow2 (ELECTRICITY) | pow2 (ACID), 'D' | CLR_BRIGHT_WHITE_RED, "The Dragon Lord", "The Dragon Lord's Hide", "A?A?A?A?A?B?B?B?"},
    {144,10,3000,200,100,200,20, 20,  2,   0, 7,20000,1000,3000,NO_THING, AMULET_OF_THE_PLAINS, UNIQUE_MADE, M_TALK_EVIL, M_MOVE_SPIRIT, M_MELEE_SPIRIT, M_MELEE_NORMAL, M_SP_DEMON, MOBILE | HOSTILE | INTANGIBLE | M_INVISIBLE, pow2 (FLAME) | pow2 (COLD) | pow2 (ELECTRICITY) | pow2 (POISON) | pow2 (ACID) | pow2 (FEAR) | pow2 (SLEEP) | pow2 (NEGENERGY) | pow2 (OTHER_MAGIC) | pow2 (THEFT) | pow2 (GAZE) | pow2 (INFECTION), 'D' | CLR_GREY_RED, "The Demon Emperor", "The Demon Emperor's Regalia", "L?L?L?L?L?R?R?R?R?R?"},
    {145,10,2000,200,100,200, 5,  1, 12,   0, 0,5000,10000,  1, NO_THING, ORB_OF_EARTH, UNIQUE_MADE, M_TALK_SILENT, M_MOVE_SPIRIT, M_MELEE_NORMAL, M_MELEE_NORMAL, M_SP_DE, MOBILE | INTANGIBLE, pow2 (FLAME) | pow2 (COLD) | pow2 (POISON) | pow2 (ACID) | pow2 (FEAR) | pow2 (SLEEP) | pow2 (NEGENERGY) | pow2 (OTHER_MAGIC) | pow2 (THEFT) | pow2 (GAZE) | pow2 (INFECTION), 'E' | CLR_BROWN_WHITE, "The Elemental Lord of Earth", "some dirt", "A?B?B?B?"},
    {146,10,1000,100,50, 50, 20,  1,  1, 100, 0,5000,    0,  1, NO_THING, ORB_OF_AIR, UNIQUE_MADE, M_TALK_SILENT, M_MOVE_SPIRIT, M_MELEE_NORMAL, M_STRIKE_LBALL, M_SP_WHIRL, MOBILE | FLYING | INTANGIBLE | M_INVISIBLE, pow2 (FLAME) | pow2 (COLD) | pow2 (ELECTRICITY) | pow2 (POISON) | pow2 (ACID) | pow2 (FEAR) | pow2 (SLEEP) | pow2 (NEGENERGY) | pow2 (THEFT) | pow2 (GAZE) | pow2 (INFECTION), 'A' | CLR_LIGHT_BLUE_WHITE, "The Elemental Lord of Air", "some air", "A?A?A?A?A?A?A?A?A?B?"},
    {147,10,1500,100,100,100, 5,  1,  5, 100, 0,5000,   10,  1, NO_THING, ORB_OF_WATER, UNIQUE_MADE, M_TALK_SILENT, M_MOVE_NORMAL, M_MELEE_NORMAL, M_STRIKE_SNOWBALL, M_NO_OP, MOBILE | ONLYSWIM, pow2 (COLD) | pow2 (ELECTRICITY) | pow2 (POISON) | pow2 (ACID) | pow2 (FEAR) | pow2 (SLEEP) | pow2 (NEGENERGY) | pow2 (OTHER_MAGIC) | pow2 (THEFT) | pow2 (GAZE) | pow2 (INFECTION), 'W' | CLR_BLUE_WHITE, "The Elemental Lord of Water", "some water", "A?A?A?B?B?B?"},
    {148,10,1500,200,100,255,10,  1,  3, 100, 0,5000, 1000,  1, NO_THING, ORB_OF_FIRE, UNIQUE_MADE, M_TALK_SILENT, M_MOVE_NORMAL, M_MELEE_FIRE, M_STRIKE_FBALL, M_NO_OP, MOBILE, pow2 (FLAME) | pow2 (ELECTRICITY) | pow2 (POISON) | pow2 (ACID) | pow2 (FEAR) | pow2 (SLEEP) | pow2 (NEGENERGY) | pow2 (OTHER_MAGIC) | pow2 (THEFT) | pow2 (GAZE) | pow2 (INFECTION), 'F' | CLR_LIGHT_RED_WHITE, "The Elemental Lord of Fire", "a spark", "A?A?A?R?R?R?R?"},
    {149,10,1000,200,200,200,20, 20,  2,   0,10,10000,10000, 1, NO_THING, ORB_OF_MASTERY, UNIQUE_MADE, M_TALK_EVIL, M_MOVE_SMART, M_MELEE_MASTER, M_STRIKE_MASTER, M_SP_MASTER, MOBILE | FLYING | SWIMMING, pow2 (NORMAL_DAMAGE) | pow2 (COLD) | pow2 (ELECTRICITY) | pow2 (POISON) | pow2 (ACID) | pow2 (FEAR) | pow2 (SLEEP) | pow2 (NEGENERGY) | pow2 (OTHER_MAGIC) | pow2 (THEFT) | pow2 (GAZE) | pow2 (INFECTION), 'M' | CLR_PURPLE_WHITE, "The Elemental Master", "A burning mound of bubbling mud", "A?A?A?A?R?R?R?R?"}
};
//}}}-------------------------------------------------------------------
//{{{ Help files
//----------------------------------------------------------------------
//{{{2 Overview
const char Help_Overview[] =
"OMEGA OVERVIEW\n"
"==============\n"
"omega is a complex rogue-style game of dungeon exploration.  Unlike\n"
"other such games, there are a number of ways to \"win\", depending on\n"
"various actions taken during play. The ways you can get your name on\n"
"the hiscore board include becoming the highest ranked head of a guild,\n"
"sect, college, etc., as well as gaining the most points figured from\n"
"possessions and experience. The game (via the oracle) may impose some\n"
"structure on your exploration, but you need not follow all of the\n"
"oracle's advice. There *is* a \"total winner\" status, by the way.\n"
"\n"
"omega offers a richness of playing detail that goes beyond a simple\n"
"game like rogue. However, the majority of gameplay is very similar to\n"
"rogue, hack, ultrarogue, larn, and other such games. The player is\n"
"represented by the highlighted \"@\" symbol, objects and terrain\n"
"features are represented by non-alphabetic symbols, monsters are\n"
"represented by the various upper and lower case letters, and other\n"
"humans are represented by a non-highlighted \"@\". It is recommended\n"
"that the novice read the man pages for rogue or some other such game\n"
"and perhaps play a few games before playing omega. \n"
"\n"
"omega currently consists of a countryside in which are sited a city,\n"
"various villages, several dungeons, and some \"special\" sites, such as\n"
"the temple HQ's of the various priesthoods. The game starts in the\n"
"city of Rampart, where all the guilds have headquarters. As a dungeon\n"
"is explored, all generated levels are saved in memory. However, only\n"
"one dungeon is \"remembered\" at a time, so if you enter two dungeons,\n"
"the first will be forgotten (it will be regenerated should you wish to\n"
"enter it again). However, if a dungeon is \"completed\", this fact will\n"
"be remembered, basically meaning that the unique monster at the bottom\n"
"will not be regenerated if you've killed him already.\n"
"\n"
"You will first want to explore the city of Rampart. Although the\n"
"architecture and important sites remain the same from game to game,\n"
"details such as the precise position of certain shops, etc. will\n"
"change. Rampart is relatively safe if you don't wander into places\n"
"that are obviously dangerous or do things like breaking and entering.\n"
"If you are reckless, it is quite possible to get killed before ever\n"
"finding the dungeons. The 'M' command now allows you to return\n"
"immediately to any establishment you have already visited that game,\n"
"and is preset to \"know\" the permanent locations of Rampart.\n"
"\n"
"Many Rampart establishments are only open in the daytime, but some are\n"
"only active at night. Due to the stringent anti-crime measures taken\n"
"by the Duke, it is usually safe to sleep in the streets. You will find\n"
"it far more rewarding, however, to rent or purchase your own lodgings,\n"
"if you can afford them.\n"
"\n"
"The point of omega, though, is dungeon exploration. There are a number\n"
"of dungeons hidden about the countryside. These dungeons are at\n"
"different levels of difficulty and have various idiosyncratic\n"
"differences. The easiest are the Caves of the Goblin King, which is\n"
"due south of Rampart, in the nearby mountains. The caves must be\n"
"searched for with 's' command unless you are lucky enough to move\n"
"directly onto them.";
//}}}2------------------------------------------------------------------
//{{{2 Character creation
const char Help_Character[] =
"CHARACTER CREATION AND DEVELOPMENT\n"
"==================================\n"
"You will have the opportunity to create a new character each time you\n"
"start up omega. A random selection of statistics will be generated.\n"
"To discourage sitting around trying to get an \"optimal\" character,\n"
"you can only \"reroll\" 9 times before having to reload the game. Since\n"
"your statistics tend to go up as you play, it isn't fatal to\n"
"have low statistics when you start out. On the other hand, high stats\n"
"don't hurt, either....\n"
"\n"
"Each statistic is very important, but depending what sort of character\n"
"you wish to play, different statistics will stand out. Combat is\n"
"helped by Strength and Dexterity, Magic-use is aided by Intelligence\n"
"and Power. Agility helps you avoid getting hit and avoid traps, while\n"
"Constitution determines how many hits you can take before dying.\n"
"\n"
"A new option for omega is to \"play yourself\". By answering a sequence\n"
"of personal questions, omega arrives at what your personal statistics\n"
"should be. It is in seriously bad taste to lie.... Only the character\n"
"you create in this mode can be saved to the omegarc. The questions are\n"
"set up so that a \"renaissance man\" type of person will probably have\n"
"stats generally around 14-15. The rest of us will stay around 9-12 on\n"
"average. If you are really impressively good at something you can\n"
"start out with a stat of up to 18, but this is pretty unlikely for\n"
"most of us.\n"
"\n"
"As you continue to play, you will hopefully accumulate experience and\n"
"treasure. You can gain skills and abilities through various means,\n"
"the most important being training in one of the Rampart establishments\n"
"devoted to self-improvement. Thus, you can choose to work out at the\n"
"gym, study at the Collegium Magii, meditate at a Temple, and\n"
"so on. Certain skills and abilities may only be gained through\n"
"adventuring, and others are mutually exclusive. For example, you can\n"
"only be a priest of one particular deity. One of the appeals of\n"
"omega is the variety of different types of characters you can play,\n"
"each with its own set of goals and benefits.\n"
"\n"
"Another aspect of your character is alignment. Characters are aligned\n"
"towards the primal forces of Chaos and Law, or they can be neutral.\n"
"Various actions affect this factor; killing peaceful monsters, or\n"
"committing burgalry, for example, are chaotic acts.  Alignment\n"
"determines which guilds will let you in, the behavior of some\n"
"artifacts and monsters, as well as much else.\n"
"\n"
"Your character will gain various statuses, abilities, immunities,\n"
"etc., both harmful and beneficial. Most of the time these are kept\n"
"hidden from you, but there are various ways of discovering the truth\n"
"-- self-knowledge is very useful in omega. Remember, the oracle\n"
"knows all, sees all....\n"
"\n"
"Experience is accrued for skillful actions in the game. As in rogue,\n"
"one of the main methods of acquiring experience is defeating monsters.\n"
"You need not kill an opponent to defeat it (you can use the 'threaten'\n"
"option of the 't' command, sometimes).  There are many other ways of\n"
"gaining experience, though, including learning spells, deactivating\n"
"traps, using artifacts correctly, etc. \n"
"\n"
"Experience goes towards two kinds of character development. First,\n"
"in the bizarre tradition of role-playing games like * & *, hit\n"
"points and combat capabilities go up as experience is gained.\n"
"\n"
"Secondly, experience gained is shared out to each of the guilds,\n"
"priesthoods, etc. that the character belongs to. The more guilds\n"
"the character is a member of, the slower promotion will be\n"
"(except in the City Nobility, and among the Gladiators, where\n"
"promotion is not based on experience, but on quests and combats,\n"
"respectively). Regardless of the number of guilds the character\n"
"is a member of, the first type of experience (overall character\n"
"level) will not be adversely affected.";
//}}}2------------------------------------------------------------------
//{{{2 Inventory
const char Help_Inventory[] =
"INVENTORY\n"
"=========\n"
"omega inventories are now much different from other rogue-like games.\n"
"\n"
"Unlike rogue, et al., you cannot directly employ every object you\n"
"carry. Every object may either go into your \"pack\" or into some\n"
"specific inventory slot such as \"belt\" or \"ready hand\" or \"left\n"
"shoulder\" etc. Some slots are dedicated to certain types of items (eg,\n"
"only armor can go in the armor slot), but other slots are general\n"
"purpose, like your belt or your shoulder.\n"
"\n"
"It takes game time to transfer an object from your pack, (which can\n"
"hold a fixed number of items) to specific slots from which the item may be\n"
"useable. The pack is implemented as a mock-stack, so the deeper you burrow\n"
"into your pack looking for some item, the more game time you are\n"
"wasting. Therefore, carrying a whole lot of (semi)useless items may be\n"
"a strategically bad move.  Of course, as always, you can't carry more\n"
"weight (either in your pack or in your inventory slots) than is\n"
"dictated by your current STRENGTH and AGILITY.\n"
"\n"
"INVENTORY CONTROL MODES\n"
"=======================\n"
"There are two type of inventory control: Display mode and Quick mode.\n"
"Which mode you use by default can be toggled by the TOPLINE option\n"
"settable with the 'O' command and in your .omegarc file.\n"
"\n"
"In Display mode, all the inventory slots are displayed in the menu\n"
"window. In Quick mode, only the message lines above the main window\n"
"are used, and therefore the slots are not all displayed, only\n"
"one of them at a time. Additionally, the commands are slightly \n"
"different. One nuance to the display -- note the character following\n"
"the index of the slot in the following two lines.\n"
"\n"
"-- b> weapon hand: mace\n"
"-- c) left shoulder: short sword\n"
"\n"
"The '>' in the first line indicates the mace is \"in use\", while \n"
"the ') in the second lines means the short sword is not currently\n"
"being used. If it is possible for an item to be \"in use\" in the\n"
"current slot, it will be indicated by a '>'. Therefore, if you\n"
"don't want to put some new unidentified weapon into use, don't\n"
"try to carry it in the weapon hand; the same holds for shields\n"
"in the shield slot, etc.\n"
"\n"
"The amount of time each action takes is indicated below. Dropping two\n"
"items takes twice as long as dropping one item.\n"
"\n"
"DISPLAY MODE:\n"
"^l,^r: Redisplay inventory (if screen is munged). Takes no time.\n"
"d:  Drop the item currently in the 'up-in-the-air' slot, or in the\n"
"    'selected' slot if there is nothing 'up-in-the-air'. Takes one second.\n"
"p:  Put the 'up-in-the-air' item into your pack, or the 'selected'\n"
"    item, if the 'up-in-the-air' item is vacant. Takes 5 seconds.\n"
"l:  Looks at the item in the selected slot.  Takes no time.\n"
"s:  Show the contents of your pack. Takes 5 seconds.\n"
"t:  Take an item from your pack and put into the 'selected' slot, or\n"
"    if that is full, tries to put into the 'up-in-the-air' slot.\n"
"    Takes 5 seconds + 1 second/item examined in pack. I.e., rummaging\n"
"    at three items before taking one takes 8 seconds.\n"
"e:  Exchange the 'up-in-the-air' slot with the 'selected' slot. This\n"
"    will merge two quantities of an item together if possible.\n"
"    Either slot may be vacant, in which case it is a simple 'put'\n"
"    or 'take' to a slot.\n"
"    Takes 2 seconds.\n"
"x:  Same as 'e' but quit inventory mode immediately if the\n"
"    up-in-the-air slot is vacant after the action (ie, if it wasn't\n"
"    an exchange, but was just a put). Helpful when picking up new items.\n"
"    Takes 2 seconds.\n"
">:  Cause the next slot lower down to be the 'selected' one. Takes no time.\n"
"<:  Cause the next slot higher up to be the 'selected' one. Takes no time.\n"
"?:  Print this help. Takes no time.\n"
"ESCAPE: return to game, dropping any object in the 'up-in-the-air' slot.\n"
"        Takes no time.\n"
"\n"
"In Display Mode, the 'selected' slot is the one with a highlighted\n"
"'->' before it.\n"
"\n"
"The 'e' and 'x' commands in both modes are the ones you'll use most;\n"
"it combines taking and putting from inventory slot to \"up-in-air\"\n"
"where picked-up items will be, and where you will drop items from on\n"
"leaving the inventory mode. Usually the pack is used for items which\n"
"it is not important to be able to get at easily, while inventory\n"
"slots are for useful items.";
//}}}2------------------------------------------------------------------
//{{{2 Movement
const char Help_Movement[] =
"MOVEMENT\n"
"========\n"
"The mechanics of movement are much like rogue. You can use either the\n"
"vi keys (hjklbnyu) or the numeric keypad to move; the capitalized vi\n"
"keys or a keypad direction preceded by a 5 \"run\" in that direction\n"
"until either an object is run into, or an \"interesting\" location is\n"
"hit. More locations are \"interesting\" if the \"RUNSTOP\" option is set.\n"
"In most environments, a single move takes but a few seconds of game\n"
"time, depending on the player's \"Spd\" score, but some places movement\n"
"takes longer. In the countryside environment, the scale is much larger\n"
"than usual (say 5KM/move), so a single move can take hours. (Riding a\n"
"horse can reduce the amount of time taken in the country, depending on\n"
"the terrain being travelled). In the city or village environments, the\n"
"scale is such that a move takes about 30 seconds or so of gametime.";
//}}}2------------------------------------------------------------------
//{{{2 Combat
const char Help_Combat[] =
"COMBAT\n"
"======\n"
"As in most rogue-like games, you attack an adjacent monster by\n"
"attempting to move onto it.\n"
"\n"
"At this time, your current real statistics on to-hit, damage, defense,\n"
"and armor protection are always displayed. It is felt that the\n"
"character would know (or quickly discover) how well any weapon, armor,\n"
"etc. actually work. By comparing different weapons, armor, and shield\n"
"combinations, you can decide what martial configuration is best for\n"
"your character.  Keep in mind that cost of equipment does not\n"
"necessarily indicate how good it is (though it is a good rule of\n"
"thumb).\n"
"\n"
"The 'F' command allows you to select precisely which actions you will\n"
"take in a combat round -- parry high, thrust low, etc.  You have\n"
"several maneuver points for use in each combat round, depending on\n"
"level, status as a gladiator, etc. At the beginning of the game, your\n"
"combat options are already set for you, but as you progress, and\n"
"possibly gain more maneuvers, the preset options will not be\n"
"automatically updated, so remember to do it yourself periodically, as\n"
"you may be losing the chance at more maneuvers if you do not.\n"
"\n"
"The following are your combat options:\n"
"\n"
"A) ATTACK: Punch, Cut, Strike, Shoot, or Thrust (depending on weapon) -- 1 pt.\n"
"B) BLOCK: Prepare to block or parry enemy attack                      -- 1 pt.\n"
"R) RIPOSTE: BLOCK/ATTACK (thrusting weapons only)                     -- 2 pts.\n"
"L) LUNGE: More chance to hit than usual, does more damage             -- 2 pts.\n"
"\n"
"Keep on selecting maneuvers until you have no maneuver points left.\n"
"It does absolutely no good to have points left over.\n"
"\n"
"Following the choice of the maneuver, you must select a line for the\n"
"action: L (low), C (center), or H (high). Some monster will tend to\n"
"attack in a given line (short/small monsters, for example, will\n"
"tend to attack low), so this choice is meaningful. If you block or\n"
"riposte low, when the attack is high, you will have wasted the effect\n"
"of the block. On the other hand, if you block low twice, and the\n"
"attack is low, you will defend doubly. If you don't block or riposte\n"
"at all, it will be particularly easy for monsters to hit you.\n"
"\n"
"Keep on selecting maneuvers until you have no maneuver points left.\n"
"Hitting the BACKSPACE or DELETE key will erase your current choices\n"
"and let you start over. When you are satisfied, confirm your\n"
"choice with the RETURN key. Once a choice is selected, it stays in\n"
"effect until you change it; you need not reset it for each attack.\n"
"Selecting combat maneuvers takes no time.\n"
"\n"
"If you are in VERBOSE mode (settable with the 'O' command) you'll see\n"
"just what the monster is doing, whether the blocks are effective,\n"
"and so on. Otherwise you'll just be told whether you hit or miss.\n"
"\n"
"If you select VERBOSE mode, you will be able to see just where your\n"
"opponent attacks. Some monsters may show a particular sequence or\n"
"location which they attack in; you can use this information to your\n"
"advantage by setting your own attacks and blocks accordingly.\n"
"\n"
"You can try multiple identical maneuvers if you like. If you don't try\n"
"any BLOCKS at all, it will be easier than usual for your opponent to\n"
"hit.  Even if you don't get the location correct, a BLOCK still does\n"
"some good. A RIPOSTE attacks in the same line as its block, if you got\n"
"the line right. I.e., if you are attacked high, a high riposte will\n"
"get a free attack back. If you are attacked high three times, a high\n"
"riposte will get three free attacks back. Each block or riposte is\n"
"good for your entire move; no matter how many monsters attack you you\n"
"will be able to block or riposte each one. Naturally, each attack can\n"
"only be used against one monster.\n"
"\n"
"Heavy edged weapons and smashing weapons do CUTTING or STRIKING\n"
"damage. Your strength affects this, so if it is high you will do\n"
"much more damage, and if low, much less.\n"
"\n"
"Light edged weapons, missile weapons, and pointed weapons do THRUSTING\n"
"or SHOOTING attacks, whose chance to hit is greatly affected by your\n"
"dexterity. Note that you can't use a missile weapon very well in\n"
"ordinary combat (i.e. when you move adjacent to a monster) -- you\n"
"would be using a crossbow as a club, for instance. Instead,\n"
"use the 'f' command to fire missiles when at long range, and change\n"
"to a melee weapon for close combat.\n"
"\n"
"A note on weapons use: A weapon is not prepared for combat unless\n"
"it is in the \"weapon hand\" slot in your inventory. Anywhere else\n"
"means you are just carrying it around.... Two-handed weapons will\n"
"automatically take both hand slots, so remember to have both\n"
"hands free if you want to wield such a weapon.\n"
"\n"
"Shields, of course, will only have effect in the \"shield\" slot,\n"
"while armor is only considered to be worn if it is in the \"armor\"\n"
"slot.";
//}}}2------------------------------------------------------------------
//{{{2 Bugs and Features
const char Help_Bugs[] =
"BUGS AND FEATURES\n"
"=================\n"
"\"It's not a bug, it's an undocumented feature.\" \n"
"\n"
"omega is not just a large program, it is a large, complicated, and not\n"
"particularly well written program. There are so many features that not\n"
"all can be well tested, particularly in combination with one another.\n"
"Therefore, it is likely that your version, whatever it is, has some\n"
"bugs. The author no longer strives to correct bugs and remedy\n"
"misfeatures, so you probably shouldn't send mail to\n"
"brothers@paul.rutgers.edu.  However, if you send such mail to\n"
"omega@alcyone.darkside.com (Erik Max Francis), there may be attempts\n"
"to remedy the problem - don't hold your breath, though.";
//}}}2------------------------------------------------------------------
//{{{2 Magic
const char Help_Magic[] =
"MAGIC\n"
"=====\n"
"Your character may be taught spells by his or her guild or priesthood,\n"
"at the collegium magii, by the sorceror's guild, or pick them up from\n"
"a scroll of spells. Every spell costs some amount of mana to cast.\n"
"Mana is not normally regenerated except when the player goes up an\n"
"experience level, but various magic items augment mana, and the\n"
"sorceror's guild will recharge you for a fee. If you learn the same\n"
"spell multiple times, the effective power drain to cast the spell is\n"
"lowered. Some spells have a constant effect, while others vary\n"
"according to your level.  The spell of ritual magic has a variety of\n"
"different effects depending on the circumstances under which it is\n"
"cast. The spell of high magic is the most powerful, by the way.";
//}}}2------------------------------------------------------------------
//{{{2 Countryside
const char Help_Countryside[] =
"THE COUNTRYSIDE\n"
"===============\n"
"Rampart is set in a strange landscape of different terrain types. The\n"
"land is surrounded in part by a mystic sea of chaos which it is\n"
"probably a good idea to avoid. Screen characters have a different\n"
"meaning in the countryside than they do elsewhere, by the way, and\n"
"there is a different command set (accessible by '?' when out of the\n"
"city). Time passes much more quickly in countryside movement; you will\n"
"have to bring a lot of food with you, though you may wish to\n"
"hunt ('H' command) to supplement your food reserves; many of the\n"
"animals that you may encounter, for example, are edible.\n"
"\n"
"Since each countryside site is quite a large area, you may have\n"
"to search ('s' command) to learn of interesting sites nearby.\n"
"\n"
"Countryside Map Example:\n"
"\n"
"++----.--- The @ as usual is the player; the +'s are an arm of the Sea of\n"
"+++++O--^^ Chaos; the O is the city of Rampart; the ^'s are mountains;\n"
"----@----^ the .'s are a road; the -'s are plains and meadows;\n"
"---.-^^^^^ and the * is a dungeon entrance.\n"
"...-^*^^^^\n"
"\n"
"The countryside can be rather dangerous for low level characters to\n"
"explore at random. However, maps of the surrounding areas can be\n"
"purchased at the villages which can be found mostly at the ends\n"
"of the roads. Each village has a \"special\" magical site somewhere\n"
"inside its borders. These sites may be beneficial or harmful\n"
"depending how they are approached and the circumstances of their\n"
"activation.\n"
"\n"
"There are a number of \"special\" locations in the countryside; these\n"
"are basically one-level screens of various sorts, or they may be\n"
"multi-level dungeons. If you follow the advice of the oracle\n"
"who may be found somewhere in Rampart, you will wind up visiting most\n"
"of these sites.\n"
"\n"
"A special site or dungeon may be entered from the Countryside\n"
"environment by means of the '>' command. Depending whether or not it\n"
"is a multilevel or single level site, it may be exited simply by\n"
"moving off the edge of the screen, or by ascending or descending a\n"
"stairway from the first level of the dungeon.";
//}}}2------------------------------------------------------------------
//{{{2 Screen
const char Help_Screen[] =
"WHAT YOU SEE ON THE SCREEN \n"
"==========================\n"
"\n"
"Map: The large central area represents a map of the current\n"
"environment.  On it, the highlighted @ always represents the player,\n"
"letters represent monsters, non-highlighted @'s represent other\n"
"humans, and other characters represent either objects or terrain\n"
"features.  The map will take up as much of the screen as is possible,\n"
"so if you are running in a windowing system make the omega window as\n"
"large as it can be.\n"
"\n"
"Since the same character can represent different things or entities\n"
"(even in the same environment), a good command to remember is 'x'\n"
"(examine), which will tell you precisely what you are dealing with in\n"
"any situation. This is especially helpful before you have terrain\n"
"types like deep pools and lava pits (!) memorized.... You can use the\n"
"CONFIRM option to avoid walking absent-mindedly into a trap, pool,\n"
"etc.  The '/' command will give you more general help, for example\n"
"telling you that the ')' character represents a weapon. The '?'\n"
"command can give you either a command list or this file. The command\n"
"lists will differ depending whether you are in the countryside, or a\n"
"dungeon or city.\n"
"\n"
"Messages: Directly above the map window is a three line area that\n"
"shows current messages from the game describing what is going on.\n"
"The ^p command, or ^o for IBM users, shows old messages.\n"
"\n"
"Location: Directly beneath the map window is a one line description\n"
"of your current location or environment.\n"
"\n"
"Statistics: Beneath the Location line are two lines that show\n"
"the current state of the player's statistics such as strength,\n"
"dexterity, constitution, agility, intelligence, power, mana,\n"
"gold pieces, carrying capacity, etc. When the value given looks\n"
"like  number1 / number2, number1 represents the current value,\n"
"and number2 represents a maximum value.\n"
"\n"
"Flags: In the upper right corner of the omega display, below the date\n"
"and time, and phase of the moon, you will see four flags: The top\n"
"describes your food status, the second describes your status with\n"
"respect to poison, the third your health (diseased or healthy), and\n"
"the fourth, your mode of transportation.  While you are diseased you\n"
"will not regain hit points naturally. While you are poisoned you will\n"
"take damage periodically until cured or killed (or the poison's effect\n"
"can naturally fade).  When your food runs out, and you start starving,\n"
"you will take more and more damage as time passes until finally you\n"
"die of starvation. It's always a good idea to stock up at the\n"
"Commandant's.... If you manage to acquire a horse, your SPEED will\n"
"probably improve (unless you were very fast already). Regardless of\n"
"SPEED, having a horse will save some time in countryside movement, and\n"
"may also help out your combat strength. However, horses tend not to\n"
"enjoy going into dungeons....\n"
"\n"
"Combat Abilities: In the lower right of the display, your current\n"
"combat abilities are displayed. These are:\n"
"\n"
"Hit: is your current chance to hit with the weapon currently wielded,\n"
"     the higher the better. This factor varies with your DEXTERITY,\n"
"     experience level, and the weapon wielded.\n"
"Dmg: is the maximum amount of damage you can do with a hit. This factor\n"
"     varies with your STRENGTH and the weapon wielded.\n"
"Def: is your defensive rating, how hard it is for monsters to hit you. This\n"
"     factor varies with your AGILITY and experience level, and with\n"
"     the shield being used.\n"
"Arm: is your armor rating, how much damage is absorbed when a monster\n"
"     hits you before you take damage. Armor only absorbs normal damage;\n"
"     various effects and forms of magic may penetrate armor. This factor\n"
"     varies with the armor worn.\n"
"Spd: is a speed factor, how quickly or how slowly you move. This factor is\n"
"     calculated about a base of 1.00; 2.00 means you move twice as fast;\n"
"     0.50 means you move twice as slowly as normal. This factor varies\n"
"     with your AGILITY and the amount of weight you are carrying.\n"
"     Every monster has its own speed factor. The maximum speed is 5.0;\n"
"     the minimum is infinitesimal.\n"
"\n"
"Magic spells, special items, and monster effects can affect all these factors,\n"
"positively and negatively.";
//}}}2------------------------------------------------------------------
//{{{2 Saving and restoring
const char Help_Saving[] =
"SAVING AND RESTORING\n"
"==================== \n"
"Since omega's dungeon is quite large, only the current dungeon and\n"
"the city level will be saved; other dungeons you may have entered\n"
"previously will be regenerated as you re-enter them. If you know the\n"
"spell of Return, however, (learnable at the Explorers' Club) you will\n"
"be able to warp to your deepest excursion in any dungeon without\n"
"having to retraverse the old levels in between. Some other shortcuts\n"
"exist for \"warping\" from one locale or level to another.\n"
"\n"
"Games can be restored by giving the save file as a command line argument\n"
"as in:\n"
"\n"
"% omega quasar.sav\n"
"\n"
"To at least simulate the continuity of character in the game, saved\n"
"files will be unlinked on restoration. Of course, you *can* copy them.\n"
"\n"
"Save files are automatically compressed unless the flag\n"
"COMPRESS_SAVE_FILES is undefined in odefs.h. This doubles the amount\n"
"of time taken to save, but reduces the typical save file by a factor\n"
"of more than 10.  If you feel that the wait is too long, and have space\n"
"to burn, you can turn off save-file compression in the Options menu.";
//}}}2------------------------------------------------------------------
//{{{2 Options
const char Help_Options[] =
"OPTION SETTINGS\n"
"===============\n"
"Options may be set with the 'O' command. The options are:\n"
"\n"
"(F) BELLICOSE: Attack friendly monsters without confirmation.\n"
"(F) JUMPMOVE: Don't display things until the end of a run.\n"
"(T) RUNSTOP: Stop a run when something like a door is passed.\n"
"(F) PICKUP: Automatically pick things up when you move over them.\n"
"(T) CONFIRM: Ask confirmation before you do some dangerous things.\n"
"(F) TOPINV: Display inventory to message line, not to full screen.\n"
"(F) PACKADD: Add new items to pack, instead of going into inventory mode.\n"
"(T) COMPRESS: Determines if saved games are compressed (recommended).\n"
"(T) COLOUR: On IBMs, this toggles whether the display in in colour.\n"
"(V) VERBOSITY: TERSE, MEDIUM, or VERBOSE, the level of detail in combat.\n"
"(1) SEARCHNUM: the number of turns spent searching when you hit the 's' key.\n"
"\n"
"The default values are parenthesized. These options will be recovered from\n"
"your .omegarc if you use one.";
//}}}2------------------------------------------------------------------
//{{{2 City command list
const char Help_CityCommands[] =
"DUNGEON/CITY COMMAND LIST:\n"
"______________________________________________________________________________\n"
"key  : description                                                : # seconds\n"
"______________________________________________________________________________\n"
"^f   : abort the spell of shadow form                             :     0\n"
"^g   : enter wizard mode (WIZARD only)                            :     0\n"
"^i   : list items in pack                                         :     5\n"
"^l   : redraw screen (redraw each window)                         :     0\n"
"^p   : print previous message (more or less)                      :     0\n"
"^o   : print previous message (IBM version)                       :     0\n"
"^r   : redraw screen (redraw curscr)                              :     0\n"
"^x   : make a wish (wizard mode and Adepts only)                  :     0\n"
"^w   : magic-map screen (wizard mode only)                        :     0\n"
"a    : activate a wand, staff, or rod                             :     10*\n"
"c    : close door                                                 :     2*\n"
"d    : drop object at current location                            :     5*\n"
"e    : eat something                                              :     30\n"
"f    : fire/throw something                                       :     5*\n"
"g    : pick up object at current location                         :     10*\n"
"i    : primary inventory mode (as per TOPINV option)              :  variable\n"
"m    : cast a magic spell                                         :     20\n"
"o    : open door                                                  :     5*\n"
"p    : pick the pocket of an adjacent monster                     :     20*\n"
"q    : quaff a potion                                             :     10\n"
"r    : read a scroll                                              :     30*\n"
"s    : search all around you, # of times = SEARCHNUM              :     20\n"
"t    : talk to an adjacent monster                                :     10\n"
"v    : vault over a few intervening spaces                        :     10*\n"
"x    : examine a location                                         :     1\n"
"z    : bash something (adjacent location)                         :     10\n"
"A    : activate an item or artifact                               :     10\n"
"C    : call an item something                                     :     0\n"
"D    : disarm an adjacent trap                                    :     30\n"
"E    : dismount (exit?) from your horse                           :     10*\n"
"F    : set combat action sequence                                 :     0\n"
"G    : give something to an adjacent monster                      :     15\n"
"I    : secondary inventory mode (as per TOPINV option)            :  variable\n"
"M    : move quickly to some discovered site (city only)           :  variable\n"
"O    : set options                                                :     0\n"
"P    : public license information                                 :     0\n"
"Q    : quit                                                       :     0\n"
"R    : rename character                                           :     0\n"
"S    : save game and quit                                         :     0\n"
"T    : dig a tunnel through a wall                                :     30*\n"
"V    : version information                                        :     0\n"
"Z    : bash something (item carried)                              :     10*\n"
".    : rest a turn                                                :     10\n"
",    : sleep some number of minutes                               :  60..6000\n"
"@    : stay in current location, activating any local effect here :     5\n"
">    : go down a level                                            :     0~\n"
"<    : go up a level                                              :     0~\n"
"/    : identify the onscreen usage of some character              :     0\n"
"\n"
"vi keys (hjklbnyu) & keypad number: \n"
"       walk one space or fight adjacent monster (Regular Combat\n"
"       Mode), pick up things if PICKUP option set                 :     5*+\n"
"\n"
"capitalized vi keys (HJKLBNYU) or 5 followed by keypad number:\n"
"       run in that direction, fight adjacent monster if\n"
"       BELLIGERENT option set,don't display slowly if JUMPMOVE \n"
"       option set, stop for doorways, etc. if RUNSTOP option \n"
"       set.                                                       :     4*+\n"
"\n"
"?    : display or copy help files                                 :     0\n"
"\n"
"* Actual time taken varies according to SPEED factor.\n"
"+ Takes longer in City or Village (different scale for movement)\n"
"~ Reduced to 0 time by popular demand";
//}}}2------------------------------------------------------------------
//{{{2 Country command list
const char Help_CountryCommands[] =
"COUNTRYSIDE COMMAND LIST:\n"
"______________________________________________________________________________\n"
"key  : description                                                : time taken\n"
"______________________________________________________________________________\n"
"^p   : print previous message                                     : ---\n"
"^o   : print previous message (IBM version)                       : ---\n"
"^r   : redraw screen                                              : ---\n"
"d    : drop object (will be lost forever)                         : ---\n"
"e    : eat something                                              : ---\n"
"i    : inventory of things in pack (selected inventory mode)      : ---\n"
"s    : search all around you for hidden traces                    : 1 hour\n"
"H    : hunt for food                                              : 3 hours\n"
"I    : inventory of things in pack (alternate inventory mode)     : ---\n"
"O    : set options                                                : ---\n"
"P    : public license information                                 : ---\n"
"Q    : quit                                                       : ---\n"
"R    : rename character                                           : ---\n"
"S    : save game and quit                                         : ---\n"
"V    : version information                                        : ---\n"
">    : enter a village, city, dungeon, et al.                     : ---\n"
"/    : identify the onscreen usage of some character              : ---\n"
"?    : display or copy help files                                 : ---\n"
"\n"
"vi keys & keypad numbers:                                         : variable w.\n"
"       walk one space (about 20 kilometers)                         terrain";
//}}}2------------------------------------------------------------------
//}}}-------------------------------------------------------------------
//{{{ Data files
//{{{2 Abyss intro
const char Data_AbyssIntro[] =
"After entering the abyss, your surroundings dissolved in a violent\n"
"blue flash. You seemed to float through creme-de-menthe colored\n"
"gelatin for an eternity. Gradually you seemed to be approaching some\n"
"surface in the distance.....\n"
"\n"
"With a noise far louder than a popping soap-bubble, you find yourself\n"
"standing at one end of a narrow stone causeway. The floor is of\n"
"alternating squares of blue-veined orange and orange-veined blue\n"
"marble, for an effect rather like the Trump Tower atrium, except\n"
"worse. The causeway seems to hang suspended in the midst of a great\n"
"void. In the distance ahead the path seems to widen out into a large\n"
"platform. A chill runs down your spine and the floor is cold beneath\n"
"your bare feet. You notice that all your possessions seem to have been\n"
"lost somewhere.... All you have is your bare hands and brain....\n"
"\n"
"There is the sound of a single bell chiming, and the musical tone\n"
"somehow reverberates for long moments despite the lack of walls to\n"
"provide echoes.\n"
"\n"
"The note fades.\n"
"\n"
"With a sudden roar, a blue flame erupts from the center of the\n"
"causeway! A strangely familiar voice rings out from it:\n"
"\n"
"\"Foolish Mortal! You have entered upon the Adept's Challenge....\"\n"
"\n"
"\"There is no escape!\"\n"
"\n"
"\"There is no victory!\"\n"
"\n"
"The flame flickers out. Well, you've really done it this time, haven't you?";
//}}}2------------------------------------------------------------------
//{{{2 Play yourself intro
const char Data_Intro[] =
"(The project demo is due tomorrow, er, that is, today, at 0900. It's\n"
"now 0300. Printouts, pizza boxes, and szechuan kitchen trays litter\n"
"your desk, and the bitmap screen seems to warp in and out of focus....\n"
"The compiler seems to be taking *FOREVER*....)\n"
"\n"
"(You do a routine check on your filesystem...)\n"
"\n"
"% df\n"
"Filesystem            kbytes    used   avail capacity  Mounted on\n"
"/users                     0    6818       0   999%    /\n"
"/secur/omega            +inf    +inf    +inf     0%    /\n"
"\n"
"(How odd. The /secur filesystem definitely shouldn't be mounted; it must\n"
"be space used by the omega team, a very hush-hush project you know\n"
"nothing about. Well, no wonder the compiler is wedged; you don't seem\n"
"to have any disk space left. Unfortunately, there is no night\n"
"operator and you don't know the su password. Cursing, you figure you\n"
"can at least find out what the damn omega group is doing....)\n"
"\n"
"% cd /secur/omega\n"
"% ls \n"
"src/    bin/    doc/    tmp/    lib/    README\n"
"% more README\n"
"If you are reading this file, you should not be reading this file!\n"
"Call your project cadre leader at once!\n"
"%\n"
"\n"
"(Not bloody likely!)\n"
"\n"
"% ls doc\n"
"% (Hmm, not big on documentation, huh)\n"
"% ls src\n"
"omega.mpc\n"
"% (Wow, that must be mondo-parallel c, for the new optical machine!)\n"
"% ls bin\n"
"omega\n"
"% (Well, hell, what can happen? And if it does, it's not *MY* fault\n"
"the damn filesystem is mounted!)\n"
"% omega\n"
"\n"
"(This was definitely a mistake. For long moments nothing seemed to happen.\n"
"Then there was a flare of bluish light from the workstation screen,\n"
"like a brightness of 256 on a scale of 0..7. You wake up with a\n"
"serious headache outside the massive gates of a walled city. You notice\n"
"a pouch at your side that seems to be filled with.... gold! You seem\n"
"to hear a faint voice fading off into the distance \"Interrupt SIGZAP\n"
"at 0x0\"....)";
//}}}2------------------------------------------------------------------
//{{{2 Title message
const char Data_Title[] =
" red_kangaroo presents:                    T~~ \n"
"                                           | \n"
"                                          /\"\\ \n"
"                                  T~~     |'| T~~ \n"
"                              T~~ |    T~ WWWW| \n"
"                              |  /\"\\   |  |  |/\\T~~ \n"
"                             /\"\\ WWW  /\"\\ |' |WW| \n"
"                            WWWWW/\\| /   \\|'/\\|/\"\\ \n"
"                            |   /__\\/]WWW[\\/__\\WWWW \n"
"                            |\"  WWWW'|I_I|'WWWW'  | \n"
"                            |   |' |/  -  \\|' |'  | \n"
"                            |'  |  |OM+E+GA|' |   | \n"
"                            |   |' | |[_]| |  |'  | \n"
"                            |   |  |_|###|_|  |   | \n"
"                            '---'--'-/___\\-'--'---' \n"
"\n"
"                  Omega is copyright (C) 1987, 1988, 1989 by:\n"
"                             Laurence R. Brothers\n"
"          This product includes software developed by the University\n"
"                of California, Berkeley and its contributors.";
//}}}2------------------------------------------------------------------
//{{{2 Religion scroll
const char Data_ScrollReligion[] =
"Theogony of Omega,\n"
"\n"
"being an explication of the Ways of the Gods of Omega.\n"
"\n"
"Know ye, O ignorant ones, that Omega is the battleground of the mighty forces\n"
"of Law and Chaos. Powerful are the Champions and Avatars of the two great\n"
"forces, but most powerful of all are the Gods who, it is rumored, have\n"
"been summoned from other planes to represent the two forces.\n"
"\n"
"On the side of Law:\n"
"\n"
"  Odin, God of Vengeance, the Gallows God -- followers of Odin vow to\n"
"eradicate the taint of chaos wherever it may be found. Odin rewards\n"
"his faithful with the power to strike strong blows against the\n"
"Legions of Chaos.\n"
"\n"
"  Athena, Goddess of Justice and Wisdom -- devotees of Athena, while\n"
"prepared to fight the forces of chaos, choose a more intellectual\n"
"approach to the battle, marshalling the more subtle powers of magic\n"
"in their striving.\n"
"\n"
"On the side of Chaos:\n"
"\n"
"  Set, God of Destruction -- worshippers of the Black Hand of Set\n"
"will follow any path to destroy and corrupt Law and its minions.\n"
"Set teaches his followers powers equally as potent as those of Odin.\n"
"\n"
"  Hecate, Goddess of Illusion -- Hecate matches Athena's wisdom with \n"
"deception, and matches Athena's defensive powers with destructive\n"
"abilities.\n"
"\n"
"Followers of any of these deities must be careful to keep to the true path --\n"
"the wrath of a God is fearsome.... It is also said the Gods are jealous --\n"
"honoring another deity, even of the same alignment, is an extremely bad idea.\n"
"On the other hand, any act against a God of the opposite alignment would\n"
"be looked on kindly by your patron deity.\n"
"\n"
"It should be noted that there are still other Gods of Omega. The Druids\n"
"think themselves beyond the struggles of Law and Chaos and attempt to\n"
"maintain a Balance between the two -- they are despised by all the\n"
"aligned Gods. The ArchDruid is said to command powers greater than\n"
"any other cleric.\n"
"\n"
"The mysterious Lords of Destiny are said to preside over the struggles\n"
"of Law and Chaos as a referee rules a bout. Their powers are said to\n"
"be extremely subtle and pervasive, and while no one has ever seen this\n"
"power displayed, the self-appointed clerics of these mysterious beings\n"
"claim great abilities.  Certain it is that the aligned Gods have never\n"
"acted against the followers of the Lords of Destiny, though whether\n"
"this is because of their power, or because they offer no threat, is\n"
"unknown.";
//}}}2------------------------------------------------------------------
//{{{2 Rampart guidebook
const char Data_ScrollRampartGuide[] =
"Welcome to Rampart! A guidebook of the Rampart Tourist Association.\n"
"\n"
"Rampart is the sole city in this area of LOCUS, a bastion which the\n"
"wild forces of both Law and Chaos cannot breach. Within the town there\n"
"is a truce between the followers of Law and Chaos which is hardly ever\n"
"broken.\n"
"\n"
"The visitor to Rampart will enjoy a wide range of divertissements,\n"
"from shopping at Rampart's diverse assortment of shops, to attending\n"
"games at the Rampart Coliseum, to a relaxing picnic at the Botanical\n"
"Gardens. Distinguished visitors may apply for an audience with the\n"
"Duke, who is said to be the greatest courtier on LOCUS. Others spend\n"
"their nights (and days) gambling at the Rampart Casino.\n"
"\n"
"The traveller who intends a longer stay in Rampart should check out the\n"
"various guilds, societies, and clubs which protect and employ almost\n"
"all the citizenry. The Commandant of the Mercenary Guild is always\n"
"hiring, just for example. Many travellers are attracted to the\n"
"Collegium Magii -- scholarships are available to the qualified.\n"
"\n"
"A few cautions should be offered to the unwary. Rampart's sewer system\n"
"is dangerous, and while there is ABSOLUTELY NO TRUTH to the rumors of\n"
"treasure-filled dungeons and caverns beneath the city, many\n"
"disappearances each year are attributed to unauthorized excursions\n"
"below. For the larcenous-minded, be warned that Rampart's City Guard\n"
"is uncorruptible. It is just a matter of time, promises Morgon the\n"
"Justiciar, until the secret hideout of the Thieves' Guild is\n"
"uncovered....\n"
"\n"
"But these warnings are surely unnecessary for the vast majority of our\n"
"visitors who will be perfectly safe if they act with reasonable caution.";
//}}}2------------------------------------------------------------------
//{{{2 Hi Magick Scroll
const char Data_ScrollHiMagick[] =
"Being un Explication et Tretys on ye Powers of Hy Magick.\n"
"\n"
"It ys whysper'd among the dark mages of Kulthar and also ye Gnomes of\n"
"Zrenheim that Hy Magick is unattainable. But I know the truth! Hy\n"
"Magick ys hard to come bye, tis true, but ye mightie and ye fortunat\n"
"may even discover ye secrets thereof with carful application of\n"
"their talents. It ys known that many have had the oppportunitie to\n"
"invoke ye Hy Magick but have failed du to ignorance, or fear.\n"
"For knoe ye this: If Hy Magick be not invoked with exactitude,\n"
"it will not manifest, so ye mages, ye prentices, ye thaumaturges,\n"
"ye seers: Hearken! And learn ye the proper words of invocation.\n"
"Make sure the word is spoken precislie as I put it herein.\n"
"\n"
"'Death' is said by some to be a blessing, and it may be had from\n"
"ye offices of Hy Magick as I put it.\n"
"\n"
"Many practicers of the Recondite Arts might wish for 'Power', but\n"
"this ys truelie not as great an office as one myte desyre, being\n"
"of strong but ephemeral effect.\n"
"\n"
"'Skill', howbeit, ys a very fortunat benefice of Hy Magick.\n"
"\n"
"Let not ye greed for material possessions best ye, for 'Wealth'\n"
"does not signifie so much as some other choyces.\n"
"\n"
"The ones who walk the Way of the Druid may have occasion to request\n"
"'Balance' of ye Hy Magick, while other clarical seekers may\n"
"desire 'Law' or 'Chaos'.\n"
"\n"
"Truelie, 'Knowledge' is a great thing to gayn of the Hy Magick.\n"
"\n"
"Those poor ones who are afflicted myte well wish for 'Health'.\n"
"\n"
"Travellers culd find theyr destinations with 'Location'.\n"
"\n"
"Shuld a certain ytem of power be beyond thy grasp, 'Acquisition'\n"
"mought well gayn yt for thee.\n"
"\n"
"Those seekers who learn from ye studie ye beast supernal could\n"
"wish for 'Summoning'.\n"
"\n"
"Onlie True Adepts may wish for 'Destruction', it ys sayed.\n"
"\n"
"It ys mine hope that this list may be of servis to other seekers after\n"
"Wysdom.\n"
"\n"
"-- Octariis Otharian, Mage of Klosternon. Cycle of the Eft, Year of\n"
"the Slug, Day of the Dolphin.";
//}}}2------------------------------------------------------------------
//{{{2 Adept's manifesto
const char Data_ScrollAdept[] =
"[Librarian's note. This odd item just won't go away. Every time we\n"
"lend it out, it just appears back in the stacks.... Even burning it\n"
"didn't work....]\n"
"\n"
"                    The Adepts' Manifesto.\n"
"\n"
"\"Mastery of the World can only be achieved by the Master of the Self.\n"
"For one who has total Self-Control, all things are possible.\"\n"
"\n"
"[Obvious hyperbole -- Librarian]\n"
"\n"
"\"The apex of High Magic is a triviality for a true Adept.\"\n"
"\n"
"[Surely, this is exaggeration -- Librarian]\n"
"\n"
"\"An Adept makes his own Destiny, and Destiny caters to the whims of\n"
"an Adept.\"\n"
"\n"
"[Oh, come now, be serious. -- Librarian]\n"
"\n"
"\"Only Self-Improvement can lead to Adepthood. Those who have not\n"
"raised their personal attributes to the highest values will\n"
"be inevitably destroyed in their quest for Adepthood.\"\n"
"\n"
"[Say what? -- Librarian]\n"
"\n"
"\"The Challenge of Adepthood is maintained by the Lords of Destiny.\"\n"
"\n"
"[Nonsense! -- Librarian]\n"
"\n"
"\"An Adept-Aspirant must be prepared to go beyond the trivial powers of\n"
"Magic, even spurning the tawdry attractions of Artifacts, if the\n"
"Aspirant would achieve Adepthood.\"\n"
"\n"
"[Me, I'll take the Artifacts... -- Librarian]\n"
"\n"
"\"The final test of an Adept is Mastery of that Void which lies both\n"
"within and without the Self.\"\n"
"\n"
"[Zen Gobbledygook. -- Librarian]\n"
"\n"
"\"Only an Adept can become a Total Winner.\"\n"
"\n"
"[What the heck is a Total Winner? -- Librarian]";
//}}}2------------------------------------------------------------------
//}}}-------------------------------------------------------------------
//{{{ Levels
//
// Level data is prefaced with 4 bytes containing
// width, height, player start x, player start y
//
//{{{2 Abyss
const char Level_Abyss[] =
"\100\020\040\017"
"0000000000000000000000000.....#####.....000000000000000000000000\n"
"0000000000000000000000000...###~.6###...000000000000000000000000\n"
"0000000000000000000000000..##.......##..000000000000000000000000\n"
"0000000000000000000000000..##(.....;##..000000000000000000000000\n"
"0000000000000000000000000...##.....##...000000000000000000000000\n"
"0000000000000000000000000.##1##.V.##2##.000000000000000000000000\n"
"0000000000000000000000000..###..3..###..000000000000000000000000\n"
"0000000000000000000000000...............000000000000000000000000\n"
"0000000000000000000000000000000...000000000000000000000000000000\n"
"0000000000000000000000000000000...000000000000000000000000000000\n"
"0000000000000000000000000000000...000000000000000000000000000000\n"
"0000000000000000000000000000000...000000000000000000000000000000\n"
"0000000000000000000000000000000...000000000000000000000000000000\n"
"0000000000000000000000000000000...000000000000000000000000000000\n"
"0000000000000000000000000000000...000000000000000000000000000000\n"
"0000000000000000000000000000000...000000000000000000000000000000\n"
"0000000000000000000000000000000...000000000000000000000000000000";
//}}}2------------------------------------------------------------------
//{{{2 Arena
const char Level_Arena[] =
"\100\020\005\007"
"################################################################\n"
"################################################################\n"
"################################################################\n"
"################................................################\n"
"############........................................############\n"
"##########............................................##########\n"
"##########............................................##########\n"
"XPP...........................................................##\n"
"XPP...........................................................##\n"
"##########............................................##########\n"
"##########............................................##########\n"
"############........................................############\n"
"################................................################\n"
"################################################################\n"
"################################################################\n"
"################################################################";
//}}}2------------------------------------------------------------------
//{{{2 Circle
const char Level_Circle[] =
"\100\020\040\016"
"#################.e############ss###########.e##################\n"
"#################.###########......####.....##.#################\n"
"####!?.L.#######.########n...######....########.################\n"
"########.S......#######...#####ss#####n.########.###############\n"
"##############.#######.###.#n.......####.########.##############\n"
"##############.#######.####.S######S.####.#######.##############\n"
"#############.#######.#####.#......#.#####.#######.#############\n"
"#############.######s.###s.##TSSSST##.s###.s######.#############\n"
"#############.######s.###s.####-#####.s###.s######.#############\n"
"#############.#######.#####.##D..P##.n####.#######.#############\n"
"##############.#######.#####.######.#####.#######.##############\n"
"##############.#######.######......#..###.#######.##############\n"
"###############.#######..n#####ss#####...#######........########\n"
"################.########....######...n########.#######.########\n"
"#################.##.....####......###########.########...######\n"
"##################e.###########ss###########e.##################";
//}}}2------------------------------------------------------------------
//{{{2 City of Rampart
const char Level_City[] =
"\100\100\076\025"
"################################################################\n"
"#..............................................................#\n"
"#.######.#######..########..##x....x##.#########..###.###.###..#\n"
"#.######.######x..####x###..###.((.###.#########..#######.##x..#\n"
"#.x####x........................((.....#########..#######.###..#\n"
"#.######.######x..#x#..#x#..##x.((.###.####........####s.......#\n"
"#.######.#######..###..###..###.((.x##.###C.......#######.###..#\n"
"#.................###xx###......((.....###C.......#######.###..#\n"
"#..#######..####..########...#x.((.x##.####.......###.###.#x#..#\n"
"#.##2#6#5##.x###.###########.##.((.###.#########...............#\n"
"#..#.....#..###x.#.........#....((.x##.#########...............#\n"
"#.####.####.####.#.###.###.#.##.((.....#########..##....##...#.#\n"
"#..#3...4#.......#.##x.x##.#.#x.((.##.............###MM###..##.#\n"
"#.####.####.##...............##.((.##..######..##..######..#x#.#\n"
"#....#t#....##...#.##x.x##.#.#x.((.x#..#####x..###..####..####.#\n"
"#................#.###.###.#.##.((.##..######........##..#####.#\n"
"#.#########.####.#.........#.......##..######.######....####x..#\n"
"#.#########.####.#####.#####....##.....######.#####x#..#x###.###\n"
"#.#########.###x................##...........................#G#\n"
"#.####H.....####.....#B#...G##################################G#\n"
"#.#########.........#####......................................X\n"
"#.#########.######..#####......................................X\n"
"#.#########.######...###...G##################################G#\n"
"#............................................................#G#\n"
"#(((((((((((((((((((((((((((.......###########.###.#############\n"
"#(((((((((((((((((((((((((((.......#####x#####.###.#####x#####.#\n"
"#((.......................((...................................#\n"
"#((..((((.##########.((((.((.......######..#######.##x.##x.##x.#\n"
"#((..(=~1.#==#==#==#.1~=(.((.......x#####..##x#x##.###.###.###.#\n"
"#((..((((.#==#~~#==#.((((.((.......######......................#\n"
"#((.......#111~~111#......((.......######..#####...##...##.###.#\n"
"#((..((((.(..~~~~..(.((((.((...............#####x#.##...##.....#\n"
"#((..((((.(((1..1(((.((((.((.......#######......##.####.##x###.#\n"
"#((.......................g........x######.#x##....####.####x#.#\n"
"#((.#.((((((((((((((((((((((.......#######.####.##.##x#.....##.#\n"
"#(#y#mmmmmmmmmmmmmmmmmmmmmm(.......x######.x###.##..........##.#\n"
"#(#-#mmmmmmmmmmmmmmmmmmmmmm(.......#######.####....######...##.#\n"
"#(#.#mmmmmmmmmmmmmmmmmmmmmm(.......................######.####.#\n"
"#(#u#mmmmmmmmmmmmmmmmmmmmmm(.......###......###.##........##x#.#\n"
"#(#.#mmmmmmmmmmmmmmmmmmmmmm(.......x##.####.###.##.#x##x#...##.#\n"
"#(#.#mmmmmmmmmmmmmmmmmmmmmm(.......###.x###........######...##.#\n"
"#(#-#mmmmmmmmmmmmmmmmmmmmmm(...........####.#x##x#.######.#.##.#\n"
"#(#.#mmmmmmmmmmmmmmmmmmmmmm(.......###......######........#....#\n"
"#(#.#mmmmmmmmmmmmmmmmmmmmmm(.......x##.####.....##.##.###.#.##.#\n"
"#(#u#mmmmmmmmmmmmmmmmmmmmmm(.......###.####.###.#x.#x.###...##.#\n"
"#(#.#mmmmmmmmmmmmmmmmmmmmmm(.......x##......###.......##x##.##.#\n"
"#(#.#mmmmmmmmmmmmmmmmmmmmmm(.......###..####x##.##.##.#####.##.#\n"
"#(#.#mmmmmmmmmmmmmmmmmmmmmm(............####...................#\n"
"#(#u#######################(.......###.......##..###.#######.#.#\n"
"#(#.....-.....u-1-$U$U$U$U#(.......###..####.##..###.####x##.#.#\n"
"#(#########################(.......##x##x###.##..#x#.###..##.#.#\n"
"#(((((((((((((((((((((((((((...................................#\n"
"#.####~~~~~~~~~~~~~~~~~~####.......##*##*##*##.....######......#\n"
"#.####S#####################.......##*##*#*###...###....###....#\n"
"#.###S###S#^################.......##T#T#T#T##..###..##..###...#\n"
"#.##,###S#^#v#%%%%%%########.......##7#7#7#7##..##..####..##...#\n"
"#..#S##,######%%%%%%####c..........##R#R#R#R##..##..####..##...#\n"
"#..#^#%#####V%%%%%%%####c..........##.......##..###..##..###...#\n"
"#.###,######%%%%%%%%########.......####GJG####..####....####...#\n"
"#.##########################.....................####..####....#\n"
"#.##########################.......####GPG####.....##AA##......#\n"
"#.####..................####.......#####S#####.................#\n"
"#..................................####GjG####.................#\n"
"################################################################";
//}}}2------------------------------------------------------------------
//{{{2 Countryside
const char Level_Country[] =
"\100\100\033\023"
"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"
"+^^^^^%^^^^^^^^^^^^^---^^^^^^^^^^^^^^^^((((((------^^^^^^^^^~~^^\n"
"++^^~^^v^^^---------------((((((((((((((((((--------^^^^^^~~~~~^\n"
"+++^^~^^v^^------------(((((((((((((((((((((((((------^^^~~~~|~^\n"
"++++^^~^^v^------((((((((((((((^^^((((((((((------------^^^~v~~^\n"
"++++^~^^v^^----------((((((((((^5^((((((((-------.......a~~^~~~^\n"
"++++^^~d^^--------------(((((((^(^(((((---------.------~~^^^^^^^\n"
"+++----~.......-----------((((((((((-----------.------~~--^^^^^^\n"
"++++--~--------.......------((((((((((((------.-------~^^^^^^^^^\n"
"+++++++---------------.......(((((((((-------.--------~~--^^^^^^\n"
"+++++++----------------------......-.........--------~~-----^^^^\n"
"+++++++++--------------((((((((((((b(((((((------^^3~~------^^^^\n"
"++++++++++++---------------(((((((.(((((((------^^^~~-------^^^^\n"
"++++++++++-------((((((((((((((((.(((((((((((((^^^-~~-------^^^^\n"
"+++++++----------------(((((((((.(((((((((---^^^----~~------^^^^\n"
"++++++++++++---------------((((.(((((((((--^^^-----~~---^^^^^^^^\n"
"++++++++++++++++--------------.--------^^^^^------~~---^^^^^^^^^\n"
"+++++++++++++++++++----------.------^^^^---------~~---^^^^^^^^^^\n"
"++++++++++++++++++++++++----.---v^^^^^^^^-((((--~~----------^^^^\n"
"+++++++++++++++++++++++++++O--^^^v^^^((((((((((((~~---^^^^^^^^^^\n"
"+++++++++++++-------------.----^^^v^(((((((((((((~~(----^^^^^^^^\n"
"++++++++++---------------.-^^^^^^(((((((((((((((~~((--------^^^^\n"
"++++++++++++++++++++++...-^*^^^^(((((((((((((((~~((((((-----^^^^\n"
"++++++++++++++++++-...-----^^^-(((((((((((((((~~(((((((((---^^^^\n"
"+++++++++++++-----.------^^^^((((((((((((((((~~(((((((((----^^^^\n"
"++++++++++-------.-----^^^^--((((((((((((((~~~(((((((((-----^^^^\n"
"++++++++++++++++.------^^^-------(((((((((~~(((((((((((((---^^^^\n"
"+++++++++++++++.------^^^---((((((((((((((~~(((((((--------^^^^^\n"
"+++++++++++++++.-----^^^(((((((((((((((~~~~(((((((-------^^^^^^^\n"
"+++++++++++++--.------vvv(((((((((((~~~~(((((((((-------^^^(^^^^\n"
"+++++++++++++++.-----^1^-------((~~~~(((((((((((((((-^^^^^((^^^^\n"
"+++++++++++++++.------^^^-----((~~~((((((((((((((((-^^^(((((^^^^\n"
"++++++++++-----.----^^^------(((((~~(((((((((---^^^^((((((((^^^^\n"
"++++++++-------.----^^^-(((((((((((~((((((((((((--^(((((((^^^^^^\n"
"++++++---------.-----^^^(((((((((((~~((((((((((--^^^(((((((($^^^\n"
"+++++++++++++++.----^^-(((((((((((~~((((((((((--^^^(((((((((^^^^\n"
"+++++++++++++++.-----^^^^((((((((~~(((((((((-------((((((((^^^^^\n"
"++++++++++++---.----^^^(((((((~~~~(((((((-----^^((((((((((((^^^^\n"
"+++++++++++++++.---^^^-(((~~~~~(((((((------^^^(((((((((((((^^^^\n"
"+++++++++++++++.---^^^^~~~~(((((----------^^^(((((((((((((((^^^^\n"
"++++++++++c....-...~~~~~-((((((((((-----^^^^((((((((((((((((^^^^\n"
"++++++++++++++~~~~~.f==-------------^^^^((((((((((((((((^^^^^^^^\n"
"++++++++++++=====~==~======----^^^^^^^^^(((((((((((((((((((^^^^^\n"
"+++++++++++=====~====~====---------------e((((((((((((((^^^^^^^^\n"
"+++++++++++++++~====~====-^^-------^^^^^^(((((((((((((((((((^^^^\n"
"+++++++++++++++====~====-^^------^^^(((((((((2(((((((((((---^^^^\n"
"+++++++++++++=====~4~===---^-----^^(((((((((((---^^^^-------^^^^\n"
"++++++++++=======~==~====---------^^(((((((((--^^-----------^^^^\n"
"+++++++++++++===~====~====--^--------((((((---^^-)))-^^-----^^^^\n"
"++++++++++++===~======~===---^^^^------------^^~)))))--^^---^^^^\n"
"+++++++++++++++======~======-----^^^------^----)~)))))--^^^-^^^^\n"
"+++++++++++++++=====~===^=======--~~~^^^-^---))))~))))))----^^^^\n"
"++++++++++++++=====~=============~~=~~~~^^))))))))~))))))))-^^^^\n"
"+++++++++++++++===~=~~~~~~=====~~~~====^^^^)))))))~)))))))^^^^^^\n"
"++++&++++++++++==~========~~~~~~==~~~~===^^^)))))))~))))))))^^^^\n"
"+++++=+++++++===~=~=========~===~~===~~~~~~^^)))))))~)))))^^^^^^\n"
"++++++=++++++++~===~===^===~===~=====~====^^^^)))))))~))))))^^^^\n"
"+++++++=+++++++====~~=====~===^=======~====^^)))))))))~)))))^^^+\n"
"++++++++=+========~==~===~===^!^=======~===^^^))+++)))~)))^^^^++\n"
"+++++++++=++=====~====~=~~====^=======~=~=^^^)))+6+))~)))^^^^+++\n"
"++++++++++++====~======~==~++++======~===~==^^))+++))~^^^^^+++++\n"
"++++++++++++++++========~++++++++===~====~^^^^^^^+^^^^~^^+++++++\n"
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
//}}}2------------------------------------------------------------------
//{{{2 Court of the Archmage
const char Level_Court[] =
"\100\030\040\002"
"################################################################\n"
"################################.###############################\n"
"################################.###############################\n"
"################################.###############################\n"
"###############################G.G##############################\n"
"###############################...##############################\n"
"###############################...##############################\n"
"##############################G...G#############################\n"
"##############################.....#############################\n"
"##############################.....#############################\n"
"#############################G.....G############################\n"
"#############################.......############################\n"
"#############################.......############################\n"
"############################G.......G###########################\n"
"###########################..e.....e..##########################\n"
"#########################....e.....e....########################\n"
"#########################....e.....e....########################\n"
"#########################.....n...n.....########################\n"
"##########################....n...n....#########################\n"
"###########################.....5.....##########################\n"
"#############################..T.T..############################\n"
"###############################.<.##############################\n"
"################################################################\n"
"################################################################\n"
"################################################################";
//}}}2------------------------------------------------------------------
//{{{2 Dragon lair
const char Level_DragonLair[] =
"\100\020\002\011"
"################################################################\n"
"################################################################\n"
"#############..........M...........#############################\n"
"########$..$..........................#############$$$$$########\n"
"#####.........................M.........##########$$$$$$$$$#####\n"
"####.....M............$.................$######Ss#$$$$d$$$$$####\n"
"###................................###########$#s#$$$d$d$$$$$###\n"
"X..........$.......................SpTT77TTppW$#s.$$d$D$d$$$$$##\n"
"X..........................$.......SpTT77TTppW$#s.$$$d$d$$$$$###\n"
"X......M...........................###########$#s#$$$$d$$$$$####\n"
"#####.................M..............$$$#######Ss#$$$$$$$$######\n"
"########....$..................M.....$$###########$$$$$$$#######\n"
"###########..........................###########################\n"
"################################################################\n"
"################################################################\n"
"################################################################";
//}}}2------------------------------------------------------------------
//{{{2 House
const char Level_House[] =
"\100\020\002\015"
"################################################################\n"
"################################################################\n"
"##cccccccccccccc#DDDDDDDDDDDDDDDDDDD|ccccc##GGGG|BBBBBBBBBBBB###\n"
"##cccccccccccccc#DDDDDDDDDDDDDDDDDDD|ccccc#3GGGG|BBBBBBBBBBBB###\n"
"##cccccccccccccc#DDDDDDDDDDDDDDDDDDD#############BBBBBBBBBBBB###\n"
"##cccccccccccccc#DDDDDDDDDDDDDDDDDDD#.....|BBBBBBBBBBBBBBBBBB###\n"
"##############||#DDDDDDDDNDDDDDDDDDD#.....|BBBBBBBBBBBBBBBBBB###\n"
"##K#K#K#K#K#K#KK|DDDDDDDDDDDDDDDDDDD#.....#BBBBBBBBBBBBBBBBBB###\n"
"##KKKKKKKKKKKKKK|DDDDDDDDDDDDDDDDDDD#.....#BBBBBBBBBBBBBBBBBB###\n"
"###KKKKKKKKKKKKK#DDDDDDDDDDDDDDDDDDDD.....#BBBBBBBBBBBBBBBBBB###\n"
"##3KKKKKKKKKKKKK#DDDDDDDDDDDDDDDDDDDD.....#NBBBBBBBBBBBBBBBBB###\n"
"##############||#######||########################BBBBBBBBBBBB###\n"
"X...................................|cccc##3cccc|BBBBBBBBBBBB###\n"
"X...................................|cccc3##cccc|BBBBBBBBBBBB###\n"
"################################################################\n"
"################################################################";
//}}}2------------------------------------------------------------------
//{{{2 Mansion
const char Level_Mansion[] =
"\100\020\002\010"
"################################################################\n"
"#DDDDDDDDKaKS............S...dppT..ppT..ppT.....ppppppp#########\n"
"#DDDDDDDDKKa#............####S#######################TT###a#####\n"
"#DDDDDDDDKKK#...........a33##^########################+####3333#\n"
"#DDDDDDDDKKK#...........a####B#########################a###3333#\n"
"#DDDDDDDDKKK##############^##B#############################3333#\n"
"####|||###################^##BBBBBBBHBBBBBBBBBBaBBBBBBBBB##3333#\n"
"X............pRPdTTTTTTppTT##BBBBBBBBBBBBBBBBBBBBBBBBBBBB##AAAA#\n"
"X............pRPdTTTTTTppTT##BBBBBBBBBBBBBBBBBBBBBBBBBBBB##SSSS#\n"
"###||#||#####################BBBBBBaBBBBBBBBBBBBBBBBBBBBB##adad#\n"
"#cccc#GGGG#BBBBBBBBBBBBBB####BBNBBBBBBBBBBBBBBBBBBBBBBBBa##SSSS#\n"
"#ccac#GGGG#BBBBBBBBBBBBBB####BBBBBBBBBBBBBBBBBBBBBBBBBBBB##GGGG#\n"
"#cccc#GGaG#BBBBBBBBBBBBBB####BBBBBBBBBBBBBBBBBBBBBBBBBBBB##GGGG#\n"
"#cccc#GGGG|aBBBBBaBBBBBBB####BBBBBBBBBBBBBBBBBaBBBBBBBBBBB|GGGG#\n"
"################################################################";
//}}}2------------------------------------------------------------------
//{{{2 Hovel
const char Level_Hovel[] =
"\100\020\002\011"
"################################################################\n"
"#3##############################################################\n"
"##DDDDDDDDDDDDDDDDDDDDDDDDDDDDDD#BBBBBBBBBBBBBBBBBBBBBBBBBBBBB##\n"
"##DDDDDDDDDDDDDDDDDDDDDDDDDDDDDD#BBBBBBBBBBBBBBBBBBBBBBBBBBBBB##\n"
"##DDDDDDDDDDDDDDDDDDDDDDDDDDDDDD#BBBBNBBBBBBBBBBBBBBBBBBBBBBBB##\n"
"##DDDDDDDDDDDDDDDDDDDDDDDDDDDDDD#BBBBBBBBBBBBBBBBBBBBBBBBBBBBB##\n"
"##DDDDDDDDDDDDDDDDDDDDDDDDDDDDDD#BBBBBBBBBBBBBBBBBBBBBBBBBBBBB##\n"
"##DDDDDDDDDDDDDDDDDDDDDDDDDDDDDD#BBBBBBBBBBBBBBBBBBBBBBBBBBBBB##\n"
"X.DDDDDDDDDDDDDDDDDDDDDDDDDDDDDD.BBBBBBBBBBBBBBBBBBBBBBBBBBBBB##\n"
"X.DDDDDDDDDDDDDDDDDDDDDDDDDDDDDD.BBBBBBBBBBBBBBBBBBBBBBBBBBBBB##\n"
"##DDDDDDDDDDDDDDDDDDDDDDDDDDDDDD#BBBBBBBBBBBBBBBBBBBBBBBBBBBBB##\n"
"##DDDDDDDDDDDDDDDDDDDDDDDDDDDDDD#BBBBBBBBBBBBBBBBBBBBBBBBBBBBB##\n"
"##DDDDDDDDDDDDDDDDDDDDDDDDDDDDDD#BBBBBBBBBBBBBBBBBBBBBBBBBBBBB##\n"
"##DDDDDDDDDDDDDDDDDDDDDDDDDDDDDD#BBBBBBBBBBBBBBBBBBBBBBBBBBBBB##\n"
"##############################################################3#\n"
"################################################################";
//}}}2------------------------------------------------------------------
//{{{2 Rampart maze 1
const char Level_Maze1[] =
"(z(((((((((((((((((((("
"(.(...(.....(.(..(...("
"(...(.(.(-(.(.s(.(((.("
"(((((.(.(s(.(.(.(..(.("
"(.......(((.(((.((...("
"(-(((((.(.....((.(((.("
"(s(.....(.((((((s..(.("
"(((.(.(((.((((((((.(.("
"(...(..s(.((Os.....(.("
"(-(((((((.((((((((((.("
"(s..s((.(.((((((.(.(.("
"(((((>s(-.......(.(s.("
"((((((((((((((((((((((";
//}}}2------------------------------------------------------------------
//{{{2 Rampart maze 2
const char Level_Maze2[] =
"(z(((((((((((((((((((("
"(....................("
"((((((((((((((((((((.("
"(s-.(............s.(.("
"(s(.(.((((((((((((.(.("
"(((.(.(............(.("
"(...(.(-((((((((((.(.("
"(.(.(.(s(..s.......(.("
"(.(.(.(((-(.((((((((.("
"(.(.s.(sss(..........("
"(-((-((((((((((((((((("
"(s>(...............-O("
"((((((((((((((((((((((";
//}}}2------------------------------------------------------------------
//{{{2 Rampart maze 3
const char Level_Maze3[] =
"(z(((((((((((((((((((("
"(....................("
"((.(((-(((.((((-(((.(("
"(...(.s.(...(........("
"((.(((((((.((((((((((("
"(........(...........("
"((-((((.((((((-((((.(("
"(....(...(......(....("
"(-((-(-(-((-(((-((-((-"
"(s(..s.(.(..(...(...(s"
"((((((((.(((((.(((.((("
"(>s-s-s-.-.(s-.s(O.-ss"
"((((((((((((((((((((((";
//}}}2------------------------------------------------------------------
//{{{2 Rampart maze 4
const char Level_Maze4[] =
"(z(((((((((((((((((((("
"(.(...(.....-.(...(ss("
"(...(.(.((((((s((.((-("
"(((((.(.(((((.(.((.(.("
"(.....(.((O((.((.((..("
"(.(((((.((.((-(((.((.("
"(...-...(.(s(s-s(..(.("
"(((.(.(((.(((((((..(.("
"(...(..s(.(.((..-(((.("
"(.(((((((..(..(.(.((.("
"(.(.(.(.(((((((.((.(.("
"(..(.(.(ss(>s-..(.(s.("
"((((((((((((((((((((((";
//}}}2------------------------------------------------------------------
//{{{2 Magic isle
const char Level_MagicIsle[] =
"\100\020\076\016"
"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"~~~~~~~~~~~...........................................~~~~~~~~~~\n"
"~~~~..............4444444444444444444444444444.............~~~~~\n"
"~~~...........44444#########################4444444.........~~~~\n"
"~~~.............##########=####################444..........~~~~\n"
"~~~~..............###m###=#=########=#######4444444...........~~\n"
"~~~~~~~~~~......444###===##=#######=#=#########444444......~~~~~\n"
"~~~~~~~~.......444##m====###=#####=##=###########44.....~~~~~~~~\n"
"~~~~~.........44444##==E==m##=###=###=#########444444......~~~~~\n"
"~~~~...........444##m======###===####==##########4.....~~~~~~~~~\n"
"~~~~~~..........444###===m############===####44....~~~~~~~~~~~~~\n"
"~~~~~~~~~.........444######4444#######44==444...~~~~~~~~~~~~~~~~\n"
"~~~~~~~~~~~~~................444444444....=...............~~~~~~\n"
"~~~~~~~~~~~~~~~~~~~~~~~~~~~................=.................~~~\n"
"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.......~\n"
"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~XXXXXXX";
//}}}2------------------------------------------------------------------
//{{{2 Star peak
const char Level_StarPeak[] =
"\100\020\002\011"
"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n"
"X.....44.4........444..........................................X\n"
"X.44......4.4.444......444#####################4#####......44.4X\n"
"X....###...........######4#...................#44444###...#....X\n"
"X4.##....####444####....###.#################.###########...#..X\n"
"X.##..#.###.44444.#.#......##...............##..........##...#.X\n"
"X.#..##.#M#..####.#.#.######-#############.#..##########.##...#X\n"
"X.....##.###...MMM#.#.#MMMMM#pppTT-s7A-L###.##.44.....##..##..#X\n"
"X...4..###M####MMM4.#.#M#MMM###############...#..######.###..#.X\n"
"X...44...###$$....4.#.#######.........#############...###...#..X\n"
"X.#..444...####.....#..........#######.............####........X\n"
"X.##..44444...###############44444444###############.....444...X\n"
"X..##......4................#######4444..............4...##....X\n"
"X...#############....4444444..........4444444444.####.....#....X\n"
"X..............................................................X\n"
"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
//}}}2------------------------------------------------------------------
//{{{2 Temple
const char Level_Temple[] =
"\100\020\040\017"
"################################################################\n"
"######m....##########################x##x##x##x##x##############\n"
"######H....-..................SS.-.................-x###########\n"
"######m....#####################8####x##x##x##x##x##############\n"
"###############################.m.##############################\n"
"##############################..#..#############################\n"
"###..#######################..m...m..######################..###\n"
"###.x-.....................|.#.#.#.#.|....................-x.###\n"
"###..###-#####-#####-#######....?....######-#####-#####-###..###\n"
"#######.x.###.x.###.x.######.#.#.#.#.#####.x.###.x.###.x.#######\n"
"###..#######################.........######################..###\n"
"###.x-.....................|.#.#-#.#.|....................-x.###\n"
"###..###-#####-#####-#######...#W#...######-#####-#####-###..###\n"
"#######.x.###.x.###.x.##########.#########.x.###.x.###.x.#######\n"
"###############################...##############################\n"
"###############################XXX##############################";
//}}}2------------------------------------------------------------------
//{{{2 Village 1
const char Level_Village1[] =
"\100\020\000\006"
"################################################################\n"
"##.....#x#x#..########.##...###h-.....##.#fff-^G^-fff#.#x##x#.##\n"
"#####..................###...####.###-##.######-######........##\n"
"##GG#..#####..########.####..##h-.-h#HG#......#g#......####...##\n"
"X.GG#..#x#x#..########.#x###.########..#.#####...###...####...##\n"
"X...-........................########SS#.x####..#~~~#.........##\n"
"X...-....................................#####...###..........##\n"
"X.GG#..#x###..##x##x##.################.......................##\n"
"##GG#..####x..########.#...(....(s..(.#..####.###...#####.#.#.##\n"
"#####..x####..########.#.((..((..(....#..x###.#x#...####.#.#.###\n"
"##.....###x#..##x##x##.#.(s(...(..s(.G#.............x##.#.#.#.##\n"
"##.....................#......(..(((..C..##.##.#####.###.#.#.###\n"
"##.#########..########.#.(((......(s.(#..#x.##.##x##x##.#.#.#.##\n"
"##.########x..#####x...#..s(.(s(....(.#.....##.......###.#.#.!##\n"
"################################################################\n"
"################################################################";
//}}}2------------------------------------------------------------------
//{{{2 Village 2
const char Level_Village2[] =
"\100\020\047\017"
"((((((((((((((((((((((((((((((((((((((((((###~~~#(((((((((((((((\n"
"((((((((((((((((((((((((((((((((((((((((((#~~~###(((((((((((((((\n"
"((.....(..(.....(...(.(...(.....((.......###~~~#..............((\n"
"X...(......(.((.........(....(..(...###.##~~~###.#x#.#x#.#x#..((\n"
"((G....(.........(...(.....(........#G###~~~##...x#x.x#x.x#x..((\n"
"((...(......(..(......G.G........((.#.############x#.#x#.#x#..((\n"
"((((((((((....#########S#######(..............................((\n"
"((............##h..####.H..####.....#.############x#.#x#.#x#..((\n"
"((.........(..###..#######.####.(((.#G###~~~#....x#x.x#x.x#x..((\n"
"((..''1''..(..##h..#fffff#.h###.....###~~~###.((.#x#.#x#.#x#..((\n"
"((..'1~1'..(..###..###g###..###...###~~~###..................G((\n"
"((..1~!~1..(..##h...........h##.###~~~###...(....(...(.(...(...X\n"
"((..'1~1'..(..###.#.#.#.#.#~#####~~~###........(.........(....((\n"
"((..''1''..(..##h.#h#h#h#h#~~~#~~~###...G...(....(..(..(....(.((\n"
"((((((((((((((################~~~##((((.((((((((((((((((((((((((\n"
"(((((((((((((((((((((((((((((~~~(((((((X((((((((((((((((((((((((";
//}}}2------------------------------------------------------------------
//{{{2 Village 3
const char Level_Village3[] =
"\100\020\076\010"
"+++++++++++++++++++~~~...#######XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n"
"++++++++++++++++++~~~...#######................................X\n"
"++++++++++++++++~~~~~..#######...#####..#####...#####..#####...X\n"
"+++++++++++++++~~.....#######....####x..####x...####x..####x...X\n"
"++++++++++++++~~~....#######.....##x##..##x##...##x##..##x##...X\n"
"+++++++++++++.......#######....................................X\n"
"++++++~~~+++~......###########.......x##x....#x#....x##x.......X\n"
"+++##++++~+~~~....#######GGGG#.......###x....#x#....x###.......X\n"
"++#!.#++++~~.....-------.......................................X\n"
"+++##+++++~~~....-------.........##x##..##x##...##x##..##x##...X\n"
"+++++++++++~~~....#######GGGG#...####x..####x...####x..####x...X\n"
"++++++++++++~......###########...#####..#####...#####..#####...X\n"
"+++++++++++++~~.....#######......#h.##..#Gff#...#(((#..#~~~#...X\n"
"++++++++++++++~~~....#######.....#h.HS..g.ff#...#(((....~~~#...X\n"
"+++++++++++++++~~~~...#######....#####..#####...#####..#####...X\n"
"++++++++++++++++~~~~~~.#######XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
//}}}2------------------------------------------------------------------
//{{{2 Village 4
const char Level_Village4[] =
"\100\020\040\017"
"XXXXXXX#####~~####XXXXXX##!##XXXXXXXXXXXXXXX##########XXXXXXX###\n"
"X............~~..#####..##.##............####..............##..X\n"
"#...#x#.......~~.x#.x##...........########............x####....X\n"
"######x......~~..##.#x####..######.........#x#......####.......X\n"
"#(s(####....~~.....................##x.....x########...........X\n"
"#.(..(..#####~~.x#x.........#x#....x##...#####.................X\n"
"#..s...(..(#~~######....x#..##########.........................X\n"
"#.(..s..s~~~#~~.....#####x#####................................X\n"
"######~~~...C..~~......................................#x#....##\n"
"X....~#######~~.....x#......................#x#........x#x.###.X\n"
"~~~~~.......##~~....##......#x#.............x##.##########.....X\n"
"X............~~########.....x#x........#########...............X\n"
"X...........~~.............############........................X\n"
"X..........~~..................................................X\n"
"X.........~~...................................................X\n"
"XXXXXXXXX~~XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
//}}}2------------------------------------------------------------------
//{{{2 Village 5
const char Level_Village5[] =
"\100\020\002\010"
"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n"
"X..............################################................X\n"
"X.............##################################...............X\n"
"X............##.....##h-...HS....##.##..##.....##..............X\n"
"X...........##..#x#..########G...x#.x#.##.##.##.##.............X\n"
"X..........##..x###x..##..............##..#x.x#..##............X\n"
"X..........##G..#x#...##.....~~~~.....##........G##............X\n"
"X..........--................~!!~.........x#x#x..--............X\n"
"X..........--................~!!~.........x#x#x..--............X\n"
"X..........##G..#x#...##.....~~~~.....##........G##............X\n"
"X..........##..x###x..##..............##..#x.x#..##............X\n"
"X...........##..#x#..##.#x.#x...G########.##.##.##.............X\n"
"X............##.....##..##.##....g^^^fff##.....##..............X\n"
"X.............##################################...............X\n"
"X..............################################................X\n"
"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
//}}}2------------------------------------------------------------------
//{{{2 Village 6
const char Level_Village6[] =
"\100\020\002\002"
"~~~~~~~~~~~~~~~~~XXXXXXXXXXXXXXXX~XXXXXXXXXXXXXXXXX~~~~~~XX~XXXX\n"
"~~~~~~~~~~~~~~~~~~~~~.....##.((...~.......((((.##.....~~~~..~..X\n"
"X....(.....##....~~~~~~~~~##x..x#..~~~~~...((..#x.~~~~~~~....~.X\n"
"X.###...((.#x........~~~~~~~~~.##.~..##x~....~~~~~~~~~......~..X\n"
"X.x#x........((..##.~.....~~~~~~~~~~~~##~~~~~~~~~~....((((.~...X\n"
"~~~~~~~~~........x#..~..##....~~~~~~~~~~~~~......##.(((...~..((X\n"
"X.##..~~~~~~~~~.....~...#x.((........~~~~~~~~~~..x#..((((..~..((\n"
"X.x#......~~~~~~~~~~.......((.##.(...##..~..~~~~~~~~~...##..~.((\n"
"X..(.##........~~~~~~~~~......x#.#x..x#.~......~~~~~~~~.#x.~..((\n"
"X##..x#..#x#..~..#x.~~~~~~~~~....##....~...((((..~~~~~~~..~..(((\n"
"X#x....(.#x#.~...##.....~~~~~~~~......~...((((((...~~~~~~~.....X\n"
"X..(........~.~.(((((.~~~~~~....x#.(...~.....((((....~~~~~~~~..X\n"
"X.....(....~...~(.((~~~~~~.....(##.##...~..((((((.........~~~~~~\n"
"X.(.......~.((((~(((((~~~~~~~......#x..~......((((....#x#......X\n"
"X....(...~..((((.~..(!(((~~~~~~~~~......~......((((((.#x#..((((X\n"
"XXXXXXXX~XX((XXXXX~XXXX((((XX~~~~~~~~~~~~XXX(((((((((XXXXXXXXXXX";
//}}}2------------------------------------------------------------------
//}}}-------------------------------------------------------------------
