#include "glob.h"
#include <sys/types.h>
#include <unistd.h>

//----------------------------------------------------------------------

static void initstats(void);
static void save_omegarc(void);
static void load_omegarc (const char* filename);
static int competence_check(int attack);
static void user_character_stats(void);
static void omegan_character_stats(void);
static bool personal_question_yn (const char* q);

//----------------------------------------------------------------------

// set player to begin with
player::player (void)
: player_pod()
, rank()
, immunity()
, status()
, guildxp()
, name("Player")
, meleestr(64)
, possessions()
, pack()
{
}

void initplayer (void)
{
    Player.itemweight = 0;
    Player.food = 36;
    Behavior = -1;
    Player.options = 0;
    fill (Player.immunity, 0);
    fill (Player.status, 0);
    fill (Player.rank, 0);
    fill (Player.guildxp, 0);
    for (auto& p : Player.possessions)
	p.id = NO_THING;
    Player.pack.clear();
    Player.patron = 0;
    Player.alignment = 0;
    Player.cash = 250;

    bool oldchar = false;
    snprintf (ArrayBlock(Str1), OMEGA_PLAYER_FILE, getenv("HOME"));
    if (0 == access (Str1, R_OK) && personal_question_yn ("Use saved character? [yn] ")) {
	clearmsg();
	try {
	    load_omegarc (Str1);
	    oldchar = true;
	} catch (exception& e) {
	    print1 (e.what());
	    morewait();
	}
    }
    if (!oldchar) {
	optionset (RUNSTOP);
	optionset (CONFIRM);
	initstats();
    }
    Searchnum = max (1, min (9, Searchnum));
    Player.hp = Player.maxhp = Player.maxcon;
    Player.mana = Player.maxmana = Player.calcmana();
    Player.click = 1;
    Player.meleestr = "CCBC";
    calc_melee();
    ScreenOffset = -1000;	// to force a redraw
}

static void initstats (void)
{
    do {
	print1 ("Do you want to run a character [c] or play yourself [p]?");
	char response = (char) mcigetc();
	if (response == 'c')
	    omegan_character_stats();
	else if (response == 'p') {
	    user_character_stats();
	    print1 ("Do you want to save this set-up to .omegarc in your home directory? [yn] ");
	    if (ynq1() == 'y') {
		print1 ("First, set options.");
		setoptions();
		try {
		    save_omegarc();
		} catch (exception& e) {
		    print1 (e.what());
		    morewait();
		}
	    }
	    displayfile (Data_Intro);
	} else
	    continue;
    } while (false);
    xredraw();
}

template <typename Stm>
static inline void omegarc_serialize_player_static (Stm& stm)
{
    stm & Player.iq & Player.maxiq
	& Player.pow & Player.maxpow
	& Player.dex & Player.maxdex
	& Player.agi & Player.maxagi
	& Player.str & Player.maxstr
	& Player.con & Player.maxcon
	& Player.cash & Player.options
	& Player.preference;
}

template <typename Stm>
static inline void omegarc_write (Stm& stm)
{
    const uint32_t fmt = OMEGA_PLAYER_FORMAT;
    const uint32_t savefmt = OMEGA_SAVE_FORMAT;
    stm << fmt << savefmt << Searchnum << Verbosity << Player.name;
    omegarc_serialize_player_static (stm);
}

static void save_omegarc (void)
{
    snprintf (ArrayBlock(Str1), OMEGA_PLAYER_FILE, getenv("HOME"));
    mkpath (Str1);
    sizestream ss;
    omegarc_write (ss);
    memblock buf (ss.pos());
    ostream os (buf);
    omegarc_write (os);
    buf.write_file (Str1);
}

static void load_omegarc (const char* filename)
{
    memblock buf;
    buf.read_file (filename);
    istream is (buf);

    uint32_t fmt, savefmt;
    is.verify_remaining ("load_omegarc", stream_size_of(fmt)+stream_size_of(savefmt)+stream_size_of(Searchnum)+stream_size_of(Verbosity));
    is >> fmt >> savefmt >> Searchnum >> Verbosity;
    if (fmt != OMEGA_PLAYER_FORMAT)
	runtime_error::emit ("omegarc format is not readable");
    is >> Player.name;
    sizestream ss;
    omegarc_serialize_player_static (ss);
    is.verify_remaining ("load_omegarc", ss.pos());
    omegarc_serialize_player_static (is);
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
    ability += Player.level/5;
    ability = min(9,max(0,ability));
    return (ability);
}

static bool personal_question_yn (const char* q)
    { print1 (q); return (ynq1() == 'y'); }
static unsigned personal_question_num (const char* q)
    { print1 (q); return (parsenum()); }

static void user_character_stats (void)
{
    int num, iqpts = 0, numints = 0, ok, agipts = 0, dexpts = 0, powpts = 0, conpts = 0;
    print1 ("OK, now try to answer the following questions honestly:");
    morewait();
    num = personal_question_num ("How many pounds can you bench press? ");
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

    if (personal_question_yn ("Took an official IQ test? [yn] ")) {
	num = personal_question_num ("So, whadja get? ") / 10;
	if (num > 18) {
	    print2 ("Even if it's true, I don't believe it.");
	    morewait();
	    clearmsg();
	    num = 18;
	}
	iqpts += num;
	numints++;
    }

    if (personal_question_yn ("Took Undergraduate entrance exams? [yn] ")) {
	do {
	    num = personal_question_num ("So, what percentile? ");
	    ok = (num < 100);
	    if (!ok) {
		print2 ("That's impossible!");
		morewait();
		clearmsg();
	    }
	} while (!ok);
	iqpts += (num - 49) * 9 / 50 + 9;
	++numints;
    }
    if (personal_question_yn ("Took Graduate entrance exams? [yn] ")) {
	do {
	    num = personal_question_num ("So, what percentile? ");
	    ok = (num < 100);
	    if (!ok) {
		print2 ("That's impossible!");
		morewait();
		clearmsg();
	    }
	} while (!ok);
	iqpts += (num - 49) * 9 / 50 + 9;
	++numints;
    }

    if (!numints) {
	if (personal_question_yn ("Pretty dumb, aren't you? [yn] ")) {
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
    if (personal_question_yn ("Can you dance? [yn] ")) {
	++agipts;
	if (personal_question_yn (" Well? [yn] "))
	    agipts += 2;
    }
    if (personal_question_yn ("Do you have training in a martial art or gymnastics? [yn] ")) {
	agipts += 2;
	if (personal_question_yn ("Do you have dan rank or equivalent? [yn] "))
	    agipts += 4;
    }
    clearmsg();
    if (personal_question_yn ("Do you play some field sport? [yn] ")) {
	++agipts;
	if (personal_question_yn (" Are you good? [yn] "))
	    ++agipts;
    }
    if (personal_question_yn ("Do you cave, mountaineer, etc.? [yn] "))
	agipts += 3;
    if (personal_question_yn ("Do you skate or ski? [yn] ")) {
	agipts += 2;
	if (personal_question_yn (" Well? [yn] "))
	    agipts += 2;
    }
    if (personal_question_yn ("Are you physically handicapped? [yn] "))
	agipts -= 4;
    if (personal_question_yn ("Are you accident prone? [yn] "))
	agipts -= 4;
    if (!personal_question_yn ("Can you use a bicycle? [yn] "))
	agipts -= 4;
    Player.agi = Player.maxagi = 9 + agipts / 2;
    if (personal_question_yn ("Do you play video games? [yn] ")) {
	dexpts += 2;
	if (personal_question_yn ("Do you get high scores? [yn] "))
	    dexpts += 4;
    }
    clearmsg();
    if (personal_question_yn ("Are you an archer, fencer, or marksman? [yn] ")) {
	dexpts += 2;
	if (personal_question_yn ("A good one? [yn] "))
	    dexpts += 4;
    }
    clearmsg();
    if (personal_question_yn ("Have you ever picked a lock? [yn] ")) {
	dexpts += 2;
	print2 ("Really. Well, the police are being notified.");
    }
    morewait();
    clearmsg();
    num = personal_question_num ("What's your typing speed (words per minute) ");
    if (num > 125) {
	print2 ("Tell me another one....");
	morewait();
	clearmsg();
	num = 125;
    }
    dexpts += num / 25;
    if (personal_question_yn ("Hold your arm out. Tense your fist. Hand shaking? [yn] "))
	dexpts -= 3;
    if (personal_question_yn ("Ambidextrous, are you? [yn] "))
	dexpts += 4;
    if (personal_question_yn ("Can you cut a deck of cards with one hand? [yn] "))
	dexpts += 2;
    if (personal_question_yn ("Can you tie your shoes blindfolded? [yn] "))
	dexpts -= 3;
    Player.dex = Player.maxdex = 6 + dexpts / 2;
    if (!personal_question_yn ("Do you ever get colds? [yn] "))
	conpts += 4;
    else if (personal_question_yn (" Frequently? [yn] "))
	conpts -= 4;
    if (personal_question_yn ("Had any serious accident or illness this year? [yn] "))
	conpts -= 4;
    else
	conpts += 4;
    if (personal_question_yn ("Have a chronic disease? [yn] "))
	conpts -= 4;
    if (personal_question_yn ("Overweight or underweight by more than 20 percent? [yn] "))
	conpts -= 2;
    if (personal_question_yn ("High Blood Pressure? [yn] "))
	conpts -= 2;
    if (personal_question_yn ("Smoke? [yn] "))
	conpts -= 3;
    if (personal_question_yn ("Take aerobics classes? [yn] "))
	conpts += 2;
    num = personal_question_num ("How many miles can you run? ");
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
    if (personal_question_yn ("Do animals react oddly to your presence? [yn] ")) {
	print2 ("How curious that must be.");
	morewait();
	clearmsg();
	powpts += 2;
    }
    if (personal_question_yn ("Can you see auras? [yn] ")) {
	nprint1 (" How strange.");
	morewait();
	powpts += 3;
    }
    if (personal_question_yn ("Ever have an out-of-body experience? [yn] ")) {
	print2 ("Wow, man. Fly the friendly skies....");
	morewait();
	clearmsg();
	powpts += 3;
    }
    if (personal_question_yn ("Did you ever cast a spell? [yn] ")) {
	powpts += 3;
	if (personal_question_yn (" Did it work? [yn] ")) {
	    powpts += 7;
	    print2 ("Sure it did.");
	    morewait();
	    clearmsg();
	}
    }
    if (personal_question_yn ("Do you have ESP? [yn] ")) {
	powpts += 3;
	print2 ("Somehow, I knew you were going to say that.");
	morewait();
	clearmsg();
    }
    if (personal_question_yn ("Do you have PK? [yn] ")) {
	powpts += 6;
	print2 ("I can't tell you how much that moves me.");
	morewait();
	clearmsg();
    }
    if (personal_question_yn ("Do you believe in ghosts? [yn] ")) {
	powpts += 2;
	print2 ("I do! I do! I do believe in ghosts!");
	morewait();
	clearmsg();
    }
    if (personal_question_yn ("Are you Irish? [yn] ")) {
	powpts += 2;
	nprint1 (" Is that blarney or what?");
	morewait();
    }
    Player.pow = Player.maxpow = 3 + powpts / 2;
    clearmsg();
    print1 ("Please enter your character's name: ");
    Player.name = msgscanstring();
    Player.name[0] = toupper (Player.name[0]);
    print1 ("Are you sexually interested in males or females? [mf] ");
    do
	Player.preference = (char) mcigetc();
    while (Player.preference != 'm' && Player.preference != 'f' && Player.preference != 'y' && Player.preference != 'n');	// :-)
}

static void omegan_character_stats (void)
{
    print1 ("To reroll hit ESCAPE; hit any other key to accept these stats.");
    unsigned i = 0;
    do {
	++i;
	sprintf (Str1, "You have only %d chance%s to reroll... ", REROLLS - i, (i == (REROLLS - 1)) ? "" : "s");
	print2 (Str1);
	unsigned share;	// Baseline for related stats
	Player.iq  = Player.maxiq  = 4 + random_range(5) + (share = random_range(12));
	Player.pow = Player.maxpow = 4 + random_range(5) + share;
	Player.dex = Player.maxdex = 4 + random_range(5) + (share = random_range(12));
	Player.agi = Player.maxagi = 4 + random_range(5) + share;
	Player.str = Player.maxstr = 4 + random_range(5) + (share = random_range(12));
	Player.con = Player.maxcon = 4 + random_range(5) + share;
	Player.cash = random_range (500);
	calc_melee();
	dataprint();
    } while (i < REROLLS && mgetc() == KEY_ESCAPE);
    clearmsg();
    print1 ("Please enter your character's name: ");
    Player.name = msgscanstring();
    Player.name[0] = toupper (Player.name[0]);
    print1 ("Is your character sexually interested in males or females? [mf] ");
    do
	Player.preference = (char) mcigetc();
    while (Player.preference != 'm' && Player.preference != 'f' && Player.preference != 'y' && Player.preference != 'n');	// :-)
}
