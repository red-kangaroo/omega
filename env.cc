#include "glob.h"
#include <unistd.h>

//----------------------------------------------------------------------

static void make_prime(int i, int j);
static void make_archmage(int i, int j);
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
static void make_guard(int i, int j);
static void make_horse(int i, int j);
static void make_merchant(int i, int j);
static void make_sheep(int i, int j);
static void special_village_site(int i, int j, int villagenum);
static void buyfromstock(int base, int numitems);
static void cureforpay(void);
static void gymtrain(int *maxstat, int *stat);
static void healforpay(void);
static void wake_statue(int x, int y, int first);
static void make_site_treasure(int i, int j, int itemlevel);
static void make_specific_treasure(int i, int j, int iid);
static void repair_jail(void);

//----------------------------------------------------------------------


// loads the arena level into Level
void load_arena (void)
{
    int i, j;
    char site;
    FILE *fd;

    pob openerBox = (pob) checkmalloc (sizeof (objtype));
    *openerBox = Objects[THING_DOOR_OPENER];

    TempLevel = Level;
    if (ok_to_free (TempLevel)) {
	free_level (TempLevel);
	TempLevel = NULL;
    }
    Level = ((plv) checkmalloc (sizeof (levtype)));
    clear_level (Level);
    Level->environment = E_ARENA;
    strcpy (Str3, Omegalib);
    strcat (Str3, "arena.dat");
    fd = checkfopen (Str3, "rb");
    site = cryptkey ("arena.dat");
    for (j = 0; j < LENGTH; j++) {
	for (i = 0; i < WIDTH; i++) {
	    Level->site[i][j].lstatus = SEEN + LIT;
	    Level->site[i][j].roomnumber = RS_ARENA;
	    site = getc (fd) ^ site;
	    Level->site[i][j].p_locf = L_NO_OP;
	    switch (site) {
		case 'P':
		    Level->site[i][j].locchar = PORTCULLIS;
		    Level->site[i][j].p_locf = L_PORTCULLIS;
		    break;
		case 'X':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_ARENA_EXIT;
		    break;
		case '#':
		    Level->site[i][j].locchar = WALL;
		    break;
		case '.':
		    Level->site[i][j].locchar = FLOOR;
		    break;
	    }
	    Level->site[i][j].showchar = Level->site[i][j].locchar;
	}
	site = getc (fd) ^ site;
    }
    fclose (fd);
    Level->site[60][7].creature = Arena_Monster;
    Arena_Monster->x = 60;
    Arena_Monster->y = 7;
    Arena_Monster->sense = 50;
    m_pickup (Arena_Monster, openerBox);
    m_status_set (Arena_Monster, AWAKE);
    Level->mlist = (pml) checkmalloc (sizeof (mltype));
    Level->mlist->m = Arena_Monster;
    Level->mlist->next = NULL;
    // hehehehe cackled the dungeon master....
    print2 ("Your opponent holds the only way you can leave!");
    Arena_Monster->hp += Arena_Monster->level * 10;
    Arena_Monster->hit += Arena_Monster->hit;
    Arena_Monster->dmg += Arena_Monster->dmg / 2;
}

// loads the sorcereror's circle into Level
void load_circle (int populate)
{
    int i, j;
    int safe = (Player.rank[CIRCLE] > 0);
    char site;
    FILE *fd;

    TempLevel = Level;
    if (ok_to_free (TempLevel)) {
	free_level (TempLevel);
	TempLevel = NULL;
    }
    Level = ((plv) checkmalloc (sizeof (levtype)));
    clear_level (Level);
    Level->environment = E_CIRCLE;
    strcpy (Str3, Omegalib);
    strcat (Str3, "circle.dat");
    fd = checkfopen (Str3, "rb");
    site = cryptkey ("circle.dat");
    for (j = 0; j < LENGTH; j++) {
	for (i = 0; i < WIDTH; i++) {
	    Level->site[i][j].lstatus = 0;
	    Level->site[i][j].roomnumber = RS_CIRCLE;
	    Level->site[i][j].p_locf = L_NO_OP;
	    site = getc (fd) ^ site;
	    switch (site) {
		case 'P':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate) {
			make_prime (i, j);	// prime sorceror
			Level->site[i][j].creature->specialf = M_SP_PRIME;
			if (!safe)
			    m_status_set (Level->site[i][j].creature, HOSTILE);
		    }
		    break;
		case 'D':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate) {
			make_site_monster (i, j, DEMON_PRINCE);	// prime circle demon
			if (safe)
			    m_status_reset (Level->site[i][j].creature, HOSTILE);
			Level->site[i][j].creature->specialf = M_SP_COURT;
		    }
		    break;
		case 's':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate) {
			make_site_monster (i, j, SERV_CHAOS);	// servant of chaos
			Level->site[i][j].creature->specialf = M_SP_COURT;
			if (safe)
			    m_status_reset (Level->site[i][j].creature, HOSTILE);
		    }
		    break;
		case 'e':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate) {
			make_site_monster (i, j, ENCHANTOR);	// enchanter
			Level->site[i][j].creature->specialf = M_SP_COURT;
			if (safe)
			    m_status_reset (Level->site[i][j].creature, HOSTILE);
		    }
		    break;
		case 'n':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate) {
			make_site_monster (i, j, NECROMANCER);	// necromancer
			Level->site[i][j].creature->specialf = M_SP_COURT;
			if (safe)
			    m_status_reset (Level->site[i][j].creature, HOSTILE);
		    }
		    break;
		case 'T':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate) {
			make_site_monster (i, j, THAUMATURGIST);	// High Thaumaturgist
			Level->site[i][j].creature->specialf = M_SP_COURT;
			if (safe)
			    m_status_reset (Level->site[i][j].creature, HOSTILE);
		    }
		    break;
		case '#':
		    Level->site[i][j].locchar = WALL;
		    Level->site[i][j].aux = 1000;
		    break;
		case 'L':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_CIRCLE_LIBRARY;
		    break;
		case '?':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_TOME1;
		    break;
		case '!':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_TOME2;
		    break;
		case 'S':
		    Level->site[i][j].locchar = FLOOR;
		    lset (i, j, SECRET);
		    break;
		case '.':
		    Level->site[i][j].locchar = FLOOR;
		    break;
		case '-':
		    Level->site[i][j].locchar = CLOSED_DOOR;
		    break;
	    }
	}
	site = getc (fd) ^ site;
    }
    fclose (fd);
}

// make the prime sorceror
static void make_prime (int i, int j)
{
    pml ml = ((pml) checkmalloc (sizeof (mltype)));
    pmt m = ((pmt) checkmalloc (sizeof (montype)));
    pol ol;
    pob o;
    make_hiscore_npc (m, 10);	// 10 is index for prime
    m->x = i;
    m->y = j;
    Level->site[i][j].creature = m;
    ml->m = m;
    ml->next = Level->mlist;
    Level->mlist = ml;

    if (Objects[ARTIFACTID + 21].uniqueness != UNIQUE_TAKEN) {
	ol = ((pol) checkmalloc (sizeof (oltype)));
	o = ((pob) checkmalloc (sizeof (objtype)));
	*o = Objects[ARTIFACTID + 21];
	ol->thing = o;
	ol->next = NULL;
	m->possessions = ol;
    }
}

// loads the court of the archmage into Level
void load_court (int populate)
{
    int i, j;
    char site;
    FILE *fd;

    TempLevel = Level;
    if (ok_to_free (TempLevel)) {
	free_level (TempLevel);
	TempLevel = NULL;
    }
    Level = ((plv) checkmalloc (sizeof (levtype)));
    clear_level (Level);
    Level->environment = E_COURT;
    strcpy (Str3, Omegalib);
    strcat (Str3, "court.dat");
    fd = checkfopen (Str3, "rb");
    site = cryptkey ("court.dat");
    for (j = 0; j < LENGTH; j++) {
	for (i = 0; i < WIDTH; i++) {
	    Level->site[i][j].lstatus = 0;
	    Level->site[i][j].roomnumber = RS_COURT;
	    Level->site[i][j].p_locf = L_NO_OP;
	    site = getc (fd) ^ site;
	    switch (site) {
		case '5':
		    Level->site[i][j].locchar = CHAIR;
		    Level->site[i][j].p_locf = L_THRONE;
		    if (populate) {
			make_specific_treasure (i, j, ARTIFACTID + 22);
			make_archmage (i, j);
			m_status_reset (Level->site[i][j].creature, HOSTILE);
			m_status_reset (Level->site[i][j].creature, MOBILE);
		    }
		    break;
		case 'e':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate) {
			make_site_monster (i, j, ENCHANTOR);	// enchanter
			m_status_reset (Level->site[i][j].creature, HOSTILE);
			Level->site[i][j].creature->specialf = M_SP_COURT;
		    }
		    break;
		case 'n':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate) {
			make_site_monster (i, j, NECROMANCER);	// necromancer
			m_status_reset (Level->site[i][j].creature, HOSTILE);
			Level->site[i][j].creature->specialf = M_SP_COURT;
		    }
		    break;
		case 'T':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate) {
			make_site_monster (i, j, THAUMATURGIST);	// High Thaumaturgist
			m_status_reset (Level->site[i][j].creature, HOSTILE);
			Level->site[i][j].creature->specialf = M_SP_COURT;
		    }
		    break;
		case '#':
		    Level->site[i][j].locchar = WALL;
		    Level->site[i][j].aux = 1000;
		    break;
		case 'G':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].locchar = FLOOR;
		    if (populate) {
			make_site_monster (i, j, GUARD);	// guard
			m_status_reset (Level->site[i][j].creature, HOSTILE);
		    }
		    break;
		case '<':
		    Level->site[i][j].locchar = STAIRS_UP;
		    Level->site[i][j].p_locf = L_ESCALATOR;
		    break;
		case '.':
		    Level->site[i][j].locchar = FLOOR;
		    break;
	    }
	}
	site = getc (fd) ^ site;
    }
    fclose (fd);
}

// make the archmage
static void make_archmage (int i, int j)
{
    pml ml = ((pml) checkmalloc (sizeof (mltype)));
    pmt m = ((pmt) checkmalloc (sizeof (montype)));
    make_hiscore_npc (m, 9);	// 9 is index for archmage
    m->x = i;
    m->y = j;
    Level->site[i][j].creature = m;
    ml->m = m;
    ml->next = Level->mlist;
    Level->mlist = ml;
    m->specialf = M_SP_COURT;
}

// loads the abyss level into Level
void load_abyss (void)
{
    int i, j;
    char site;

    FILE *fd;

    TempLevel = Level;
    if (ok_to_free (TempLevel)) {
	free_level (TempLevel);
	TempLevel = NULL;
    }
    Level = ((plv) checkmalloc (sizeof (levtype)));

    clear_level (Level);

    strcpy (Str3, Omegalib);
    strcat (Str3, "abyss.dat");
    fd = checkfopen (Str3, "rb");
    site = cryptkey ("abyss.dat");
    for (j = 0; j < LENGTH; j++) {
	for (i = 0; i < WIDTH; i++) {
	    site = getc (fd) ^ site;
	    Level->site[i][j].roomnumber = RS_ADEPT;
	    switch (site) {
		case '0':
		    Level->site[i][j].locchar = VOID_CHAR;
		    Level->site[i][j].p_locf = L_VOID;
		    break;
		case 'V':
		    Level->site[i][j].locchar = VOID_CHAR;
		    Level->site[i][j].p_locf = L_VOID_STATION;
		    break;
		case '1':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_VOICE1;
		    break;
		case '2':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_VOICE2;
		    break;
		case '3':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_VOICE3;
		    break;
		case '~':
		    Level->site[i][j].locchar = WATER;
		    Level->site[i][j].p_locf = L_WATER_STATION;
		    break;
		case ';':
		    Level->site[i][j].locchar = FIRE;
		    Level->site[i][j].p_locf = L_FIRE_STATION;
		    break;
		case '"':
		    Level->site[i][j].locchar = HEDGE;
		    Level->site[i][j].p_locf = L_EARTH_STATION;
		    break;
		case '6':
		    Level->site[i][j].locchar = WHIRLWIND;
		    Level->site[i][j].p_locf = L_AIR_STATION;
		    break;
		case '#':
		    Level->site[i][j].locchar = WALL;
		    break;
		case '.':
		    Level->site[i][j].locchar = FLOOR;
		    break;
	    }
	}
	site = getc (fd) ^ site;
    }
    fclose (fd);
}

// loads the city level
void load_city (int populate)
{
    int i, j;
    pml ml;
    char site;

    FILE *fd;

    initrand (E_CITY, 0);

    strcpy (Str3, Omegalib);
    strcat (Str3, "city.dat");
    fd = checkfopen (Str3, "rb");
    site = cryptkey ("city.dat");

    TempLevel = Level;
    if (ok_to_free (TempLevel)) {
	free_level (TempLevel);
	TempLevel = NULL;
    }
    Level = ((plv) checkmalloc (sizeof (levtype)));
    clear_level (Level);
    Level->depth = 0;
    Level->environment = E_CITY;
    for (j = 0; j < LENGTH; j++) {
	for (i = 0; i < WIDTH; i++) {
	    lset (i, j, SEEN);
	    site = getc (fd) ^ site;
	    switch (site) {
		case 'g':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_GARDEN;
		    break;
		case 'y':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_CEMETARY;
		    break;
		case 'x':
		    assign_city_function (i, j);
		    break;
		case 't':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_TEMPLE;
		    CitySiteList[L_TEMPLE - CITYSITEBASE][0] = TRUE;
		    CitySiteList[L_TEMPLE - CITYSITEBASE][1] = i;
		    CitySiteList[L_TEMPLE - CITYSITEBASE][2] = j;
		    break;
		case 'T':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_PORTCULLIS_TRAP;
		    Level->site[i][j].aux = NOCITYMOVE;
		    break;
		case 'R':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_RAISE_PORTCULLIS;
		    Level->site[i][j].aux = NOCITYMOVE;
		    break;
		case '7':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_PORTCULLIS;
		    Level->site[i][j].aux = NOCITYMOVE;
		    break;
		case 'C':
		    Level->site[i][j].locchar = OPEN_DOOR;
		    Level->site[i][j].p_locf = L_COLLEGE;
		    CitySiteList[L_COLLEGE - CITYSITEBASE][0] = TRUE;
		    CitySiteList[L_COLLEGE - CITYSITEBASE][1] = i;
		    CitySiteList[L_COLLEGE - CITYSITEBASE][2] = j;
		    break;
		case 's':
		    Level->site[i][j].locchar = OPEN_DOOR;
		    Level->site[i][j].p_locf = L_SORCERORS;
		    CitySiteList[L_SORCERORS - CITYSITEBASE][0] = TRUE;
		    CitySiteList[L_SORCERORS - CITYSITEBASE][1] = i;
		    CitySiteList[L_SORCERORS - CITYSITEBASE][2] = j;
		    break;
		case 'M':
		    Level->site[i][j].locchar = OPEN_DOOR;
		    Level->site[i][j].p_locf = L_MERC_GUILD;
		    CitySiteList[L_MERC_GUILD - CITYSITEBASE][0] = TRUE;
		    CitySiteList[L_MERC_GUILD - CITYSITEBASE][1] = i;
		    CitySiteList[L_MERC_GUILD - CITYSITEBASE][2] = j;
		    break;
		case 'c':
		    Level->site[i][j].locchar = OPEN_DOOR;
		    Level->site[i][j].p_locf = L_CASTLE;
		    CitySiteList[L_CASTLE - CITYSITEBASE][0] = TRUE;
		    CitySiteList[L_CASTLE - CITYSITEBASE][1] = i;
		    CitySiteList[L_CASTLE - CITYSITEBASE][2] = j;
		    break;
		case '?':
		    mazesite (i, j, populate);
		    break;
		case 'P':
		    Level->site[i][j].locchar = OPEN_DOOR;
		    Level->site[i][j].p_locf = L_ORDER;
		    CitySiteList[L_ORDER - CITYSITEBASE][0] = TRUE;
		    CitySiteList[L_ORDER - CITYSITEBASE][1] = i;
		    CitySiteList[L_ORDER - CITYSITEBASE][2] = j;
		    break;
		case 'H':
		    Level->site[i][j].locchar = OPEN_DOOR;
		    Level->site[i][j].p_locf = L_CHARITY;
		    CitySiteList[L_CHARITY - CITYSITEBASE][0] = TRUE;
		    CitySiteList[L_CHARITY - CITYSITEBASE][1] = i;
		    CitySiteList[L_CHARITY - CITYSITEBASE][2] = j;
		    break;
		case 'j':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate)
			make_justiciar (i, j);
		    break;
		case 'J':
		    Level->site[i][j].locchar = CLOSED_DOOR;
		    Level->site[i][j].p_locf = L_JAIL;
		    break;
		case 'A':
		    Level->site[i][j].locchar = OPEN_DOOR;
		    Level->site[i][j].p_locf = L_ARENA;
		    CitySiteList[L_ARENA - CITYSITEBASE][0] = TRUE;
		    CitySiteList[L_ARENA - CITYSITEBASE][1] = i;
		    CitySiteList[L_ARENA - CITYSITEBASE][2] = j;
		    break;
		case 'B':
		    Level->site[i][j].locchar = OPEN_DOOR;
		    Level->site[i][j].p_locf = L_BANK;
		    CitySiteList[L_BANK - CITYSITEBASE][0] = TRUE;
		    CitySiteList[L_BANK - CITYSITEBASE][1] = i;
		    CitySiteList[L_BANK - CITYSITEBASE][2] = j;
		    lset (i, j + 1, STOPS);
		    lset (i + 1, j, STOPS);
		    lset (i - 1, j, STOPS);
		    lset (i, j - 1, STOPS);
		    break;
		case 'X':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_COUNTRYSIDE;
		    CitySiteList[L_COUNTRYSIDE - CITYSITEBASE][0] = TRUE;
		    CitySiteList[L_COUNTRYSIDE - CITYSITEBASE][1] = i;
		    CitySiteList[L_COUNTRYSIDE - CITYSITEBASE][2] = j;
		    break;
		case 'v':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_VAULT;
		    Level->site[i][j].aux = NOCITYMOVE;
		    lset (i, j, SECRET);
		    break;
		case 'S':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].aux = NOCITYMOVE;
		    lset (i, j, SECRET);
		    break;
		case 'G':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate) {
			make_site_monster (i, j, GUARD);
			Level->site[i][j].creature->aux1 = i;
			Level->site[i][j].creature->aux2 = j;
		    }
		    break;
		case 'u':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate)
			make_minor_undead (i, j);
		    break;
		case 'U':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate)
			make_major_undead (i, j);
		    break;
		case 'V':
		    Level->site[i][j].showchar = WALL;
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_VAULT;
		    if (populate)
			make_site_treasure (i, j, 5);
		    Level->site[i][j].aux = NOCITYMOVE;
		    lset (i, j, SECRET);
		    break;
		case '%':
		    Level->site[i][j].showchar = WALL;
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_TRAP_SIREN;
		    if (populate)
			make_site_treasure (i, j, 5);
		    Level->site[i][j].aux = NOCITYMOVE;
		    lset (i, j, SECRET);
		    break;
		case '$':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate)
			make_site_treasure (i, j, 5);
		    break;
		case '2':
		    Level->site[i][j].locchar = ALTAR;
		    Level->site[i][j].p_locf = L_ALTAR;
		    Level->site[i][j].aux = ODIN;
		    break;
		case '3':
		    Level->site[i][j].locchar = ALTAR;
		    Level->site[i][j].p_locf = L_ALTAR;
		    Level->site[i][j].aux = SET;
		    break;
		case '4':
		    Level->site[i][j].locchar = ALTAR;
		    Level->site[i][j].p_locf = L_ALTAR;
		    Level->site[i][j].aux = ATHENA;
		    break;
		case '5':
		    Level->site[i][j].locchar = ALTAR;
		    Level->site[i][j].p_locf = L_ALTAR;
		    Level->site[i][j].aux = HECATE;
		    break;
		case '6':
		    Level->site[i][j].locchar = ALTAR;
		    Level->site[i][j].p_locf = L_ALTAR;
		    Level->site[i][j].aux = DESTINY;
		    break;
		case '^':
		    Level->site[i][j].showchar = WALL;
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = TRAP_BASE + random_range (NUMTRAPS);
		    lset (i, j, SECRET);
		    break;
		case '"':
		    Level->site[i][j].locchar = HEDGE;
		    break;
		case '~':
		    Level->site[i][j].locchar = WATER;
		    Level->site[i][j].p_locf = L_WATER;
		    break;
		case '=':
		    Level->site[i][j].locchar = WATER;
		    Level->site[i][j].p_locf = L_MAGIC_POOL;
		    break;
		case '*':
		    Level->site[i][j].locchar = WALL;
		    Level->site[i][j].aux = 10;
		    break;
		case '#':
		    Level->site[i][j].locchar = WALL;
		    Level->site[i][j].aux = 500;
		    break;
		case '.':
		    Level->site[i][j].locchar = FLOOR;
		    break;
		case ',':
		    Level->site[i][j].showchar = WALL;
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].aux = NOCITYMOVE;
		    lset (i, j, SECRET);
		    break;
		case '-':
		    Level->site[i][j].locchar = CLOSED_DOOR;
		    break;
		case '1':
		    Level->site[i][j].locchar = STATUE;
		    break;
		default:
		    printf ("\nOops... missed a case: '%c'   \n", site);
		    morewait();
	    }

	    if (loc_statusp (i, j, SEEN)) {
		if (loc_statusp (i, j, SECRET))
		    Level->site[i][j].showchar = WALL;
		else
		    Level->site[i][j].showchar = Level->site[i][j].locchar;
	    }
	}
	site = getc (fd) ^ site;
    }
    City = Level;

    // make all city monsters asleep, and shorten their wakeup range to 2
    // to prevent players from being molested by vicious monsters on
    // the streets
    for (ml = Level->mlist; ml != NULL; ml = ml->next) {
	m_status_reset (ml->m, AWAKE);
	ml->m->wakeup = 2;
    }
    fclose (fd);
    initrand (E_RESTORE, 0);
}

static void assign_city_function (int x, int y)
{
    static int setup = 0;
    static int next = 0;
    static int permutation[64];	// number of x's in city map
    int i, j, k, l;

    Level->site[x][y].aux = TRUE;

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
	Level->site[x][y].locchar = CLOSED_DOOR;
	Level->site[x][y].p_locf = L_HOUSE;
	if (random_range (5))
	    Level->site[x][y].aux = LOCKED;
    } else
	switch (permutation[next]) {
	    case 0:
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_ARMORER;
		CitySiteList[L_ARMORER - CITYSITEBASE][1] = x;
		CitySiteList[L_ARMORER - CITYSITEBASE][2] = y;
		break;
	    case 1:
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_CLUB;
		CitySiteList[L_CLUB - CITYSITEBASE][1] = x;
		CitySiteList[L_CLUB - CITYSITEBASE][2] = y;
		break;
	    case 2:
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_GYM;
		CitySiteList[L_GYM - CITYSITEBASE][1] = x;
		CitySiteList[L_GYM - CITYSITEBASE][2] = y;
		break;
	    case 3:
		Level->site[x][y].locchar = CLOSED_DOOR;
		Level->site[x][y].p_locf = L_THIEVES_GUILD;
		CitySiteList[L_THIEVES_GUILD - CITYSITEBASE][1] = x;
		CitySiteList[L_THIEVES_GUILD - CITYSITEBASE][2] = y;
		break;
	    case 4:
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_HEALER;
		CitySiteList[L_HEALER - CITYSITEBASE][1] = x;
		CitySiteList[L_HEALER - CITYSITEBASE][2] = y;
		break;
	    case 5:
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_CASINO;
		CitySiteList[L_CASINO - CITYSITEBASE][1] = x;
		CitySiteList[L_CASINO - CITYSITEBASE][2] = y;
		break;
	    case 7:
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_DINER;
		CitySiteList[L_DINER - CITYSITEBASE][1] = x;
		CitySiteList[L_DINER - CITYSITEBASE][2] = y;
		break;
	    case 8:
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_CRAP;
		CitySiteList[L_CRAP - CITYSITEBASE][1] = x;
		CitySiteList[L_CRAP - CITYSITEBASE][2] = y;
		break;
	    case 6:
	    case 9:
	    case 20:
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_COMMANDANT;
		CitySiteList[L_COMMANDANT - CITYSITEBASE][1] = x;
		CitySiteList[L_COMMANDANT - CITYSITEBASE][2] = y;
		break;
	    case 21:
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_TAVERN;
		CitySiteList[L_TAVERN - CITYSITEBASE][1] = x;
		CitySiteList[L_TAVERN - CITYSITEBASE][2] = y;
		break;
	    case 10:
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_ALCHEMIST;
		CitySiteList[L_ALCHEMIST - CITYSITEBASE][1] = x;
		CitySiteList[L_ALCHEMIST - CITYSITEBASE][2] = y;
		break;
	    case 11:
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_DPW;
		CitySiteList[L_DPW - CITYSITEBASE][1] = x;
		CitySiteList[L_DPW - CITYSITEBASE][2] = y;
		break;
	    case 12:
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_LIBRARY;
		CitySiteList[L_LIBRARY - CITYSITEBASE][1] = x;
		CitySiteList[L_LIBRARY - CITYSITEBASE][2] = y;
		break;
	    case 13:
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_PAWN_SHOP;
		CitySiteList[L_PAWN_SHOP - CITYSITEBASE][1] = x;
		CitySiteList[L_PAWN_SHOP - CITYSITEBASE][2] = y;
		break;
	    case 14:
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_CONDO;
		CitySiteList[L_CONDO - CITYSITEBASE][1] = x;
		CitySiteList[L_CONDO - CITYSITEBASE][2] = y;
		break;
	    case 15:
		Level->site[x][y].locchar = CLOSED_DOOR;
		Level->site[x][y].p_locf = L_BROTHEL;
		CitySiteList[L_BROTHEL - CITYSITEBASE][1] = x;
		CitySiteList[L_BROTHEL - CITYSITEBASE][2] = y;
		break;
	    default:
		Level->site[x][y].locchar = CLOSED_DOOR;
		switch (random_range (6)) {
		    case 0:
			Level->site[x][y].p_locf = L_HOVEL;
			break;
		    case 1:
		    case 2:
		    case 3:
		    case 4:
			Level->site[x][y].p_locf = L_HOUSE;
			break;
		    case 5:
			Level->site[x][y].p_locf = L_MANSION;
			break;
		}
		if (random_range (5))
		    Level->site[x][y].aux = LOCKED;
		break;
	}
    next++;
}

// makes a hiscore npc for mansions
static void make_justiciar (int i, int j)
{
    pml ml = ((pml) checkmalloc (sizeof (mltype)));
    ml->m = ((pmt) checkmalloc (sizeof (montype)));
    *(ml->m) = Monsters[NPC];
    make_hiscore_npc (ml->m, 15);
    ml->m->x = i;
    ml->m->y = j;
    Level->site[i][j].creature = ml->m;
    ml->m->click = (Tick + 1) % 60;
    ml->next = Level->mlist;
    Level->mlist = ml;
    m_status_reset (ml->m, AWAKE);
}

// loads the city level
void resurrect_guards (void)
{
    int i, j;
    char site;

    FILE *fd;

    strcpy (Str3, Omegalib);
    strcat (Str3, "city.dat");
    fd = checkfopen (Str3, "rb");
    site = cryptkey ("city.dat");
    for (j = 0; j < LENGTH; j++) {
	for (i = 0; i < WIDTH; i++) {
	    site = getc (fd) ^ site;
	    if (site == 'G') {
		make_site_monster (i, j, GUARD);
		Level->site[i][j].creature->monstring = "undead guardsman";
		Level->site[i][j].creature->meleef = M_MELEE_SPIRIT;
		Level->site[i][j].creature->movef = M_MOVE_SPIRIT;
		Level->site[i][j].creature->strikef = M_STRIKE_MISSILE;
		Level->site[i][j].creature->immunity = EVERYTHING - pow2 (NORMAL_DAMAGE);
		Level->site[i][j].creature->hp *= 2;
		Level->site[i][j].creature->hit *= 2;
		Level->site[i][j].creature->dmg *= 2;
		Level->site[i][j].creature->ac *= 2;
		m_status_set (Level->site[i][j].creature, HOSTILE);
		m_status_set (Level->site[i][j].creature, AWAKE);
	    }
	}
	site = getc (fd) ^ site;
    }
    fclose (fd);
}

static void mazesite (int i, int j, int populate)
{
    static FILE *fd = NULL;
    static int k = 0;
    static char site;
    if (fd == NULL) {
	strcpy (Str2, Omegalib);
	strcpy (Str4, "maze .dat");
	Str4[4] = '1' + random_range (4);
	strcat (Str2, Str4);
	fd = checkfopen (Str2, "rb");
	site = cryptkey ("mazes");
    }
    site = getc (fd) ^ site;
    k++;
    if (k == 286)
	fclose (fd);
    switch (site) {
	case '"':
	    Level->site[i][j].locchar = HEDGE;
	    if (random_range (10))
		Level->site[i][j].p_locf = L_HEDGE;
	    else
		Level->site[i][j].p_locf = L_TRIFID;
	    break;
	case '-':
	    Level->site[i][j].locchar = CLOSED_DOOR;
	    break;
	case '.':
	    Level->site[i][j].locchar = FLOOR;
	    break;
	case '>':
	    Level->site[i][j].locchar = STAIRS_DOWN;
	    Level->site[i][j].p_locf = L_SEWER;
	    break;
	case 'z':
	    Level->site[i][j].locchar = FLOOR;
	    Level->site[i][j].p_locf = L_MAZE;
	    break;
	case 'O':
	    Level->site[i][j].locchar = OPEN_DOOR;
	    Level->site[i][j].p_locf = L_ORACLE;
	    CitySiteList[L_ORACLE - CITYSITEBASE][1] = i;
	    CitySiteList[L_ORACLE - CITYSITEBASE][2] = j;
	    break;
	case '?':
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
	    Level->site[i][j].locchar = FLOOR;
	    Level->site[i][j].p_locf = TRAP_BASE + random_range (NUMTRAPS);
	    break;
	case 2:
	case 3:
	    Level->site[i][j].locchar = FLOOR;
	    if (populate)
		make_site_monster (i, j, -1);
	    break;
	case 4:
	case 5:
	    Level->site[i][j].locchar = FLOOR;
	    if (populate)
		make_site_treasure (i, j, 5);
	    break;
	default:
	    Level->site[i][j].locchar = FLOOR;
    }
}

// undead are not hostile unless disturbed....
static void make_minor_undead (int i, int j)
{
    int mid;
    if (random_range (2))
	mid = GHOST;
    else
	mid = HAUNT;
    make_site_monster (i, j, mid);
    m_status_reset (Level->site[i][j].creature, AWAKE);
    m_status_reset (Level->site[i][j].creature, HOSTILE);
}

// undead are not hostile unless disturbed....
static void make_major_undead (int i, int j)
{
    int mid;
    if (random_range (2))
	mid = LICHE;
    else
	mid = VAMP_LORD;
    make_site_monster (i, j, mid);
    m_status_reset (Level->site[i][j].creature, AWAKE);
    m_status_reset (Level->site[i][j].creature, HOSTILE);
}

static const char jail[5][11] = {
    {'#', '#', '*', '#', '#', '*', '#', '#', '*', '#', '#'},
    {'#', '#', '*', '#', '#', '*', '#', '*', '#', '#', '#'},
    {'#', '#', 'T', '#', 'T', '#', 'T', '#', 'T', '#', '#'},
    {'#', '#', '7', '#', '7', '#', '7', '#', '7', '#', '#'},
    {'#', '#', 'R', '#', 'R', '#', 'R', '#', 'R', '#', '#'}
};

// fixes up the jail in case it has been munged by player action
static void repair_jail (void)
{
    int i, j;
    for (i = 0; i < 11; i++)
	for (j = 0; j < 5; j++) {
	    switch (jail[j][i]) {
		case '#':
		    City->site[i + 35][j + 52].locchar = WALL;
		    City->site[i + 35][j + 52].p_locf = L_NO_OP;
		    City->site[i + 35][j + 52].aux = NOCITYMOVE;
		    break;
		case '*':
		    City->site[i + 35][j + 52].locchar = WALL;
		    City->site[i + 35][j + 52].p_locf = L_NO_OP;
		    City->site[i + 35][j + 52].aux = 10;
		    break;
		case 'T':
		    City->site[i + 35][j + 52].locchar = FLOOR;
		    City->site[i + 35][j + 52].p_locf = L_PORTCULLIS_TRAP;
		    City->site[i + 35][j + 52].aux = NOCITYMOVE;
		    break;
		case '7':
		    City->site[i + 35][j + 52].locchar = FLOOR;
		    City->site[i + 35][j + 52].p_locf = L_PORTCULLIS;
		    City->site[i + 35][j + 52].aux = NOCITYMOVE;
		    break;
		case 'R':
		    City->site[i + 35][j + 52].locchar = FLOOR;
		    City->site[i + 35][j + 52].p_locf = L_RAISE_PORTCULLIS;
		    City->site[i + 35][j + 52].aux = NOCITYMOVE;
		    break;
	    }
	    lreset (i + 35, j + 52, CHANGED);
	}
}

// loads the countryside level from the data file
void load_country (void)
{
    int i, j;
    char site;

    FILE *fd;

    strcpy (Str3, Omegalib);
    strcat (Str3, "country.dat");
    fd = checkfopen (Str3, "rb");
    site = cryptkey ("country.dat");

    for (j = 0; j < LENGTH; j++) {
	for (i = 0; i < WIDTH; i++) {
	    site = getc (fd) ^ site;
	    Country[i][j].aux = 0;
	    Country[i][j].status = 0;
	    switch (site) {
		case (PASS & 0xff):
		    Country[i][j].base_terrain_type = PASS;
		    Country[i][j].current_terrain_type = MOUNTAINS;
		    break;
		case (CASTLE & 0xff):
		    Country[i][j].base_terrain_type = CASTLE;
		    Country[i][j].current_terrain_type = MOUNTAINS;
		    break;
		case (STARPEAK & 0xff):
		    Country[i][j].base_terrain_type = STARPEAK;
		    Country[i][j].current_terrain_type = MOUNTAINS;
		    break;
		case (CAVES & 0xff):
		    Country[i][j].base_terrain_type = CAVES;
		    Country[i][j].current_terrain_type = MOUNTAINS;
		    break;
		case (VOLCANO & 0xff):
		    Country[i][j].base_terrain_type = VOLCANO;
		    Country[i][j].current_terrain_type = MOUNTAINS;
		    break;
		case (DRAGONLAIR & 0xff):
		    Country[i][j].base_terrain_type = DRAGONLAIR;
		    Country[i][j].current_terrain_type = DESERT;
		    break;
		case (MAGIC_ISLE & 0xff):
		    Country[i][j].base_terrain_type = MAGIC_ISLE;
		    Country[i][j].current_terrain_type = CHAOS_SEA;
		    break;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		    Country[i][j].current_terrain_type = Country[i][j].base_terrain_type = VILLAGE;
		    Country[i][j].aux = 1 + site - 'a';
		    break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		    Country[i][j].current_terrain_type = Country[i][j].base_terrain_type = TEMPLE;
		    Country[i][j].aux = site - '0';
		    break;
		case (PLAINS & 0xff):
		    Country[i][j].current_terrain_type = Country[i][j].base_terrain_type = PLAINS;
		    break;
		case (TUNDRA & 0xff):
		    Country[i][j].current_terrain_type = Country[i][j].base_terrain_type = TUNDRA;
		    break;
		case (ROAD & 0xff):
		    Country[i][j].current_terrain_type = Country[i][j].base_terrain_type = ROAD;
		    break;
		case (MOUNTAINS & 0xff):
		    Country[i][j].current_terrain_type = Country[i][j].base_terrain_type = MOUNTAINS;
		    break;
		case (RIVER & 0xff):
		    Country[i][j].current_terrain_type = Country[i][j].base_terrain_type = RIVER;
		    break;
		case (CITY & 0xff):
		    Country[i][j].current_terrain_type = Country[i][j].base_terrain_type = CITY;
		    break;
		case (FOREST & 0xff):
		    Country[i][j].current_terrain_type = Country[i][j].base_terrain_type = FOREST;
		    break;
		case (JUNGLE & 0xff):
		    Country[i][j].current_terrain_type = Country[i][j].base_terrain_type = JUNGLE;
		    break;
		case (SWAMP & 0xff):
		    Country[i][j].current_terrain_type = Country[i][j].base_terrain_type = SWAMP;
		    break;
		case (DESERT & 0xff):
		    Country[i][j].current_terrain_type = Country[i][j].base_terrain_type = DESERT;
		    break;
		case (CHAOS_SEA & 0xff):
		    Country[i][j].current_terrain_type = Country[i][j].base_terrain_type = CHAOS_SEA;
		    break;
	    }
	}
	site = getc (fd) ^ site;
    }
    fclose (fd);
}

// loads the dragon's lair into Level
void load_dlair (int empty, int populate)
{
    int i, j;
    char site;

    FILE *fd;

    if (empty) {
	mprint ("The Lair is now devoid of inhabitants and treasure.");
	morewait();
    }

    if (!populate)
	empty = TRUE;
    TempLevel = Level;
    if (ok_to_free (TempLevel)) {
	free_level (TempLevel);
	TempLevel = NULL;
    }
    Level = ((plv) checkmalloc (sizeof (levtype)));
    clear_level (Level);
    Level->environment = E_DLAIR;
    strcpy (Str3, Omegalib);
    strcat (Str3, "dlair.dat");
    fd = checkfopen (Str3, "rb");
    site = cryptkey ("dlair.dat");
    for (j = 0; j < LENGTH; j++) {
	for (i = 0; i < WIDTH; i++) {
	    Level->site[i][j].lstatus = 0;
	    if (i < 48)
		Level->site[i][j].roomnumber = RS_CAVERN;
	    else
		Level->site[i][j].roomnumber = RS_DRAGONLORD;
	    Level->site[i][j].p_locf = L_NO_OP;
	    site = getc (fd) ^ site;
	    switch (site) {
		case 'D':
		    Level->site[i][j].locchar = FLOOR;
		    if (!empty) {
			make_site_monster (i, j, DRAGON_LORD);
			Level->site[i][j].creature->specialf = M_SP_LAIR;
		    }
		    break;
		case 'd':
		    Level->site[i][j].locchar = FLOOR;
		    if (!empty) {
			make_site_monster (i, j, DRAGON);	// elite dragons, actually
			Level->site[i][j].creature->specialf = M_SP_LAIR;
			Level->site[i][j].creature->hit *= 2;
			Level->site[i][j].creature->dmg *= 2;
		    }
		    break;
		case 'W':
		    Level->site[i][j].locchar = FLOOR;
		    if (!empty)
			make_site_monster (i, j, KING_WYV);
		    break;
		case 'M':
		    Level->site[i][j].locchar = FLOOR;
		    if (!empty)
			make_site_monster (i, j, RANDOM);
		    break;
		case 'S':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].showchar = WALL;
		    if (!empty)
			lset (i, j, SECRET);
		    Level->site[i][j].roomnumber = RS_SECRETPASSAGE;
		    break;
		case '$':
		    Level->site[i][j].locchar = FLOOR;
		    if (!empty)
			make_site_treasure (i, j, 10);
		    break;
		case 's':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_TRAP_SIREN;
		    break;
		case '7':
		    if (!empty)
			Level->site[i][j].locchar = PORTCULLIS;
		    else
			Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_PORTCULLIS;
		    break;
		case 'R':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_RAISE_PORTCULLIS;
		    break;
		case 'p':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_PORTCULLIS;
		    break;
		case 'T':
		    Level->site[i][j].locchar = FLOOR;
		    if (!empty)
			Level->site[i][j].p_locf = L_PORTCULLIS_TRAP;
		    break;
		case 'X':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_TACTICAL_EXIT;
		    break;
		case '#':
		    Level->site[i][j].locchar = WALL;
		    Level->site[i][j].aux = 150;
		    break;
		case '.':
		    Level->site[i][j].locchar = FLOOR;
		    break;
	    }
	}
	site = getc (fd) ^ site;
    }
    fclose (fd);
}

// loads the star peak into Level
void load_speak (int empty, int populate)
{
    int i, j, safe = Player.alignment > 0;
    char site;

    FILE *fd;

    if (empty) {
	mprint ("The peak is now devoid of inhabitants and treasure.");
	morewait();
    }

    if (!populate)
	empty = TRUE;

    TempLevel = Level;
    if (ok_to_free (TempLevel)) {
	free_level (TempLevel);
	TempLevel = NULL;
    }
    Level = ((plv) checkmalloc (sizeof (levtype)));
    clear_level (Level);
    Level->environment = E_STARPEAK;
    strcpy (Str3, Omegalib);
    strcat (Str3, "speak.dat");
    fd = checkfopen (Str3, "rb");
    site = cryptkey ("speak.dat");
    for (j = 0; j < LENGTH; j++) {
	for (i = 0; i < WIDTH; i++) {
	    Level->site[i][j].lstatus = 0;
	    Level->site[i][j].roomnumber = RS_STARPEAK;
	    Level->site[i][j].p_locf = L_NO_OP;
	    site = getc (fd) ^ site;
	    switch (site) {
		case 'S':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].showchar = WALL;
		    lset (i, j, SECRET);
		    Level->site[i][j].roomnumber = RS_SECRETPASSAGE;
		    break;
		case 'L':
		    Level->site[i][j].locchar = FLOOR;
		    if (!empty) {
			make_site_monster (i, j, LAWBRINGER);
			if (safe)
			    m_status_reset (Level->site[i][j].creature, HOSTILE);
		    }
		    break;
		case 's':
		    Level->site[i][j].locchar = FLOOR;
		    if (!empty) {
			make_site_monster (i, j, SERV_LAW);	// servant of law
			if (safe)
			    m_status_reset (Level->site[i][j].creature, HOSTILE);
		    }
		    break;
		case 'M':
		    Level->site[i][j].locchar = FLOOR;
		    if (!empty) {
			make_site_monster (i, j, -1);
			if (safe)
			    m_status_reset (Level->site[i][j].creature, HOSTILE);
		    }
		    break;
		case '$':
		    Level->site[i][j].locchar = FLOOR;
		    if (!empty)
			make_site_treasure (i, j, 10);
		    break;
		case '7':
		    if (!empty)
			Level->site[i][j].locchar = PORTCULLIS;
		    else
			Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_PORTCULLIS;
		    break;
		case 'R':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_RAISE_PORTCULLIS;
		    break;
		case '-':
		    Level->site[i][j].locchar = CLOSED_DOOR;
		    break;
		case '|':
		    Level->site[i][j].locchar = OPEN_DOOR;
		    break;
		case 'p':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_PORTCULLIS;
		    break;
		case 'T':
		    Level->site[i][j].locchar = FLOOR;
		    if (!empty)
			Level->site[i][j].p_locf = L_PORTCULLIS_TRAP;
		    break;
		case 'X':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_TACTICAL_EXIT;
		    break;
		case '#':
		    Level->site[i][j].locchar = WALL;
		    Level->site[i][j].aux = 150;
		    break;
		case '4':
		    Level->site[i][j].locchar = RUBBLE;
		    Level->site[i][j].p_locf = L_RUBBLE;
		    break;
		case '.':
		    Level->site[i][j].locchar = FLOOR;
		    break;
	    }
	}
	site = getc (fd) ^ site;
    }
    fclose (fd);
}

// loads the magic isle into Level
void load_misle (int empty, int populate)
{
    int i, j;
    char site;

    FILE *fd;

    if (empty) {
	mprint ("The isle is now devoid of inhabitants and treasure.");
	morewait();
    }

    if (!populate)
	empty = TRUE;

    TempLevel = Level;
    if (ok_to_free (TempLevel)) {
	free_level (TempLevel);
	TempLevel = NULL;
    }
    Level = ((plv) checkmalloc (sizeof (levtype)));
    clear_level (Level);
    Level->environment = E_MAGIC_ISLE;
    strcpy (Str3, Omegalib);
    strcat (Str3, "misle.dat");
    fd = checkfopen (Str3, "rb");
    site = cryptkey ("misle.dat");
    for (j = 0; j < LENGTH; j++) {
	for (i = 0; i < WIDTH; i++) {
	    Level->site[i][j].lstatus = 0;
	    Level->site[i][j].roomnumber = RS_MAGIC_ISLE;
	    Level->site[i][j].p_locf = L_NO_OP;
	    site = getc (fd) ^ site;
	    switch (site) {
		case 'E':
		    Level->site[i][j].locchar = FLOOR;
		    if (!empty)
			make_site_monster (i, j, EATER);	// eater of magic
		    break;
		case 'm':
		    Level->site[i][j].locchar = FLOOR;
		    if (!empty)
			make_site_monster (i, j, MIL_PRIEST);	// militant priest
		    break;
		case 'n':
		    Level->site[i][j].locchar = FLOOR;
		    if (!empty)
			make_site_monster (i, j, NAZGUL);
		    break;
		case 'X':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_TACTICAL_EXIT;
		    break;
		case '#':
		    Level->site[i][j].locchar = WALL;
		    Level->site[i][j].aux = 150;
		    break;
		case '4':
		    Level->site[i][j].locchar = RUBBLE;
		    Level->site[i][j].p_locf = L_RUBBLE;
		    break;
		case '~':
		    Level->site[i][j].locchar = WATER;
		    Level->site[i][j].p_locf = L_CHAOS;
		    break;
		case '=':
		    Level->site[i][j].locchar = WATER;
		    Level->site[i][j].p_locf = L_MAGIC_POOL;
		    break;
		case '-':
		    Level->site[i][j].locchar = CLOSED_DOOR;
		    break;
		case '|':
		    Level->site[i][j].locchar = OPEN_DOOR;
		    break;
		case '.':
		    Level->site[i][j].locchar = FLOOR;
		    break;
	    }
	}
	site = getc (fd) ^ site;
    }
    fclose (fd);
}

// loads a temple into Level
void load_temple (int deity, int populate)
{
    int i, j;
    char site;
    pml ml;
    FILE *fd;

    // WDT HACK: I don't know why this is wrong.  Shrug.  David Givens
    // suggested removing it, and he has more experience with Omega
    // than I, so...
    //  initrand(Current_Environment, deity);
    TempLevel = Level;
    if (ok_to_free (TempLevel)) {
	free_level (TempLevel);
	TempLevel = NULL;
    }
    Level = ((plv) checkmalloc (sizeof (levtype)));
    clear_level (Level);
    Level->environment = E_TEMPLE;
    strcpy (Str3, Omegalib);
    strcat (Str3, "temple.dat");
    fd = checkfopen (Str3, "rb");
    site = cryptkey ("temple.dat");
    for (j = 0; j < LENGTH; j++) {
	for (i = 0; i < WIDTH; i++) {
	    switch (deity) {
		case ODIN:
		    Level->site[i][j].roomnumber = RS_ODIN;
		    break;
		case SET:
		    Level->site[i][j].roomnumber = RS_SET;
		    break;
		case HECATE:
		    Level->site[i][j].roomnumber = RS_HECATE;
		    break;
		case ATHENA:
		    Level->site[i][j].roomnumber = RS_ATHENA;
		    break;
		case DRUID:
		    Level->site[i][j].roomnumber = RS_DRUID;
		    break;
		case DESTINY:
		    Level->site[i][j].roomnumber = RS_DESTINY;
		    break;
	    }
	    site = getc (fd) ^ site;
	    switch (site) {
		case '8':
		    Level->site[i][j].locchar = ALTAR;
		    Level->site[i][j].p_locf = L_ALTAR;
		    Level->site[i][j].aux = deity;
		    break;
		case 'H':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate && (!Player.patron || strcmp (Player.name, Priest[Player.patron]) || Player.rank[PRIESTHOOD] != HIGHPRIEST))
			make_high_priest (i, j, deity);
		    break;
		case 'S':
		    Level->site[i][j].locchar = FLOOR;
		    if (!Player.patron || strcmp (Player.name, Priest[Player.patron]) || Player.rank[PRIESTHOOD] != HIGHPRIEST)
			lset (i, j, SECRET);
		    break;
		case 'W':
		    Level->site[i][j].locchar = FLOOR;
		    if (deity != Player.patron && deity != DRUID)
			Level->site[i][j].p_locf = L_TEMPLE_WARNING;
		    break;
		case 'm':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate)
			make_site_monster (i, j, MIL_PRIEST);	// militant priest
		    break;
		case 'd':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate)
			make_site_monster (i, j, DOBERMAN);	// doberman death hound
		    break;
		case 'X':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_TACTICAL_EXIT;
		    break;
		case '#':
		    if (deity != DRUID) {
			Level->site[i][j].locchar = WALL;
			Level->site[i][j].aux = 150;
		    } else {
			Level->site[i][j].locchar = HEDGE;
			Level->site[i][j].p_locf = L_HEDGE;
		    }
		    break;
		case '.':
		    Level->site[i][j].locchar = FLOOR;
		    break;
		case 'x':
		    Level->site[i][j].locchar = FLOOR;
		    random_temple_site (i, j, deity, populate);
		    break;
		case '?':
		    if (deity != DESTINY)
			Level->site[i][j].locchar = FLOOR;
		    else {
			Level->site[i][j].locchar = ABYSS;
			Level->site[i][j].p_locf = L_ADEPT;
		    }
		    break;
		case '-':
		    Level->site[i][j].locchar = CLOSED_DOOR;
		    break;
		case '|':
		    Level->site[i][j].locchar = OPEN_DOOR;
		    break;
	    }
	}
	site = getc (fd) ^ site;
    }
    // Main Temple is peaceful for player of same sect,druids always peaceful.
    if ((Player.patron == deity) || (deity == DRUID))
	for (ml = Level->mlist; ml != NULL; ml = ml->next)
	    m_status_reset (ml->m, HOSTILE);
    fclose (fd);
    //  initrand(-2, 0);
}

static void random_temple_site (int i, int j, int deity UNUSED, int populate)
{
    switch (random_range (12)) {
	case 0:
	    if (populate)
		make_site_monster (i, j, MEND_PRIEST);
	    break;		// mendicant priest
	case 1:
	    Level->site[i][j].locchar = WATER;
	    Level->site[i][j].p_locf = L_MAGIC_POOL;
	case 2:
	    if (populate)
		make_site_monster (i, j, INNER_DEMON);
	    break;		// inner circle demon
	case 3:
	    if (populate)
		make_site_monster (i, j, ANGEL);
	    break;		// angel of apropriate sect
	case 4:
	    if (populate)
		make_site_monster (i, j, HIGH_ANGEL);
	    break;		// high angel of apropriate sect
	case 5:
	    if (populate)
		make_site_monster (i, j, ARCHANGEL);
	    break;		// archangel of apropriate sect
    }
}

static void make_high_priest (int i, int j, int deity)
{
    pml ml = ((pml) checkmalloc (sizeof (mltype)));
    pmt m = ((pmt) checkmalloc (sizeof (montype)));
    make_hiscore_npc (m, deity);
    m->x = i;
    m->y = j;
    Level->site[i][j].creature = m;
    ml->m = m;
    ml->next = Level->mlist;
    Level->mlist = ml;
}

void l_merc_guild (void)
{
    pob newitem;

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
	if (Player.rank[LEGION] > 0) {
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
		    if (Player.rank[ARENA] > 0) {
			print1 ("The Centurion checks your record, and gets angry:");
			print2 ("'The legion don't need any Arena Jocks. Git!'");
		    } else if (Player.rank[ORDER] > 0) {
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
			newitem = ((pob) checkmalloc (sizeof (objtype)));
			*newitem = Objects[WEAPONID + 1];	// shortsword
			gain_item (newitem);
			newitem = ((pob) checkmalloc (sizeof (objtype)));
			*newitem = Objects[ARMORID + 1];	// leather
			gain_item (newitem);
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
		    strcpy (Commandant, Player.name);
		    Commandantlevel = Player.level;
		    morewait();
		    Commandantbehavior = fixnpc (4);
		    save_hiscore_npc (8);
		    clearmsg();
		    print1 ("You now know the Spell of Regeneration.");
		    Spells[S_REGENERATE].known = TRUE;
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
		    Spells[S_HERO].known = TRUE;
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
    pob o;
    int x, y;

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
	if (Player.rank[NOBILITY] == 0) {
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
	    if (find_and_remove_item (WEAPONID + 34, -1)) {
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
	    if (find_and_remove_item (ARMORID + 12, -1)) {
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
	    if (find_item (&o, ARTIFACTID + 0, -1)) {
		print1 ("My sincerest thanks, my lord.");
		print2 ("You have proved yourself a true paragon of chivalry");
		morewait();
		print1 ("I abdicate the Duchy in your favor....");
		print2 ("Oh, you can keep the Orb, by the way....");
		Player.rank[NOBILITY] = DUKE;
		gain_experience (10000);
		strcpy (Duke, Player.name);
		morewait();
		Dukebehavior = fixnpc (4);
		save_hiscore_npc (12);
		for (y = 52; y < 63; y++)
		    for (x = 2; x < 52; x++) {
			if (Level->site[x][y].p_locf == L_TRAP_SIREN) {
			    Level->site[x][y].p_locf = L_NO_OP;
			    lset (x, y, CHANGED);
			}
			if (x >= 12 && loc_statusp (x, y, SECRET)) {
			    lreset (x, y, SECRET);
			    lset (x, y, CHANGED);
			}
			if (x >= 20 && x <= 23 && y == 56) {
			    Level->site[x][y].locchar = FLOOR;
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
    pob newitem;
    int i, prize, monsterlevel;

    print1 ("Rampart Coliseum");
    if (Player.rank[ARENA] == 0) {
	print2 ("Enter the games, or Register as a Gladiator? [e,r,ESCAPE] ");
	do
	    response = (char) mcigetc();
	while ((response != 'e') && (response != 'r') && (response != ESCAPE));
    } else {
	print2 ("Enter the games? [yn] ");
	response = ynq2();
	if (response == 'y')
	    response = 'e';
	else
	    response = ESCAPE;
    }
    if (response == 'r') {
	if (Player.rank[ARENA] > 0)
	    print2 ("You're already a gladiator....");
	else if (Player.rank[ORDER] > 0)
	    print2 ("We don't let Paladins into our Guild.");
	else if (Player.rank[LEGION] > 0)
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
	    newitem = ((pob) checkmalloc (sizeof (objtype)));
	    *newitem = Objects[WEAPONID + 17];	// club
	    gain_item (newitem);
	    newitem = ((pob) checkmalloc (sizeof (objtype)));
	    *newitem = Objects[SHIELDID + 2];	// shield
	    gain_item (newitem);
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
	Arena_Monster = ((pmt) checkmalloc (sizeof (montype)));
	Arena_Victory = FALSE;
	switch (Arena_Opponent) {
	    case 0:
		*Arena_Monster = Monsters[GEEK];
		break;
	    case 1:
		*Arena_Monster = Monsters[HORNET];
		break;
	    case 2:
		*Arena_Monster = Monsters[HYENA];
		break;
	    case 3:
		*Arena_Monster = Monsters[GOBLIN];
		break;
	    case 4:
		*Arena_Monster = Monsters[GRUNT];
		break;
	    case 5:
		*Arena_Monster = Monsters[TOVE];
		break;
	    case 6:
		*Arena_Monster = Monsters[APPR_NINJA];
		break;
	    case 7:
		*Arena_Monster = Monsters[SALAMANDER];
		break;
	    case 8:
		*Arena_Monster = Monsters[ANT];
		break;
	    case 9:
		*Arena_Monster = Monsters[MANTICORE];
		break;
	    case 10:
		*Arena_Monster = Monsters[SPECTRE];
		break;
	    case 11:
		*Arena_Monster = Monsters[BANDERSNATCH];
		break;
	    case 12:
		*Arena_Monster = Monsters[LICHE];
		break;
	    case 13:
		*Arena_Monster = Monsters[AUTO_MAJOR];
		break;
	    case 14:
		*Arena_Monster = Monsters[JABBERWOCK];
		break;
	    case 15:
		*Arena_Monster = Monsters[JOTUN];
		break;
	    default:
		if ((Player.rank[ARENA] < 5) && (Player.rank[ARENA] > 0)) {
		    strcpy (Str1, Champion);
		    strcat (Str1, ", the arena champion");
		    *Arena_Monster = Monsters[HISCORE_NPC];
		    Arena_Monster->monstring = salloc (Str1);
		    strcpy (Str2, "The corpse of ");
		    strcat (Str2, Str1);
		    Arena_Monster->corpsestr = salloc (Str2);
		    Arena_Monster->level = 20;
		    Arena_Monster->hp = Championlevel * Championlevel * 5;
		    Arena_Monster->hit = Championlevel * 4;
		    Arena_Monster->ac = Championlevel * 3;
		    Arena_Monster->dmg = 100 + Championlevel * 2;
		    Arena_Monster->xpv = Championlevel * Championlevel * 5;
		    Arena_Monster->speed = 3;
		    Arena_Monster->meleestr = m_melee_str(Championlevel/5);
		    m_status_set (Arena_Monster, MOBILE);
		    m_status_set (Arena_Monster, HOSTILE);
		} else {
		    do
			i = random_range (ML9 - ML0) + ML0;
		    while (i == NPC || i == HISCORE_NPC || i == ZERO_NPC || (Monsters[i].uniqueness != COMMON) || (Monsters[i].dmg == 0));
		    *Arena_Monster = Monsters[i];
		}
		break;
	}
	monsterlevel = Arena_Monster->level;
	if (Arena_Monster->level != 20) {
	    strcpy (Str1, nameprint());
	    strcat (Str1, " the ");
	    strcat (Str1, Arena_Monster->monstring);
	    Arena_Monster->monstring = salloc (Str1);
	    strcpy (Str2, "The corpse of ");
	    strcat (Str2, Str1);
	    Arena_Monster->corpsestr = salloc (Str2);
	}
	Arena_Monster->uniqueness = UNIQUE_MADE;
	print1 ("You have a challenger: ");
	print2 (Arena_Monster->monstring);
	Arena_Monster->attacked = TRUE;
	m_status_set (Arena_Monster, HOSTILE);
	morewait();
	clearmsg();
	change_environment (E_ARENA);
	print1 ("Let the battle begin....");

	time_clock (TRUE);
	while (Current_Environment == E_ARENA)
	    time_clock (FALSE);

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
	// free(corpse);

	if (!Arena_Victory) {
	    print1 ("The crowd boos your craven behavior!!!");
	    if (Player.rank[ARENA] > 0) {
		print2 ("You are thrown out of the Gladiator's Guild!");
		morewait();
		clearmsg();
		if (Gymcredit > 0)
		    print1 ("Your credit at the gym is cut off!");
		Gymcredit = 0;
		Player.rank[ARENA] = -1;
	    }
	} else {
	    Arena_Opponent++;
	    if (monsterlevel == 20) {
		print1 ("The crowd roars its approval!");
		if (Player.rank[ARENA]) {
		    print2 ("You are the new Arena Champion!");
		    Championlevel = Player.level;
		    strcpy (Champion, Player.name);
		    Player.rank[ARENA] = 5;
		    morewait();
		    Championbehavior = fixnpc (4);
		    save_hiscore_npc (11);
		    print1 ("You are awarded the Champion's Spear: Victrix!");
		    morewait();
		    newitem = ((pob) checkmalloc (sizeof (objtype)));
		    *newitem = Objects[WEAPONID + 35];
		    gain_item (newitem);

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
	    prize = max (25, monsterlevel * 50);
	    if (Player.rank[ARENA] > 0)
		prize *= 2;
	    mnumprint (prize);
	    nprint1 ("Au.");
	    Player.cash += prize;
	    if ((Player.rank[ARENA] < 4) && (Arena_Opponent > 5) && (Arena_Opponent % 3 == 0)) {
		if (Player.rank[ARENA] > 0) {
		    Player.rank[ARENA]++;
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
    int fee, count, i, number, done = FALSE, dues = 1000;
    char c, action;
    pob lockpick;
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
	    strcpy (Shadowlord, Player.name);
	    Shadowlordlevel = Player.level;
	    morewait();
	    Shadowlordbehavior = fixnpc (4);
	    save_hiscore_npc (7);
	    clearmsg();
	    print1 ("You learn the Spell of Shadowform.");
	    Spells[S_SHADOWFORM].known = TRUE;
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
	    if (Player.rank[THIEVES] == 0)
		menuprint ("a: Join the Thieves' Guild.\n");
	    else
		menuprint ("b: Raise your Guild rank.\n");
	    menuprint ("c: Get an item identified.\n");
	    if (Player.rank[THIEVES] > 0)
		menuprint ("d: Fence an item.\n");
	    menuprint ("ESCAPE: Leave this Den of Iniquity.");
	    showmenu();
	    action = mgetc();
	    if (action == ESCAPE)
		done = TRUE;
	    else if (action == 'a') {
		done = TRUE;
		if (Player.rank[THIEVES] > 0)
		    print2 ("You are already a member!");
		else if (Player.alignment > 10)
		    print2 ("You are too lawful to be a thief!");
		else {
		    dues += dues * (12 - Player.dex) / 9;
		    dues += Player.alignment * 5;
		    dues = max (100, dues);
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
			    Spells[S_OBJ_DET].known = TRUE;
			    lockpick = ((pob) checkmalloc (sizeof (objtype)));
			    *lockpick = Objects[THING_LOCKPICK];
			    gain_item (lockpick);
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
		if (Player.rank[THIEVES] == 0)
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
			Spells[S_APPORT].known = TRUE;
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
			Spells[S_INVISIBLE].known = TRUE;
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
			Spells[S_LEVITATE].known = TRUE;
			Player.rank[THIEVES] = ATHIEF;
			Player.dex++;
			Player.maxdex++;
		    }
		}
	    } else if (action == 'c') {
		if (Player.rank[THIEVES] == 0) {
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
		    for (i = 1; i < MAXITEMS; i++)
			if (Player.possessions[i] != NULL)
			    if (Player.possessions[i]->known < 2)
				count++;
		    for (i = 0; i < Player.packptr; i++)
			if (Player.pack[i] != NULL)
			    if (Player.pack[i]->known < 2)
				count++;
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
		if (Player.rank[THIEVES] == 0)
		    print2 ("Fence? Who said anything about a fence?");
		else {
		    print1 ("Fence one item or go through pack? [ip] ");
		    if ((char) mcigetc() == 'i') {
			i = getitem (NULL_ITEM);
			if ((i == ABORT) || (Player.possessions[i] == NULL))
			    print2 ("Huh, Is this some kind of set-up?");
			else if (Player.possessions[i]->blessing < 0)
			    print2 ("I don't want to buy a cursed item!");
			else {
			    clearmsg();
			    print1 ("I'll give you ");
			    mlongprint (2 * item_value (Player.possessions[i]) / 3);
			    nprint1 ("Au each. OK? [yn] ");
			    if (ynq1() == 'y') {
				number = getnumber (Player.possessions[i]->number);
				if ((number >= Player.possessions[i]->number) && Player.possessions[i]->used) {
				    Player.possessions[i]->used = FALSE;
				    item_use (Player.possessions[i]);
				}
				Player.cash += number * 2 * item_value (Player.possessions[i]) / 3;
				// Fenced artifacts could turn up anywhere, really...
				if (Objects[Player.possessions[i]->id].uniqueness > UNIQUE_UNMADE)
				    Objects[Player.possessions[i]->id].uniqueness = UNIQUE_UNMADE;
				dispose_lost_objects (number, Player.possessions[i]);
				dataprint();
			    } else
				print2 ("Hey, gimme a break, it was a fair price!");
			}
		    } else {
			for (i = 0; i < Player.packptr; i++) {
			    if (Player.pack[i]->blessing > -1) {
				clearmsg();
				print1 ("Sell ");
				nprint1 (itemid (Player.pack[i]));
				nprint1 (" for ");
				mlongprint (2 * item_value (Player.pack[i]) / 3);
				nprint1 ("Au each? [ynq] ");
				if ((c = ynq1()) == 'y') {
				    number = getnumber (Player.pack[i]->number);
				    Player.cash += 2 * number * item_value (Player.pack[i]) / 3;
				    Player.pack[i]->number -= number;
				    if (Player.pack[i]->number < 1) {
					// Fenced an artifact?  You just might see it again.
					if (Objects[Player.pack[i]->id].uniqueness > UNIQUE_UNMADE)
					    Objects[Player.pack[i]->id].uniqueness = UNIQUE_UNMADE;
					free (Player.pack[i]);
					Player.pack[i] = NULL;
				    }
				    dataprint();
				} else if (c == 'q')
				    break;
			    }
			}
			fixpack();
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
    int done = FALSE, enrolled = FALSE;
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
		strcpy (Archmage, Player.name);
		Archmagelevel = Player.level;
		Player.rank[COLLEGE] = ARCHMAGE;
		Player.maxiq += 5;
		Player.iq += 5;
		Player.maxpow += 5;
		Player.pow += 5;
		morewait();
		Archmagebehavior = fixnpc (4);
		save_hiscore_npc (9);
	    }
	    menuclear();
	    menuprint ("May we help you?\n\n");
	    menuprint ("a: Enroll in the College.\n");
	    menuprint ("b: Raise your College rank.\n");
	    menuprint ("c: Do spell research.\n");
	    menuprint ("ESCAPE: Leave these hallowed halls.\n");
	    showmenu();
	    action = mgetc();
	    if (action == ESCAPE)
		done = TRUE;
	    else if (action == 'a') {
		if (Player.rank[COLLEGE] > 0)
		    print2 ("You are already enrolled!");
		else if (Player.iq < 13)
		    print2 ("Your low IQ renders you incapable of being educated.");
		else if (Player.rank[CIRCLE] > 0)
		    print2 ("Sorcery and our Magic are rather incompatable, no?");
		else {
		    if (Player.iq > 17) {
			print2 ("You are given a scholarship!");
			morewait();
			enrolled = TRUE;
		    } else {
			print1 ("Tuition is 1000Au. ");
			nprint1 ("Pay it? [yn] ");
			if (ynq1() == 'y') {
			    if (Player.cash < 1000)
				print2 ("You don't have the funds!");
			    else {
				Player.cash -= 1000;
				enrolled = TRUE;
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
		if (Player.rank[COLLEGE] == 0)
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
			Spells[S_RITUAL].known = TRUE;
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
			Spells[S_IDENTIFY].known = TRUE;
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
    int done = FALSE, fee = 3000;
    long total;
    print1 ("The Circle of Sorcerors.");
    if (Player.rank[CIRCLE] == -1) {
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
		strcpy (Prime, Player.name);
		Primelevel = Player.level;
		morewait();
		Primebehavior = fixnpc (4);
		save_hiscore_npc (10);
		clearmsg();
		print1 ("You learn the Spell of Disintegration!");
		morewait();
		clearmsg();
		Spells[S_DISINTEGRATE].known = TRUE;
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
	    if (action == ESCAPE)
		done = TRUE;
	    else if (action == 'a') {
		if (Player.rank[CIRCLE] > 0)
		    print2 ("You are already an initiate!");
		else if (Player.alignment > 0)
		    print2 ("You may not join -- you reek of Law!");
		else if (Player.rank[COLLEGE] != 0)
		    print2 ("Foolish Mage!  You don't have the right attitude to Power!");
		else {
		    fee += Player.alignment * 100;
		    fee += fee * (12 - Player.pow) / 9;
		    fee = max (100, fee);
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
			    Spells[S_MISSILE].known = TRUE;
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
		if (Player.rank[CIRCLE] == 0)
		    print2 ("You have not even been initiated, yet!");
		else if (Player.alignment > -1) {
		    print1 ("Ahh! You have grown too lawful!!!");
		    print2 ("You are hereby blackballed from the Circle!");
		    Player.rank[CIRCLE] = -1;
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
		    done = TRUE;
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
			Spells[S_DISRUPT].known = TRUE;
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
			Spells[S_LBALL].known = TRUE;
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
			Spells[S_FIREBOLT].known = TRUE;
			Player.rank[CIRCLE] = ENCHANTER;
			Player.maxpow += 2;
			Player.pow += 2;
		    }
		}
	    } else if (action == 'c') {
		done = TRUE;
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
    pob newitem;
    pml ml;
    print1 ("The Headquarters of the Order of Paladins.");
    morewait();
    if ((Player.rank[ORDER] == PALADIN) && (Player.level > Justiciarlevel) && gamestatusp (GAVE_STARGEM) && Player.alignment > 300) {
	print1 ("You have succeeded in your quest!");
	morewait();
	print1 ("The previous Justiciar steps down in your favor.");
	print2 ("You are now the Justiciar of Rampart and the Order!");
	strcpy (Justiciar, Player.name);
	for (ml = Level->mlist; ml && (ml->m->id != HISCORE_NPC || ml->m->aux2 != 15); ml = ml->next)
	    // just scan for current Justicar */ ;
	if (ml) {
	    Level->site[ml->m->x][ml->m->y].creature = NULL;
	    erase_monster (ml->m);
	    ml->m->hp = -1;	// signals "death" -- no credit to player, though
	}
	Justiciarlevel = Player.level;
	morewait();
	Justiciarbehavior = fixnpc (4);
	save_hiscore_npc (15);
	clearmsg();
	print1 ("You are awarded a blessed shield of deflection!");
	morewait();
	newitem = ((pob) checkmalloc (sizeof (objtype)));
	*newitem = Objects[SHIELDID + 7];	// shield of deflection
	newitem->blessing = 9;
	gain_item (newitem);
	morewait();
	Player.rank[ORDER] = JUSTICIAR;
	Player.maxstr += 5;
	Player.str += 5;
	Player.maxpow += 5;
	Player.pow += 5;
    }
    if (Player.alignment < 1) {
	if (Player.rank[ORDER] > 0) {
	    print1 ("You have been tainted by chaos!");
	    print2 ("You are stripped of your rank in the Order!");
	    morewait();
	    Player.rank[ORDER] = -1;
	    send_to_jail();
	} else
	    print1 ("Get thee hence, minion of chaos!");
    } else if (Player.rank[ORDER] == -1)
	print1 ("Thee again?  Get thee hence, minion of chaos!");
    else if (Player.rank[ORDER] == 0) {
	if (Player.rank[ARENA] != 0)
	    print1 ("We do not accept bloodstained gladiators into our Order.");
	else if (Player.rank[LEGION] != 0)
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
		newitem = ((pob) checkmalloc (sizeof (objtype)));
		*newitem = Objects[WEAPONID + 19];	// spear
		newitem->blessing = 9;
		newitem->plus = 1;
		newitem->known = 2;
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
		newitem = ((pob) checkmalloc (sizeof (objtype)));
		*newitem = Objects[ARMORID + 11];	// mithril plate armor
		newitem->blessing = 9;
		newitem->known = 2;
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
		Spells[S_HERO].known = TRUE;
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
		newitem = ((pob) checkmalloc (sizeof (objtype)));
		*newitem = Objects[WEAPONID + 25];	// mace of disruption
		newitem->known = 2;
		gain_item (newitem);
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
		newitem = ((pob) checkmalloc (sizeof (objtype)));
		*newitem = Objects[ARTIFACTID + 7];	// holy hand grenade.
		newitem->known = 2;
		gain_item (newitem);
	    }
	}
    }
}

// loads the house level into Level
void load_house (int kind, int populate)
{
    int i, j;
    char site;
    int stops;

    FILE *fd;

    TempLevel = Level;
    initrand (Current_Environment, Player.x + Player.y + hour() * 10);
    if (ok_to_free (TempLevel)) {
	free_level (TempLevel);
	TempLevel = NULL;
    }
    Level = ((plv) checkmalloc (sizeof (levtype)));
    clear_level (Level);
    strcpy (Str3, Omegalib);
    switch (kind) {
	case E_HOUSE:
	    strcat (Str3, "home1.dat");
	    Level->environment = E_HOUSE;
	    site = cryptkey ("home1.dat");
	    break;
	case E_MANSION:
	    strcat (Str3, "home2.dat");
	    Level->environment = E_MANSION;
	    site = cryptkey ("home2.dat");
	    break;
	default:
	case E_HOVEL:
	    strcat (Str3, "home3.dat");
	    Level->environment = E_HOVEL;
	    site = cryptkey ("home3.dat");
	    break;
    }
    fd = checkfopen (Str3, "rb");
    stops = 0;
    for (j = 0; j < LENGTH; j++) {
	for (i = 0; i < WIDTH; i++) {
	    if (kind == E_HOVEL)
		Level->site[i][j].lstatus = SEEN;
	    else
		Level->site[i][j].lstatus = 0;
	    Level->site[i][j].roomnumber = RS_CORRIDOR;
	    Level->site[i][j].p_locf = L_NO_OP;
	    site = getc (fd) ^ site;
	    switch (site) {
		case 'N':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].roomnumber = RS_BEDROOM;
		    if (random_range (2) && populate)
			make_house_npc (i, j);
		    break;
		case 'H':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].roomnumber = RS_BEDROOM;
		    if (random_range (2) && populate)
			make_mansion_npc (i, j);
		    break;
		case 'D':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].roomnumber = RS_DININGROOM;
		    break;
		case '.':
		    Level->site[i][j].locchar = FLOOR;
		    if (stops) {
			lset (i, j, STOPS);
			stops = 0;
		    }
		    break;
		case 'c':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].roomnumber = RS_CLOSET;
		    break;
		case 'G':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].roomnumber = RS_BATHROOM;
		    break;
		case 'B':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].roomnumber = RS_BEDROOM;
		    break;
		case 'K':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].roomnumber = RS_KITCHEN;
		    break;
		case 'S':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].showchar = WALL;
		    lset (i, j, SECRET);
		    Level->site[i][j].roomnumber = RS_SECRETPASSAGE;
		    break;
		case '3':
		    Level->site[i][j].locchar = SAFE;
		    Level->site[i][j].showchar = WALL;
		    lset (i, j, SECRET);
		    Level->site[i][j].p_locf = L_SAFE;
		    break;
		case '^':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = TRAP_BASE + random_range (NUMTRAPS);
		    break;
		case 'P':
		    Level->site[i][j].locchar = PORTCULLIS;
		    Level->site[i][j].p_locf = L_PORTCULLIS;
		    break;
		case 'R':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_RAISE_PORTCULLIS;
		    break;
		case 'p':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_PORTCULLIS;
		    break;
		case 'T':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_PORTCULLIS_TRAP;
		    break;
		case 'X':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_HOUSE_EXIT;
		    stops = 1;
		    break;
		case '#':
		    Level->site[i][j].locchar = WALL;
		    switch (kind) {
			case E_HOVEL:
			    Level->site[i][j].aux = 10;
			    break;
			case E_HOUSE:
			    Level->site[i][j].aux = 50;
			    break;
			case E_MANSION:
			    Level->site[i][j].aux = 150;
			    break;
		    }
		    break;
		case '|':
		    Level->site[i][j].locchar = OPEN_DOOR;
		    Level->site[i][j].roomnumber = RS_CORRIDOR;
		    lset (i, j, STOPS);
		    break;
		case '+':
		    Level->site[i][j].locchar = CLOSED_DOOR;
		    Level->site[i][j].roomnumber = RS_CORRIDOR;
		    Level->site[i][j].aux = LOCKED;
		    lset (i, j, STOPS);
		    break;
		case 'd':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].roomnumber = RS_CORRIDOR;
		    if (populate)
			make_site_monster (i, j, DOBERMAN);
		    break;
		case 'a':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].roomnumber = RS_CORRIDOR;
		    Level->site[i][j].p_locf = L_TRAP_SIREN;
		    break;
		case 'A':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].roomnumber = RS_CORRIDOR;
		    if (populate)
			make_site_monster (i, j, AUTO_MINOR);	// automaton
		    break;
	    }
	    Level->site[i][j].showchar = ' ';
	}
	site = getc (fd) ^ site;
    }
    fclose (fd);
    initrand (E_RESTORE, 0);
}

// makes a log npc for houses and hovels
static void make_house_npc (int i, int j)
{
    pml ml = ((pml) checkmalloc (sizeof (mltype)));
    pob ob;
    ml->m = ((pmt) checkmalloc (sizeof (montype)));
    *(ml->m) = Monsters[NPC];
    make_log_npc (ml->m);
    if (ml->m->id == NPC)
	mprint ("You detect signs of life in this house.");
    else
	mprint ("An eerie shiver runs down your spine as you enter....");
    // if not == NPC, then we got a ghost off the npc list
    ml->m->x = i;
    ml->m->y = j;
    Level->site[i][j].creature = ml->m;
    ml->m->click = (Tick + 1) % 50;
    ml->next = Level->mlist;
    Level->mlist = ml;
    m_status_set (ml->m, HOSTILE);
    if (nighttime())
	m_status_reset (ml->m, AWAKE);
    else
	m_status_set (ml->m, AWAKE);
    if (ml->m->startthing > -1) {
	ob = ((pob) checkmalloc (sizeof (objtype)));
	*ob = Objects[ml->m->startthing];
	m_pickup (ml->m, ob);
    }
}

// makes a hiscore npc for mansions
static void make_mansion_npc (int i, int j)
{
    pml ml = ((pml) checkmalloc (sizeof (mltype)));
    ml->m = ((pmt) checkmalloc (sizeof (montype)));
    *(ml->m) = Monsters[NPC];
    make_hiscore_npc (ml->m, random_range (14) + 1);
    mprint ("You detect signs of life in this house.");
    ml->m->x = i;
    ml->m->y = j;
    Level->site[i][j].creature = ml->m;
    ml->m->click = (Tick + 1) % 50;
    ml->next = Level->mlist;
    Level->mlist = ml;
    m_status_set (ml->m, HOSTILE);
    if (nighttime())
	m_status_reset (ml->m, AWAKE);
    else
	m_status_set (ml->m, AWAKE);
}

// loads the village level into Level
void load_village (int villagenum, int populate)
{
    int i, j;
    char site;

    FILE *fd;

    TempLevel = Level;
    if (ok_to_free (TempLevel)) {
	free_level (TempLevel);
	TempLevel = NULL;
    }

    initrand (Current_Environment, villagenum);

    assign_village_function (0, 0, TRUE);

    Level = ((plv) checkmalloc (sizeof (levtype)));
    clear_level (Level);
    Level->environment = E_VILLAGE;
    strcpy (Str3, Omegalib);
    switch (villagenum) {
	case 1:
	    strcat (Str3, "village1.dat");
	    break;
	case 2:
	    strcat (Str3, "village2.dat");
	    break;
	case 3:
	    strcat (Str3, "village3.dat");
	    break;
	case 4:
	    strcat (Str3, "village4.dat");
	    break;
	case 5:
	    strcat (Str3, "village5.dat");
	    break;
	case 6:
	    strcat (Str3, "village6.dat");
	    break;
    }
    site = cryptkey ("village.dat");
    fd = checkfopen (Str3, "rb");
    for (j = 0; j < LENGTH; j++) {
	for (i = 0; i < WIDTH; i++) {
	    lset (i, j, SEEN);
	    site = getc (fd) ^ site;
	    Level->site[i][j].p_locf = L_NO_OP;
	    switch (site) {
		case 'f':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate)
			make_food_bin (i, j);
		    break;
		case 'g':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_GRANARY;
		    break;
		case 'h':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate)
			make_horse (i, j);
		    break;
		case 'S':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_STABLES;
		    break;
		case 'H':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate)
			make_merchant (i, j);
		    break;
		case 'C':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_COMMONS;
		    break;
		case 's':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate)
			make_sheep (i, j);
		    break;
		case 'x':
		    assign_village_function (i, j, FALSE);
		    break;
		case 'X':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_COUNTRYSIDE;
		    break;
		case 'G':
		    Level->site[i][j].locchar = FLOOR;
		    if (populate) {
			make_guard (i, j);
			Level->site[i][j].creature->aux1 = i;
			Level->site[i][j].creature->aux2 = j;
		    }
		    break;
		case '^':
		    Level->site[i][j].locchar = FLOOR;
		    Level->site[i][j].p_locf = L_TRAP_SIREN;
		    break;
		case '"':
		    Level->site[i][j].locchar = HEDGE;
		    Level->site[i][j].p_locf = L_HEDGE;
		    break;
		case '~':
		    Level->site[i][j].locchar = WATER;
		    Level->site[i][j].p_locf = L_WATER;
		    break;
		case '+':
		    Level->site[i][j].locchar = WATER;
		    Level->site[i][j].p_locf = L_CHAOS;
		    break;
		case '\'':
		    Level->site[i][j].locchar = HEDGE;
		    Level->site[i][j].p_locf = L_TRIFID;
		    break;
		case '!':
		    special_village_site (i, j, villagenum);
		    break;
		case '#':
		    Level->site[i][j].locchar = WALL;
		    Level->site[i][j].aux = 100;
		    break;
		case '.':
		    Level->site[i][j].locchar = FLOOR;
		    break;
		case '-':
		    Level->site[i][j].locchar = CLOSED_DOOR;
		    break;
		case '1':
		    Level->site[i][j].locchar = STATUE;
		    break;
	    }
	    if (loc_statusp (i, j, SECRET))
		Level->site[i][j].showchar = WALL;
	    else
		Level->site[i][j].showchar = Level->site[i][j].locchar;
	}
	site = getc (fd) ^ site;
    }
    fclose (fd);
    initrand (E_RESTORE, 0);
}

static void make_guard (int i, int j)
{
    pml tml = ((pml) (checkmalloc (sizeof (mltype))));
    tml->m = (Level->site[i][j].creature = make_creature (GUARD));
    tml->m->x = i;
    tml->m->y = j;
    tml->next = Level->mlist;
    Level->mlist = tml;
}

static void make_sheep (int i, int j)
{
    pml tml = ((pml) (checkmalloc (sizeof (mltype))));
    tml->m = (Level->site[i][j].creature = make_creature (SHEEP));
    tml->m->x = i;
    tml->m->y = j;
    tml->next = Level->mlist;
    Level->mlist = tml;
}

static void make_food_bin (int i, int j)
{
    pol tol;
    int k;

    for (k = 0; k < 10; k++) {
	tol = ((pol) checkmalloc (sizeof (oltype)));
	tol->thing = ((pob) checkmalloc (sizeof (objtype)));
	make_food (tol->thing, 15);	// grain
	tol->next = Level->site[i][j].things;
	Level->site[i][j].things = tol;
    }
}

static void make_horse (int i, int j)
{
    pml tml = ((pml) (checkmalloc (sizeof (mltype))));
    tml->m = (Level->site[i][j].creature = make_creature (HORSE));
    tml->m->x = i;
    tml->m->y = j;
    tml->next = Level->mlist;
    Level->mlist = tml;
}

static void make_merchant (int i, int j)
{
    pml tml = ((pml) (checkmalloc (sizeof (mltype))));
    tml->m = (Level->site[i][j].creature = make_creature (MERCHANT));
    tml->m->x = i;
    tml->m->y = j;
    tml->next = Level->mlist;
    Level->mlist = tml;
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
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_ARMORER;
		break;
	    case 1:
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_HEALER;
		break;
	    case 2:
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_TAVERN;
		break;
	    case 3:
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_COMMANDANT;
		break;
	    case 4:
		Level->site[x][y].locchar = OPEN_DOOR;
		Level->site[x][y].p_locf = L_CARTOGRAPHER;
		break;
	    default:
		Level->site[x][y].locchar = CLOSED_DOOR;
		if (random_range (2))
		    Level->site[x][y].aux = LOCKED;
		if (random_range (2))
		    Level->site[x][y].p_locf = L_HOVEL;
		else
		    Level->site[x][y].p_locf = L_HOUSE;
		break;
	}
    }
}

static void special_village_site (int i, int j, int villagenum)
{
    if (villagenum == 1) {
	Level->site[i][j].locchar = ALTAR;
	Level->site[i][j].p_locf = L_LAWSTONE;
    }
    if (villagenum == 2) {
	Level->site[i][j].locchar = ALTAR;
	Level->site[i][j].p_locf = L_BALANCESTONE;
    } else if (villagenum == 3) {
	Level->site[i][j].locchar = ALTAR;
	Level->site[i][j].p_locf = L_CHAOSTONE;
    } else if (villagenum == 4) {
	Level->site[i][j].locchar = ALTAR;
	Level->site[i][j].p_locf = L_MINDSTONE;
    } else if (villagenum == 5) {
	Level->site[i][j].locchar = ALTAR;
	Level->site[i][j].p_locf = L_SACRIFICESTONE;
    } else if (villagenum == 6) {
	Level->site[i][j].locchar = ALTAR;
	Level->site[i][j].p_locf = L_VOIDSTONE;
    }
}

// Functions dealing with dungeon and country levels aside from actual
// level structure generation

// monsters for tactical encounters
void make_country_monsters (int terrain)
{
    pml tml, ml = NULL;
    static int plains[10] = { BUNNY, BUNNY, HORNET, QUAIL, HAWK, DEER, WOLF, LION, BRIGAND, RANDOM };
//    {BUNNY,BUNNY,BLACKSNAKE,HAWK,IMPALA,WOLF,LION,BRIGAND,RANDOM};
    // DG changed (WDT: I'd like to see a blacksnake).
    static int forest[10] = { BUNNY, QUAIL, HAWK, BADGER, DEER, DEER, WOLF, BEAR, BRIGAND, RANDOM };
    static int jungle[10] = { ANTEATER, PARROT, MAMBA, ANT, ANT, HYENA, HYENA, ELEPHANT, LION, RANDOM };
    static int river[10] = { QUAIL, TROUT, TROUT, MANOWAR, BASS, BASS, CROC, CROC, BRIGAND, RANDOM };
    static int swamp[10] = { BASS, BASS, CROC, CROC, BOGTHING, ANT, ANT, RANDOM, RANDOM, RANDOM };
    static int desert[10] = { HAWK, HAWK, CAMEL, CAMEL, HYENA, HYENA, LION, LION, RANDOM, RANDOM };
    static int tundra[10] = { WOLF, WOLF, BEAR, BEAR, DEER, DEER, RANDOM, RANDOM, RANDOM, RANDOM };
    static int mountain[10] = { BUNNY, SHEEP, WOLF, WOLF, HAWK, HAWK, HAWK, RANDOM, RANDOM, RANDOM };
    int *monsters, i, nummonsters;

    nummonsters = (random_range (5) + 1) * (random_range (3) + 1);

    switch (terrain) {
	case PLAINS:
	    monsters = plains;
	    break;
	case FOREST:
	    monsters = forest;
	    break;
	case JUNGLE:
	    monsters = jungle;
	    break;
	case RIVER:
	    monsters = river;
	    break;
	case SWAMP:
	    monsters = swamp;
	    break;
	case MOUNTAINS:
	case PASS:
	case VOLCANO:
	    monsters = mountain;
	    break;
	case DESERT:
	    monsters = desert;
	    break;
	case TUNDRA:
	    monsters = tundra;
	    break;
	default:
	    monsters = NULL;
    }
    for (i = 0; i < nummonsters; i++) {
	tml = ((pml) checkmalloc (sizeof (mltype)));
	tml->m = ((pmt) checkmalloc (sizeof (montype)));
	if (monsters == NULL)
	    tml->m = m_create (random_range (WIDTH), random_range (LENGTH), TRUE, difficulty());
	else {
	    tml->m = make_creature (*(monsters + random_range (10)));
	    tml->m->x = random_range (WIDTH);
	    tml->m->y = random_range (LENGTH);
	}
	Level->site[tml->m->x][tml->m->y].creature = tml->m;
	tml->m->sense = WIDTH;
	if (m_statusp (tml->m, ONLYSWIM)) {
	    Level->site[tml->m->x][tml->m->y].locchar = WATER;
	    Level->site[tml->m->x][tml->m->y].p_locf = L_WATER;
	    lset (tml->m->x, tml->m->y, CHANGED);
	}

	tml->next = ml;
	ml = tml;
    }
    Level->mlist = ml;
}

// monstertype is more or less Current_Dungeon
// The caves and sewers get harder as you penetrate them; the castle
// is completely random, but also gets harder as it is explored;
// the astral and the volcano just stay hard...
void populate_level (int monstertype)
{
    pml head, tml;
    int i, j, k, monsterid = RANDOM, nummonsters = (random_range (difficulty() / 3) + 1) * 3 + 8;

    if (monstertype == E_CASTLE)
	nummonsters += 10;
    else if (monstertype == E_ASTRAL)
	nummonsters += 10;
    else if (monstertype == E_VOLCANO)
	nummonsters += 20;

    head = tml = ((pml) checkmalloc (sizeof (mltype)));

    for (k = 0; k < nummonsters; k++) {

	findspace (&i, &j, -1);

	switch (monstertype) {
	    case E_CAVES:
		if (Level->depth * 10 + random_range (100) > 150)
		    monsterid = GOBLIN_SHAMAN;
		else if (Level->depth * 10 + random_range (100) > 100)
		    monsterid = GOBLIN_CHIEF;	// Goblin Chieftain
		else if (random_range (100) > 50)
		    monsterid = GOBLIN;
		else
		    monsterid = RANDOM;	// IE, random monster
		break;
	    case E_SEWERS:
		if (!random_range (3))
		    monsterid = -1;
		else
		    switch (random_range (Level->depth + 3)) {
			case 0:
			    monsterid = SEWER_RAT;
			    break;
			case 1:
			    monsterid = AGGRAVATOR;
			    break;	// aggravator fungus
			case 2:
			    monsterid = BLIPPER;
			    break;	// blipper rat
			case 3:
			    monsterid = NIGHT_GAUNT;
			    break;
			case 4:
			    monsterid = NASTY;
			    break;	// transparent nasty
			case 5:
			    monsterid = MURK;
			    break;	// murk fungus
			case 6:
			    monsterid = CATOBLEPAS;
			    break;
			case 7:
			    monsterid = ACID_CLOUD;
			    break;
			case 8:
			    monsterid = DENEBIAN;
			    break;	// denebian slime devil
			case 9:
			    monsterid = CROC;
			    break;	// giant crocodile
			case 10:
			    monsterid = TESLA;
			    break;	// tesla monster
			case 11:
			    monsterid = SHADOW;
			    break;	// shadow spirit
			case 12:
			    monsterid = BOGTHING;
			    break;	// bogthing
			case 13:
			    monsterid = WATER_ELEM;
			    break;	// water elemental
			case 14:
			    monsterid = TRITON;
			    break;
			case 15:
			    monsterid = ROUS;
			    break;
			default:
			    monsterid = RANDOM;
			    break;	// whatever seems good
		    }
		break;
	    case E_ASTRAL:
		if (random_range (2))	// random astral creatures
		    switch (random_range (12)) {
			case 0:
			    monsterid = THOUGHTFORM;
			    break;
			case 1:
			    monsterid = FUZZY;
			    break;	// astral fuzzy
			case 2:
			    monsterid = BAN_SIDHE;
			    break;
			case 3:
			    monsterid = GRUE;
			    break;	// astral grue
			case 4:
			    monsterid = SHADOW;
			    break;	// shadow spirit
			case 5:
			    monsterid = ASTRAL_VAMP;
			    break;	// astral vampire
			case 6:
			    monsterid = MANABURST;
			    break;
			case 7:
			    monsterid = RAKSHASA;
			    break;
			case 8:
			    monsterid = ILL_FIEND;
			    break;	// illusory fiend
			case 9:
			    monsterid = MIRRORMAST;
			    break;	// mirror master
			case 10:
			    monsterid = ELDER_GRUE;
			    break;	// elder etheric grue
			case 11:
			    monsterid = SHADOW_SLAY;
			    break;	// shadow slayer
		} else if (random_range (2) && (Level->depth == 1))	// plane of earth
		    monsterid = EARTH_ELEM;	// earth elemental
		else if (random_range (2) && (Level->depth == 2))	// plane of air
		    monsterid = AIR_ELEM;	// air elemental
		else if (random_range (2) && (Level->depth == 3))	// plane of water
		    monsterid = WATER_ELEM;	// water elemental
		else if (random_range (2) && (Level->depth == 4))	// plane of fire
		    monsterid = FIRE_ELEM;	// fire elemental
		else if (random_range (2) && (Level->depth == 5))	// deep astral
		    switch (random_range (12)) {
			case 0:
			    monsterid = NIGHT_GAUNT;
			    break;
			case 1:
			    monsterid = SERV_LAW;
			    break;	// servant of law
			case 2:
			    monsterid = SERV_CHAOS;
			    break;	// servant of chaos
			case 3:
			    monsterid = FROST_DEMON;
			    break;	// lesser frost demon
			case 4:
			    monsterid = OUTER_DEMON;
			    break;	// outer circle demon
			case 5:
			    monsterid = DEMON_SERP;
			    break;	// demon serpent
			case 6:
			    monsterid = ANGEL;
			    break;
			case 7:
			    monsterid = INNER_DEMON;
			    break;	// inner circle demon
			case 8:
			    monsterid = FDEMON_L;
			    break;	// frost demon lord
			case 9:
			    monsterid = HIGH_ANGEL;
			    break;
			case 10:
			    monsterid = DEMON_PRINCE;
			    break;	// prime circle demon
			case 11:
			    monsterid = ARCHANGEL;
			    break;
		} else
		    monsterid = RANDOM;
		break;
	    case E_VOLCANO:
		if (random_range (2)) {
		    do
			monsterid = random_range (ML10 - ML4) + ML4;
		    while (Monsters[monsterid].uniqueness != COMMON);
		} else
		    switch (random_range (Level->depth / 2 + 2)) {	// evil & fire creatures
			case 0:
			    monsterid = HAUNT;
			    break;
			case 1:
			    monsterid = INCUBUS;
			    break;
			case 2:
			    monsterid = DRAGONETTE;
			    break;
			case 3:
			    monsterid = FROST_DEMON;
			    break;
			case 4:
			    monsterid = SPECTRE;
			    break;
			case 5:
			    monsterid = LAVA_WORM;
			    break;
			case 6:
			    monsterid = FIRE_ELEM;
			    break;
			case 7:
			    monsterid = LICHE;
			    break;
			case 8:
			    monsterid = RAKSHASA;
			    break;
			case 9:
			    monsterid = DEMON_SERP;
			    break;
			case 10:
			    monsterid = NAZGUL;
			    break;
			case 11:
			    monsterid = FLAME_DEV;
			    break;
			case 12:
			    monsterid = LOATHLY;
			    break;
			case 13:
			    monsterid = ZOMBIE;
			    break;
			case 14:
			    monsterid = INNER_DEMON;
			    break;
			case 15:
			    monsterid = BAD_FAIRY;
			    break;
			case 16:
			    monsterid = DRAGON;
			    break;
			case 17:
			    monsterid = FDEMON_L;
			    break;
			case 18:
			    monsterid = SHADOW_SLAY;
			    break;
			case 19:
			    monsterid = DEATHSTAR;
			    break;
			case 20:
			    monsterid = VAMP_LORD;
			    break;
			case 21:
			    monsterid = DEMON_PRINCE;
			    break;
			default:
			    monsterid = RANDOM;
			    break;
		    }
		break;
	    case E_CASTLE:
		if (random_range (4) == 1) {
		    if (difficulty() < 5)
			monsterid = ENCHANTOR;
		    else if (difficulty() < 6)
			monsterid = NECROMANCER;
		    else if (difficulty() < 8)
			monsterid = FIRE_ELEM;
		    else
			monsterid = THAUMATURGIST;
		} else
		    monsterid = RANDOM;
		break;

	    default:
		monsterid = RANDOM;
		break;
	}

	assert (RANDOM == -1);	// WDT: the following test slightly assumes
				// this.
	if (monsterid > RANDOM)
	    Level->site[i][j].creature = make_creature (monsterid);
	else
	    Level->site[i][j].creature = m_create (i, j, TRUE, difficulty());

	Level->site[i][j].creature->x = i;
	Level->site[i][j].creature->y = j;

	if (m_statusp (Level->site[i][j].creature, ONLYSWIM)) {
	    Level->site[i][j].locchar = WATER;
	    Level->site[i][j].p_locf = L_WATER;
	    lset (i, j, CHANGED);
	}

	tml->next = ((pml) checkmalloc (sizeof (mltype)));
	tml->next->m = Level->site[i][j].creature;
	tml = tml->next;
    }

    if (Level->mlist == NULL) {
	tml->next = NULL;
	Level->mlist = head->next;
    } else {
	tml->next = Level->mlist;
	Level->mlist = head->next;
    }
}

// Add a wandering monster possibly
void wandercheck (void)
{
    int x, y;
    pml tml;
    if (random_range (MaxDungeonLevels) < difficulty()) {
	findspace (&x, &y, -1);
	tml = ((pml) checkmalloc (sizeof (mltype)));
	tml->next = Level->mlist;
	tml->m = Level->site[x][y].creature = m_create (x, y, WANDERING, difficulty());
	Level->mlist = tml;
    }
}

// call make_creature and place created monster on Level->mlist and Level
void make_site_monster (int i, int j, int mid)
{
    pml ml = ((pml) checkmalloc (sizeof (mltype)));
    pmt m;
    if (mid > -1)
	Level->site[i][j].creature = (m = make_creature (mid));
    else
	Level->site[i][j].creature = (m = m_create (i, j, WANDERING, difficulty()));
    m->x = i;
    m->y = j;
    ml->m = m;
    ml->next = Level->mlist;
    Level->mlist = ml;
}

// make and return an appropriate monster for the level and depth
// called by populate_level, doesn't actually add to mlist for some reason
// eventually to be more intelligent
pmt m_create (int x, int y, int kind, int level)
{
    pmt newmonster;
    int monster_range;
    int mid;

    switch (level) {
	case 0:
	    monster_range = ML1;
	    break;
	case 1:
	    monster_range = ML2;
	    break;
	case 2:
	    monster_range = ML3;
	    break;
	case 3:
	    monster_range = ML4;
	    break;
	case 4:
	    monster_range = ML5;
	    break;
	case 5:
	    monster_range = ML6;
	    break;
	case 6:
	    monster_range = ML7;
	    break;
	case 7:
	    monster_range = ML8;
	    break;
	case 8:
	    monster_range = ML9;
	    break;
	case 9:
	    monster_range = ML10;
	    break;
	default:
	    monster_range = NUMMONSTERS;
	    break;
    }

    do
	mid = random_range (monster_range);
    while (Monsters[mid].uniqueness != COMMON);
    newmonster = make_creature (mid);

    // no duplicates of unique monsters
    if (kind == WANDERING)
	m_status_set (newmonster, WANDERING);
    newmonster->x = x;
    newmonster->y = y;
    return (newmonster);
}

// make creature # mid, totally random if mid == -1
// make creature allocates space for the creature
pmt make_creature (int mid)
{
    pmt newmonster = ((pmt) checkmalloc (sizeof (montype)));
    pob ob;
    int i, treasures;

    if (mid == -1)
	mid = random_range (ML9);
    *newmonster = Monsters[mid];
    if ((mid == ANGEL) || (mid == HIGH_ANGEL) || (mid == ARCHANGEL)) {
	// aux1 field of an angel is its deity
	if (Current_Environment == E_TEMPLE)
	    newmonster->aux1 = Country[LastCountryLocX][LastCountryLocY].aux;
	else
	    newmonster->aux1 = random_range (6) + 1;
	strcpy (Str3, Monsters[mid].monstring);
	switch (newmonster->aux1) {
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
	newmonster->monstring = salloc (Str3);
    } else if (mid == ZERO_NPC || mid == WEREHUMAN) {
	// generic 0th level human, or a were-human
	newmonster->monstring = mantype();
	strcpy (Str1, "dead ");
	strcat (Str1, newmonster->monstring);
	newmonster->corpsestr = salloc (Str1);
    } else if ((newmonster->monchar & 0xff) == '!') {
	// the nymph/satyr and incubus/succubus
	if (Player.preference == 'f' || (Player.preference != 'm' && random_range (2))) {
	    newmonster->monchar = 'n' | CLR (RED);
	    newmonster->monstring = "nymph";
	    newmonster->corpsestr = "dead nymph";
	} else {
	    newmonster->monchar = 's' | CLR (RED);
	    newmonster->monstring = "satyr";
	    newmonster->corpsestr = "dead satyr";
	}
	if (newmonster->id == INCUBUS) {
	    if ((newmonster->monchar & 0xff) == 'n')
		newmonster->corpsestr = "dead succubus";
	    else
		newmonster->corpsestr = "dead incubus";
	}
    }
    if (mid == NPC)
	make_log_npc (newmonster);
    else if (mid == HISCORE_NPC)
	make_hiscore_npc (newmonster, random_range (15));
    else {
	if (newmonster->sleep < random_range (100))
	    m_status_set (newmonster, AWAKE);
	if (newmonster->startthing > -1 && Objects[newmonster->startthing].uniqueness <= UNIQUE_MADE) {
	    ob = ((pob) checkmalloc (sizeof (objtype)));
	    *ob = Objects[newmonster->startthing];
	    m_pickup (newmonster, ob);
	}
	treasures = random_range (newmonster->treasure);
	for (i = 0; i < treasures; i++) {
	    do {
		ob = (pob) (create_object (newmonster->level));
		if (ob->uniqueness != COMMON) {
		    Objects[ob->id].uniqueness = UNIQUE_UNMADE;
		    free (ob);
		    ob = NULL;
		}
	    } while (!ob);
	    m_pickup (newmonster, ob);
	}
    }
    newmonster->click = (Tick + 1) % 50;
    return (newmonster);
}

// drop treasures randomly onto level
void stock_level (void)
{
    int i, j, k, numtreasures = 2 * random_range (difficulty() / 4) + 4;

    // put cash anywhere, including walls, put other treasures only on floor
    for (k = 0; k < numtreasures + 10; k++) {
	do {
	    i = random_range (WIDTH);
	    j = random_range (LENGTH);
	} while (Level->site[i][j].locchar != FLOOR);
	make_site_treasure (i, j, difficulty());
	i = random_range (WIDTH);
	j = random_range (LENGTH);
	Level->site[i][j].things = ((pol) checkmalloc (sizeof (oltype)));
	Level->site[i][j].things->thing = ((pob) checkmalloc (sizeof (objtype)));
	make_cash (Level->site[i][j].things->thing, difficulty());
	Level->site[i][j].things->next = NULL;
	// caves have more random cash strewn around
	if (Current_Dungeon == E_CAVES) {
	    i = random_range (WIDTH);
	    j = random_range (LENGTH);
	    Level->site[i][j].things = ((pol) checkmalloc (sizeof (oltype)));
	    Level->site[i][j].things->thing = ((pob) checkmalloc (sizeof (objtype)));
	    make_cash (Level->site[i][j].things->thing, difficulty());
	    Level->site[i][j].things->next = NULL;
	    i = random_range (WIDTH);
	    j = random_range (LENGTH);
	    Level->site[i][j].things = ((pol) checkmalloc (sizeof (oltype)));
	    Level->site[i][j].things->thing = ((pob) checkmalloc (sizeof (objtype)));
	    make_cash (Level->site[i][j].things->thing, difficulty());
	    Level->site[i][j].things->next = NULL;
	}
    }
}

// make a new object (of at most level itemlevel) at site i,j on level
static void make_site_treasure (int i, int j, int itemlevel)
{
    pol tmp = ((pol) checkmalloc (sizeof (oltype)));
    tmp->thing = ((pob) create_object (itemlevel));
    tmp->next = Level->site[i][j].things;
    Level->site[i][j].things = tmp;
}

// make a specific new object at site i,j on level
static void make_specific_treasure (int i, int j, int iid)
{
    pol tmp;
    if (Objects[iid].uniqueness == UNIQUE_TAKEN)
	return;
    tmp = ((pol) checkmalloc (sizeof (oltype)));
    tmp->thing = ((pob) checkmalloc (sizeof (objtype)));
    *(tmp->thing) = Objects[iid];
    tmp->next = Level->site[i][j].things;
    Level->site[i][j].things = tmp;
}

// returns a "level of difficulty" based on current environment
// and depth in dungeon. Is somewhat arbitrary. value between 1 and 10.
// May not actually represent real difficulty, but instead level
// of items, monsters encountered.
int difficulty (void)
{
    int depth = 1;
    if (Level != NULL)
	depth = Level->depth;
    switch (Current_Environment) {
	case E_COUNTRYSIDE:
	    return (7);
	case E_CITY:
	    return (3);
	case E_VILLAGE:
	    return (1);
	case E_TACTICAL_MAP:
	    return (7);
	case E_SEWERS:
	    return (depth / 6) + 3;
	case E_CASTLE:
	    return (depth / 4) + 4;
	case E_CAVES:
	    return (depth / 3) + 1;
	case E_VOLCANO:
	    return (depth / 4) + 5;
	case E_ASTRAL:
	    return (8);
	case E_ARENA:
	    return (5);
	case E_HOVEL:
	    return (3);
	case E_MANSION:
	    return (7);
	case E_HOUSE:
	    return (5);
	case E_DLAIR:
	    return (9);
	case E_ABYSS:
	    return (10);
	case E_STARPEAK:
	    return (9);
	case E_CIRCLE:
	    return (8);
	case E_MAGIC_ISLE:
	    return (8);
	case E_TEMPLE:
	    return (8);
	default:
	    return (3);
    }
}

// the bank; can be broken into (!)
void l_bank (void)
{
    int done = FALSE, valid = FALSE;
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
		    done = TRUE;
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
		done = TRUE;
	    } else if ((response == 'O') && (strcmp (Password, "") == 0)) {
		clearmsg();
		print1 ("Opening new account.");
		nprint1 (" Please enter new password: ");
		strcpy (Password, msgscanstring());
		if (strcmp (Password, "") == 0) {
		    print3 ("Illegal to use null password -- aborted.");
		    done = TRUE;
		} else {
		    print2 ("Password validated; account saved.");
		    valid = TRUE;
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
    int done = FALSE;
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
	    if (action == ESCAPE)
		done = TRUE;
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
    int i;
    char item;
    pob newitem;

    print2 ("Purchase which item? [ESCAPE to quit] ");
    menuclear();
    for (i = 0; i < numitems; i++) {
	strcpy (Str4, " :");
	Str4[0] = i + 'a';
	strcat (Str4, Objects[base + i].objstr);
	menuprint (Str4);
	menuprint ("\n");
    }
    showmenu();
    item = ' ';
    while ((item != ESCAPE) && ((item < 'a') || (item >= 'a' + numitems)))
	item = mgetc();
    if (item != ESCAPE) {
	i = item - 'a';
	newitem = ((pob) checkmalloc (sizeof (objtype)));
	*newitem = Objects[base + i];
	newitem->known = 2;
	clearmsg();
	print1 ("I can let you have it for ");
	mlongprint (2 * true_item_value (newitem));
	nprint1 ("Au. Buy it? [yn] ");
	if (ynq1() == 'y') {
	    if (Player.cash < 2 * true_item_value (newitem)) {
		print2 ("Why not try again some time you have the cash?");
		free (newitem);
	    } else {
		Player.cash -= 2 * true_item_value (newitem);
		dataprint();
		gain_item (newitem);
	    }
	} else
	    free (newitem);
    }
}

void l_club (void)
{
#define hinthour club_hinthour
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
		    Spells[S_RETURN].known = TRUE;
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
	while ((response != 's') && (response != 'l') && (response != ESCAPE));
	if (response == 'l') {
	    if (hinthour == hour())
		print2 ("You don't hear anything useful.");
	    else {
		print1 ("You overhear a conversation....");
		hint();
		hinthour = hour();
	    }
	} else if (response == 's') {
	    buyfromstock (THING_KEY, 2);
	    xredraw();
	} else if (response == ESCAPE)
	    print2 ("Be seeing you, old chap!");
    }
}

#undef hinthour

void l_gym (void)
{
    int done = TRUE;
    int trained = 0;
    clearmsg();
    do {
	print1 ("The Rampart Gymnasium");
	if ((Gymcredit > 0) || (Player.rank[ARENA])) {
	    nprint1 ("-- Credit: ");
	    mlongprint (Gymcredit);
	    nprint1 ("Au.");
	}
	done = FALSE;
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
	    case ESCAPE:
		clearmsg();
		if (trained == 0)
		    print1 ("Well, it's your body you're depriving!");
		else if (trained < 3)
		    print1 ("You towel yourself off, and find the exit.");
		else
		    print1 ("A refreshing bath, and you're on your way.");
		done = TRUE;
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
    pob item;
    int i, j;
    switch (random_range (difficulty() + 3) - 1) {
	default:
	    l_statue_wake();
	    break;
	case 0:
	    print1 ("The statue crumbles with a clatter of gravel.");
	    Level->site[x][y].locchar = RUBBLE;
	    Level->site[x][y].p_locf = L_RUBBLE;
	    plotspot (x, y, TRUE);
	    lset (x, y, CHANGED);
	    break;
	case 1:
	    print1 ("The statue stoutly resists your attack.");
	    break;
	case 2:
	    print1 ("The statue crumbles with a clatter of gravel.");
	    Level->site[x][y].locchar = RUBBLE;
	    Level->site[x][y].p_locf = L_RUBBLE;
	    plotspot (x, y, TRUE);
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
		Level->site[x][y].locchar = STAIRS_DOWN;
		Level->site[x][y].p_locf = L_NO_OP;
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
	    if (Player.possessions[O_WEAPON_HAND] != NULL) {
		print1 ("Your weapon sinks deeply into the statue and is sucked away!");
		item = Player.possessions[O_WEAPON_HAND];
		conform_lost_object (Player.possessions[O_WEAPON_HAND]);
		item->blessing = -1 - abs (item->blessing);
		drop_at (x, y, item);
	    }
	    break;
	case 9:
	    print1 ("The statue extends an arm. Beams of light illuminate the level!");
	    for (i = 0; i < WIDTH; i++)
		for (j = 0; j < LENGTH; j++) {
		    lset (i, j, SEEN);
		    if (loc_statusp (i, j, SECRET)) {
			lreset (i, j, SECRET);
			lset (i, j, CHANGED);
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
	wake_statue (x + Dirs[0][i], y + Dirs[1][i], TRUE);
}

static void wake_statue (int x, int y, int first)
{
    int i;
    pml tml;
    if (Level->site[x][y].locchar == STATUE) {
	if (!first)
	    mprint ("Another statue awakens!");
	else
	    mprint ("A statue springs to life!");
	Level->site[x][y].locchar = FLOOR;
	lset (x, y, CHANGED);
	tml = ((pml) checkmalloc (sizeof (mltype)));
	tml->m = (Level->site[x][y].creature = m_create (x, y, 0, difficulty() + 1));
	m_status_set (Level->site[x][y].creature, HOSTILE);
	tml->next = Level->mlist;
	Level->mlist = tml;
	for (i = 0; i < 8; i++)
	    wake_statue (x + Dirs[0][i], y + Dirs[1][i], FALSE);
    }
}

void l_casino (void)
{
    int i, done = FALSE, a, b, c, match;
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
	    } else if (response == ESCAPE)
		done = TRUE;
	}
    }
}

void l_commandant (void)
{
    int num;
    pob food;
    print1 ("Commandant Sonder's Rampart-fried Lyzzard partes. Open 24 hrs.");
    print2 ("Buy a bucket! Only 5 Au. Make a purchase? [yn] ");
    if (ynq2() == 'y') {
	clearmsg();
	print1 ("How many? ");
	num = (int) parsenum();
	if (num < 1)
	    print3 ("Cute. Real cute.");
	else if (num * 5 > Player.cash)
	    print3 ("No handouts here, mac!");
	else {
	    Player.cash -= num * 5;
	    food = ((pob) checkmalloc (sizeof (objtype)));
	    *food = Objects[FOOD_RATION];
	    food->number = num;
	    if (num == 1)
		print2 ("There you go, mac! One Lyzzard Bucket, coming up.");
	    else
		print2 ("A passel of Lyzzard Buckets, for your pleasure.");
	    morewait();
	    gain_item (food);
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
#define hinthour tavern_hinthour
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
	while ((response != 'a') && (response != 'b') && (response != 'c') && (response != 'd') && (response != ESCAPE));
	switch (response) {
	    case 'a':
		if (Player.cash < 1)
		    print2 ("Aw hell, have one on me.");
		else {
		    Player.cash -= 1;
		    dataprint();
		    if (hinthour != hour()) {
			if (random_range (3)) {
			    print1 ("You overhear a rumor...");
			    hint();
			} else
			    print1 ("You don't hear much of interest.");
			hinthour = hour();
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
		    toggle_item_use (TRUE);
		    Player.str = min (Player.str, Player.maxstr);
		    Player.con = min (Player.con, Player.maxcon);
		    Player.agi = min (Player.agi, Player.maxagi);
		    Player.dex = min (Player.dex, Player.maxdex);
		    Player.iq = min (Player.iq, Player.maxiq);
		    Player.pow = min (Player.pow, Player.maxpow);
		    toggle_item_use (FALSE);
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

#undef hinthour

void l_alchemist (void)
{
    int i, done = FALSE, mlevel;
    char response;
    pob obj;
    print1 ("Ambrosias' Potions et cie.");
    if (nighttime())
	print2 ("Ambrosias doesn't seem to be in right now.");
    else
	while (!done) {
	    morewait();
	    clearmsg();
	    print1 ("a: Sell monster components.");
	    print2 ("b: Pay for transformation.");
	    print3 ("ESCAPE: Leave this place.");
	    response = (char) mcigetc();
	    if (response == 'a') {
		clearmsg();
		done = TRUE;
		i = getitem (CORPSE);
		if ((i != ABORT) && (Player.possessions[i] != NULL)) {
		    obj = Player.possessions[i];
		    if (Monsters[obj->charge].transformid == -1) {
			print1 ("I don't want such a thing.");
			if (obj->basevalue > 0)
			    print2 ("You might be able to sell it to someone else, though.");
		    } else {
			clearmsg();
			print1 ("I'll give you ");
			mnumprint (obj->basevalue / 3);
			nprint1 ("Au for it. Take it? [yn] ");
			if (ynq1() == 'y') {
			    Player.cash += (obj->basevalue / 3);
			    conform_lost_objects (1, obj);
			} else
			    print2 ("Well, keep the smelly old thing, then!");
		    }
		} else
		    print2 ("So nu?");
	    } else if (response == 'b') {
		clearmsg();
		done = TRUE;
		i = getitem (CORPSE);
		if ((i != ABORT) && (Player.possessions[i] != NULL)) {
		    obj = Player.possessions[i];
		    if (Monsters[obj->charge].transformid == -1)
			print1 ("Oy vey! You want me to transform such a thing?");
		    else {
			mlevel = Monsters[obj->charge].level;
			print1 ("It'll cost you ");
			mnumprint (max (10, obj->basevalue * 2));
			nprint1 ("Au for the transformation. Pay it? [yn] ");
			if (ynq1() == 'y') {
			    if (Player.cash < max (10, obj->basevalue * 2))
				print2 ("You can't afford it!");
			    else {
				print1 ("Voila! A tap of the Philosopher's Stone...");
				Player.cash -= max (10, obj->basevalue * 2);
				*obj = Objects[Monsters[obj->charge].transformid];
				if ((obj->id >= STICKID) && (obj->id < STICKID + NUMSTICKS))
				    obj->charge = 20;
				if (obj->plus == 0)
				    obj->plus = mlevel;
				if (obj->blessing == 0)
				    obj->blessing = 1;
			    }
			} else
			    print2 ("I don't need your business, anyhow.");
		    }
		} else
		    print2 ("So nu?");
	    } else if (response == ESCAPE)
		done = TRUE;
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
	    else if (strcmp (Player.name, Str1) != 0) {
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
    int studied = FALSE;
    int done = FALSE, fee = 1000;
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
		    done = TRUE;
		} else {
		    Player.cash -= fee;
		    do {
			studied = TRUE;
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
			    theologyfile();
			} else if (response == 'b') {
			    print1 ("You unroll a slick four-color document...");
			    morewait();
			    cityguidefile();
			} else if (response == 'c') {
			    print1 ("This scroll is written in a strange magical script...");
			    morewait();
			    wishfile();
			} else if (response == 'd') {
			    print1 ("You find a strange document, obviously misfiled");
			    print2 ("under the heading 'acrylic fungus painting technique'");
			    morewait();
			    adeptfile();
			} else if (response == 'e') {
			    if (random_range (30) > Player.iq) {
				print2 ("You feel more knowledgeable!");
				Player.iq++;
				Player.maxiq++;
				dataprint();
				if (Player.maxiq < 19 && fee != max (50, 1000 - (18 - Player.maxiq) * 125)) {
				    morewait();
				    clearmsg();
				    print1 ("Your revised fee is: ");
				    mnumprint (fee = max (50, 1000 - (18 - Player.maxiq) * 125));
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
			} else if (response == ESCAPE) {
			    done = TRUE;
			    print1 ("That was an expensive browse...");
			} else
			    studied = FALSE;
		    } while (!studied);
		}
		xredraw();
	    } else {
		done = TRUE;
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
    int i, j, k, limit, number, done = FALSE;
    char item, action;

    if (nighttime())
	print1 ("Shop Closed: Have a Nice (K)Night");
    else {
	limit = min (5, Date - Pawndate);
	Pawndate = Date;
	for (k = 0; k < limit; k++) {
	    if (Pawnitems[0] != NULL) {
		if (Objects[Pawnitems[0]->id].uniqueness > UNIQUE_UNMADE)
		    Objects[Pawnitems[0]->id].uniqueness = UNIQUE_UNMADE;
		// could turn up anywhere, really :)
		free (Pawnitems[0]);
		Pawnitems[0] = NULL;
	    }
	    for (i = 0; i < PAWNITEMS - 1; i++)
		Pawnitems[i] = Pawnitems[i + 1];
	    Pawnitems[PAWNITEMS - 1] = NULL;
	    for (i = 0; i < PAWNITEMS; i++)
		if (Pawnitems[i] == NULL)
		    do {
			if (Pawnitems[i] != NULL)
			    free (Pawnitems[i]);
			Pawnitems[i] = create_object (5);
			Pawnitems[i]->known = 2;
		    } while ((Pawnitems[i]->objchar == CASH) || (Pawnitems[i]->objchar == ARTIFACT) || (true_item_value (Pawnitems[i]) <= 0));
	}
	while (!done) {
	    print1 ("Knight's Pawn Shop:");
	    print2 ("Buy item, Sell item, sell Pack contents, Leave [b,s,p,ESCAPE] ");
	    menuclear();
	    for (i = 0; i < PAWNITEMS; i++)
		if (Pawnitems[i] != NULL) {
		    strcpy (Str3, " :");
		    Str3[0] = i + 'a';
		    strcat (Str3, itemid (Pawnitems[i]));
		    menuprint (Str3);
		    menuprint ("\n");
		}
	    showmenu();
	    action = (char) mcigetc();
	    if (action == ESCAPE)
		done = TRUE;
	    else if (action == 'b') {
		print2 ("Purchase which item? [ESCAPE to quit] ");
		item = ' ';
		while ((item != ESCAPE) && ((item < 'a') || (item >= 'a' + PAWNITEMS)))
		    item = (char) mcigetc();
		if (item != ESCAPE) {
		    i = item - 'a';
		    if (Pawnitems[i] == NULL)
			print3 ("No such item!");
		    else if (true_item_value (Pawnitems[i]) <= 0) {
			print1 ("Hmm, how did that junk get on my shelves?");
			print2 ("I'll just remove it.");
			free (Pawnitems[i]);
			Pawnitems[i] = NULL;
		    } else {
			clearmsg();
			print1 ("The low, low, cost is: ");
			mlongprint (Pawnitems[i]->number * true_item_value (Pawnitems[i]));
			nprint1 (" Buy it? [ynq] ");
			if (ynq1() == 'y') {
			    if (Player.cash < Pawnitems[i]->number * true_item_value (Pawnitems[i])) {
				print2 ("No credit! Gwan, Beat it!");
				morewait();
			    } else {
				Player.cash -= Pawnitems[i]->number * true_item_value (Pawnitems[i]);
				Objects[Pawnitems[i]->id].known = 1;
				gain_item (Pawnitems[i]);
				Pawnitems[i] = NULL;
			    }
			}
		    }
		}
	    } else if (action == 's') {
		menuclear();
		print2 ("Sell which item: ");
		i = getitem (NULL_ITEM);
		if ((i != ABORT) && (Player.possessions[i] != NULL)) {
		    if (cursed (Player.possessions[i])) {
			print1 ("No loans on cursed items! I been burned before....");
			morewait();
		    } else if (true_item_value (Player.possessions[i]) <= 0) {
			print1 ("That looks like a worthless piece of junk to me.");
			morewait();
		    } else {
			clearmsg();
			print1 ("You can get ");
			mlongprint (item_value (Player.possessions[i]) / 2);
			nprint1 ("Au each. Sell [yn]? ");
			if (ynq1() == 'y') {
			    number = getnumber (Player.possessions[i]->number);
			    if ((number >= Player.possessions[i]->number) && Player.possessions[i]->used) {
				Player.possessions[i]->used = FALSE;
				item_use (Player.possessions[i]);
			    }
			    Player.cash += number * item_value (Player.possessions[i]) / 2;
			    free (Pawnitems[0]);
			    for (j = 0; j < PAWNITEMS - 1; j++)
				Pawnitems[j] = Pawnitems[j + 1];
			    Pawnitems[PAWNITEMS - 1] = ((pob) checkmalloc (sizeof (objtype)));
			    *(Pawnitems[PAWNITEMS - 1]) = *(Player.possessions[i]);
			    Pawnitems[PAWNITEMS - 1]->number = number;
			    Pawnitems[PAWNITEMS - 1]->known = 2;
			    dispose_lost_objects (number, Player.possessions[i]);
			    dataprint();
			}
		    }
		}
	    } else if (action == 'p') {
		for (i = 0; i < Player.packptr; i++) {
		    if (Player.pack[i]->blessing > -1 && true_item_value (Player.pack[i]) > 0) {
			clearmsg();
			print1 ("Sell ");
			nprint1 (itemid (Player.pack[i]));
			nprint1 (" for ");
			mlongprint (item_value (Player.pack[i]) / 2);
			nprint1 ("Au each? [yn] ");
			if (ynq1() == 'y') {
			    number = getnumber (Player.pack[i]->number);
			    if (number > 0) {
				Player.cash += number * item_value (Player.pack[i]) / 2;
				free (Pawnitems[0]);
				for (j = 0; j < PAWNITEMS - 1; j++)
				    Pawnitems[j] = Pawnitems[j + 1];
				Pawnitems[PAWNITEMS - 1] = ((pob) checkmalloc (sizeof (objtype)));
				*(Pawnitems[PAWNITEMS - 1]) = *(Player.pack[i]);
				Pawnitems[PAWNITEMS - 1]->number = number;
				Pawnitems[PAWNITEMS - 1]->known = 2;
				Player.pack[i]->number -= number;
				if (Player.pack[i]->number < 1) {
				    free (Player.pack[i]);
				    Player.pack[i] = NULL;
				}
				dataprint();
			    }
			}
		    }
		}
		fixpack();
	    }
	}
    }
    calc_melee();
    xredraw();
}

void l_condo (void)
{
    pol ol, prev = NULL;
    int i, done = FALSE, over = FALSE, weeksleep = FALSE;
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
		    Condoitems = NULL;
		}
	    }
	} else if (response == 'r') {
	    print2 ("Weekly Rental, 1000Au. Pay for it? [yn] ");
	    if (ynq2() == 'y') {
		if (Player.cash < 1000)
		    print2 ("Hey, pay the rent or out you go....");
		else {
		    weeksleep = TRUE;
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
		i = getitem (NULL_ITEM);
		if (i != ABORT) {
		    if (Player.possessions[i]->blessing < 0)
			print2 ("The item just doesn't want to be stored away...");
		    else {
			ol = ((pol) checkmalloc (sizeof (oltype)));
			ol->thing = Player.possessions[i];
			ol->next = Condoitems;
			Condoitems = ol;
			conform_unused_object (Player.possessions[i]);
			Player.possessions[i] = NULL;
		    }
		}
	    } else if (response == 'b') {
		ol = Condoitems;
		while ((ol != NULL) && (!over)) {
		    print1 ("Retrieve ");
		    nprint1 (itemid (ol->thing));
		    nprint1 (" [ynq] ");
		    response = (char) mcigetc();
		    if (response == 'y') {
			gain_item (ol->thing);
			if (ol == Condoitems)
			    Condoitems = Condoitems->next;
			else if (prev != NULL)
			    prev->next = ol->next;
		    } else if (response == 'q')
			over = TRUE;
		    prev = ol;
		    ol = ol->next;
		}
	    } else if (response == 'c') {
		weeksleep = TRUE;
		print1 ("You take a week off to rest...");
		morewait();
	    } else if (response == 'd') {
		clearmsg();
		print1 ("You sure you want to retire, now? [yn] ");
		if (ynq1() == 'y') {
		    p_win();
		}
	    } else if (response == ESCAPE)
		done = TRUE;
	}
	xredraw();
    }
    if (weeksleep) {
	clearmsg();
	print1 ("Taking a week off to rest...");
	morewait();
	toggle_item_use (TRUE);
	Player.hp = Player.maxhp;
	Player.str = Player.maxstr;
	Player.agi = Player.maxagi;
	Player.con = Player.maxcon;
	Player.dex = Player.maxdex;
	Player.iq = Player.maxiq;
	Player.pow = Player.maxpow;
	for (i = 0; i < NUMSTATI; i++)
	    if (Player.status[i] < 1000)
		Player.status[i] = 0;
	toggle_item_use (FALSE);
	Player.food = 36;
	print2 ("You're once again fit and ready to continue your adventure.");
	Time += 60 * 24 * 7;
	Date += 7;
	moon_check();
	timeprint();
    }
}

static void gymtrain (int *maxstat, int *stat)
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
    pml ml;

    for (ml = Level->mlist; ml != NULL; ml = ml->next)
	if ((ml->m->id == GUARD) ||
	    ((ml->m->id == HISCORE_NPC) && (ml->m->aux2 == 15))) {	// justiciar
	    m_status_reset (ml->m, HOSTILE);
	    ml->m->specialf = M_NO_OP;
	    if (ml->m->id == GUARD && ml->m->hp > 0 && ml->m->aux1 > 0) {
		if (Level->site[ml->m->x][ml->m->y].creature == ml->m)
		    Level->site[ml->m->x][ml->m->y].creature = NULL;
		ml->m->x = ml->m->aux1;
		ml->m->y = ml->m->aux2;
		Level->site[ml->m->x][ml->m->y].creature = ml->m;
	    } else if (ml->m->id == HISCORE_NPC && ml->m->hp > 0 && Current_Environment == E_CITY) {
		if (Level->site[ml->m->x][ml->m->y].creature == ml->m)
		    Level->site[ml->m->x][ml->m->y].creature = NULL;
		ml->m->x = 40;
		ml->m->y = 62;
		Level->site[ml->m->x][ml->m->y].creature = ml->m;
	    }
	}
    if (Current_Environment == E_CITY)
	Level->site[40][60].p_locf = L_ORDER;	// undoes action in alert_guards
}

void send_to_jail (void)
{
    if (Player.rank[ORDER] > 0) {
	print1 ("A member of the Order of Paladins sent to jail!");
	print2 ("It cannot be!");
	morewait();
	print1 ("You are immediately expelled permanently from the Order!");
	print2 ("Your name is expunged from the records....");
	Player.rank[ORDER] = -1;
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
    if (!gamestatusp (ATTACKED_ORACLE)) {
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
    int damage = 0, stuck = TRUE;
    print1 ("The hedge comes alive with a surge of alien growth!");
    while (stuck) {
	dataprint();
	damage += Level->depth / 2 + 1;
	print2 ("Razor-edged vines covered in suckers attach themselves to you.");
	morewait();
	if (find_and_remove_item (THING_SALT_WATER, -1)) {
	    print1 ("Thinking fast, you toss salt water on the trifid...");
	    print2 ("The trifid disintegrates with a frustrated sigh.");
	    Level->site[Player.x][Player.y].locchar = FLOOR;
	    Level->site[Player.x][Player.y].p_locf = L_NO_OP;
	    lset (Player.x, Player.y, CHANGED);
	    gain_experience (1000);
	    stuck = FALSE;
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
			stuck = FALSE;
		    } else
			print1 ("Well, THAT didn't work.");
		    break;
		case 'b':
		    print1 ("Well, at least you're facing your fate with dignity.");
		    break;
		case 'c':
		    if ((Player.patron == DRUID) && (Player.rank[PRIESTHOOD] > random_range (5))) {
			print1 ("A shaft of golden light bathes the alien plant");
			print2 ("which grudginly lets you go....");
			stuck = FALSE;
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
	Level->site[12][56].locchar = FLOOR;
    } else {
	print2 ("The door is closed.");
	Level->site[12][56].locchar = WALL;
	morewait();
	clearmsg();
	print1 ("Try to crack it? [yn] ");
	if (ynq1() == 'y') {
	    if (random_range (100) < Player.rank[THIEVES] * Player.rank[THIEVES]) {
		print2 ("The lock clicks open!!!");
		gain_experience (5000);
		Level->site[12][56].locchar = FLOOR;
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
	while ((response != 'a') && (response != 'b') && (response != 'c') && (response != ESCAPE));
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
			toggle_item_use (TRUE);
			Player.str = min (Player.str, Player.maxstr);
			Player.con = min (Player.con, Player.maxcon);
			Player.agi = min (Player.agi, Player.maxagi);
			Player.dex = min (Player.dex, Player.maxdex);
			Player.iq = min (Player.iq, Player.maxiq);
			Player.pow = min (Player.pow, Player.maxpow);
			toggle_item_use (FALSE);
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
    if ((Level->site[x][y].p_locf >= CITYSITEBASE) && (Level->site[x][y].p_locf < CITYSITEBASE + NUMCITYSITES))
	CitySiteList[Level->site[x][y].p_locf - CITYSITEBASE][0] = TRUE;
    switch (Level->site[x][y].p_locf) {
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
	    if (Level->site[x][y].locchar == FLOOR)
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
    char response;
    if (gamestatusp (ATTACKED_ORACLE) && (!gamestatusp (COMPLETED_ASTRAL))) {
	print1 ("You come before a blue crystal dais. You see a broken mirror.");
	print2 ("Look in the mirror? [yn] ");
	if (ynq2() == 'y') {
	    print1 ("A strange force rips you from your place....");
	    Player.hp = 1;
	    print2 ("You feel drained....");
	    dataprint();
	    print3 ("You find yourself in a weird flickery maze.");
	    change_environment (E_ASTRAL);
	}
    } else {
	print1 ("You come before a blue crystal dais. There is a bell and a mirror.");
	print2 ("Ring the bell [b], look in the mirror [m], or leave [ESCAPE] ");
	do
	    response = (char) mcigetc();
	while ((response != 'b') && (response != 'm') && (response != ESCAPE));
	if (response == 'b') {
	    print1 ("The ringing note seems to last forever.");
	    print2 ("You notice a robed figure in front of you....");
	    morewait();
	    print1 ("The oracle doffs her cowl. Her eyes glitter with blue fire!");
	    print2 ("Attack her? [yn] ");
	    if (ynq2() == 'y') {
		setgamestatus (ATTACKED_ORACLE);
		print1 ("The oracle deftly avoids your attack.");
		print2 ("She sneers at you and vanishes.");
	    } else {
		print2 ("She stares at you...and speaks:");
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
	    }
	} else if (response == 'm') {
	    print1 ("You seem to see yourself. Odd....");
	    knowledge (1);
	} else
	    print2 ("You leave this immanent place.");
    }
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
    pob newitem;
    int attempt = 0;
    print1 ("You have discovered a safe!");
    print2 ("Pick the lock [p], Force the door [f], or ignore [ESCAPE]");
    do
	response = (char) mcigetc();
    while ((response != 'p') && (response != 'f') && (response != ESCAPE));
    if (response == 'p')
	attempt = (2 * Player.dex + Player.rank[THIEVES] * 10 - random_range (100)) / 10;
    else if (response == 'f')
	attempt = (Player.dmg - random_range (100)) / 10;
    if (attempt > 0) {
	Player.alignment -= 4;
	gain_experience (50);
	print2 ("The door springs open!");
	Level->site[Player.x][Player.y].locchar = FLOOR;
	Level->site[Player.x][Player.y].p_locf = L_NO_OP;
	lset (Player.x, Player.y, CHANGED);
	if (random_range (2) == 1) {
	    print1 ("You find:");
	    do {
		newitem = create_object (difficulty());
		print2 (itemid (newitem));
		morewait();
		gain_item (newitem);
	    } while (random_range (3) == 1);
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
	    Level->site[Player.x][Player.y].locchar = RUBBLE;
	    Level->site[Player.x][Player.y].p_locf = L_RUBBLE;
	    lset (Player.x, Player.y, CHANGED);
	} else if (attempt == -3) {
	    print1 ("The safe jolts you with electricity!");
	    lball (Player.x, Player.y, Player.x, Player.y, 30);
	} else if (attempt < -3) {
	    print1 ("You are hit by an acid spray!");
	    if (Player.possessions[O_CLOAK] != NULL) {
		print2 ("Your cloak is destroyed!");
		conform_lost_object (Player.possessions[O_CLOAK]);
		p_damage (10, ACID, "a safe");
	    } else if (Player.possessions[O_ARMOR] != NULL) {
		print2 ("Your armor corrodes!");
		Player.possessions[O_ARMOR]->dmg -= 3;
		Player.possessions[O_ARMOR]->hit -= 3;
		Player.possessions[O_ARMOR]->aux -= 3;
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
		case 1:
		    x = 56;
		    y = 5;
		    break;
		default:
		case 2:
		    x = 35;
		    y = 11;
		    break;
		case 3:
		    x = 10;
		    y = 40;
		    break;
		case 4:
		    x = 7;
		    y = 6;
		    break;
		case 5:
		    x = 40;
		    y = 43;
		    break;
		case 6:
		    x = 20;
		    y = 41;
		    break;
	    }
	    for (i = x - 15; i <= x + 15; i++)
		for (j = y - 15; j <= y + 15; j++)
		    if ((i >= 0) && (i < 64) && (j >= 0) && (j < 64)) {
			if (Country[i][j].current_terrain_type != Country[i][j].base_terrain_type) {
			    c_set (i, j, CHANGED);
			    Country[i][j].current_terrain_type = Country[i][j].base_terrain_type;
			}
			c_set (i, j, SEEN);
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
