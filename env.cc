// Omega is free software, distributed under the MIT license

#include "glob.h"
#include <unistd.h>

//----------------------------------------------------------------------

static monster& make_prime(int i, int j);
static void assign_city_function (location& s, int x, int y);
static void make_justiciar(int i, int j);
static void mazesite (char c, location& s, int i, int j);
static void make_minor_undead(int i, int j);
static void make_major_undead(int i, int j);
static void random_temple_site(int i, int j, int deity);
static void make_high_priest(int i, int j, int deity);
static void make_house_npc(int i, int j);
static void make_mansion_npc(int i, int j);
static void assign_village_function (location& s, int x, int y, bool setup);
static void make_food_bin(int i, int j);
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

chtype location::showchar (void) const noexcept
{
    if (!(lstatus & SEEN))
	return (' ');
    if (lstatus & SECRET) {
	switch (locchar) {
	    case CASTLE:
	    case CAVES:
	    case PASS:
	    case STARPEAK:
	    case VOLCANO:	return (MOUNTAINS);
	    case DRAGONLAIR:	return (DESERT);
	    case MAGIC_ISLE:	return (CHAOS_SEA);
	    default:		return (WALL);
	}
    }
    return (locchar);
}

//----------------------------------------------------------------------

level::level (void)
: _site (MAXWIDTH*MAXLENGTH)
, mlist()
, things()
, environment (E_NEVER_NEVER_LAND)
, width(0)
, height(0)
, lastx(0)
, lasty(0)
, depth(0)
, generated(0)
, tunnelled(0)
{
}

// erase the level w/o deallocating it
void level::clear (void)
{
    generated = false;
    tunnelled = 0;
    depth = 0;
    mlist.clear();
    fill (_site, (location){ WALL, min<uint8_t>(UINT8_MAX,20*difficulty()), L_NO_OP, 0, RS_WALLSPACE });
}

// returns true if its ok to get rid of a level
bool level::ok_to_free (void) const
{
    return (this
	    && environment != E_COUNTRYSIDE
	    && environment != E_CITY
	    && environment != E_VILLAGE
	    && !IsDungeon());
}

// Returns true if the level should not be preserved
bool level::IsTransient (void) const
{
    return (environment == E_TACTICAL_MAP ||
	    environment == E_ARENA ||
	    environment == E_HOUSE ||
	    environment == E_HOVEL ||
	    environment == E_MANSION);
}

uint8_t level::MaxDepth (void) const
{
    static const uint8_t c_DungeonDepth [E_NUMDUNGEONS+1] =
	{ CAVELEVELS, SEWERLEVELS, CASTLELEVELS, VOLCANOLEVELS, ASTRALLEVELS, 0 };
    return (c_DungeonDepth [min<unsigned>(environment-E_FIRST_DUNGEON,ArraySize(c_DungeonDepth)-1)]);
}

monster* level::creature (int x, int y)
{
    foreach (m, mlist)
	if (m->x == x && m->y == y)
	    return (&*m);
    return (NULL);
}

object* level::thing (int x, int y)
{
    foreach (i, things)
	if (i->x == x && i->y == y)
	    return (&*i);
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
    auto no = things.insert (things.end(), o);
    no->x = x;
    no->y = y;
    no->used = false;
    if (n != (unsigned) RANDOM)
	no->number = n;
}

template <typename SiteFunc>
/*static*/ inline void level::load_map (EEnvironment e, const char* edata, SiteFunc sf)
{
    Level->environment = e;
    Level->resize (edata[0], edata[1]);
    if (!Level->lastx) {
	Level->lastx = edata[2];
	Level->lasty = edata[3];
    }
    const char* ld = &edata[4];
    for (unsigned j = 0; j < Level->height; ++j, ++ld)
	for (unsigned i = 0; i < Level->width; ++i)
	    sf (*ld++, Level->site(i,j), i, j);
}

void level::Generate (EEnvironment e, uint8_t subeid)
{
    Level = this;
    clear();
    environment = e;
    if (IsDungeon())
	depth = subeid;
    else if (e == E_VILLAGE)
	SetVillageId (subeid);
    else if (e == E_TEMPLE)
	SetTempleDeity (subeid);

    // Load the default environment contents
    switch (environment) {
	default:
	case E_COUNTRYSIDE:	load_country(); break;
	case E_CITY:		load_city(); break;
	case E_VILLAGE:		load_village (VillageId()); break;
	case E_TACTICAL_MAP:	load_encounter (subeid); break;
	case E_ARENA:		load_arena(); break;
	case E_HOVEL:
	case E_HOUSE:
	case E_MANSION:		load_house (environment); break;
	case E_CAVES:
	case E_SEWERS:
	case E_CASTLE:
	case E_VOLCANO:
	case E_ASTRAL:		generate_level (depth);
				populate_level();
				stock_level(); break;
	case E_DLAIR:		load_dlair (gamestatusp (KILLED_DRAGONLORD)); break;
	case E_STARPEAK:	load_speak (gamestatusp (KILLED_LAWBRINGER)); break;
	case E_MAGIC_ISLE:	load_misle (gamestatusp (KILLED_EATER)); break;
	case E_TEMPLE:		load_temple (TempleDeity()); break;
	case E_CIRCLE:		load_circle(); break;
	case E_COURT:		load_court(); break;
    }
}

//----------------------------------------------------------------------

EEnvironment CWorld::LastEnvironment (void) const
{
    return (_levels.size() > 1 ? _levels[_levels.size()-2].environment : E_NEVER_NEVER_LAND);
}

vector<level>::iterator CWorld::FindEnvironment (EEnvironment e, uint8_t subeid)
{
    foreach (l, _levels)
	if (l->environment == e
	    && (!l->IsDungeon() || l->depth == subeid)
	    && (e != E_VILLAGE || l->VillageId() == subeid)
	    && (e != E_TEMPLE || l->TempleDeity() == subeid))
	    return (l);
    return (_levels.end());
}

void CWorld::MoveInCountry (uint8_t x, uint8_t y)
{
    auto country = FindEnvironment (E_COUNTRYSIDE);
    assert (country < _levels.end());
    country->lastx = x;
    country->lasty = y;
}

void CWorld::LoadEnvironment (EEnvironment e, uint8_t subeid)
{
    if (Level) {
	Level->lastx = Player.x;
	Level->lasty = Player.y;
	if (Level->IsTransient()) {
	    _levels.pop_back();			// Encounters and arena maps are never kept
	    Level = nullptr;
	}
    }
    vector<level>::iterator nl = FindEnvironment (e, subeid);
    if (nl < _levels.end()) {
	rotate (nl, nl+1, _levels.end());	// Keep last used level last
	Level = &_levels.back();
    } else {
	if (_levels.size() >= c_MaxLevels) {	// Make room for new level
	    foreach (l, _levels) {
		if (l->ok_to_free()) {
		    --(l = _levels.erase(l));
		    break;
		}
	    }
	}
	Level = &*(nl = _levels.emplace (_levels.end()));
	Level->Generate (e, subeid);
    }
    Player.x = Level->lastx;
    Player.y = Level->lasty;
}

void CWorld::DeleteLevel (EEnvironment e, uint8_t subeid)
{
    auto l = FindEnvironment (e, subeid);
    if (l < _levels.end()) {
	if (Level == &*l)
	    Level = nullptr;
	_levels.erase (l);
    }
}

//----------------------------------------------------------------------

// loads the arena level into Level
void load_arena (void)
{
    level::load_map (E_ARENA, Level_Arena, [](char sc, location& s, unsigned, unsigned) {
	s.locchar = FLOOR;
	s.p_locf = L_NO_OP;
	s.lstatus = SEEN| LIT;
	s.roomnumber = RS_ARENA;
	switch (sc) {
	    case 'P': s.p_locf = L_PORTCULLIS; s.locchar = PORTCULLIS; break;
	    case 'X': s.p_locf = L_ARENA_EXIT; break;
	    case '#': s.locchar = WALL; break;
	}
    });

    static const uint8_t _opponents[] = {
	GEEK, HORNET, HYENA, GOBLIN, GRUNT, TOVE, APPR_NINJA, SALAMANDER, ANT, MANTICORE,
	SPECTRE, BANDERSNATCH, LICHE, AUTO_MAJOR, JABBERWOCK, JOTUN, HISCORE_NPC
    };
    monster& m = make_site_monster (60, 7, _opponents[min<unsigned>(Arena_Opponent,ArraySize(_opponents))]);
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
	char buf[128];
	snprintf (ArrayBlock(buf), "The corpse of %s the %s", nameprint(), m.monstring);
	m.corpsestr = strdup (buf);
	m.monstring = strdup (buf+strlen("The corpse of "));
    }
    m.uniqueness = UNIQUE_MADE;
    m.attacked = true;
    m_status_set (m, HOSTILE);
    m.sense = 50;
    m.pickup (Objects[THING_DOOR_OPENER]);
    m.hp += m.level * 10;
    m.hit += m.hit;
    m.dmg += m.dmg / 2;

    // hehehehe cackled the dungeon master....
    mprint ("You have a challenger: ");
    mprint (m.monstring);
    morewait();
    mprint ("Your opponent holds the only way you can leave!");
}

// loads the sorcereror's circle into Level
void load_circle (void)
{
    int safe = (Player.rank[CIRCLE] >= INITIATE);
    level::load_map (E_CIRCLE, Level_Circle, [&](char sc, location& s, unsigned i, unsigned j) {
	s.locchar = FLOOR;
	s.p_locf = L_NO_OP;
	s.lstatus = 0;
	s.roomnumber = RS_CIRCLE;
	switch (sc) {
	    case 'P': {
		monster& m = make_prime (i, j);
		m.specialf = M_SP_PRIME;
		if (!safe)
		    m_status_set (m, HOSTILE);
		break; }
	    case 'D': {
		monster& m = make_site_monster (i, j, DEMON_PRINCE);
		if (safe)
		    m_status_reset (m, HOSTILE);
		m.specialf = M_SP_COURT;
		break; }
	    case 's': {
		monster& m = make_site_monster (i, j, SERV_CHAOS);
		m.specialf = M_SP_COURT;
		if (safe)
		    m_status_reset (m, HOSTILE);
		break; }
	    case 'e': {
		monster& m = make_site_monster (i, j, ENCHANTOR);
		m.specialf = M_SP_COURT;
		if (safe)
		    m_status_reset (m, HOSTILE);
		break; }
	    case 'n': {
		monster& m = make_site_monster (i, j, NECROMANCER);
		m.specialf = M_SP_COURT;
		if (safe)
		    m_status_reset (m, HOSTILE);
		break; }
	    case 'T': {
		monster& m = make_site_monster (i, j, THAUMATURGIST);
		m.specialf = M_SP_COURT;
		if (safe)
		    m_status_reset (m, HOSTILE);
		break; }
	    case '#': s.locchar = WALL; s.aux = 255; break;
	    case 'L': s.p_locf = L_CIRCLE_LIBRARY; break;
	    case '?': s.p_locf = L_TOME1; break;
	    case '!': s.p_locf = L_TOME2; break;
	    case 'S': lset (i, j, SECRET); break;
	    case '-': s.locchar = CLOSED_DOOR; break;
	}
    });
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
void load_court (void)
{
    World.MoveInCountry(6,1);	// HyMagick can teleport directly here
    level::load_map (E_COURT, Level_Court, [&](char sc, location& s, unsigned i, unsigned j) {
	s.locchar = FLOOR;
	s.p_locf = L_NO_OP;
	s.lstatus = 0;
	s.roomnumber = RS_COURT;
	switch (sc) {
	    case '5': {
		s.locchar = CHAIR;
		s.p_locf = L_THRONE;
		make_specific_treasure (i, j, SCEPTRE_OF_HIGH_MAGIC);
		monster& m = make_site_monster (i, j, NPC);
		make_hiscore_npc (m, NPC_ARCHMAGE);
		m.specialf = M_SP_COURT;
		m_status_reset (m, HOSTILE);
		m_status_reset (m, MOBILE);
		break; }
	    case 'e': {
		monster& m = make_site_monster (i, j, ENCHANTOR);
		m_status_reset (m, HOSTILE);
		m.specialf = M_SP_COURT;
		break; }
	    case 'n': {
		monster& m = make_site_monster (i, j, NECROMANCER);
		m_status_reset (m, HOSTILE);
		m.specialf = M_SP_COURT;
		break; }
	    case 'T': {
		monster& m = make_site_monster (i, j, THAUMATURGIST);
		m_status_reset (m, HOSTILE);
		m.specialf = M_SP_COURT;
		break; }
	    case 'G': {
		monster& m = make_site_monster (i, j, GUARD);
		m_status_reset (m, HOSTILE);
		break; }
	    case '#': s.locchar = WALL; s.aux = 255; break;
	    case '<': s.locchar = STAIRS_UP; s.p_locf = L_ESCALATOR; break;
	}
    });
}

// loads the abyss level into Level
void load_abyss (void)
{
    level::load_map (E_ABYSS, Level_Abyss, [](char sc, location& s, unsigned, unsigned) {
	s.locchar = FLOOR;
	s.roomnumber = RS_ADEPT;
	switch (sc) {
	    case '0': s.locchar = VOID_CHAR;	s.p_locf = L_VOID; break;
	    case 'V': s.locchar = VOID_CHAR;	s.p_locf = L_VOID_STATION; break;
	    case '1':				s.p_locf = L_VOICE1; break;
	    case '2':				s.p_locf = L_VOICE2; break;
	    case '3':				s.p_locf = L_VOICE3; break;
	    case '~': s.locchar = WATER;	s.p_locf = L_WATER_STATION; break;
	    case ';': s.locchar = FIRE;		s.p_locf = L_FIRE_STATION; break;
	    case '(': s.locchar = HEDGE;	s.p_locf = L_EARTH_STATION; break;
	    case '6': s.locchar = WHIRLWIND;	s.p_locf = L_AIR_STATION; break;
	    case '#': s.locchar = WALL;		break;
	}
    });
}

static void SetCitySiteLocation (ELocation l, int x, int y)
{
    CitySiteList[l - CITYSITEBASE].known = true;
    CitySiteList[l - CITYSITEBASE].x = x;
    CitySiteList[l - CITYSITEBASE].y = y;
}

// loads the city level
void load_city (void)
{
    initrand (E_CITY, 0);
    Level->depth = 0;
    static const char* _mazes[] = { Level_Maze1, Level_Maze2, Level_Maze3, Level_Maze4 };
    const char* mazed = _mazes[xrand()%ArraySize(_mazes)];

    level::load_map (E_CITY, Level_City, [&](char sc, location& s, unsigned i, unsigned j) {
	s.locchar = FLOOR;
	s.lstatus |= SEEN;
	switch (sc) {
	    case 'g':
		s.p_locf = L_GARDEN;
		break;
	    case 'y':
		s.p_locf = L_CEMETARY;
		break;
	    case 'x':
		assign_city_function (s, i, j);
		break;
	    case 't':
		s.p_locf = L_TEMPLE;
		SetCitySiteLocation (L_TEMPLE, i, j);
		break;
	    case 'T':
		s.p_locf = L_PORTCULLIS_TRAP;
		s.aux = NOCITYMOVE;
		break;
	    case 'R':
		s.p_locf = L_RAISE_PORTCULLIS;
		s.aux = NOCITYMOVE;
		break;
	    case '7':
		s.p_locf = L_PORTCULLIS;
		s.aux = NOCITYMOVE;
		break;
	    case 'C':
		s.locchar = OPEN_DOOR;
		s.p_locf = L_COLLEGE;
		SetCitySiteLocation (L_COLLEGE, i, j);
		break;
	    case 's':
		s.locchar = OPEN_DOOR;
		s.p_locf = L_SORCERORS;
		SetCitySiteLocation (L_SORCERORS, i, j);
		break;
	    case 'M':
		s.locchar = OPEN_DOOR;
		s.p_locf = L_MERC_GUILD;
		SetCitySiteLocation (L_MERC_GUILD, i, j);
		break;
	    case 'c':
		s.locchar = OPEN_DOOR;
		s.p_locf = L_CASTLE;
		SetCitySiteLocation (L_CASTLE, i, j);
		break;
	    case 'm':
		mazesite (*mazed++, s, i, j);
		break;
	    case 'P':
		s.locchar = OPEN_DOOR;
		s.p_locf = L_ORDER;
		SetCitySiteLocation (L_ORDER, i, j);
		break;
	    case 'H':
		s.locchar = OPEN_DOOR;
		s.p_locf = L_CHARITY;
		SetCitySiteLocation (L_CHARITY, i, j);
		break;
	    case 'j':
		make_justiciar (i, j);
		break;
	    case 'J':
		s.locchar = CLOSED_DOOR;
		s.p_locf = L_JAIL;
		break;
	    case 'A':
		s.locchar = OPEN_DOOR;
		s.p_locf = L_ARENA;
		SetCitySiteLocation (L_ARENA, i, j);
		break;
	    case 'B':
		s.locchar = OPEN_DOOR;
		s.p_locf = L_BANK;
		SetCitySiteLocation (L_BANK, i, j);
		lset (i, j + 1, STOPS);
		lset (i + 1, j, STOPS);
		lset (i - 1, j, STOPS);
		lset (i, j - 1, STOPS);
		break;
	    case 'X':
		s.p_locf = L_COUNTRYSIDE;
		SetCitySiteLocation (L_COUNTRYSIDE, i, j);
		break;
	    case 'v':
		s.p_locf = L_VAULT;
		s.aux = NOCITYMOVE;
		lset (i, j, SECRET);
		break;
	    case 'S':
		s.aux = NOCITYMOVE;
		lset (i, j, SECRET);
		break;
	    case 'G': {
		monster& m = make_site_monster (i, j, GUARD);
		m.aux1 = i;
		m.aux2 = j;
		break; }
	    case 'u':
		make_minor_undead (i, j);
		break;
	    case 'U':
		make_major_undead (i, j);
		break;
	    case 'V':
		s.p_locf = L_VAULT;
		make_site_treasure (i, j, 5);
		s.aux = NOCITYMOVE;
		lset (i, j, SECRET);
		break;
	    case '%':
		s.p_locf = L_TRAP_SIREN;
		make_site_treasure (i, j, 5);
		s.aux = NOCITYMOVE;
		lset (i, j, SECRET);
		break;
	    case '$':
		make_site_treasure (i, j, 5);
		break;
	    case '2':
		s.locchar = ALTAR;
		s.p_locf = L_ALTAR;
		s.aux = ODIN;
		break;
	    case '3':
		s.locchar = ALTAR;
		s.p_locf = L_ALTAR;
		s.aux = SET;
		break;
	    case '4':
		s.locchar = ALTAR;
		s.p_locf = L_ALTAR;
		s.aux = ATHENA;
		break;
	    case '5':
		s.locchar = ALTAR;
		s.p_locf = L_ALTAR;
		s.aux = HECATE;
		break;
	    case '6':
		s.locchar = ALTAR;
		s.p_locf = L_ALTAR;
		s.aux = DESTINY;
		break;
	    case '^':
		s.p_locf = TRAP_BASE + random_range (NUMTRAPS);
		lset (i, j, SECRET);
		break;
	    case '(':
		s.locchar = HEDGE;
		break;
	    case '~':
		s.locchar = WATER;
		s.p_locf = L_WATER;
		break;
	    case '=':
		s.locchar = WATER;
		s.p_locf = L_MAGIC_POOL;
		break;
	    case '*':
		s.locchar = WALL;
		s.aux = 10;
		break;
	    case '#':
		s.locchar = WALL;
		s.aux = 255;
		break;
	    case ',':
		s.aux = NOCITYMOVE;
		lset (i, j, SECRET);
		break;
	    case '-':
		s.locchar = CLOSED_DOOR;
		break;
	    case '1':
		s.locchar = STATUE;
		break;
	}
    });

    // make all city monsters asleep, and shorten their wakeup range to 2
    // to prevent players from being molested by vicious monsters on the streets
    foreach (m, Level->mlist) {
	m_status_set (*m, ASLEEP);
	m->wakeup = 2;
    }
    initrand (E_RESTORE, 0);
}

static void assign_city_function (location& s, int x, int y)
{
    static uint8_t permutation[64];	// number of x's in city map
    static unsigned next = 0;
    static bool setup = false;
    if (!setup) {
	iota (ArrayRange(permutation), 0);
	random_shuffle (ArrayRange(permutation));
	setup = true;
    }

    s.locchar = OPEN_DOOR;
    s.aux = true;

    lset (x, y + 1, STOPS);
    lset (x + 1, y, STOPS);
    lset (x - 1, y, STOPS);
    lset (x, y - 1, STOPS);
    lset (x, y, STOPS);

    static const uint8_t _citylocs[] = {
	L_ARMORER, L_CLUB, L_GYM, L_THIEVES_GUILD, L_HEALER,
	L_CASINO, L_DINER, L_CRAP, L_COMMANDANT, L_COMMANDANT,
	L_COMMANDANT, L_TAVERN, L_ALCHEMIST, L_DPW, L_LIBRARY,
	L_PAWN_SHOP, L_CONDO, L_BROTHEL
    };
    unsigned loc;
    if (next >= ArraySize(permutation) || ArraySize(_citylocs) <= (loc = permutation[next++])) {
	s.locchar = CLOSED_DOOR;
	unsigned ht = xrand() % 8;
	s.p_locf = (ht < 2 ? L_HOVEL : (ht > 6 ? L_MANSION : L_HOUSE));
	if (xrand()%4)
	    s.aux = LOCKED;
    } else {
	uint8_t sitef = _citylocs[loc];
	CitySiteList[sitef - CITYSITEBASE].x = x;
	CitySiteList[sitef - CITYSITEBASE].y = y;
	s.p_locf = sitef;
	if (sitef == L_BROTHEL || sitef == L_THIEVES_GUILD)
	    s.locchar = CLOSED_DOOR;
    }
}

// makes a hiscore npc for mansions
static void make_justiciar (int i, int j)
{
    monster& m = make_site_monster (i, j, NPC);
    make_hiscore_npc (m, NPC_JUSTICIAR);
    m.click = (Tick + 1) % 60;
    m_status_set (m, ASLEEP);
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
	    }
	}
    }
}

static void mazesite (char c, location& s, int i, int j)
{
    switch (c) {
	case '(':
	    s.locchar = HEDGE;
	    s.p_locf = L_HEDGE;
	    if (!random_range(10))
		s.p_locf = L_TRIFID;
	    break;
	case '-':
	    s.locchar = CLOSED_DOOR;
	    break;
	case '>':
	    s.locchar = STAIRS_DOWN;
	    s.p_locf = L_SEWER;
	    break;
	case 'z':
	    s.p_locf = L_MAZE;
	    break;
	case 'O':
	    s.locchar = OPEN_DOOR;
	    s.p_locf = L_ORACLE;
	    CitySiteList[L_ORACLE - CITYSITEBASE].x = i;
	    CitySiteList[L_ORACLE - CITYSITEBASE].y = j;
	    break;
	case 's':
	    switch (xrand()%4) {
		case 0: s.p_locf = TRAP_BASE + random_range (NUMTRAPS); break;
		case 1: make_site_monster (i, j, RANDOM); break;
		case 2: make_site_treasure (i, j, 5); break;
	    }
	    break;
    }
    lreset (i, j, SEEN);
}

// undead are not hostile unless disturbed....
static void make_minor_undead (int i, int j)
{
    monster& m = make_site_monster (i, j, random_range(2) ? GHOST : HAUNT);
    m_status_set (m, ASLEEP);
    m_status_reset (m, HOSTILE);
}

// undead are not hostile unless disturbed....
static void make_major_undead (int i, int j)
{
    monster& m = make_site_monster (i, j, random_range(2) ? LICHE : VAMP_LORD);
    m_status_set (m, ASLEEP);
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
    for (unsigned j = 0; j < ArraySize(jail); ++j) {
	for (unsigned i = 0; i < ArraySize(jail[0]); ++i) {
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
	    location& s (Level->site(i+35,j+52));
	    s.locchar = locchar;
	    s.p_locf = p_locf;
	    s.aux = aux;
	    lreset (i+35, j+52, CHANGED);
	}
    }
}

// loads the countryside level from the data file
void load_country (void)
{
    level::load_map (E_COUNTRYSIDE, Level_Country, [&](char sc, location& s, unsigned, unsigned) {
	s.aux = 0;
	s.lstatus = 0;
	switch (sc) {
	    case char(PASS):
		s.locchar = PASS;
		s.lstatus |= SECRET;
		break;
	    case char(CASTLE):
		s.locchar = CASTLE;
		s.lstatus |= SECRET;
		break;
	    case char(STARPEAK):
		s.locchar = STARPEAK;
		s.lstatus |= SECRET;
		break;
	    case char(CAVES):
		s.locchar = CAVES;
		s.lstatus |= SECRET;
		break;
	    case char(VOLCANO):
		s.locchar = VOLCANO;
		s.lstatus |= SECRET;
		break;
	    case char(DRAGONLAIR):
		s.locchar = DRAGONLAIR;
		s.lstatus |= SECRET;
		break;
	    case char(MAGIC_ISLE):
		s.locchar = MAGIC_ISLE;
		s.lstatus |= SECRET;
		break;
	    case 'a':
	    case 'b':
	    case 'c':
	    case 'd':
	    case 'e':
	    case 'f':
		s.locchar = VILLAGE;
		s.aux = sc - 'a';
		break;
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
		s.locchar = TEMPLE;
		s.aux = sc - '1';
		break;
	    case char(PLAINS):
		s.locchar = PLAINS;
		break;
	    case char(TUNDRA):
		s.locchar = TUNDRA;
		break;
	    case char(ROAD):
		s.locchar = ROAD;
		break;
	    case char(MOUNTAINS):
		s.locchar = MOUNTAINS;
		break;
	    case char(RIVER):
		s.locchar = RIVER;
		break;
	    case char(CITY):
		s.locchar = CITY;
		break;
	    case char(FOREST):
		s.locchar = FOREST;
		break;
	    case char(JUNGLE):
		s.locchar = JUNGLE;
		break;
	    case char(SWAMP):
		s.locchar = SWAMP;
		break;
	    case char(DESERT):
		s.locchar = DESERT;
		break;
	    case char(CHAOS_SEA):
		s.locchar = CHAOS_SEA;
		break;
	}
    });
}

// loads the dragon's lair into Level
void load_dlair (int empty)
{
    if (empty) {
	mprint ("The Lair is now devoid of inhabitants and treasure.");
	morewait();
    }
    level::load_map (E_DLAIR, Level_DragonLair, [empty](char sc, location& s, unsigned i, unsigned j) {
	s.locchar = FLOOR;
	s.p_locf = L_NO_OP;
	s.lstatus = 0;
	if (i < 48)
	    s.roomnumber = RS_CAVERN;
	else
	    s.roomnumber = RS_DRAGONLORD;
	switch (sc) {
	    case 'D':
		if (!empty) {
		    monster& m = make_site_monster (i, j, DRAGON_LORD);
		    m.specialf = M_SP_LAIR;
		}
		break;
	    case 'd':
		if (!empty) {
		    monster& m = make_site_monster (i, j, DRAGON);
		    m.specialf = M_SP_LAIR;
		    m.hit *= 2;	// elite dragons, actually
		    m.dmg *= 2;
		}
		break;
	    case 'W':
		if (!empty)
		    make_site_monster (i, j, KING_WYV);
		break;
	    case 'M':
		if (!empty)
		    make_site_monster (i, j, RANDOM);
		break;
	    case 'S':
		lset (i, j, SECRET);
		s.roomnumber = RS_SECRETPASSAGE;
		break;
	    case '$':
		if (!empty)
		    make_site_treasure (i, j, 10);
		break;
	    case 's':
		s.p_locf = L_TRAP_SIREN;
		break;
	    case '7':
		if (!empty)
		    s.locchar = PORTCULLIS;
		s.p_locf = L_PORTCULLIS;
		break;
	    case 'R':
		s.p_locf = L_RAISE_PORTCULLIS;
		break;
	    case 'p':
		s.p_locf = L_PORTCULLIS;
		break;
	    case 'T':
		if (!empty)
		    s.p_locf = L_PORTCULLIS_TRAP;
		break;
	    case 'X':
		s.p_locf = L_TACTICAL_EXIT;
		break;
	    case '#':
		s.locchar = WALL;
		s.aux = 150;
		break;
	}
    });
}

// loads the star peak into Level
void load_speak (int empty)
{
    const bool safe = Player.alignment > 0;
    if (empty) {
	mprint ("The peak is now devoid of inhabitants and treasure.");
	morewait();
    }
    level::load_map (E_STARPEAK, Level_StarPeak, [&](char sc, location& s, unsigned i, unsigned j) {
	s.locchar = FLOOR;
	s.lstatus = 0;
	s.roomnumber = RS_STARPEAK;
	s.p_locf = L_NO_OP;
	switch (sc) {
	    case 'S':
		lset (i, j, SECRET);
		s.roomnumber = RS_SECRETPASSAGE;
		break;
	    case 'L':
		if (!empty) {
		    monster& m = make_site_monster (i, j, LAWBRINGER);
		    if (safe)
			m_status_reset (m, HOSTILE);
		}
		break;
	    case 's':
		if (!empty) {
		    monster& m = make_site_monster (i, j, SERV_LAW);
		    if (safe)
			m_status_reset (m, HOSTILE);
		}
		break;
	    case 'M':
		if (!empty) {
		    monster& m = make_site_monster (i, j, -1);
		    if (safe)
			m_status_reset (m, HOSTILE);
		}
		break;
	    case '$':
		if (!empty)
		    make_site_treasure (i, j, 10);
		break;
	    case '7':
		if (!empty)
		    s.locchar = PORTCULLIS;
		s.p_locf = L_PORTCULLIS;
		break;
	    case 'R':
		s.p_locf = L_RAISE_PORTCULLIS;
		break;
	    case '-':
		s.locchar = CLOSED_DOOR;
		break;
	    case '|':
		s.locchar = OPEN_DOOR;
		break;
	    case 'p':
		s.p_locf = L_PORTCULLIS;
		break;
	    case 'T':
		if (!empty)
		    s.p_locf = L_PORTCULLIS_TRAP;
		break;
	    case 'X':
		s.p_locf = L_TACTICAL_EXIT;
		break;
	    case '#':
		s.locchar = WALL;
		s.aux = 150;
		break;
	    case '4':
		s.locchar = RUBBLE;
		s.p_locf = L_RUBBLE;
		break;
	}
    });
}

// loads the magic isle into Level
void load_misle (int empty)
{
    if (empty) {
	mprint ("The isle is now devoid of inhabitants and treasure.");
	morewait();
    }
    level::load_map (E_MAGIC_ISLE, Level_MagicIsle, [&](char sc, location& s, unsigned i, unsigned j) {
	s.locchar = FLOOR;
	s.lstatus = 0;
	s.roomnumber = RS_MAGIC_ISLE;
	s.p_locf = L_NO_OP;
	switch (sc) {
	    case 'E':
		if (!empty)
		    make_site_monster (i, j, EATER);
		break;
	    case 'm':
		if (!empty)
		    make_site_monster (i, j, MIL_PRIEST);
		break;
	    case 'n':
		if (!empty)
		    make_site_monster (i, j, NAZGUL);
		break;
	    case 'X':
		s.p_locf = L_TACTICAL_EXIT;
		break;
	    case '#':
		s.locchar = WALL;
		s.aux = 150;
		break;
	    case '4':
		s.locchar = RUBBLE;
		s.p_locf = L_RUBBLE;
		break;
	    case '~':
		s.locchar = WATER;
		s.p_locf = L_CHAOS;
		break;
	    case '=':
		s.locchar = WATER;
		s.p_locf = L_MAGIC_POOL;
		break;
	    case '-':
		s.locchar = CLOSED_DOOR;
		break;
	    case '|':
		s.locchar = OPEN_DOOR;
		break;
	}
    });
}

// loads a temple into Level
void load_temple (int deity)
{
    Level->SetTempleDeity (deity);
    const uint8_t roomid = deity - ODIN + RS_ODIN;
    level::load_map (E_TEMPLE, Level_Temple, [&](char sc, location& s, unsigned i, unsigned j) {
	s.locchar = FLOOR;
	s.roomnumber = roomid;
	switch (sc) {
	    case '8':
		s.locchar = ALTAR;
		s.p_locf = L_ALTAR;
		s.aux = deity;
		break;
	    case 'H':
		if (!Player.patron || Player.name != Priest[Player.patron] || Player.rank[PRIESTHOOD] != HIGHPRIEST)
		    make_high_priest (i, j, deity);
		break;
	    case 'S':
		if (!Player.patron || Player.name != Priest[Player.patron] || Player.rank[PRIESTHOOD] != HIGHPRIEST)
		    lset (i, j, SECRET);
		break;
	    case 'W':
		if (deity != Player.patron && deity != DRUID)
		    s.p_locf = L_TEMPLE_WARNING;
		break;
	    case 'm':
		make_site_monster (i, j, MIL_PRIEST);
		break;
	    case 'd':
		make_site_monster (i, j, DOBERMAN);
		break;
	    case 'X':
		s.p_locf = L_TACTICAL_EXIT;
		break;
	    case '#':
		if (deity != DRUID) {
		    s.locchar = WALL;
		    s.aux = 150;
		} else {
		    s.locchar = HEDGE;
		    s.p_locf = L_HEDGE;
		}
		break;
	    case 'x':
		random_temple_site (i, j, deity);
		break;
	    case '?':
		if (deity == DESTINY) {
		    s.locchar = ABYSS;
		    s.p_locf = L_ADEPT;
		}
		break;
	    case '-':
		s.locchar = CLOSED_DOOR;
		break;
	    case '|':
		s.locchar = OPEN_DOOR;
		break;
	}
    });
    // Main Temple is peaceful for player of same sect, druids always peaceful.
    if (Player.patron == deity || deity == DRUID)
	foreach (m, Level->mlist)
	    m_status_reset (*m, HOSTILE);
}

static void random_temple_site (int i, int j, int deity UNUSED)
{
    switch (random_range (12)) {
	case 0:
	    make_site_monster (i, j, MEND_PRIEST);
	    break;
	case 1:
	    Level->site(i,j).locchar = WATER;
	    Level->site(i,j).p_locf = L_MAGIC_POOL;
	case 2:
	    make_site_monster (i, j, INNER_DEMON);
	    break;
	case 3:
	    make_site_monster (i, j, ANGEL);
	    break;
	case 4:
	    make_site_monster (i, j, HIGH_ANGEL);
	    break;
	case 5:
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
    mprint ("Legion of Destiny, Mercenary Guild, Inc.");
    if (nighttime())
	mprint ("The barracks are under curfew right now.");
    else {
	mprint ("You enter Legion HQ, ");
	if (Player.rank[LEGION] == COMMANDANT) {
	    mprint ("Your aide follows you to the staff room.");
	    morewait();
	    clearmsg();
	}
	if (Player.rank[LEGION] >= LEGIONAIRE) {
	    mprint ("and report to your commander.");
	    morewait();
	}
	switch (Player.rank[LEGION]) {
	    case 0:
		mprint ("and see the Recruiting Centurion.");
		morewait();
		mprint ("Do you wish to join the Legion? [yn] ");
		if (ynq() == 'y') {
		    clearmsg();
		    if (Player.rank[ARENA] >= TRAINEE) {
			mprint ("The Centurion checks your record, and gets angry:");
			mprint ("'The legion don't need any Arena Jocks. Git!'");
		    } else if (Player.rank[ORDER] >= GALLANT) {
			mprint ("The Centurion checks your record, and is amused:");
			mprint ("'A paladin in the ranks? You must be joking.'");
		    } else if (Player.con < 12) {
			mprint ("The Centurion looks you over, sadly.");
			mprint ("You are too fragile to join the legion.");
		    } else if (Player.str < 10) {
			mprint ("The Centurion looks at you contemptuously.");
			mprint ("Your strength is too low to pass the physical!");
		    } else {
			mprint ("You are tested for strength and stamina... and you pass!");
			mprintf ("Commandant %s shakes your hand.", Commandant);
			morewait();
			mprint ("The Legion pays you a 500Au induction fee.");
			morewait();
			mprint ("You are also issued a shortsword and leather.");
			mprint ("You are now a Legionaire.");
			morewait();
			clearmsg();
			gain_item (Objects[WEAPON_SHORT_SWORD]);
			gain_item (Objects[ARMOR_SOFT_LEATHER]);
			Player.cash += 500;
			Player.rank[LEGION] = LEGIONAIRE;
			Player.guildxp[LEGION] = 1;
			++Player.str;
			++Player.con;
			++Player.maxstr;
			++Player.maxcon;
		    }
		}
		break;
	    case COMMANDANT:
		mprint ("You find the disposition of your forces satisfactory.");
		break;
	    case COLONEL:
		if ((Player.level > Commandantlevel) && find_and_remove_item (CORPSEID, DEMON_EMP)) {
		    mprint ("You liberated the Demon Emperor's Regalia!");
		    morewait();
		    clearmsg();
		    mprint ("The Legion is assembled in salute to you!");
		    mprint ("The Regalia is held high for all to see and admire.");
		    morewait();
		    clearmsg();
		    mprintf ("Commandant %s promotes you to replace him,", Commandant);
		    mprint ("and announces his own overdue retirement.");
		    morewait();
		    clearmsg();
		    mprint ("You are the new Commandant of the Legion!");
		    mprint ("The Emperor's Regalia is sold for a ridiculous sum.");
		    morewait();
		    clearmsg();
		    mprint ("You now know the Spell of Regeneration.");
		    learn_spell (S_REGENERATE);
		    Player.rank[LEGION] = COMMANDANT;
		    Player.maxstr += 2;
		    Player.str += 2;
		    Player.maxcon += 2;
		    Player.con += 2;
		    mprint ("Your training is complete. You get top salary.");
		    Player.cash += 20000;
		} else if (Player.level <= Commandantlevel) {
		    clearmsg();
		    mprint ("Your CO expresses satisfaction with your progress.");
		    mprint ("But your service record does not yet permit promotion.");
		} else {
		    clearmsg();
		    mprint ("Why do you come empty handed?");
		    mprint ("You must return with the Regalia of the Demon Emperor!");
		}
		break;
	    case FORCE_LEADER:
		clearmsg();
		mprint ("Your CO expresses satisfaction with your progress.");
		if (Player.guildxp[LEGION] < 4000)
		    mprint ("But your service record does not yet permit promotion.");
		else {
		    mprint ("You have been promoted to Legion Colonel!");
		    morewait();
		    mprint ("Your next promotion is contingent on the return of");
		    mprint ("the Regalia of the Demon Emperor.");
		    morewait();
		    mprint ("The Demon Emperor holds court at the base of a volcano");
		    mprint ("to the far south, in the heart of a swamp.");
		    morewait();
		    clearmsg();
		    mprint ("You have been taught the spell of heroism!");
		    learn_spell (S_HERO);
		    Player.rank[LEGION] = COLONEL;
		    ++Player.maxstr;
		    ++Player.str;
		    ++Player.maxcon;
		    ++Player.con;
		    mprint ("You are given advanced training, and a raise.");
		    Player.cash += 10000;
		}
		break;
	    case CENTURION:
		clearmsg();
		mprint ("Your CO expresses satisfaction with your progress.");
		if (Player.guildxp[LEGION] < 1500)
		    mprint ("But your service record does not yet permit promotion.");
		else {
		    mprint ("You are now a Legion Force-Leader!");
		    Player.rank[LEGION] = FORCE_LEADER;
		    Player.maxstr++;
		    Player.str++;
		    morewait();
		    clearmsg();
		    mprint ("You receive more training, and bonus pay.");
		    Player.cash += 5000;
		}
		break;
	    case LEGIONAIRE:
		clearmsg();
		mprint ("Your CO expresses satisfaction with your progress.");
		if (Player.guildxp[LEGION] < 400)
		    mprint ("But your service record does not yet permit promotion.");
		else {
		    mprint ("You are promoted to Legion Centurion!");
		    morewait();
		    clearmsg();
		    mprint ("You get advanced training, and a higher salary.");
		    Player.rank[LEGION] = CENTURION;
		    ++Player.maxcon;
		    ++Player.con;
		    Player.cash += 2000;
		}
		break;
	}
    }
}

void l_castle (void)
{
    if (Player.level < 3) {
	mprint ("You can't possibly enter the castle, you nobody!");
	mprint ("Come back when you are famous.");
    } else {
	mprint ("You are ushered into the castle.");
	if (Player.rank[NOBILITY] < DUKE) {
	    mprintf ("His Grace, %s -- Duke of Rampart! <fanfare>", Duke);
	    morewait();
	    clearmsg();
	}
	if (Player.rank[NOBILITY] == NOT_NOBILITY) {
	    mprint ("Well, sirrah, wouldst embark on a quest? [yn] ");
	    if (ynq() == 'y') {
		mprint ("Splendid. Bring me the head of the Goblin King.");
		Player.rank[NOBILITY] = COMMONER;
	    } else {
		mprint ("You scoundrel! Guards! Take this blackguard away!");
		morewait();
		p_damage (25, UNSTOPPABLE, "castle guards for lese majeste");
		send_to_jail();
	    }
	} else if (Player.rank[NOBILITY] == COMMONER) {
	    if (find_and_remove_item (CORPSEID, GOBLIN_KING)) {
		mprint ("Good job, sirrah! I promote you to the rank of esquire.");
		Player.rank[NOBILITY] = ESQUIRE;
		gain_experience (100);
		mprint ("Now that you have proved yourself true, another quest!");
		morewait();
		mprint ("Bring to me a Holy Defender!");
		mprint ("One is said to be in the possession of the Great Wyrm");
		morewait();
		clearmsg();
		mprint ("in the depths of the sewers below the city.");
	    } else
		mprint ("Do not return until you achieve the quest, caitiff!");
	} else if (Player.rank[NOBILITY] == ESQUIRE) {
	    if (find_and_remove_item (WEAPON_DEFENDER, -1)) {
		mprint ("My thanks, squire. In return, I dub thee knight!");
		Player.rank[NOBILITY] = KNIGHT;
		gain_experience (1000);
		mprint ("If thou wouldst please me further...");
		morewait();
		mprint ("Bring me a suit of dragonscale armor.");
		mprint ("You might have to kill a dragon to get one....");
	    } else
		mprint ("Greetings, squire. My sword? What, you don't have it?");
	} else if (Player.rank[NOBILITY] == KNIGHT) {
	    if (find_and_remove_item (ARMOR_DRAGONSCALE, -1)) {
		mprint ("Thanks, good sir knight.");
		mprint ("Here are letters patent to a peerage!");
		Player.rank[NOBILITY] = LORD;
		gain_experience (10000);
		morewait();
		mprint ("If you would do me a final service...");
		mprint ("I require the Orb of Mastery. If you would be so kind...");
		morewait();
		mprint ("By the way, you might find the Orb in the possession");
		mprint ("Of the Elemental Master on the Astral Plane");
	    } else
		mprint ("Your quest is not yet complete, sir knight.");
	} else if (Player.rank[NOBILITY] == LORD) {
	    if (find_item (ORB_OF_MASTERY)) {
		mprint ("My sincerest thanks, my lord.");
		mprint ("You have proved yourself a true paragon of chivalry");
		morewait();
		mprint ("I abdicate the Duchy in your favor....");
		mprint ("Oh, you can keep the Orb, by the way....");
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
		mprint ("I didn't really think you were up to the task....");
	}
    }
}

void l_arena (void)
{
    char response;
    int prize;

    mprint ("Rampart Coliseum");
    if (Player.rank[ARENA] == NOT_IN_ARENA) {
	mprint ("Enter the games, or Register as a Gladiator? [e,r,ESCAPE] ");
	do
	    response = (char) mcigetc();
	while (response != 'e' && response != 'r' && response != KEY_ESCAPE);
    } else {
	mprint ("Enter the games? [yn] ");
	response = ynq();
	if (response == 'y')
	    response = 'e';
	else
	    response = KEY_ESCAPE;
    }
    if (response == 'r') {
	if (Player.rank[ARENA] >= TRAINEE)
	    mprint ("You're already a gladiator....");
	else if (Player.rank[ORDER] >= GALLANT)
	    mprint ("We don't let Paladins into our Guild.");
	else if (Player.rank[LEGION] >= LEGIONAIRE)
	    mprint ("We don't train no stinkin' mercs!");
	else if (Player.str < 13)
	    mprint ("Yer too weak to train!");
	else if (Player.agi < 12)
	    mprint ("Too clumsy to be a gladiator!");
	else {
	    mprint ("Ok, yer now an Arena Trainee.");
	    mprint ("Here's a wooden sword, and a shield");
	    morewait();
	    clearmsg();
	    gain_item (Objects[WEAPON_CLUB]);
	    gain_item (Objects[SHIELD_SMALL_ROUND]);
	    Player.rank[ARENA] = TRAINEE;
	    Arena_Opponent = 3;
	    morewait();
	    clearmsg();
	    mprint ("You've got 5000Au credit at the Gym.");
	    Gymcredit += 5000;
	}
    } else if (response == 'e') {
	mprint ("OK, we're arranging a match....");
	morewait();
	clearmsg();
	change_environment (E_ARENA);
	mprint ("Let the battle begin....");

	time_clock (true);
	while (Level->environment == E_ARENA)
	    time_clock (false);

	if (!Arena_Victory) {
	    mprint ("The crowd boos your craven behavior!!!");
	    if (Player.rank[ARENA] >= TRAINEE) {
		mprint ("You are thrown out of the Gladiator's Guild!");
		morewait();
		clearmsg();
		if (Gymcredit > 0)
		    mprint ("Your credit at the gym is cut off!");
		Gymcredit = 0;
		Player.rank[ARENA] = FORMER_GLADIATOR;
	    }
	} else {
	    Arena_Opponent++;
	    if (Arena_Victory == 21) {
		mprint ("The crowd roars its approval!");
		if (Player.rank[ARENA]) {
		    mprint ("You are the new Arena Champion!");
		    Player.rank[ARENA] = CHAMPION;
		    morewait();
		    mprint ("You are awarded the Champion's Spear: Victrix!");
		    morewait();
		    gain_item (Objects[WEAPON_VICTRIX]);

		} else {
		    mprint ("As you are not an official gladiator, you are not made Champion.");
		    morewait();
		}
	    }
	    morewait();
	    clearmsg();
	    prize = max (25, (Arena_Victory-1) * 50);
	    if (Player.rank[ARENA] >= TRAINEE)
		prize *= 2;
	    mprintf ("Good fight! Your prize is: %uAu", prize);
	    Player.cash += prize;
	    if (Player.rank[ARENA] < GLADIATOR && Arena_Opponent > 5 && !(Arena_Opponent % 3)) {
		if (Player.rank[ARENA] >= TRAINEE) {
		    ++Player.rank[ARENA];
		    morewait();
		    mprint ("You've been promoted to a stronger class!");
		    mprint ("You are also entitled to additional training.");
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
    mprint ("You have penetrated to the Lair of the Thieves' Guild.");
    if (!nighttime())
	mprint ("There aren't any thieves around in the daytime.");
    else {
	if (Player.rank[THIEVES] == TMASTER && Player.level > Shadowlordlevel && find_and_remove_item(THING_JUSTICIAR_BADGE, -1)) {
	    mprint ("You nicked the Justiciar's Badge!");
	    morewait();
	    mprint ("The Badge is put in a place of honor in the Guild Hall.");
	    mprint ("You are now the Shadowlord of the Thieves' Guild!");
	    morewait();
	    mprint ("Who says there's no honor among thieves?");
	    morewait();
	    clearmsg();
	    mprint ("You learn the Spell of Shadowform.");
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
		    mprint ("You are already a member!");
		else if (Player.alignment > 10)
		    mprint ("You are too lawful to be a thief!");
		else {
		    dues += dues * (12 - Player.dex) / 9;
		    dues += Player.alignment * 5;
		    dues = max (100U, dues);
		    clearmsg();
		    mprintf ("Dues are %uAu. Pay it? [yn] ", dues);
		    if (ynq() == 'y') {
			if (Player.cash < dues) {
			    mprint ("You can't cheat the Thieves' Guild!");
			    mprint ("... but the Thieves' Guild can cheat you....");
			    Player.cash = 0;
			} else {
			    mprintf ("Shadowlord %s enters your name into the roll of the Guild.", Shadowlord);
			    morewait();
			    clearmsg();
			    mprint ("As a special bonus, you get a free lockpick.");
			    mprint ("You are taught the spell of Object Detection.");
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
		    mprint ("You are not even a member!");
		else if (Player.rank[THIEVES] == SHADOWLORD)
		    mprint ("You can't get any higher than this!");
		else if (Player.rank[THIEVES] == TMASTER) {
		    if (Player.level <= Shadowlordlevel)
			mprint ("You are not experienced enough to advance.");
		    else
			mprint ("You must bring back the Justiciar's Badge!");
		} else if (Player.rank[THIEVES] == THIEF) {
		    if (Player.guildxp[THIEVES] < 4000)
			mprint ("You are not experienced enough to advance.");
		    else {
			mprint ("You are now a Master Thief of the Guild!");
			mprint ("You are taught the Spell of Apportation.");
			morewait();
			mprint ("To advance to the next level you must return with");
			mprint ("the badge of the Justiciar (cursed be his name).");
			morewait();
			clearmsg();
			mprint ("The Justiciar's office is just south of the gaol.");
			learn_spell (S_APPORT);
			Player.rank[THIEVES] = TMASTER;
			Player.maxagi++;
			Player.maxdex++;
			Player.agi++;
			Player.dex++;
		    }
		} else if (Player.rank[THIEVES] == ATHIEF) {
		    if (Player.guildxp[THIEVES] < 1500)
			mprint ("You are not experienced enough to advance.");
		    else {
			mprint ("You are now a ranking Thief of the Guild!");
			mprint ("You learn the Spell of Invisibility.");
			learn_spell (S_INVISIBLE);
			Player.rank[THIEVES] = THIEF;
			Player.agi++;
			Player.maxagi++;
		    }
		} else if (Player.rank[THIEVES] == TMEMBER) {
		    if (Player.guildxp[THIEVES] < 400)
			mprint ("You are not experienced enough to advance.");
		    else {
			mprint ("You are now an Apprentice Thief!");
			mprint ("You are taught the Spell of Levitation.");
			learn_spell (S_LEVITATE);
			Player.rank[THIEVES] = ATHIEF;
			Player.dex++;
			Player.maxdex++;
		    }
		}
	    } else if (action == 'c') {
		fee = (Player.rank[THIEVES] == NOT_A_THIEF) ? 50 : 5;
		mprintf ("RTG, Inc, Appraisers. Identification Fee: %uAu/item.", fee);
		mprint ("Identify one item, or all possessions? [ip] ");
		if ((char) mcigetc() == 'i') {
		    if (Player.cash < fee)
			mprint ("Try again when you have the cash.");
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
			count += !object_is_known(&*i);
		    clearmsg();
		    mprintf ("The fee will be: %uAu. Pay it? [yn] ", max (count*fee, fee));
		    if (ynq() == 'y') {
			if (Player.cash < max (count * fee, fee))
			    mprint ("Try again when you have the cash.");
			else {
			    Player.cash -= max (count * fee, fee);
			    dataprint();
			    identify (1);
			}
		    }
		}
	    } else if (action == 'd') {
		if (Player.rank[THIEVES] == NOT_A_THIEF)
		    mprint ("Fence? Who said anything about a fence?");
		else {
		    mprint ("Fence one item or go through pack? [ip] ");
		    if ((char) mcigetc() == 'i') {
			int i = getitem (NULL_ITEM);
			if (i == ABORT || !Player.has_possession(i))
			    mprint ("Huh, Is this some kind of set-up?");
			else if (Player.possessions[i].blessing < 0)
			    mprint ("I don't want to buy a cursed item!");
			else {
			    clearmsg();
			    unsigned cost = 2 * item_value (Player.possessions[i]) / 3;
			    mprintf ("I'll give you %u Au each. OK? [yn] ", cost);
			    if (ynq() == 'y') {
				number = getnumber (Player.possessions[i].number);
				// Fenced artifacts could turn up anywhere, really...
				if (object_uniqueness(Player.possessions[i]) > UNIQUE_UNMADE)
				    set_object_uniqueness (Player.possessions[i], UNIQUE_UNMADE);
				Player.remove_possession (i, number);
				Player.cash += number * cost;
				dataprint();
			    } else
				mprint ("Hey, gimme a break, it was a fair price!");
			}
		    } else {
			foreach (i, Player.pack) {
			    if (i->blessing > -1) {
				clearmsg();
				mprintf ("Sell %s for %u Au each? [ynq] ", itemid(&*i), 2*item_value(&*i)/3);
				if ((c = ynq()) == 'q')
				    break;
				else if (c == 'y') {
				    number = getnumber (i->number);
				    Player.cash += 2 * number * item_value (&*i) / 3;
				    if ((i->number -= number) < 1) {
					// Fenced an artifact?  You just might see it again.
					if (object_uniqueness(&*i) > UNIQUE_UNMADE)
					    set_object_uniqueness (&*i, UNIQUE_UNMADE);
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
    mprint ("The Collegium Magii. Founded 16937, AOF.");
    if (nighttime())
	mprint ("The Registration desk is closed at night....");
    else {
	while (!done) {
	    if ((Player.rank[COLLEGE] == MAGE) && (Player.level > Archmagelevel) && find_and_remove_item (CORPSEID, EATER)) {
		mprint ("You brought back the heart of the Eater of Magic!");
		morewait();
		mprint ("The Heart is sent to the labs for analysis.");
		mprint ("The Board of Trustees appoints you Archmage!");
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
		    mprint ("You are already enrolled!");
		else if (Player.iq < 13)
		    mprint ("Your low IQ renders you incapable of being educated.");
		else if (Player.rank[CIRCLE] >= INITIATE)
		    mprint ("Sorcery and our Magic are rather incompatable, no?");
		else {
		    if (Player.iq > 17) {
			mprint ("You are given a scholarship!");
			morewait();
			enrolled = true;
		    } else {
			mprint ("Tuition is 1000Au. Pay it? [yn] ");
			if (ynq() == 'y') {
			    if (Player.cash < 1000)
				mprint ("You don't have the funds!");
			    else {
				Player.cash -= 1000;
				enrolled = true;
				dataprint();
			    }
			}
		    }
		    if (enrolled) {
			mprintf ("Archmage %s greets you and congratulates you on your acceptance.", Archmage);
			mprint ("You are now enrolled in the Collegium Magii!");
			morewait();
			mprint ("You are now a Novice.");
			mprint ("You may research 1 spell, for your intro class.");
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
		    mprint ("You have not even been initiated, yet!");
		else if (Player.rank[COLLEGE] == ARCHMAGE)
		    mprint ("You are at the pinnacle of mastery in the Collegium.");
		else if (Player.rank[COLLEGE] == MAGE) {
		    if (Player.level <= Archmagelevel)
			mprint ("You are not experienced enough to advance.");
		    else
			mprint ("You must return with the heart of the Eater of Magic!");
		} else if (Player.rank[COLLEGE] == PRECEPTOR) {
		    if (Player.guildxp[COLLEGE] < 4000)
			mprint ("You are not experienced enough to advance.");
		    else {
			mprint ("You are now a Mage of the Collegium Magii!");
			mprint ("You may research 6 spells for postdoctoral research.");
			Spellsleft += 6;
			morewait();
			mprint ("To become Archmage, you must return with the");
			mprint ("heart of the Eater of Magic");
			morewait();
			clearmsg();
			mprint ("The Eater may be found on a desert isle somewhere.");
			Player.rank[COLLEGE] = MAGE;
			Player.maxiq += 2;
			Player.iq += 2;
			Player.maxpow += 2;
			Player.pow += 2;
		    }
		} else if (Player.rank[COLLEGE] == STUDENT) {
		    if (Player.guildxp[COLLEGE] < 1500)
			mprint ("You are not experienced enough to advance.");
		    else {
			mprint ("You are now a Preceptor of the Collegium Magii!");
			mprint ("You are taught the basics of ritual magic.");
			morewait();
			clearmsg();
			mprint ("Your position allows you to research 4 spells.");
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
			mprint ("You are not experienced enough to advance.");
		    else {
			mprint ("You are now a Student at the Collegium Magii!");
			mprint ("You are taught the spell of identification.");
			morewait();
			clearmsg();
			mprint ("Thesis research credit is 2 spells.");
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
		    mprintf ("Research permitted: %u Spells.", Spellsleft);
		    morewait();
		}
		if (Spellsleft < 1) {
		    mprint ("Extracurricular Lab fee: 2000 Au. Pay it? [yn] ");
		    if (ynq() == 'y') {
			if (Player.cash < 2000)
			    mprint ("Try again when you have the cash.");
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
    mprint ("The Circle of Sorcerors.");
    if (Player.rank[CIRCLE] == FORMER_SOURCEROR) {
	mprint ("Fool! Didn't we tell you to go away?");
	Player.mana = 0;
	dataprint();
    } else
	while (!done) {
	    if ((Player.rank[CIRCLE] == HIGHSORCEROR) && (Player.level > Primelevel) && find_and_remove_item (CORPSEID, LAWBRINGER)) {
		mprint ("You obtained the Crown of the Lawgiver!");
		morewait();
		mprint ("The Crown is ritually sacrificed to the Lords of Chaos.");
		mprint ("You are now the Prime Sorceror of the Inner Circle!");
		morewait();
		clearmsg();
		mprint ("You learn the Spell of Disintegration!");
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
		    mprint ("You are already an initiate!");
		else if (Player.alignment > 0)
		    mprint ("You may not join -- you reek of Law!");
		else if (Player.rank[COLLEGE] >= NOVICE)
		    mprint ("Foolish Mage!  You don't have the right attitude to Power!");
		else {
		    fee += Player.alignment * 100;
		    fee += fee * (12 - Player.pow) / 9;
		    fee = max (100U, fee);
		    clearmsg();
		    mprintf ("For you, there is an initiation fee of %u Au. Pay it? [yn] ", fee);
		    if (ynq() == 'y') {
			if (Player.cash < fee)
			    mprint ("Try again when you have the cash!");
			else {
			    mprintf ("Prime Sorceror %s conducts your initiation into the circle of novices.", Prime);
			    morewait();
			    clearmsg();
			    mprint ("You learn the Spell of Magic Missiles.");
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
		    mprint ("You have not even been initiated, yet!");
		else if (Player.alignment > -1) {
		    mprint ("Ahh! You have grown too lawful!!!");
		    mprint ("You are hereby blackballed from the Circle!");
		    Player.rank[CIRCLE] = FORMER_SOURCEROR;
		    morewait();
		    clearmsg();
		    mprint ("A pox upon thee!");
		    if (!Player.immunity[INFECTION])
			Player.status[DISEASED] += 100;
		    mprint ("And a curse on your possessions!");
		    morewait();
		    clearmsg();
		    acquire (-1);
		    clearmsg();
		    enchant (-1);
		    bless (-1);
		    mprint ("Die, false sorceror!");
		    p_damage (25, UNSTOPPABLE, "a sorceror's curse");
		    done = true;
		} else if (Player.rank[CIRCLE] == PRIME) {
		    mprint ("You are at the pinnacle of mastery in the Circle.");
		} else if (Player.rank[CIRCLE] == HIGHSORCEROR) {
		    if (Player.level <= Primelevel)
			mprint ("You are not experienced enough to advance.");
		    else
			mprint ("You must return with the Crown of the LawBringer!");
		} else if (Player.rank[CIRCLE] == SORCEROR) {
		    if (Player.guildxp[CIRCLE] < 4000)
			mprint ("You are not experienced enough to advance.");
		    else {
			mprint ("You are now a High Sorceror of the Inner Circle!");
			mprint ("You learn the Spell of Disruption!");
			morewait();
			clearmsg();
			mprint ("To advance you must return with the LawBringer's Crown!");
			mprint ("The LawBringer resides on Star Peak.");
			learn_spell (S_DISRUPT);
			Player.rank[CIRCLE] = HIGHSORCEROR;
			Player.maxpow += 5;
			Player.pow += 5;
		    }
		} else if (Player.rank[CIRCLE] == ENCHANTER) {
		    if (Player.guildxp[CIRCLE] < 1500)
			mprint ("You are not experienced enough to advance.");
		    else {
			mprint ("You are now a member of the Circle of Sorcerors!");
			mprint ("You learn the Spell of Ball Lightning!");
			learn_spell (S_LBALL);
			Player.rank[CIRCLE] = SORCEROR;
			Player.maxpow += 2;
			Player.pow += 2;
		    }
		} else if (Player.rank[CIRCLE] == INITIATE) {
		    if (Player.guildxp[CIRCLE] < 400)
			mprint ("You are not experienced enough to advance.");
		    else {
			mprint ("You are now a member of the Circle of Enchanters!");
			mprint ("You learn the Spell of Firebolts.");
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
		mprintf ("That will be: %u Au. Pay it? [yn] ", fee);
		if (ynq() == 'y') {
		    if (Player.cash < fee)
			mprint ("Begone, deadbeat, or face the wrath of the Circle!");
		    else {
			Player.cash -= fee;
			total = Player.calcmana();
			while (Player.mana < total) {
			    Player.mana++;
			    dataprint();
			}
			mprint ("Have a sorcerous day, now!");
		    }
		} else
		    mprint ("Be seeing you!");
	    }
	}
    xredraw();
}

void l_order (void)
{
    mprint ("The Headquarters of the Order of Paladins.");
    morewait();
    if (Player.rank[ORDER] == PALADIN && Player.level > Justiciarlevel && gamestatusp(GAVE_STARGEM) && Player.alignment > 300) {
	mprint ("You have succeeded in your quest!");
	morewait();
	foreach (m, Level->mlist)
	    if (m->id == HISCORE_NPC && m->aux2 == NPC_JUSTICIAR)
		--(m = Level->mlist.erase(m));
	mprint ("The previous Justiciar steps down in your favor.");
	mprint ("You are now the Justiciar of Rampart and the Order!");
	morewait();
	clearmsg();
	mprint ("You are awarded a blessed shield of deflection!");
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
	    mprint ("You have been tainted by chaos!");
	    mprint ("You are stripped of your rank in the Order!");
	    morewait();
	    Player.rank[ORDER] = FORMER_PALADIN;
	    send_to_jail();
	} else
	    mprint ("Get thee hence, minion of chaos!");
    } else if (Player.rank[ORDER] == FORMER_PALADIN)
	mprint ("Thee again?  Get thee hence, minion of chaos!");
    else if (Player.rank[ORDER] == NOT_IN_ORDER) {
	if (Player.rank[ARENA] >= TRAINEE)
	    mprint ("We do not accept bloodstained gladiators into our Order.");
	else if (Player.rank[LEGION] >= LEGIONAIRE)
	    mprint ("Go back to your barracks, mercenary!");
	else {
	    mprint ("Dost thou wish to join our Order? [yn] ");
	    if (ynq() == 'y') {
		mprintf ("Justiciar %s welcomes you to the Order.", Justiciar);
		mprint ("'Mayest thou always follow the sublime path of Law.'");
		morewait();
		mprint ("You are now a Gallant in the Order.");
		mprint ("You are given a horse and a blessed spear.");
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
	mprint ("'Welcome back, Paladin.'");
	if (!gamestatusp (MOUNTED)) {
	    mprint ("You are given a new steed.");
	    setgamestatus (MOUNTED);
	}
	morewait();
	clearmsg();
	if ((Player.hp < Player.maxhp) || (Player.status[DISEASED]) || (Player.status[POISONED]))
	    mprint ("Your wounds are treated by a medic.");
	cleanse (0);
	Player.hp = Player.maxhp;
	Player.food = 40;
	mprint ("You get a hot meal from the refectory.");
	morewait();
	clearmsg();
	if (Player.rank[ORDER] == PALADIN) {
	    if (Player.level <= Justiciarlevel)
		mprint ("You are not experienced enough to advance.");
	    else if (Player.alignment < 300)
		mprint ("You are not sufficiently Lawful as yet to advance.");
	    else
		mprint ("You must give the Star Gem to the LawBringer.");
	} else if (Player.rank[ORDER] == CHEVALIER) {
	    if (Player.guildxp[ORDER] < 4000)
		mprint ("You are not experienced enough to advance.");
	    else if (Player.alignment < 200)
		mprint ("You are not sufficiently Lawful as yet to advance.");
	    else {
		mprint ("You are made a Paladin of the Order!");
		mprint ("You learn the Spell of Heroism and get Mithril Plate!");
		morewait();
		object newitem = Objects[ARMOR_MITHRIL_PLATE];
		newitem.blessing = 9;
		learn_object (newitem);
		gain_item (newitem);
		morewait();
		clearmsg();
		mprint ("To advance you must rescue the Star Gem and return it");
		mprint ("to its owner, the LawBringer, who resides on Star Peak.");
		morewait();
		mprint ("The Star Gem was stolen by the cursed Prime Sorceror,");
		mprint ("whose headquarters may be found beyond the Astral Plane.");
		morewait();
		mprint ("The Oracle will send you to the Astral Plane if you");
		mprint ("prove yourself worthy to her.");
		morewait();
		learn_spell (S_HERO);
		Player.rank[ORDER] = PALADIN;
	    }
	} else if (Player.rank[ORDER] == GUARDIAN) {
	    if (Player.guildxp[ORDER] < 1500)
		mprint ("You are not experienced enough to advance.");
	    else if (Player.alignment < 125)
		mprint ("You are not yet sufficiently Lawful to advance.");
	    else {
		Player.rank[ORDER] = CHEVALIER;
		mprint ("You are made a Chevalier of the Order!");
		mprint ("You are given a Mace of Disruption!");
		morewait();
		clearmsg();
		learn_object (Objects[WEAPON_MACE_OF_DISRUPTION]);
		gain_item (Objects[WEAPON_MACE_OF_DISRUPTION]);
	    }
	} else if (Player.rank[ORDER] == GALLANT) {
	    if (Player.guildxp[ORDER] < 400)
		mprint ("You are not experienced enough to advance.");
	    else if (Player.alignment < 50)
		mprint ("You are not Lawful enough to advance.");
	    else {
		mprint ("You are made a Guardian of the Order of Paladins!");
		mprint ("You are given a Holy Hand Grenade (of Antioch).");
		morewait();
		mprint ("You hear a nasal monotone in the distance....");
		mprint ("'...and the number of thy counting shall be 3...'");
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
void load_house (EEnvironment kind)
{
    initrand (Level->environment, Player.x + Player.y + hour() * 10);
    const char* mapdata = Level_Hovel;
    uint8_t wallstrength = 10, stops = 0;
    if (kind == E_HOUSE) {
	mapdata = Level_House;
	wallstrength = 50;
    } else if (kind == E_MANSION) {
	mapdata = Level_Mansion;
	wallstrength = 150;
    }
    level::load_map (kind, mapdata, [&](char sc, location& s, unsigned i, unsigned j) {
	s.locchar = FLOOR;
	s.lstatus = (kind == E_HOVEL ? SEEN : 0);
	s.roomnumber = RS_CORRIDOR;
	s.p_locf = L_NO_OP;
	switch (sc) {
	    case 'N':
		s.roomnumber = RS_BEDROOM;
		if (random_range(2))
		    make_house_npc (i, j);
		break;
	    case 'H':
		s.roomnumber = RS_BEDROOM;
		if (random_range(2))
		    make_mansion_npc (i, j);
		break;
	    case 'D': s.roomnumber = RS_DININGROOM; break;
	    case '.':
		if (stops) {
		    lset (i, j, STOPS);
		    stops = 0;
		}
		break;
	    case 'c': s.roomnumber = RS_CLOSET; break;
	    case 'G': s.roomnumber = RS_BATHROOM; break;
	    case 'B': s.roomnumber = RS_BEDROOM; break;
	    case 'K': s.roomnumber = RS_KITCHEN; break;
	    case 'S':
		lset (i, j, SECRET);
		s.roomnumber = RS_SECRETPASSAGE;
		break;
	    case '3':
		s.locchar = SAFE;
		lset (i, j, SECRET);
		s.p_locf = L_SAFE;
		break;
	    case '^': s.p_locf = TRAP_BASE + random_range (NUMTRAPS); break;
	    case 'P': s.locchar = PORTCULLIS;
	    case 'p': s.p_locf = L_PORTCULLIS; break;
	    case 'R': s.p_locf = L_RAISE_PORTCULLIS; break;
	    case 'T': s.p_locf = L_PORTCULLIS_TRAP; break;
	    case 'X': s.p_locf = L_HOUSE_EXIT; stops = 1; break;
	    case '#':
		s.locchar = WALL;
		s.aux = wallstrength;
		break;
	    case '|':
		s.locchar = OPEN_DOOR;
		s.roomnumber = RS_CORRIDOR;
		lset (i, j, STOPS);
		break;
	    case '+':
		s.locchar = CLOSED_DOOR;
		s.roomnumber = RS_CORRIDOR;
		s.aux = LOCKED;
		lset (i, j, STOPS);
		break;
	    case 'd':
		s.locchar = FLOOR;
		s.roomnumber = RS_CORRIDOR;
		make_site_monster (i, j, DOBERMAN);
		break;
	    case 'a':
		s.locchar = FLOOR;
		s.roomnumber = RS_CORRIDOR;
		s.p_locf = L_TRAP_SIREN;
		break;
	    case 'A':
		s.locchar = FLOOR;
		s.roomnumber = RS_CORRIDOR;
		make_site_monster (i, j, AUTO_MINOR);
		break;
	}
    });
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
	m_status_reset (m, ASLEEP);
    else
	m_status_set (m, ASLEEP);
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
	m_status_set (m, ASLEEP);
    else
	m_status_reset (m, ASLEEP);
}

// loads the village level into Level
void load_village (uint8_t villagenum)
{
    initrand (Level->environment, villagenum);
    location d;
    assign_village_function (d, 0, 0, true);
    static const char* _villages[] = {
	Level_Village1, Level_Village2, Level_Village3,
	Level_Village4, Level_Village5, Level_Village6
    };
    static const uint8_t _vstone[] = {
	L_LAWSTONE, L_BALANCESTONE, L_CHAOSTONE,
	L_MINDSTONE, L_SACRIFICESTONE, L_VOIDSTONE
    };
    if (villagenum >= ArraySize(_villages))
	throw runtime_error ("village not found");
    Level->SetVillageId (villagenum);
    level::load_map (E_VILLAGE, _villages[villagenum], [&](char sc, location& s, unsigned i, unsigned j) {
	s.locchar = FLOOR;
	s.lstatus |= SEEN;
	s.p_locf = L_NO_OP;
	switch (sc) {
	    case 'f': make_food_bin (i, j); break;
	    case 'h': make_site_monster (i, j, HORSE); break;
	    case 'H': make_site_monster (i, j, MERCHANT); break;
	    case 's': make_site_monster (i, j, SHEEP); break;
	    case 'x': assign_village_function (s, i, j, false); break;
	    case 'G': {
		monster& m = make_site_monster (i, j, GUARD);
		m.aux1 = i;
		m.aux2 = j;
		break; }
	    case 'g': s.p_locf = L_GRANARY; break;
	    case 'S': s.p_locf = L_STABLES; break;
	    case 'C': s.p_locf = L_COMMONS; break;
	    case 'X': s.p_locf = L_COUNTRYSIDE; break;
	    case '^': s.p_locf = L_TRAP_SIREN; break;
	    case '(': s.locchar = HEDGE; s.p_locf = L_HEDGE; break;
	    case '~': s.locchar = WATER; s.p_locf = L_WATER; break;
	    case '+': s.locchar = WATER; s.p_locf = L_CHAOS; break;
	    case '\'': s.locchar = HEDGE; s.p_locf = L_TRIFID; break;
	    case '!': s.locchar = ALTAR; s.p_locf = _vstone[villagenum]; break;
	    case '#': s.locchar = WALL; s.aux = 100; break;
	    case '-': s.locchar = CLOSED_DOOR; break;
	    case '1': s.locchar = STATUE; break;
	}
    });
    initrand (E_RESTORE, 0);
}

static void make_food_bin (int i, int j)
{
    for (unsigned k = 0; k < 10; k++)
	Level->make_thing (i, j, FOOD_GRAIN);
}

static void assign_village_function (location& s, int x, int y, bool setup)
{
    static unsigned next = 0;
    static uint8_t permutation[24];	// number of x's in village map

    if (setup) {
	next = 0;
	iota (ArrayRange(permutation), 0);
	random_shuffle (ArrayRange(permutation));
	return;
    }

    lset (x, y + 1, STOPS);
    lset (x + 1, y, STOPS);
    lset (x - 1, y, STOPS);
    lset (x, y - 1, STOPS);
    lset (x, y, STOPS);

    static const uint8_t _villageloc[] = {
	L_ARMORER, L_HEALER, L_TAVERN, L_COMMANDANT, L_CARTOGRAPHER
    };
    unsigned loc;
    if (next >= ArraySize(permutation) || ArraySize(_villageloc) <= (loc = permutation[next++])) {
	s.locchar = CLOSED_DOOR;
	s.aux = xrand()%1 ? LOCKED : 0;
	s.p_locf = xrand()%1 ? L_HOVEL : L_HOUSE;
    } else {
	s.locchar = OPEN_DOOR;
	s.p_locf = _villageloc[loc];
    }
}

// Functions dealing with dungeon and country levels aside from actual
// level structure generation

// The caves and sewers get harder as you penetrate them; the castle
// is completely random, but also gets harder as it is explored;
// the astral and the volcano just stay hard...
void populate_level (void)
{
    EEnvironment monstertype = Level->environment;
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
	    default:
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
		    monsterid = _sewerMonsters[min<unsigned>(ArraySize(_sewerMonsters),random_range(Level->depth+3))];
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
		    monsterid = _volcanoMonsters[random_range(min<unsigned>(Level->depth/2+2,ArraySize(_volcanoMonsters)))];
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
    if (random_range (Level->MaxDepth()) < difficulty()) {
	int x, y;
	findspace (&x, &y);
	make_site_monster (x, y, RANDOM);
    }
}

// call make_creature and place created monster on Level->mlist and Level
monster& make_site_monster (int i, int j, int mid, int wandering, int dlevel)
{
    #if USE_UCC
	monster& m = Level->mlist.push_back();
    #else
	monster& m = *Level->mlist.insert (Level->mlist.end(), monster());
    #endif
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
    unsigned monster_range = _ranges[min<unsigned>(level,ArraySize(_ranges)-1)];
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
    if (mid == ANGEL || mid == HIGH_ANGEL || mid == ARCHANGEL) {
	// aux1 field of an angel is its deity
	if (Level->environment == E_TEMPLE)
	    m.aux1 = Level->TempleDeity();
	else
	    m.aux1 = ODIN+random_range(NUMRELIGIONS-ODIN);
	static const char _religion[NUMRELIGIONS+1][8] = { "Atheism", "Nature", "Odin", "Set", "Hecate", "Athena", "Destiny", "Balance" };
	snprintf (ArrayBlock(Str3), "%s of %s", Monsters[mid].monstring, _religion[m.aux1+1]);
	m.monstring = strdup (Str3);
    } else if (mid == ZERO_NPC || mid == WEREHUMAN) {
	// generic 0th level human, or a were-human
	m.monstring = mantype();
	snprintf (ArrayBlock(Str1), "dead %s", m.monstring);
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
	if (m.sleep > random_range (100))
	    m_status_set (m, ASLEEP);
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
    snprintf (ArrayBlock(buf), "The body of %s", npc.monstring);
    npc.corpsestr = strdup (buf);
}

// sets npc behavior given level and behavior code
static void determine_npc_behavior (monster& npc, int level, int behavior)
{
    npc.hp = (level + 1) * 20;
    npc.status = MOBILE + WANDERING;
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
	const unsigned cashFactor = (Level->environment == E_CAVES ? 3 : 1);
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
    switch (Level->environment) {
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
    unsigned amount;
    char response;
    mprint ("First Bank of Omega: Autoteller Carrel.");

    if (gamestatusp (BANK_BROKEN))
	mprint ("You see a damaged autoteller.");
    else {
	mprint ("The proximity sensor activates the autoteller as you approach.");
	morewait();
	clearmsg();
	while (!done) {
	    mprintf ("Current Balance: %u Au. Enter command (? for help) > ", Balance);
	    response = mgetc();
	    if (response == '?') {
		menuclear();
		menuprint ("?: This List.\n");
		if (!Password[0])
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
	    } else if (response == 'P' && Password[0]) {
		clearmsg();
		mprint ("Password: ");
		char passwd [ArraySize(Password)];
		strncpy (passwd, msgscanstring(), sizeof(passwd));
		ArrayEnd(passwd)[-1] = 0;
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
			mprint ("Ah ha! Trying to rob the bank, eh?");
			mprint ("Take him away, boys!");
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
			mprint ("The cash machine begins to spew gold pieces!");
			mprint ("You pick up your entire balance and then some!");
			Player.cash += Balance + 1000 + random_range (3000);
			Balance = 0;
			setgamestatus (BANK_BROKEN);
		    }
		} else
		    mprint ("Password accepted. Working.");
	    } else if (response == 'D' && valid) {
		clearmsg();
		mprint ("Amount: ");
		amount = get_money (Player.cash);
		if (amount < 1)
		    mprint ("Transaction aborted.");
		else if (amount > Player.cash)
		    mprint ("Deposit too large -- transaction aborted.");
		else {
		    mprint ("Transaction accomplished.");
		    Balance += amount;
		    Player.cash -= amount;
		}
	    } else if (response == 'W' && valid) {
		clearmsg();
		mprint ("Amount: ");
		amount = get_money (Balance);
		if (amount < 1)
		    mprint ("Transaction aborted.");
		else if (amount > Balance)
		    mprint ("Withdrawal too large -- transaction aborted.");
		else {
		    mprint ("Transaction accomplished.");
		    Balance -= amount;
		    Player.cash += amount;
		}
	    } else if (response == 'X') {
		clearmsg();
		mprint ("Bye!");
		done = true;
	    } else if (response == 'O' && !Password[0]) {
		clearmsg();
		mprint ("Opening new account. Please enter new password: ");
		strncpy (Password, msgscanstring(), sizeof(Password));
		ArrayEnd(Password)[-1] = 0;
		if (!Password[0]) {
		    mprint ("Illegal to use null password -- aborted.");
		    done = true;
		} else {
		    mprint ("Password validated; account saved.");
		    valid = true;
		}
	    } else
		mprint (" Illegal command.");
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
	mprint ("Unfortunately, this is Julie's lunch hour -- try again later.");
    else if (nighttime())
	mprint ("It seems that Julie keeps regular business hours.");
    else {
	while (!done) {
	    clearmsg();
	    mprint ("Julie's: Buy Armor, Weapons, or Leave [a,w,ESCAPE] ");
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
    mprint ("Purchase which item? [ESCAPE to quit] ");
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
	unsigned cost = 2 * true_item_value (newitem);
	mprintf ("I can let you have it for %u Au. Buy it? [yn] ", cost);
	if (ynq() == 'y') {
	    if (Player.cash < cost)
		mprint ("Why not try again some time you have the cash?");
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

    mprint ("Rampart Explorers' Club.");
    if (!gamestatusp (CLUB_MEMBER)) {
	if (Player.level < 2)
	    mprint ("Only reknowned adventurers need apply.");
	else {
	    mprint ("Dues are 100Au. Pay it? [yn] ");
	    if (ynq() == 'y') {
		if (Player.cash < 100)
		    mprint ("Beat it, or we'll blackball you!");
		else {
		    mprint ("Welcome to the club! You are taught the spell of Return.");
		    mprint ("When cast on the first level of a dungeon it");
		    morewait();
		    clearmsg();
		    mprint ("will transport you down to the lowest level");
		    mprint ("you have explored, and vice versa.");
		    learn_spell (S_RETURN);
		    Player.cash -= 100;
		    setgamestatus (CLUB_MEMBER);
		}
	    } else
		mprint ("OK, but you're missing out on our benefits....");
	}
    } else {
	mprint ("Shop at the club store or listen for rumors [sl] ");
	do
	    response = (char) mcigetc();
	while (response != 's' && response != 'l' && response != KEY_ESCAPE);
	if (response == 'l') {
	    if (club_hinthour == hour())
		mprint ("You don't hear anything useful.");
	    else {
		mprint ("You overhear a conversation....");
		hint();
		club_hinthour = hour();
	    }
	} else if (response == 's') {
	    buyfromstock (THING_KEY, 2);
	    xredraw();
	} else if (response == KEY_ESCAPE)
	    mprint ("Be seeing you, old chap!");
    }
}

void l_gym (void)
{
    int done = true;
    int trained = 0;
    clearmsg();
    do {
	if (Gymcredit > 0 || Player.rank[ARENA])
	    mprintf ("The Rampart Gymnasium -- Credit: %u Au", Gymcredit);
	else
	    mprint ("The Rampart Gymnasium");
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
		    mprint ("Well, it's your body you're depriving!");
		else if (trained < 3)
		    mprint ("You towel yourself off, and find the exit.");
		else
		    mprint ("A refreshing bath, and you're on your way.");
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
    mprint ("Rampart Healers. Member RMA.");
    morewait();
    clearmsg();
    mprint ("a: Heal injuries (50 crowns)");
    mprint ("b: Cure disease (250 crowns)");
    mprint ("ESCAPE: Leave these antiseptic alcoves.");
    switch ((char) mcigetc()) {
	case 'a':
	    healforpay();
	    break;
	case 'b':
	    cureforpay();
	    break;
	default:
	    mprint ("OK, but suppose you have Acute Satyriasis?");
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
	    mprint ("The statue crumbles with a clatter of gravel.");
	    Level->site(x,y).locchar = RUBBLE;
	    Level->site(x,y).p_locf = L_RUBBLE;
	    plotspot (x, y, true);
	    lset (x, y, CHANGED);
	    break;
	case 1:
	    mprint ("The statue stoutly resists your attack.");
	    break;
	case 2:
	    mprint ("The statue crumbles with a clatter of gravel.");
	    Level->site(x,y).locchar = RUBBLE;
	    Level->site(x,y).p_locf = L_RUBBLE;
	    plotspot (x, y, true);
	    lset (x, y, CHANGED);
	    make_site_treasure (x, y, difficulty());
	    break;
	case 3:
	    mprint ("The statue hits you back!");
	    p_damage (random_range (difficulty() * 5), UNSTOPPABLE, "a statue");
	    break;
	case 4:
	    if (Level->IsDungeon() && Level->depth < Level->MaxDepth()) {
		mprint ("You hear the whirr of some mechanism.");
		mprint ("The statue glides smoothly into the floor!");
		Level->site(x,y).locchar = STAIRS_DOWN;
		Level->site(x,y).p_locf = L_NO_OP;
		lset (x, y, CHANGED | STOPS);
		break;
	    }
	case 5:
	    mprint ("The statue looks slightly pained. It speaks:");
	    morewait();
	    clearmsg();
	    hint();
	    break;
	case 6:
	    mprint ("A strange radiation emanates from the statue!");
	    dispel (-1);
	    break;
	case 7:
	    mprint ("The statue was covered with contact cement!");
	    mprint ("You can't move....");
	    Player.status[IMMOBILE] += random_range (6) + 2;
	    break;
	case 8:		// I think this is particularly evil. Heh heh.
	    if (Player.has_possession(O_WEAPON_HAND)) {
		mprint ("Your weapon sinks deeply into the statue and is sucked away!");
		Player.possessions[O_WEAPON_HAND].blessing = -1 - absv (Player.possessions[O_WEAPON_HAND].blessing);
		drop_at (x, y, Player.possessions[O_WEAPON_HAND]);
		Player.remove_possession(O_WEAPON_HAND);
	    }
	    break;
	case 9:
	    mprint ("The statue extends an arm. Beams of light illuminate the level!");
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
    mprint ("Rampart Mithril Nugget Casino.");
    if (random_range (10) == 1)
	mprint ("Casino closed due to Grand Jury investigation.");
    else {
	while (!done) {
	    morewait();
	    clearmsg();
	    mprint ("a: Drop 100Au in the slots.");
	    mprint ("b: Risk 1000Au  at roulette.");
	    mprint ("ESCAPE: Leave this green baize hall.");
	    response = (char) mcigetc();
	    if (response == 'a') {
		if (Player.cash < 100)
		    mprint ("No credit, jerk.");
		else {
		    Player.cash -= 100;
		    dataprint();
		    for (i = 0; i < 20; i++) {
			napms (i == 19 ? 1000 : 250);
			a = random_range (10);
			b = random_range (10);
			c = random_range (10);
			clearmsg();
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
			mprint ("Jackpot Winner!");
			winnings += (a + 2) * (b + 2) * (c + 2) * 5;
			Player.cash += (a + 2) * (b + 2) * (c + 2) * 5;
			dataprint();
		    } else if (a == b) {
			mprint ("Winner!");
			Player.cash += (a + 2) * (b + 2) * 5;
			dataprint();
			winnings += (a + 2) * (b + 2) * 5;
		    } else if (a == c) {
			mprint ("Winner!");
			Player.cash += (a + 2) * (c + 2) * 5;
			dataprint();
			winnings += (a + 2) * (c + 2) * 5;
		    } else if (c == b) {
			mprint ("Winner!");
			Player.cash += (c + 2) * (b + 2) * 5;
			dataprint();
			winnings += (c + 2) * (b + 2) * 5;
		    } else {
			mprint ("Loser!");
			winnings -= 100;
		    }
		}
	    } else if (response == 'b') {
		if (Player.cash < 1000)
		    mprint ("No credit, jerk.");
		else {
		    Player.cash -= 1000;
		    dataprint();
		    mprint ("Red or Black? [rb]");
		    do
			response = (char) mcigetc();
		    while ((response != 'r') && (response != 'b'));
		    match = (response == 'r' ? 0 : 1);
		    for (i = 0; i < 20; i++) {
			napms (i == 19 ? 1000 : 250);
			a = random_range (37);
			b = a % 2;
			if (a == 0)
			    mprint (" 0 ");
			else if (a == 1)
			    mprint (" 0 - 0 ");
			else
			    mprintf ("%s %u", !b ? "Red" : "Black", a-1);
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
			mprint (" 0 ");
		    else if (a == 1)
			mprint (" 0 - 0 ");
		    else
			mprintf ("%s %u", !b ? "Red" : "Black", a-1);
		    if ((a > 1) && (b == match)) {
			mprint (" Winner!");
			winnings += 1000;
			Player.cash += 2000;
			dataprint();
		    } else {
			mprint (" Loser!");
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
    mprint ("Commandant Sonder's Rampart-fried Lyzzard partes. Open 24 hrs.");
    mprint ("Buy a bucket! Only 5 Au. Make a purchase? [yn] ");
    if (ynq() == 'y') {
	clearmsg();
	mprint ("How many? ");
	int num = (int) parsenum();
	if (num < 1)
	    mprint ("Cute. Real cute.");
	else if (num * 5U > Player.cash)
	    mprint ("No handouts here, mac!");
	else {
	    Player.cash -= num * 5;
	    object food = Objects[FOOD_RATION];
	    food.number = num;
	    if (num == 1)
		mprint ("There you go, mac! One Lyzzard Bucket, coming up.");
	    else
		mprint ("A passel of Lyzzard Buckets, for your pleasure.");
	    gain_item (food);
	    morewait();
	}
    } else
	mprint ("Don't blame the Commandant if you starve!");
}

void l_diner (void)
{
    mprint ("The Rampart Diner. All you can eat, 25Au.");
    mprint ("Place an order? [yn] ");
    if (ynq() == 'y') {
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
    mprint ("Les Crapeuleaux. (****) ");
    if ((hour() < 17) || (hour() > 23))
	mprint ("So sorry, we are closed 'til the morrow...");
    else {
	mprint ("May I take your order? [yn] ");
	if (ynq() == 'y') {
	    if (Player.cash < 1000)
		mprint ("So sorry, you have not the funds for dinner.");
	    else {
		mprint ("Hope you enjoyed your tres expensive meal, m'sieur...");
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
    mprint ("The Centaur and Nymph -- J. Riley, prop.");
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
		    mprint ("Aw hell, have one on me.");
		else {
		    Player.cash -= 1;
		    dataprint();
		    if (tavern_hinthour != hour()) {
			if (random_range (3)) {
			    mprint ("You overhear a rumor...");
			    hint();
			} else
			    mprint ("You don't hear much of interest.");
			tavern_hinthour = hour();
		    } else
			mprint ("You just hear the same conversations again.");
		}
		break;
	    case 'b':
		if (Player.cash < 10)
		    mprint ("I don't serve the Dew on no tab, buddy!");
		else {
		    Player.cash -= 10;
		    mprint ("Ahhhhh....");
		    if (Player.status[POISONED] || Player.status[DISEASED])
			mprint ("Phew! That's, er, smooth stuff!");
		    Player.status[POISONED] = 0;
		    Player.status[DISEASED] = 0;
		    showflags();
		}
		break;
	    case 'c':
		if (Player.cash < 100) {
		    mprint ("Whatta feeb!");
		    mprint ("Outta my establishment.... Now!");
		    p_damage (random_range (20), UNSTOPPABLE, "Riley's right cross");
		    morewait();
		} else {
		    Player.cash -= 100;
		    dataprint();
		    mprint ("'What a guy!'");
		    morewait();
		    mprint ("'Hey, thanks, fella.'");
		    morewait();
		    mprint ("'Make mine a double...'");
		    morewait();
		    clearmsg();
		    switch (random_range (4)) {
			case 0:
			    mprint ("'You're a real pal. Say, have you heard.... ");
			    hint();
			    break;
			case 1:
			    mprint ("A wandering priest of Dionysus blesses you...");
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
			    mprint ("A thirsty bard promises to put your name in a song!");
			    gain_experience (20);
			    break;
			case 3:
			    mprint ("Riley draws you a shot of his 'special reserve'");
			    mprint ("Drink it [yn]?");
			    if (ynq() == 'y') {
				if (Player.con < random_range (20)) {
				    mprint ("<cough> Quite a kick!");
				    mprint ("You feel a fiery warmth in your tummy....");
				    Player.con++;
				    Player.maxcon++;
				} else
				    mprint ("You toss it back nonchalantly.");
			    }
		    }
		}
		break;
	    case 'd':
		if (Player.cash < 25)
		    mprint ("Pay in advance, mac!");
		else {
		    Player.cash -= 25;
		    mprint ("How about a shot o' the dew for a nightcap?");
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
		    mprint ("The next day.....");
		    if (hour() > 10)
			mprint ("Oh my! You overslept!");
		}
		break;
	    default:
		mprint ("So? Just looking? Go on!");
		break;
	}
    } else
	mprint ("The pub don't open til dark, fella.");
    xredraw();
}

void l_alchemist (void)
{
    mprint ("Ambrosias' Potions et cie.");
    if (nighttime()) {
	mprint ("Ambrosias doesn't seem to be in right now.");
	return;
    }
    while (true) {
	morewait();
	clearmsg();
	mprint ("a: Sell monster components.");
	mprint ("b: Pay for transformation.");
	mprint ("ESCAPE: Leave this place.");
	char response = mcigetc();
	if (response == 'a') {
	    clearmsg();
	    int i = getitem (CORPSE);
	    if (i == ABORT || !Player.has_possession(i))
		mprint ("So nu?");
	    else {
		object& obj = Player.possessions[i];
		if (Monsters[obj.charge].transformid == NO_THING) {
		    mprint ("I don't want such a thing.");
		    if (obj.basevalue > 0)
			mprint ("You might be able to sell it to someone else, though.");
		} else {
		    clearmsg();
		    unsigned cost = (obj.number * obj.basevalue) / 3;
		    mprintf ("I'll give you %u Au for it. Take it? [yn] ", cost);
		    if (ynq() != 'y')
			mprint ("Well, keep the smelly old thing, then!");
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
		mprint ("So nu?");
	    else {
		object& obj = Player.possessions[i];
		if (Monsters[obj.charge].transformid == NO_THING)
		    mprint ("Oy vey! You want me to transform such a thing?");
		else {
		    int mlevel = Monsters[obj.charge].level;
		    unsigned cost = max (10, obj.basevalue * 2);
		    mprintf ("It'll cost you %u Au for the transformation. Pay it? [yn] ", cost);
		    if (ynq() != 'y')
			mprint ("I don't need your business, anyhow.");
		    else {
			if (Player.cash < cost)
			    mprint ("You can't afford it!");
			else {
			    mprint ("Voila! A tap of the Philosopher's Stone...");
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
    mprint ("Rampart Department of Public Works.");
    if (Date - LastDay < 7)
	mprint ("G'wan! Get a job!");
    else if (Player.cash < 100) {
	mprint ("Do you want to go on the dole? [yn] ");
	if (ynq() == 'y') {
	    mprint ("Well, ok, but spend it wisely.");
	    morewait();
	    mprint ("Please enter your name for our records:");
	    strcpy (Str1, msgscanstring());
	    if (Str1[0] >= 'a' && Str1[0] <= 'z')
		Str1[0] += 'A' - 'a';
	    if (Str1[0] == '\0')
		mprint ("Maybe you should come back when you've learned to write.");
	    else if (Player.name != (const char*) Str1) {
		mprint ("Aha! Welfare Fraud! It's off to gaol for you, lout!");
		morewait();
		send_to_jail();
	    } else {
		mprint ("Here's your handout, layabout!");
		LastDay = Date;
		Player.cash = 99;
		dataprint();
	    }
	}
    } else
	mprint ("You're too well off for us to help you!");
}

void l_library (void)
{
    char response;
    bool studied = false;
    bool done = false;
    unsigned fee = 1000;
    mprint ("Rampart Public Library.");
    if (nighttime())
	mprint ("CLOSED");
    else {
	morewait();
	mprint ("Library Research Fee: 1000Au.");
	if (Player.maxiq < 18) {
	    mprint ("The Rampart student aid system has arranged a grant!");
	    mprintf ("Your revised fee is: %u Au.", fee = max (50, 1000 - (18 - Player.maxiq) * 125));
	}
	morewait();
	while (!done) {
	    mprint ("Pay the fee? [yn] ");
	    if (ynq() == 'y') {
		if (Player.cash < fee) {
		    mprint ("No payee, No studee.");
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
			    mprint ("You unfurl an ancient, yellowing scroll...");
			    morewait();
			    displayfile (Data_ScrollReligion);
			    xredraw();
			} else if (response == 'b') {
			    mprint ("You unroll a slick four-color document...");
			    morewait();
			    displayfile (Data_ScrollRampartGuide);
			    xredraw();
			} else if (response == 'c') {
			    mprint ("This scroll is written in a strange magical script...");
			    morewait();
			    displayfile (Data_ScrollHiMagick);
			    xredraw();
			} else if (response == 'd') {
			    mprint ("You find a strange document, obviously misfiled");
			    mprint ("under the heading 'acrylic fungus painting technique'");
			    morewait();
			    displayfile (Data_ScrollAdept);
			    xredraw();
			} else if (response == 'e') {
			    if (random_range (30) > Player.iq) {
				mprint ("You feel more knowledgeable!");
				Player.iq++;
				Player.maxiq++;
				dataprint();
				if (Player.maxiq < 19 && fee != max (50U, 1000U - (18U - Player.maxiq) * 125U)) {
				    morewait();
				    clearmsg();
				    mprintf ("Your revised fee is: %u Au.", fee = max (50U, 1000U - (18U - Player.maxiq) * 125U));
				    morewait();
				}
			    } else {
				clearmsg();
				mprint ("You find advice in an ancient tome: ");
				morewait();
				hint();
				morewait();
			    }
			} else if (response == KEY_ESCAPE) {
			    done = true;
			    mprint ("That was an expensive browse...");
			} else
			    studied = false;
		    } while (!studied);
		}
		xredraw();
	    } else {
		done = true;
		if (studied)
		    mprint ("Come back anytime we're open, 7am to 8pm.");
		else
		    mprint ("You philistine!");
	    }
	}
    }
}

void l_pawn_shop (void)
{
    if (nighttime()) {
	mprint ("Shop Closed: Have a Nice (K)Night");
	return;
    }

    unsigned limit = min<unsigned> (Pawnitems.size()/4, Date - Pawndate);
    Pawndate = Date;
    for (unsigned i = 0; i < limit; ++i)
	if (object_uniqueness(Pawnitems[0]) > UNIQUE_UNMADE)
	    set_object_uniqueness (Pawnitems[0], UNIQUE_UNMADE);
    Pawnitems.erase (Pawnitems.begin(), Pawnitems.begin()+limit);
    while (Pawnitems.size() < PAWNITEMS) {
	object o = create_object (5);
	if (o.objchar != CASH && o.objchar != ARTIFACT && true_item_value(o) > 0)
	    Pawnitems.push_back (o);
    }

    while (true) {
	mprint ("Knight's Pawn Shop:");
	mprint ("Buy item, Sell item, sell Pack contents, Leave [b,s,p,ESCAPE] ");
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
	    mprint ("Purchase which item? [ESCAPE to quit] ");
	    unsigned item = ' ';
	    while (item != KEY_ESCAPE && (item < 'a' || item >= 'a' + Pawnitems.size()))
		item = mcigetc();
	    if (item != KEY_ESCAPE) {
		unsigned i = item - 'a';
		if (true_item_value (Pawnitems[i]) <= 0) {
		    mprint ("Hmm, how did that junk get on my shelves?");
		    mprint ("I'll just remove it.");
		    Pawnitems.erase (Pawnitems.iat(i));
		} else {
		    clearmsg();
		    unsigned cost = Pawnitems[i].number * true_item_value (Pawnitems[i]);
		    mprintf ("The low, low, cost is: %u Au. Buy it? [ynq] ", cost);
		    if (ynq() == 'y') {
			if (Player.cash < cost) {
			    mprint ("No credit! Gwan, Beat it!");
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
	    mprint ("Sell which item: ");
	    int i = getitem (NULL_ITEM);
	    if (i != ABORT && Player.has_possession(i)) {
		if (cursed (Player.possessions[i])) {
		    mprint ("No loans on cursed items! I been burned before....");
		    morewait();
		} else if (true_item_value (Player.possessions[i]) <= 0) {
		    mprint ("That looks like a worthless piece of junk to me.");
		    morewait();
		} else {
		    clearmsg();
		    unsigned cost = item_value (Player.possessions[i]) / 2;
		    mprintf ("You can get %u Au each. Sell [yn]? ", cost);
		    if (ynq() == 'y') {
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
		if (i->blessing <= 0 || true_item_value(*i) <= 0)
		    continue;
		clearmsg();
		unsigned cost = item_value (*i) / 2;
		mprintf ("Sell %s for %u Au each? [yn] ", itemid(*i), cost);
		if (ynq() == 'y') {
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
	mprint ("Rampart Arms. Weekly Rentals and Purchases");
	mprint ("Which are you interested in [r,p, or ESCAPE] ");
	response = mgetc();
	if (response == 'p') {
	    mprint ("Only 50,000Au. Buy it? [yn] ");
	    if (ynq() == 'y') {
		if (Player.cash < 50000)
		    mprint ("No mortgages, buddy.");
		else {
		    setgamestatus (SOLD_CONDO);
		    Player.cash -= 50000;
		    dataprint();
		    mprint ("You are the proud owner of a luxurious condo penthouse.");
		    Condoitems.clear();
		}
	    }
	} else if (response == 'r') {
	    mprint ("Weekly Rental, 1000Au. Pay for it? [yn] ");
	    if (ynq() == 'y') {
		if (Player.cash < 1000)
		    mprint ("Hey, pay the rent or out you go....");
		else {
		    weeksleep = true;
		    Player.cash -= 1000;
		    dataprint();
		}
	    }
	} else
	    mprint ("Please keep us in mind for your housing needs.");
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
			mprint ("The item just doesn't want to be stored away...");
		    else {
			Condoitems.push_back (Player.possessions[i]);
			Player.remove_possession (i);
		    }
		}
	    } else if (response == 'b') {
		foreach (i, Condoitems) {
		    mprintf ("Retrieve %s [ynq] ", itemid(*i));
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
		mprint ("You take a week off to rest...");
		morewait();
	    } else if (response == 'd') {
		clearmsg();
		mprint ("You sure you want to retire, now? [yn] ");
		if (ynq() == 'y') {
		    p_win();
		}
	    } else if (response == KEY_ESCAPE)
		done = true;
	}
	xredraw();
    }
    if (weeksleep) {
	clearmsg();
	mprint ("Taking a week off to rest...");
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
	mprint ("You're once again fit and ready to continue your adventure.");
	Time += 60 * 24 * 7;
	Date += 7;
	moon_check();
	timeprint();
    }
}

static void gymtrain (uint8_t *maxstat, uint8_t *stat)
{
    if (Gymcredit + Player.cash < 2000)
	mprint ("You can't afford our training!");
    else {
	if (Gymcredit > 2000)
	    Gymcredit -= 2000;
	else {
	    Player.cash -= (2000 - Gymcredit);
	    Gymcredit = 0;
	}
	if ((*maxstat < 30) && ((*maxstat < random_range (30)) || (random_range (3) == 1))) {
	    mprint ("Sweat. Sweat. The training pays off!");
	    (*maxstat)++;
	    (*stat)++;
	} else {
	    mprint ("Sweat. Sweat. You feel the healthy glow of a good workout.");
	    if (*stat < *maxstat) {
		(*stat)++;
		mprint ("A feeling of rehabilitation washes through you.");
	    }
	}
    }
    dataprint();
}

static void healforpay (void)
{
    if (Player.cash < 50)
	mprint ("You can't afford to be healed!");
    else {
	Player.cash -= 50;
	Player.hp += 20 + random_range (20);
	if (Player.hp > Player.maxhp)
	    Player.hp = Player.maxhp;
	mprint ("Another medical marvel....");
    }
    calc_melee();
}

static void cureforpay (void)
{
    if (Player.cash < 250)
	mprint ("You can't afford to be cured!");
    else {
	Player.cash -= 250;
	Player.status[DISEASED] = 0;
	mprint ("Quarantine lifted....");
	showflags();
    }
}

void pacify_guards (void)
{
    foreach (m, Level->mlist) {
	if (m->id == GUARD || (m->id == HISCORE_NPC && m->aux2 == NPC_JUSTICIAR)) {
	    m_status_reset (*m, HOSTILE);
	    m->specialf = M_NO_OP;
	    if (m->id == GUARD && m->hp > 0 && m->aux1 > 0) {
		m->x = m->aux1;
		m->y = m->aux2;
	    } else if (m->id == HISCORE_NPC && m->hp > 0 && Level->environment == E_CITY) {
		m->x = 40;
		m->y = 62;
	    }
	}
    }
    if (Level->environment == E_CITY)
	Level->site(40,60).p_locf = L_ORDER;	// undoes action in alert_guards
}

void send_to_jail (void)
{
    if (gamestatusp (DESTROYED_ORDER))
	mprint ("The destruction of the Order of Paladins has negated the law!");
    else if (Level->environment != E_CITY && World.LastEnvironment() != E_CITY)
	mprint ("Fortunately, there is no jail around here, so you are freed!");
    else if (Player.rank[ORDER] >= GALLANT) {
	mprint ("A member of the Order of Paladins sent to jail!");
	mprint ("It cannot be!");
	morewait();
	mprint ("You are immediately expelled permanently from the Order!");
	mprint ("Your name is expunged from the records....");
	Player.rank[ORDER] = FORMER_PALADIN;
    } else {
	pacify_guards();
	if (Level->environment != E_CITY) {
	    setgamestatus (SUPPRESS_PRINTING);
	    change_environment (E_CITY);
	    resetgamestatus (SUPPRESS_PRINTING);
	}
	if (gamestatusp (UNDEAD_GUARDS)) {
	    mprint ("You are taken to a weirdly deserted chamber where an undead");
	    mprint ("Magistrate presides over a court of ghosts and haunts.");
	    morewait();
	    mprint ("'Mr. Foreman, what is the verdict?'");
	    mprint ("'Guilty as charged, your lordship.'");
	    morewait();
	    clearmsg();
	    mprint ("'Guilty...");
	    napms(500);
	    mprint ("Guilty...");
	    napms(500);
	    mprint ("Guilty...");
	    napms(500);
	    mprint ("Guilty...'");
	    napms(500);
	    clearmsg();
	    mprint ("The members of the court close in around, fingers pointing.");
	    mprint ("You feel insubstantial hands closing around your throat....");
	    mprint ("You feel your life draining away!");
	    while (Player.level > 0) {
		--Player.level;
		Player.xp /= 2;
		Player.hp /= 2;
		dataprint();
	    }
	    Player.maxhp = Player.maxcon;
	    morewait();
	    mprint ("You are finally released, a husk of your former self....");
	    Player.x = 58;
	    Player.y = 40;
	    screencheck (58);
	} else if (Player.alignment + random_range (200) < 0) {
	    mprint ("Luckily for you, a smooth-tongued advocate from the");
	    mprint ("Rampart Chaotic Liberties Union gets you off!");
	    Player.x = 58;
	    Player.y = 40;
	    screencheck (58);
	} else {
	    switch (Imprisonment++) {
		case 0:
		    mprint ("The Magistrate sternly reprimands you.");
		    mprint ("As a first-time offender, you are given probation.");
		    Player.y = 58;
		    Player.x = 40;
		    screencheck (58);
		    break;
		case 1:
		    mprint ("The Magistrate expresses shame for your conduct.");
		    mprint ("You are thrown in jail!");
		    morewait();
		    repair_jail();
		    Player.y = 54;
		    Player.x = 37 + (2 * random_range (4));
		    screencheck (54);
		    l_portcullis_trap();
		    break;
		default:
		    mprint ("The Magistrate renders summary judgement.");
		    mprint ("You are sentenced to prison!");
		    morewait();
		    mprint ("The guards recognize you as a 'three-time-loser'");
		    mprint ("...and beat you up a little to teach you a lesson.");
		    p_damage (random_range(Imprisonment*10), UNSTOPPABLE, "police brutality");
		    morewait();
		    repair_jail();
		    Player.y = 54;
		    Player.x = 37 + (2 * random_range(4));
		    screencheck (54);
		    l_portcullis_trap();
	    }
	}
    }
}

void l_adept (void)
{
    mprint ("You see a giant shimmering gate in the form of an omega.");
    if (gamestatusp (SPOKE_TO_ORACLE)) {
	if (Player.str + Player.con + Player.iq + Player.pow < 100)
	    mprint ("A familiar female voice says: I would not advise this now....");
	else
	    mprint ("A familiar female voice says: Go for it!");
	morewait();
	clearmsg();
    }
    mprint ("Enter the mystic portal? [yn] ");
    if (ynq() != 'y') {
	if (Player.level > 100) {
	    mprint ("The Lords of Destiny spurn your cowardice....");
	    Player.xp = 0;
	    Player.level = 0;
	    Player.hp = Player.maxhp = Player.con;
	    Player.mana = Player.calcmana();
	    mprint ("You suddenly feel very inexperienced.");
	    dataprint();
	}
    } else {
	clearmsg();
	mprint ("You pass through the portal.");
	morewait();
	drawomega();
	mprint ("Like wow man! Colors! ");
	if (Player.patron != DESTINY) {
	    mprint ("Strange forces try to tear you apart!");
	    p_damage (random_range(200), UNSTOPPABLE, "a vortex of chaos");
	} else
	    mprint ("Some strange force shields you from a chaos vortex!");
	morewait();
	mprint ("Your head spins for a moment....");
	mprint ("and clears....");
	morewait();
	Player.hp = Player.maxhp;
	Player.mana = Player.calcmana();
	change_environment (E_ABYSS);
    }
}

void l_trifid (void)
{
    int damage = 0, stuck = true;
    mprint ("The hedge comes alive with a surge of alien growth!");
    while (stuck) {
	dataprint();
	damage += Level->depth / 2 + 1;
	mprint ("Razor-edged vines covered in suckers attach themselves to you.");
	morewait();
	if (find_and_remove_item (THING_SALT_WATER, -1)) {
	    mprint ("Thinking fast, you toss salt water on the trifid...");
	    mprint ("The trifid disintegrates with a frustrated sigh.");
	    Level->site(Player.x,Player.y).locchar = FLOOR;
	    Level->site(Player.x,Player.y).p_locf = L_NO_OP;
	    lset (Player.x, Player.y, CHANGED);
	    gain_experience (1000);
	    stuck = false;
	} else {
	    p_damage (damage, UNSTOPPABLE, "a trifid");
	    morewait();
	    mprint ("You are entangled in tendrils...");
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
			mprint ("Amazing! You're now free.");
			mprint ("The trifid writhes hungrily at you.");
			stuck = false;
		    } else
			mprint ("Well, THAT didn't work.");
		    break;
		case 'b':
		    mprint ("Well, at least you're facing your fate with dignity.");
		    break;
		case 'c':
		    if (Player.patron == DRUID && Player.rank[PRIESTHOOD] >= LAY+random_range(HIGHPRIEST-LAY)) {
			mprint ("A shaft of golden light bathes the alien plant");
			mprint ("which grudginly lets you go....");
			stuck = false;
		    } else
			mprint ("You receive no divine aid as yet.");
		    break;
		case 'd':
		    mprint ("The hedge doesn't answer your entreaties.");
		    break;
		case 'e':
		    mprint ("You forgot your ruby slippers, stupid.");
		    break;
		default:
		    mprint ("The hedge enjoys your camp play-acting....");
		    break;
	    }
	}
    }
    xredraw();
}

void l_vault (void)
{
    mprint ("You come to a thick vault door with a complex time lock.");
    if (hour() == 23) {
	mprint ("The door is open.");
	Level->site(12,56).locchar = FLOOR;
    } else {
	mprint ("The door is closed.");
	Level->site(12,56).locchar = WALL;
	morewait();
	clearmsg();
	mprint ("Try to crack it? [yn] ");
	if (ynq() == 'y') {
	    if (random_range (100) < Player.rank[THIEVES] * Player.rank[THIEVES]) {
		mprint ("The lock clicks open!!!");
		gain_experience (5000);
		Level->site(12,56).locchar = FLOOR;
	    } else {
		mprint ("Uh, oh, set off the alarm.... The castle guard arrives....");
		morewait();
		if (Player.rank[NOBILITY] == DUKE) {
		    clearmsg();
		    mprint ("\"Ah, just testing us, your Grace?  I hope we're up to scratch.\"");
		    morewait();
		} else
		    send_to_jail();
	    }
	} else
	    mprint ("Good move.");
    }
}

void l_brothel (void)
{
    char response;
    mprint ("You come to a heavily reinforced inner door.");
    mprint ("A sign reads `The House of the Eclipse'");
    morewait();
    clearmsg();
    mprint ("Try to enter? [yn] ");
    if (ynq() == 'y') {
	menuclear();
	menuprint ("a:knock on the door.\n");
	menuprint ("b:try to pick the lock.\n");
	menuprint ("c:bash down the door.\n");
	menuprint ("ESCAPE: Leave this house of ill repute.\n");
	showmenu();
	do
	    response = menugetc();
	while (response != 'a' && response != 'b' && response != 'c' && response != KEY_ESCAPE);
	xredraw();
	if (response == 'a') {
	    if (!nighttime())
		mprint ("There is no reponse.");
	    else {
		mprint ("A window opens in the door.");
		mprint ("`500Au, buddy. For the night.' pay it? [yn] ");
		if (ynq() == 'y') {
		    if (Player.cash < 500) {
			mprint ("`What, no roll?!'");
			mprint ("The bouncer bounces you a little and lets you go.");
			p_damage (25, UNSTOPPABLE, "da bouncer");
		    } else {
			Player.cash -= 500;
			mprint ("You are ushered into an opulently appointed hall.");
			mprint ("After an expensive dinner (takeout from Les Crapuleux)");
			morewait();
			if (Player.preference == 'n') {
			    static const char _nopref[] =
				"you spend the evening playing German Whist with\0"
				"you spend the evening discussing philosophy with\0"
				"you spend the evening playing chess against\0"
				"you spend the evening telling your adventures to";
			    mprint (zstrn(_nopref, xrand()%4, 4));
			    mprint ("various employees of the House of the Eclipse.");
			} else {
			    mprint ("you spend an enjoyable and educational evening with");
			    if (Player.preference == 'm' || (Player.preference == 'y' && random_range (2))) {
				static const char _mpref[] =
				    "Skarn the Insatiable, a satyr.\0"
				    "Dryden the Defanged, an incubus.\0"
				    "Gorgar the Equipped, a centaur.\0"
				    "Hieronymus, the mendicant priest of Eros.";
				mprint (zstrn(_mpref, xrand()%4, 4));
			    } else {
				static const char _fpref[] =
				    "Noreen the Nymph (omaniac)\0"
				    "Angelface, a recanted succubus.\0"
				    "Corporal Sue of the City Guard (moonlighting).\0"
				    "Sheena the Queena the Jungle, a wereleopard.";
				mprint (zstrn(_fpref, xrand()%4, 4));
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
			    ++Player.iq;	// whatever :-)
			else if (Player.maxhp < ++Player.con)
			    ++Player.maxhp;
			Player.hp = Player.maxhp;
			gain_experience (100);
			timeprint();
			dataprint();
			showflags();
			morewait();
			clearmsg();
			if (Player.preference == 'n')
			    mprint ("You arise refreshed the next morning...");
			else
			    mprint ("You arise, tired but happy, the next morning...");
		    }
		} else
		    mprint ("What are you, some kinda prude?");
	    }
	} else if (response == 'b') {
	    if (nighttime()) {
		mprint ("As you fumble at the lock, the door opens....");
		mprint ("The bouncer tosses you into the street.");
	    } else
		mprint ("The door appears to be bolted and barred from behind.");
	} else if (response == 'c') {
	    if (nighttime()) {
		mprint ("As you charge toward the door it opens....");
		mprint ("Yaaaaah! Thud!");
		morewait();
		mprint ("You run past the startled bouncer into a wall.");
		p_damage (20, UNSTOPPABLE, "a move worthy of Clouseau");
		mprint ("The bouncer tosses you into the street.");
	    } else {
		mprint ("Ouch! The door resists your efforts.");
		p_damage (1, UNSTOPPABLE, "a sturdy door");
		morewait();
		mprint ("You hear an irritated voice from inside:");
		mprint ("'Keep it down out there! Some of us are trying to sleep!'");
	    }
	}
    }
}

// if signp is true, always print message, otherwise do so only sometimes
void sign_print (int x, int y, int signp)
{
    if (Level->site(x,y).p_locf >= CITYSITEBASE && Level->site(x,y).p_locf < CITYSITEBASE + NUMCITYSITES)
	CitySiteList[Level->site(x,y).p_locf - CITYSITEBASE].known = true;
    switch (Level->site(x,y).p_locf) {
	case L_CHARITY:
	    mprint ("You notice a sign: The Rampart Orphanage And Hospice For The Needy.");
	    break;
	case L_MANSION:
	    mprint ("You notice a sign:");
	    mprint ("This edifice protected by DeathWatch Devices, Ltd.");
	    morewait();
	    break;
	case L_GRANARY:
	    mprint ("You notice a sign:");
	    mprint ("Public Granary: Entrance Strictly Forbidden.");
	    break;
	case L_PORTCULLIS:
	    if (Level->site(x,y).locchar == FLOOR)
		mprint ("You see a groove in the floor and slots above you.");
	    break;
	case L_STABLES:
	    mprint ("You notice a sign:");
	    mprint ("Village Stables");
	    break;
	case L_COMMONS:
	    mprint ("You notice a sign:");
	    mprint ("Village Commons: No wolves allowed.");
	    break;
	case L_MAZE:
	    mprint ("You notice a sign:");
	    mprint ("Hedge maze closed for trifid extermination.");
	    break;
	case L_BANK:
	    if (signp) {
		mprint ("You notice a sign:");
		mprint ("First Bank of Omega: Autoteller Carrel.");
	    }
	    break;
	case L_TEMPLE:
	    mprint ("You see the ornate portico of the Rampart Pantheon");
	    break;
	case L_ARMORER:
	    if (signp) {
		mprint ("You notice a sign:");
		mprint ("Julie's Armor of Proof and Weapons of Quality");
	    }
	    break;
	case L_CLUB:
	    if (signp) {
		mprint ("You notice a sign:");
		mprint ("Rampart Explorers' Club.");
	    }
	    break;
	case L_GYM:
	    if (signp) {
		mprint ("You notice a sign:");
		mprint ("The Rampart Gymnasium, (affil. Rampart Coliseum).");
	    }
	    break;
	case L_HEALER:
	    if (signp) {
		mprint ("You notice a sign:");
		mprint ("Rampart Healers. Member RMA.");
	    }
	    break;
	case L_CASINO:
	    if (signp) {
		mprint ("You notice a sign:");
		mprint ("Rampart Mithril Nugget Casino.");
	    }
	    break;
	case L_SEWER:
	    mprint ("A sewer entrance. You don't want to go down THERE, do you?");
	    break;
	case L_COMMANDANT:
	    if (signp) {
		mprint ("You notice a sign:");
		mprint ("Commandant Sonder's Rampart-fried Lyzzard partes. Open 24 hrs.");
	    }
	    break;
	case L_DINER:
	    if (signp) {
		mprint ("You notice a sign:");
		mprint ("The Rampart Diner. All you can eat, 25Au.");
	    }
	    break;
	case L_CRAP:
	    if (signp) {
		mprint ("You notice a sign:");
		mprint ("Les Crapeuleaux. (****)");
	    }
	    break;
	case L_TAVERN:
	    if (signp) {
		mprint ("You notice a sign:");
		mprint ("The Centaur and Nymph -- J. Riley, prop.");
	    }
	    break;
	case L_ALCHEMIST:
	    if (signp) {
		mprint ("You notice a sign:");
		mprint ("Ambrosias' Potions et cie.");
	    }
	    break;
	case L_DPW:
	    if (signp) {
		mprint ("You notice a sign:");
		mprint ("Rampart Department of Public Works.");
	    }
	    break;
	case L_LIBRARY:
	    if (signp) {
		mprint ("You notice a sign:");
		mprint ("Rampart Public Library.");
	    }
	    break;
	case L_CONDO:
	    if (signp) {
		mprint ("You notice a sign:");
		if (gamestatusp (SOLD_CONDO))
		    mprint ("Home Sweet Home");
		else
		    mprint ("Luxury Condominium For Sale; Inquire Within");
	    }
	    break;
	case L_PAWN_SHOP:
	    if (signp) {
		mprint ("You notice a sign:");
		mprint ("Knight's Pawn Shop.");
	    }
	    break;
	case L_CEMETARY:
	    mprint ("You notice a sign:");
	    mprint ("Rampart City Cemetary. Closed -- Full.");
	    break;
	case L_GARDEN:
	    mprint ("You notice a sign:");
	    mprint ("Rampart Botanical Gardens---Do not deface statues.");
	    break;
	case L_JAIL:
	    mprint ("You notice a sign:");
	    mprint ("Rampart City Gaol -- always room for more.");
	    break;
	case L_ORACLE:
	    mprint ("You notice a sign:");
	    mprint ("The Oracle of the Cyan Flames");
	    morewait();
	    break;
    }
}

void l_countryside (void)
{
    if (optionp (CONFIRM)) {
	clearmsg();
	mprint ("Do you really want to return to the countryside? ");
	if (ynq() != 'y')
	    return;
    }
    change_environment (E_COUNTRYSIDE);
}

void l_oracle (void)
{
    mprint ("You come before a blue crystal dais. There is a bell and a mirror.");
    mprint ("Ring the bell [b], look in the mirror [m], or leave [ESCAPE] ");
    char response;
    do
	response = mcigetc();
    while (response != 'b' && response != 'm' && response != KEY_ESCAPE);
    if (response == 'b') {
	mprint ("The ringing note seems to last forever.");
	mprint ("You notice a robed figure in front of you....");
	morewait();
	mprint ("The oracle doffs her cowl. Her eyes glitter with blue fire!");
	mprint ("She stares at you...and speaks:");
	setgamestatus (SPOKE_TO_ORACLE);
	if (!gamestatusp (SPOKE_TO_DRUID)) {
	    mprint ("'The ArchDruid speaks wisdom in his forest shrine.'");
	} else if (!gamestatusp (COMPLETED_CAVES)) {
	    mprint ("'Thou mayest find aught of interest in the caves to the South.'");
	} else if (!gamestatusp (COMPLETED_SEWERS)) {
	    mprint ("'Turn thy attention to the abyssal depths of the city.'");
	} else if (!gamestatusp (COMPLETED_CASTLE)) {
	    mprint ("'Explorest thou the depths of the Castle of the ArchMage.'");
	} else if (!gamestatusp (COMPLETED_ASTRAL)) {
	    morewait();
	    mprint ("'Journey to the Astral Plane and meet the Gods' servants.'");
	    mprint ("The oracle holds out her hand. Do you take it? [yn] ");
	    if (ynq() == 'y') {
		mprint ("'Beware: Only the Star Gem can escape the Astral Plane.'");
		mprint ("A magic portal opens behind the oracle. She leads you");
		morewait();
		mprint ("through a sequence of special effects that would have");
		mprint ("IL&M technicians cursing in awe and deposits you in an");
		morewait();
		clearmsg();
		mprint ("odd looking room whose walls seem strangely insubstantial.");
		gain_experience (5000);
		change_environment (E_ASTRAL);
	    } else
		mprint ("You detect the hint of a sneer from the oracle.");
	} else if (!gamestatusp (COMPLETED_VOLCANO)) {
	    mprint ("'The infernal maw may yield its secrets to thee now.'");
	} else if (!gamestatusp (COMPLETED_CHALLENGE)) {
	    mprint ("'The challenge of adepthood yet awaits thee.'");
	} else {
	    morewait();
	    mprint ("'My lord: Thou hast surpassed my tutelage forever.");
	    mprint ("Fare thee well.'");
	    mprint ("The oracle replaces her hood and seems to fade away....");
	}
    } else if (response == 'm') {
	mprint ("You seem to see yourself. Odd....");
	knowledge (1);
    } else
	mprint ("You leave this immanent place.");
}

void l_mansion (void)
{
    mprint ("Enter the mansion? [yn] ");
    if (ynq() == 'y')
	change_environment (E_MANSION);
}

void l_house (void)
{
    mprint ("Enter the house? [yn] ");
    if (ynq() == 'y')
	change_environment (E_HOUSE);
}

void l_hovel (void)
{
    mprint ("Enter the hovel? [yn] ");
    if (ynq() == 'y')
	change_environment (E_HOVEL);
}

void l_safe (void)
{
    char response;
    int attempt = 0;
    mprint ("You have discovered a safe!");
    mprint ("Pick the lock [p], Force the door [f], or ignore [ESCAPE]");
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
	mprint ("The door springs open!");
	Level->site(Player.x,Player.y).locchar = FLOOR;
	Level->site(Player.x,Player.y).p_locf = L_NO_OP;
	lset (Player.x, Player.y, CHANGED);
	if (random_range(2)) {
	    mprint ("You find:");
	    do {
		object newitem = create_object (difficulty());
		mprint (itemid (newitem));
		gain_item (newitem);
		morewait();
	    } while (!random_range(3));
	} else
	    mprint ("The safe was empty (awwwww....)");
    } else {
	mprint ("Your attempt at burglary failed!");
	if (attempt == -1) {
	    mprint ("A siren goes off! You see flashing red lights everywhere!");
	    morewait();
	    if (World.LastEnvironment() == E_CITY) {
		mprint ("The city guard shows up! They collar you in no time flat!");
		change_environment (E_CITY);
		morewait();
		send_to_jail();
	    }
	} else if (attempt == -2) {
	    mprint ("There is a sudden flash!");
	    p_damage (random_range (25), FLAME, "a safe");
	    mprint ("The safe has self-destructed.");
	    Level->site(Player.x,Player.y).locchar = RUBBLE;
	    Level->site(Player.x,Player.y).p_locf = L_RUBBLE;
	    lset (Player.x, Player.y, CHANGED);
	} else if (attempt == -3) {
	    mprint ("The safe jolts you with electricity!");
	    lball (Player.x, Player.y, Player.x, Player.y, 30);
	} else if (attempt < -3) {
	    mprint ("You are hit by an acid spray!");
	    if (Player.has_possession(O_CLOAK)) {
		mprint ("Your cloak is destroyed!");
		Player.remove_possession (O_CLOAK);
		p_damage (10, ACID, "a safe");
	    } else if (Player.has_possession(O_ARMOR)) {
		mprint ("Your armor corrodes!");
		Player.possessions[O_ARMOR].dmg -= 3;
		Player.possessions[O_ARMOR].hit -= 3;
		Player.possessions[O_ARMOR].aux -= 3;
		p_damage (10, ACID, "a safe");
	    } else {
		mprint ("The acid hits your bare flesh!");
		p_damage (random_range (100), ACID, "a safe");
	    }
	}
    }
}

void l_cartographer (void)
{
    mprint ("Ye Olde Mappe Shoppe.");
    mprint ("Map of the local area: 500Au. Buy it? [yn] ");
    if (ynq() == 'y') {
	if (Player.cash < 500)
	    mprint ("Cursed be cheapskates! May you never find an aid station....");
	else {
	    mprint ("You now have the local area mapped.");
	    Player.cash -= 500;
	    dataprint();
	    int x, y;
	    switch (Level->VillageId()) {
		case 0: x = 56; y =  5; break;
		default:
		case 1: x = 35; y = 11; break;
		case 2: x = 10; y = 40; break;
		case 3: x =  7; y =  6; break;
		case 4: x = 40; y = 43; break;
		case 5: x = 20; y = 41; break;
	    }
	    auto country = World.FindEnvironment (E_COUNTRYSIDE);
	    for (int i = x - 15; i <= x + 15; ++i)
		for (int j = y - 15; j <= y + 15; ++j)
		    if (i >= 0 && i < country->width && j >= 0 && j < country->height)
			country->site(i,j).lstatus |= SEEN;
	}
    } else
	mprint ("Don't blame me if you get lost....");
}

void l_charity (void)
{
    long donation;
    mprint ("'Greetings, friend. Do you wish to make a donation?' [yn] ");
    if (ynq() != 'y')
	mprint ("'Pinchpurse!'");
    else {
	clearmsg();
	mprint ("How much can you give? ");
	donation = parsenum();
	if (donation < 1)
	    mprint ("'Go stick your head in a pig.'");
	else if (donation > Player.cash)
	    mprint ("'I'm afraid you're charity is bigger than your purse!'");
	else if (donation < max (100, Player.level * Player.level * 100)) {
	    mprint ("'Oh, can't you do better than that?'");
	    mprint ("'Well, I guess we'll take it....'");
	    if (Player.alignment < 10)
		Player.alignment++;
	    Player.cash -= donation;
	} else {
	    mprint ("'Oh thank you kindly, friend, and bless you!'");
	    Player.cash -= donation;
	    Player.alignment += 5;
	}
    }
    dataprint();
}
