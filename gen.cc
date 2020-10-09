// Omega is free software, distributed under the MIT license

#include "glob.h"
#include <time.h>

//----------------------------------------------------------------------

static void build_room(int x, int y, int l, char rsi, int baux);
static void corridor_crawl(int *fx, int *fy, int sx, int sy, int n, chtype loc, char rsi);
static void find_stairs (void);
static void make_forest (void);
static void make_country_screen (char terrain);
static void make_country_monsters (char terrain);
static void make_general_map(const char* terrain);
static void make_jungle(void);
static void make_mountains(void);
static void make_plains(void);
static void make_river(void);
static void make_road(void);
static void make_swamp(void);
static void makedoor(int x, int y);
static void maze_corridor(int fx, int fy, int tx, int ty, int rsi, int num);
static void room_corridor(int fx, int fy, int tx, int ty, int baux);
static void sewer_corridor(int x, int y, int dx, int dy, chtype locchar);
static void straggle_corridor(int fx, int fy, int tx, int ty, chtype loc, char rsi);
static void cavern_level(void);
static void install_specials(void);
static void install_traps(void);
static void make_stairs(void);
static void maze_level(void);
static void room_level(void);
static void sewer_level(void);

//----------------------------------------------------------------------

// erase the level w/o deallocating it
void clear_level (struct level *dungeon_level)
{
    if (dungeon_level)
	dungeon_level->clear();
}

void generate_level (int tolevel)
{
    deepest[Level->environment] = max<int> (deepest[Level->environment], tolevel);
    initrand (Level->environment, tolevel);
    Level->depth = tolevel;
    Level->generated = true;
    Level->resize (64, 64);
    switch (Level->environment) {
	default:
	case E_CAVES:
	    if (!random_range(4) && tolevel < Level->MaxDepth())
		room_level();
	    else
		cavern_level();
	    break;
	case E_SEWERS:
	    if (!random_range(4) && tolevel < Level->MaxDepth())
		room_level();
	    else
		sewer_level();
	    break;
	case E_CASTLE:
	    room_level();
	    break;
	case E_VOLCANO:
	    switch (random_range(3)) {
		case 0: cavern_level(); break;
		case 1: room_level(); break;
		case 2: maze_level(); break;
	    }
	    break;
	case E_ASTRAL:
	    maze_level();
	    break;
    }
    install_traps();
    install_specials();
    make_stairs();
    make_stairs();
    if (!(Level->lastx|Level->lasty))
	find_stairs();
    initrand (E_RESTORE, 0);
}

// Looks for level tolevel in current dungeon which is named by
// Dungeon, which may be NULL. If the level is found, and rewrite_level
// is false, and the level has already been generated, nothing happens
// beyond Level being set correctly. Otherwise the level is recreated
// from scratch
void change_level (int, int tolevel, int rewrite_level)
{
    assert (Level);
    if (rewrite_level)
	World.DeleteLevel (Level->environment, tolevel);
    World.LoadEnvironment (Level->environment, tolevel);
    ScreenOffset = Player.y - (ScreenLength / 2);
    show_screen();
    screencheck (Player.y);
    drawvision (Player.x, Player.y);
    // synchronize with player on level change
    Player.click = (Tick + 1) % 60;
    roomcheck();
}

// keep going in one orthogonal direction or another until we hit our destination
static void straggle_corridor (int fx, int fy, int tx, int ty, chtype loc, char rsi)
{
    while (fx != tx || fy != ty) {
	int dx = tx - fx;
	int dy = ty - fy;
	if (urandom_range (absv(dx) + absv(dy)) < absv(dx))
	    corridor_crawl (&fx, &fy, sign (dx), 0, random_range (absv (dx)) + 1, loc, rsi);
	else
	    corridor_crawl (&fx, &fy, 0, sign (dy), random_range (absv (dy)) + 1, loc, rsi);
    }
}

static void makedoor (int x, int y)
{
    if (random_range (20) <= Level->depth / 10) {
	Level->site(x,y).locchar = FLOOR;
	lset (x, y, SECRET);
    } else if (random_range (20) <= Level->depth / 2) {
	Level->site(x,y).locchar = CLOSED_DOOR;
	if (random_range (20) <= Level->depth / 10)
	    lset (x, y, SECRET);
	if (random_range (40) <= Level->depth)
	    Level->site(x,y).aux = LOCKED;
	else
	    Level->site(x,y).aux = UNLOCKED;
    } else {
	Level->site(x,y).locchar = OPEN_DOOR;
	Level->site(x,y).aux = UNLOCKED;
    }
    if (!loc_statusp (x, y, SECRET)) {
	lset (x, y + 1, STOPS);
	lset (x + 1, y, STOPS);
	lset (x - 1, y, STOPS);
	lset (x, y - 1, STOPS);
	lset (x, y, STOPS);
    }
    Level->site(x,y).p_locf = L_NO_OP;
    // prevents water corridors from being instant death in sewers
}

static void corridor_crawl (int* fx, int* fy, int sx, int sy, int n, chtype loc, char rsi)
{
    int i;
    for (i = 0; i < n; i++) {
	*fx += sx;
	*fy += sy;
	if ((*fx < (int)Level->width) && (*fx > -1) && (*fy > -1) && (*fy < (int)Level->height)) {
	    Level->site(*fx,*fy).locchar = loc;
	    if (Level->site(*fx,*fy).roomnumber == RS_WALLSPACE)
		Level->site(*fx,*fy).roomnumber = rsi;
	    if (loc == WATER)
		Level->site(*fx,*fy).p_locf = L_WATER;
	    else if (loc == FLOOR)
		Level->site(*fx,*fy).p_locf = L_NO_OP;
	    else if (loc == RUBBLE)
		Level->site(*fx,*fy).p_locf = L_RUBBLE;
	}
    }
}

// puts the player on the first set of stairs from the apt level
// if can't find them, just drops player anywhere....
static void find_stairs (void)
{
    bool found = false;
    for (unsigned i = 0; i < Level->width; ++i) {
	for (unsigned j = 0; j < Level->height; ++j) {
	    if (Level->site(i,j).locchar == STAIRS_UP && !found) {
		found = true;	// Always STAIRS_UP because when going up the levels always exist
		Level->lastx = i;
		Level->lasty = j;
		break;
	    }
	}
    }
    if (!found) {
	int x,y;
	findspace (&x, &y);
	Level->lastx = x; Level->lasty = y;
    }
}

static void install_traps (void)
{
    for (unsigned i = 0; i < Level->width; i++)
	for (unsigned j = 0; j < Level->height; j++)
	    if (Level->site(i,j).locchar == FLOOR && Level->site(i,j).p_locf == L_NO_OP && random_range (500) <= (int)(Level->depth / 6))
		Level->site(i,j).p_locf = TRAP_BASE + random_range (NUMTRAPS);
}

// x, y, is top left corner, l is length of side, rsi is room string index
// baux is so all rooms will have a key field.
static void build_room (int x, int y, int l, char rsi, int baux)
{
    for (int j = y; j <= y + l; j++) {
	for (int i = x; i <= x + l; i++) {
	    Level->site(i,j).roomnumber = rsi;
	    if (baux)
		Level->site(i,j).aux = baux;
	}
    }
    for (int j = y + 1; j < y + l; j++) {
	for (int i = x + 1; i < x + l; i++) {
	    Level->site(i,j).locchar = FLOOR;
	    Level->site(i,j).p_locf = L_NO_OP;
	}
    }
}

static void cavern_level (void)
{
    int i, fx, fy, tx, ty, t, l, e;
    char rsi;

    if (Level->environment == E_CAVES && Level->depth == CAVELEVELS)
	rsi = RS_GOBLINKING;
    else
	rsi = RS_CAVERN;
    t = random_range (Level->height / 2);
    l = random_range (Level->width / 2);
    e = random_range (Level->width / 8) + Level->width / 8;
    build_room (t, l, e, rsi, 0);

    for (i = 0; i < 16; i++) {
	findspace (&tx, &ty);
	fx = random_range (Level->width - 2) + 1;
	fy = random_range (Level->height - 2) + 1;
	straggle_corridor (fx, fy, tx, ty, FLOOR, RS_CORRIDOR);
    }
    while (random_range (3) == 1) {
	findspace (&tx, &ty);
	fx = random_range (Level->width - 2) + 1;
	fy = random_range (Level->height - 2) + 1;
	straggle_corridor (fx, fy, tx, ty, WATER, RS_PONDS);
    }
    if (Level->environment == E_CAVES) {
	if ((Level->depth == CAVELEVELS) && (!gamestatusp (COMPLETED_CAVES))) {
	    findspace (&tx, &ty);
	    make_site_monster (tx, ty, GOBLIN_KING);
	}
    } else if (Level->environment == E_VOLCANO) {
	if (Level->depth == VOLCANOLEVELS) {
	    findspace (&tx, &ty);
	    make_site_monster (tx, ty, DEMON_EMP);
	}
    }
}

static void sewer_level (void)
{
    int tx, ty, t, l, e;
    char rsi;
    chtype lchar;

    rsi = RS_DRAINED_SEWER;
    for (unsigned i = 0, numrooms = random_range(3)+3; i < numrooms; ++i) {
	do {
	    t = random_range (Level->height - 10) + 1;
	    l = random_range (Level->width - 10) + 1;
	    e = 4;
	} while ((Level->site(l,t).roomnumber == rsi) || (Level->site(l + e,t).roomnumber == rsi) || (Level->site(l,t + e).roomnumber == rsi) || (Level->site(l + e,t + e).roomnumber == rsi));
	if (random_range (5)) {
	    lchar = FLOOR;
	    rsi = RS_DRAINED_SEWER;
	} else {
	    lchar = WATER;
	    rsi = RS_DROWNED_SEWER;
	}
	build_room (l, t, e, rsi, 0);
	sewer_corridor (l, t, -1, -1, lchar);
	sewer_corridor (l + e, t, 1, -1, lchar);
	sewer_corridor (l, t + e, -1, 1, lchar);
	sewer_corridor (l + e, t + e, 1, 1, lchar);
    }
    if (Level->environment == E_SEWERS) {
	if ((Level->depth == SEWERLEVELS) && (!gamestatusp (COMPLETED_SEWERS))) {
	    findspace (&tx, &ty);
	    make_site_monster (tx, ty, GREAT_WYRM);
	}
    }
}

static void sewer_corridor (int x, int y, int dx, int dy, chtype locchar)
{
    int continuing = true;
    makedoor (x, y);
    x += dx;
    y += dy;
    while (continuing) {
	Level->site(x,y).locchar = locchar;
	if (locchar == WATER)
	    Level->site(x,y).p_locf = L_WATER;
	else
	    Level->site(x,y).p_locf = L_NO_OP;
	Level->site(x,y).roomnumber = RS_SEWER_DUCT;
	x += dx;
	y += dy;
	if (locchar == WATER)
	    continuing = (inbounds (x, y) && (Level->site(x,y).locchar == WALL || Level->site(x,y).locchar == WATER));
	else
	    continuing = (inbounds (x, y) && (Level->site(x,y).roomnumber == RS_WALLSPACE || Level->site(x,y).roomnumber == RS_SEWER_DUCT));
    }
    if (inbounds (x, y))
	makedoor (x, y);
}

static void install_specials (void)
{
    for (unsigned x = 0; x < Level->width; x++) {
	for (unsigned y = 0; y < Level->height; y++) {
	    if (Level->site(x,y).locchar == FLOOR && Level->site(x,y).p_locf == L_NO_OP && random_range(300) < difficulty()) {
		unsigned i = random_range(100);
		if (i < 10) {
		    Level->site(x,y).locchar = ALTAR;
		    Level->site(x,y).p_locf = L_ALTAR;
		    Level->site(x,y).aux = random_range(10);
		} else if (i < 20) {
		    Level->site(x,y).locchar = WATER;
		    Level->site(x,y).p_locf = L_MAGIC_POOL;
		} else if (i < 35) {
		    Level->site(x,y).locchar = RUBBLE;
		    Level->site(x,y).p_locf = L_RUBBLE;
		} else if (i < 40) {
		    Level->site(x,y).locchar = LAVA;
		    Level->site(x,y).p_locf = L_LAVA;
		} else if (i < 45) {
		    Level->site(x,y).locchar = FIRE;
		    Level->site(x,y).p_locf = L_FIRE;
		} else if (i < 50 && Level->environment != E_ASTRAL) {
		    Level->site(x,y).locchar = LIFT;
		    Level->site(x,y).p_locf = L_LIFT;
		} else if (i < 55 && Level->environment != E_VOLCANO) {
		    Level->site(x,y).locchar = HEDGE;
		    Level->site(x,y).p_locf = L_HEDGE;
		} else if (i < 57) {
		    Level->site(x,y).locchar = HEDGE;
		    Level->site(x,y).p_locf = L_TRIFID;
		} else if (i < 70) {
		    Level->site(x,y).locchar = STATUE;
		    if (random_range(100) < difficulty()) {
			for (unsigned j = 0; j < 8; j++) {
			    if (Level->site(x + Dirs[0][j],y + Dirs[1][j]).p_locf != L_NO_OP)
				Level->site(x + Dirs[0][j],y + Dirs[1][j]).locchar = FLOOR;
			    Level->site(x + Dirs[0][j],y + Dirs[1][j]).p_locf = L_STATUE_WAKE;
			}
		    }
		} else {
		    if (Level->environment == E_VOLCANO) {
			Level->site(x,y).locchar = LAVA;
			Level->site(x,y).p_locf = L_LAVA;
		    } else if (Level->environment == E_ASTRAL) {
			if (Level->depth == 1) {
			    Level->site(x,y).locchar = RUBBLE;
			    Level->site(x,y).p_locf = L_RUBBLE;
			} else if (Level->depth == 2) {
			    Level->site(x,y).locchar = FIRE;
			    Level->site(x,y).p_locf = L_FIRE;
			} else if (Level->depth == 3) {
			    Level->site(x,y).locchar = WATER;
			    Level->site(x,y).p_locf = L_WATER;
			} else if (Level->depth == 4) {
			    Level->site(x,y).locchar = ABYSS;
			    Level->site(x,y).p_locf = L_ABYSS;
			}
		    } else {
			Level->site(x,y).locchar = WATER;
			Level->site(x,y).p_locf = L_WATER;
		    }
		}
	    }
	}
    }
}

// For each level, there should be one stairway going up and one down.
static void make_stairs (void)
{
    int i, j;
    // no stairway out of astral
    if (Level->environment != E_ASTRAL) {
	findspace (&i, &j);
	Level->site(i,j).locchar = STAIRS_UP;
	Level->site(i,j).aux = Level->depth - 1;
	lset (i, j, STOPS);
    }
    if (Level->depth < Level->MaxDepth()) {
	findspace (&i, &j);
	Level->site(i,j).locchar = STAIRS_DOWN;
	Level->site(i,j).aux = Level->depth + 1;
	lset (i, j, STOPS);
    }
}

void load_encounter (char countryLocChar)
{
    make_country_screen (countryLocChar);
    make_country_monsters (countryLocChar);
    uint8_t x = Level->width / 2, y = Level->height / 2;
    while (Level->site(x,y).locchar == WATER) {
	if (y < Level->height / 2 + 5u)
	    ++y;
	else if (x > Level->width / 2 - 10u) {
	    --x;
	    y = Level->height / 2 - 5;
	} else {
	    Level->site(x,y).locchar = FLOOR;
	    Level->site(x,y).p_locf = L_NO_OP;
	}
    }
    Level->lastx = x; Level->lasty = y;
}

// tactical map generating functions
static void make_country_screen (char terrain)
{
    Level->environment = E_TACTICAL_MAP;
    Level->resize (64, 16);
    Level->generated = true;
    switch (terrain) {
	case char(FOREST):	make_forest(); break;
	case char(JUNGLE):	make_jungle(); break;
	case char(SWAMP):	make_swamp(); break;
    case char(LAKE):
	case char(RIVER):	make_river(); break;
	case char(MOUNTAINS):
	case char(PASS):	make_mountains(); break;
	case char(ROAD):	make_road(); break;
	default:		make_plains(); break;
    }
    if (nighttime()) {
	mprint ("Night's gloom shrouds your sight.");
	for (unsigned i = 0; i < Level->width; ++i)
	    for (unsigned j = 0; j < Level->height; ++j)
		Level->site(i,j).lstatus = 0;
    }
}

// monsters for tactical encounters
static void make_country_monsters (char terrain)
{
    static const int8_t plains[] = { BUNNY, BUNNY, HORNET, QUAIL, HAWK, DEER, WOLF, LION, BRIGAND, RANDOM };
    static const int8_t forest[] = { BUNNY, QUAIL, HAWK, BADGER, DEER, DEER, WOLF, BEAR, BRIGAND, RANDOM };
    static const int8_t jungle[] = { ANTEATER, PARROT, MAMBA, ANT, ANT, HYENA, HYENA, ELEPHANT, LION, RANDOM };
    static const int8_t river[] = { QUAIL, TROUT, TROUT, MANOWAR, BASS, BASS, CROC, CROC, BRIGAND, RANDOM };
    static const int8_t swamp[] = { BASS, BASS, CROC, CROC, BOGTHING, ANT, ANT, RANDOM, RANDOM, RANDOM };
    static const int8_t desert[] = { HAWK, HAWK, CAMEL, CAMEL, HYENA, HYENA, LION, LION, RANDOM, RANDOM };
    static const int8_t tundra[] = { WOLF, WOLF, BEAR, BEAR, DEER, DEER, RANDOM, RANDOM, RANDOM, RANDOM };
    static const int8_t mountain[] = { BUNNY, SHEEP, WOLF, WOLF, HAWK, HAWK, HAWK, RANDOM, RANDOM, RANDOM };
    const int8_t* monsters = plains;
    switch (terrain) {
	case char(FOREST):	monsters = forest; break;
	case char(JUNGLE):	monsters = jungle; break;
    case char(LAKE):
	case char(RIVER):	monsters = river;  break;
	case char(SWAMP):	monsters = swamp;  break;
	case char(DESERT):	monsters = desert; break;
	case char(TUNDRA):	monsters = tundra; break;
	case char(MOUNTAINS):
	case char(PASS):	monsters = mountain; break;
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

static void make_general_map (const char* terrain)
{
    int size = strlen (terrain);
    char curr;
    for (unsigned i = 0; i < Level->width; ++i) {
	for (unsigned j = 0; j < Level->height; ++j) {
	    if ((i == 0 && j == 0) || !random_range (5))
		curr = terrain[random_range (size)];
	    else if (j == 0 || (random_range (2) && i > 0))
		curr = Level->site(i - 1,j).locchar & 0xff;
	    else
		curr = Level->site(i,j - 1).locchar & 0xff;
	    switch (curr) {
		case (FLOOR & 0xff):
		    Level->site(i,j).locchar = FLOOR;
		    Level->site(i,j).p_locf = L_NO_OP;
		    break;
		case (HEDGE & 0xff):
		    Level->site(i,j).locchar = HEDGE;
		    Level->site(i,j).p_locf = L_HEDGE;
		    break;
		case (WATER & 0xff):
		    Level->site(i,j).locchar = WATER;
		    Level->site(i,j).p_locf = L_WATER;
		    break;
		case (RUBBLE & 0xff):
		    Level->site(i,j).locchar = RUBBLE;
		    Level->site(i,j).p_locf = L_RUBBLE;
		    break;
	    }
	    Level->site(i,j).lstatus = SEEN + LIT;
	    Level->site(i,j).roomnumber = RS_COUNTRYSIDE;
	    if (!i || !j || i+1 == Level->width || j+1 == Level->height)
		Level->site(i,j).p_locf = L_TACTICAL_EXIT;
	}
    }
}

static void make_plains (void)
{
    make_general_map (".");
}

static void make_road (void)
{
    make_general_map ("\"\"~4....");
    for (unsigned x = Level->width / 2 - 3; x <= Level->width / 2 + 3u; ++x) {
	for (unsigned y = 0; y < Level->height; ++y) {
	    Level->site(x,y).locchar = FLOOR;
	    if (y && y+1 != Level->height)
		Level->site(x,y).p_locf = L_NO_OP;
	}
    }
}

static void make_forest (void)
{
    make_general_map ("\".");
    straggle_corridor (0, random_range (Level->height), Level->width, random_range (Level->height), WATER, RS_COUNTRYSIDE);
}

static void make_jungle (void)
{
    make_general_map ("\"\".");
}

static void make_river (void)
{
    make_general_map ("\".......");
    unsigned y = random_range (Level->height);
    unsigned y1 = random_range (Level->height);
    straggle_corridor (0, y, Level->width, y1, WATER, RS_COUNTRYSIDE);
    for (unsigned i = 0; i < 7; ++i) {
	if (y > Level->height / 2)	--y;
	else			++y;
	if (y1 > Level->height / 2)	--y1;
	else			++y1;
	straggle_corridor (0, y, Level->width, y1, WATER, RS_COUNTRYSIDE);
    }
}

static void make_mountains (void)
{
    int i, x, y, x1, y1;
    make_general_map ("4...");
    x = 0;
    y = random_range (Level->height);
    x1 = Level->width;
    y1 = random_range (Level->height);
    straggle_corridor (x, y, x1, y1, WATER, RS_COUNTRYSIDE);
    for (i = 0; i < 7; i++) {
	x = random_range (Level->width);
	x1 = random_range (Level->width);
	y = 0;
	y1 = Level->height;
	straggle_corridor (x, y, x1, y1, WATER, RS_COUNTRYSIDE);
    }
}

static void make_swamp (void)
{
    make_general_map ("~~\".");
}

// builds a room. Then, for each successive room, sends off at least one
// corridor which is guaranteed to connect up to another room, thus guaranteeing
// fully connected level.
static void room_level (void)
{
    int fx, fy, tx, ty, t, l, e;

    do {
	t = random_range (Level->height - 10) + 1;
	l = random_range (Level->width - 10) + 1;
	e = 6 + random_range (5);
    } while (Level->site(l,t).roomnumber != RS_WALLSPACE || Level->site(l + e,t).roomnumber != RS_WALLSPACE || Level->site(l,t + e).roomnumber != RS_WALLSPACE || Level->site(l + e,t + e).roomnumber != RS_WALLSPACE);
    char rsi = RS_ROOMBASE + random_range (NUMROOMNAMES);
    if (Level->environment == E_SEWERS && random_range (2))
	rsi = RS_SEWER_CONTROL_ROOM;
    int buildaux = min<int> (UINT8_MAX,20*difficulty());
    build_room (l, t, e, rsi, buildaux+1);

    for (unsigned i = 2, numrooms = random_range(8)+9; i <= numrooms; i++) {
	do {
	    t = random_range (Level->height - 10) + 1;
	    l = random_range (Level->width - 10) + 1;
	    e = 6 + random_range (5);
	} while (Level->site(l,t).roomnumber != RS_WALLSPACE || Level->site(l + e,t).roomnumber != RS_WALLSPACE || Level->site(l,t + e).roomnumber != RS_WALLSPACE || Level->site(l + e,t + e).roomnumber != RS_WALLSPACE);
	rsi = RS_ROOMBASE + random_range (NUMROOMNAMES);
	if (Level->environment == E_SEWERS && random_range (2))
	    rsi = RS_SEWER_CONTROL_ROOM;
	build_room (l, t, e, rsi, buildaux+i);

	// One or two corridors going out
	for (unsigned cri = 0, crn = 1+random_range(2); cri < crn; ++cri) {
	    do {
		findspace (&tx, &ty);
	    } while (Level->site(tx,ty).aux == buildaux+i);
	    // figure out where to start corridor from
	    if (ty <= t && tx <= l + e) {
		fx = l + 1 + random_range (e - 1);
		fy = t;
	    } else if (tx >= l + e && ty <= t + e) {
		fx = l + e;
		fy = t + 1 + random_range (e - 1);
	    } else if (ty >= t + e && tx >= l) {
		fx = l + 1 + random_range (e - 1);
		fy = t + e;
	    } else {
		fx = l;
		fy = t + 1 + random_range (e - 1);
	    }
	    room_corridor (fx, fy, tx, ty, buildaux+i);
	}
    }

    if (Level->environment == E_SEWERS) {
	if (Level->depth == SEWERLEVELS) {
	    findspace (&tx, &ty);
	    make_site_monster (tx, ty, GREAT_WYRM);
	}
    } else if (Level->environment == E_CASTLE) {
	if (Level->depth == CASTLELEVELS) {
	    findspace (&tx, &ty);
	    Level->site(tx,ty).locchar = STAIRS_DOWN;
	    Level->site(tx,ty).p_locf = L_ENTER_COURT;
	}
    } else if (Level->environment == E_VOLCANO) {
	if (Level->depth == VOLCANOLEVELS && !gamestatusp (COMPLETED_VOLCANO)) {
	    findspace (&tx, &ty);
	    make_site_monster (tx, ty, DEMON_EMP);
	}
    }
}

// goes from f to t unless it hits a site which is not a wall and doesn't
// have buildaux field == baux
static void room_corridor (int fx, int fy, int tx, int ty, int baux)
{
    makedoor (fx, fy);

    int dx = sign(tx - fx), dy = sign(ty - fy);
    location* psite;
    for (;;) {
	psite = &Level->site(fx+dx,fy+dy);
	if (psite->locchar != WALL || (fx+dx == tx && fy+dy == ty))
	    break;
	fx += dx; fy += dy;
	psite->locchar = FLOOR;
	psite->roomnumber = RS_CORRIDOR;
	psite->aux = baux;
	dx = sign (tx - fx);
	dy = sign (ty - fy);
	if (dx && dy) {
	    if (random_range(2))
		dx = 0;
	    else if (random_range(2))
		dy = 0;
	}

    }
    if (psite->locchar == WALL)
	makedoor (fx, fy);
}

static void maze_level (void)
{
    int tx, ty;
    char rsi = RS_VOLCANO;
    if (Level->environment == E_ASTRAL) {
	switch (Level->depth) {
	    case 1:
		rsi = RS_EARTHPLANE;
		break;
	    case 2:
		rsi = RS_AIRPLANE;
		break;
	    case 3:
		rsi = RS_WATERPLANE;
		break;
	    case 4:
		rsi = RS_FIREPLANE;
		break;
	    case 5:
		rsi = RS_HIGHASTRAL;
		break;
	}
    }
    maze_corridor (random_range (Level->width - 1) + 1, random_range (Level->height - 1) + 1, random_range (Level->width - 1) + 1, random_range (Level->height - 1) + 1, rsi, 0);
    if (Level->environment == E_ASTRAL) {
	for (unsigned i = 0; i < Level->width; i++) {
	    for (unsigned j = 0; j < Level->height; j++) {
		if (Level->site(i,j).locchar == WALL) {
		    switch (Level->depth) {
			case 1:
			    Level->site(i,j).aux = 255;
			    break;
			case 2:
			    Level->site(i,j).locchar = WHIRLWIND;
			    Level->site(i,j).p_locf = L_WHIRLWIND;
			    break;
			case 3:
			    Level->site(i,j).locchar = WATER;
			    Level->site(i,j).p_locf = L_WATER;
			    break;
			case 4:
			    Level->site(i,j).locchar = FIRE;
			    Level->site(i,j).p_locf = L_FIRE;
			    break;
			case 5:
			    Level->site(i,j).locchar = ABYSS;
			    Level->site(i,j).p_locf = L_ABYSS;
			    break;
		    }
		}
	    }
	}
	int mid = ELEM_MASTER;
	switch (Level->depth) {
	    case 1: mid = LORD_EARTH; break;
	    case 2: mid = LORD_AIR; break;
	    case 3: mid = LORD_WATER; break;
	    case 4: mid = LORD_FIRE; break;
	}
	if (Level->depth == 5) {
	    findspace (&tx, &ty);
	    Level->site(tx,ty).p_locf = L_ENTER_CIRCLE;
	    Level->site(tx,ty).locchar = STAIRS_DOWN;
	}
	if (!gamestatusp (COMPLETED_ASTRAL)) {
	    findspace (&tx, &ty);
	    make_site_monster (tx, ty, mid);
	}
    } else if (Level->environment == E_VOLCANO) {
	if (Level->depth == VOLCANOLEVELS && !gamestatusp (COMPLETED_VOLCANO)) {
	    findspace (&tx, &ty);
	    make_site_monster (tx, ty, DEMON_EMP);
	}
    }
}

// keep drawing corridors recursively for 2^5 endpoints
static void maze_corridor (int fx, int fy, int tx, int ty, int rsi, int num)
{
    if (num < 6) {
	straggle_corridor (fx, fy, tx, ty, FLOOR, rsi);
	maze_corridor (tx, ty, random_range (Level->width - 1) + 1, random_range (Level->height - 1) + 1, rsi, num + 1);
    }
}
