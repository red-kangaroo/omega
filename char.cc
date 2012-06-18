#include "glob.h"
#include <sys/types.h>
#include <unistd.h>

//----------------------------------------------------------------------

static FILE* omegarc_check(void);
static void initstats(void);
static void save_omegarc(void);
static int competence_check(int attack);
static void user_character_stats(void);
static void omegan_character_stats(void);

//----------------------------------------------------------------------

// set player to begin with
player::player (void)
: player_pod()
, rank()
, immunity()
, status()
, guildxp()
, name("player")
, meleestr(64)
, possessions()
, pack()
{
}

void initplayer (void)
{
    const char* lname = getlogin();
    if (lname && *lname)
	Player.name = lname;
    if (Player.name[0] >= 'a' && Player.name[0] <= 'z')
	Player.name[0] += 'A' - 'a';	// capitalise 1st letter
    Player.itemweight = 0;
    Player.food = 36;
    Behavior = -1;
    Player.options = 0;
    fill (Player.immunity, 0);
    fill (Player.status, 0);
    fill (Player.rank, 0);
    fill (Player.guildxp, 0);
    for (auto p : Player.possessions)
	p.id = NO_THING;
    Player.pack.clear();
    Player.patron = 0;
    Player.alignment = 0;
    Player.cash = 250;
    FILE* fd;
    int oldchar = false;
    if ((fd = omegarc_check()) != NULL) {
	int i;
	fread ((char*) &i, sizeof (int), 1, fd);
	if (i != OMEGA_VERSION) {
	    print1 ("Out of date .omegarc! Make another!");
	    morewait();
	} else {
	    oldchar = true;
	    fread ((char*) &Player, sizeof (Player), 1, fd);
	    fread ((char*) &Searchnum, sizeof (int), 1, fd);
	    fread ((char*) &Verbosity, sizeof (char), 1, fd);
	    Player.name = lname;
	    if (Player.name[0] >= 'a' && Player.name[0] <= 'z')
		Player.name[0] += 'A' - 'a';	// capitalise 1st letter
	}
	fclose (fd);
    }
    if (!oldchar) {
	optionset (RUNSTOP);
	optionset (CONFIRM);
	initstats();
    }
    Searchnum = max (1, min (9, Searchnum));
    Player.hp = Player.maxhp = Player.maxcon;
    Player.mana = Player.maxmana = calcmana();
    Player.click = 1;
    strcpy (Player.meleestr, "CCBC");
    calc_melee();
    ScreenOffset = -1000;	// to force a redraw
}

static FILE* omegarc_check (void)
{
    FILE *fd;
    sprintf (Str1, "%s/.omegarc", getenv ("HOME"));
    if ((fd = fopen (Str1, "r")) != NULL) {
	print2 ("Use .omegarc in home directory? [yn] ");
	if (ynq2() != 'y') {
	    fclose (fd);
	    fd = NULL;
	}
    }
    clearmsg();
    return (fd);
}

static void initstats (void)
{
    char response;
    print1 ("Do you want to run a character [c] or play yourself [p]?");
    do
	response = (char) mcigetc();
    while ((response != 'c') && (response != 'p'));
    if (response == 'c')
	omegan_character_stats();
    else {
	user_character_stats();
	print1 ("Do you want to save this set-up to .omegarc in your home directory? [yn] ");
	if (ynq1() == 'y')
	    save_omegarc();
	displayfile (Data_Intro);
    }
    xredraw();
}

static void save_omegarc (void)
{
    int i = OMEGA_VERSION;
    FILE *fd;
    sprintf (Str1, "%s/.omegarc", getenv ("HOME"));
    fd = fopen (Str1, "w");
    if (fd == NULL)
	print1 ("Sorry, couldn't save .omegarc for some reason.");
    else {
	fwrite ((const char* ) &i, sizeof (int), 1, fd);
	print1 ("First, set options.");
	setoptions();
	fwrite ((const char* ) &Player, sizeof (Player), 1, fd);
	fwrite ((const char* ) &Searchnum, sizeof (int), 1, fd);
	fwrite ((const char* ) &Verbosity, sizeof (char), 1, fd);
	fclose (fd);
    }
}

long calcmana (void)
{
    return (Player.pow * (long) (Player.level + 1));
}

//  npcbehavior digits 1234
//
// 4 : alignment (LAWFUL,CHAOTIC, or NEUTRAL)
// 3 : primary combat action (melee,missile,spell,thief,flight,1..5)
// 2 : competence at 4 (0..9, 0 = incompetent, 9 = masterful)
// 1 : conversation mode
// 
// status : 1 = dead, 2 = saved, 3 = retired, 4 = still playing
int fixnpc (int status)
{
    int npcbehavior = 0;
    char response;
    if (status == 1) {		// player is dead, all undead are chaotic
	npcbehavior += CHAOTIC;
	npcbehavior += 10;	// melee
	npcbehavior += 100 * min (9, ((int) (Player.level / 3)));
	npcbehavior += 1000;	// threaten
    } else if (Behavior >= 0)
	npcbehavior = Behavior;
    else {
	menuclear();
	menuprint ("NPC Behavior Determination Module\n\n");
	menuprint ("Your overall NPC behavior is:");
	if (Player.alignment < -10) {
	    npcbehavior += CHAOTIC;
	    menuprint ("\n\n CHAOTIC");
	} else if (Player.alignment > 10) {
	    npcbehavior += LAWFUL;
	    menuprint ("\n\n LAWFUL");
	} else {
	    npcbehavior += NEUTRAL;
	    menuprint ("\n\n NEUTRAL");
	}
	menuprint ("\n\n1: hand-to-hand combat");
	menuprint ("\n2: missile combat");
	menuprint ("\n3: spellcasting");
	menuprint ("\n4: thieving");
	menuprint ("\n5: escape");
	menuprint ("\n\nEnter NPC response to combat: ");
	showmenu();
	response = '0';
	while ((response != '1') && (response != '2') && (response != '3') && (response != '4') && (response != '5'))
	    response = menugetc();
	menuaddch (response);
	npcbehavior += 10 * (response - '0');
	npcbehavior += 100 * competence_check (response - '0');
	response = '0';
	menuclear();
	menuprint ("1: threaten");
	menuprint ("\n2: greet");
	menuprint ("\n3: aid");
	menuprint ("\n4: beg");
	menuprint ("\n5: silence");
	menuprint ("\n\nEnter NPC response to conversation: ");
	showmenu();
	while ((response != '1') && (response != '2') && (response != '3') && (response != '4') && (response != '5'))
	    response = menugetc();
	menuaddch (response);
	npcbehavior += 1000 * (response - '0');
	xredraw();
    }
    Behavior = npcbehavior;
    return (npcbehavior);
}

// estimates on a 0..9 scale how good a player is at something
static int competence_check (int attack)
{
    int ability = 0;
    switch (attack) {
	case 1:		// melee
	    ability += statmod (Player.str);
	case 2:		// missle
	    ability += statmod (Player.dex);
	    ability += Player.rank[LEGION];
	    ability += ((int) (Player.dmg / 10) - 1);
	    break;
	case 3:		// spellcasting
	    ability += statmod (Player.iq);
	    ability += statmod (Player.pow);
	    ability += Player.rank[CIRCLE];
	    ability += Player.rank[COLLEGE];
	    ability += Player.rank[PRIEST];
	    break;
	case 4:		// thieving
	    ability += statmod (Player.dex);
	    ability += statmod (Player.agi);
	    ability += Player.rank[THIEVES];
	    break;
	case 5:		// escape
	    ability += 2 * statmod (Player.agi);
	    break;
    }
    ability += ((int) (Player.level / 5));
    if (ability < 0)
	ability = 0;
    if (ability > 9)
	ability = 9;
    return (ability);
}

static void user_character_stats (void)
{
    int num, iqpts = 0, numints = 0, ok, agipts = 0, dexpts = 0, powpts = 0, conpts = 0;
    print1 ("OK, now try to answer the following questions honestly:");
    morewait();
    print1 ("How many pounds can you bench press? ");
    num = (int) parsenum();
    if (num < 30)
	Player.str = Player.maxstr = 3;
    else if (num < 90)
	Player.str = Player.maxstr = num / 10;
    else
	Player.str = Player.maxstr = 9 + ((num - 120) / 30);
    if (Player.str > 18) {
	print2 ("Even if it's true, I don't believe it.");
	morewait();
	clearmsg();
	Player.str = Player.maxstr = 18;
    }

    print1 ("Took an official IQ test? [yn] ");
    if (ynq1() == 'y') {
	print1 ("So, whadja get? ");
	num = (int) parsenum() / 10;
	if (num > 18) {
	    print2 ("Even if it's true, I don't believe it.");
	    morewait();
	    clearmsg();
	    num = 18;
	}
	iqpts += num;
	numints++;
    }

    print1 ("Took Undergraduate entrance exams? [yn] ");
    if (ynq1() == 'y') {
	do {
	    print1 ("So, what percentile? ");
	    num = (int) parsenum();
	    ok = (num < 100);
	    if (!ok) {
		print2 ("That's impossible!");
		morewait();
		clearmsg();
	    }
	} while (!ok);
	iqpts += (num - 49) * 9 / 50 + 9;
	numints++;
    }
    print1 ("Took Graduate entrance exams? [yn] ");
    if (ynq1() == 'y') {
	do {
	    print1 ("So, what percentile? ");
	    num = (int) parsenum();
	    ok = (num < 100);
	    if (!ok) {
		print2 ("That's impossible!");
		morewait();
		clearmsg();
	    }
	} while (!ok);
	iqpts += (num - 49) * 9 / 50 + 9;
	numints++;
    }

    if (numints == 0) {
	print1 ("Pretty dumb, aren't you? [yn] ");
	if (ynq1() == 'y') {
	    Player.iq = random_range (3) + 3;
	    print2 ("I thought so....");
	} else {
	    Player.iq = random_range (6) + 8;
	    print2 ("Well, not *that* dumb.");
	}
	morewait();
	clearmsg();
    } else
	Player.iq = iqpts / numints;
    Player.maxiq = Player.iq;
    agipts = 0;
    print1 ("Can you dance? [yn] ");
    if (ynq1() == 'y') {
	agipts++;
	nprint1 (" Well? [yn] ");
	if (ynq1() == 'y')
	    agipts += 2;
    }
    print1 ("Do you have training in a martial art or gymnastics? [yn] ");
    if (ynq1() == 'y') {
	agipts += 2;
	print2 ("Do you have dan rank or equivalent? [yn] ");
	if (ynq2() == 'y')
	    agipts += 4;
    }
    clearmsg();
    print1 ("Do you play some field sport? [yn] ");
    if (ynq1() == 'y') {
	agipts++;
	nprint1 (" Are you good? [yn] ");
	if (ynq1() == 'y')
	    agipts++;
    }
    print1 ("Do you cave, mountaineer, etc.? [yn] ");
    if (ynq1() == 'y')
	agipts += 3;
    print1 ("Do you skate or ski? [yn] ");
    if (ynq1() == 'y') {
	agipts += 2;
	nprint1 (" Well? [yn] ");
	if (ynq1() == 'y')
	    agipts += 2;
    }
    print1 ("Are you physically handicapped? [yn] ");
    if (ynq1() == 'y')
	agipts -= 4;
    print1 ("Are you accident prone? [yn] ");
    if (ynq1() == 'y')
	agipts -= 4;
    print1 ("Can you use a bicycle? [yn] ");
    if (ynq1() != 'y')
	agipts -= 4;
    Player.agi = Player.maxagi = 9 + agipts / 2;
    print1 ("Do you play video games? [yn] ");
    if (ynq1() == 'y') {
	dexpts += 2;
	print2 ("Do you get high scores? [yn] ");
	if (ynq2() == 'y')
	    dexpts += 4;
    }
    clearmsg();
    print1 ("Are you an archer, fencer, or marksman? [yn] ");
    if (ynq1() == 'y') {
	dexpts += 2;
	print2 ("A good one? [yn] ");
	if (ynq2() == 'y')
	    dexpts += 4;
    }
    clearmsg();
    print1 ("Have you ever picked a lock? [yn] ");
    if (ynq1() == 'y') {
	dexpts += 2;
	print2 ("Really. Well, the police are being notified.");
    }
    morewait();
    clearmsg();
    print1 ("What's your typing speed (words per minute) ");
    num = (int) parsenum();
    if (num > 125) {
	print2 ("Tell me another one....");
	morewait();
	clearmsg();
	num = 125;
    }
    dexpts += num / 25;
    print1 ("Hold your arm out. Tense your fist. Hand shaking? [yn] ");
    if (ynq1() == 'y')
	dexpts -= 3;
    print1 ("Ambidextrous, are you? [yn] ");
    if (ynq1() == 'y')
	dexpts += 4;
    print1 ("Can you cut a deck of cards with one hand? [yn] ");
    if (ynq1() == 'y')
	dexpts += 2;
    print1 ("Can you tie your shoes blindfolded? [yn] ");
    if (ynq1() != 'y')
	dexpts -= 3;
    Player.dex = Player.maxdex = 6 + dexpts / 2;
    print1 ("Do you ever get colds? [yn] ");
    if (ynq1() != 'y')
	conpts += 4;
    else {
	nprint1 (" Frequently? [yn] ");
	if (ynq1() == 'y')
	    conpts -= 4;
    }
    print1 ("Had any serious accident or illness this year? [yn] ");
    if (ynq1() == 'y')
	conpts -= 4;
    else
	conpts += 4;
    print1 ("Have a chronic disease? [yn] ");
    if (ynq1() == 'y')
	conpts -= 4;
    print1 ("Overweight or underweight by more than 20 percent? [yn] ");
    if (ynq1() == 'y')
	conpts -= 2;
    print1 ("High Blood Pressure? [yn] ");
    if (ynq1() == 'y')
	conpts -= 2;
    print1 ("Smoke? [yn] ");
    if (ynq1() == 'y')
	conpts -= 3;
    print1 ("Take aerobics classes? [yn] ");
    if (ynq1() == 'y')
	conpts += 2;
    print1 ("How many miles can you run? ");
    num = (int) parsenum();
    if (num > 25) {
	print2 ("Right. Sure. Give me a break.");
	morewait();
	clearmsg();
	conpts += 8;
    } else if (num < 1)
	conpts -= 3;
    else if (num < 5)
	conpts += 2;
    else if (num < 10)
	conpts += 4;
    else
	conpts += 8;
    Player.con = Player.maxcon = 12 + conpts / 3;
    print1 ("Do animals react oddly to your presence? [yn] ");
    if (ynq1() == 'y') {
	print2 ("How curious that must be.");
	morewait();
	clearmsg();
	powpts += 2;
    }
    print1 ("Can you see auras? [yn] ");
    if (ynq1() == 'y') {
	nprint1 (" How strange.");
	morewait();
	powpts += 3;
    }
    print1 ("Ever have an out-of-body experience? [yn] ");
    if (ynq1() == 'y') {
	print2 ("Wow, man. Fly the friendly skies....");
	morewait();
	clearmsg();
	powpts += 3;
    }
    print1 ("Did you ever cast a spell? [yn] ");
    if (ynq1() == 'y') {
	powpts += 3;
	nprint1 (" Did it work? [yn] ");
	if (ynq1() == 'y') {
	    powpts += 7;
	    print2 ("Sure it did.");
	    morewait();
	    clearmsg();
	}
    }
    print1 ("Do you have ESP? [yn] ");
    if (ynq1() == 'y') {
	powpts += 3;
	print2 ("Somehow, I knew you were going to say that.");
	morewait();
	clearmsg();
    }
    print1 ("Do you have PK? [yn] ");
    if (ynq1() == 'y') {
	powpts += 6;
	print2 ("I can't tell you how much that moves me.");
	morewait();
	clearmsg();
    }
    print1 ("Do you believe in ghosts? [yn] ");
    if (ynq1() == 'y') {
	powpts += 2;
	print2 ("I do! I do! I do believe in ghosts!");
	morewait();
	clearmsg();
    }
    print1 ("Are you Irish? [yn] ");
    if (ynq1() == 'y') {
	powpts += 2;
	nprint1 (" Is that blarney or what?");
	morewait();
    }
    Player.pow = Player.maxpow = 3 + powpts / 2;
    print1 ("Are you sexually interested in males or females? [mf] ");
    do
	Player.preference = (char) mcigetc();
    while ((Player.preference != 'm') && (Player.preference != 'f') && (Player.preference != 'y') && (Player.preference != 'n'));	// :-)
}

static void omegan_character_stats (void)
{
    int share1, share2, i = 0;
    print1 ("To reroll hit ESCAPE; hit any other key to accept these stats.");
    do {
	i++;
	sprintf (Str1, "You have only %d chance%s to reroll... ", REROLLS - i, (i == (REROLLS - 1)) ? "" : "s");
	print2 (Str1);
	Player.iq = Player.maxiq = 4 + random_range (5) + (share1 = random_range (6)) + (share2 = random_range (6));
	Player.pow = Player.maxpow = 4 + random_range (5) + share1 + share2;
	Player.dex = Player.maxdex = 4 + random_range (5) + (share1 = random_range (6)) + (share2 = random_range (6));
	Player.agi = Player.maxagi = 4 + random_range (5) + share1 + share2;
	Player.str = Player.maxstr = 4 + random_range (5) + (share1 = random_range (6)) + (share2 = random_range (6));
	Player.con = Player.maxcon = 4 + random_range (5) + share1 + share2;
	Player.cash = random_range (100) + random_range (100) + random_range (100) + random_range (100) + random_range (100);
	calc_melee();
	dataprint();
    } while ((i < REROLLS) && (mgetc() == KEY_ESCAPE));
    clearmsg();
    print1 ("Please enter your character's name: ");
    Player.name = msgscanstring();
    if (Player.name[0] >= 'a' && Player.name[0] <= 'z')
	Player.name[0] += 'A' - 'a';	// capitalise 1st letter
    print1 ("Is your character sexually interested in males or females? [mf] ");
    do
	Player.preference = (char) mcigetc();
    while ((Player.preference != 'm') && (Player.preference != 'f') && (Player.preference != 'y') && (Player.preference != 'n'));	// :-)

}
