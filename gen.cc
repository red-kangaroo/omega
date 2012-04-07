#include "glob.h"
#include <time.h>

//----------------------------------------------------------------------

static void build_room(int x, int y, int l, char rsi, int baux);
static void build_square_room(int x, int y, int l, char rsi, int baux);
static void corridor_crawl(int *fx, int *fy, int sx, int sy, int n, chtype loc, char rsi);
static void find_stairs(char fromlevel, char tolevel);
static plv findlevel(struct level *dungeon, char levelnum);
static void make_forest(void);
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

//----------------------------------------------------------------------

// Deallocate current dungeon
void free_dungeon (void)
{
    while (Dungeon != NULL) {
	plv tlv = Dungeon;
	Dungeon = Dungeon->next;
	free_level (tlv);
    }
}

// erase the level w/o deallocating it
void clear_level (struct level *dungeon_level)
{
    int i, j;
    if (dungeon_level != NULL) {
	dungeon_level->generated = FALSE;
	dungeon_level->numrooms = 0;
	dungeon_level->tunnelled = 0;
	dungeon_level->depth = 0;
	dungeon_level->mlist = NULL;
	dungeon_level->next = NULL;
	dungeon_level->last_visited = time(NULL);
	for (i = 0; i < MAXWIDTH; i++) {
	    for (j = 0; j < MAXLENGTH; j++) {
		dungeon_level->site[i][j].locchar = WALL;
		dungeon_level->site[i][j].showchar = SPACE;
		dungeon_level->site[i][j].creature = NULL;
		dungeon_level->site[i][j].things = NULL;
		dungeon_level->site[i][j].aux = difficulty() * 20;
		dungeon_level->site[i][j].buildaux = 0;
		dungeon_level->site[i][j].p_locf = L_NO_OP;
		dungeon_level->site[i][j].lstatus = 0;
		dungeon_level->site[i][j].roomnumber = RS_WALLSPACE;
	    }
	}
    }
}

// Looks for level tolevel in current dungeon which is named by
// Dungeon, which may be NULL. If the level is found, and rewrite_level
// is FALSE, and the level has already been generated, nothing happens
// beyond Level being set correctly. Otherwise the level is recreated
// from scratch
void change_level (int fromlevel, int tolevel, int rewrite_level)
{
    struct level *thislevel = NULL;
    Player.sx = -1;
    Player.sy = -1;		// sanctuary effect dispelled
    thislevel = findlevel (Dungeon, tolevel);
    deepest[Current_Environment] = max (deepest[Current_Environment], tolevel);
    if (thislevel == NULL) {
	thislevel = new level;
	clear_level (thislevel);
	Level = thislevel;
	Level->next = Dungeon;
	Dungeon = Level;
    }
    Level = thislevel;
    if ((!Level->generated) || rewrite_level) {
	initrand (Current_Environment, tolevel);
	Level->environment = Current_Environment;
	Level->depth = tolevel;
	Level->generated = TRUE;
	switch (Current_Environment) {
	    case E_CAVES:
		if ((random_range (4) == 0) && (tolevel < MaxDungeonLevels))
		    room_level();
		else
		    cavern_level();
		break;
	    case E_SEWERS:
		if ((random_range (4) == 0) && (tolevel < MaxDungeonLevels))
		    room_level();
		else
		    sewer_level();
		break;
	    case E_CASTLE:
		room_level();
		break;
	    case E_ASTRAL:
		maze_level();
		break;
	    case E_VOLCANO:
		switch (random_range (3)) {
		    case 0:
			cavern_level();
			break;
		    case 1:
			room_level();
			break;
		    case 2:
			maze_level();
			break;
		}
		break;
	    default:
		print3 ("This dungeon not implemented!");
		break;
	}
	install_traps();
	install_specials();
	make_stairs (fromlevel);
	make_stairs (fromlevel);
	initrand (E_RESTORE, 0);
	populate_level (Current_Environment);
	stock_level();
    }
    find_stairs (fromlevel, tolevel);
    ScreenOffset = Player.y - (ScreenLength / 2);
    show_screen();
    screencheck (Player.y);
    drawvision (Player.x, Player.y);
    // synchronize with player on level change
    Player.click = (Tick + 1) % 60;
    roomcheck();
}

// tries to find the level of depth levelnum in dungeon; if can't find it returns NULL
static plv findlevel (struct level* dungeon, char levelnum)
{
    if (dungeon == NULL)
	return (NULL);
    else {
	while ((dungeon->next != NULL) && (dungeon->depth != levelnum))
	    dungeon = dungeon->next;
	if (dungeon->depth == levelnum) {
	    dungeon->last_visited = time(NULL);
	    return (dungeon);
	} else
	    return (NULL);
    }
}

// keep going in one orthogonal direction or another until we hit our destination
static void straggle_corridor (int fx, int fy, int tx, int ty, chtype loc, char rsi)
{
    int dx, dy;
    while ((fx != tx) || (fy != ty)) {
	dx = tx - fx;
	dy = ty - fy;
	if (random_range (absv (dx) + absv (dy)) < absv (dx))
	    corridor_crawl (&fx, &fy, sign (dx), 0, random_range (absv (dx)) + 1, loc, rsi);
	else
	    corridor_crawl (&fx, &fy, 0, sign (dy), random_range (absv (dy)) + 1, loc, rsi);
    }
}

static void makedoor (int x, int y)
{
    if (random_range (20) <= Level->depth / 10) {
	Level->site[x][y].locchar = FLOOR;
	lset (x, y, SECRET);
    } else if (random_range (20) <= Level->depth / 2) {
	Level->site[x][y].locchar = CLOSED_DOOR;
	if (random_range (20) <= Level->depth / 10)
	    lset (x, y, SECRET);
	if (random_range (40) <= Level->depth)
	    Level->site[x][y].aux = LOCKED;
	else
	    Level->site[x][y].aux = UNLOCKED;
    } else {
	Level->site[x][y].locchar = OPEN_DOOR;
	Level->site[x][y].aux = UNLOCKED;
    }
    if (!loc_statusp (x, y, SECRET)) {
	lset (x, y + 1, STOPS);
	lset (x + 1, y, STOPS);
	lset (x - 1, y, STOPS);
	lset (x, y - 1, STOPS);
	lset (x, y, STOPS);
    }
    Level->site[x][y].p_locf = L_NO_OP;
    // prevents water corridors from being instant death in sewers
}

static void corridor_crawl (int* fx, int* fy, int sx, int sy, int n, chtype loc, char rsi)
{
    int i;
    for (i = 0; i < n; i++) {
	*fx += sx;
	*fy += sy;
	if ((*fx < WIDTH) && (*fx > -1) && (*fy > -1) && (*fy < LENGTH)) {
	    Level->site[*fx][*fy].locchar = loc;
	    if (Level->site[*fx][*fy].roomnumber == RS_WALLSPACE)
		Level->site[*fx][*fy].roomnumber = rsi;
	    if (loc == WATER)
		Level->site[*fx][*fy].p_locf = L_WATER;
	    else if (loc == FLOOR)
		Level->site[*fx][*fy].p_locf = L_NO_OP;
	    else if (loc == RUBBLE)
		Level->site[*fx][*fy].p_locf = L_RUBBLE;
	}
    }
}

const char* roomname (int ri)
{
    switch (ri) {
	case RS_ZORCH:
	    strcpy (Str4, "A place zorched by powerful magic.");
	    break;
	case RS_COURT:
	    strcpy (Str4, "The Court of the ArchMage.");
	    break;
	case RS_CIRCLE:
	    strcpy (Str4, "The Astral Demesne of the Circle of Sorcerors");
	    break;
	case RS_MAGIC_ISLE:
	    strcpy (Str4, "An island positively reeking of magic");
	    break;
	case RS_STARPEAK:
	    strcpy (Str4, "Near the oddly glowing peak of a mountain");
	    break;
	case RS_VOLCANO:
	    strcpy (Str4, "Deep within the bowels of the earth");
	    break;
	case RS_HIGHASTRAL:
	    strcpy (Str4, "The High Astral Plane");
	    break;
	case RS_EARTHPLANE:
	    strcpy (Str4, "The Plane of Earth");
	    break;
	case RS_WATERPLANE:
	    strcpy (Str4, "The Plane of Water");
	    break;
	case RS_FIREPLANE:
	    strcpy (Str4, "The Plane of Fire");
	    break;
	case RS_AIRPLANE:
	    strcpy (Str4, "The Plane of Air");
	    break;
	case RS_KITCHEN:
	    strcpy (Str4, "A kitchen");
	    break;
	case RS_BATHROOM:
	    strcpy (Str4, "A bathroom");
	    break;
	case RS_BEDROOM:
	    strcpy (Str4, "A bedroom");
	    break;
	case RS_DININGROOM:
	    strcpy (Str4, "A dining room");
	    break;
	case RS_SECRETPASSAGE:
	    strcpy (Str4, "A secret passage");
	    break;
	case RS_CLOSET:
	    strcpy (Str4, "A stuffy closet");
	    break;
	case RS_ARENA:
	    strcpy (Str4, "The Rampart Arena");
	    break;
	case RS_DROWNED_SEWER:
	    strcpy (Str4, "A water-filled sewer node");
	    break;
	case RS_DRAINED_SEWER:
	    strcpy (Str4, "An unused sewer node");
	    break;
	case RS_SEWER_DUCT:
	    strcpy (Str4, "A winding sewer duct");
	    break;
	case RS_DESTINY:
	    strcpy (Str4, "The Halls of Fate");
	    break;
	case RS_DRUID:
	    strcpy (Str4, "The Great Henge");
	    break;
	case RS_HECATE:
	    strcpy (Str4, "The Church of the Far Side");
	    break;
	case RS_SET:
	    strcpy (Str4, "The Temple of the Black Hand");
	    break;
	case RS_ATHENA:
	    strcpy (Str4, "The Parthenon");
	    break;
	case RS_ODIN:
	    strcpy (Str4, "The Shrine of the Noose");
	    break;
	case RS_ADEPT:
	    strcpy (Str4, "The Adept's Challenge");
	    break;
	case RS_WYRM:
	    strcpy (Str4, "The Sunken Cavern of the Great Wyrm.");
	    break;
	case RS_OCEAN:
	    strcpy (Str4, "The Underground Ocean.");
	    break;
	case RS_PONDS:
	    strcpy (Str4, "A series of subterranean pools and streams.");
	    break;
	case RS_DRAGONLORD:
	    strcpy (Str4, "The Lair of the DragonLord.");
	    break;
	case RS_GOBLINKING:
	    strcpy (Str4, "The Caves of the Goblins.");
	    break;
	case RS_CAVERN:
	    strcpy (Str4, "A vast natural cavern.");
	    break;
	case RS_CORRIDOR:
	    strcpy (Str4, "A dimly lit corridor.");
	    break;
	case RS_WALLSPACE:
	    strcpy (Str4, "A niche hollowed out of the wall.");
	    break;

	case RS_GARDEROBE:
	    strcpy (Str4, "An abandoned garderobe.");
	    break;
	case RS_DUNGEON_CELL:
	    strcpy (Str4, "A dungeon cell.");
	    break;
	case RS_TILED_CHAMBER:
	    strcpy (Str4, "A tiled chamber.");
	    break;
	case RS_CRYSTAL_CAVERN:
	    strcpy (Str4, "A crystal cavern.");
	    break;
	case RS_MASTER_BEDROOM:
	    strcpy (Str4, "Someone's bedroom.");
	    break;
	case RS_STOREROOM:
	    strcpy (Str4, "An old storeroom.");
	    break;
	case RS_CHARRED_ROOM:
	    strcpy (Str4, "A room with charred walls.");
	    break;
	case RS_MARBLE_HALL:
	    strcpy (Str4, "A marble hall.");
	    break;
	case RS_EERIE_CAVE:
	    strcpy (Str4, "An eerie cave.");
	    break;
	case RS_TREASURE_CHAMBER:
	    strcpy (Str4, "A ransacked treasure-chamber.");
	    break;
	case RS_SMOKED_ROOM:
	    strcpy (Str4, "A smoke-filled room.");
	    break;
	case RS_APARTMENT:
	    strcpy (Str4, "A well-appointed apartment.");
	    break;
	case RS_ANTECHAMBER:
	    strcpy (Str4, "An antechamber.");
	    break;
	case RS_HAREM:
	    strcpy (Str4, "An unoccupied harem.");
	    break;
	case RS_MULTIPURPOSE:
	    strcpy (Str4, "A multi-purpose room.");
	    break;
	case RS_STALACTITES:
	    strcpy (Str4, "A room filled with stalactites.");
	    break;
	case RS_GREENHOUSE:
	    strcpy (Str4, "An underground greenhouse.");
	    break;
	case RS_WATERCLOSET:
	    strcpy (Str4, "A water closet.");
	    break;
	case RS_STUDY:
	    strcpy (Str4, "A study.");
	    break;
	case RS_LIVING_ROOM:
	    strcpy (Str4, "A living room.");
	    break;
	case RS_DEN:
	    strcpy (Str4, "A comfortable den.");
	    break;
	case RS_ABATOIR:
	    strcpy (Str4, "An abatoir.");
	    break;
	case RS_BOUDOIR:
	    strcpy (Str4, "A boudoir.");
	    break;
	case RS_STAR_CHAMBER:
	    strcpy (Str4, "A star chamber.");
	    break;
	case RS_MANMADE_CAVERN:
	    strcpy (Str4, "A manmade cavern.");
	    break;
	case RS_SEWER_CONTROL_ROOM:
	    strcpy (Str4, "A sewer control room");
	    break;
	case RS_HIGH_MAGIC_SHRINE:
	    strcpy (Str4, "A shrine to High Magic");
	    break;
	case RS_MAGIC_LABORATORY:
	    strcpy (Str4, "A magic laboratory");
	    break;
	case RS_PENTAGRAM_ROOM:
	    strcpy (Str4, "A room with inscribed pentagram");
	    break;
	case RS_OMEGA_ROOM:
	    strcpy (Str4, "A chamber with a blue crystal omega dais");
	    break;
	default:
	    strcpy (Str4, "A room of mystery and allure.");
	    break;
    }
    return (Str4);
}

// puts the player on the first set of stairs from the apt level
// if can't find them, just drops player anywhere....
static void find_stairs (char fromlevel, char tolevel)
{
    int i, j, found = FALSE;
    chtype sitechar;
    if (fromlevel > tolevel)
	sitechar = STAIRS_DOWN;
    else
	sitechar = STAIRS_UP;
    for (i = 0; i < WIDTH; i++)
	for (j = 0; j < LENGTH; j++)
	    if ((Level->site[i][j].locchar == sitechar) && (!found)) {
		found = TRUE;
		Player.x = i;
		Player.y = j;
		break;
	    }
    if (!found) {
	findspace (&Player.x, &Player.y, -1);
	if (Level->environment != E_ASTRAL) {
	    Level->site[Player.x][Player.y].locchar = sitechar;
	    lset (Player.x, Player.y, CHANGED);
	}
    }
}

void install_traps (void)
{
    int i, j;
    for (i = 0; i < WIDTH; i++)
	for (j = 0; j < LENGTH; j++)
	    if ((Level->site[i][j].locchar == FLOOR) && (Level->site[i][j].p_locf == L_NO_OP) && random_range (500) <= ((int) (Level->depth / 6)))
		Level->site[i][j].p_locf = TRAP_BASE + random_range (NUMTRAPS);
}

// x, y, is top left corner, l is length of side, rsi is room string index
// baux is so all rooms will have a key field.
static void build_square_room (int x, int y, int l, char rsi, int baux)
{
    int i, j;

    for (i = x; i <= x + l; i++)
	for (j = y; j <= y + l; j++) {
	    Level->site[i][j].roomnumber = rsi;
	    Level->site[i][j].buildaux = baux;
	}
    for (i = x + 1; i < x + l; i++)
	for (j = y + 1; j < y + l; j++) {
	    Level->site[i][j].locchar = FLOOR;
	    Level->site[i][j].p_locf = L_NO_OP;
	}
}

static void build_room (int x, int y, int l, char rsi, int baux)
{
    build_square_room (x, y, l, rsi, baux);
}

void cavern_level (void)
{
    int i, fx, fy, tx, ty, t, l, e;
    char rsi;

    Level->numrooms = 1;

    if ((Current_Dungeon == E_CAVES) && (Level->depth == CAVELEVELS))
	rsi = RS_GOBLINKING;
    else
	rsi = RS_CAVERN;
    t = random_range (LENGTH / 2);
    l = random_range (WIDTH / 2);
    e = random_range (WIDTH / 8) + WIDTH / 8;
    build_square_room (t, l, e, rsi, 0);

    for (i = 0; i < 16; i++) {
	findspace (&tx, &ty, -1);
	fx = random_range (WIDTH - 2) + 1;
	fy = random_range (LENGTH - 2) + 1;
	straggle_corridor (fx, fy, tx, ty, FLOOR, RS_CORRIDOR);
    }
    while (random_range (3) == 1) {
	findspace (&tx, &ty, -1);
	fx = random_range (WIDTH - 2) + 1;
	fy = random_range (LENGTH - 2) + 1;
	straggle_corridor (fx, fy, tx, ty, WATER, RS_PONDS);
    }
    if (Current_Dungeon == E_CAVES) {
	if ((Level->depth == CAVELEVELS) && (!gamestatusp (COMPLETED_CAVES))) {
	    findspace (&tx, &ty, -1);
	    Level->mlist = new monsterlist;
	    Level->mlist->next = NULL;
	    Level->mlist->m = Level->site[tx][ty].creature = ((pmt) make_creature (GOBLIN_KING));	// goblin king
	    Level->mlist->m->x = tx;
	    Level->mlist->m->y = ty;
	}
    } else if (Current_Environment == E_VOLCANO) {
	if (Level->depth == VOLCANOLEVELS) {
	    findspace (&tx, &ty, -1);
	    Level->mlist = new monsterlist;
	    Level->mlist->next = NULL;
	    Level->mlist->m = Level->site[tx][ty].creature = ((pmt) make_creature (DEMON_EMP));	// The dark emp
	    Level->mlist->m->x = tx;
	    Level->mlist->m->y = ty;
	}
    }
}

void sewer_level (void)
{
    int i, tx, ty, t, l, e;
    char rsi;
    chtype lchar;

    Level->numrooms = random_range (3) + 3;
    rsi = RS_DRAINED_SEWER;
    for (i = 0; i < Level->numrooms; i++) {
	do {
	    t = random_range (LENGTH - 10) + 1;
	    l = random_range (WIDTH - 10) + 1;
	    e = 4;
	} while ((Level->site[l][t].roomnumber == rsi) || (Level->site[l + e][t].roomnumber == rsi) || (Level->site[l][t + e].roomnumber == rsi) || (Level->site[l + e][t + e].roomnumber == rsi));
	if (random_range (5)) {
	    lchar = FLOOR;
	    rsi = RS_DRAINED_SEWER;
	} else {
	    lchar = WATER;
	    rsi = RS_DROWNED_SEWER;
	}
	build_room (l, t, e, rsi, i);
	sewer_corridor (l, t, -1, -1, lchar);
	sewer_corridor (l + e, t, 1, -1, lchar);
	sewer_corridor (l, t + e, -1, 1, lchar);
	sewer_corridor (l + e, t + e, 1, 1, lchar);
    }
    if (Current_Dungeon == E_SEWERS) {
	if ((Level->depth == SEWERLEVELS) && (!gamestatusp (COMPLETED_SEWERS))) {
	    findspace (&tx, &ty, -1);
	    Level->mlist = new monsterlist;
	    Level->mlist->next = NULL;
	    Level->mlist->m = Level->site[tx][ty].creature = ((pmt) make_creature (GREAT_WYRM));	// The Great Wyrm
	    Level->mlist->m->x = tx;
	    Level->mlist->m->y = ty;
	}
    }
}

static void sewer_corridor (int x, int y, int dx, int dy, chtype locchar)
{
    int continuing = TRUE;
    makedoor (x, y);
    x += dx;
    y += dy;
    while (continuing) {
	Level->site[x][y].locchar = locchar;
	if (locchar == WATER)
	    Level->site[x][y].p_locf = L_WATER;
	else
	    Level->site[x][y].p_locf = L_NO_OP;
	Level->site[x][y].roomnumber = RS_SEWER_DUCT;
	x += dx;
	y += dy;
	if (locchar == WATER)
	    continuing = (inbounds (x, y) && ((Level->site[x][y].locchar == WALL) || (Level->site[x][y].locchar == WATER)));
	else
	    continuing = (inbounds (x, y) && ((Level->site[x][y].roomnumber == RS_WALLSPACE) || (Level->site[x][y].roomnumber == RS_SEWER_DUCT)));
    }
    if (inbounds (x, y))
	makedoor (x, y);
}

void install_specials (void)
{
    int i, j, x, y;

    for (x = 0; x < WIDTH; x++) {
	for (y = 0; y < LENGTH; y++) {
	    if ((Level->site[x][y].locchar == FLOOR) && (Level->site[x][y].p_locf == L_NO_OP) && (random_range (300) < difficulty())) {
		i = random_range (100);
		if (i < 10) {
		    Level->site[x][y].locchar = ALTAR;
		    Level->site[x][y].p_locf = L_ALTAR;
		    Level->site[x][y].aux = random_range (10);
		} else if (i < 20) {
		    Level->site[x][y].locchar = WATER;
		    Level->site[x][y].p_locf = L_MAGIC_POOL;
		} else if (i < 35) {
		    Level->site[x][y].locchar = RUBBLE;
		    Level->site[x][y].p_locf = L_RUBBLE;
		} else if (i < 40) {
		    Level->site[x][y].locchar = LAVA;
		    Level->site[x][y].p_locf = L_LAVA;
		} else if (i < 45) {
		    Level->site[x][y].locchar = FIRE;
		    Level->site[x][y].p_locf = L_FIRE;
		} else if ((i < 50) && (Current_Environment != E_ASTRAL)) {
		    Level->site[x][y].locchar = LIFT;
		    Level->site[x][y].p_locf = L_LIFT;
		} else if ((i < 55) && (Current_Environment != E_VOLCANO)) {
		    Level->site[x][y].locchar = HEDGE;
		    Level->site[x][y].p_locf = L_HEDGE;
		} else if (i < 57) {
		    Level->site[x][y].locchar = HEDGE;
		    Level->site[x][y].p_locf = L_TRIFID;
		} else if (i < 70) {
		    Level->site[x][y].locchar = STATUE;
		    if (random_range (100) < difficulty())
			for (j = 0; j < 8; j++) {
			    if (Level->site[x + Dirs[0][j]][y + Dirs[1][j]].p_locf != L_NO_OP)
				Level->site[x + Dirs[0][j]][y + Dirs[1][j]].locchar = FLOOR;
			    Level->site[x + Dirs[0][j]][y + Dirs[1][j]].p_locf = L_STATUE_WAKE;
			}
		} else {
		    if (Current_Environment == E_VOLCANO) {
			Level->site[x][y].locchar = LAVA;
			Level->site[x][y].p_locf = L_LAVA;
		    } else if (Current_Environment == E_ASTRAL) {
			if (Level->depth == 1) {
			    Level->site[x][y].locchar = RUBBLE;
			    Level->site[x][y].p_locf = L_RUBBLE;
			} else if (Level->depth == 2) {
			    Level->site[x][y].locchar = FIRE;
			    Level->site[x][y].p_locf = L_FIRE;
			} else if (Level->depth == 3) {
			    Level->site[x][y].locchar = WATER;
			    Level->site[x][y].p_locf = L_WATER;
			} else if (Level->depth == 4) {
			    Level->site[x][y].locchar = ABYSS;
			    Level->site[x][y].p_locf = L_ABYSS;
			}
		    } else {
			Level->site[x][y].locchar = WATER;
			Level->site[x][y].p_locf = L_WATER;
		    }
		}
	    }
	}
    }
}

// For each level, there should be one stairway going up and one down. 
// fromlevel determines whether the player is placed on the up or the down
// staircase. The aux value is currently unused elsewhere, but is set 
// to the destination level.
void make_stairs (int fromlevel)
{
    int i, j;
    // no stairway out of astral
    if (Current_Environment != E_ASTRAL) {
	findspace (&i, &j, -1);
	Level->site[i][j].locchar = STAIRS_UP;
	Level->site[i][j].aux = Level->depth - 1;
	lset (i, j, STOPS);
	if (fromlevel >= 0 && fromlevel < Level->depth) {
	    Player.x = i;
	    Player.y = j;
	}
    }
    if (Level->depth < MaxDungeonLevels) {
	findspace (&i, &j, -1);
	Level->site[i][j].locchar = STAIRS_DOWN;
	Level->site[i][j].aux = Level->depth + 1;
	lset (i, j, STOPS);
	if (fromlevel > Level->depth) {
	    Player.x = i;
	    Player.y = j;
	}
    }
}

// tactical map generating functions
void make_country_screen (int terrain)
{
    int i, j;
    TempLevel = Level;
    if (ok_to_free (TempLevel)) {
	free_level (TempLevel);
	TempLevel = NULL;
    }
    Level = new level;
    clear_level (Level);
    Level->environment = E_TACTICAL_MAP;
    Level->generated = TRUE;
    switch (terrain) {
	case FOREST:
	    make_forest();
	    break;
	case JUNGLE:
	    make_jungle();
	    break;
	case SWAMP:
	    make_swamp();
	    break;
	case RIVER:
	    make_river();
	    break;
	case MOUNTAINS:
	case PASS:
	    make_mountains();
	    break;
	case ROAD:
	    make_road();
	    break;
	default:
	    make_plains();
	    break;
    }
    if (nighttime()) {
	print3 ("Night's gloom shrouds your sight.");
	for (i = 0; i < WIDTH; i++)
	    for (j = 0; j < LENGTH; j++) {
		Level->site[i][j].showchar = SPACE;
		Level->site[i][j].lstatus = 0;
	    }
    }
}

static void make_general_map (const char* terrain)
{
    int i, j;
    int size = strlen (terrain);
    char curr;

    for (i = 0; i < WIDTH; i++)
	for (j = 0; j < LENGTH; j++) {
	    if ((i == 0 && j == 0) || !random_range (5))
		curr = terrain[random_range (size)];
	    else if (j == 0 || (random_range (2) && i > 0))
		curr = Level->site[i - 1][j].locchar & 0xff;
	    else
		curr = Level->site[i][j - 1].locchar & 0xff;
	    switch (curr) {
		case (FLOOR & 0xff):
		    Level->site[i][j].locchar = Level->site[i][j].showchar = FLOOR;
		    Level->site[i][j].p_locf = L_NO_OP;
		    break;
		case (HEDGE & 0xff):
		    Level->site[i][j].locchar = Level->site[i][j].showchar = HEDGE;
		    Level->site[i][j].p_locf = L_HEDGE;
		    break;
		case (WATER & 0xff):
		    Level->site[i][j].locchar = Level->site[i][j].showchar = WATER;
		    Level->site[i][j].p_locf = L_WATER;
		    break;
		case (RUBBLE & 0xff):
		    Level->site[i][j].locchar = Level->site[i][j].showchar = RUBBLE;
		    Level->site[i][j].p_locf = L_RUBBLE;
		    break;
	    }
	    Level->site[i][j].lstatus = SEEN + LIT;
	    Level->site[i][j].roomnumber = RS_COUNTRYSIDE;
	    if ((i == 0) || (j == 0) || (i == WIDTH - 1) || (j == LENGTH - 1))
		Level->site[i][j].p_locf = L_TACTICAL_EXIT;
	}
}

static void make_plains (void)
{
    make_general_map (".");
}

static void make_road (void)
{
    int x, y;
    make_general_map ("\"\"~4....");
    for (x = WIDTH / 2 - 3; x <= WIDTH / 2 + 3; x++) {
	for (y = 0; y < LENGTH; y++) {
	    Level->site[x][y].locchar = Level->site[x][y].showchar = FLOOR;
	    if (y != 0 && y != LENGTH - 1)
		Level->site[x][y].p_locf = L_NO_OP;
	}
    }
}

static void make_forest (void)
{
    make_general_map ("\".");
    straggle_corridor (0, random_range (LENGTH), WIDTH, random_range (LENGTH), WATER, RS_COUNTRYSIDE);
}

static void make_jungle (void)
{
    make_general_map ("\"\".");
}

static void make_river (void)
{
    int i, y, y1;
    make_general_map ("\".......");
    y = random_range (LENGTH);
    y1 = random_range (LENGTH);
    straggle_corridor (0, y, WIDTH, y1, WATER, RS_COUNTRYSIDE);
    for (i = 0; i < 7; i++) {
	if (y > LENGTH / 2)
	    y--;
	else
	    y++;
	if (y1 > LENGTH / 2)
	    y1--;
	else
	    y1++;
	straggle_corridor (0, y, WIDTH, y1, WATER, RS_COUNTRYSIDE);
    }
}

static void make_mountains (void)
{
    int i, x, y, x1, y1;
    make_general_map ("4...");
    x = 0;
    y = random_range (LENGTH);
    x1 = WIDTH;
    y1 = random_range (LENGTH);
    straggle_corridor (x, y, x1, y1, WATER, RS_COUNTRYSIDE);
    for (i = 0; i < 7; i++) {
	x = random_range (WIDTH);
	x1 = random_range (WIDTH);
	y = 0;
	y1 = LENGTH;
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
void room_level (void)
{
    int i, fx, fy, tx, ty, t, l, e;

    Level->numrooms = random_range (8) + 9;

    do {
	t = random_range (LENGTH - 10) + 1;
	l = random_range (WIDTH - 10) + 1;
	e = 4 + random_range (5);
    } while ((Level->site[l][t].roomnumber != RS_WALLSPACE) || (Level->site[l + e][t].roomnumber != RS_WALLSPACE) || (Level->site[l][t + e].roomnumber != RS_WALLSPACE) || (Level->site[l + e][t + e].roomnumber != RS_WALLSPACE));
    char rsi = RS_ROOMBASE + random_range (NUMROOMNAMES);
    if (Current_Dungeon == E_SEWERS && random_range (2))
	rsi = RS_SEWER_CONTROL_ROOM;
    build_room (l, t, e, rsi, 1);

    for (i = 2; i <= Level->numrooms; i++) {
	do {
	    t = random_range (LENGTH - 10) + 1;
	    l = random_range (WIDTH - 10) + 1;
	    e = 4 + random_range (5);
	} while ((Level->site[l][t].roomnumber != RS_WALLSPACE) || (Level->site[l + e][t].roomnumber != RS_WALLSPACE) || (Level->site[l][t + e].roomnumber != RS_WALLSPACE) || (Level->site[l + e][t + e].roomnumber != RS_WALLSPACE));
	rsi = RS_ROOMBASE + random_range (NUMROOMNAMES);
	if (Current_Dungeon == E_SEWERS && random_range (2))
	    rsi = RS_SEWER_CONTROL_ROOM;
	build_room (l, t, e, rsi, i);

	// corridor which is guaranteed to connect
	findspace (&tx, &ty, i);

	// figure out where to start corridor from
	if ((ty <= t) && (tx <= l + e)) {
	    fx = l + 1 + random_range (e - 1);
	    fy = t;
	} else if ((tx >= l + e) && (ty <= t + e)) {
	    fx = l + e;
	    fy = t + 1 + random_range (e - 1);
	} else if ((ty >= t + e) && (tx >= l)) {
	    fx = l + 1 + random_range (e - 1);
	    fy = t + e;
	} else {
	    fx = l;
	    fy = t + 1 + random_range (e - 1);
	}

	room_corridor (fx, fy, tx, ty, i);

	// corridor which may not go anywhere
	if (random_range (2)) {
	    findspace (&tx, &ty, i);
	    if ((ty <= t) && (tx <= l + e)) {
		fx = l + 1 + random_range (e - 1);
		fy = t;
	    } else if ((tx >= l + e) && (ty <= t + e)) {
		fx = l + e;
		fy = t + 1 + random_range (e - 1);
	    } else if ((ty >= t + e) && (tx >= l)) {
		fx = l + 1 + random_range (e - 1);
		fy = t + e;
	    } else {
		fx = l;
		fy = t + 1 + random_range (e - 1);
	    }
	    room_corridor (fx, fy, tx, ty, i);
	}
    }

    if (Current_Dungeon == E_SEWERS) {
	if (Level->depth == SEWERLEVELS) {
	    findspace (&tx, &ty, -1);
	    Level->mlist = new monsterlist;
	    Level->mlist->next = NULL;
	    Level->mlist->m = Level->site[tx][ty].creature = ((pmt) make_creature (GREAT_WYRM));	// The Great Wyrm
	    Level->mlist->m->x = tx;
	    Level->mlist->m->y = ty;
	}
    } else if (Current_Environment == E_CASTLE) {
	if (Level->depth == CASTLELEVELS) {
	    findspace (&tx, &ty, -1);
	    Level->site[tx][ty].locchar = STAIRS_DOWN;
	    Level->site[tx][ty].p_locf = L_ENTER_COURT;
	}
    } else if (Current_Environment == E_VOLCANO) {
	if (Level->depth == VOLCANOLEVELS && !gamestatusp (COMPLETED_VOLCANO)) {
	    findspace (&tx, &ty, -1);
	    Level->mlist = new monsterlist;
	    Level->mlist->next = NULL;
	    Level->mlist->m = Level->site[tx][ty].creature = ((pmt) make_creature (DEMON_EMP));	// The demon emp
	    Level->mlist->m->x = tx;
	    Level->mlist->m->y = ty;
	}
    }
}

// goes from f to t unless it hits a site which is not a wall and doesn't
// have buildaux field == baux
static void room_corridor (int fx, int fy, int tx, int ty, int baux)
{
    int dx, dy, continuing = TRUE;

    dx = sign (tx - fx);
    dy = sign (ty - fy);

    makedoor (fx, fy);

    fx += dx;
    fy += dy;

    while (continuing) {
	Level->site[fx][fy].locchar = FLOOR;
	Level->site[fx][fy].roomnumber = RS_CORRIDOR;
	Level->site[fx][fy].buildaux = baux;
	dx = sign (tx - fx);
	dy = sign (ty - fy);
	if ((dx != 0) && (dy != 0)) {
	    if (random_range (2))
		dx = 0;
	    else if (random_range (2))
		dy = 0;
	}
	fx += dx;
	fy += dy;
	continuing = (((fx != tx) || (fy != ty)) && ((Level->site[fx][fy].buildaux == 0) || (Level->site[fx][fy].buildaux == baux)));
    }
    makedoor (fx, fy);
}

void maze_level (void)
{
    int i, j, tx, ty;
    char rsi = RS_VOLCANO;
    if (Current_Environment == E_ASTRAL)
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
    maze_corridor (random_range (WIDTH - 1) + 1, random_range (LENGTH - 1) + 1, random_range (WIDTH - 1) + 1, random_range (LENGTH - 1) + 1, rsi, 0);
    if (Current_Dungeon == E_ASTRAL) {
	for (i = 0; i < WIDTH; i++)
	    for (j = 0; j < LENGTH; j++)
		if (Level->site[i][j].locchar == WALL)
		    switch (Level->depth) {
			case 1:
			    Level->site[i][j].aux = 500;
			    break;
			case 2:
			    Level->site[i][j].locchar = WHIRLWIND;
			    Level->site[i][j].p_locf = L_WHIRLWIND;
			    break;
			case 3:
			    Level->site[i][j].locchar = WATER;
			    Level->site[i][j].p_locf = L_WATER;
			    break;
			case 4:
			    Level->site[i][j].locchar = FIRE;
			    Level->site[i][j].p_locf = L_FIRE;
			    break;
			case 5:
			    Level->site[i][j].locchar = ABYSS;
			    Level->site[i][j].p_locf = L_ABYSS;
			    break;
		    }
	int mid = ELEM_MASTER;
	switch (Level->depth) {
	    case 1:
		mid = LORD_EARTH;
		break;
	    case 2:
		mid = LORD_AIR;
		break;
	    case 3:
		mid = LORD_WATER;
		break;
	    case 4:
		mid = LORD_FIRE;
		break;
	}
	if (Level->depth == 5) {
	    findspace (&tx, &ty, -1);
	    Level->site[tx][ty].p_locf = L_ENTER_CIRCLE;
	    Level->site[tx][ty].locchar = STAIRS_DOWN;
	}
	if (!gamestatusp (COMPLETED_ASTRAL)) {
	    findspace (&tx, &ty, -1);
	    Level->mlist = new monsterlist;
	    Level->mlist->next = NULL;
	    Level->mlist->m = Level->site[tx][ty].creature = ((pmt) make_creature (mid));
	    Level->mlist->m->x = tx;
	    Level->mlist->m->y = ty;
	}
    } else if (Current_Environment == E_VOLCANO) {
	if (Level->depth == VOLCANOLEVELS && !gamestatusp (COMPLETED_VOLCANO)) {
	    findspace (&tx, &ty, -1);
	    Level->mlist = new monsterlist;
	    Level->mlist->next = NULL;
	    Level->mlist->m = Level->site[tx][ty].creature = ((pmt) make_creature (DEMON_EMP));	// The demon emp
	    Level->mlist->m->x = tx;
	    Level->mlist->m->y = ty;
	}
    }
}

// keep drawing corridors recursively for 2^5 endpoints
static void maze_corridor (int fx, int fy, int tx, int ty, int rsi, int num)
{
    if (num < 6) {
	straggle_corridor (fx, fy, tx, ty, FLOOR, rsi);
	maze_corridor (tx, ty, random_range (WIDTH - 1) + 1, random_range (LENGTH - 1) + 1, rsi, num + 1);

    }
}
