#include "glob.h"
#include <unistd.h>
#include <stdlib.h>

//----------------------------------------------------------------------

static void restore_country(istream& is);
static void restore_level(istream& is);
static void save_country(ostream& os);
static void save_level(ostream& os, plv level);

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

	os << Player;
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

	is >> Player;
	restore_country (is);
	restore_level (is);	// the city level
	int i;
	is >> i;
	for (; i > 0; i--) {
	    restore_level (is);
	    if (Level->environment == Current_Dungeon) {
		Level->next = Dungeon;
		Dungeon = Level;
	    }
	    if (Current_Environment == E_CITY)
		Level = City;
	}
	// this disgusting kludge because LENGTH and WIDTH are globals...
	Level->width = 64;
	switch (Current_Environment) {
	    case E_COURT:
		Level->height = 24;
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
		Level->height = 16;
		break;
	    default:
		Level->height = 64;
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
void player::write (ostream& os) const
{
    player_pod::write (os);
    os << rank;
    os.skipalign (stream_align(immunity));
    os << immunity << status << guildxp;
    os.skipalign (stream_align(name));
    os << name << meleestr;
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
    os.skipalign (stream_align (possessions));
    os << possessions << pack;
    os << Pawnitems << Condoitems;
}

void player::read (istream& is)
{
    player_pod::read (is);
    is >> rank;
    is.align (stream_align(immunity));
    is >> immunity >> status >> guildxp;
    is.align (stream_align(name));
    is >> name >> meleestr;
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

    is.align (stream_align (possessions));
    is >> possessions >> pack;
    is >> Pawnitems >> Condoitems;
}

// Save whatever is pointed to by level
static void save_level (ostream& os, plv level)
{
    unsigned run;
    uint16_t i, j;
    os << ios::align(alignof(level->environment)) << level->environment << level->depth << level->numrooms << level->tunnelled;
    for (j = 0; j < MAXLENGTH; j++) {
	for (i = 0; i < MAXWIDTH; i++) {
	    if (level->site(i,j).lstatus & CHANGED) {	// this loc has been changed
		for (run = i + 1; run < MAXWIDTH &&	// find how many in a row
		     level->site(run,j).lstatus & CHANGED; run++);
		os << ios::align(alignof(i)) << i << j << run;
		for (; i < run; i++)
		    os << level->site(i,j);
	    }
	}
    }
    os << ios::align(alignof(i)) << i << j;	// signify end
    // since we don't mark the 'seen' bits as CHANGED, need to save a bitmask
    uint32_t mask = 0;
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
	    if (level->site(i,j).lstatus & SEEN)
		mask |= (1UL << (sizeof(mask)*8 - 1));
	    run--;
	}
    }
    if (run < 8*sizeof (mask))
	os << mask;
    os << level->mlist << level->things;
}

static void restore_level (istream& is)
{
    unsigned run;
    uint16_t i, j;
    uint32_t mask = 0;
    int temp_env;

    Level = new level;
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
	for (; i < run; i++)
	    is >> Level->site(i,j);
	is >> ios::align(alignof(i)) >> i >> j;
    }
    run = 0;
    is.align (alignof(mask));
    for (j = 0; j < MAXLENGTH; j++) {
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
    }
    is >> Level->mlist >> Level->things;
}

void monster::write (ostream& os) const
{
    monster_data::write (os);
    os << attacked << aux1 << aux2 << click << x << y;
    if (strcmp (monstring, Monsters[id].monstring))
	os.write_strz (monstring);
    else
	os << '\0';
    if (strcmp (corpsestr, Monsters[id].corpsestr))
	os.write_strz (corpsestr);
    else
	os << '\0';
    os.skipalign (stream_align(possessions));
    os << possessions;
}

void monster::read (istream& is)
{
    monster_data::read (is);
    if (id >= ArraySize(Monsters))
	throw runtime_error ("invalid monster");
    is >> attacked >> aux1 >> aux2 >> click >> x >> y;
    monstring = Monsters[id].monstring;
    corpsestr = Monsters[id].corpsestr;
    meleestr = Monsters[id].meleestr;

    unsigned nlen;	// These create memory leaks, but they are cheaper than fixing them
    nlen = strlen(is.ipos()); if (nlen) monstring = strdup(is.ipos()); is.skip(nlen+1);
    nlen = strlen(is.ipos()); if (nlen) corpsestr = strdup(is.ipos()); is.skip(nlen+1);

    is.align (stream_align(possessions));
    is >> possessions;
}

streamsize monster::stream_size (void) const
{
    streamsize r = monster_data::stream_size();
    if (id >= ArraySize(Monsters)) return (r);
    ++r; if (monstring && strcmp (monstring, Monsters[id].monstring)) r += strlen(monstring);
    ++r; if (corpsestr && strcmp (corpsestr, Monsters[id].corpsestr)) r += strlen(corpsestr);
    r += stream_size_of(possessions);
    return (r);
}

// Save o unless it's null, then save a special flag byte instead
// Use other values of flag byte to indicate what strings are saved
void object::write (ostream& os) const
{
    if (id >= ArraySize(Objects)) { os << id; return; }
    object_data::write (os);
    os << number << x << y;
    if (strcmp (objstr, Objects[id].objstr))
	os.write_strz (objstr);
    else os << '\0';
    if (strcmp (truename, Objects[id].truename))
	os.write_strz (truename);
    else os << '\0';
    if (strcmp (cursestr, Objects[id].cursestr))
	os.write_strz (cursestr);
    else os << '\0';
    os.skipalign(stream_align(*this));
}

// Restore an item, the first byte tells us if it's NULL, and what strings
// have been saved as different from the typical
void object::read (istream& is)
{
    is >> id;
    if (id >= ArraySize(Objects)) return;
    is.iseek (is.ipos() - stream_size_of(id));
    object_data::read (is);
    is >> number >> x >> y;
    objstr = Objects[id].objstr;
    truename = Objects[id].truename;
    cursestr = Objects[id].cursestr;
    unsigned nlen;	// The strdups leak memory, but hey, it isn't much
    nlen = strlen(is.ipos()); if (nlen) objstr = strdup(is.ipos()); is.skip(nlen+1);
    nlen = strlen(is.ipos()); if (nlen) truename = strdup(is.ipos()); is.skip(nlen+1);
    nlen = strlen(is.ipos()); if (nlen) cursestr = strdup(is.ipos()); is.skip(nlen+1);
    is.align(stream_align(*this));
}

streamsize object::stream_size (void) const
{
    streamsize r = 0;
    if (id >= ArraySize(Objects)) return (stream_size_of(id));
    r += object_data::stream_size();
    r += stream_size_of(number) + stream_size_of(x) + stream_size_of(y);
    ++r; if (objstr && strcmp (objstr, Objects[id].objstr)) r += strlen(objstr);
    ++r; if (truename && strcmp (truename, Objects[id].truename)) r += strlen(truename);
    ++r; if (cursestr && strcmp (cursestr, Objects[id].cursestr)) r += strlen(truename);
    return (Align(r,stream_align(*this)));
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

static void restore_country (istream& is)
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
