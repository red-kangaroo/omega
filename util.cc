#include "glob.h"
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

//----------------------------------------------------------------------

static int spaceok(int i, int j, int baux);

//----------------------------------------------------------------------

// x and y on level?
int inbounds (int x, int y)
{
    return (x >= 0 && y >= 0 && x < (int)Level->width && y < (int)Level->height);
}

int random_range (int k)
{
    return (k == 0 ? 0 : rand() % (unsigned)k);
}

// modify absolute y coord relative to which part of level we are on
int screenmod (int y)
{
    return (y - ScreenOffset);
}

int offscreen (int y)
{
    return (y < 0 || y < ScreenOffset || y > ScreenOffset + ScreenLength - 1 || y > (int)Level->height);
}

// always hit on a natural 0; never hit on a natural 19
int hitp (int hit, int ac)
{
    int roll = random_range (20);
    if (roll == 0)
	return (TRUE);
    else if (roll == 19)
	return (FALSE);
    else
	return (roll < (hit - ac));
}

// number of moves from x1,y1 to x2,y2
int distance (int x1, int y1, int x2, int y2)
{
    return (max (absv (x2 - x1), absv (y2 - y1)));
}

// can you shoot, or move monsters through a spot?
int unblocked (int x, int y)
{
    return (inbounds (x, y) &&
	!Level->creature(x,y) &&
	Level->site(x,y).locchar != WALL &&
	Level->site(x,y).locchar != PORTCULLIS &&
	Level->site(x,y).locchar != STATUE &&
	Level->site(x,y).locchar != HEDGE &&
	Level->site(x,y).locchar != CLOSED_DOOR &&
	!loc_statusp (x, y, SECRET) && (x != Player.x || y != Player.y));
}

// do monsters want to move through a spot
int m_unblocked (struct monster *m, int x, int y)
{
    if ((!inbounds (x, y)) || ((x == Player.x) && (y == Player.y)))
	return (FALSE);
    else if ((Level->creature(x,y) != NULL) || (Level->site(x,y).locchar == SPACE))
	return (FALSE);
    else if (m_statusp (m, ONLYSWIM))
	return (Level->site(x,y).locchar == WATER);
    else if (loc_statusp (x, y, SECRET)) {
	if (m->movef == M_MOVE_SMART) {
	    if (los_p (x, y, Player.x, Player.y)) {
		mprint ("You see a secret door swing open!");
		lreset (x, y, SECRET);
		lset (x, y, CHANGED);
	    } else
		mprint ("You hear a door creak open, and then close again.");
	    // smart monsters would close secret doors behind them if the
	    // player didn't see them using it
	    return (TRUE);
	} else
	    return (m_statusp (m, INTANGIBLE));
    } else if ((Level->site(x,y).locchar == FLOOR) || (Level->site(x,y).locchar == OPEN_DOOR))
	return (TRUE);
    else if ((Level->site(x,y).locchar == PORTCULLIS) || (Level->site(x,y).locchar == WALL) || (Level->site(x,y).locchar == STATUE))
	return (m_statusp (m, INTANGIBLE));
    else if (Level->site(x,y).locchar == WATER)
	return (m_statusp (m, SWIMMING) || m_statusp (m, ONLYSWIM) || m_statusp (m, INTANGIBLE) || m_statusp (m, FLYING));
    else if (Level->site(x,y).locchar == CLOSED_DOOR) {
	if (m->movef == M_MOVE_SMART) {
	    mprint ("You hear a door creak open.");
	    Level->site(x,y).locchar = OPEN_DOOR;
	    lset (x, y, CHANGED);
	    return (TRUE);
	} else if (random_range (m->dmg) > random_range (100)) {
	    mprint ("You hear a door shattering.");
	    Level->site(x,y).locchar = RUBBLE;
	    lset (x, y, CHANGED);
	    return (TRUE);
	} else
	    return (m_statusp (m, INTANGIBLE));
    } else if (Level->site(x,y).locchar == LAVA)
	return ((m_immunityp (m, FLAME) && m_statusp (m, SWIMMING)) || m_statusp (m, INTANGIBLE) || m_statusp (m, FLYING));
    else if (Level->site(x,y).locchar == FIRE)
	return (m_statusp (m, INTANGIBLE) || m_immunityp (m, FLAME));
    else if ((Level->site(x,y).locchar == TRAP) || (Level->site(x,y).locchar == HEDGE) || (Level->site(x,y).locchar == ABYSS))
	return ((m->movef == M_MOVE_CONFUSED) || m_statusp (m, INTANGIBLE) || m_statusp (m, FLYING));
    else
	return (TRUE);
}

// can you see through a spot?
int view_unblocked (int x, int y)
{
    if (!inbounds (x, y))
	return (FALSE);
    else if ((Level->site(x,y).locchar == WALL) || (Level->site(x,y).locchar == STATUE) || (Level->site(x,y).locchar == HEDGE) || (Level->site(x,y).locchar == FIRE) || (Level->site(x,y).locchar == CLOSED_DOOR) || loc_statusp (x, y, SECRET))
	return (FALSE);
    else
	return (TRUE);
}

// do_los moves pyx along a lineofsight from x1 to x2
// x1 and x2 are pointers because as a side effect they are changed
// to the final location of the pyx
void do_los (int pyx, int *x1, int *y1, int x2, int y2)
{
    int dx, dy, ox, oy;
    int major, minor;
    int error, delta, step;
    int blocked;

    if (x2 - *x1 < 0)
	dx = 5;
    else if (x2 - *x1 > 0)
	dx = 4;
    else
	dx = -1;
    if (y2 - *y1 < 0)
	dy = 7;
    else if (y2 - *y1 > 0)
	dy = 6;
    else
	dy = -1;
    if (absv (x2 - *x1) > absv (y2 - *y1)) {
	major = dx;
	minor = dy;
	step = absv (x2 - *x1);
	delta = 2 * absv (y2 - *y1);
    } else {
	major = dy;
	minor = dx;
	step = absv (y2 - *y1);
	delta = 2 * absv (x2 - *x1);
    }
    if (major == -1)		// x1,y2 already == x2,y2
	return;
    error = 0;
    do {
	ox = *x1;
	oy = *y1;
	*x1 += Dirs[0][major];
	*y1 += Dirs[1][major];
	error += delta;
	if (error > step) {	// don't need to check that minor >= 0
	    *x1 += Dirs[0][minor];
	    *y1 += Dirs[1][minor];
	    error -= 2 * step;
	}
	blocked = !unblocked (*x1, *y1);
	if (error < 0 && (*x1 != x2 || *y1 != y2) && blocked) {
	    *x1 -= Dirs[0][minor];
	    *y1 -= Dirs[1][minor];
	    error += 2 * step;
	    blocked = !unblocked (*x1, *y1);
	}
	Level->site(*x1,*y1).showchar = pyx;
	plotchar (pyx, *x1, *y1);
	plotspot (ox, oy, TRUE);
	napms (50);
    } while ((*x1 != x2 || *y1 != y2) && !blocked);
    plotspot (*x1, *y1, TRUE);
    levelrefresh();
}

// This is the same as do_los, except we stop before hitting nonliving obstructions
void do_object_los (int pyx, int *x1, int *y1, int x2, int y2)
{
    int dx, dy, ox, oy;
    int major, minor;
    int error, delta, step;
    int blocked;

    if (x2 - *x1 < 0)
	dx = 5;
    else if (x2 - *x1 > 0)
	dx = 4;
    else
	dx = -1;
    if (y2 - *y1 < 0)
	dy = 7;
    else if (y2 - *y1 > 0)
	dy = 6;
    else
	dy = -1;
    if (absv (x2 - *x1) > absv (y2 - *y1)) {
	major = dx;
	minor = dy;
	step = absv (x2 - *x1);
	delta = 2 * absv (y2 - *y1);
    } else {
	major = dy;
	minor = dx;
	step = absv (y2 - *y1);
	delta = 2 * absv (x2 - *x1);
    }
    if (major == -1)		// x1,y2 already == x2,y2
	return;
    error = 0;
    do {
	ox = *x1;
	oy = *y1;
	*x1 += Dirs[0][major];
	*y1 += Dirs[1][major];
	error += delta;
	if (error > step) {	// don't need to check that minor >= 0
	    *x1 += Dirs[0][minor];
	    *y1 += Dirs[1][minor];
	    error -= 2 * step;
	}
	blocked = !unblocked (*x1, *y1);
	if (error < 0 && (*x1 != x2 || *y1 != y2) && blocked) {
	    *x1 -= Dirs[0][minor];
	    *y1 -= Dirs[1][minor];
	    error += 2 * step;
	    blocked = !unblocked (*x1, *y1);
	}
	plotspot (ox, oy, TRUE);
	if (unblocked (*x1, *y1)) {
	    plotchar (pyx, *x1, *y1);
	    Level->site(*x1,*y1).showchar = pyx;
	    napms (50);
	}
    } while ((*x1 != x2 || *y1 != y2) && !blocked);
    if (!Level->creature(*x1,*y1) && blocked) {
	*x1 = ox;
	*y1 = oy;
    }
    plotspot (*x1, *y1, TRUE);
    levelrefresh();
}

// los_p checks to see whether there is an unblocked los from x1,y1 to x2,y2
int los_p (int x1, int y1, int x2, int y2)
{
    int dx, dy;
    int major, minor;
    int error, delta, step;
    int blocked;

    if (x2 - x1 < 0)
	dx = 5;
    else if (x2 - x1 > 0)
	dx = 4;
    else
	dx = -1;
    if (y2 - y1 < 0)
	dy = 7;
    else if (y2 - y1 > 0)
	dy = 6;
    else
	dy = -1;
    if (absv (x2 - x1) > absv (y2 - y1)) {
	major = dx;
	minor = dy;
	step = absv (x2 - x1);
	delta = 2 * absv (y2 - y1);
    } else {
	major = dy;
	minor = dx;
	step = absv (y2 - y1);
	delta = 2 * absv (x2 - x1);
    }
    if (major == -1)		// x1,y2 already == x2,y2
	return TRUE;
    error = 0;
    do {
	x1 += Dirs[0][major];
	y1 += Dirs[1][major];
	error += delta;
	if (error > step) {	// don't need to check that minor >= 0
	    x1 += Dirs[0][minor];
	    y1 += Dirs[1][minor];
	    error -= 2 * step;
	}
	blocked = !unblocked (x1, y1);
	if (error < 0 && (x1 != x2 || y1 != y2) && blocked) {
	    x1 -= Dirs[0][minor];
	    y1 -= Dirs[1][minor];
	    error += 2 * step;
	    blocked = !unblocked (x1, y1);
	}
    } while ((x1 != x2 || y1 != y2) && !blocked);
    return ((x1 == x2) && (y1 == y2));
}

// view_los_p sees through monsters
int view_los_p (int x1, int y1, int x2, int y2)
{
    int dx, dy;
    int major, minor;
    int error, delta, step;
    int blocked;

    if (x2 - x1 < 0)
	dx = 5;
    else if (x2 - x1 > 0)
	dx = 4;
    else
	dx = -1;
    if (y2 - y1 < 0)
	dy = 7;
    else if (y2 - y1 > 0)
	dy = 6;
    else
	dy = -1;
    if (absv (x2 - x1) > absv (y2 - y1)) {
	major = dx;
	minor = dy;
	step = absv (x2 - x1);
	delta = 2 * absv (y2 - y1);
    } else {
	major = dy;
	minor = dx;
	step = absv (y2 - y1);
	delta = 2 * absv (x2 - x1);
    }
    if (major == -1)		// x1,y2 already == x2,y2
	return TRUE;
    error = 0;
    do {
	x1 += Dirs[0][major];
	y1 += Dirs[1][major];
	error += delta;
	if (error > step) {
	    x1 += Dirs[0][minor];
	    y1 += Dirs[1][minor];
	    error -= 2 * step;
	}
	blocked = !view_unblocked (x1, y1);
	if (error < 0 && (x1 != x2 || y1 != y2) && blocked) {
	    x1 -= Dirs[0][minor];
	    y1 -= Dirs[1][minor];
	    error += 2 * step;
	    blocked = !view_unblocked (x1, y1);
	}
    } while ((x1 != x2 || y1 != y2) && !blocked);
    return ((x1 == x2) && (y1 == y2));
}

long calc_points (void)
{
    long points = 0;
    if (gamestatusp (SPOKE_TO_DRUID))
	points += 50;
    if (gamestatusp (COMPLETED_CAVES))
	points += 100;
    if (gamestatusp (COMPLETED_SEWERS))
	points += 200;
    if (gamestatusp (COMPLETED_CASTLE))
	points += 300;
    if (gamestatusp (COMPLETED_ASTRAL))
	points += 400;
    if (gamestatusp (COMPLETED_VOLCANO))
	points += 500;
    if (gamestatusp (KILLED_DRAGONLORD))
	points += 100;
    if (gamestatusp (KILLED_EATER))
	points += 100;
    if (gamestatusp (KILLED_LAWBRINGER))
	points += 100;

    points += Player.xp / 50;

    points += Player.cash / 500;

    for (unsigned i = 0; i < MAXITEMS; i++)
	if (Player.has_possession(i))
	    points += Player.possessions[i].level * (object_is_known(Player.possessions[i]) + 1);

    foreach (i, Player.pack)
	points += i->level * (object_is_known(i) + 1);

    for (unsigned i = 0; i < NUMRANKS; i++) {
	if (Player.rank[i] == 5)
	    points += 500;
	else
	    points += 20 * Player.rank[i];
    }

    if (Player.hp < 1)
	points = (points / 2);

    else if (Player.rank[ADEPT])
	points *= 10;

    return (points);
}

// returns the 24 hour clock hour
int hour (void)
{
    return ((int) (((Time + 720) / 60) % 24));
}

// returns 0, 10, 20, 30, 40, or 50
int showminute (void)
{
    return ((int) ((Time % 60) / 10) * 10);
}

// returns the 12 hour clock hour
int showhour (void)
{
    int showtime;
    if ((hour() == 0) || (hour() == 12))
	showtime = 12;
    else
	showtime = (hour() % 12);
    return (showtime);
}

// nighttime is defined from 9 PM to 6AM
int nighttime (void)
{
    return ((hour() > 20) || (hour() < 7));
}

const char* getarticle (const char* str)
{
    static const char _ans[] = "aAeEiIoOuUhH";
    return (strchr(_ans, str[0]) ? "an " : "a ");
}

int day (void)
{
    return ((Date % 30) + 1);
}

const char* ordinal (int number)
{
    static const char _ends[4][3] = {"st","nd","rd","th"};
    return (_ends[min(3,(number >= 11 && number <= 13)?3:number%10)]);
}

const char* month (void)
{
    static const char _months[] =
	"Freeze\0" "Ice\0" "Mud\0" "Storm\0" "Breeze\0"
	"Light\0" "Flame\0" "Broil\0" "Cool\0" "Haunt\0"
	"Chill\0" "Dark\0" "Twixt\0";
    return (zstrn (_months, (Date % 360) / 30, 13));
}

// WDT: code for the following two functions contributed by Sheldon 
// Simms.
// finds floor space on level with buildaux not equal to baux,
// sets x,y there. There must *be* floor space somewhere on level....
static inline int spaceok (int i, int j, int baux)
{
    return (Level->site(i,j).locchar == FLOOR &&
	    !Level->creature(i,j) &&
	    !loc_statusp (i, j, SECRET) &&
	    Level->site(i,j).buildaux != baux);
}

void findspace (int* x, int* y, int baux)
{
    unsigned i, j;
    while (!spaceok (i = random_range(Level->width), j = random_range(Level->height), baux)) ;
    *x = i;
    *y = j;
}

int confirmation (void)
{
    static const char _areyousure[] =
	"Are you sure? [yn] \0"
	"Certain about that? [yn] \0"
	"Do you really mean it? [yn] \0"
	"Confirm that, would you? [yn] \0";
    mprint (zstrn (_areyousure, random_range(4), 4));
    return (ynq() == 'y');
}

// is character c a member of string s
bool strmem (int c, const char* s)
{
    for (unsigned i = 0; i < strlen(s); ++i)
	if (s[i] == c)
	    return (true);
    return (false);
}

void calc_weight (void)
{
    int weight = 0;
    for (int i = 1; i < MAXITEMS; i++)
	if (Player.has_possession(i))
	    weight += Player.possessions[i].weight * Player.possessions[i].number;
    foreach (i, Player.pack)
	weight += i->weight * i->number;
    Player.itemweight = weight;
    dataprint();
}

// returns true if its ok to get rid of a level
int ok_to_free (plv level)
{
    return (level &&
	    level->environment != E_COUNTRYSIDE &&
	    level->environment != E_CITY &&
	    level->environment != E_VILLAGE &&
	    level->environment != Current_Dungeon);
}

// Free up monsters and items on a level
void free_level (plv level)
{
    foreach (m, level->mlist)
	m->possessions.clear();
    level->mlist.clear();
    level->things.clear();
    delete level;
}

// there are various ways for the player to receive one of these hints
void hint (void)
{
    static const char _hints[] =
	"There is an entrance to the sewers in the Garden.\0"
	"Statues can be dangerous.\0"
	"Unidentified Artifacts can be dangerous.\0"
	"The higher form of mercury is desirable.\0"
	"A sense of unease is a good thing to have.\0"
	"If you dig too much, you might cause a cave-in!\0"
	"Be Lawful: Live and Let Live.\0"
	"Be Chaotic: Live and Let Die.\0"
	"The world doesn't slow down; you speed up.\0"
	"Security is a sense of dislocation.\0"
	"Tullimore Dew is a panacea.\0"
	"Thieves hide behind closed doors.\0"
	"`No jail is escapeproof' -- John Dillinger.\0"
	"Oh, to have an apartment of your own!\0"
	"Some homes have money and treasure.\0"
	"Some homes are defended.\0"
	"Sometimes you could just wish for Death.\0"
	"A cursed wish can be fatal.\0"
	"The way you play, you should wish for Skill.\0"
	"A druid might wish for Balance.\0"
	"Mages always wish for Knowledge.\0"
	"Some fairies are good.\0"
	"An affair with a demon can be heartbreaking.\0"
	"The Explorer's Club knows a useful spell.\0"
	"They say some famous people live in mansions.\0"
	"Magic pools are totally random.\0"
	"There are five elements, including Void.\0"
	"Humans can be good or evil, lawful or chaotic.\0"
	"There are many kinds of wishes. Case counts, you know.\0"
	"There are caves due south of Rampart\0"
	"Donaldson's Giants can withstand lava.\0"
	"Ritual magic can have many different effects.\0"
	"The Mercenaries are the best equipped fighters.\0"
	"The Gladiators are the most skilled fighters.\0"
	"Rent a flat and lose any bad stati you may have.\0"
	"Some junk may be worth a fortune if identified.\0"
	"Identify humans by talking to them.\0"
	"They say the Duke has a treasure trove.\0"
	"If you yield, your opponent will gain experience.\0"
	"The Dragon Lord lives in the Waste of Time.\0"
	"A full moon bodes well for the followers of Law.\0"
	"A new moon omens evil for the Law-abiding.\0"
	"Druids revere the half-moon.\0"
	"Most grot is useless.\0"
	"Cash can sometimes be found in the walls.\0"
	"Pointy weapons break often but dig better.\0"
	"The DREADED AQUAE MORTIS is invulnerable.\0"
	"There must be *some* reason to worship Destiny!\0"
	"Kill a trifid? A puzzle! Try a saline solution!\0"
	"Beware! The Eater of Souls inhabits the abyss!\0"
	"They say there's a red-light district in town.\0"
	"The House of the Eclipse is behind a closed door.\0"
	"The Orbs may be encountered on the Astral Plane.\0"
	"The Champion should never refuse a challenge.\0"
	"They say that the autoteller program is buggy.\0"
	"It's better not to sleep on the ground.\0"
	"Try ritual magic in different kinds of rooms.\0"
	"Breaking down a wall by bashing it is a bad idea!\0"
	"Follow the Oracle's advice; she is all-wise.\0"
	"The ArchDruid lives in the northern forest.\0"
	"A search of the mountains may reveal a secret pass.\0"
	"Star Peak is to the far North-East.\0"
	"The Archmage lives in the far North-West beyond a pass.\0"
	"There is a volcano in the southern marshes.\0"
	"The Demon Emperor resides in the Volcano.\0"
	"The Lawgiver can be found at Star Peak.\0"
	"The Temple of Athena is to the North-East.\0"
	"The Temple of Set can be found in a desert.\0"
	"The Temple of Hecate is in the swamp.\0"
	"The Temple of Odin is to the South in some mountains.\0"
	"There is a curious island off a promontory of the swamp.\0"
	"The Eater of Magic can be found on an island.\0"
	"The Temple of Destiny is practically inaccessible.\0"
	"Each sect has its own main temple outside the city.\0"
	"The aligned temples are dangerous to unbelievers.\0"
	"If you are poor, maybe you should wish for Wealth.\0"
	"Need mana? Wish for Power.\0"
	"Wishing for Law, Balance, or Chaos alters alignment.\0"
	"Feeling out of sorts? Wish for Health.\0"
	"Challenge the abyss at the Temple of Destiny.\0"
	"The Circle of Sorcerors has an Astral HQ\0"
	"The Star Gem is the only way back from the Astral Plane.\0"
	"The Star Gem is guarded by the Circle of Sorcerors.\0"
	"The Star Gem is rightfully the property of the LawBringer.\0"
	"They say the Demon Emperor owns the Amulet of the Planes.\0"
	"An Amulet might get you to the Temple of Destiny.\0"
	"A wish for Location might help you become Adept.\0"
	"Some Artifacts may be used only once per day.\0"
	"Overusing Artifacts can be a bad move.\0"
	"You might starve in the Swamp or the Mountains!\0"
	"You would have to be very chaotic to attack a guard!\0"
	"You would have to be very foolhardy to attack a guard!\0"
	"Only a master of chaos would kill all the city guards!\0"
	"The Order depends on the force of the LawGiver.\0"
	"City Guards are employees of the Order.\0"
	"Disenchanted Artifacts can be restored.\0";
    mprint (zstrn (_hints, random_range(96), 96));
}

// for when a deity teaches spells to a devotee
void learnclericalspells (int deity, int level)
{
    mprint ("With your new clerical rank comes knowledge of magic...");
    Player.pow += level;
    Player.maxpow += level;
    switch (level) {
	case LAY:
	    if (deity == ODIN)
		learn_spell (S_MISSILE);
	    else if (deity == SET)
		learn_spell (S_INVISIBLE);
	    else if (deity == ATHENA)
		learn_spell (S_IDENTIFY);
	    else if (deity == HECATE)
		learn_spell (S_DRAIN);
	    else if (deity == DRUID) {
		learn_spell (S_KNOWLEDGE);
		learn_spell (S_MON_DET);
	    }
	    break;
	case ACOLYTE:
	    if (deity == ODIN) {
		learn_spell (S_LBALL);
		learn_spell (S_TRUESIGHT);
	    } else if (deity == SET) {
		learn_spell (S_SUMMON);
		learn_spell (S_FIREBOLT);
	    } else if (deity == ATHENA) {
		learn_spell (S_HEAL);
		learn_spell (S_SANCTUARY);
	    } else if (deity == HECATE) {
		learn_spell (S_SLEEP);
		learn_spell (S_DISPEL);
	    } else if (deity == DRUID) {
		learn_spell (S_HEAL);
		learn_spell (S_CURE);
	    } else if (deity == DESTINY)
		mprint ("An acolyte of the Lords of Destiny. Gee whiz.");
	    break;
	case PRIEST:
	    learn_spell (S_SANCTIFY);
	    if (deity == ODIN) {
		learn_spell (S_HERO);
		learn_spell (S_HEAL);
	    } else if (deity == SET) {
		learn_spell (S_INVISIBLE);
		learn_spell (S_DISPEL);
	    } else if (deity == ATHENA) {
		learn_spell (S_REGENERATE);
		learn_spell (S_ACCURACY);
	    } else if (deity == HECATE) {
		learn_spell (S_SHADOWFORM);
		learn_spell (S_CURE);
	    } else if (deity == DRUID) {
		learn_spell (S_DISRUPT);
		learn_spell (S_ALERT);
		learn_spell (S_CLAIRVOYANCE);
	    } else if (deity == DESTINY)
		mprint ("How useless, a new priest of the Lords of Destiny.");
	    break;
	case SPRIEST:
	    learn_spell (S_BLESS);
	    if (deity == ODIN)
		learn_spell (S_ACCURACY);
	    else if (deity == SET)
		learn_spell (S_SHADOWFORM);
	    else if (deity == ATHENA)
		learn_spell (S_HERO);
	    else if (deity == HECATE)
		learn_spell (S_POLYMORPH);
	    else if (deity == DRUID) {
		learn_spell (S_POLYMORPH);
		learn_spell (S_LEVITATE);
	    } else if (deity == DESTINY)
		mprint ("Wow, a new senior priest of the Lords of Destiny.");
	    break;
	case HIGHPRIEST:
	    if (deity == ODIN)
		learn_spell (S_RESTORE);
	    else if (deity == SET)
		learn_spell (S_HELLFIRE);
	    else if (deity == ATHENA)
		learn_spell (S_HELLFIRE);
	    else if (deity == HECATE)
		learn_spell (S_DESECRATE);
	    else if (deity == DRUID) {
		learn_spell (S_DISINTEGRATE);
		learn_spell (S_HERO);
	    } else if (deity == DESTINY) {
		mprint ("So you're now the high priest of the Lords of Destiny.");
		mprint ("You didn't think you were going to get anything, did you?");
	    }
    }
}

// for the use of the casino slot machine
const char* slotstr (int num)
{
    static const char _slots[] =
	"<Slime Mold>\0" "<Lemon>\0" "<Copper>\0" "<Nymph>\0" "<Sword>\0"
	"<Shield>\0" "<Chest>\0" "<Bar>\0" "<Orb>\0" "<Mithril Nugget>\0";
    return (zstrn (_slots, num, 10));
}

// random names for various uses
const char* nameprint (void)
{
    static const char _names[] = 
	"Orion Splash\0" "Gorgar\0" "Hieronymous\0" "Quantifor Quotron\0" "Leon\0"
	"Joyce\0" "Leticia Smiley\0" "Ogilvy the Grim\0" "Salara Storn\0" "Murgo\0"
	"Jonathan Atwilder\0" "Xylos the Tan\0" "Terence\0" "Toronado\0" "Kelly\0"
	"Cantinflas\0" "Ixel\0" "Toto\0" "Frost\0" "Aliera Erinyes\0"
	"Godel\0" "Kerst Blackblade\0" "Ebenezer\0" "Jeremiah\0" "Triskelion Shadow\0"
	"Eleskir Eremar\0" "Tyron\0" "Morgon\0" "Achmed\0" "Chin\0"
	"Fujimoto\0" "Dos Santos\0" "Federico\0" "Jaime\0" "Siobhan\0"
	"Hans\0" "Gurkov\0" "Krilos the Slayer\0" "Oxxblud\0" "Dorian\0";
    return (zstrn (_names, random_range(40), 40));
}

// returns english string equivalent of number
const char* wordnum (int num)
{
    static const char _numnames[] =
	"zero \0" "one \0" "two \0" "three \0" "four \0"
	"five \0" "six \0" "seven \0" "eight \0" "nine \0"
	"ten \0";
    return (zstrn (_numnames, num, 11));
}
