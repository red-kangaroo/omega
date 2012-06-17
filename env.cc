#include "glob.h"
#include <unistd.h>

//----------------------------------------------------------------------

static monster& make_prime(int i, int j);
static monster& make_archmage(int i, int j);
static void assign_city_function(int x, int y);
static void make_justiciar(int i, int j);
static void mazesite(int i, int j, int populate);
static void randommazesite(int i, int j, int populate);
static void make_minor_undead(int i, int j);
static void make_major_undead(int i, int j);
static void random_temple_site(int i, int j, int deity, int populate);
static void make_high_priest(int i, int j, int deity);
static void make_house_npc(int i, int j);
static void make_mansion_npc(int i, int j);
static void assign_village_function(int x, int y, int setup);
static void make_food_bin(int i, int j);
static void special_village_site(int i, int j, int villagenum);
static void buyfromstock(int base, int numitems);
static void cureforpay(void);
static void gymtrain(uint8_t* maxstat, uint8_t* stat);
static void healforpay(void);
static void wake_statue(int x, int y, int first);
static void make_site_treasure(int i, int j, int itemlevel);
static void make_specific_treasure(int i, int j, int iid);
static void repair_jail(void);
static void m_create (monster& m, int x, int y, int kind, unsigned level);
static void make_creature (monster& m, int mid);
static void determine_npc_behavior (monster& npc, int level, int behavior);
static void make_log_npc (monster& npc);

//----------------------------------------------------------------------

level::level (void)
: _site (MAXWIDTH*MAXLENGTH)
, mlist()
, things()
, next(NULL)
, environment(0)
, last_visited(0)
, depth(0)
, generated(0)
, numrooms(0)
, tunnelled(0)
{
}

// erase the level w/o deallocating it
void level::clear (void)
{
    generated = false;
    numrooms = 0;
    tunnelled = 0;
    depth = 0;
    mlist.clear();
    next = NULL;
    last_visited = time(NULL);
    fill (_site, (location){ WALL, SPACE, (uint8_t) min(UINT8_MAX,20u*difficulty()), L_NO_OP, 0, RS_WALLSPACE });
}

monster* level::creature (int x, int y)
{
    foreach (m, mlist)
	if (m->x == x && m->y == y)
	    return (m);
    return (NULL);
}

object* level::thing (int x, int y)
{
    foreach (i, things)
	if (i->x == x && i->y == y)
	    return (i);
    return (NULL);
}

void level::make_thing (int x, int y, unsigned tid, unsigned n)
{
    things.emplace_back (x, y, tid, n);
}

void level::remove_things (int x, int y)
{
    foreach (i, things)
	if (i->x == x && i->y == y)
	    --(i = things.erase(i));
}

void level::add_thing (int x, int y, const object& o, unsigned n)
{
    object* no = things.insert (things.end(), o);
    no->x = x;
    no->y = y;
    no->used = false;
    if (n != (unsigned) RANDOM)
	no->number = n;
}

//----------------------------------------------------------------------

// loads the arena level into Level
void load_arena (void)
{
    const char* ld = Level->init_from_data (E_ARENA, Level_Arena);
    for (unsigned j = 0; j < Level->height; ++j, ++ld) {
	for (unsigned i = 0; i < Level->width; i++) {
	    Level->site(i,j).lstatus = SEEN + LIT;
	    Level->site(i,j).roomnumber = RS_ARENA;
	    char site = *ld++;
	    Level->site(i,j).p_locf = L_NO_OP;
	    switch (site) {
		case 'P':
		    Level->site(i,j).locchar = PORTCULLIS;
		    Level->site(i,j).p_locf = L_PORTCULLIS;
		    break;
		case 'X':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_ARENA_EXIT;
		    break;
		case '#':
		    Level->site(i,j).locchar = WALL;
		    break;
		case '.':
		    Level->site(i,j).locchar = FLOOR;
		    break;
	    }
	    Level->site(i,j).showchar = Level->site(i,j).locchar;
	}
    }

    static const uint8_t _opponents[] = {
	GEEK, HORNET, HYENA, GOBLIN, GRUNT, TOVE, APPR_NINJA, SALAMANDER, ANT, MANTICORE,
	SPECTRE, BANDERSNATCH, LICHE, AUTO_MAJOR, JABBERWOCK, JOTUN, HISCORE_NPC
    };
    monster& m = make_site_monster (60, 7, _opponents[min((unsigned)Arena_Opponent,ArraySize(_opponents))]);
    Arena_Victory = false;

    if (m.id == HISCORE_NPC) {
	if (Player.rank[ARENA] < CHAMPION && Player.rank[ARENA] >= TRAINEE) {
	    make_hiscore_npc (m, NPC_CHAMPION);
	    strcpy (Str1, Champion);
	    strcat (Str1, ", the arena champion");
	    m.monstring = strdup (Str1);
	    strcpy (Str2, "The corpse of ");
	    strcat (Str2, Str1);
	    m.corpsestr = strdup (Str2);
	    m.level = 20;
	    m.hp = Championlevel * Championlevel * 5;
	    m.hit = Championlevel * 4;
	    m.ac = Championlevel * 3;
	    m.dmg = 100 + Championlevel * 2;
	    m.xpv = Championlevel * Championlevel * 5;
	    m.speed = 3;
	    m.meleestr = m_melee_str(Championlevel/5);
	    m_status_set (m, MOBILE);
	    m_status_set (m, HOSTILE);
	} else {
	    int i;
	    do
		i = random_range (ML9 - ML0) + ML0;
	    while (i == NPC || i == HISCORE_NPC || i == ZERO_NPC || Monsters[i].uniqueness != COMMON || !Monsters[i].dmg);
	    m = Monsters[i];
	}
    }
    if (m.level != 20) {
	strcpy (Str1, nameprint());
	strcat (Str1, " the ");
	strcat (Str1, m.monstring);
	m.monstring = strdup (Str1);
	strcpy (Str2, "The corpse of ");
	strcat (Str2, Str1);
	m.corpsestr = strdup (Str2);
    }
    m.uniqueness = UNIQUE_MADE;
    m.attacked = true;
    m_status_set (m, HOSTILE);
    m.sense = 50;
    m.pickup (Objects[THING_DOOR_OPENER]);
    m_status_set (m, AWAKE);
    m.hp += m.level * 10;
    m.hit += m.hit;
    m.dmg += m.dmg / 2;

    // hehehehe cackled the dungeon master....
    print1 ("You have a challenger: ");
    print2 (m.monstring);
    morewait();
    print2 ("Your opponent holds the only way you can leave!");
}

// loads the sorcereror's circle into Level
void load_circle (int populate)
{
    int safe = (Player.rank[CIRCLE] >= INITIATE);
    const char* ld = Level->init_from_data (E_CIRCLE, Level_Circle);
    for (unsigned j = 0; j < Level->height; ++j, ++ld) {
	for (unsigned i = 0; i < Level->width; ++i) {
	    Level->site(i,j).lstatus = 0;
	    Level->site(i,j).roomnumber = RS_CIRCLE;
	    Level->site(i,j).p_locf = L_NO_OP;
	    char site = *ld++;
	    switch (site) {
		case 'P':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate) {
			monster& m = make_prime (i, j);
			m.specialf = M_SP_PRIME;
			if (!safe)
			    m_status_set (m, HOSTILE);
		    }
		    break;
		case 'D':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate) {
			monster& m = make_site_monster (i, j, DEMON_PRINCE);
			if (safe)
			    m_status_reset (m, HOSTILE);
			m.specialf = M_SP_COURT;
		    }
		    break;
		case 's':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate) {
			monster& m = make_site_monster (i, j, SERV_CHAOS);
			m.specialf = M_SP_COURT;
			if (safe)
			    m_status_reset (m, HOSTILE);
		    }
		    break;
		case 'e':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate) {
			monster& m = make_site_monster (i, j, ENCHANTOR);
			m.specialf = M_SP_COURT;
			if (safe)
			    m_status_reset (m, HOSTILE);
		    }
		    break;
		case 'n':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate) {
			monster& m = make_site_monster (i, j, NECROMANCER);
			m.specialf = M_SP_COURT;
			if (safe)
			    m_status_reset (m, HOSTILE);
		    }
		    break;
		case 'T':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate) {
			monster& m = make_site_monster (i, j, THAUMATURGIST);
			m.specialf = M_SP_COURT;
			if (safe)
			    m_status_reset (m, HOSTILE);
		    }
		    break;
		case '#':
		    Level->site(i,j).locchar = WALL;
		    Level->site(i,j).aux = 255;
		    break;
		case 'L':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_CIRCLE_LIBRARY;
		    break;
		case '?':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_TOME1;
		    break;
		case '!':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_TOME2;
		    break;
		case 'S':
		    Level->site(i,j).locchar = FLOOR;
		    lset (i, j, SECRET);
		    break;
		case '.':
		    Level->site(i,j).locchar = FLOOR;
		    break;
		case '-':
		    Level->site(i,j).locchar = CLOSED_DOOR;
		    break;
	    }
	}
    }
}

// make the prime sorceror
static monster& make_prime (int i, int j)
{
    monster& m = make_site_monster (i, j, NPC);
    make_hiscore_npc (m, NPC_PRIME);
    if (object_uniqueness(STAR_GEM) != UNIQUE_TAKEN)
	m.possessions.emplace_back (Objects[STAR_GEM]);
    return (m);
}

// loads the court of the archmage into Level
void load_court (int populate)
{
    const char* ld = Level->init_from_data (E_COURT, Level_Court);
    LastCountryLocX = *ld++;
    LastCountryLocY = *ld++;
    for (unsigned j = 0; j < Level->height; ++j, ++ld) {
	for (unsigned i = 0; i < Level->width; ++i) {
	    Level->site(i,j).lstatus = 0;
	    Level->site(i,j).roomnumber = RS_COURT;
	    Level->site(i,j).p_locf = L_NO_OP;
	    char site = *ld++;
	    switch (site) {
		case '5':
		    Level->site(i,j).locchar = CHAIR;
		    Level->site(i,j).p_locf = L_THRONE;
		    if (populate) {
			make_specific_treasure (i, j, SCEPTRE_OF_HIGH_MAGIC);
			monster& m = make_archmage (i, j);
			m_status_reset (m, HOSTILE);
			m_status_reset (m, MOBILE);
		    }
		    break;
		case 'e':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate) {
			monster& m = make_site_monster (i, j, ENCHANTOR);
			m_status_reset (m, HOSTILE);
			m.specialf = M_SP_COURT;
		    }
		    break;
		case 'n':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate) {
			monster& m = make_site_monster (i, j, NECROMANCER);
			m_status_reset (m, HOSTILE);
			m.specialf = M_SP_COURT;
		    }
		    break;
		case 'T':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate) {
			monster& m = make_site_monster (i, j, THAUMATURGIST);
			m_status_reset (m, HOSTILE);
			m.specialf = M_SP_COURT;
		    }
		    break;
		case '#':
		    Level->site(i,j).locchar = WALL;
		    Level->site(i,j).aux = 255;
		    break;
		case 'G':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).locchar = FLOOR;
		    if (populate) {
			monster& m = make_site_monster (i, j, GUARD);
			m_status_reset (m, HOSTILE);
		    }
		    break;
		case '<':
		    Level->site(i,j).locchar = STAIRS_UP;
		    Level->site(i,j).p_locf = L_ESCALATOR;
		    break;
		case '.':
		    Level->site(i,j).locchar = FLOOR;
		    break;
	    }
	}
    }
}

// make the archmage
static monster& make_archmage (int i, int j)
{
    monster& m = make_site_monster (i, j, NPC);
    make_hiscore_npc (m, NPC_ARCHMAGE);
    m.specialf = M_SP_COURT;
    return (m);
}

// loads the abyss level into Level
void load_abyss (void)
{
    const char* ld = Level->init_from_data (E_ABYSS, Level_Abyss);
    for (unsigned j = 0; j < Level->height; ++j, ++ld) {
	for (unsigned i = 0; i < Level->width; ++i) {
	    char site = *ld++;
	    Level->site(i,j).roomnumber = RS_ADEPT;
	    switch (site) {
		case '0':
		    Level->site(i,j).locchar = VOID_CHAR;
		    Level->site(i,j).p_locf = L_VOID;
		    break;
		case 'V':
		    Level->site(i,j).locchar = VOID_CHAR;
		    Level->site(i,j).p_locf = L_VOID_STATION;
		    break;
		case '1':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_VOICE1;
		    break;
		case '2':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_VOICE2;
		    break;
		case '3':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_VOICE3;
		    break;
		case '~':
		    Level->site(i,j).locchar = WATER;
		    Level->site(i,j).p_locf = L_WATER_STATION;
		    break;
		case ';':
		    Level->site(i,j).locchar = FIRE;
		    Level->site(i,j).p_locf = L_FIRE_STATION;
		    break;
		case '(':
		    Level->site(i,j).locchar = HEDGE;
		    Level->site(i,j).p_locf = L_EARTH_STATION;
		    break;
		case '6':
		    Level->site(i,j).locchar = WHIRLWIND;
		    Level->site(i,j).p_locf = L_AIR_STATION;
		    break;
		case '#':
		    Level->site(i,j).locchar = WALL;
		    break;
		case '.':
		    Level->site(i,j).locchar = FLOOR;
		    break;
	    }
	}
    }
}

// loads the city level
void load_city (int populate)
{
    initrand (E_CITY, 0);
    Level->depth = 0;
    const char* ld = Level->init_from_data (E_CITY, Level_City);
    for (unsigned j = 0; j < Level->height; ++j, ++ld) {
	for (unsigned i = 0; i < Level->width; ++i) {
	    lset (i, j, SEEN);
	    char site = *ld++;
	    switch (site) {
		case 'g':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_GARDEN;
		    break;
		case 'y':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_CEMETARY;
		    break;
		case 'x':
		    assign_city_function (i, j);
		    break;
		case 't':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_TEMPLE;
		    CitySiteList[L_TEMPLE - CITYSITEBASE][0] = true;
		    CitySiteList[L_TEMPLE - CITYSITEBASE][1] = i;
		    CitySiteList[L_TEMPLE - CITYSITEBASE][2] = j;
		    break;
		case 'T':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_PORTCULLIS_TRAP;
		    Level->site(i,j).aux = NOCITYMOVE;
		    break;
		case 'R':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_RAISE_PORTCULLIS;
		    Level->site(i,j).aux = NOCITYMOVE;
		    break;
		case '7':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_PORTCULLIS;
		    Level->site(i,j).aux = NOCITYMOVE;
		    break;
		case 'C':
		    Level->site(i,j).locchar = OPEN_DOOR;
		    Level->site(i,j).p_locf = L_COLLEGE;
		    CitySiteList[L_COLLEGE - CITYSITEBASE][0] = true;
		    CitySiteList[L_COLLEGE - CITYSITEBASE][1] = i;
		    CitySiteList[L_COLLEGE - CITYSITEBASE][2] = j;
		    break;
		case 's':
		    Level->site(i,j).locchar = OPEN_DOOR;
		    Level->site(i,j).p_locf = L_SORCERORS;
		    CitySiteList[L_SORCERORS - CITYSITEBASE][0] = true;
		    CitySiteList[L_SORCERORS - CITYSITEBASE][1] = i;
		    CitySiteList[L_SORCERORS - CITYSITEBASE][2] = j;
		    break;
		case 'M':
		    Level->site(i,j).locchar = OPEN_DOOR;
		    Level->site(i,j).p_locf = L_MERC_GUILD;
		    CitySiteList[L_MERC_GUILD - CITYSITEBASE][0] = true;
		    CitySiteList[L_MERC_GUILD - CITYSITEBASE][1] = i;
		    CitySiteList[L_MERC_GUILD - CITYSITEBASE][2] = j;
		    break;
		case 'c':
		    Level->site(i,j).locchar = OPEN_DOOR;
		    Level->site(i,j).p_locf = L_CASTLE;
		    CitySiteList[L_CASTLE - CITYSITEBASE][0] = true;
		    CitySiteList[L_CASTLE - CITYSITEBASE][1] = i;
		    CitySiteList[L_CASTLE - CITYSITEBASE][2] = j;
		    break;
		case 'm':
		    mazesite (i, j, populate);
		    break;
		case 'P':
		    Level->site(i,j).locchar = OPEN_DOOR;
		    Level->site(i,j).p_locf = L_ORDER;
		    CitySiteList[L_ORDER - CITYSITEBASE][0] = true;
		    CitySiteList[L_ORDER - CITYSITEBASE][1] = i;
		    CitySiteList[L_ORDER - CITYSITEBASE][2] = j;
		    break;
		case 'H':
		    Level->site(i,j).locchar = OPEN_DOOR;
		    Level->site(i,j).p_locf = L_CHARITY;
		    CitySiteList[L_CHARITY - CITYSITEBASE][0] = true;
		    CitySiteList[L_CHARITY - CITYSITEBASE][1] = i;
		    CitySiteList[L_CHARITY - CITYSITEBASE][2] = j;
		    break;
		case 'j':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate)
			make_justiciar (i, j);
		    break;
		case 'J':
		    Level->site(i,j).locchar = CLOSED_DOOR;
		    Level->site(i,j).p_locf = L_JAIL;
		    break;
		case 'A':
		    Level->site(i,j).locchar = OPEN_DOOR;
		    Level->site(i,j).p_locf = L_ARENA;
		    CitySiteList[L_ARENA - CITYSITEBASE][0] = true;
		    CitySiteList[L_ARENA - CITYSITEBASE][1] = i;
		    CitySiteList[L_ARENA - CITYSITEBASE][2] = j;
		    break;
		case 'B':
		    Level->site(i,j).locchar = OPEN_DOOR;
		    Level->site(i,j).p_locf = L_BANK;
		    CitySiteList[L_BANK - CITYSITEBASE][0] = true;
		    CitySiteList[L_BANK - CITYSITEBASE][1] = i;
		    CitySiteList[L_BANK - CITYSITEBASE][2] = j;
		    lset (i, j + 1, STOPS);
		    lset (i + 1, j, STOPS);
		    lset (i - 1, j, STOPS);
		    lset (i, j - 1, STOPS);
		    break;
		case 'X':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_COUNTRYSIDE;
		    CitySiteList[L_COUNTRYSIDE - CITYSITEBASE][0] = true;
		    CitySiteList[L_COUNTRYSIDE - CITYSITEBASE][1] = i;
		    CitySiteList[L_COUNTRYSIDE - CITYSITEBASE][2] = j;
		    break;
		case 'v':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_VAULT;
		    Level->site(i,j).aux = NOCITYMOVE;
		    lset (i, j, SECRET);
		    break;
		case 'S':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).aux = NOCITYMOVE;
		    lset (i, j, SECRET);
		    break;
		case 'G':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate) {
			monster& m = make_site_monster (i, j, GUARD);
			m.aux1 = i;
			m.aux2 = j;
		    }
		    break;
		case 'u':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate)
			make_minor_undead (i, j);
		    break;
		case 'U':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate)
			make_major_undead (i, j);
		    break;
		case 'V':
		    Level->site(i,j).showchar = WALL;
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_VAULT;
		    if (populate)
			make_site_treasure (i, j, 5);
		    Level->site(i,j).aux = NOCITYMOVE;
		    lset (i, j, SECRET);
		    break;
		case '%':
		    Level->site(i,j).showchar = WALL;
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_TRAP_SIREN;
		    if (populate)
			make_site_treasure (i, j, 5);
		    Level->site(i,j).aux = NOCITYMOVE;
		    lset (i, j, SECRET);
		    break;
		case '$':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate)
			make_site_treasure (i, j, 5);
		    break;
		case '2':
		    Level->site(i,j).locchar = ALTAR;
		    Level->site(i,j).p_locf = L_ALTAR;
		    Level->site(i,j).aux = ODIN;
		    break;
		case '3':
		    Level->site(i,j).locchar = ALTAR;
		    Level->site(i,j).p_locf = L_ALTAR;
		    Level->site(i,j).aux = SET;
		    break;
		case '4':
		    Level->site(i,j).locchar = ALTAR;
		    Level->site(i,j).p_locf = L_ALTAR;
		    Level->site(i,j).aux = ATHENA;
		    break;
		case '5':
		    Level->site(i,j).locchar = ALTAR;
		    Level->site(i,j).p_locf = L_ALTAR;
		    Level->site(i,j).aux = HECATE;
		    break;
		case '6':
		    Level->site(i,j).locchar = ALTAR;
		    Level->site(i,j).p_locf = L_ALTAR;
		    Level->site(i,j).aux = DESTINY;
		    break;
		case '^':
		    Level->site(i,j).showchar = WALL;
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = TRAP_BASE + random_range (NUMTRAPS);
		    lset (i, j, SECRET);
		    break;
		case '(':
		    Level->site(i,j).locchar = HEDGE;
		    break;
		case '~':
		    Level->site(i,j).locchar = WATER;
		    Level->site(i,j).p_locf = L_WATER;
		    break;
		case '=':
		    Level->site(i,j).locchar = WATER;
		    Level->site(i,j).p_locf = L_MAGIC_POOL;
		    break;
		case '*':
		    Level->site(i,j).locchar = WALL;
		    Level->site(i,j).aux = 10;
		    break;
		case '#':
		    Level->site(i,j).locchar = WALL;
		    Level->site(i,j).aux = 255;
		    break;
		case '.':
		    Level->site(i,j).locchar = FLOOR;
		    break;
		case ',':
		    Level->site(i,j).showchar = WALL;
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).aux = NOCITYMOVE;
		    lset (i, j, SECRET);
		    break;
		case '-':
		    Level->site(i,j).locchar = CLOSED_DOOR;
		    break;
		case '1':
		    Level->site(i,j).locchar = STATUE;
		    break;
		default:
		    printf ("\nOops... missed a case: '%c'   \n", site);
		    morewait();
	    }

	    if (loc_statusp (i, j, SEEN)) {
		if (loc_statusp (i, j, SECRET))
		    Level->site(i,j).showchar = WALL;
		else
		    Level->site(i,j).showchar = Level->site(i,j).locchar;
	    }
	}
    }
    City = Level;

    // make all city monsters asleep, and shorten their wakeup range to 2
    // to prevent players from being molested by vicious monsters on the streets
    foreach (m, Level->mlist) {
	m_status_reset (*m, AWAKE);
	m->wakeup = 2;
    }
    initrand (E_RESTORE, 0);
}

static void assign_city_function (int x, int y)
{
    static int setup = 0;
    static int next = 0;
    static int permutation[64];	// number of x's in city map
    int i, j, k, l;

    Level->site(x,y).aux = true;

    lset (x, y + 1, STOPS);
    lset (x + 1, y, STOPS);
    lset (x - 1, y, STOPS);
    lset (x, y - 1, STOPS);
    lset (x, y, STOPS);

    if (setup == 0) {
	setup = 1;
	for (i = 0; i < 64; i++)
	    permutation[i] = i;
	for (i = 0; i < 500; i++) {
	    j = random_range (64);
	    k = random_range (64);
	    l = permutation[j];
	    permutation[j] = permutation[k];
	    permutation[k] = l;
	}
    }
    if (next > 63) {		// in case someone changes the no. of x's
	Level->site(x,y).locchar = CLOSED_DOOR;
	Level->site(x,y).p_locf = L_HOUSE;
	if (random_range (5))
	    Level->site(x,y).aux = LOCKED;
    } else
	switch (permutation[next]) {
	    case 0:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_ARMORER;
		CitySiteList[L_ARMORER - CITYSITEBASE][1] = x;
		CitySiteList[L_ARMORER - CITYSITEBASE][2] = y;
		break;
	    case 1:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_CLUB;
		CitySiteList[L_CLUB - CITYSITEBASE][1] = x;
		CitySiteList[L_CLUB - CITYSITEBASE][2] = y;
		break;
	    case 2:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_GYM;
		CitySiteList[L_GYM - CITYSITEBASE][1] = x;
		CitySiteList[L_GYM - CITYSITEBASE][2] = y;
		break;
	    case 3:
		Level->site(x,y).locchar = CLOSED_DOOR;
		Level->site(x,y).p_locf = L_THIEVES_GUILD;
		CitySiteList[L_THIEVES_GUILD - CITYSITEBASE][1] = x;
		CitySiteList[L_THIEVES_GUILD - CITYSITEBASE][2] = y;
		break;
	    case 4:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_HEALER;
		CitySiteList[L_HEALER - CITYSITEBASE][1] = x;
		CitySiteList[L_HEALER - CITYSITEBASE][2] = y;
		break;
	    case 5:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_CASINO;
		CitySiteList[L_CASINO - CITYSITEBASE][1] = x;
		CitySiteList[L_CASINO - CITYSITEBASE][2] = y;
		break;
	    case 7:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_DINER;
		CitySiteList[L_DINER - CITYSITEBASE][1] = x;
		CitySiteList[L_DINER - CITYSITEBASE][2] = y;
		break;
	    case 8:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_CRAP;
		CitySiteList[L_CRAP - CITYSITEBASE][1] = x;
		CitySiteList[L_CRAP - CITYSITEBASE][2] = y;
		break;
	    case 6:
	    case 9:
	    case 20:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_COMMANDANT;
		CitySiteList[L_COMMANDANT - CITYSITEBASE][1] = x;
		CitySiteList[L_COMMANDANT - CITYSITEBASE][2] = y;
		break;
	    case 21:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_TAVERN;
		CitySiteList[L_TAVERN - CITYSITEBASE][1] = x;
		CitySiteList[L_TAVERN - CITYSITEBASE][2] = y;
		break;
	    case 10:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_ALCHEMIST;
		CitySiteList[L_ALCHEMIST - CITYSITEBASE][1] = x;
		CitySiteList[L_ALCHEMIST - CITYSITEBASE][2] = y;
		break;
	    case 11:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_DPW;
		CitySiteList[L_DPW - CITYSITEBASE][1] = x;
		CitySiteList[L_DPW - CITYSITEBASE][2] = y;
		break;
	    case 12:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_LIBRARY;
		CitySiteList[L_LIBRARY - CITYSITEBASE][1] = x;
		CitySiteList[L_LIBRARY - CITYSITEBASE][2] = y;
		break;
	    case 13:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_PAWN_SHOP;
		CitySiteList[L_PAWN_SHOP - CITYSITEBASE][1] = x;
		CitySiteList[L_PAWN_SHOP - CITYSITEBASE][2] = y;
		break;
	    case 14:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_CONDO;
		CitySiteList[L_CONDO - CITYSITEBASE][1] = x;
		CitySiteList[L_CONDO - CITYSITEBASE][2] = y;
		break;
	    case 15:
		Level->site(x,y).locchar = CLOSED_DOOR;
		Level->site(x,y).p_locf = L_BROTHEL;
		CitySiteList[L_BROTHEL - CITYSITEBASE][1] = x;
		CitySiteList[L_BROTHEL - CITYSITEBASE][2] = y;
		break;
	    default:
		Level->site(x,y).locchar = CLOSED_DOOR;
		switch (random_range (6)) {
		    case 0:
			Level->site(x,y).p_locf = L_HOVEL;
			break;
		    case 1:
		    case 2:
		    case 3:
		    case 4:
			Level->site(x,y).p_locf = L_HOUSE;
			break;
		    case 5:
			Level->site(x,y).p_locf = L_MANSION;
			break;
		}
		if (random_range (5))
		    Level->site(x,y).aux = LOCKED;
		break;
	}
    next++;
}

// makes a hiscore npc for mansions
static void make_justiciar (int i, int j)
{
    monster& m = make_site_monster (i, j, NPC);
    make_hiscore_npc (m, NPC_JUSTICIAR);
    m.click = (Tick + 1) % 60;
    m_status_reset (m, AWAKE);
}

// loads the city level
void resurrect_guards (void)
{
    const char* ld = Level_City;
    for (unsigned j = 0; j < Level->height; ++j, ++ld) {
	for (unsigned i = 0; i < Level->width; ++i) {
	    if (*ld++ == 'G') {
		monster& m = make_site_monster (i, j, GUARD);
		m.monstring = "undead guardsman";
		m.meleef = M_MELEE_SPIRIT;
		m.movef = M_MOVE_SPIRIT;
		m.strikef = M_STRIKE_MISSILE;
		m.immunity = EVERYTHING - pow2 (NORMAL_DAMAGE);
		m.hp *= 2;
		m.hit *= 2;
		m.dmg *= 2;
		m.ac *= 2;
		m_status_set (m, HOSTILE);
		m_status_set (m, AWAKE);
	    }
	}
    }
}

static void mazesite (int i, int j, int populate)
{
    static const char* _mazes[] = { Level_Maze1, Level_Maze2, Level_Maze3, Level_Maze4 };
    static const char* ld = NULL;
    static int k = 0;
    if (!ld)
	ld = _mazes[random_range(ArraySize(_mazes))];
    char site = *ld++;
    if (++k >= 286) {
	k = 0;
	ld = NULL;
    }
    switch (site) {
	case '(':
	    Level->site(i,j).locchar = HEDGE;
	    if (random_range (10))
		Level->site(i,j).p_locf = L_HEDGE;
	    else
		Level->site(i,j).p_locf = L_TRIFID;
	    break;
	case '-':
	    Level->site(i,j).locchar = CLOSED_DOOR;
	    break;
	case '.':
	    Level->site(i,j).locchar = FLOOR;
	    break;
	case '>':
	    Level->site(i,j).locchar = STAIRS_DOWN;
	    Level->site(i,j).p_locf = L_SEWER;
	    break;
	case 'z':
	    Level->site(i,j).locchar = FLOOR;
	    Level->site(i,j).p_locf = L_MAZE;
	    break;
	case 'O':
	    Level->site(i,j).locchar = OPEN_DOOR;
	    Level->site(i,j).p_locf = L_ORACLE;
	    CitySiteList[L_ORACLE - CITYSITEBASE][1] = i;
	    CitySiteList[L_ORACLE - CITYSITEBASE][2] = j;
	    break;
	case 's':
	    randommazesite (i, j, populate);
	    break;
    }
    lreset (i, j, SEEN);
}

static void randommazesite (int i, int j, int populate)
{
    switch (random_range (7)) {
	case 0:
	case 1:
	    Level->site(i,j).locchar = FLOOR;
	    Level->site(i,j).p_locf = TRAP_BASE + random_range (NUMTRAPS);
	    break;
	case 2:
	case 3:
	    Level->site(i,j).locchar = FLOOR;
	    if (populate)
		make_site_monster (i, j, RANDOM);
	    break;
	case 4:
	case 5:
	    Level->site(i,j).locchar = FLOOR;
	    if (populate)
		make_site_treasure (i, j, 5);
	    break;
	default:
	    Level->site(i,j).locchar = FLOOR;
    }
}

// undead are not hostile unless disturbed....
static void make_minor_undead (int i, int j)
{
    monster& m = make_site_monster (i, j, random_range(2) ? GHOST : HAUNT);
    m_status_reset (m, AWAKE);
    m_status_reset (m, HOSTILE);
}

// undead are not hostile unless disturbed....
static void make_major_undead (int i, int j)
{
    monster& m = make_site_monster (i, j, random_range(2) ? LICHE : VAMP_LORD);
    m_status_reset (m, AWAKE);
    m_status_reset (m, HOSTILE);
}

// fixes up the jail in case it has been munged by player action
static void repair_jail (void)
{
    static const char jail[5][12] = {
	"##*##*##*##",
	"##*##*#*###",
	"##T#T#T#T##",
	"##7#7#7#7##",
	"##R#R#R#R##"
    };
    for (unsigned i = 0; i < ArraySize(jail); i++) {
	for (unsigned j = 0; j < ArraySize(jail[i]); j++) {
	    chtype locchar = FLOOR;
	    char p_locf = L_NO_OP;
	    int aux = NOCITYMOVE;
	    if (jail[j][i] == '*')
		aux = 10;
	    if (jail[j][i] == '#' || jail[j][i] == '*')
		locchar = WALL;
	    if (jail[j][i] == 'T')
		p_locf = L_PORTCULLIS_TRAP;
	    if (jail[j][i] == '7')
		p_locf = L_PORTCULLIS;
	    if (jail[j][i] == 'R')
		p_locf = L_RAISE_PORTCULLIS;
	    City->site(i + 35,j + 52).locchar = locchar;
	    City->site(i + 35,j + 52).p_locf = p_locf;
	    City->site(i + 35,j + 52).aux = aux;
	    lreset (i + 35, j + 52, CHANGED);
	}
    }
}

// loads the countryside level from the data file
void load_country (void)
{
    const char* ld = Level->init_from_data (E_COUNTRYSIDE, Level_Country);
    for (unsigned j = 0; j < Level->height; ++j, ++ld) {
	for (unsigned i = 0; i < Level->width; ++i) {
	    char site = *ld++;
	    Level->site(i,j).aux = 0;
	    Level->site(i,j).lstatus = 0;
	    switch (site) {
		case (PASS & 0xff):
		    Level->site(i,j).locchar = PASS;
		    Level->site(i,j).showchar = MOUNTAINS;
		    break;
		case (CASTLE & 0xff):
		    Level->site(i,j).locchar = CASTLE;
		    Level->site(i,j).showchar = MOUNTAINS;
		    break;
		case (STARPEAK & 0xff):
		    Level->site(i,j).locchar = STARPEAK;
		    Level->site(i,j).showchar = MOUNTAINS;
		    break;
		case (CAVES & 0xff):
		    Level->site(i,j).locchar = CAVES;
		    Level->site(i,j).showchar = MOUNTAINS;
		    break;
		case (VOLCANO & 0xff):
		    Level->site(i,j).locchar = VOLCANO;
		    Level->site(i,j).showchar = MOUNTAINS;
		    break;
		case (DRAGONLAIR & 0xff):
		    Level->site(i,j).locchar = DRAGONLAIR;
		    Level->site(i,j).showchar = DESERT;
		    break;
		case (MAGIC_ISLE & 0xff):
		    Level->site(i,j).locchar = MAGIC_ISLE;
		    Level->site(i,j).showchar = CHAOS_SEA;
		    break;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		    Level->site(i,j).showchar = Level->site(i,j).locchar = VILLAGE;
		    Level->site(i,j).aux = 1 + site - 'a';
		    break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		    Level->site(i,j).showchar = Level->site(i,j).locchar = TEMPLE;
		    Level->site(i,j).aux = site - '0';
		    break;
		case (PLAINS & 0xff):
		    Level->site(i,j).showchar = Level->site(i,j).locchar = PLAINS;
		    break;
		case (TUNDRA & 0xff):
		    Level->site(i,j).showchar = Level->site(i,j).locchar = TUNDRA;
		    break;
		case (ROAD & 0xff):
		    Level->site(i,j).showchar = Level->site(i,j).locchar = ROAD;
		    break;
		case (MOUNTAINS & 0xff):
		    Level->site(i,j).showchar = Level->site(i,j).locchar = MOUNTAINS;
		    break;
		case (RIVER & 0xff):
		    Level->site(i,j).showchar = Level->site(i,j).locchar = RIVER;
		    break;
		case (CITY & 0xff):
		    Level->site(i,j).showchar = Level->site(i,j).locchar = CITY;
		    break;
		case (FOREST & 0xff):
		    Level->site(i,j).showchar = Level->site(i,j).locchar = FOREST;
		    break;
		case (JUNGLE & 0xff):
		    Level->site(i,j).showchar = Level->site(i,j).locchar = JUNGLE;
		    break;
		case (SWAMP & 0xff):
		    Level->site(i,j).showchar = Level->site(i,j).locchar = SWAMP;
		    break;
		case (DESERT & 0xff):
		    Level->site(i,j).showchar = Level->site(i,j).locchar = DESERT;
		    break;
		case (CHAOS_SEA & 0xff):
		    Level->site(i,j).showchar = Level->site(i,j).locchar = CHAOS_SEA;
		    break;
	    }
	}
    }
}

// loads the dragon's lair into Level
void load_dlair (int empty, int populate)
{
    if (empty) {
	mprint ("The Lair is now devoid of inhabitants and treasure.");
	morewait();
    }
    if (!populate)
	empty = true;
    const char* ld = Level->init_from_data (E_DLAIR, Level_DragonLair);
    for (unsigned j = 0; j < Level->height; ++j, ++ld) {
	for (unsigned i = 0; i < Level->width; ++i) {
	    Level->site(i,j).lstatus = 0;
	    if (i < 48)
		Level->site(i,j).roomnumber = RS_CAVERN;
	    else
		Level->site(i,j).roomnumber = RS_DRAGONLORD;
	    Level->site(i,j).p_locf = L_NO_OP;
	    char site = *ld++;
	    switch (site) {
		case 'D':
		    Level->site(i,j).locchar = FLOOR;
		    if (!empty) {
			monster& m = make_site_monster (i, j, DRAGON_LORD);
			m.specialf = M_SP_LAIR;
		    }
		    break;
		case 'd':
		    Level->site(i,j).locchar = FLOOR;
		    if (!empty) {
			monster& m = make_site_monster (i, j, DRAGON);	// elite dragons, actually
			m.specialf = M_SP_LAIR;
			m.hit *= 2;
			m.dmg *= 2;
		    }
		    break;
		case 'W':
		    Level->site(i,j).locchar = FLOOR;
		    if (!empty)
			make_site_monster (i, j, KING_WYV);
		    break;
		case 'M':
		    Level->site(i,j).locchar = FLOOR;
		    if (!empty)
			make_site_monster (i, j, RANDOM);
		    break;
		case 'S':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).showchar = WALL;
		    if (!empty)
			lset (i, j, SECRET);
		    Level->site(i,j).roomnumber = RS_SECRETPASSAGE;
		    break;
		case '$':
		    Level->site(i,j).locchar = FLOOR;
		    if (!empty)
			make_site_treasure (i, j, 10);
		    break;
		case 's':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_TRAP_SIREN;
		    break;
		case '7':
		    if (!empty)
			Level->site(i,j).locchar = PORTCULLIS;
		    else
			Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_PORTCULLIS;
		    break;
		case 'R':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_RAISE_PORTCULLIS;
		    break;
		case 'p':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_PORTCULLIS;
		    break;
		case 'T':
		    Level->site(i,j).locchar = FLOOR;
		    if (!empty)
			Level->site(i,j).p_locf = L_PORTCULLIS_TRAP;
		    break;
		case 'X':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_TACTICAL_EXIT;
		    break;
		case '#':
		    Level->site(i,j).locchar = WALL;
		    Level->site(i,j).aux = 150;
		    break;
		case '.':
		    Level->site(i,j).locchar = FLOOR;
		    break;
	    }
	}
    }
}

// loads the star peak into Level
void load_speak (int empty, int populate)
{
    const bool safe = Player.alignment > 0;
    if (empty) {
	mprint ("The peak is now devoid of inhabitants and treasure.");
	morewait();
    }
    if (!populate)
	empty = true;
    const char* ld = Level->init_from_data (E_STARPEAK, Level_StarPeak);
    for (unsigned j = 0; j < Level->height; ++j, ++ld) {
	for (unsigned i = 0; i < Level->width; ++i) {
	    Level->site(i,j).lstatus = 0;
	    Level->site(i,j).roomnumber = RS_STARPEAK;
	    Level->site(i,j).p_locf = L_NO_OP;
	    char site = *ld++;
	    switch (site) {
		case 'S':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).showchar = WALL;
		    lset (i, j, SECRET);
		    Level->site(i,j).roomnumber = RS_SECRETPASSAGE;
		    break;
		case 'L':
		    Level->site(i,j).locchar = FLOOR;
		    if (!empty) {
			monster& m = make_site_monster (i, j, LAWBRINGER);
			if (safe)
			    m_status_reset (m, HOSTILE);
		    }
		    break;
		case 's':
		    Level->site(i,j).locchar = FLOOR;
		    if (!empty) {
			monster& m = make_site_monster (i, j, SERV_LAW);
			if (safe)
			    m_status_reset (m, HOSTILE);
		    }
		    break;
		case 'M':
		    Level->site(i,j).locchar = FLOOR;
		    if (!empty) {
			monster& m = make_site_monster (i, j, -1);
			if (safe)
			    m_status_reset (m, HOSTILE);
		    }
		    break;
		case '$':
		    Level->site(i,j).locchar = FLOOR;
		    if (!empty)
			make_site_treasure (i, j, 10);
		    break;
		case '7':
		    if (!empty)
			Level->site(i,j).locchar = PORTCULLIS;
		    else
			Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_PORTCULLIS;
		    break;
		case 'R':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_RAISE_PORTCULLIS;
		    break;
		case '-':
		    Level->site(i,j).locchar = CLOSED_DOOR;
		    break;
		case '|':
		    Level->site(i,j).locchar = OPEN_DOOR;
		    break;
		case 'p':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_PORTCULLIS;
		    break;
		case 'T':
		    Level->site(i,j).locchar = FLOOR;
		    if (!empty)
			Level->site(i,j).p_locf = L_PORTCULLIS_TRAP;
		    break;
		case 'X':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_TACTICAL_EXIT;
		    break;
		case '#':
		    Level->site(i,j).locchar = WALL;
		    Level->site(i,j).aux = 150;
		    break;
		case '4':
		    Level->site(i,j).locchar = RUBBLE;
		    Level->site(i,j).p_locf = L_RUBBLE;
		    break;
		case '.':
		    Level->site(i,j).locchar = FLOOR;
		    break;
	    }
	}
    }
}

// loads the magic isle into Level
void load_misle (int empty, int populate)
{
    if (empty) {
	mprint ("The isle is now devoid of inhabitants and treasure.");
	morewait();
    }
    if (!populate)
	empty = true;
    const char* ld = Level->init_from_data (E_MAGIC_ISLE, Level_MagicIsle);
    for (unsigned j = 0; j < Level->height; ++j, ++ld) {
	for (unsigned i = 0; i < Level->width; ++i) {
	    Level->site(i,j).lstatus = 0;
	    Level->site(i,j).roomnumber = RS_MAGIC_ISLE;
	    Level->site(i,j).p_locf = L_NO_OP;
	    char site = *ld++;
	    switch (site) {
		case 'E':
		    Level->site(i,j).locchar = FLOOR;
		    if (!empty)
			make_site_monster (i, j, EATER);
		    break;
		case 'm':
		    Level->site(i,j).locchar = FLOOR;
		    if (!empty)
			make_site_monster (i, j, MIL_PRIEST);
		    break;
		case 'n':
		    Level->site(i,j).locchar = FLOOR;
		    if (!empty)
			make_site_monster (i, j, NAZGUL);
		    break;
		case 'X':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_TACTICAL_EXIT;
		    break;
		case '#':
		    Level->site(i,j).locchar = WALL;
		    Level->site(i,j).aux = 150;
		    break;
		case '4':
		    Level->site(i,j).locchar = RUBBLE;
		    Level->site(i,j).p_locf = L_RUBBLE;
		    break;
		case '~':
		    Level->site(i,j).locchar = WATER;
		    Level->site(i,j).p_locf = L_CHAOS;
		    break;
		case '=':
		    Level->site(i,j).locchar = WATER;
		    Level->site(i,j).p_locf = L_MAGIC_POOL;
		    break;
		case '-':
		    Level->site(i,j).locchar = CLOSED_DOOR;
		    break;
		case '|':
		    Level->site(i,j).locchar = OPEN_DOOR;
		    break;
		case '.':
		    Level->site(i,j).locchar = FLOOR;
		    break;
	    }
	}
    }
}

// loads a temple into Level
void load_temple (int deity, int populate)
{
    const char* ld = Level->init_from_data (E_TEMPLE, Level_Temple);
    for (unsigned j = 0; j < Level->height; ++j, ++ld) {
	for (unsigned i = 0; i < Level->width; ++i) {
	    switch (deity) {
		case ODIN:	Level->site(i,j).roomnumber = RS_ODIN; break;
		case SET:	Level->site(i,j).roomnumber = RS_SET; break;
		case HECATE:	Level->site(i,j).roomnumber = RS_HECATE; break;
		case ATHENA:	Level->site(i,j).roomnumber = RS_ATHENA; break;
		case DRUID:	Level->site(i,j).roomnumber = RS_DRUID; break;
		case DESTINY:	Level->site(i,j).roomnumber = RS_DESTINY; break;
	    }
	    char site = *ld++;
	    switch (site) {
		case '8':
		    Level->site(i,j).locchar = ALTAR;
		    Level->site(i,j).p_locf = L_ALTAR;
		    Level->site(i,j).aux = deity;
		    break;
		case 'H':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate && (!Player.patron || Player.name != Priest[Player.patron] || Player.rank[PRIESTHOOD] != HIGHPRIEST))
			make_high_priest (i, j, deity);
		    break;
		case 'S':
		    Level->site(i,j).locchar = FLOOR;
		    if (!Player.patron || Player.name != Priest[Player.patron] || Player.rank[PRIESTHOOD] != HIGHPRIEST)
			lset (i, j, SECRET);
		    break;
		case 'W':
		    Level->site(i,j).locchar = FLOOR;
		    if (deity != Player.patron && deity != DRUID)
			Level->site(i,j).p_locf = L_TEMPLE_WARNING;
		    break;
		case 'm':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate)
			make_site_monster (i, j, MIL_PRIEST);
		    break;
		case 'd':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate)
			make_site_monster (i, j, DOBERMAN);
		    break;
		case 'X':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_TACTICAL_EXIT;
		    break;
		case '#':
		    if (deity != DRUID) {
			Level->site(i,j).locchar = WALL;
			Level->site(i,j).aux = 150;
		    } else {
			Level->site(i,j).locchar = HEDGE;
			Level->site(i,j).p_locf = L_HEDGE;
		    }
		    break;
		case '.':
		    Level->site(i,j).locchar = FLOOR;
		    break;
		case 'x':
		    Level->site(i,j).locchar = FLOOR;
		    random_temple_site (i, j, deity, populate);
		    break;
		case '?':
		    if (deity != DESTINY)
			Level->site(i,j).locchar = FLOOR;
		    else {
			Level->site(i,j).locchar = ABYSS;
			Level->site(i,j).p_locf = L_ADEPT;
		    }
		    break;
		case '-':
		    Level->site(i,j).locchar = CLOSED_DOOR;
		    break;
		case '|':
		    Level->site(i,j).locchar = OPEN_DOOR;
		    break;
	    }
	}
    }
    // Main Temple is peaceful for player of same sect, druids always peaceful.
    if (Player.patron == deity || deity == DRUID)
	foreach (m, Level->mlist)
	    m_status_reset (*m, HOSTILE);
}

static void random_temple_site (int i, int j, int deity UNUSED, int populate)
{
    switch (random_range (12)) {
	case 0:
	    if (populate)
		make_site_monster (i, j, MEND_PRIEST);
	    break;
	case 1:
	    Level->site(i,j).locchar = WATER;
	    Level->site(i,j).p_locf = L_MAGIC_POOL;
	case 2:
	    if (populate)
		make_site_monster (i, j, INNER_DEMON);
	    break;
	case 3:
	    if (populate)
		make_site_monster (i, j, ANGEL);
	    break;
	case 4:
	    if (populate)
		make_site_monster (i, j, HIGH_ANGEL);
	    break;
	case 5:
	    if (populate)
		make_site_monster (i, j, ARCHANGEL);
	    break;
    }
}

static void make_high_priest (int i, int j, int deity)
{
    monster& m = make_site_monster (i, j, NPC);
    make_hiscore_npc (m, deity);
}

void l_merc_guild (void)
{
    print1 ("Legion of Destiny, Mercenary Guild, Inc.");
    if (nighttime())
	print2 ("The barracks are under curfew right now.");
    else {
	print2 ("You enter Legion HQ, ");
	if (Player.rank[LEGION] == COMMANDANT) {
	    nprint2 ("Your aide follows you to the staff room.");
	    morewait();
	    clearmsg();
	}
	if (Player.rank[LEGION] >= LEGIONAIRE) {
	    nprint2 ("and report to your commander.");
	    morewait();
	}
	switch (Player.rank[LEGION]) {
	    case 0:
		nprint2 ("and see the Recruiting Centurion.");
		morewait();
		print2 ("Do you wish to join the Legion? [yn] ");
		if (ynq2() == 'y') {
		    clearmsg();
		    if (Player.rank[ARENA] >= TRAINEE) {
			print1 ("The Centurion checks your record, and gets angry:");
			print2 ("'The legion don't need any Arena Jocks. Git!'");
		    } else if (Player.rank[ORDER] >= GALLANT) {
			print1 ("The Centurion checks your record, and is amused:");
			print2 ("'A paladin in the ranks? You must be joking.'");
		    } else if (Player.con < 12) {
			print1 ("The Centurion looks you over, sadly.");
			print2 ("You are too fragile to join the legion.");
		    } else if (Player.str < 10) {
			print1 ("The Centurion looks at you contemptuously.");
			print2 ("Your strength is too low to pass the physical!");
		    } else {
			print1 ("You are tested for strength and stamina...");
			morewait();
			nprint1 (" and you pass!");
			print2 ("Commandant ");
			nprint2 (Commandant);
			nprint2 (" shakes your hand.");
			morewait();
			print2 ("The Legion pays you a 500Au induction fee.");
			morewait();
			print1 ("You are also issued a shortsword and leather.");
			print2 ("You are now a Legionaire.");
			morewait();
			clearmsg();
			gain_item (Objects[WEAPON_SHORT_SWORD]);
			gain_item (Objects[ARMOR_SOFT_LEATHER]);
			Player.cash += 500;
			Player.rank[LEGION] = LEGIONAIRE;
			Player.guildxp[LEGION] = 1;
			Player.str++;
			Player.con++;
			Player.maxstr++;
			Player.maxcon++;
		    }
		}
		break;
	    case COMMANDANT:
		print1 ("You find the disposition of your forces satisfactory.");
		break;
	    case COLONEL:
		if ((Player.level > Commandantlevel) && find_and_remove_item (CORPSEID, DEMON_EMP)) {
		    print1 ("You liberated the Demon Emperor's Regalia!");
		    morewait();
		    clearmsg();
		    print1 ("The Legion is assembled in salute to you!");
		    print2 ("The Regalia is held high for all to see and admire.");
		    morewait();
		    clearmsg();
		    print1 ("Commandant ");
		    nprint1 (Commandant);
		    nprint1 (" promotes you to replace him,");
		    print2 ("and announces his own overdue retirement.");
		    morewait();
		    clearmsg();
		    print1 ("You are the new Commandant of the Legion!");
		    print2 ("The Emperor's Regalia is sold for a ridiculous sum.");
		    morewait();
		    clearmsg();
		    print1 ("You now know the Spell of Regeneration.");
		    learn_spell (S_REGENERATE);
		    Player.rank[LEGION] = COMMANDANT;
		    Player.maxstr += 2;
		    Player.str += 2;
		    Player.maxcon += 2;
		    Player.con += 2;
		    print2 ("Your training is complete. You get top salary.");
		    Player.cash += 20000;
		} else if (Player.level <= Commandantlevel) {
		    clearmsg();
		    print1 ("Your CO expresses satisfaction with your progress.");
		    print2 ("But your service record does not yet permit promotion.");
		} else {
		    clearmsg();
		    print1 ("Why do you come empty handed?");
		    print2 ("You must return with the Regalia of the Demon Emperor!");
		}
		break;
	    case FORCE_LEADER:
		clearmsg();
		print1 ("Your CO expresses satisfaction with your progress.");
		if (Player.guildxp[LEGION] < 4000)
		    print2 ("But your service record does not yet permit promotion.");
		else {
		    print2 ("You have been promoted to Legion Colonel!");
		    morewait();
		    print1 ("Your next promotion is contingent on the return of");
		    print2 ("the Regalia of the Demon Emperor.");
		    morewait();
		    print1 ("The Demon Emperor holds court at the base of a volcano");
		    print2 ("to the far south, in the heart of a swamp.");
		    morewait();
		    clearmsg();
		    print1 ("You have been taught the spell of heroism!");
		    learn_spell (S_HERO);
		    Player.rank[LEGION] = COLONEL;
		    Player.maxstr++;
		    Player.str++;
		    Player.maxcon++;
		    Player.con++;
		    print2 ("You are given advanced training, and a raise.");
		    Player.cash += 10000;
		}
		break;
	    case CENTURION:
		clearmsg();
		print1 ("Your CO expresses satisfaction with your progress.");
		if (Player.guildxp[LEGION] < 1500)
		    print2 ("But your service record does not yet permit promotion.");
		else {
		    print2 ("You are now a Legion Force-Leader!");
		    Player.rank[LEGION] = FORCE_LEADER;
		    Player.maxstr++;
		    Player.str++;
		    morewait();
		    clearmsg();
		    print1 ("You receive more training, and bonus pay.");
		    Player.cash += 5000;
		}
		break;
	    case LEGIONAIRE:
		clearmsg();
		print1 ("Your CO expresses satisfaction with your progress.");
		if (Player.guildxp[LEGION] < 400)
		    print2 ("But your service record does not yet permit promotion.");
		else {
		    print2 ("You are promoted to Legion Centurion!");
		    morewait();
		    clearmsg();
		    print1 ("You get advanced training, and a higher salary.");
		    Player.rank[LEGION] = CENTURION;
		    Player.maxcon++;
		    Player.con++;
		    Player.cash += 2000;
		}
		break;
	}
    }
}

void l_castle (void)
{
    if (Player.level < 3) {
	print1 ("You can't possibly enter the castle, you nobody!");
	print2 ("Come back when you are famous.");
    } else {
	print1 ("You are ushered into the castle.");
	if (Player.rank[NOBILITY] < DUKE) {
	    print2 ("His Grace, ");
	    nprint2 (Duke);
	    nprint2 ("-- Duke of Rampart! <fanfare>");
	    morewait();
	    clearmsg();
	}
	if (Player.rank[NOBILITY] == NOT_NOBILITY) {
	    print1 ("Well, sirrah, wouldst embark on a quest? [yn] ");
	    if (ynq1() == 'y') {
		print2 ("Splendid. Bring me the head of the Goblin King.");
		Player.rank[NOBILITY] = COMMONER;
	    } else {
		print1 ("You scoundrel! Guards! Take this blackguard away!");
		morewait();
		p_damage (25, UNSTOPPABLE, "castle guards for lese majeste");
		send_to_jail();
	    }
	} else if (Player.rank[NOBILITY] == COMMONER) {
	    if (find_and_remove_item (CORPSEID, GOBLIN_KING)) {
		print1 ("Good job, sirrah! I promote you to the rank of esquire.");
		Player.rank[NOBILITY] = ESQUIRE;
		gain_experience (100);
		print2 ("Now that you have proved yourself true, another quest!");
		morewait();
		print1 ("Bring to me a Holy Defender!");
		print2 ("One is said to be in the possession of the Great Wyrm");
		morewait();
		clearmsg();
		print1 ("in the depths of the sewers below the city.");
	    } else
		print2 ("Do not return until you achieve the quest, caitiff!");
	} else if (Player.rank[NOBILITY] == ESQUIRE) {
	    if (find_and_remove_item (WEAPON_DEFENDER, -1)) {
		print1 ("My thanks, squire. In return, I dub thee knight!");
		Player.rank[NOBILITY] = KNIGHT;
		gain_experience (1000);
		print2 ("If thou wouldst please me further...");
		morewait();
		print1 ("Bring me a suit of dragonscale armor.");
		print2 ("You might have to kill a dragon to get one....");
	    } else
		print2 ("Greetings, squire. My sword? What, you don't have it?");
	} else if (Player.rank[NOBILITY] == KNIGHT) {
	    if (find_and_remove_item (ARMOR_DRAGONSCALE, -1)) {
		print1 ("Thanks, good sir knight.");
		print2 ("Here are letters patent to a peerage!");
		Player.rank[NOBILITY] = LORD;
		gain_experience (10000);
		morewait();
		print1 ("If you would do me a final service...");
		print2 ("I require the Orb of Mastery. If you would be so kind...");
		morewait();
		print1 ("By the way, you might find the Orb in the possession");
		print2 ("Of the Elemental Master on the Astral Plane");
	    } else
		print2 ("Your quest is not yet complete, sir knight.");
	} else if (Player.rank[NOBILITY] == LORD) {
	    if (find_item (ORB_OF_MASTERY)) {
		print1 ("My sincerest thanks, my lord.");
		print2 ("You have proved yourself a true paragon of chivalry");
		morewait();
		print1 ("I abdicate the Duchy in your favor....");
		print2 ("Oh, you can keep the Orb, by the way....");
		Player.rank[NOBILITY] = DUKE;
		gain_experience (10000);
		morewait();
		for (int y = 52; y < 63; y++)
		    for (int x = 2; x < 52; x++) {
			if (Level->site(x,y).p_locf == L_TRAP_SIREN) {
			    Level->site(x,y).p_locf = L_NO_OP;
			    lset (x, y, CHANGED);
			}
			if (x >= 12 && loc_statusp (x, y, SECRET)) {
			    lreset (x, y, SECRET);
			    lset (x, y, CHANGED);
			}
			if (x >= 20 && x <= 23 && y == 56) {
			    Level->site(x,y).locchar = FLOOR;
			    lset (x, y, CHANGED);
			}
		    }

	    } else
		print2 ("I didn't really think you were up to the task....");
	}
    }
}

void l_arena (void)
{
    char response;
    int prize;

    print1 ("Rampart Coliseum");
    if (Player.rank[ARENA] == NOT_IN_ARENA) {
	print2 ("Enter the games, or Register as a Gladiator? [e,r,ESCAPE] ");
	do
	    response = (char) mcigetc();
	while ((response != 'e') && (response != 'r') && (response != KEY_ESCAPE));
    } else {
	print2 ("Enter the games? [yn] ");
	response = ynq2();
	if (response == 'y')
	    response = 'e';
	else
	    response = KEY_ESCAPE;
    }
    if (response == 'r') {
	if (Player.rank[ARENA] >= TRAINEE)
	    print2 ("You're already a gladiator....");
	else if (Player.rank[ORDER] >= GALLANT)
	    print2 ("We don't let Paladins into our Guild.");
	else if (Player.rank[LEGION] >= LEGIONAIRE)
	    print2 ("We don't train no stinkin' mercs!");
	else if (Player.str < 13)
	    print2 ("Yer too weak to train!");
	else if (Player.agi < 12)
	    print2 ("Too clumsy to be a gladiator!");
	else {
	    print1 ("Ok, yer now an Arena Trainee.");
	    print2 ("Here's a wooden sword, and a shield");
	    morewait();
	    clearmsg();
	    gain_item (Objects[WEAPON_CLUB]);
	    gain_item (Objects[SHIELD_SMALL_ROUND]);
	    Player.rank[ARENA] = TRAINEE;
	    Arena_Opponent = 3;
	    morewait();
	    clearmsg();
	    print1 ("You've got 5000Au credit at the Gym.");
	    Gymcredit += 5000;
	}
    } else if (response == 'e') {
	print1 ("OK, we're arranging a match....");
	morewait();
	clearmsg();
	change_environment (E_ARENA);
	print1 ("Let the battle begin....");

	time_clock (true);
	while (Current_Environment == E_ARENA)
	    time_clock (false);

	// WDT -- Sheldon Simms points out that these objects are not
	// wastes of space; on the contrary, they can be carried out of the
	// arena.  Freeing them was causing subtle and hard to find problems.
	// However, not freeing them is causing a couple of tiny memory leaks.
	// This should be fixed, probably by modifying the object destruction
	// procedures to account for this case.  I'm not really concerned.
	// David Given has proposed a nicer solution, but it still causes a
	// memory leak.  Obviously, we need a special field just for names
	// in the monster struct.  Yadda yadda -- I'll mmark this with a
	// HACK!, and comme back to it later.
	// can not free the corpse string... it is referenced in the
	// corpse string of the corpse object.
	// Unfortunately, this will cause a memory leak, but I don't see
	// any way to avoid it.  This fixes the munged arena corpse names
	// problem. -DAG
	// delete corpse;

	if (!Arena_Victory) {
	    print1 ("The crowd boos your craven behavior!!!");
	    if (Player.rank[ARENA] >= TRAINEE) {
		print2 ("You are thrown out of the Gladiator's Guild!");
		morewait();
		clearmsg();
		if (Gymcredit > 0)
		    print1 ("Your credit at the gym is cut off!");
		Gymcredit = 0;
		Player.rank[ARENA] = FORMER_GLADIATOR;
	    }
	} else {
	    Arena_Opponent++;
	    if (Arena_Victory == 21) {
		print1 ("The crowd roars its approval!");
		if (Player.rank[ARENA]) {
		    print2 ("You are the new Arena Champion!");
		    Player.rank[ARENA] = CHAMPION;
		    morewait();
		    print1 ("You are awarded the Champion's Spear: Victrix!");
		    morewait();
		    gain_item (Objects[WEAPON_VICTRIX]);

		} else {
		    print1 ("As you are not an official gladiator,");
		    nprint1 ("you are not made Champion.");
		    morewait();
		}
	    }
	    morewait();
	    clearmsg();
	    print1 ("Good fight! ");
	    nprint1 ("Your prize is: ");
	    prize = max (25, (Arena_Victory-1) * 50);
	    if (Player.rank[ARENA] >= TRAINEE)
		prize *= 2;
	    mnumprint (prize);
	    nprint1 ("Au.");
	    Player.cash += prize;
	    if (Player.rank[ARENA] < GLADIATOR && Arena_Opponent > 5 && !(Arena_Opponent % 3)) {
		if (Player.rank[ARENA] >= TRAINEE) {
		    ++Player.rank[ARENA];
		    morewait();
		    print1 ("You've been promoted to a stronger class!");
		    print2 ("You are also entitled to additional training.");
		    Gymcredit += Arena_Opponent * 1000;
		}
	    }
	}
	xredraw();
    } else
	clearmsg();
}

void l_thieves_guild (void)
{
    unsigned fee, count, number, done = false, dues = 1000;
    char c, action;
    print1 ("You have penetrated to the Lair of the Thieves' Guild.");
    if (!nighttime())
	print2 ("There aren't any thieves around in the daytime.");
    else {
	if ((Player.rank[THIEVES] == TMASTER) && (Player.level > Shadowlordlevel) && find_and_remove_item (THING_JUSTICIAR_BADGE, -1)) {
	    print2 ("You nicked the Justiciar's Badge!");
	    morewait();
	    print1 ("The Badge is put in a place of honor in the Guild Hall.");
	    print2 ("You are now the Shadowlord of the Thieves' Guild!");
	    morewait();
	    print1 ("Who says there's no honor among thieves?");
	    morewait();
	    clearmsg();
	    print1 ("You learn the Spell of Shadowform.");
	    learn_spell (S_SHADOWFORM);
	    morewait();
	    clearmsg();
	    Player.rank[THIEVES] = SHADOWLORD;
	    Player.maxagi += 2;
	    Player.maxdex += 2;
	    Player.agi += 2;
	    Player.dex += 2;
	}
	while (!done) {
	    menuclear();
	    if (Player.rank[THIEVES] == NOT_A_THIEF)
		menuprint ("a: Join the Thieves' Guild.\n");
	    else
		menuprint ("b: Raise your Guild rank.\n");
	    menuprint ("c: Get an item identified.\n");
	    if (Player.rank[THIEVES] >= TMEMBER)
		menuprint ("d: Fence an item.\n");
	    menuprint ("ESCAPE: Leave this Den of Iniquity.");
	    showmenu();
	    action = mgetc();
	    if (action == KEY_ESCAPE)
		done = true;
	    else if (action == 'a') {
		done = true;
		if (Player.rank[THIEVES] >= TMEMBER)
		    print2 ("You are already a member!");
		else if (Player.alignment > 10)
		    print2 ("You are too lawful to be a thief!");
		else {
		    dues += dues * (12 - Player.dex) / 9;
		    dues += Player.alignment * 5;
		    dues = max (100U, dues);
		    clearmsg();
		    mprint ("Dues are");
		    mnumprint (dues);
		    mprint (" Au. Pay it? [yn] ");
		    if (ynq1() == 'y') {
			if (Player.cash < dues) {
			    print1 ("You can't cheat the Thieves' Guild!");
			    print2 ("... but the Thieves' Guild can cheat you....");
			    Player.cash = 0;
			} else {
			    print1 ("Shadowlord ");
			    nprint1 (Shadowlord);
			    print2 ("enters your name into the roll of the Guild.");
			    morewait();
			    clearmsg();
			    print1 ("As a special bonus, you get a free lockpick.");
			    print2 ("You are taught the spell of Object Detection.");
			    morewait();
			    learn_spell (S_OBJ_DET);
			    gain_item (Objects[THING_LOCKPICK]);
			    Player.cash -= dues;
			    dataprint();
			    Player.guildxp[THIEVES] = 1;
			    Player.rank[THIEVES] = TMEMBER;
			    Player.maxdex++;
			    Player.dex++;
			    Player.agi++;
			    Player.maxagi++;
			}
		    }
		}
	    } else if (action == 'b') {
		if (Player.rank[THIEVES] == NOT_A_THIEF)
		    print2 ("You are not even a member!");
		else if (Player.rank[THIEVES] == SHADOWLORD)
		    print2 ("You can't get any higher than this!");
		else if (Player.rank[THIEVES] == TMASTER) {
		    if (Player.level <= Shadowlordlevel)
			print2 ("You are not experienced enough to advance.");
		    else
			print2 ("You must bring back the Justiciar's Badge!");
		} else if (Player.rank[THIEVES] == THIEF) {
		    if (Player.guildxp[THIEVES] < 4000)
			print2 ("You are not experienced enough to advance.");
		    else {
			print1 ("You are now a Master Thief of the Guild!");
			print2 ("You are taught the Spell of Apportation.");
			morewait();
			print1 ("To advance to the next level you must return with");
			print2 ("the badge of the Justiciar (cursed be his name).");
			morewait();
			clearmsg();
			print1 ("The Justiciar's office is just south of the gaol.");
			learn_spell (S_APPORT);
			Player.rank[THIEVES] = TMASTER;
			Player.maxagi++;
			Player.maxdex++;
			Player.agi++;
			Player.dex++;
		    }
		} else if (Player.rank[THIEVES] == ATHIEF) {
		    if (Player.guildxp[THIEVES] < 1500)
			print2 ("You are not experienced enough to advance.");
		    else {
			print1 ("You are now a ranking Thief of the Guild!");
			print2 ("You learn the Spell of Invisibility.");
			learn_spell (S_INVISIBLE);
			Player.rank[THIEVES] = THIEF;
			Player.agi++;
			Player.maxagi++;
		    }
		} else if (Player.rank[THIEVES] == TMEMBER) {
		    if (Player.guildxp[THIEVES] < 400)
			print2 ("You are not experienced enough to advance.");
		    else {
			print1 ("You are now an Apprentice Thief!");
			print2 ("You are taught the Spell of Levitation.");
			learn_spell (S_LEVITATE);
			Player.rank[THIEVES] = ATHIEF;
			Player.dex++;
			Player.maxdex++;
		    }
		}
	    } else if (action == 'c') {
		if (Player.rank[THIEVES] == NOT_A_THIEF) {
		    print1 ("RTG, Inc, Appraisers. Identification Fee: 50Au/item.");
		    fee = 50;
		} else {
		    fee = 5;
		    print1 ("The fee is 5Au per item.");
		}
		print2 ("Identify one item, or all possessions? [ip] ");
		if ((char) mcigetc() == 'i') {
		    if (Player.cash < fee)
			print2 ("Try again when you have the cash.");
		    else {
			Player.cash -= fee;
			dataprint();
			identify (0);
		    }
		} else {
		    count = 0;
		    for (unsigned i = 1; i < MAXITEMS; i++)
			if (Player.has_possession(i))
			    if (!object_is_known(Player.possessions[i]))
				count++;
		    foreach (i, Player.pack)
			count += !object_is_known(i);
		    clearmsg();
		    print1 ("The fee will be: ");
		    mnumprint (max (count * fee, fee));
		    nprint1 ("Au. Pay it? [yn] ");
		    if (ynq1() == 'y') {
			if (Player.cash < max (count * fee, fee))
			    print2 ("Try again when you have the cash.");
			else {
			    Player.cash -= max (count * fee, fee);
			    dataprint();
			    identify (1);
			}
		    }
		}
	    } else if (action == 'd') {
		if (Player.rank[THIEVES] == NOT_A_THIEF)
		    print2 ("Fence? Who said anything about a fence?");
		else {
		    print1 ("Fence one item or go through pack? [ip] ");
		    if ((char) mcigetc() == 'i') {
			int i = getitem (NULL_ITEM);
			if (i == ABORT || !Player.has_possession(i))
			    print2 ("Huh, Is this some kind of set-up?");
			else if (Player.possessions[i].blessing < 0)
			    print2 ("I don't want to buy a cursed item!");
			else {
			    clearmsg();
			    unsigned cost = 2 * item_value (Player.possessions[i]) / 3;
			    print1 ("I'll give you ");
			    mlongprint (cost);
			    nprint1 ("Au each. OK? [yn] ");
			    if (ynq1() == 'y') {
				number = getnumber (Player.possessions[i].number);
				// Fenced artifacts could turn up anywhere, really...
				if (object_uniqueness(Player.possessions[i]) > UNIQUE_UNMADE)
				    set_object_uniqueness (Player.possessions[i], UNIQUE_UNMADE);
				Player.remove_possession (i, number);
				Player.cash += number * cost;
				dataprint();
			    } else
				print2 ("Hey, gimme a break, it was a fair price!");
			}
		    } else {
			foreach (i, Player.pack) {
			    if (i->blessing > -1) {
				clearmsg();
				print1 ("Sell ");
				nprint1 (itemid (i));
				nprint1 (" for ");
				mlongprint (2 * item_value (i) / 3);
				nprint1 ("Au each? [ynq] ");
				if ((c = ynq1()) == 'q')
				    break;
				else if (c == 'y') {
				    number = getnumber (i->number);
				    Player.cash += 2 * number * item_value (i) / 3;
				    if ((i->number -= number) < 1) {
					// Fenced an artifact?  You just might see it again.
					if (object_uniqueness(i) > UNIQUE_UNMADE)
					    set_object_uniqueness (i, UNIQUE_UNMADE);
					--(i = Player.pack.erase(i));
				    }
				    dataprint();
				}
			    }
			}
		    }
		}
	    }
	}
    }
    xredraw();
}

void l_college (void)
{
    char action;
    int done = false, enrolled = false;
    print1 ("The Collegium Magii. Founded 16937, AOF.");
    if (nighttime())
	print2 ("The Registration desk is closed at night....");
    else {
	while (!done) {
	    if ((Player.rank[COLLEGE] == MAGE) && (Player.level > Archmagelevel) && find_and_remove_item (CORPSEID, EATER)) {
		print1 ("You brought back the heart of the Eater of Magic!");
		morewait();
		print1 ("The Heart is sent to the labs for analysis.");
		print2 ("The Board of Trustees appoints you Archmage!");
		morewait();
		clearmsg();
		Player.rank[COLLEGE] = ARCHMAGE;
		Player.maxiq += 5;
		Player.iq += 5;
		Player.maxpow += 5;
		Player.pow += 5;
		morewait();
	    }
	    menuclear();
	    menuprint ("May we help you?\n\n");
	    menuprint ("a: Enroll in the College.\n");
	    menuprint ("b: Raise your College rank.\n");
	    menuprint ("c: Do spell research.\n");
	    menuprint ("ESCAPE: Leave these hallowed halls.\n");
	    showmenu();
	    action = mgetc();
	    if (action == KEY_ESCAPE)
		done = true;
	    else if (action == 'a') {
		if (Player.rank[COLLEGE] >= NOVICE)
		    print2 ("You are already enrolled!");
		else if (Player.iq < 13)
		    print2 ("Your low IQ renders you incapable of being educated.");
		else if (Player.rank[CIRCLE] >= INITIATE)
		    print2 ("Sorcery and our Magic are rather incompatable, no?");
		else {
		    if (Player.iq > 17) {
			print2 ("You are given a scholarship!");
			morewait();
			enrolled = true;
		    } else {
			print1 ("Tuition is 1000Au. ");
			nprint1 ("Pay it? [yn] ");
			if (ynq1() == 'y') {
			    if (Player.cash < 1000)
				print2 ("You don't have the funds!");
			    else {
				Player.cash -= 1000;
				enrolled = true;
				dataprint();
			    }
			}
		    }
		    if (enrolled) {
			print1 ("Archmage ");
			nprint1 (Archmage);
			nprint1 (" greets you and congratulates you on your acceptance.");
			print2 ("You are now enrolled in the Collegium Magii!");
			morewait();
			print1 ("You are now a Novice.");
			print2 ("You may research 1 spell, for your intro class.");
			Spellsleft = 1;
			Player.rank[COLLEGE] = INITIATE;
			Player.guildxp[COLLEGE] = 1;
			Player.maxiq += 1;
			Player.iq += 1;
			Player.maxpow += 1;
			Player.pow += 1;
		    }
		}
	    } else if (action == 'b') {
		if (Player.rank[COLLEGE] == NOT_IN_COLLEGE)
		    print2 ("You have not even been initiated, yet!");
		else if (Player.rank[COLLEGE] == ARCHMAGE)
		    print2 ("You are at the pinnacle of mastery in the Collegium.");
		else if (Player.rank[COLLEGE] == MAGE) {
		    if (Player.level <= Archmagelevel)
			print2 ("You are not experienced enough to advance.");
		    else
			print2 ("You must return with the heart of the Eater of Magic!");
		} else if (Player.rank[COLLEGE] == PRECEPTOR) {
		    if (Player.guildxp[COLLEGE] < 4000)
			print2 ("You are not experienced enough to advance.");
		    else {
			print1 ("You are now a Mage of the Collegium Magii!");
			print2 ("You may research 6 spells for postdoctoral research.");
			Spellsleft += 6;
			morewait();
			print1 ("To become Archmage, you must return with the");
			print2 ("heart of the Eater of Magic");
			morewait();
			clearmsg();
			print1 ("The Eater may be found on a desert isle somewhere.");
			Player.rank[COLLEGE] = MAGE;
			Player.maxiq += 2;
			Player.iq += 2;
			Player.maxpow += 2;
			Player.pow += 2;
		    }
		} else if (Player.rank[COLLEGE] == STUDENT) {
		    if (Player.guildxp[COLLEGE] < 1500)
			print2 ("You are not experienced enough to advance.");
		    else {
			print1 ("You are now a Preceptor of the Collegium Magii!");
			print2 ("You are taught the basics of ritual magic.");
			morewait();
			clearmsg();
			print1 ("Your position allows you to research 4 spells.");
			Spellsleft += 4;
			learn_spell (S_RITUAL);
			Player.rank[COLLEGE] = PRECEPTOR;
			Player.maxiq += 1;
			Player.iq += 1;
			Player.maxpow += 1;
			Player.pow += 1;
		    }
		} else if (Player.rank[COLLEGE] == NOVICE) {
		    if (Player.guildxp[COLLEGE] < 400)
			print2 ("You are not experienced enough to advance.");
		    else {
			print1 ("You are now a Student at the Collegium Magii!");
			print2 ("You are taught the spell of identification.");
			morewait();
			clearmsg();
			print1 ("Thesis research credit is 2 spells.");
			Spellsleft += 2;
			learn_spell (S_IDENTIFY);
			Player.rank[COLLEGE] = STUDENT;
			Player.maxiq += 1;
			Player.iq += 1;
			Player.maxpow += 1;
			Player.pow += 1;
		    }
		}
	    } else if (action == 'c') {
		clearmsg();
		if (Spellsleft > 0) {
		    print1 ("Research permitted: ");
		    mnumprint (Spellsleft);
		    nprint1 (" Spells.");
		    morewait();
		}
		if (Spellsleft < 1) {
		    print1 ("Extracurricular Lab fee: 2000 Au. ");
		    nprint1 ("Pay it? [yn] ");
		    if (ynq1() == 'y') {
			if (Player.cash < 2000)
			    print1 ("Try again when you have the cash.");
			else {
			    Player.cash -= 2000;
			    dataprint();
			    Spellsleft = 1;
			}
		    }
		}
		if (Spellsleft > 0) {
		    learnspell (0);
		    Spellsleft--;
		}
	    }
	}
    }
    xredraw();
}

void l_sorcerors (void)
{
    char action;
    bool done = false;
    unsigned fee = 3000;
    long total;
    print1 ("The Circle of Sorcerors.");
    if (Player.rank[CIRCLE] == FORMER_SOURCEROR) {
	print2 ("Fool! Didn't we tell you to go away?");
	Player.mana = 0;
	dataprint();
    } else
	while (!done) {
	    if ((Player.rank[CIRCLE] == HIGHSORCEROR) && (Player.level > Primelevel) && find_and_remove_item (CORPSEID, LAWBRINGER)) {
		print2 ("You obtained the Crown of the Lawgiver!");
		morewait();
		print1 ("The Crown is ritually sacrificed to the Lords of Chaos.");
		print2 ("You are now the Prime Sorceror of the Inner Circle!");
		morewait();
		clearmsg();
		print1 ("You learn the Spell of Disintegration!");
		morewait();
		clearmsg();
		learn_spell (S_DISINTEGRATE);
		Player.rank[CIRCLE] = PRIME;
		Player.maxpow += 10;
		Player.pow += 10;
	    }
	    menuclear();
	    menuprint ("May we help you?\n\n");
	    menuprint ("a: Become an Initiate of the Circle.\n");
	    menuprint ("b: Raise your rank in the Circle.\n");
	    menuprint ("c: Restore mana points\n");
	    menuprint ("ESCAPE: Leave these Chambers of Power.\n");
	    showmenu();
	    action = mgetc();
	    if (action == KEY_ESCAPE)
		done = true;
	    else if (action == 'a') {
		if (Player.rank[CIRCLE] >= INITIATE)
		    print2 ("You are already an initiate!");
		else if (Player.alignment > 0)
		    print2 ("You may not join -- you reek of Law!");
		else if (Player.rank[COLLEGE] >= NOVICE)
		    print2 ("Foolish Mage!  You don't have the right attitude to Power!");
		else {
		    fee += Player.alignment * 100;
		    fee += fee * (12 - Player.pow) / 9;
		    fee = max (100U, fee);
		    clearmsg();
		    mprint ("For you, there is an initiation fee of");
		    mnumprint (fee);
		    mprint (" Au.");
		    print2 ("Pay it? [yn] ");
		    if (ynq2() == 'y') {
			if (Player.cash < fee)
			    print3 ("Try again when you have the cash!");
			else {
			    print1 ("Prime Sorceror ");
			    nprint1 (Prime);
			    print2 ("conducts your initiation into the circle of novices.");
			    morewait();
			    clearmsg();
			    print1 ("You learn the Spell of Magic Missiles.");
			    learn_spell (S_MISSILE);
			    Player.cash -= fee;
			    dataprint();
			    Player.rank[CIRCLE] = INITIATE;
			    Player.guildxp[CIRCLE] = 1;
			    Player.maxpow++;
			    Player.pow++;
			}
		    }
		}
	    } else if (action == 'b') {
		if (Player.rank[CIRCLE] == NOT_IN_CIRCLE)
		    print2 ("You have not even been initiated, yet!");
		else if (Player.alignment > -1) {
		    print1 ("Ahh! You have grown too lawful!!!");
		    print2 ("You are hereby blackballed from the Circle!");
		    Player.rank[CIRCLE] = FORMER_SOURCEROR;
		    morewait();
		    clearmsg();
		    print1 ("A pox upon thee!");
		    if (!Player.immunity[INFECTION])
			Player.status[DISEASED] += 100;
		    print2 ("And a curse on your possessions!");
		    morewait();
		    clearmsg();
		    acquire (-1);
		    clearmsg();
		    enchant (-1);
		    bless (-1);
		    print3 ("Die, false sorceror!");
		    p_damage (25, UNSTOPPABLE, "a sorceror's curse");
		    done = true;
		} else if (Player.rank[CIRCLE] == PRIME) {
		    print2 ("You are at the pinnacle of mastery in the Circle.");
		} else if (Player.rank[CIRCLE] == HIGHSORCEROR) {
		    if (Player.level <= Primelevel)
			print2 ("You are not experienced enough to advance.");
		    else
			print2 ("You must return with the Crown of the LawBringer!");
		} else if (Player.rank[CIRCLE] == SORCEROR) {
		    if (Player.guildxp[CIRCLE] < 4000)
			print2 ("You are not experienced enough to advance.");
		    else {
			print1 ("You are now a High Sorceror of the Inner Circle!");
			print2 ("You learn the Spell of Disruption!");
			morewait();
			clearmsg();
			print1 ("To advance you must return with the LawBringer's Crown!");
			print2 ("The LawBringer resides on Star Peak.");
			learn_spell (S_DISRUPT);
			Player.rank[CIRCLE] = HIGHSORCEROR;
			Player.maxpow += 5;
			Player.pow += 5;
		    }
		} else if (Player.rank[CIRCLE] == ENCHANTER) {
		    if (Player.guildxp[CIRCLE] < 1500)
			print2 ("You are not experienced enough to advance.");
		    else {
			print1 ("You are now a member of the Circle of Sorcerors!");
			print2 ("You learn the Spell of Ball Lightning!");
			learn_spell (S_LBALL);
			Player.rank[CIRCLE] = SORCEROR;
			Player.maxpow += 2;
			Player.pow += 2;
		    }
		} else if (Player.rank[CIRCLE] == INITIATE) {
		    if (Player.guildxp[CIRCLE] < 400)
			print2 ("You are not experienced enough to advance.");
		    else {
			print1 ("You are now a member of the Circle of Enchanters!");
			print2 ("You learn the Spell of Firebolts.");
			learn_spell (S_FIREBOLT);
			Player.rank[CIRCLE] = ENCHANTER;
			Player.maxpow += 2;
			Player.pow += 2;
		    }
		}
	    } else if (action == 'c') {
		done = true;
		fee = Player.level * 100;
		if (Player.rank[CIRCLE])
		    fee = fee / 2;
		clearmsg();
		print1 ("That will be: ");
		mnumprint (fee);
		nprint1 ("Au. Pay it? [yn] ");
		if (ynq1() == 'y') {
		    if (Player.cash < fee)
			print2 ("Begone, deadbeat, or face the wrath of the Circle!");
		    else {
			Player.cash -= fee;
			total = calcmana();
			while (Player.mana < total) {
			    Player.mana++;
			    dataprint();
			}
			print2 ("Have a sorcerous day, now!");
		    }
		} else
		    print2 ("Be seeing you!");
	    }
	}
    xredraw();
}

void l_order (void)
{
    print1 ("The Headquarters of the Order of Paladins.");
    morewait();
    if (Player.rank[ORDER] == PALADIN && Player.level > Justiciarlevel && gamestatusp(GAVE_STARGEM) && Player.alignment > 300) {
	print1 ("You have succeeded in your quest!");
	morewait();
	foreach (m, Level->mlist)
	    if (m->id == HISCORE_NPC && m->aux2 == NPC_JUSTICIAR)
		--(m = Level->mlist.erase(m));
	print1 ("The previous Justiciar steps down in your favor.");
	print2 ("You are now the Justiciar of Rampart and the Order!");
	morewait();
	clearmsg();
	print1 ("You are awarded a blessed shield of deflection!");
	morewait();
	object newitem = Objects[SHIELD_OF_DEFLECTION];
	newitem.blessing = 9;
	gain_item (newitem);
	morewait();
	Player.rank[ORDER] = JUSTICIAR;
	Player.maxstr += 5;
	Player.str += 5;
	Player.maxpow += 5;
	Player.pow += 5;
    }
    if (Player.alignment < 1) {
	if (Player.rank[ORDER] >= GALLANT) {
	    print1 ("You have been tainted by chaos!");
	    print2 ("You are stripped of your rank in the Order!");
	    morewait();
	    Player.rank[ORDER] = FORMER_PALADIN;
	    send_to_jail();
	} else
	    print1 ("Get thee hence, minion of chaos!");
    } else if (Player.rank[ORDER] == FORMER_PALADIN)
	print1 ("Thee again?  Get thee hence, minion of chaos!");
    else if (Player.rank[ORDER] == NOT_IN_ORDER) {
	if (Player.rank[ARENA] >= TRAINEE)
	    print1 ("We do not accept bloodstained gladiators into our Order.");
	else if (Player.rank[LEGION] >= LEGIONAIRE)
	    print1 ("Go back to your barracks, mercenary!");
	else {
	    print1 ("Dost thou wish to join our Order? [yn] ");
	    if (ynq1() == 'y') {
		print1 ("Justiciar ");
		nprint1 (Justiciar);
		nprint1 (" welcomes you to the Order.");
		print2 ("'Mayest thou always follow the sublime path of Law.'");
		morewait();
		print1 ("You are now a Gallant in the Order.");
		print2 ("You are given a horse and a blessed spear.");
		morewait();
		Player.rank[ORDER] = GALLANT;
		Player.guildxp[ORDER] = 1;
		setgamestatus (MOUNTED);
		object newitem = Objects[WEAPON_SPEAR];
		newitem.blessing = 9;
		newitem.plus = 1;
		learn_object (newitem);
		gain_item (newitem);
	    }
	}
    } else {
	print1 ("'Welcome back, Paladin.'");
	if (!gamestatusp (MOUNTED)) {
	    print2 ("You are given a new steed.");
	    setgamestatus (MOUNTED);
	}
	morewait();
	clearmsg();
	if ((Player.hp < Player.maxhp) || (Player.status[DISEASED]) || (Player.status[POISONED]))
	    print1 ("Your wounds are treated by a medic.");
	cleanse (0);
	Player.hp = Player.maxhp;
	Player.food = 40;
	print2 ("You get a hot meal from the refectory.");
	morewait();
	clearmsg();
	if (Player.rank[ORDER] == PALADIN) {
	    if (Player.level <= Justiciarlevel)
		print2 ("You are not experienced enough to advance.");
	    else if (Player.alignment < 300)
		print2 ("You are not sufficiently Lawful as yet to advance.");
	    else
		print2 ("You must give the Star Gem to the LawBringer.");
	} else if (Player.rank[ORDER] == CHEVALIER) {
	    if (Player.guildxp[ORDER] < 4000)
		print2 ("You are not experienced enough to advance.");
	    else if (Player.alignment < 200)
		print2 ("You are not sufficiently Lawful as yet to advance.");
	    else {
		print1 ("You are made a Paladin of the Order!");
		print2 ("You learn the Spell of Heroism and get Mithril Plate!");
		morewait();
		object newitem = Objects[ARMOR_MITHRIL_PLATE];
		newitem.blessing = 9;
		learn_object (newitem);
		gain_item (newitem);
		morewait();
		clearmsg();
		print1 ("To advance you must rescue the Star Gem and return it");
		print2 ("to its owner, the LawBringer, who resides on Star Peak.");
		morewait();
		print1 ("The Star Gem was stolen by the cursed Prime Sorceror,");
		print2 ("whose headquarters may be found beyond the Astral Plane.");
		morewait();
		print1 ("The Oracle will send you to the Astral Plane if you");
		print2 ("prove yourself worthy to her.");
		morewait();
		learn_spell (S_HERO);
		Player.rank[ORDER] = PALADIN;
	    }
	} else if (Player.rank[ORDER] == GUARDIAN) {
	    if (Player.guildxp[ORDER] < 1500)
		print2 ("You are not experienced enough to advance.");
	    else if (Player.alignment < 125)
		print2 ("You are not yet sufficiently Lawful to advance.");
	    else {
		Player.rank[ORDER] = CHEVALIER;
		print1 ("You are made a Chevalier of the Order!");
		print2 ("You are given a Mace of Disruption!");
		morewait();
		clearmsg();
		learn_object (Objects[WEAPON_MACE_OF_DISRUPTION]);
		gain_item (Objects[WEAPON_MACE_OF_DISRUPTION]);
	    }
	} else if (Player.rank[ORDER] == GALLANT) {
	    if (Player.guildxp[ORDER] < 400)
		print2 ("You are not experienced enough to advance.");
	    else if (Player.alignment < 50)
		print2 ("You are not Lawful enough to advance.");
	    else {
		print1 ("You are made a Guardian of the Order of Paladins!");
		print2 ("You are given a Holy Hand Grenade (of Antioch).");
		morewait();
		print1 ("You hear a nasal monotone in the distance....");
		print2 ("'...and the number of thy counting shall be 3...'");
		morewait();
		clearmsg();
		Player.rank[ORDER] = GUARDIAN;
		learn_object (Objects[HOLY_HAND_GRENADE]);
		gain_item (Objects[HOLY_HAND_GRENADE]);
	    }
	}
    }
}

// loads the house level into Level
void load_house (int kind, int populate)
{
    initrand (Current_Environment, Player.x + Player.y + hour() * 10);
    const char* ld = Level->init_from_data (kind, kind == E_HOUSE ? Level_House : (kind == E_MANSION ? Level_Mansion : Level_Hovel));
    unsigned stops = 0;
    for (unsigned j = 0; j < Level->height; ++j, ++ld) {
	for (unsigned i = 0; i < Level->width; ++i) {
	    if (kind == E_HOVEL)
		Level->site(i,j).lstatus = SEEN;
	    else
		Level->site(i,j).lstatus = 0;
	    Level->site(i,j).roomnumber = RS_CORRIDOR;
	    Level->site(i,j).p_locf = L_NO_OP;
	    char site = *ld++;
	    switch (site) {
		case 'N':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).roomnumber = RS_BEDROOM;
		    if (random_range (2) && populate)
			make_house_npc (i, j);
		    break;
		case 'H':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).roomnumber = RS_BEDROOM;
		    if (random_range (2) && populate)
			make_mansion_npc (i, j);
		    break;
		case 'D':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).roomnumber = RS_DININGROOM;
		    break;
		case '.':
		    Level->site(i,j).locchar = FLOOR;
		    if (stops) {
			lset (i, j, STOPS);
			stops = 0;
		    }
		    break;
		case 'c':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).roomnumber = RS_CLOSET;
		    break;
		case 'G':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).roomnumber = RS_BATHROOM;
		    break;
		case 'B':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).roomnumber = RS_BEDROOM;
		    break;
		case 'K':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).roomnumber = RS_KITCHEN;
		    break;
		case 'S':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).showchar = WALL;
		    lset (i, j, SECRET);
		    Level->site(i,j).roomnumber = RS_SECRETPASSAGE;
		    break;
		case '3':
		    Level->site(i,j).locchar = SAFE;
		    Level->site(i,j).showchar = WALL;
		    lset (i, j, SECRET);
		    Level->site(i,j).p_locf = L_SAFE;
		    break;
		case '^':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = TRAP_BASE + random_range (NUMTRAPS);
		    break;
		case 'P':
		    Level->site(i,j).locchar = PORTCULLIS;
		    Level->site(i,j).p_locf = L_PORTCULLIS;
		    break;
		case 'R':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_RAISE_PORTCULLIS;
		    break;
		case 'p':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_PORTCULLIS;
		    break;
		case 'T':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_PORTCULLIS_TRAP;
		    break;
		case 'X':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_HOUSE_EXIT;
		    stops = 1;
		    break;
		case '#':
		    Level->site(i,j).locchar = WALL;
		    switch (kind) {
			case E_HOVEL:
			    Level->site(i,j).aux = 10;
			    break;
			case E_HOUSE:
			    Level->site(i,j).aux = 50;
			    break;
			case E_MANSION:
			    Level->site(i,j).aux = 150;
			    break;
		    }
		    break;
		case '|':
		    Level->site(i,j).locchar = OPEN_DOOR;
		    Level->site(i,j).roomnumber = RS_CORRIDOR;
		    lset (i, j, STOPS);
		    break;
		case '+':
		    Level->site(i,j).locchar = CLOSED_DOOR;
		    Level->site(i,j).roomnumber = RS_CORRIDOR;
		    Level->site(i,j).aux = LOCKED;
		    lset (i, j, STOPS);
		    break;
		case 'd':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).roomnumber = RS_CORRIDOR;
		    if (populate)
			make_site_monster (i, j, DOBERMAN);
		    break;
		case 'a':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).roomnumber = RS_CORRIDOR;
		    Level->site(i,j).p_locf = L_TRAP_SIREN;
		    break;
		case 'A':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).roomnumber = RS_CORRIDOR;
		    if (populate)
			make_site_monster (i, j, AUTO_MINOR);
		    break;
	    }
	    Level->site(i,j).showchar = ' ';
	}
    }
    initrand (E_RESTORE, 0);
}

// makes a log npc for houses and hovels
static void make_house_npc (int i, int j)
{
    monster& m = make_site_monster (i, j, NPC);
    mprint ("An eerie shiver runs down your spine as you enter....");
    m.click = (Tick + 1) % 50;
    m_status_set (m, HOSTILE);
    if (nighttime())
	m_status_reset (m, AWAKE);
    else
	m_status_set (m, AWAKE);
    if (m.startthing != NO_THING)
	m.pickup (Objects[m.startthing]);
}

// makes a hiscore npc for mansions
static void make_mansion_npc (int i, int j)
{
    monster& m = make_site_monster (i, j, HISCORE_NPC);
    mprint ("You detect signs of life in this house.");
    m.click = (Tick + 1) % 50;
    m_status_set (m, HOSTILE);
    if (nighttime())
	m_status_reset (m, AWAKE);
    else
	m_status_set (m, AWAKE);
}

// loads the village level into Level
void load_village (int villagenum, int populate)
{
    initrand (Current_Environment, villagenum);
    assign_village_function (0, 0, true);
    static const char* _villages[] = {
	Level_Village1, Level_Village2, Level_Village3,
	Level_Village4, Level_Village5, Level_Village6
    };
    const char* ld = Level->init_from_data (E_VILLAGE, _villages[villagenum-1]);
    for (unsigned j = 0; j < Level->height; ++j, ++ld) {
	for (unsigned i = 0; i < Level->width; ++i) {
	    lset (i, j, SEEN);
	    char site = *ld++;
	    Level->site(i,j).p_locf = L_NO_OP;
	    switch (site) {
		case 'f':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate)
			make_food_bin (i, j);
		    break;
		case 'g':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_GRANARY;
		    break;
		case 'h':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate)
			make_site_monster (i, j, HORSE);
		    break;
		case 'S':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_STABLES;
		    break;
		case 'H':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate)
			make_site_monster (i, j, MERCHANT);
		    break;
		case 'C':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_COMMONS;
		    break;
		case 's':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate)
			make_site_monster (i, j, SHEEP);
		    break;
		case 'x':
		    assign_village_function (i, j, false);
		    break;
		case 'X':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_COUNTRYSIDE;
		    break;
		case 'G':
		    Level->site(i,j).locchar = FLOOR;
		    if (populate) {
			monster& m = make_site_monster (i, j, GUARD);
			m.aux1 = i;
			m.aux2 = j;
		    }
		    break;
		case '^':
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_TRAP_SIREN;
		    break;
		case '(':
		    Level->site(i,j).locchar = HEDGE;
		    Level->site(i,j).p_locf = L_HEDGE;
		    break;
		case '~':
		    Level->site(i,j).locchar = WATER;
		    Level->site(i,j).p_locf = L_WATER;
		    break;
		case '+':
		    Level->site(i,j).locchar = WATER;
		    Level->site(i,j).p_locf = L_CHAOS;
		    break;
		case '\'':
		    Level->site(i,j).locchar = HEDGE;
		    Level->site(i,j).p_locf = L_TRIFID;
		    break;
		case '!':
		    special_village_site (i, j, villagenum);
		    break;
		case '#':
		    Level->site(i,j).locchar = WALL;
		    Level->site(i,j).aux = 100;
		    break;
		case '.':
		    Level->site(i,j).locchar = FLOOR;
		    break;
		case '-':
		    Level->site(i,j).locchar = CLOSED_DOOR;
		    break;
		case '1':
		    Level->site(i,j).locchar = STATUE;
		    break;
	    }
	    if (loc_statusp (i, j, SECRET))
		Level->site(i,j).showchar = WALL;
	    else
		Level->site(i,j).showchar = Level->site(i,j).locchar;
	}
    }
    initrand (E_RESTORE, 0);
}

static void make_food_bin (int i, int j)
{
    for (unsigned k = 0; k < 10; k++)
	Level->make_thing (i, j, FOOD_GRAIN);
}

static void assign_village_function (int x, int y, int setup)
{
    static int next = 0;
    static int permutation[24];	// number of x's in village map
    int i, j, k;

    if (setup) {
	next = 0;
	for (i = 0; i < 24; i++)
	    permutation[i] = i;
	for (i = 0; i < 24; i++) {
	    j = permutation[i];
	    k = random_range (24);
	    permutation[i] = permutation[k];
	    permutation[k] = j;
	}
    } else {

	lset (x, y + 1, STOPS);
	lset (x + 1, y, STOPS);
	lset (x - 1, y, STOPS);
	lset (x, y - 1, STOPS);
	lset (x, y, STOPS);

	switch (permutation[next++]) {
	    case 0:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_ARMORER;
		break;
	    case 1:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_HEALER;
		break;
	    case 2:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_TAVERN;
		break;
	    case 3:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_COMMANDANT;
		break;
	    case 4:
		Level->site(x,y).locchar = OPEN_DOOR;
		Level->site(x,y).p_locf = L_CARTOGRAPHER;
		break;
	    default:
		Level->site(x,y).locchar = CLOSED_DOOR;
		if (random_range (2))
		    Level->site(x,y).aux = LOCKED;
		if (random_range (2))
		    Level->site(x,y).p_locf = L_HOVEL;
		else
		    Level->site(x,y).p_locf = L_HOUSE;
		break;
	}
    }
}

static void special_village_site (int i, int j, int villagenum)
{
    if (villagenum == 1) {
	Level->site(i,j).locchar = ALTAR;
	Level->site(i,j).p_locf = L_LAWSTONE;
    }
    if (villagenum == 2) {
	Level->site(i,j).locchar = ALTAR;
	Level->site(i,j).p_locf = L_BALANCESTONE;
    } else if (villagenum == 3) {
	Level->site(i,j).locchar = ALTAR;
	Level->site(i,j).p_locf = L_CHAOSTONE;
    } else if (villagenum == 4) {
	Level->site(i,j).locchar = ALTAR;
	Level->site(i,j).p_locf = L_MINDSTONE;
    } else if (villagenum == 5) {
	Level->site(i,j).locchar = ALTAR;
	Level->site(i,j).p_locf = L_SACRIFICESTONE;
    } else if (villagenum == 6) {
	Level->site(i,j).locchar = ALTAR;
	Level->site(i,j).p_locf = L_VOIDSTONE;
    }
}

// Functions dealing with dungeon and country levels aside from actual
// level structure generation

// monsters for tactical encounters
void make_country_monsters (int terrain)
{
    static const int8_t plains[] = { BUNNY, BUNNY, HORNET, QUAIL, HAWK, DEER, WOLF, LION, BRIGAND, RANDOM };
    static const int8_t forest[] = { BUNNY, QUAIL, HAWK, BADGER, DEER, DEER, WOLF, BEAR, BRIGAND, RANDOM };
    static const int8_t jungle[] = { ANTEATER, PARROT, MAMBA, ANT, ANT, HYENA, HYENA, ELEPHANT, LION, RANDOM };
    static const int8_t river[] = { QUAIL, TROUT, TROUT, MANOWAR, BASS, BASS, CROC, CROC, BRIGAND, RANDOM };
    static const int8_t swamp[] = { BASS, BASS, CROC, CROC, BOGTHING, ANT, ANT, RANDOM, RANDOM, RANDOM };
    static const int8_t desert[] = { HAWK, HAWK, CAMEL, CAMEL, HYENA, HYENA, LION, LION, RANDOM, RANDOM };
    static const int8_t tundra[] = { WOLF, WOLF, BEAR, BEAR, DEER, DEER, RANDOM, RANDOM, RANDOM, RANDOM };
    static const int8_t mountain[] = { BUNNY, SHEEP, WOLF, WOLF, HAWK, HAWK, HAWK, RANDOM, RANDOM, RANDOM };
    const int8_t* monsters = mountain;
    switch (terrain) {
	case PLAINS: monsters = plains; break;
	case FOREST: monsters = forest; break;
	case JUNGLE: monsters = jungle; break;
	case RIVER:  monsters = river;  break;
	case SWAMP:  monsters = swamp;  break;
	case DESERT: monsters = desert; break;
	case TUNDRA: monsters = tundra; break;
    }
    const unsigned nummonsters = 1+random_range(8);
    for (unsigned i = 0; i < nummonsters; i++) {
	monster& m = make_site_monster (random_range(Level->width), random_range(Level->height), monsters[random_range(ArraySize(mountain))]);
	m.sense = Level->width;
	if (m_statusp (m, ONLYSWIM)) {
	    Level->site(m.x,m.y).locchar = WATER;
	    Level->site(m.x,m.y).p_locf = L_WATER;
	    lset (m.x, m.y, CHANGED);
	}
    }
}

// monstertype is more or less Current_Dungeon
// The caves and sewers get harder as you penetrate them; the castle
// is completely random, but also gets harder as it is explored;
// the astral and the volcano just stay hard...
void populate_level (int monstertype)
{
    int nummonsters = (random_range (difficulty() / 3) + 1) * 3 + 8;
    if (monstertype == E_CASTLE)
	nummonsters += 10;
    else if (monstertype == E_ASTRAL)
	nummonsters += 10;
    else if (monstertype == E_VOLCANO)
	nummonsters += 20;

    for (int k = 0; k < nummonsters; k++) {

	int i, j, monsterid = RANDOM;
	findspace (&i, &j);

	switch (monstertype) {
	    case E_CAVES:
		if (Level->depth * 10 + random_range(100) > 150)
		    monsterid = GOBLIN_SHAMAN;
		else if (Level->depth * 10 + random_range(100) > 100)
		    monsterid = GOBLIN_CHIEF;	// Goblin Chieftain
		else if (random_range(100) > 50)
		    monsterid = GOBLIN;
		break;
	    case E_SEWERS:
		if (random_range (3)) {
		    static const uint8_t _sewerMonsters[] = {
			SEWER_RAT, AGGRAVATOR, BLIPPER, NIGHT_GAUNT, NASTY, MURK, CATOBLEPAS, ACID_CLOUD,
			DENEBIAN, CROC, TESLA, SHADOW, BOGTHING, WATER_ELEM, TRITON, ROUS
		    };
		    monsterid = _sewerMonsters[min(ArraySize(_sewerMonsters),(unsigned)random_range(Level->depth+3))];
		}
		break;
	    case E_ASTRAL:
		if (random_range (2)) {	// random astral creatures
		    static const uint8_t _astralMonsters[] = {
			THOUGHTFORM, FUZZY, BAN_SIDHE, GRUE, SHADOW, ASTRAL_VAMP,
			MANABURST, RAKSHASA, ILL_FIEND, MIRRORMAST, ELDER_GRUE, SHADOW_SLAY
		    };
		    monsterid = _astralMonsters[random_range(ArraySize(_astralMonsters))];
		} else if (random_range (2) && (Level->depth == 1))	// plane of earth
		    monsterid = EARTH_ELEM;
		else if (random_range (2) && (Level->depth == 2))	// plane of air
		    monsterid = AIR_ELEM;
		else if (random_range (2) && (Level->depth == 3))	// plane of water
		    monsterid = WATER_ELEM;
		else if (random_range (2) && (Level->depth == 4))	// plane of fire
		    monsterid = FIRE_ELEM;
		else if (random_range (2) && (Level->depth == 5)) {	// deep astral
		    static const uint8_t _deepAstralMonsters[] = {
			NIGHT_GAUNT, SERV_LAW, SERV_CHAOS, FROST_DEMON, OUTER_DEMON, DEMON_SERP,
			ANGEL, INNER_DEMON, FDEMON_L, HIGH_ANGEL, DEMON_PRINCE, ARCHANGEL
		    };
		    monsterid = _deepAstralMonsters[random_range(ArraySize(_deepAstralMonsters))];
		}
		break;
	    case E_VOLCANO:
		if (random_range (2)) {
		    do {
			monsterid = random_range (ML10 - ML4) + ML4;
		    } while (Monsters[monsterid].uniqueness != COMMON);
		} else {
		    static const uint8_t _volcanoMonsters[] = {
			HAUNT, INCUBUS, DRAGONETTE, FROST_DEMON, SPECTRE,
			LAVA_WORM, FIRE_ELEM, LICHE, RAKSHASA, DEMON_SERP,
			NAZGUL, FLAME_DEV, LOATHLY, ZOMBIE, INNER_DEMON,
			BAD_FAIRY, DRAGON, FDEMON_L, SHADOW_SLAY, DEATHSTAR,
			VAMP_LORD, DEMON_PRINCE
		    };
		    monsterid = _volcanoMonsters[random_range(min(unsigned(Level->depth/2+2),ArraySize(_volcanoMonsters)))];
		}
		break;
	    case E_CASTLE:
		if (random_range (4) == 1) {
		    if (difficulty() < 5)	monsterid = ENCHANTOR;
		    else if (difficulty() < 6)	monsterid = NECROMANCER;
		    else if (difficulty() < 8)	monsterid = FIRE_ELEM;
		    else			monsterid = THAUMATURGIST;
		}
		break;
	}
	monster& m = make_site_monster (i, j, monsterid, 0);
	if (m_statusp (m, ONLYSWIM)) {
	    Level->site(i,j).locchar = WATER;
	    Level->site(i,j).p_locf = L_WATER;
	    lset (i, j, CHANGED);
	}
    }
}

// Add a wandering monster possibly
void wandercheck (void)
{
    if (random_range (MaxDungeonLevels) < difficulty()) {
	int x, y;
	findspace (&x, &y);
	make_site_monster (x, y, RANDOM);
    }
}

// call make_creature and place created monster on Level->mlist and Level
monster& make_site_monster (int i, int j, int mid, int wandering, int dlevel)
{
    Level->mlist.push_back();
    monster& m = Level->mlist.back();
    if (mid >= ML0)
	make_creature (m, mid);
    else
	m_create (m, i, j, wandering, difficulty()+dlevel);
    m.x = i;
    m.y = j;
    return (m);
}

// make and return an appropriate monster for the level and depth
// called by populate_level, doesn't actually add to mlist for some reason
// eventually to be more intelligent
static void m_create (monster& m, int x, int y, int kind, unsigned level)
{
    static const uint8_t _ranges[] = { ML1, ML2, ML3, ML4, ML5, ML6, ML7, ML8, ML9, ML10, NUMMONSTERS };
    unsigned monster_range = _ranges[min(level,ArraySize(_ranges)-1)];
    unsigned mid;
    do
	mid = random_range (monster_range);
    while (Monsters[mid].uniqueness != COMMON);
    make_creature (m, mid);

    // no duplicates of unique monsters
    if (kind == WANDERING)
	m_status_set (m, WANDERING);
    m.x = x;
    m.y = y;
}

// make creature # mid, totally random if mid == -1
// make creature allocates space for the creature
static void make_creature (monster& m, int mid)
{
    if (mid == -1)
	mid = random_range (ML9);
    m = Monsters[mid];
    if ((mid == ANGEL) || (mid == HIGH_ANGEL) || (mid == ARCHANGEL)) {
	// aux1 field of an angel is its deity
	if (Current_Environment == E_TEMPLE)
	    m.aux1 = Country->site(LastCountryLocX,LastCountryLocY).aux;
	else
	    m.aux1 = random_range (6) + 1;
	strcpy (Str3, Monsters[mid].monstring);
	switch (m.aux1) {
	    case ODIN:
		strcat (Str3, " of Odin");
		break;
	    case SET:
		strcat (Str3, " of Set");
		break;
	    case HECATE:
		strcat (Str3, " of Hecate");
		break;
	    case ATHENA:
		strcat (Str3, " of Athena");
		break;
	    case DESTINY:
		strcat (Str3, " of Destiny");
		break;
	    case DRUID:
		strcat (Str3, " of the Balance");
		break;
	}
	m.monstring = strdup (Str3);
    } else if (mid == ZERO_NPC || mid == WEREHUMAN) {
	// generic 0th level human, or a were-human
	m.monstring = mantype();
	strcpy (Str1, "dead ");
	strcat (Str1, m.monstring);
	m.corpsestr = strdup (Str1);
    } else if ((m.monchar & 0xff) == '!') {
	// the nymph/satyr and incubus/succubus
	if (Player.preference == 'f' || (Player.preference != 'm' && random_range (2))) {
	    m.monchar = 'n' | CLR_RED_BLACK;
	    m.monstring = "nymph";
	    m.corpsestr = "dead nymph";
	} else {
	    m.monchar = 's' | CLR_RED_BLACK;
	    m.monstring = "satyr";
	    m.corpsestr = "dead satyr";
	}
	if (m.id == INCUBUS) {
	    if ((m.monchar & 0xff) == 'n')
		m.corpsestr = "dead succubus";
	    else
		m.corpsestr = "dead incubus";
	}
    }
    if (mid == NPC)
	make_log_npc (m);
    else if (mid == HISCORE_NPC)
	make_hiscore_npc (m, random_range (NPC_MAX));
    else {
	if (m.sleep < random_range (100))
	    m_status_set (m, AWAKE);
	if (m.startthing != NO_THING && object_uniqueness(m.startthing) <= UNIQUE_MADE)
	    m.pickup (Objects[m.startthing]);
	for (unsigned treasures = random_range (m.treasure); m.possessions.size() < treasures;) {
	    object o = create_object (m.level);
	    if (object_uniqueness(o) != COMMON)
		set_object_uniqueness (o, UNIQUE_UNMADE);
	    else
		m.pickup (o);
	}
    }
    m.click = (Tick + 1) % 50;
}

// makes one of the highscore npcs
void make_hiscore_npc (monster& npc, int npcid)
{
    int st = -1;
    int x = npc.x, y = npc.y;
    npc = Monsters[HISCORE_NPC];
    npc.aux2 = npcid;
    npc.x = x;
    npc.y = y;
    // each of the high score npcs can be created here
    switch (npcid) {
	default:
	case NPC_HIGHSCORE:
	    npc.monstring = Hiscorer;
	    determine_npc_behavior (npc, Hilevel, Hibehavior);
	    break;
	case NPC_HIGHPRIEST_ODIN:
	case NPC_HIGHPRIEST_SET:
	case NPC_HIGHPRIEST_ATHENA:
	case NPC_HIGHPRIEST_HECATE:
	case NPC_HIGHPRIEST_DRUID:
	case NPC_HIGHPRIEST_DESTINY:
	    npc.monstring = Priest[npcid];
	    determine_npc_behavior (npc, Priestlevel[npcid], Priestbehavior[npcid]);
	    st = HOLY_SYMBOL_OF_ODIN-1 + npcid;	// appropriate holy symbol...
	    set_object_uniqueness (st, UNIQUE_MADE);
	    if (npcid == DRUID)
		npc.talkf = M_TALK_DRUID;
	    if (Player.patron == npcid)
		m_status_reset (npc, HOSTILE);
	    break;
	case NPC_SHADOWLORD:
	    npc.monstring = Shadowlord;
	    determine_npc_behavior (npc, Shadowlordlevel, Shadowlordbehavior);
	    break;
	case NPC_COMMANDANT:
	    npc.monstring = Commandant;
	    determine_npc_behavior (npc, Commandantlevel, Commandantbehavior);
	    if (Player.rank[LEGION])
		m_status_reset (npc, HOSTILE);
	    break;
	case NPC_ARCHMAGE:
	    npc.monstring = Archmage;
	    determine_npc_behavior (npc, Archmagelevel, Archmagebehavior);
	    st = KEY_OF_KOLWYNIA;
	    npc.talkf = M_TALK_ARCHMAGE;
	    m_status_reset (npc, WANDERING);
	    m_status_reset (npc, HOSTILE);
	    break;
	case NPC_PRIME:
	    npc.monstring = Prime;
	    determine_npc_behavior (npc, Primelevel, Primebehavior);
	    npc.talkf = M_TALK_PRIME;
	    npc.specialf = M_SP_PRIME;
	    if (Player.alignment < 0)
		m_status_reset (npc, HOSTILE);
	    break;
	case NPC_CHAMPION:
	    npc.monstring = Champion;
	    determine_npc_behavior (npc, Championlevel, Championbehavior);
	    if (Player.rank[ARENA])
		m_status_reset (npc, HOSTILE);
	    break;
	case NPC_DUKE:
	    npc.monstring = Duke;
	    determine_npc_behavior (npc, Dukelevel, Dukebehavior);
	    break;
	case NPC_LORD_OF_CHAOS:
	    npc.monstring = Chaoslord;
	    determine_npc_behavior (npc, Chaoslordlevel, Chaoslordbehavior);
	    if (Player.alignment < 0 && random_range (2))
		m_status_reset (npc, HOSTILE);
	    break;
	case NPC_LORD_OF_LAW:
	    npc.monstring = Lawlord;
	    determine_npc_behavior (npc, Lawlordlevel, Lawlordbehavior);
	    if (Player.alignment > 0)
		m_status_reset (npc, HOSTILE);
	    break;
	case NPC_JUSTICIAR:
	    npc.monstring = Justiciar;
	    determine_npc_behavior (npc, Justiciarlevel, Justiciarbehavior);
	    st = THING_JUSTICIAR_BADGE;
	    npc.talkf = M_TALK_GUARD;
	    npc.specialf = M_SP_WHISTLEBLOWER;
	    m_status_reset (npc, WANDERING);
	    m_status_reset (npc, HOSTILE);
	    break;
    }
    if (st > -1 && object_uniqueness(st) == UNIQUE_MADE)
	npc.pickup (Objects[st]);
    char buf[80];
    strcpy (buf, "The body of ");
    strcat (buf, npc.monstring);
    npc.corpsestr = strdup (buf);
}

// sets npc behavior given level and behavior code
static void determine_npc_behavior (monster& npc, int level, int behavior)
{
    npc.hp = (level + 1) * 20;
    npc.status = AWAKE + MOBILE + WANDERING;
    int combatype = (behavior % 100) / 10;
    int competence = (behavior % 1000) / 100;
    int talktype = behavior / 1000;
    npc.level = competence;
    if (npc.level < 2 * difficulty())
	npc.status += HOSTILE;
    npc.xpv = npc.level * 20;
    switch (combatype) {
	case 1:		// melee
	    npc.meleef = M_MELEE_NORMAL;
	    npc.dmg = competence * 5;
	    npc.hit = competence * 3;
	    npc.speed = 3;
	    break;
	case 2:		// missile
	    npc.meleef = M_MELEE_NORMAL;
	    npc.strikef = M_STRIKE_MISSILE;
	    npc.dmg = competence * 3;
	    npc.hit = competence * 2;
	    npc.speed = 4;
	    break;
	case 3:		// spellcasting
	    npc.meleef = M_MELEE_NORMAL;
	    npc.dmg = competence;
	    npc.hit = competence;
	    npc.specialf = M_SP_SPELL;
	    npc.speed = 6;
	    break;
	case 4:		// thievery
	    npc.meleef = M_MELEE_NORMAL;
	    npc.dmg = competence;
	    npc.hit = competence;
	    npc.specialf = M_SP_THIEF;
	    npc.speed = 3;
	    break;
	case 5:		// flee
	    npc.dmg = competence;
	    npc.hit = competence;
	    npc.meleef = M_MELEE_NORMAL;
	    npc.specialf = M_MOVE_SCAREDY;
	    npc.speed = 3;
	    break;
    }
    if (npc.talkf == M_TALK_MAN) {
	switch (talktype) {
	    case 1: npc.talkf = M_TALK_EVIL; break;
	    case 2: npc.talkf = M_TALK_MAN; break;
	    case 3: npc.talkf = M_TALK_HINT; break;
	    case 4: npc.talkf = M_TALK_BEG; break;
	    default:
	    case 5: npc.talkf = M_TALK_SILENT; break;
	}
    }
    npc.uniqueness = UNIQUE_MADE;
}

// makes an ordinary npc (maybe undead)
static void make_log_npc (monster& npc)
{
    int level = random_range(16);
    npc.hp = level * 20;
    uint8_t ghostid = LICHE;
    if (level < 3)
	ghostid = GHOST;
    else if (level < 7)
	ghostid = HAUNT;
    else if (level < 12)
	ghostid = SPECTRE;
    int x = npc.x, y = npc.y;
    npc = Monsters[ghostid];
    npc.x = x; npc.y = y;
    determine_npc_behavior (npc, level, 2718);
}

// drop treasures randomly onto level
void stock_level (void)
{
    // put cash anywhere, including walls, put other treasures only on floor
    const unsigned numtreasures = 2 * random_range (difficulty() / 4) + 4;
    for (unsigned i, j, k = 0; k < numtreasures + 10; k++) {
	do {
	    i = random_range (Level->width);
	    j = random_range (Level->height);
	} while (Level->site(i,j).locchar != FLOOR);
	make_site_treasure (i, j, difficulty());
	// caves have more random cash strewn around
	const unsigned cashFactor = (Current_Dungeon == E_CAVES ? 3 : 1);
	for (unsigned l = 0; l < cashFactor; ++l) {
	    i = random_range (Level->width);
	    j = random_range (Level->height);
	    Level->add_thing (i, j, make_cash (difficulty()));
	}
    }
}

// make a new object (of at most level itemlevel) at site i,j on level
static void make_site_treasure (int i, int j, int itemlevel)
{
    Level->add_thing (i, j, create_object(itemlevel));
}

// make a specific new object at site i,j on level
static void make_specific_treasure (int i, int j, int iid)
{
    if (object_uniqueness(iid) == UNIQUE_TAKEN)
	return;
    Level->make_thing (i, j, iid);
}

// returns a "level of difficulty" based on current environment
// and depth in dungeon. Is somewhat arbitrary. value between 1 and 10.
// May not actually represent real difficulty, but instead level
// of items, monsters encountered.
int difficulty (void)
{
    const unsigned depth = Level ? Level->depth : 1;
    switch (Current_Environment) {
	case E_COUNTRYSIDE:	return (7);
	case E_CITY:		return (3);
	case E_VILLAGE:		return (1);
	case E_TACTICAL_MAP:	return (4);
	case E_SEWERS:		return (depth / 6) + 3;
	case E_CASTLE:		return (depth / 4) + 4;
	case E_CAVES:		return (depth / 3) + 1;
	case E_VOLCANO:		return (depth / 4) + 5;
	case E_ASTRAL:		return (8);
	case E_ARENA:		return (5);
	case E_HOVEL:		return (3);
	case E_MANSION:		return (7);
	case E_HOUSE:		return (5);
	case E_DLAIR:		return (9);
	case E_ABYSS:		return (10);
	case E_STARPEAK:	return (9);
	case E_CIRCLE:		return (8);
	case E_MAGIC_ISLE:	return (8);
	case E_TEMPLE:		return (8);
	default:		return (3);
    }
}

// the bank; can be broken into (!)
void l_bank (void)
{
    int done = false, valid = false;
    long amount;
    char response;
    char passwd[64];
    print1 ("First Bank of Omega: Autoteller Carrel.");

    if (gamestatusp (BANK_BROKEN))
	print2 ("You see a damaged autoteller.");
    else {
	print2 ("The proximity sensor activates the autoteller as you approach.");
	morewait();
	clearmsg();
	while (!done) {
	    print1 ("Current Balance: ");
	    mlongprint (Balance);
	    nprint1 ("Au. ");
	    nprint1 (" Enter command (? for help) > ");
	    response = mgetc();
	    if (response == '?') {
		menuclear();
		menuprint ("?: This List.\n");
		if (strcmp (Password, "") == 0)
		    menuprint ("O: Open an account.\n");
		else {
		    menuprint ("P: Enter password.\n");
		    menuprint ("D: Deposit.\n");
		    menuprint ("W: Withdraw\n");
		}
		menuprint ("X: eXit\n");
		showmenu();
		morewait();
		xredraw();
		continue;
	    } else if ((response == 'P') && (strcmp (Password, "") != 0)) {
		clearmsg();
		print1 ("Password: ");
		strcpy (passwd, msgscanstring());
		valid = (strcmp (passwd, Password) == 0);
		if (!valid) {
		    done = true;
		    menuclear();
		    menuprint ("Alert! Alert! Invalid Password!\n");
		    menuprint ("The police are being summoned!\n");
		    menuprint ("Please wait for the police to arrive....\n\n");
		    menuprint ("----Hit space bar to continue----\n");
		    showmenu();
		    response = menugetc();
		    if (response == ' ') {
			Player.alignment += 5;
			xredraw();
			print1 ("Ah ha! Trying to rob the bank, eh?");
			print2 ("Take him away, boys!");
			morewait();
			send_to_jail();
		    } else {
			Player.alignment -= 5;
			menuclear();
			napms (1000);
			menuprint ("^@^@^@^@^@00AD1203BC0F0000FFFFFFFFFFFF\n");
			menuprint ("Interrupt in _get_space. Illegal Character.\n");
			showmenu();
			napms (3000);
			menuprint ("Aborting _police_alert.....\n");
			menuprint ("Attempting reboot.....\n");
			showmenu();
			napms (3000);
			menuprint ("Warning: Illegal shmop at _count_cash.\n");
			menuprint ("Warning: Command Buffer NOT CLEARED\n");
			showmenu();
			napms (3000);
			menuprint ("Reboot Complete. Execution Continuing.\n");
			menuprint ("Withdrawing: 4294967297 Au.\n");
			menuprint ("Warning: Arithmetic Overflow in _withdraw\n");
			showmenu();
			napms (3000);
			menuprint ("Yo mama. Core dumped.\n");
			showmenu();
			napms (3000);
			xredraw();
			clearmsg();
			print1 ("The cash machine begins to spew gold pieces!");
			print2 ("You pick up your entire balance and then some!");
			Player.cash += Balance + 1000 + random_range (3000);
			Balance = 0;
			setgamestatus (BANK_BROKEN);
		    }
		} else
		    print2 ("Password accepted. Working.");
	    } else if ((response == 'D') && valid) {
		clearmsg();
		print1 ("Amount: ");
		amount = get_money (Player.cash);
		if (amount < 1)
		    print3 ("Transaction aborted.");
		else if (amount > Player.cash)
		    print3 ("Deposit too large -- transaction aborted.");
		else {
		    print2 ("Transaction accomplished.");
		    Balance += amount;
		    Player.cash -= amount;
		}
	    } else if ((response == 'W') && valid) {
		clearmsg();
		print1 ("Amount: ");
		amount = get_money (Balance);
		if (amount < 1)
		    print3 ("Transaction aborted.");
		else if (amount > Balance)
		    print3 ("Withdrawal too large -- transaction aborted.");
		else {
		    print2 ("Transaction accomplished.");
		    Balance -= amount;
		    Player.cash += amount;
		}
	    } else if (response == 'X') {
		clearmsg();
		print1 ("Bye!");
		done = true;
	    } else if ((response == 'O') && (strcmp (Password, "") == 0)) {
		clearmsg();
		print1 ("Opening new account.");
		nprint1 (" Please enter new password: ");
		strcpy (Password, msgscanstring());
		if (strcmp (Password, "") == 0) {
		    print3 ("Illegal to use null password -- aborted.");
		    done = true;
		} else {
		    print2 ("Password validated; account saved.");
		    valid = true;
		}
	    } else
		print3 (" Illegal command.");
	    dataprint();
	}
    }
    xredraw();
}

void l_armorer (void)
{
    int done = false;
    char action;
    if (hour() == 12)
	print3 ("Unfortunately, this is Julie's lunch hour -- try again later.");
    else if (nighttime())
	print3 ("It seems that Julie keeps regular business hours.");
    else {
	while (!done) {
	    clearmsg();
	    print1 ("Julie's: Buy Armor, Weapons, or Leave [a,w,ESCAPE] ");
	    action = mgetc();
	    if (action == KEY_ESCAPE)
		done = true;
	    else if (action == 'a')
		buyfromstock (ARMORID, 10);
	    else if (action == 'w')
		buyfromstock (WEAPONID, 23);
	}
    }
    xredraw();
}

static void buyfromstock (int base, int numitems)
{
    print2 ("Purchase which item? [ESCAPE to quit] ");
    menuclear();
    for (int i = 0; i < numitems; i++) {
	strcpy (Str4, " :");
	Str4[0] = i + 'a';
	strcat (Str4, Objects[base + i].objstr);
	menuprint (Str4);
	menuprint ("\n");
    }
    showmenu();
    char item = ' ';
    while (item != KEY_ESCAPE && (item < 'a' || item >= 'a' + numitems))
	item = mgetc();
    if (item != KEY_ESCAPE) {
	int i = item - 'a';
	const object& newitem = Objects[base + i];
	clearmsg();
	print1 ("I can let you have it for ");
	unsigned cost = 2 * true_item_value (newitem);
	mlongprint (cost);
	nprint1 ("Au. Buy it? [yn] ");
	if (ynq1() == 'y') {
	    if (Player.cash < cost)
		print2 ("Why not try again some time you have the cash?");
	    else {
		Player.cash -= cost;
		dataprint();
		gain_item (newitem);
	    }
	}
    }
}

void l_club (void)
{
    char response;

    print1 ("Rampart Explorers' Club.");
    if (!gamestatusp (CLUB_MEMBER)) {
	if (Player.level < 2)
	    print3 ("Only reknowned adventurers need apply.");
	else {
	    print2 ("Dues are 100Au. Pay it? [yn] ");
	    if (ynq2() == 'y') {
		if (Player.cash < 100)
		    print3 ("Beat it, or we'll blackball you!");
		else {
		    print1 ("Welcome to the club! You are taught the spell of Return.");
		    print2 ("When cast on the first level of a dungeon it");
		    morewait();
		    clearmsg();
		    print1 ("will transport you down to the lowest level");
		    print2 ("you have explored, and vice versa.");
		    learn_spell (S_RETURN);
		    Player.cash -= 100;
		    setgamestatus (CLUB_MEMBER);
		}
	    } else
		print2 ("OK, but you're missing out on our benefits....");
	}
    } else {
	print2 ("Shop at the club store or listen for rumors [sl] ");
	do
	    response = (char) mcigetc();
	while ((response != 's') && (response != 'l') && (response != KEY_ESCAPE));
	if (response == 'l') {
	    if (club_hinthour == hour())
		print2 ("You don't hear anything useful.");
	    else {
		print1 ("You overhear a conversation....");
		hint();
		club_hinthour = hour();
	    }
	} else if (response == 's') {
	    buyfromstock (THING_KEY, 2);
	    xredraw();
	} else if (response == KEY_ESCAPE)
	    print2 ("Be seeing you, old chap!");
    }
}

void l_gym (void)
{
    int done = true;
    int trained = 0;
    clearmsg();
    do {
	print1 ("The Rampart Gymnasium");
	if ((Gymcredit > 0) || (Player.rank[ARENA])) {
	    nprint1 ("-- Credit: ");
	    mlongprint (Gymcredit);
	    nprint1 ("Au.");
	}
	done = false;
	menuclear();
	menuprint ("Train for 2000 Au. Choose:\n");
	menuprint ("\na: work out in the weight room");
	menuprint ("\nb: use our gymnastics equipment");
	menuprint ("\nc: take our new anaerobics course");
	menuprint ("\nd: enroll in dance lessons.");
	menuprint ("\nESCAPE: Leave this place.");
	showmenu();
	switch (mgetc()) {
	    case 'a':
		gymtrain (&(Player.maxstr), &(Player.str));
		break;
	    case 'b':
		gymtrain (&(Player.maxdex), &(Player.dex));
		break;
	    case 'c':
		gymtrain (&(Player.maxcon), &(Player.con));
		break;
	    case 'd':
		gymtrain (&(Player.maxagi), &(Player.agi));
		break;
	    case KEY_ESCAPE:
		clearmsg();
		if (trained == 0)
		    print1 ("Well, it's your body you're depriving!");
		else if (trained < 3)
		    print1 ("You towel yourself off, and find the exit.");
		else
		    print1 ("A refreshing bath, and you're on your way.");
		done = true;
		break;
	    default:
		trained--;
		break;
	}
	trained++;
    } while (!done);
    xredraw();
    calc_melee();
}

void l_healer (void)
{
    print1 ("Rampart Healers. Member RMA.");
    morewait();
    clearmsg();
    print1 ("a: Heal injuries (50 crowns)");
    print2 ("b: Cure disease (250 crowns)");
    print3 ("ESCAPE: Leave these antiseptic alcoves.");
    switch ((char) mcigetc()) {
	case 'a':
	    healforpay();
	    break;
	case 'b':
	    cureforpay();
	    break;
	default:
	    print3 ("OK, but suppose you have Acute Satyriasis?");
	    break;
    }
}

void statue_random (int x, int y)
{
    switch (random_range (difficulty() + 3) - 1) {
	default:
	    l_statue_wake();
	    break;
	case 0:
	    print1 ("The statue crumbles with a clatter of gravel.");
	    Level->site(x,y).locchar = RUBBLE;
	    Level->site(x,y).p_locf = L_RUBBLE;
	    plotspot (x, y, true);
	    lset (x, y, CHANGED);
	    break;
	case 1:
	    print1 ("The statue stoutly resists your attack.");
	    break;
	case 2:
	    print1 ("The statue crumbles with a clatter of gravel.");
	    Level->site(x,y).locchar = RUBBLE;
	    Level->site(x,y).p_locf = L_RUBBLE;
	    plotspot (x, y, true);
	    lset (x, y, CHANGED);
	    make_site_treasure (x, y, difficulty());
	    break;
	case 3:
	    print1 ("The statue hits you back!");
	    p_damage (random_range (difficulty() * 5), UNSTOPPABLE, "a statue");
	    break;
	case 4:
	    print1 ("The statue looks slightly pained. It speaks:");
	    morewait();
	    clearmsg();
	    hint();
	    break;
	case 5:
	    if ((Current_Environment == Current_Dungeon) || (Current_Environment == E_CITY)) {
		print1 ("You hear the whirr of some mechanism.");
		print2 ("The statue glides smoothly into the floor!");
		// WDT HACK: I shouldn't be making this choice on a level
		// where no stairs can be (or perhaps I should, and I should
		// implement a bonus level!).
		Level->site(x,y).locchar = STAIRS_DOWN;
		Level->site(x,y).p_locf = L_NO_OP;
		lset (x, y, CHANGED | STOPS);
	    }
	    break;
	case 6:
	    print1 ("The statue was covered with contact cement!");
	    print2 ("You can't move....");
	    Player.status[IMMOBILE] += random_range (6) + 2;
	    break;
	case 7:
	    print1 ("A strange radiation emanates from the statue!");
	    dispel (-1);
	    break;
	case 8:		// I think this is particularly evil. Heh heh.
	    if (Player.has_possession(O_WEAPON_HAND)) {
		print1 ("Your weapon sinks deeply into the statue and is sucked away!");
		Player.possessions[O_WEAPON_HAND].blessing = -1 - absv (Player.possessions[O_WEAPON_HAND].blessing);
		drop_at (x, y, Player.possessions[O_WEAPON_HAND]);
		Player.remove_possession(O_WEAPON_HAND);
	    }
	    break;
	case 9:
	    print1 ("The statue extends an arm. Beams of light illuminate the level!");
	    for (unsigned i = 0; i < Level->width; i++) {
		for (unsigned j = 0; j < Level->height; j++) {
		    lset (i, j, SEEN);
		    if (loc_statusp (i, j, SECRET)) {
			lreset (i, j, SECRET);
			lset (i, j, CHANGED);
		    }
		}
	    }
	    show_screen();
	    break;
    }
}

void l_statue_wake (void)
{
    int i;
    int x = Player.x, y = Player.y;
    for (i = 0; i < 9; i++)
	wake_statue (x + Dirs[0][i], y + Dirs[1][i], true);
}

static void wake_statue (int x, int y, int first)
{
    if (Level->site(x,y).locchar == STATUE) {
	if (!first)
	    mprint ("Another statue awakens!");
	else
	    mprint ("A statue springs to life!");
	Level->site(x,y).locchar = FLOOR;
	lset (x, y, CHANGED);
	monster& m = make_site_monster (x, y, 0, 1);
	m_status_set (m, HOSTILE);
	for (int i = 0; i < 8; i++)
	    wake_statue (x + Dirs[0][i], y + Dirs[1][i], false);
    }
}

void l_casino (void)
{
    int i, done = false, a, b, c, match;
    char response;
    print1 ("Rampart Mithril Nugget Casino.");
    if (random_range (10) == 1)
	print2 ("Casino closed due to Grand Jury investigation.");
    else {
	while (!done) {
	    morewait();
	    clearmsg();
	    print1 ("a: Drop 100Au in the slots.");
	    print2 ("b: Risk 1000Au  at roulette.");
	    print3 ("ESCAPE: Leave this green baize hall.");
	    response = (char) mcigetc();
	    if (response == 'a') {
		if (Player.cash < 100)
		    print3 ("No credit, jerk.");
		else {
		    Player.cash -= 100;
		    dataprint();
		    for (i = 0; i < 20; i++) {
			napms (i == 19 ? 1000 : 250);
			a = random_range (10);
			b = random_range (10);
			c = random_range (10);
			clearmsg1();
			mprint (slotstr (a));
			mprint (slotstr (b));
			mprint (slotstr (c));
		    }
		    if (winnings > 0)
			do {
			    a = random_range (10);
			    b = random_range (10);
			    c = random_range (10);
			} while ((a == b) || (a == c) || (b == c));
		    else {
			a = random_range (10);
			b = random_range (10);
			c = random_range (10);
		    }
		    clearmsg();
		    mprint (slotstr (a));
		    mprint (slotstr (b));
		    mprint (slotstr (c));
		    if ((a == b) && (a == c)) {
			print3 ("Jackpot Winner!");
			winnings += (a + 2) * (b + 2) * (c + 2) * 5;
			Player.cash += (a + 2) * (b + 2) * (c + 2) * 5;
			dataprint();
		    } else if (a == b) {
			print3 ("Winner!");
			Player.cash += (a + 2) * (b + 2) * 5;
			dataprint();
			winnings += (a + 2) * (b + 2) * 5;
		    } else if (a == c) {
			print3 ("Winner!");
			Player.cash += (a + 2) * (c + 2) * 5;
			dataprint();
			winnings += (a + 2) * (c + 2) * 5;
		    } else if (c == b) {
			print3 ("Winner!");
			Player.cash += (c + 2) * (b + 2) * 5;
			dataprint();
			winnings += (c + 2) * (b + 2) * 5;
		    } else {
			print3 ("Loser!");
			winnings -= 100;
		    }
		}
	    } else if (response == 'b') {
		if (Player.cash < 1000)
		    mprint ("No credit, jerk.");
		else {
		    Player.cash -= 1000;
		    dataprint();
		    print1 ("Red or Black? [rb]");
		    do
			response = (char) mcigetc();
		    while ((response != 'r') && (response != 'b'));
		    match = (response == 'r' ? 0 : 1);
		    for (i = 0; i < 20; i++) {
			napms (i == 19 ? 1000 : 250);
			a = random_range (37);
			b = a % 2;
			if (a == 0)
			    print1 (" 0 ");
			else if (a == 1)
			    print1 (" 0 - 0 ");
			else {
			    print1 ((b == 0) ? "Red " : "Black ");
			    mnumprint (a - 1);
			}
		    }
		    if (winnings > 0)
			do {
			    a = random_range (37);
			    b = a % 2;
			} while (b == match);
		    else {
			a = random_range (37);
			b = a % 2;
		    }
		    if (a == 0)
			print1 (" 0 ");
		    else if (a == 1)
			print1 (" 0 - 0 ");
		    else {
			print1 ((b == 0) ? "Red " : "Black ");
			mnumprint (a - 1);
		    }
		    if ((a > 1) && (b == match)) {
			print3 (" Winner!");
			winnings += 1000;
			Player.cash += 2000;
			dataprint();
		    } else {
			print3 (" Loser!");
			winnings -= 1000;
			dataprint();
		    }
		}
	    } else if (response == KEY_ESCAPE)
		done = true;
	}
    }
}

void l_commandant (void)
{
    print1 ("Commandant Sonder's Rampart-fried Lyzzard partes. Open 24 hrs.");
    print2 ("Buy a bucket! Only 5 Au. Make a purchase? [yn] ");
    if (ynq2() == 'y') {
	clearmsg();
	print1 ("How many? ");
	int num = (int) parsenum();
	if (num < 1)
	    print3 ("Cute. Real cute.");
	else if (num * 5U > Player.cash)
	    print3 ("No handouts here, mac!");
	else {
	    Player.cash -= num * 5;
	    object food = Objects[FOOD_RATION];
	    food.number = num;
	    if (num == 1)
		print2 ("There you go, mac! One Lyzzard Bucket, coming up.");
	    else
		print2 ("A passel of Lyzzard Buckets, for your pleasure.");
	    gain_item (food);
	    morewait();
	}
    } else
	print2 ("Don't blame the Commandant if you starve!");
}

void l_diner (void)
{
    print1 ("The Rampart Diner. All you can eat, 25Au.");
    print2 ("Place an order? [yn] ");
    if (ynq2() == 'y') {
	if (Player.cash < 25)
	    mprint ("TANSTAAFL! Now git!");
	else {
	    Player.cash -= 25;
	    dataprint();
	    Player.food = 44;
	    foodcheck();
	}
    }
}

void l_crap (void)
{
    print1 ("Les Crapeuleaux. (****) ");
    if ((hour() < 17) || (hour() > 23))
	print2 ("So sorry, we are closed 'til the morrow...");
    else {
	print2 ("May I take your order? [yn] ");
	if (ynq2() == 'y') {
	    if (Player.cash < 1000)
		print2 ("So sorry, you have not the funds for dinner.");
	    else {
		print2 ("Hope you enjoyed your tres expensive meal, m'sieur...");
		Player.cash -= 1000;
		dataprint();
		Player.food += 8;
		foodcheck();
	    }
	}
    }
}

void l_tavern (void)
{
    char response;
    print1 ("The Centaur and Nymph -- J. Riley, prop.");
    if (nighttime()) {
	menuclear();
	menuprint ("Riley says: Whataya have?\n\n");
	menuprint ("a: Pint of Riley's ultra-dark 1Au\n");
	menuprint ("b: Shot of Tullimore Dew 10Au\n");
	menuprint ("c: Round for the House. 100Au\n");
	menuprint ("d: Bed and Breakfast. 25Au\n");
	menuprint ("ESCAPE: Leave this comfortable haven.\n");
	showmenu();
	do
	    response = (char) mcigetc();
	while ((response != 'a') && (response != 'b') && (response != 'c') && (response != 'd') && (response != KEY_ESCAPE));
	switch (response) {
	    case 'a':
		if (Player.cash < 1)
		    print2 ("Aw hell, have one on me.");
		else {
		    Player.cash -= 1;
		    dataprint();
		    if (tavern_hinthour != hour()) {
			if (random_range (3)) {
			    print1 ("You overhear a rumor...");
			    hint();
			} else
			    print1 ("You don't hear much of interest.");
			tavern_hinthour = hour();
		    } else
			print1 ("You just hear the same conversations again.");
		}
		break;
	    case 'b':
		if (Player.cash < 10)
		    print2 ("I don't serve the Dew on no tab, buddy!");
		else {
		    Player.cash -= 10;
		    print1 ("Ahhhhh....");
		    if (Player.status[POISONED] || Player.status[DISEASED])
			print2 ("Phew! That's, er, smooth stuff!");
		    Player.status[POISONED] = 0;
		    Player.status[DISEASED] = 0;
		    showflags();
		}
		break;
	    case 'c':
		if (Player.cash < 100) {
		    print1 ("Whatta feeb!");
		    print2 ("Outta my establishment.... Now!");
		    p_damage (random_range (20), UNSTOPPABLE, "Riley's right cross");
		    morewait();
		} else {
		    Player.cash -= 100;
		    dataprint();
		    print1 ("'What a guy!'");
		    morewait();
		    print2 ("'Hey, thanks, fella.'");
		    morewait();
		    print3 ("'Make mine a double...'");
		    morewait();
		    clearmsg();
		    switch (random_range (4)) {
			case 0:
			    print1 ("'You're a real pal. Say, have you heard.... ");
			    hint();
			    break;
			case 1:
			    print1 ("A wandering priest of Dionysus blesses you...");
			    if ((Player.patron == ODIN) || (Player.patron == ATHENA))
				Player.alignment++;
			    else if ((Player.patron == HECATE) || (Player.patron == SET))
				Player.alignment--;
			    else if (Player.alignment > 0)
				Player.alignment--;
			    else
				Player.alignment++;
			    break;
			case 2:
			    print1 ("A thirsty bard promises to put your name in a song!");
			    gain_experience (20);
			    break;
			case 3:
			    print1 ("Riley draws you a shot of his 'special reserve'");
			    print2 ("Drink it [yn]?");
			    if (ynq2() == 'y') {
				if (Player.con < random_range (20)) {
				    print1 ("<cough> Quite a kick!");
				    print2 ("You feel a fiery warmth in your tummy....");
				    Player.con++;
				    Player.maxcon++;
				} else
				    print2 ("You toss it back nonchalantly.");
			    }
		    }
		}
		break;
	    case 'd':
		if (Player.cash < 25)
		    print2 ("Pay in advance, mac!");
		else {
		    Player.cash -= 25;
		    print2 ("How about a shot o' the dew for a nightcap?");
		    morewait();
		    Time += (6 + random_range (4)) * 60;
		    Player.status[POISONED] = 0;
		    Player.status[DISEASED] = 0;
		    Player.food = 40;
		    // reduce temporary stat gains to max stat levels
		    toggle_item_use (true);
		    Player.str = min (Player.str, Player.maxstr);
		    Player.con = min (Player.con, Player.maxcon);
		    Player.agi = min (Player.agi, Player.maxagi);
		    Player.dex = min (Player.dex, Player.maxdex);
		    Player.iq = min (Player.iq, Player.maxiq);
		    Player.pow = min (Player.pow, Player.maxpow);
		    toggle_item_use (false);
		    timeprint();
		    dataprint();
		    showflags();
		    print1 ("The next day.....");
		    if (hour() > 10)
			print2 ("Oh my! You overslept!");
		}
		break;
	    default:
		print2 ("So? Just looking? Go on!");
		break;
	}
    } else
	print2 ("The pub don't open til dark, fella.");
    xredraw();
}

void l_alchemist (void)
{
    print1 ("Ambrosias' Potions et cie.");
    if (nighttime()) {
	print2 ("Ambrosias doesn't seem to be in right now.");
	return;
    }
    while (true) {
	morewait();
	clearmsg();
	print1 ("a: Sell monster components.");
	print2 ("b: Pay for transformation.");
	print3 ("ESCAPE: Leave this place.");
	char response = mcigetc();
	if (response == 'a') {
	    clearmsg();
	    int i = getitem (CORPSE);
	    if (i == ABORT || !Player.has_possession(i))
		print2 ("So nu?");
	    else {
		object& obj = Player.possessions[i];
		if (Monsters[obj.charge].transformid == NO_THING) {
		    print1 ("I don't want such a thing.");
		    if (obj.basevalue > 0)
			print2 ("You might be able to sell it to someone else, though.");
		} else {
		    clearmsg();
		    unsigned cost = (obj.number * obj.basevalue) / 3;
		    print1 ("I'll give you ");
		    mnumprint (cost);
		    nprint1 ("Au for it. Take it? [yn] ");
		    if (ynq1() != 'y')
			print2 ("Well, keep the smelly old thing, then!");
		    else {
			Player.cash += cost;
			Player.remove_possession(i);
		    }
		}
	    }
	} else if (response == 'b') {
	    clearmsg();
	    int i = getitem (CORPSE);
	    if (i == ABORT || !Player.has_possession(i))
		print2 ("So nu?");
	    else {
		object& obj = Player.possessions[i];
		if (Monsters[obj.charge].transformid == NO_THING)
		    print1 ("Oy vey! You want me to transform such a thing?");
		else {
		    int mlevel = Monsters[obj.charge].level;
		    unsigned cost = max (10, obj.basevalue * 2);
		    print1 ("It'll cost you ");
		    mnumprint (cost);
		    nprint1 ("Au for the transformation. Pay it? [yn] ");
		    if (ynq1() != 'y')
			print2 ("I don't need your business, anyhow.");
		    else {
			if (Player.cash < cost)
			    print2 ("You can't afford it!");
			else {
			    print1 ("Voila! A tap of the Philosopher's Stone...");
			    Player.cash -= cost;
			    obj = Objects[Monsters[obj.charge].transformid];
			    if (obj.id >= STICKID && obj.id < STICKID + NUMSTICKS)
				obj.charge = 20;
			    if (obj.plus == 0)
				obj.plus = mlevel;
			    if (obj.blessing == 0)
				obj.blessing = 1;
			}
		    }
		}
	    }
	} else if (response == KEY_ESCAPE)
	    break;
    }
}

void l_dpw (void)
{
    print1 ("Rampart Department of Public Works.");
    if (Date - LastDay < 7)
	print2 ("G'wan! Get a job!");
    else if (Player.cash < 100) {
	print2 ("Do you want to go on the dole? [yn] ");
	if (ynq2() == 'y') {
	    print1 ("Well, ok, but spend it wisely.");
	    morewait();
	    print1 ("Please enter your name for our records:");
	    strcpy (Str1, msgscanstring());
	    if (Str1[0] >= 'a' && Str1[0] <= 'z')
		Str1[0] += 'A' - 'a';
	    if (Str1[0] == '\0')
		print1 ("Maybe you should come back when you've learned to write.");
	    else if (Player.name != (const char*) Str1) {
		print3 ("Aha! Welfare Fraud! It's off to gaol for you, lout!");
		morewait();
		send_to_jail();
	    } else {
		print2 ("Here's your handout, layabout!");
		LastDay = Date;
		Player.cash = 99;
		dataprint();
	    }
	}
    } else
	print2 ("You're too well off for us to help you!");
}

void l_library (void)
{
    char response;
    bool studied = false;
    bool done = false;
    unsigned fee = 1000;
    print1 ("Rampart Public Library.");
    if (nighttime())
	print2 ("CLOSED");
    else {
	morewait();
	print1 ("Library Research Fee: 1000Au.");
	if (Player.maxiq < 18) {
	    print2 ("The Rampart student aid system has arranged a grant!");
	    morewait();
	    clearmsg();
	    print1 ("Your revised fee is: ");
	    mnumprint (fee = max (50, 1000 - (18 - Player.maxiq) * 125));
	    nprint1 ("Au.");
	}
	morewait();
	while (!done) {
	    print1 ("Pay the fee? [yn] ");
	    if (ynq1() == 'y') {
		if (Player.cash < fee) {
		    print2 ("No payee, No studee.");
		    done = true;
		} else {
		    Player.cash -= fee;
		    do {
			studied = true;
			dataprint();
			menuclear();
			menuprint ("Peruse a scroll:\n");
			menuprint ("a: Omegan Theology\n");
			menuprint ("b: Guide to Rampart\n");
			menuprint ("c: High Magick\n");
			menuprint ("d: Odd Uncatalogued Document\n");
			menuprint ("e: Attempt Advanced Research\n");
			menuprint ("ESCAPE: Leave this font of learning.\n");
			showmenu();
			response = (char) mcigetc();
			if (response == 'a') {
			    print1 ("You unfurl an ancient, yellowing scroll...");
			    morewait();
			    displayfile (Data_ScrollReligion);
			    xredraw();
			} else if (response == 'b') {
			    print1 ("You unroll a slick four-color document...");
			    morewait();
			    displayfile (Data_ScrollRampartGuide);
			    xredraw();
			} else if (response == 'c') {
			    print1 ("This scroll is written in a strange magical script...");
			    morewait();
			    displayfile (Data_ScrollHiMagick);
			    xredraw();
			} else if (response == 'd') {
			    print1 ("You find a strange document, obviously misfiled");
			    print2 ("under the heading 'acrylic fungus painting technique'");
			    morewait();
			    displayfile (Data_ScrollAdept);
			    xredraw();
			} else if (response == 'e') {
			    if (random_range (30) > Player.iq) {
				print2 ("You feel more knowledgeable!");
				Player.iq++;
				Player.maxiq++;
				dataprint();
				if (Player.maxiq < 19 && fee != max (50U, 1000U - (18U - Player.maxiq) * 125U)) {
				    morewait();
				    clearmsg();
				    print1 ("Your revised fee is: ");
				    mnumprint (fee = max (50U, 1000U - (18U - Player.maxiq) * 125U));
				    nprint1 ("Au.");
				    morewait();
				}
			    } else {
				clearmsg1();
				print1 ("You find advice in an ancient tome: ");
				morewait();
				hint();
				morewait();
			    }
			} else if (response == KEY_ESCAPE) {
			    done = true;
			    print1 ("That was an expensive browse...");
			} else
			    studied = false;
		    } while (!studied);
		}
		xredraw();
	    } else {
		done = true;
		if (studied)
		    print2 ("Come back anytime we're open, 7am to 8pm.");
		else
		    print2 ("You philistine!");
	    }
	}
    }
}

void l_pawn_shop (void)
{
    if (nighttime()) {
	print1 ("Shop Closed: Have a Nice (K)Night");
	return;
    }

    unsigned limit = min (Pawnitems.size()/4, unsigned(Date - Pawndate));
    Pawndate = Date;
    for (unsigned i = 0; i < limit; ++i)
	if (object_uniqueness(Pawnitems[0]) > UNIQUE_UNMADE)
	    set_object_uniqueness (Pawnitems[0], UNIQUE_UNMADE);
    Pawnitems.erase (Pawnitems.begin(), limit);
    while (Pawnitems.size() < PAWNITEMS) {
	object o = create_object (5);
	if (o.objchar != CASH && o.objchar != ARTIFACT && true_item_value(o) > 0)
	    Pawnitems.push_back (o);
    }

    while (true) {
	print1 ("Knight's Pawn Shop:");
	print2 ("Buy item, Sell item, sell Pack contents, Leave [b,s,p,ESCAPE] ");
	menuclear();
	for (unsigned i = 0; i < Pawnitems.size(); i++) {
	    strcpy (Str3, " :");
	    Str3[0] = i + 'a';
	    strcat (Str3, itemid (Pawnitems[i]));
	    menuprint (Str3);
	    menuprint ("\n");
	}

	showmenu();
	char action = mcigetc();
	if (action == KEY_ESCAPE)
	    break;
	else if (action == 'b') {
	    print2 ("Purchase which item? [ESCAPE to quit] ");
	    unsigned item = ' ';
	    while (item != KEY_ESCAPE && (item < 'a' || item >= 'a' + Pawnitems.size()))
		item = mcigetc();
	    if (item != KEY_ESCAPE) {
		unsigned i = item - 'a';
		if (true_item_value (Pawnitems[i]) <= 0) {
		    print1 ("Hmm, how did that junk get on my shelves?");
		    print2 ("I'll just remove it.");
		    Pawnitems.erase (Pawnitems.iat(i));
		} else {
		    clearmsg();
		    print1 ("The low, low, cost is: ");
		    unsigned cost = Pawnitems[i].number * true_item_value (Pawnitems[i]);
		    mlongprint (cost);
		    nprint1 (" Buy it? [ynq] ");
		    if (ynq1() == 'y') {
			if (Player.cash < cost) {
			    print2 ("No credit! Gwan, Beat it!");
			    morewait();
			} else {
			    Player.cash -= cost;
			    gain_item (Pawnitems[i]);
			    Pawnitems.erase (Pawnitems.iat(i));
			}
		    }
		}
	    }
	} else if (action == 's') {
	    menuclear();
	    print2 ("Sell which item: ");
	    int i = getitem (NULL_ITEM);
	    if (i != ABORT && Player.has_possession(i)) {
		if (cursed (Player.possessions[i])) {
		    print1 ("No loans on cursed items! I been burned before....");
		    morewait();
		} else if (true_item_value (Player.possessions[i]) <= 0) {
		    print1 ("That looks like a worthless piece of junk to me.");
		    morewait();
		} else {
		    clearmsg();
		    unsigned cost = item_value (Player.possessions[i]) / 2;
		    print1 ("You can get ");
		    mlongprint (cost);
		    nprint1 ("Au each. Sell [yn]? ");
		    if (ynq1() == 'y') {
			unsigned number = getnumber (Player.possessions[i].number);
			Player.cash += number * cost;
			Pawnitems.erase (Pawnitems.begin());
			Pawnitems.emplace_back (split_item (Player.possessions[i], number));
			Player.remove_possession (i, number);
			dataprint();
		    }
		}
	    }
	} else if (action == 'p') {
	    foreach (i, Player.pack) {
		if (i->blessing <= 0 || true_item_value(i) <= 0)
		    continue;
		clearmsg();
		print1 ("Sell ");
		nprint1 (itemid (i));
		nprint1 (" for ");
		unsigned cost = item_value (i) / 2;
		mlongprint (cost);
		nprint1 ("Au each? [yn] ");
		if (ynq1() == 'y') {
		    unsigned number = getnumber (i->number);
		    if (number > 0) {
			Player.cash += number * cost;
			Pawnitems.erase (Pawnitems.begin());
			Pawnitems.emplace_back (*i);
			Pawnitems.back().number = number;
			if ((i->number -= number) < 1)
			    --(i = Player.pack.erase(i));
			dataprint();
		    }
		}
	    }
	}
    }
    calc_melee();
    xredraw();
}

void l_condo (void)
{
    int done = false, weeksleep = false;
    char response;

    if (!gamestatusp (SOLD_CONDO)) {
	print1 ("Rampart Arms. Weekly Rentals and Purchases");
	print2 ("Which are you interested in [r,p, or ESCAPE] ");
	response = mgetc();
	if (response == 'p') {
	    print2 ("Only 50,000Au. Buy it? [yn] ");
	    if (ynq2() == 'y') {
		if (Player.cash < 50000)
		    print3 ("No mortgages, buddy.");
		else {
		    setgamestatus (SOLD_CONDO);
		    Player.cash -= 50000;
		    dataprint();
		    print2 ("You are the proud owner of a luxurious condo penthouse.");
		    Condoitems.clear();
		}
	    }
	} else if (response == 'r') {
	    print2 ("Weekly Rental, 1000Au. Pay for it? [yn] ");
	    if (ynq2() == 'y') {
		if (Player.cash < 1000)
		    print2 ("Hey, pay the rent or out you go....");
		else {
		    weeksleep = true;
		    Player.cash -= 1000;
		    dataprint();
		}
	    }
	} else
	    print2 ("Please keep us in mind for your housing needs.");
    } else {
	while (!done) {
	    menuclear();
	    menuprint ("Home Sweet Home\n");
	    menuprint ("a: Leave items in your safe.\n");
	    menuprint ("b: Retrieve items.\n");
	    menuprint ("c: Take a week off to rest.\n");
	    menuprint ("d: Retire permanently.\n");
	    menuprint ("ESCAPE: Leave this place.\n");
	    showmenu();
	    response = (char) mcigetc();
	    if (response == 'a') {
		int i = getitem (NULL_ITEM);
		if (i != ABORT) {
		    if (Player.possessions[i].blessing < 0)
			print2 ("The item just doesn't want to be stored away...");
		    else {
			Condoitems.push_back (Player.possessions[i]);
			Player.remove_possession (i);
		    }
		}
	    } else if (response == 'b') {
		foreach (i, Condoitems) {
		    print1 ("Retrieve ");
		    nprint1 (itemid(i));
		    nprint1 (" [ynq] ");
		    response = (char) mcigetc();
		    if (response == 'q')
			break;
		    else if (response == 'y') {
			gain_item (*i);
			--(i = Condoitems.erase(i));
		    }
		}
	    } else if (response == 'c') {
		weeksleep = true;
		print1 ("You take a week off to rest...");
		morewait();
	    } else if (response == 'd') {
		clearmsg();
		print1 ("You sure you want to retire, now? [yn] ");
		if (ynq1() == 'y') {
		    p_win();
		}
	    } else if (response == KEY_ESCAPE)
		done = true;
	}
	xredraw();
    }
    if (weeksleep) {
	clearmsg();
	print1 ("Taking a week off to rest...");
	morewait();
	toggle_item_use (true);
	Player.hp = Player.maxhp;
	Player.str = Player.maxstr;
	Player.agi = Player.maxagi;
	Player.con = Player.maxcon;
	Player.dex = Player.maxdex;
	Player.iq = Player.maxiq;
	Player.pow = Player.maxpow;
	for (unsigned i = 0; i < NUMSTATI; i++)
	    if (Player.status[i] < 1000)
		Player.status[i] = 0;
	toggle_item_use (false);
	Player.food = 36;
	print2 ("You're once again fit and ready to continue your adventure.");
	Time += 60 * 24 * 7;
	Date += 7;
	moon_check();
	timeprint();
    }
}

static void gymtrain (uint8_t *maxstat, uint8_t *stat)
{
    if (Gymcredit + Player.cash < 2000)
	print2 ("You can't afford our training!");
    else {
	if (Gymcredit > 2000)
	    Gymcredit -= 2000;
	else {
	    Player.cash -= (2000 - Gymcredit);
	    Gymcredit = 0;
	}
	print2 ("Sweat. Sweat. ");
	if ((*maxstat < 30) && ((*maxstat < random_range (30)) || (random_range (3) == 1))) {
	    nprint2 ("The training pays off!");
	    (*maxstat)++;
	    (*stat)++;
	} else {
	    nprint2 ("You feel the healthy glow of a good workout.");
	    if (*stat < *maxstat) {
		(*stat)++;
		print3 ("A feeling of rehabilitation washes through you.");
	    }
	}
    }
    dataprint();
}

static void healforpay (void)
{
    if (Player.cash < 50)
	print2 ("You can't afford to be healed!");
    else {
	Player.cash -= 50;
	Player.hp += 20 + random_range (20);
	if (Player.hp > Player.maxhp)
	    Player.hp = Player.maxhp;
	print2 ("Another medical marvel....");
    }
    calc_melee();
}

static void cureforpay (void)
{
    if (Player.cash < 250)
	print2 ("You can't afford to be cured!");
    else {
	Player.cash -= 250;
	Player.status[DISEASED] = 0;
	print2 ("Quarantine lifted....");
	showflags();
    }
}

void pacify_guards (void)
{
    foreach (m, Level->mlist) {
	if (m->id == GUARD || (m->id == HISCORE_NPC && m->aux2 == NPC_JUSTICIAR)) {
	    m_status_reset (m, HOSTILE);
	    m->specialf = M_NO_OP;
	    if (m->id == GUARD && m->hp > 0 && m->aux1 > 0) {
		m->x = m->aux1;
		m->y = m->aux2;
	    } else if (m->id == HISCORE_NPC && m->hp > 0 && Current_Environment == E_CITY) {
		m->x = 40;
		m->y = 62;
	    }
	}
    }
    if (Current_Environment == E_CITY)
	Level->site(40,60).p_locf = L_ORDER;	// undoes action in alert_guards
}

void send_to_jail (void)
{
    if (Player.rank[ORDER] >= GALLANT) {
	print1 ("A member of the Order of Paladins sent to jail!");
	print2 ("It cannot be!");
	morewait();
	print1 ("You are immediately expelled permanently from the Order!");
	print2 ("Your name is expunged from the records....");
	Player.rank[ORDER] = FORMER_PALADIN;
    } else if (gamestatusp (DESTROYED_ORDER))
	print1 ("The destruction of the Order of Paladins has negated the law!");
    else if ((Current_Environment != E_CITY) && (Last_Environment != E_CITY))
	print1 ("Fortunately, there is no jail around here, so you are freed!");
    else {
	pacify_guards();
	if (((Current_Environment == E_HOUSE) || (Current_Environment == E_MANSION) || (Current_Environment == E_HOVEL)) && (Last_Environment == E_CITY)) {
	    setgamestatus (SUPPRESS_PRINTING);
	    change_environment (E_CITY);
	    resetgamestatus (SUPPRESS_PRINTING);
	}
	if (Current_Environment == E_CITY) {
	    if (gamestatusp (UNDEAD_GUARDS)) {
		print1 ("You are taken to a weirdly deserted chamber where an undead");
		print2 ("Magistrate presides over a court of ghosts and haunts.");
		morewait();
		print1 ("'Mr. Foreman, what is the verdict?'");
		print2 ("'Guilty as charged, your lordship.'");
		morewait();
		clearmsg();
		print1 ("'Guilty...");
		morewait();
		nprint1 ("Guilty...");
		morewait();
		nprint1 ("Guilty...");
		morewait();
		nprint1 ("Guilty...'");
		clearmsg();
		print1 ("The members of the court close in around, fingers pointing.");
		print2 ("You feel insubstantial hands closing around your throat....");
		print3 ("You feel your life draining away!");
		while (Player.level > 0) {
		    Player.level--;
		    Player.xp /= 2;
		    Player.hp /= 2;
		    dataprint();
		}
		Player.maxhp = Player.maxcon;
		morewait();
		print1 ("You are finally released, a husk of your former self....");
		Player.x = 58;
		Player.y = 40;
		screencheck (58);
	    } else if (Player.alignment + random_range (200) < 0) {
		print1 ("Luckily for you, a smooth-tongued advocate from the");
		print2 ("Rampart Chaotic Liberties Union gets you off!");
		Player.x = 58;
		Player.y = 40;
		screencheck (58);
	    } else
		switch (Imprisonment++) {
		    case 0:
			print1 ("The Magistrate sternly reprimands you.");
			print2 ("As a first-time offender, you are given probation.");
			Player.y = 58;
			Player.x = 40;
			screencheck (58);
			break;
		    case 1:
			print1 ("The Magistrate expresses shame for your conduct.");
			print2 ("You are thrown in jail!");
			morewait();
			repair_jail();
			Player.y = 54;
			Player.x = 37 + (2 * random_range (4));
			screencheck (54);
			l_portcullis_trap();
			break;
		    default:
			print1 ("The Magistrate renders summary judgement.");
			print2 ("You are sentenced to prison!");
			morewait();
			print1 ("The guards recognize you as a 'three-time-loser'");
			print2 ("...and beat you up a little to teach you a lesson.");
			p_damage (random_range (Imprisonment * 10), UNSTOPPABLE, "police brutality");
			morewait();
			repair_jail();
			Player.y = 54;
			Player.x = 37 + (2 * random_range (4));
			screencheck (54);
			l_portcullis_trap();
		}
	}
    }
}

void l_adept (void)
{
    print1 ("You see a giant shimmering gate in the form of an omega.");
    if (gamestatusp (SPOKE_TO_ORACLE)) {
	if (Player.str + Player.con + Player.iq + Player.pow < 100)
	    print2 ("A familiar female voice says: I would not advise this now....");
	else
	    print2 ("A familiar female voice says: Go for it!");
	morewait();
	clearmsg();
    }
    print2 ("Enter the mystic portal? [yn] ");
    if (ynq2() != 'y') {
	if (Player.level > 100) {
	    print1 ("The Lords of Destiny spurn your cowardice....");
	    Player.xp = 0;
	    Player.level = 0;
	    Player.hp = Player.maxhp = Player.con;
	    Player.mana = calcmana();
	    print2 ("You suddenly feel very inexperienced.");
	    dataprint();
	}
    } else {
	clearmsg();
	print1 ("You pass through the portal.");
	morewait();
	drawomega();
	print1 ("Like wow man! Colors! ");
	if (Player.patron != DESTINY) {
	    print2 ("Strange forces try to tear you apart!");
	    p_damage (random_range (200), UNSTOPPABLE, "a vortex of chaos");
	} else
	    print2 ("Some strange force shields you from a chaos vortex!");
	morewait();
	print1 ("Your head spins for a moment....");
	print2 ("and clears....");
	morewait();
	Player.hp = Player.maxhp;
	Player.mana = calcmana();
	change_environment (E_ABYSS);
    }
}

void l_trifid (void)
{
    int damage = 0, stuck = true;
    print1 ("The hedge comes alive with a surge of alien growth!");
    while (stuck) {
	dataprint();
	damage += Level->depth / 2 + 1;
	print2 ("Razor-edged vines covered in suckers attach themselves to you.");
	morewait();
	if (find_and_remove_item (THING_SALT_WATER, -1)) {
	    print1 ("Thinking fast, you toss salt water on the trifid...");
	    print2 ("The trifid disintegrates with a frustrated sigh.");
	    Level->site(Player.x,Player.y).locchar = FLOOR;
	    Level->site(Player.x,Player.y).p_locf = L_NO_OP;
	    lset (Player.x, Player.y, CHANGED);
	    gain_experience (1000);
	    stuck = false;
	} else {
	    p_damage (damage, UNSTOPPABLE, "a trifid");
	    morewait();
	    print1 ("You are entangled in tendrils...");
	    menuclear();
	    menuprint ("a: Try to break free.\n");
	    menuprint ("b: Hang limp and hope the tendrils uncoil.\n");
	    menuprint ("c: Pray for assistance.\n");
	    menuprint ("d: Attempt to bargain with the hedge.\n");
	    menuprint ("e: Click your heels together and wish for escape.\n");
	    menuprint ("ANYTHING ELSE: writhe and scream hopelessly.\n");
	    showmenu();
	    switch (menugetc()) {
		case 'a':
		    if (Player.str > random_range (200)) {
			print1 ("Amazing! You're now free.");
			print2 ("The trifid writhes hungrily at you.");
			stuck = false;
		    } else
			print1 ("Well, THAT didn't work.");
		    break;
		case 'b':
		    print1 ("Well, at least you're facing your fate with dignity.");
		    break;
		case 'c':
		    if (Player.patron == DRUID && Player.rank[PRIESTHOOD] >= LAY+random_range(HIGHPRIEST-LAY)) {
			print1 ("A shaft of golden light bathes the alien plant");
			print2 ("which grudginly lets you go....");
			stuck = false;
		    } else
			print1 ("You receive no divine aid as yet.");
		    break;
		case 'd':
		    print1 ("The hedge doesn't answer your entreaties.");
		    break;
		case 'e':
		    print1 ("You forgot your ruby slippers, stupid.");
		    break;
		default:
		    print1 ("The hedge enjoys your camp play-acting....");
		    break;
	    }
	}
    }
    xredraw();
}

void l_vault (void)
{
    print1 ("You come to a thick vault door with a complex time lock.");
    if ((hour() == 23)) {
	print2 ("The door is open.");
	Level->site(12,56).locchar = FLOOR;
    } else {
	print2 ("The door is closed.");
	Level->site(12,56).locchar = WALL;
	morewait();
	clearmsg();
	print1 ("Try to crack it? [yn] ");
	if (ynq1() == 'y') {
	    if (random_range (100) < Player.rank[THIEVES] * Player.rank[THIEVES]) {
		print2 ("The lock clicks open!!!");
		gain_experience (5000);
		Level->site(12,56).locchar = FLOOR;
	    } else {
		print2 ("Uh, oh, set off the alarm.... The castle guard arrives....");
		morewait();
		if (Player.rank[NOBILITY] == DUKE) {
		    clearmsg();
		    print1 ("\"Ah, just testing us, your Grace?  I hope we're up to scratch.\"");
		    morewait();
		} else
		    send_to_jail();
	    }
	} else
	    print2 ("Good move.");
    }
}

void l_brothel (void)
{
    char response;
    print1 ("You come to a heavily reinforced inner door.");
    print2 ("A sign reads `The House of the Eclipse'");
    morewait();
    clearmsg();
    print1 ("Try to enter? [yn] ");
    if (ynq1() == 'y') {
	menuclear();
	menuprint ("a:knock on the door.\n");
	menuprint ("b:try to pick the lock.\n");
	menuprint ("c:bash down the door.\n");
	menuprint ("ESCAPE: Leave this house of ill repute.\n");
	showmenu();
	do
	    response = menugetc();
	while ((response != 'a') && (response != 'b') && (response != 'c') && (response != KEY_ESCAPE));
	xredraw();
	if (response == 'a') {
	    if (!nighttime())
		print2 ("There is no reponse.");
	    else {
		print1 ("A window opens in the door.");
		print2 ("`500Au, buddy. For the night.' pay it? [yn] ");
		if (ynq2() == 'y') {
		    if (Player.cash < 500) {
			print1 ("`What, no roll?!'");
			print2 ("The bouncer bounces you a little and lets you go.");
			p_damage (25, UNSTOPPABLE, "da bouncer");
		    } else {
			Player.cash -= 500;
			print1 ("You are ushered into an opulently appointed hall.");
			print2 ("After an expensive dinner (takeout from Les Crapuleux)");
			morewait();
			if (Player.preference == 'n') {
			    switch (random_range (4)) {
				case 0:
				    print1 ("you spend the evening playing German Whist with");
				    break;
				case 1:
				    print1 ("you spend the evening discussing philosophy with");
				    break;
				case 2:
				    print1 ("you spend the evening playing chess against");
				    break;
				case 3:
				    print1 ("you spend the evening telling your adventures to");
			    }
			    print2 ("various employees of the House of the Eclipse.");
			} else {
			    print1 ("you spend an enjoyable and educational evening with");
			    if (Player.preference == 'm' || (Player.preference == 'y' && random_range (2)))
				switch (random_range (4)) {
				    case 0:
					print2 ("Skarn the Insatiable, a satyr.");
					break;
				    case 1:
					print2 ("Dryden the Defanged, an incubus.");
					break;
				    case 2:
					print2 ("Gorgar the Equipped, a centaur.");
					break;
				    case 3:
					print2 ("Hieronymus, the mendicant priest of Eros.");
					break;
			    } else
				switch (random_range (4)) {
				    case 0:
					print2 ("Noreen the Nymph (omaniac)");
					break;
				    case 1:
					print2 ("Angelface, a recanted succubus.");
					break;
				    case 2:
					print2 ("Corporal Sue of the City Guard (moonlighting).");
					break;
				    case 3:
					print2 ("Sheena the Queena the Jungle, a wereleopard.");
					break;
				}
			}
			morewait();
			if (hour() > 12)
			    Time += ((24 - hour()) + 8) * 60;
			else {
			    Time += ((9 - hour()) * 60);
			    Date++;
			}
			Player.food = 40;
			Player.status[DISEASED] = 0;
			Player.status[POISONED] = 0;
			Player.hp = Player.maxhp;
			// reduce temporary stat gains to max stat levels
			toggle_item_use (true);
			Player.str = min (Player.str, Player.maxstr);
			Player.con = min (Player.con, Player.maxcon);
			Player.agi = min (Player.agi, Player.maxagi);
			Player.dex = min (Player.dex, Player.maxdex);
			Player.iq = min (Player.iq, Player.maxiq);
			Player.pow = min (Player.pow, Player.maxpow);
			toggle_item_use (false);
			if (Player.preference == 'n')
			    Player.iq++;	// whatever :-)
			else
			    Player.con++;
			gain_experience (100);
			timeprint();
			dataprint();
			showflags();
			morewait();
			clearmsg();
			if (Player.preference == 'n')
			    print1 ("You arise refreshed the next morning...");
			else
			    print1 ("You arise, tired but happy, the next morning...");
		    }
		} else
		    print2 ("What are you, some kinda prude?");
	    }
	} else if (response == 'b') {
	    if (nighttime()) {
		print1 ("As you fumble at the lock, the door opens....");
		print2 ("The bouncer tosses you into the street.");
	    } else
		print1 ("The door appears to be bolted and barred from behind.");
	} else if (response == 'c') {
	    if (nighttime()) {
		print1 ("As you charge toward the door it opens....");
		print2 ("Yaaaaah! Thud!");
		morewait();
		print1 ("You run past the startled bouncer into a wall.");
		p_damage (20, UNSTOPPABLE, "a move worthy of Clouseau");
		print2 ("The bouncer tosses you into the street.");
	    } else {
		print1 ("Ouch! The door resists your efforts.");
		p_damage (1, UNSTOPPABLE, "a sturdy door");
		morewait();
		print1 ("You hear an irritated voice from inside:");
		print2 ("'Keep it down out there! Some of us are trying to sleep!'");
	    }
	}
    }
}

// if signp is true, always print message, otherwise do so only sometimes
void sign_print (int x, int y, int signp)
{
    if ((Level->site(x,y).p_locf >= CITYSITEBASE) && (Level->site(x,y).p_locf < CITYSITEBASE + NUMCITYSITES))
	CitySiteList[Level->site(x,y).p_locf - CITYSITEBASE][0] = true;
    switch (Level->site(x,y).p_locf) {
	case L_CHARITY:
	    print1 ("You notice a sign: The Rampart Orphanage And Hospice For The Needy.");
	    break;
	case L_MANSION:
	    print1 ("You notice a sign:");
	    print2 ("This edifice protected by DeathWatch Devices, Ltd.");
	    morewait();
	    break;
	case L_GRANARY:
	    print1 ("You notice a sign:");
	    print2 ("Public Granary: Entrance Strictly Forbidden.");
	    break;
	case L_PORTCULLIS:
	    if (Level->site(x,y).locchar == FLOOR)
		print1 ("You see a groove in the floor and slots above you.");
	    break;
	case L_STABLES:
	    print1 ("You notice a sign:");
	    print2 ("Village Stables");
	    break;
	case L_COMMONS:
	    print1 ("You notice a sign:");
	    print2 ("Village Commons: No wolves allowed.");
	    break;
	case L_MAZE:
	    print1 ("You notice a sign:");
	    print2 ("Hedge maze closed for trifid extermination.");
	    break;
	case L_BANK:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("First Bank of Omega: Autoteller Carrel.");
	    }
	    break;
	case L_TEMPLE:
	    print1 ("You see the ornate portico of the Rampart Pantheon");
	    break;
	case L_ARMORER:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Julie's Armor of Proof and Weapons of Quality");
	    }
	    break;
	case L_CLUB:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Rampart Explorers' Club.");
	    }
	    break;
	case L_GYM:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("The Rampart Gymnasium, (affil. Rampart Coliseum).");
	    }
	    break;
	case L_HEALER:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Rampart Healers. Member RMA.");
	    }
	    break;
	case L_CASINO:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Rampart Mithril Nugget Casino.");
	    }
	    break;
	case L_SEWER:
	    print1 ("A sewer entrance. You don't want to go down THERE, do you?");
	    break;
	case L_COMMANDANT:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Commandant Sonder's Rampart-fried Lyzzard partes. Open 24 hrs.");
	    }
	    break;
	case L_DINER:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("The Rampart Diner. All you can eat, 25Au.");
	    }
	    break;
	case L_CRAP:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Les Crapeuleaux. (****)");
	    }
	    break;
	case L_TAVERN:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("The Centaur and Nymph -- J. Riley, prop.");
	    }
	    break;
	case L_ALCHEMIST:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Ambrosias' Potions et cie.");
	    }
	    break;
	case L_DPW:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Rampart Department of Public Works.");
	    }
	    break;
	case L_LIBRARY:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Rampart Public Library.");
	    }
	    break;
	case L_CONDO:
	    if (signp) {
		print1 ("You notice a sign:");
		if (gamestatusp (SOLD_CONDO))
		    print2 ("Home Sweet Home");
		else
		    print2 ("Luxury Condominium For Sale; Inquire Within");
	    }
	    break;
	case L_PAWN_SHOP:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Knight's Pawn Shop.");
	    }
	    break;
	case L_CEMETARY:
	    print1 ("You notice a sign:");
	    print2 ("Rampart City Cemetary. Closed -- Full.");
	    break;
	case L_GARDEN:
	    print1 ("You notice a sign:");
	    print2 ("Rampart Botanical Gardens---Do not deface statues.");
	    break;
	case L_JAIL:
	    print1 ("You notice a sign:");
	    print2 ("Rampart City Gaol -- always room for more.");
	    break;
	case L_ORACLE:
	    print1 ("You notice a sign:");
	    print2 ("The Oracle of the Cyan Flames");
	    morewait();
	    break;
    }
}

void l_countryside (void)
{
    if (optionp (CONFIRM)) {
	clearmsg();
	print1 ("Do you really want to return to the countryside? ");
	if (ynq1() != 'y')
	    return;
    }
    change_environment (E_COUNTRYSIDE);
}

void l_oracle (void)
{
    print1 ("You come before a blue crystal dais. There is a bell and a mirror.");
    print2 ("Ring the bell [b], look in the mirror [m], or leave [ESCAPE] ");
    char response;
    do
	response = mcigetc();
    while (response != 'b' && response != 'm' && response != KEY_ESCAPE);
    if (response == 'b') {
	print1 ("The ringing note seems to last forever.");
	print2 ("You notice a robed figure in front of you....");
	morewait();
	print1 ("The oracle doffs her cowl. Her eyes glitter with blue fire!");
	print2 ("She stares at you...and speaks:");
	setgamestatus (SPOKE_TO_ORACLE);
	if (!gamestatusp (SPOKE_TO_DRUID)) {
	    print3 ("'The ArchDruid speaks wisdom in his forest shrine.'");
	} else if (!gamestatusp (COMPLETED_CAVES)) {
	    print3 ("'Thou mayest find aught of interest in the caves to the South.'");
	} else if (!gamestatusp (COMPLETED_SEWERS)) {
	    print3 ("'Turn thy attention to the abyssal depths of the city.'");
	} else if (!gamestatusp (COMPLETED_CASTLE)) {
	    print3 ("'Explorest thou the depths of the Castle of the ArchMage.'");
	} else if (!gamestatusp (COMPLETED_ASTRAL)) {
	    morewait();
	    print1 ("'Journey to the Astral Plane and meet the Gods' servants.'");
	    print2 ("The oracle holds out her hand. Do you take it? [yn] ");
	    if (ynq2() == 'y') {
		print1 ("'Beware: Only the Star Gem can escape the Astral Plane.'");
		print2 ("A magic portal opens behind the oracle. She leads you");
		morewait();
		print1 ("through a sequence of special effects that would have");
		print2 ("IL&M technicians cursing in awe and deposits you in an");
		morewait();
		clearmsg();
		print1 ("odd looking room whose walls seem strangely insubstantial.");
		gain_experience (5000);
		change_environment (E_ASTRAL);
	    } else
		print3 ("You detect the hint of a sneer from the oracle.");
	} else if (!gamestatusp (COMPLETED_VOLCANO)) {
	    print3 ("'The infernal maw may yield its secrets to thee now.'");
	} else if (!gamestatusp (COMPLETED_CHALLENGE)) {
	    print3 ("'The challenge of adepthood yet awaits thee.'");
	} else {
	    morewait();
	    print1 ("'My lord: Thou hast surpassed my tutelage forever.");
	    print2 ("Fare thee well.'");
	    print3 ("The oracle replaces her hood and seems to fade away....");
	}
    } else if (response == 'm') {
	print1 ("You seem to see yourself. Odd....");
	knowledge (1);
    } else
	print2 ("You leave this immanent place.");
}

void l_mansion (void)
{
    print1 ("Enter the mansion? [yn] ");
    if (ynq1() == 'y')
	change_environment (E_MANSION);
}

void l_house (void)
{
    print1 ("Enter the house? [yn] ");
    if (ynq1() == 'y')
	change_environment (E_HOUSE);
}

void l_hovel (void)
{
    print1 ("Enter the hovel? [yn] ");
    if (ynq1() == 'y')
	change_environment (E_HOVEL);
}

void l_safe (void)
{
    char response;
    int attempt = 0;
    print1 ("You have discovered a safe!");
    print2 ("Pick the lock [p], Force the door [f], or ignore [ESCAPE]");
    do
	response = (char) mcigetc();
    while (response != 'p' && response != 'f' && response != KEY_ESCAPE);
    if (response == 'p')
	attempt = (2 * Player.dex + Player.rank[THIEVES] * 10 - random_range (100)) / 10;
    else if (response == 'f')
	attempt = (Player.dmg - random_range (100)) / 10;
    if (attempt > 0) {
	Player.alignment -= 4;
	gain_experience (50);
	print2 ("The door springs open!");
	Level->site(Player.x,Player.y).locchar = FLOOR;
	Level->site(Player.x,Player.y).p_locf = L_NO_OP;
	lset (Player.x, Player.y, CHANGED);
	if (random_range(2)) {
	    print1 ("You find:");
	    do {
		object newitem = create_object (difficulty());
		print2 (itemid (newitem));
		gain_item (newitem);
		morewait();
	    } while (!random_range(3));
	} else
	    print2 ("The safe was empty (awwwww....)");
    } else {
	print3 ("Your attempt at burglary failed!");
	if (attempt == -1) {
	    print1 ("A siren goes off! You see flashing red lights everywhere!");
	    morewait();
	    if (Last_Environment == E_CITY) {
		print2 ("The city guard shows up! They collar you in no time flat!");
		change_environment (E_CITY);
		morewait();
		send_to_jail();
	    }
	} else if (attempt == -2) {
	    print1 ("There is a sudden flash!");
	    p_damage (random_range (25), FLAME, "a safe");
	    print2 ("The safe has self-destructed.");
	    Level->site(Player.x,Player.y).locchar = RUBBLE;
	    Level->site(Player.x,Player.y).p_locf = L_RUBBLE;
	    lset (Player.x, Player.y, CHANGED);
	} else if (attempt == -3) {
	    print1 ("The safe jolts you with electricity!");
	    lball (Player.x, Player.y, Player.x, Player.y, 30);
	} else if (attempt < -3) {
	    print1 ("You are hit by an acid spray!");
	    if (Player.has_possession(O_CLOAK)) {
		print2 ("Your cloak is destroyed!");
		Player.remove_possession (O_CLOAK);
		p_damage (10, ACID, "a safe");
	    } else if (Player.has_possession(O_ARMOR)) {
		print2 ("Your armor corrodes!");
		Player.possessions[O_ARMOR].dmg -= 3;
		Player.possessions[O_ARMOR].hit -= 3;
		Player.possessions[O_ARMOR].aux -= 3;
		p_damage (10, ACID, "a safe");
	    } else {
		print2 ("The acid hits your bare flesh!");
		p_damage (random_range (100), ACID, "a safe");
	    }
	}
    }
}

void l_cartographer (void)
{
    int i, j, x, y;
    print1 ("Ye Olde Mappe Shoppe.");
    print2 ("Map of the local area: 500Au. Buy it? [yn] ");
    if (ynq2() == 'y') {
	if (Player.cash < 500)
	    print3 ("Cursed be cheapskates! May you never find an aid station....");
	else {
	    print3 ("You now have the local area mapped.");
	    Player.cash -= 500;
	    dataprint();
	    switch (Villagenum) {
		case 1: x = 56; y =  5; break;
		default:
		case 2: x = 35; y = 11; break;
		case 3: x = 10; y = 40; break;
		case 4: x =  7; y =  6; break;
		case 5: x = 40; y = 43; break;
		case 6: x = 20; y = 41; break;
	    }
	    for (i = x - 15; i <= x + 15; i++) {
		for (j = y - 15; j <= y + 15; j++) {
		    if (i >= 0 && i < Country->width && j >= 0 && j < Country->height) {
			if (Country->site(i,j).showchar != Country->site(i,j).locchar) {
			    c_set (i, j, CHANGED);
			    Country->site(i,j).showchar = Country->site(i,j).locchar;
			}
			c_set (i, j, SEEN);
		    }
		}
	    }
	}
    } else
	print3 ("Don't blame me if you get lost....");
}

void l_charity (void)
{
    long donation;
    print2 ("'Greetings, friend. Do you wish to make a donation?' [yn] ");
    if (ynq2() != 'y')
	print3 ("'Pinchpurse!'");
    else {
	clearmsg();
	print1 ("How much can you give? ");
	donation = parsenum();
	if (donation < 1)
	    print2 ("'Go stick your head in a pig.'");
	else if (donation > Player.cash)
	    print2 ("'I'm afraid you're charity is bigger than your purse!'");
	else if (donation < max (100, Player.level * Player.level * 100)) {
	    print2 ("'Oh, can't you do better than that?'");
	    print3 ("'Well, I guess we'll take it....'");
	    if (Player.alignment < 10)
		Player.alignment++;
	    Player.cash -= donation;
	} else {
	    print2 ("'Oh thank you kindly, friend, and bless you!'");
	    Player.cash -= donation;
	    Player.alignment += 5;
	}
    }
    dataprint();
}
