#include "glob.h"
#include <unistd.h>
#include <stdlib.h>

//----------------------------------------------------------------------

static void restore_country(istream& is, int ver);
static pob restore_item(istream& is, int ver);
static pol restore_itemlist(istream& is, int ver);
static void restore_level(istream& is, int ver);
static void restore_monsters(istream& is, plv level, int ver);
static void restore_player(istream& is, int ver);
static void save_country(ostream& os);
static void save_item (ostream& os, pob o);
static void save_itemlist (ostream& os, pol ol);
static void save_level(ostream& os, plv level);
static void save_monsters(ostream& os, pml ml);
static void save_player(ostream& os);

//----------------------------------------------------------------------

// Various functions for doing game saves and restores
// The game remembers various player information, the city level,
// the country level, and the last or current dungeon level

//*************** SAVE FUNCTIONS

// Checks to see if save file already exists.
// Checks to see if save file can be opened for write.
// The player, the city level, and the current dungeon level are saved.

int save_game (const char* savestr)
{
    plv current, levelToSave;

    if (access (savestr, R_OK) == 0) {
	if (access (savestr, W_OK) == 0) {
	    mprint (" Overwrite old file?");
	    if (ynq() != 'y')
		return (FALSE);
	} else {
	    mprint (" File already exists.");
	    return (FALSE);
	}
    }
    print1 ("Saving Game....");
    bool writeok = false;
    try {
	memblock buf (UINT16_MAX);
	ostream os (buf);

	// write the version number
	os << unsigned(OMEGA_VERSION);

	save_player (os);
	save_country (os);
	save_level (os, City);

	if (Current_Environment == E_CITY || Current_Environment == E_COUNTRYSIDE)
	    levelToSave = Dungeon;
	else if (Current_Environment == Current_Dungeon)
	    levelToSave = Dungeon;
	else
	    levelToSave = Level;
	int i;
	for (i = 0, current = levelToSave; current; current = current->next, i++);
	os << i;
	for (current = levelToSave; current; current = current->next)
	    if (current != Level)
		save_level (os, current);
	if (levelToSave)
	    save_level (os, Level);	// put current level last

	buf.resize (os.pos());
	buf.write_file (savestr);
	print1 ("Game Saved.");
	writeok = true;
    } catch (...) {
	print1 ("Something didn't work... save aborted.");
    }
    morewait();
    clearmsg();
    return (writeok);
}

// read player data, city level, dungeon level,
// check on validity of save file, etc.
// return TRUE if game restored, FALSE otherwise
int restore_game (const char* savestr)
{
    if (access (savestr, F_OK | R_OK | W_OK) == -1) {	// access uses real uid
	print1 ("Unable to access save file: ");
	nprint1 (savestr);
	morewait();
	return FALSE;
    }

    memblock buf;
    try {
	buf.read_file (savestr);
	istream is (buf);

	print1 ("Restoring...");

	int ver;
	is >> ver;

	restore_player (is, ver);
	restore_country (is, ver);
	restore_level (is, ver);	// the city level
	int i;
	is >> i;
	for (; i > 0; i--) {
	    restore_level (is, ver);
	    if (Level->environment == Current_Dungeon) {
		Level->next = Dungeon;
		Dungeon = Level;
	    }
	    if (Current_Environment == E_CITY)
		Level = City;
	}
	// this disgusting kludge because LENGTH and WIDTH are globals...
	WIDTH = 64;
	switch (Current_Environment) {
	    case E_COURT:
		LENGTH = 24;
		break;
	    case E_ARENA:
	    case E_ABYSS:
	    case E_CIRCLE:
	    case E_MANSION:
	    case E_HOUSE:
	    case E_HOVEL:
	    case E_DLAIR:
	    case E_STARPEAK:
	    case E_MAGIC_ISLE:
	    case E_TEMPLE:
	    case E_VILLAGE:
		LENGTH = 16;
		break;
	    default:
		LENGTH = 64;
		break;
	}
	print3 ("Restoration complete.");
	ScreenOffset = -1000;	// to force a redraw
	setgamestatus (SKIP_MONSTERS);
    } catch (const exception& e) {
	char errbuf[80];
	snprintf (ArrayBlock(errbuf), "Error restoring %s: %s", savestr, e.what().c_str());
	print1 (errbuf);
	morewait();
	return (FALSE);
    }
    return (TRUE);
}

// saves game on SIGHUP
// no longer tries to compress, which hangs
void signalsave (int sig UNUSED)
{
    save_game ("omega.sav");
    print1 ("Signal - Saving file 'omega.sav'.");
    morewait();
    endgraf();
    exit (0);
}

// also saves some globals like Level->depth...

static void save_player (ostream& os)
{
    // Save random global state information
    Player.click = (Tick + 1) % 60;
    os.write (&Player, sizeof(Player));
    os.write (Password, sizeof(Password));
    os.write (CitySiteList, sizeof (CitySiteList));
    os << ios::align(alignof(GameStatus)) << GameStatus;
    os << Time;
    os << Gymcredit;
    os << Balance;
    os << FixedPoints;
    os << SpellKnown;
    os << Current_Environment;
    os << Last_Environment;
    os << Current_Dungeon;
    os << Villagenum;
    os << Verbosity;
    os << Tick;
    os << Searchnum;
    os << Behavior;
    os << Phase;
    os << Date;
    os << Spellsleft;
    os << SymbolUseHour;
    os << ViewHour;
    os << HelmHour;
    os << Constriction;
    os << Blessing;
    os << LastDay;
    os << RitualHour;
    os << Lawstone;
    os << Chaostone;
    os << Mindstone;
    os << Arena_Opponent;
    os << Imprisonment;
    os << StarGemUse;
    os << HiMagicUse;
    os << HiMagic;
    os << LastCountryLocX;
    os << LastCountryLocY;
    os << LastTownLocX;
    os << LastTownLocY;
    os << Pawndate;
    os << Command_Duration;
    os << Precipitation;
    os << Lunarity;
    os << ZapHour;
    os << RitualRoom;
    os << twiddle;
    os << saved;
    os << onewithchaos;
    os << club_hinthour;
    os << winnings;
    os << tavern_hinthour;
    os.write (deepest, sizeof(deepest));
    os.write (level_seed, sizeof(level_seed));

    // Save player item knowledge
    os.write (Spells, sizeof (Spells));
    os.write (ObjectAttrs, sizeof(ObjectAttrs));

    // Save player possessions
    if (Player.possessions[O_READY_HAND] == Player.possessions[O_WEAPON_HAND])
	Player.possessions[O_READY_HAND] = NULL;
    for (unsigned i = 0; i < MAXITEMS; i++)
	save_item (os, Player.possessions[i]);
    for (unsigned i = 0; i < MAXPACK; i++)
	save_item (os, Player.pack[i]);
    for (unsigned i = 0; i < PAWNITEMS; i++)
	save_item (os, Pawnitems[i]);

    // Save items in condo vault
    save_itemlist (os, Condoitems);
}

static void restore_player (istream& is, int ver)
{
    int i;
    is.read (&Player, sizeof(Player));
    is.read (Password, sizeof(Password));
    is.read (CitySiteList, sizeof(CitySiteList));
    is >> ios::align(alignof(GameStatus)) >> GameStatus;
    is >> Time;
    is >> Gymcredit;
    is >> Balance;
    is >> FixedPoints;
    is >> SpellKnown;
    is >> Current_Environment;
    is >> Last_Environment;
    is >> Current_Dungeon;
    is >> Villagenum;
    is >> Verbosity;
    is >> Tick;
    is >> Searchnum;
    is >> Behavior;
    is >> Phase;
    is >> Date;
    is >> Spellsleft;
    is >> SymbolUseHour;
    is >> ViewHour;
    is >> HelmHour;
    is >> Constriction;
    is >> Blessing;
    is >> LastDay;
    is >> RitualHour;
    is >> Lawstone;
    is >> Chaostone;
    is >> Mindstone;
    is >> Arena_Opponent;
    is >> Imprisonment;
    is >> StarGemUse;
    is >> HiMagicUse;
    is >> HiMagic;
    is >> LastCountryLocX;
    is >> LastCountryLocY;
    is >> LastTownLocX;
    is >> LastTownLocY;
    is >> Pawndate;
    is >> Command_Duration;
    is >> Precipitation;
    is >> Lunarity;
    is >> ZapHour;
    is >> RitualRoom;
    is >> twiddle;
    is >> saved;
    is >> onewithchaos;
    is >> club_hinthour;
    is >> winnings;
    is >> tavern_hinthour;
    is.read (deepest, sizeof(deepest));
    is.read (level_seed, sizeof(level_seed));
    is.read (Spells, sizeof(Spells));
    is.read (ObjectAttrs, sizeof(ObjectAttrs));

    switch (Current_Dungeon) {
	case E_ASTRAL:
	    MaxDungeonLevels = ASTRALLEVELS;
	    break;
	case E_SEWERS:
	    MaxDungeonLevels = SEWERLEVELS;
	    break;
	case E_CASTLE:
	    MaxDungeonLevels = CASTLELEVELS;
	    break;
	case E_CAVES:
	    MaxDungeonLevels = CAVELEVELS;
	    break;
	case E_VOLCANO:
	    MaxDungeonLevels = VOLCANOLEVELS;
	    break;
    }

    for (i = 0; i < MAXITEMS; i++)
	Player.possessions[i] = restore_item (is, ver);

    if (!Player.possessions[O_READY_HAND] && Player.possessions[O_WEAPON_HAND] && twohandedp (Player.possessions[O_WEAPON_HAND]->id))
	Player.possessions[O_READY_HAND] = Player.possessions[O_WEAPON_HAND];

    for (i = 0; i < MAXPACK; i++)
	Player.pack[i] = restore_item (is, ver);
    for (i = 0; i < PAWNITEMS; i++)
	Pawnitems[i] = restore_item (is, ver);
    Condoitems = restore_itemlist (is, ver);
}

// Save whatever is pointed to by level
static void save_level (ostream& os, plv level)
{
    unsigned i, j, run;
    os << ios::align(alignof(level->environment)) << level->environment << level->depth << level->numrooms << level->tunnelled;
    for (j = 0; j < MAXLENGTH; j++) {
	for (i = 0; i < MAXWIDTH; i++) {
	    if (level->site[i][j].lstatus & CHANGED) {	// this loc has been changed
		for (run = i + 1; run < MAXWIDTH &&	// find how many in a row
		     level->site[run][j].lstatus & CHANGED; run++);
		os << ios::align(alignof(i)) << i << j << run;
		for (; i < run; i++)
		    os.write (&level->site[i][j], sizeof (struct location));
	    }
	}
    }
    os << ios::align(alignof(i)) << i << j;	// signify end
    // since we don't mark the 'seen' bits as CHANGED, need to save a bitmask
    unsigned long mask = 0;
    run = 8 * sizeof (mask);
    os.align (alignof(mask));
    for (j = 0; j < MAXLENGTH; j++) {
	for (i = 0; i < MAXWIDTH; i++) {
	    if (run == 0) {
		run = 8 * sizeof(mask);
		os << mask;
		mask = 0;
	    }
	    mask >>= 1;
	    if (level->site[i][j].lstatus & SEEN)
		mask |= (1UL << (sizeof(mask)*8 - 1));
	    run--;
	}
    }
    if (run < 8*sizeof (mask))
	os << mask;
    save_monsters (os, level->mlist);
    for (i = 0; i < MAXWIDTH; i++) {
	for (j = 0; j < MAXLENGTH; j++) {
	    if (level->site[i][j].things) {
		os << ios::align(alignof(i)) << i << j;
		save_itemlist (os, level->site[i][j].things);
	    }
	}
    }
    os << ios::align(alignof(i)) << i << j;	// signify end
}

static void restore_level (istream& is, int ver)
{
    int i, j, run;
    unsigned long int mask = 0;
    int temp_env;

    Level = (plv) checkmalloc (sizeof (levtype));
    clear_level (Level);
    is >> ios::align(alignof(Level->environment)) >> Level->environment >> Level->depth >> Level->numrooms >> Level->tunnelled;
    Level->generated = TRUE;
    temp_env = Current_Environment;
    Current_Environment = Level->environment;
    switch (Level->environment) {
	case E_COUNTRYSIDE:
	    load_country();
	    break;
	case E_CITY:
	    load_city (FALSE);
	    break;
	case E_VILLAGE:
	    load_village (Country[LastCountryLocX][LastCountryLocY].aux, FALSE);
	    break;
	case E_CAVES:
	    initrand (Current_Environment, Level->depth);
	    if ((random_range (4) == 0) && (Level->depth < MaxDungeonLevels))
		room_level();
	    else
		cavern_level();
	    break;
	case E_SEWERS:
	    initrand (Current_Environment, Level->depth);
	    if ((random_range (4) == 0) && (Level->depth < MaxDungeonLevels))
		room_level();
	    else
		sewer_level();
	    break;
	case E_CASTLE:
	    initrand (Current_Environment, Level->depth);
	    room_level();
	    break;
	case E_ASTRAL:
	    initrand (Current_Environment, Level->depth);
	    maze_level();
	    break;
	case E_VOLCANO:
	    initrand (Current_Environment, Level->depth);
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
	case E_HOVEL:
	case E_MANSION:
	case E_HOUSE:
	    load_house (Level->environment, FALSE);
	    break;
	case E_DLAIR:
	    load_dlair (gamestatusp (KILLED_DRAGONLORD), FALSE);
	    break;
	case E_STARPEAK:
	    load_speak (gamestatusp (KILLED_LAWBRINGER), FALSE);
	    break;
	case E_MAGIC_ISLE:
	    load_misle (gamestatusp (KILLED_EATER), FALSE);
	    break;
	case E_TEMPLE:
	    load_temple (Country[LastCountryLocX][LastCountryLocY].aux, FALSE);
	    break;
	case E_CIRCLE:
	    load_circle (FALSE);
	    break;
	case E_COURT:
	    load_court (FALSE);
	    break;
	default:
	    print3 ("This dungeon not implemented!");
	    break;
    }
    if (Level->depth > 0) {	// dungeon...
	install_traps();
	install_specials();
	make_stairs (-1);
	make_stairs (-1);
	initrand (E_RESTORE, 0);
    }
    Current_Environment = temp_env;
    is >> ios::align(alignof(i)) >> i >> j;
    while (j < MAXLENGTH && i < MAXWIDTH) {
	is >> run;
	for (; i < run; i++) {
	    is.read (&Level->site[i][j], sizeof(Level->site[i][j]));
	    Level->site[i][j].creature = NULL;
	    Level->site[i][j].things = NULL;
	}
	is >> ios::align(alignof(i)) >> i >> j;
    }
    run = 0;
    is.align (alignof(mask));
    for (j = 0; j < MAXLENGTH; j++)
	for (i = 0; i < MAXWIDTH; i++) {
	    if (run == 0) {
		run = 8 * sizeof(mask);
		is >> mask;
	    }
	    if (mask & 1)
		lset (i, j, SEEN);
	    mask >>= 1;
	    run--;
	}
    restore_monsters (is, Level, ver);
    is >> ios::align(alignof(i)) >> i >> j;
    while (j < MAXLENGTH && i < MAXWIDTH) {
	Level->site[i][j].things = restore_itemlist (is, ver);
	is >> ios::align(alignof(i)) >> i >> j;
    }
}

static void save_monsters (ostream& os, pml ml)
{
    int nummonsters = 0;
    // First count monsters
    for (pml tml = ml; tml; tml = tml->next)
	nummonsters++;
    os << ios::align(alignof(nummonsters)) << nummonsters;

    // Now save monsters
    for (pml tml = ml; tml; tml = tml->next) {
	os.write (tml->m, sizeof(*(tml->m)));
	uint8_t type = 0;
	if (strcmp (tml->m->monstring, Monsters[tml->m->id].monstring))
	    type |= 1;
	if (strcmp (tml->m->corpsestr, Monsters[tml->m->id].corpsestr))
	    type |= 2;
	os << type;
	if (type & 1)
	    os.write_strz (tml->m->monstring);
	if (type & 2)
	    os.write_strz (tml->m->corpsestr);
	save_itemlist (os, tml->m->possessions);
    }
}

static void restore_monsters (istream& is, plv level, int ver)
{
    pml ml = NULL;
    int i, nummonsters;
    unsigned char type;
    string s;

    level->mlist = NULL;

    is >> ios::align(alignof(nummonsters)) >> nummonsters;

    for (i = 0; i < nummonsters; i++) {
	ml = ((pml) checkmalloc (sizeof (mltype)));
	ml->m = ((pmt) checkmalloc (sizeof (montype)));
	is.read (ml->m, sizeof(*(ml->m)));
	is >> type;
	if (ml->m->id >= ArraySize(Monsters))
	    throw runtime_error ("invalid monster");
	ml->m->monstring = Monsters[ml->m->id].monstring;
	ml->m->corpsestr = Monsters[ml->m->id].corpsestr;
	ml->m->meleestr = Monsters[ml->m->id].meleestr;
	if (type & 1) { is.read_strz (s); ml->m->monstring = salloc (s); }
	if (type & 2) { is.read_strz (s); ml->m->corpsestr = salloc (s); }
	ml->m->possessions = restore_itemlist (is, ver);
	if (ml->m->x >= MAXWIDTH || ml->m->y >= MAXLENGTH)
	    throw runtime_error ("invalid monster location");
	level->site[ml->m->x][ml->m->y].creature = ml->m;
	ml->next = level->mlist;
	level->mlist = ml;
    }
}

// Save o unless it's null, then save a special flag byte instead
// Use other values of flag byte to indicate what strings are saved
static void save_item (ostream& os, pob o)
{
    uint8_t type = 0;
    if (!o) { os << type; return; }
    type = 8;
    if (strcmp (o->objstr, Objects[o->id].objstr))	type |= 1;
    if (strcmp (o->truename, Objects[o->id].truename))	type |= 2;
    if (strcmp (o->cursestr, Objects[o->id].cursestr))	type |= 4;
    os << type;
    os.write (o, sizeof(*o));
    if (type & 1)	os.write_strz (o->objstr);
    if (type & 2)	os.write_strz (o->truename);
    if (type & 4)	os.write_strz (o->cursestr);
}

// Restore an item, the first byte tells us if it's NULL, and what strings
// have been saved as different from the typical
static pob restore_item (istream& is, int ver UNUSED)
{
    uint8_t type;
    is >> type;
    if (!type)
	return (NULL);
    pob obj = (pob) checkmalloc (sizeof (objtype));
    is.read (obj, sizeof(*obj));
    if (obj->id >= ArraySize(Objects))
	throw runtime_error ("invalid item");
    obj->objstr = Objects[obj->id].objstr;
    obj->truename = Objects[obj->id].truename;
    obj->cursestr = Objects[obj->id].cursestr;
    string s;
    if (type & 1) { is.read_strz (s); obj->objstr = salloc (s); }
    if (type & 2) { is.read_strz (s); obj->truename = salloc (s); }
    if (type & 4) { is.read_strz (s); obj->cursestr = salloc (s); }
    return obj;
}

static void save_itemlist (ostream& os, pol ol)
{
    uint32_t numitems = 0;
    for (pol tol = ol; tol != NULL; tol = tol->next)
	numitems++;
    os << ios::align(alignof(numitems)) << numitems;
    for (pol tol = ol; tol != NULL; tol = tol->next)
	save_item (os, tol->thing);
}

static pol restore_itemlist (istream& is, int ver)
{
    pol ol = NULL, cur = NULL, o = NULL;
    int numitems, firsttime = TRUE;
    is >> ios::align(alignof(numitems)) >> numitems;
    for (int i = 0; i < numitems; i++) {
	o = (pol) checkmalloc (sizeof (oltype));
	o->thing = restore_item (is, ver);
	o->next = NULL;
	if (firsttime == TRUE) {
	    ol = cur = o;
	    firsttime = FALSE;
	} else {
	    cur->next = o;
	    cur = o;
	}
    }
    return (ol);
}

static void save_country (ostream& os)
{
    unsigned i, j;
    for (i = 0; i < MAXWIDTH; i++) {
	for (j = 0; j < MAXLENGTH; j++) {
	    if (c_statusp (i, j, CHANGED)) {
		os << ios::align(alignof(i)) << i << j;
		os.write (&Country[i][j], sizeof(Country[i][j]));
	    }
	}
    }
    os << ios::align(alignof(i)) << i << j;
    // since we don't mark the 'seen' bits as CHANGED, need to save a bitmask
    unsigned long mask = 0;
    int run = 8*sizeof (mask);
    os.align (alignof(mask));
    for (i = 0; i < MAXWIDTH; i++) {
	for (j = 0; j < MAXLENGTH; j++) {
	    if (run == 0) {
		run = 8 * sizeof(mask);
		os << mask;
		mask = 0;
	    }
	    mask >>= 1;
	    if (c_statusp (i, j, SEEN))
		mask |= (1UL << (sizeof(mask)*8 - 1));
	    --run;
	}
    }
    if (run < (int)(8 * sizeof(long int)))
	os << mask;
}

static void restore_country (istream& is, int ver UNUSED)
{
    int i, j;
    int run;
    unsigned long int mask = 0;

    load_country();
    is >> ios::align(alignof(i)) >> i >> j;
    while (i < MAXWIDTH && j < MAXLENGTH) {
	is.read (&Country[i][j], sizeof(Country[i][j]));
	is >> ios::align(alignof(i)) >> i >> j;
    }
    run = 0;
    is.align (alignof(mask));
    for (i = 0; i < MAXWIDTH; i++) {
	for (j = 0; j < MAXLENGTH; j++) {
	    if (run == 0) {
		run = 8*sizeof(mask);
		is >> mask;
	    }
	    if (mask & 1)
		c_set (i, j, SEEN);
	    mask >>= 1;
	    run--;
	}
    }
}
