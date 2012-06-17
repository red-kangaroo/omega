#include "glob.h"
#include <unistd.h>

//----------------------------------------------------------------------

static void l_air_station(void);
static void l_arena_exit(void);
static void l_balancestone(void);
static void l_chaostone(void);
static void l_circle_library(void);
static void l_drop_every_portcullis(void);
static void l_earth_station(void);
static void l_enter_circle(void);
static void l_enter_court(void);
static void l_escalator(void);
static void l_fire(void);
static void l_fire_station(void);
static void l_hedge(void);
static void l_house_exit(void);
static void l_lava(void);
static void l_lawstone(void);
static void l_lift(void);
static void l_magic_pool(void);
static void l_mindstone(void);
static void l_no_op(void);
static void l_rubble(void);
static void l_sacrificestone(void);
static void l_tactical_exit(void);
static void l_temple_warning(void);
static void l_throne(void);
static void l_tome1(void);
static void l_tome2(void);
static void l_voice1(void);
static void l_voice2(void);
static void l_voice3(void);
static void l_void(void);
static void l_void_station(void);
static void l_voidstone(void);
static void l_water(void);
static void l_water_station(void);
static void l_whirlwind(void);
static void stationcheck(void);

//----------------------------------------------------------------------

// various miscellaneous location functions
static void l_water (void)
{
    if (!gamestatusp (MOUNTED)) {
	if (Player.has_possession(O_ARMOR)) {
	    print1 ("Your heavy armor drags you under the water!");
	    morewait();
	    p_drown();
	    print2 ("You reach the surface again.");
	} else if (Player.itemweight > ((int) (Player.maxweight / 2))) {
	    print1 ("The weight of your burden drags you under water!");
	    morewait();
	    p_drown();
	    print2 ("You reach the surface again.");
	} else
	    switch (random_range (32)) {
		case 0: print1 ("Splish. Splash!"); break;
		case 1: print1 ("I want my ducky!"); break;
		case 2: print1 ("So finally you take a bath!"); break;
		case 3: print1 ("You must be very thirsty!"); break;
	    }
    } else {
	switch (random_range (32)) {
	    case 0: print1 ("Your horse frolics playfully in the water."); break;
	    case 1: print1 ("Your horse quenches its thirst."); break;
	    case 2: print1 ("Your steed begins to swim...."); break;
	    case 3: print1 ("Your mount thrashes about in the water."); break;
	}
    }
}

void l_chaos (void)
{
    if (gamestatusp (MOUNTED)) {
	print1 ("Your steed tries to swim in the raw Chaos, but seems to");
	print2 ("be having some difficulties...");
	morewait();
	print1 ("probably because it's just turned into a chaffinch.");
	morewait();
	resetgamestatus (MOUNTED);
    }
    if (!onewithchaos)
	print1 ("You are immersed in raw Chaos....");
    if (Player.rank[ADEPT]) {
	if (!onewithchaos) {
	    onewithchaos = 1;
	    print2 ("You achieve oneness of Chaos....");
	}
	Player.mana = max (Player.mana, calcmana());
	Player.hp = max (Player.hp, Player.maxhp);
    } else if (Player.rank[PRIESTHOOD] && !saved) {
	print2 ("A mysterious force protects you from the Chaos!");
	print3 ("Wow.... You feel a bit smug.");
	gain_experience (500);
	saved = true;
    } else {
	print2 ("Uh oh....");
	if (saved)
	    nprint2 ("Nothing mysterious happens this time....");
	morewait();
	print1 ("Congratulations! You've achieved maximal entropy!");
	Player.alignment -= 50;
	gain_experience (1000);
	p_death ("immersion in raw Chaos");
    }
}

static void l_hedge (void)
{
    if (Player.patron == DRUID)
	print1 ("You move through the hedges freely.");
    else {
	print1 ("You struggle in the brambly hedge... ");
	switch (random_range (6)) {
	    case 0:
		print2 ("You are stabbed by thorns!");
		p_damage (random_range (6), NORMAL_DAMAGE, "a hedge");
		print3 ("The thorns were poisonous!");
		p_poison (random_range (12));
		break;
	    case 1:
		print2 ("You are stabbed by thorns!");
		p_damage (random_range (12), NORMAL_DAMAGE, "a hedge");
		break;
	    case 2:
		print2 ("You seem to have gotten stuck in the hedge.");
		Player.status[IMMOBILE] += random_range (5) + 1;
		break;
	    case 3:
		if (Player.has_possession(O_CLOAK)) {
		    print2 ("Your cloak was torn on the brambles!");
		    Player.remove_possession (O_CLOAK);
		} else
		    print2 ("Ouch! These thorns are scratchy!");
		break;
	    default:
		print2 ("You make your way through unscathed.");
		break;
	}
    }
}

static void l_lava (void)
{
    print1 ("Very clever -- walking into a pool of lava...");
    if (gamestatusp (MOUNTED)) {
	print2 ("Your horse is incinerated... You fall in too!");
	resetgamestatus (MOUNTED);
    }
    morewait();
    if (Player.name == "Saltheart Foamfollower") {
	print1 ("Strangely enough, you don't seem terribly affected.");
	p_damage (1, UNSTOPPABLE, "slow death in a pool of lava");
    } else {
	p_damage (random_range (75), FLAME, "incineration in a pool of lava");
	if (Player.hp > 0)
	    p_drown();
	Player.status[IMMOBILE] += 2;
    }
}

static void l_fire (void)
{
    print1 ("You boldly stride through the curtain of fire...");
    if (gamestatusp (MOUNTED)) {
	print2 ("Your horse is fried and so are you...");
	resetgamestatus (MOUNTED);
    }
    p_damage (random_range (100), FLAME, "self-immolation");
}

void l_abyss (void)
{
    int i;
    if (Current_Environment != Current_Dungeon) {
	print1 ("You fall through a dimensional portal!");
	morewait();
	strategic_teleport (-1);
    } else {
	print1 ("You enter the infinite abyss!");
	morewait();
	if (random_range (100) == 13) {
	    print1 ("As you fall you see before you what seems like");
	    print2 ("an infinite congerie of iridescent bubbles.");
	    print3 ("You have met Yog Sothoth!!!");
	    morewait();
	    clearmsg();
	    if (Player.alignment > -10)
		p_death ("the Eater of Souls");
	    else {
		print1 ("The All-In-One must have taken pity on you.");
		print2 ("A transdimensional portal appears...");
		morewait();
		change_level (Level->depth, Level->depth + 1, false);
		gain_experience (2000);
		Player.alignment -= 50;
	    }
	} else {
	    i = 0;
	    print1 ("You fall...");
	    while (random_range (3) != 2) {
		if (i % 6 == 0)
		    print2 ("and fall... ");
		else
		    nprint2 ("and fall... ");
		i++;
		morewait();
	    }
	    i++;
	    print1 ("Finally,you emerge through an interdimensional interstice...");
	    morewait();
	    if (Level->depth + i > MaxDungeonLevels) {
		print2 ("You emerge high above the ground!!!!");
		print3 ("Yaaaaaaaah........");
		morewait();
		change_environment (E_COUNTRYSIDE);
		do {
		    Player.x = random_range (Level->width);
		    Player.y = random_range (Level->height);
		} while (Country->site(Player.x,Player.y).showchar() == CHAOS_SEA);
		p_damage (i * 50, NORMAL_DAMAGE, "a fall from a great height");
	    } else {
		print2 ("You built up some velocity during your fall, though....");
		morewait();
		p_damage (i * 5, NORMAL_DAMAGE, "a fall through the abyss");
		change_level (Level->depth, Level->depth + i, false);
		gain_experience (i * i * 50);
	    }
	}
    }
}

static void l_lift (void)
{
    char response;
    int levelnum;
    int too_far = 0;

    Level->site(Player.x,Player.y).locchar = FLOOR;
    Level->site(Player.x,Player.y).p_locf = L_NO_OP;
    lset (Player.x, Player.y, CHANGED);
    print1 ("You walk onto a shimmering disk....");
    print2 ("The disk vanishes, and a glow surrounds you.");
    print3 ("You feel weightless.... You feel ghostly....");
    morewait();
    clearmsg();
    print1 ("Go up, down, or neither [u,d,ESCAPE] ");
    do
	response = (char) mcigetc();
    while ((response != 'u') && (response != 'd') && (response != KEY_ESCAPE));
    if (response != KEY_ESCAPE) {
	print1 ("How many levels?");
	levelnum = (int) parsenum();
	if (levelnum > 6) {
	    too_far = 1;
	    levelnum = 6;
	}
	if (response == 'u' && Level->depth - levelnum < 1) {
	    int lDepth = levelnum - Level->depth;
	    change_environment (E_COUNTRYSIDE);	// "you return to the countryside."
	    if (lDepth > 0) {
		nprint1 ("..");
		print2 ("...and keep going up!  You hang in mid air...");
		morewait();
		print3 ("\"What goes up...\"");
		morewait();
		print3 ("Yaaaaaaaah........");
		p_damage (lDepth * 10, NORMAL_DAMAGE, "a fall from a great height");
	    }
	    return;
	} else if (response == 'd' && Level->depth + levelnum > MaxDungeonLevels) {
	    too_far = 1;
	    levelnum = MaxDungeonLevels - Level->depth;
	}
	if (levelnum == 0) {
	    print1 ("Nothing happens.");
	    return;
	}
	if (too_far) {
	    print1 ("The lift gives out partway...");
	    print2 ("You rematerialize.....");
	} else
	    print1 ("You rematerialize.....");
	change_level (Level->depth, (response == 'd' ? Level->depth + levelnum : Level->depth - levelnum), false);
	roomcheck();
    }
}

static void l_magic_pool (void)
{
    int possibilities = random_range (100);
    print1 ("This pool seems to be enchanted....");
    if (gamestatusp (MOUNTED)) {
	if (random_range (2)) {
	    print2 ("Your horse is polymorphed into a fig newton.");
	    resetgamestatus (MOUNTED);
	} else
	    print2 ("Whatever it was, your horse enjoyed it....");
    } else if (possibilities == 0) {
	print1 ("Oh no! You encounter the DREADED AQUAE MORTIS...");
	if (random_range (1000) < Player.level * Player.level * Player.level) {
	    print2 ("The DREADED AQUAE MORTIS throttles you within inches....");
	    print3 ("but for some reason chooses to let you escape.");
	    gain_experience (500);
	    Player.hp = 1;
	} else
	    p_death ("the DREADED AQUAE MORTIS!");
    } else if (possibilities < 25)
	augment (0);
    else if (possibilities < 30)
	augment (1);
    else if (possibilities < 60)
	augment (-1);
    else if (possibilities < 65)
	cleanse (1);
    else if (possibilities < 80) {
	if (Player.has_possession(O_WEAPON_HAND)) {
	    print1 ("You drop your weapon in the pool! It's gone forever!");
	    Player.remove_possession (O_WEAPON_HAND);
	} else
	    print1 ("You feel fortunate.");
    } else if (possibilities < 90) {
	if (Player.has_possession(O_WEAPON_HAND)) {
	    print1 ("Your weapon leaves the pool with a new edge....");
	    Player.possessions[O_WEAPON_HAND].plus += random_range (10) + 1;
	    calc_melee();
	} else
	    print1 ("You feel unfortunate.");
    } else if (possibilities < 95) {
	Player.hp += 10;
	print1 ("You feel healthier after the dip...");
    } else if (possibilities < 99) {
	print1 ("Oooh, a tainted pool...");
	p_poison (10);
    } else if (possibilities == 99) {
	print1 ("Wow! A pool of azoth!");
	heal (10);
	cleanse (1);
	Player.mana = calcmana() * 3;
	Player.str = (Player.maxstr++) * 3;
    }
    print2 ("The pool seems to have dried up.");
    Level->site(Player.x,Player.y).locchar = TRAP;
    Level->site(Player.x,Player.y).p_locf = L_TRAP_PIT;
    lset (Player.x, Player.y, CHANGED);
}

static void l_no_op (void)
{
}

static void l_tactical_exit (void)
{
    if (optionp (CONFIRM)) {
	clearmsg();
	print1 ("Do you really want to leave this place? ");
	if (ynq1() != 'y')
	    return;
    }
    // Free up monsters and items, and the level
    free_level (Level);
    Level = NULL;
    if ((Current_Environment == E_TEMPLE) || (Current_Environment == E_TACTICAL_MAP))
	change_environment (E_COUNTRYSIDE);
    else
	change_environment (Last_Environment);
}

static void l_rubble (void)
{
    int screwup = random_range (100) - (Player.agi + Player.level);
    print1 ("You climb over the unstable pile of rubble....");
    if (screwup < 0)
	print2 ("No problem!");
    else {
	print2 ("You tumble and fall in a small avalanche of debris!");
	print3 ("You're trapped in the pile!");
	Player.status[IMMOBILE] += 2;
	p_damage (screwup / 5, UNSTOPPABLE, "rubble and debris");
	morewait();
    }
}

// Drops all portcullises in 5 moves
void l_portcullis_trap (void)
{
    bool slam = false;
    print3 ("Click.");
    morewait();
    for (unsigned i = max (Player.x - 5, 0); i < min (6u+Player.x, Level->width); i++) {
	for (unsigned j = max (Player.y - 5, 0); j < min (6u+Player.y, Level->height); j++) {
	    if ((Level->site(i,j).p_locf == L_PORTCULLIS) && (Level->site(i,j).locchar != PORTCULLIS)) {
		Level->site(i,j).locchar = PORTCULLIS;
		lset (i, j, CHANGED);
		putspot (i, j, PORTCULLIS);
		if ((int)i == Player.x && (int)j == Player.y) {
		    print3 ("Smash! You've been hit by a falling portcullis!");
		    morewait();
		    p_damage (random_range (1000), NORMAL_DAMAGE, "a portcullis");
		}
		slam = true;
	    }
	}
    }
    if (slam)
	print3 ("You hear heavy walls slamming down!");
}

// drops every portcullis on level, then kills itself and all similar traps.
static void l_drop_every_portcullis (void)
{
    bool slam = false;
    print3 ("Click.");
    morewait();
    for (unsigned i = 0; i < Level->width; i++) {
	for (unsigned j = 0; j < Level->height; j++) {
	    if (Level->site(i,j).p_locf == L_DROP_EVERY_PORTCULLIS) {
		Level->site(i,j).p_locf = L_NO_OP;
		lset (i, j, CHANGED);
	    } else if ((Level->site(i,j).p_locf == L_PORTCULLIS) && (Level->site(i,j).locchar != PORTCULLIS)) {
		Level->site(i,j).locchar = PORTCULLIS;
		lset (i, j, CHANGED);
		putspot (i, j, PORTCULLIS);
		if ((int)i == Player.x && (int)j == Player.y) {
		    print3 ("Smash! You've been hit by a falling portcullis!");
		    morewait();
		    p_damage (random_range (1000), NORMAL_DAMAGE, "a portcullis");
		}
		slam = true;
	    }
	}
    }
    if (slam)
	print3 ("You hear heavy walls slamming down!");
}

void l_raise_portcullis (void)
{
    bool open = false;
    for (unsigned i = 0; i < Level->width; i++) {
	for (unsigned j = 0; j < Level->height; j++) {
	    if (Level->site(i,j).locchar == PORTCULLIS) {
		Level->site(i,j).locchar = FLOOR;
		lset (i, j, CHANGED);
		putspot (i, j, FLOOR);
		open = true;
	    }
	}
    }
    if (open)
	print1 ("You hear the sound of steel on stone!");
}

static void l_arena_exit (void)
{
    resetgamestatus (ARENA_MODE);
    free_level (Level);
    Level = NULL;
    change_environment (E_CITY);
}

static void l_house_exit (void)
{
    if (optionp (CONFIRM)) {
	clearmsg();
	print1 ("Do you really want to leave this abode? ");
	if (ynq1() != 'y')
	    return;
    }
    free_level (Level);
    Level = NULL;
    change_environment (Last_Environment);
}

static void l_void (void)
{
    clearmsg();
    print1 ("Geronimo!");
    morewait();
    clearmsg();
    print1 ("You leap into the void.");
    if (!Level->mlist.empty()) {
	print2 ("Death peers over the edge and gazes quizzically at you....");
	morewait();
	print3 ("'Bye-bye,' he says... 'We'll meet again.'");
    }
    morewait();
    while (Player.hp > 0) {
	Time += 60;
	hourly_check();
	napms (250);
    }
}

static void l_fire_station (void)
{
    print1 ("The flames leap up, and the heat is incredible.");
    if (Player.immunity[FLAME]) {
	print2 ("You feel the terrible heat despite your immunity to fire!");
	morewait();
    }
    print2 ("Enter the flames? [yn] ");
    if (ynq2() == 'y') {
	if (Player.hp == 1)
	    p_death ("total incineration");
	else
	    Player.hp = 1;
	dataprint();
	print1 ("You feel like you are being incinerated! Jump back? [yn] ");
	if (ynq1() == 'y')
	    print2 ("Phew! That was close!");
	else {
	    Player.pow -= (15 + random_range (15));
	    if (Player.pow > 0) {
		print2 ("That's odd, the flame seems to have cooled down now....");
		print3 ("A flicker of fire seems to dance above the void nearby.");
		Level->site(Player.x,Player.y).locchar = FLOOR;
		Level->site(Player.x,Player.y).p_locf = L_NO_OP;
		stationcheck();
	    } else {
		print2 ("The flames seem to have leached away all your mana!");
		p_death ("the Essence of Fire");
	    }
	}
    } else
	print2 ("You flinch away from the all-consuming fire.");
}

static void l_water_station (void)
{
    print1 ("The fluid seems murky and unknowably deep.");
    print2 ("It bubbles and hisses threateningly.");
    morewait();
    if (Player.status[BREATHING]) {
	print1 ("You don't feel sanguine about trying to breathe that stuff!");
	morewait();
    }
    if (Player.immunity[ACID]) {
	print2 ("The vapor burns despite your immunity to acid!");
	morewait();
    }
    print1 ("Enter the fluid? [yn] ");
    if (ynq1() == 'y') {
	if (Player.hp == 1)
	    p_death ("drowning in acid (ick, what a way to go)");
	else
	    Player.hp = 1;
	dataprint();
	print2 ("You choke....");
	morewait();
	nprint2 ("Your lungs burn....");
	morewait();
	print2 ("Your body begins to disintegrate.... Leave the pool? [yn] ");
	if (ynq2() == 'y')
	    print2 ("Phew! That was close!");
	else {
	    clearmsg();
	    Player.con -= (15 + random_range (15));
	    if (Player.con > 0) {
		print1 ("That's odd, the fluid seems to have been neutralized....");
		print2 ("A moist miasma wafts above the void nearby.");
		Level->site(Player.x,Player.y).locchar = FLOOR;
		Level->site(Player.x,Player.y).p_locf = L_NO_OP;
		stationcheck();
	    } else {
		print2 ("The bubbling fluid has destroyed your constitution!");
		p_death ("the Essence of Water");
	    }
	}

    } else
	print2 ("You step back from the pool of acid.");
}

static void l_air_station (void)
{
    print1 ("The whirlwind spins wildly and crackles with lightning.");
    if (Player.immunity[ELECTRICITY])
	print2 ("You feel static cling despite your immunity to electricity!");
    morewait();
    print1 ("Enter the storm? [yn] ");
    if (ynq1() == 'y') {
	if (Player.hp == 1)
	    p_death ("being torn apart and then electrocuted");
	else
	    Player.hp = 1;
	dataprint();
	print1 ("You are buffeted and burnt by the storm....");
	print2 ("You begin to lose consciousness.... Leave the storm? [yn] ");
	if (ynq1() == 'y')
	    print2 ("Phew! That was close!");
	else {
	    Player.iq -= (random_range (15) + 15);
	    if (Player.iq > 0) {
		print1 ("That's odd, the storm subsides....");
		print2 ("A gust of wind brushes past the void nearby.");
		Level->site(Player.x,Player.y).locchar = FLOOR;
		Level->site(Player.x,Player.y).p_locf = L_NO_OP;
		stationcheck();
	    } else {
		print2 ("The swirling storm has destroyed your intelligence!");
		p_death ("the Essence of Air");
	    }
	}
    } else
	print2 ("You step back from the ominous whirlwind.");
}

static void l_earth_station (void)
{
    print1 ("The tendrilled mass reaches out for you from the muddy ooze.");
    if (find_item (THING_SALT_WATER))
	print2 ("A splash of salt water does nothing to dissuade the vines.");
    morewait();
    print1 ("Enter the overgrown mire? [yn] ");
    if (ynq1() == 'y') {
	if (Player.hp == 1)
	    p_death ("being eaten alive");
	else
	    Player.hp = 1;
	dataprint();
	print1 ("You are being dragged into the muck. Suckers bite you....");
	print2 ("You're about to be entangled.... Leave the mud? [yn] ");
	if (ynq2() == 'y')
	    print2 ("Phew! That was close!");
	else {
	    Player.str -= (15 + random_range (15));
	    if (Player.str > 0) {
		print1 ("That's odd, the vine withdraws....");
		print2 ("A spatter of dirt sprays into the void nearby.");
		Level->site(Player.x,Player.y).locchar = FLOOR;
		Level->site(Player.x,Player.y).p_locf = L_NO_OP;
		stationcheck();
	    } else {
		print2 ("The tendril has destroyed your strength!");
		p_death ("the Essence of Earth");
	    }
	}
    } else
	print2 ("You step back from the ominous vegetation.");
}

static void stationcheck (void)
{
    bool stationsleft = false;
    morewait();
    clearmsg();
    print1 ("You feel regenerated.");
    Player.hp = Player.maxhp;
    dataprint();
    for (unsigned i = 0; i < Level->width; ++i)
	for (unsigned j = 0; j < Level->height; ++j)
	    if ((Level->site(i,j).locchar == WATER) || (Level->site(i,j).locchar == HEDGE) || (Level->site(i,j).locchar == WHIRLWIND) || (Level->site(i,j).locchar == FIRE))
		stationsleft = true;
    if (!stationsleft) {
	print1 ("There is a noise like a wild horse's neigh.");
	print2 ("You spin around, and don't see anyone around at all");
	print3 ("except for a spurred black cloaked figure carrying a scythe.");
	morewait();
	clearmsg();
	print1 ("Death coughs apologetically. He seems a little embarrassed.");
	print2 ("A voice peals out:");
	print3 ("'An Adept must be able to conquer Death himself....");
	make_site_monster (32, 4, DEATH);
    }
}

// To survive the void, the other four stations must be visited first,
// to activate the void, then something (Death's scythe, possibly) 
// must be thrown in to satiate the void, then all other items must
// be dropped, then the void must be entered.
static void l_void_station (void)
{
    int i, something = false;
    print1 ("You are at the brink of an endless void. Enter it? [yn] ");
    if (ynq() == 'y') {
	if (Level->mlist.empty()) {
	    print2 ("You fall forever. Eventually you die of starvation.");
	    morewait();
	    while (Player.hp > 0) {
		Time += 60;
		hourly_check();
		napms (250);
	    }
	} else {
	    print1 ("You enter the void.");
	    print2 ("You feel a sudden surge of power from five directions.");
	    morewait();
	    something = !Player.pack.empty();
	    if (!something)
		for (i = 0; i < MAXITEMS && !something; i++)
		    if (Player.has_possession(i))
			something = true;
	    if (something) {
		print1 ("The flow of power is disrupted by something!");
		print2 ("The power is unbalanced! You lose control!");
		morewait();
		print1 ("Each of your cells explodes with a little scream of pain.");
		print2 ("Your disrupted essence merges with the megaflow.");
		p_death ("the Power of the Void");
	    } else if (!gamestatusp (PREPARED_VOID)) {
		print1 ("The hungry void swallows you whole!");
		print2 ("Your being dissipates with a pathetic little sigh....");
		p_death ("the Emptyness of the Void");
	    } else {
		print1 ("The flow of power rages through your body,");
		print2 ("but you manage to master the surge!");
		print3 ("You feel adept....");
		morewait();
		clearmsg();
		print1 ("With a thought, you soar up through the void to the");
		print2 ("place from whence you came.");
		print3 ("As the platform of the Challenge dwindles beneath you");
		morewait();
		clearmsg();
		print1 ("You see Death raise his scythe to you in a salute.");
		++Player.rank[ADEPT];
		setgamestatus (COMPLETED_CHALLENGE);
		FixedPoints = calc_points();
		// set so change_environment puts player in correct temple!
		Player.x = 49;
		Player.y = 59;
		print2 ("You find yourself back in the Temple of Destiny.");
		morewait();
		change_environment (E_TEMPLE);
	    }
	}
    } else
	print2 ("You back away from the edge....");
}

static void l_voice1 (void)
{
    print1 ("A mysterious voice says: The Hunger of the Void must be satiated.");
    Level->site(Player.x,Player.y).p_locf = L_NO_OP;
}

static void l_voice2 (void)
{
    print1 ("A strange voice recites: Enter the Void as you entered the World.");
    Level->site(Player.x,Player.y).p_locf = L_NO_OP;
}

static void l_voice3 (void)
{
    print1 ("An eerie voice resounds: The Void is the fifth Elemental Station.");
    Level->site(Player.x,Player.y).p_locf = L_NO_OP;
}

static void l_whirlwind (void)
{
    print1 ("Buffeting winds swirl you up!");
    p_damage (random_range (difficulty() * 10), NORMAL_DAMAGE, "a magic whirlwind");
    if (random_range (2)) {
	print2 ("You are jolted by lightning!");
	p_damage (random_range (difficulty() * 10), ELECTRICITY, "a magic whirlwind");
    }
    morewait();
    if (random_range (2)) {
	print1 ("The whirlwind carries you off....");
	if (random_range (20) == 17)
	    print2 ("'I don't think we're in Kansas anymore, toto.'");
	p_teleport (0);
    }
}

static void l_enter_circle (void)
{
    print1 ("You see a translucent stairway before you, leading down.");
    print2 ("Take it? [yn] ");
    if (ynq() == 'y')
	change_environment (E_CIRCLE);
}

static void l_circle_library (void)
{
    print1 ("You see before you the arcane library of the Circle of Sorcerors.");
}

static void l_tome1 (void)
{
    menuclear();
    menuprint ("\nYou discover in a dusty tome some interesting information....");
    menuprint ("\nThe Star Gem holds a vast amount of mana, usable");
    menuprint ("\nfor either Law or Chaos. It is magically linked to Star Peak");
    menuprint ("\nand can either be activated or destroyed there. If destroyed,");
    menuprint ("\nits power will be used for Chaos, if activated, for Law.");
    menuprint ("\n\nIt is said the LawBringer has waited for an eternity");
    menuprint ("\nat Star Peak for someone to bring him the gem.");
    menuprint ("\nIt is also rumored that while anyone might destroy the gem,");
    menuprint ("\nreleasing chaotic energy, only the LawBringer can release");
    menuprint ("\nthe lawful potential of the gem.");
    showmenu();
    morewait();
    xredraw();
}

static void l_tome2 (void)
{
    menuclear();
    menuprint ("\nYou discover in some ancient notes that the Star Gem can be");
    menuprint ("\nused for transportation, but also read a caution that it must");
    menuprint ("\nbe allowed to recharge a long time between uses.");
    menuprint ("\nA marginal note says 'if only it could be reset to go somewhere");
    menuprint ("\nbesides Star Peak, the gem might be useful....'");
    showmenu();
    morewait();
    xredraw();
}

static void l_temple_warning (void)
{
    print1 ("A stern voice thunders in the air around you:");
    print2 ("'No unbelievers may enter these sacred precincts;");
    print3 ("those who defile this shrine will be destroyed!");
}

static void l_throne (void)
{
    print1 ("You have come upon a huge ornately appointed throne!");
    print2 ("Sit in it? [yn] ");
    if (ynq1() == 'y') {
	if (!find_item (SCEPTRE_OF_HIGH_MAGIC)) {
	    print1 ("The throne emits an eerie violet-black radiance.");
	    print2 ("You find, to your horror, that you cannot get up!");
	    print3 ("You feel an abstract sucking sensation...");
	    forget_all_spells();
	    Player.pow = 3;
	    Player.mana = 0;
	    Player.hp = 1;
	    dispel (-1);
	    morewait();
	    clearmsg();
	    print1 ("The radiance finally ceases. You can get up now.");
	} else {
	    if (HiMagicUse == Date)
		print3 ("You hear the sound of a magic kazoo played by an asthmatic.");
	    else {
		HiMagicUse = Date;
		print1 ("Following some strange impulse, you raise the Sceptre....");
		print2 ("You hear a magical fanfare, repeated three times.");
		switch (HiMagic++) {
		    case 0:
			print3 ("Strength.");
			Player.str += 5;
			Player.maxstr += 5;
			break;
		    case 1:
			print3 ("Constitution.");
			Player.con += 5;
			Player.maxcon += 5;
			break;
		    case 2:
			print3 ("Dexterity.");
			Player.dex += 5;
			Player.maxdex += 5;
			break;
		    case 3:
			print3 ("Agility.");
			Player.agi += 5;
			Player.maxagi += 5;
			break;
		    case 4:
			print3 ("Intelligence.");
			Player.iq += 5;
			Player.maxiq += 5;
			break;
		    case 5:
			print3 ("Power.");
			Player.pow += 5;
			Player.maxpow += 5;
			break;
		    default:
			if (spell_is_known (S_WISH)) {
			    print1 ("A mysterious voice mutters peevishly....");
			    print2 ("So what do you want now? A medal?");
			} else {
			    print1 ("Mystic runes appear in the air before you:");
			    print2 ("They appear to describe some high-powered spell.");
			    morewait();
			    print1 ("You hear a distant voice....");
			    print2 ("'You may now tread the path of High Magic.'");
			    learn_spell (S_WISH);
			}
			break;
		    case 17:
			print1 ("Weird flickering lights play over the throne.");
			print2 ("You hear a strange droning sound, as of a magical");
			morewait();
			print1 ("artifact stressed by excessive use....");
			print2 ("With an odd tinkling sound the throne shatters!");
			Level->site(Player.x,Player.y).locchar = RUBBLE;
			Level->site(Player.x,Player.y).p_locf = L_RUBBLE;
			lset (Player.x, Player.y, CHANGED);
			if (find_and_remove_item (SCEPTRE_OF_HIGH_MAGIC, -1)) {
			    morewait();
			    print1 ("Your sceptre reverberates with the noise, and");
			    print2 ("it too explodes in a spray of shards.");
			}
			break;
		}
		calc_melee();
		dataprint();
	    }
	}
    }
}

static void l_escalator (void)
{
    print1 ("You have found an extremely long stairway going straight up.");
    print2 ("The stairs are grilled steel and the bannister is rubber.");
    morewait();
    print1 ("Take the stairway? [yn] ");
    if (ynq1() == 'y') {
	print1 ("The stairs suddenly start moving with a grind of gears!");
	print2 ("You are wafted to the surface....");
	change_environment (E_COUNTRYSIDE);
    }
}

static void l_enter_court (void)
{
    print1 ("You have found a magical portal! Enter it? [yn] ");
    if (ynq1() == 'y') {
	if (!gamestatusp (COMPLETED_CASTLE)) {
	    if (gamestatusp (SPOKE_TO_ORACLE)) {
		print2 ("A dulcet voice says: 'Jolly good show!'");
		morewait();
	    }
	    setgamestatus (COMPLETED_CASTLE);
	}
	change_environment (E_COURT);
    }
}

static void l_chaostone (void)
{
    print1 ("This is a menhir carved of black marble with veins of gold.");
    print2 ("It emanates an aura of raw chaos, which is not terribly");
    morewait();
    print1 ("surprising, considering its location.");
    if (Player.alignment < 0)
	print2 ("You feel an almost unbearable attraction to the stone.");
    else
	print2 ("You find it extremely difficult to approach the stone.");
    morewait();
    clearmsg();
    print1 ("Touch it? [yn] ");
    if (ynq1() == 'y') {
	print1 ("A sudden flux of energy surrounds you!");
	morewait();
	if (stonecheck (-1)) {
	    print2 ("You feel stronger!");
	    Player.maxstr = min (Player.maxstr + 10, max (30, Player.maxstr));
	    dataprint();
	}
    } else
	print1 ("You step back from the ominous dolmech.");
}

static void l_balancestone (void)
{
    print1 ("This is a massive granite slab teetering dangerously on a corner.");
    print2 ("You feel a sense of balance as you regard it.");
    morewait();
    clearmsg();
    print1 ("Touch it? [yn] ");
    if (ynq1() == 'y') {
	print1 ("A vortex of mana spins about you!");
	if (absv (Player.alignment) > random_range (50)) {
	    print2 ("The cyclone whirls you off to a strange place!");
	    morewait();
	    change_environment (E_COUNTRYSIDE);
	    do {
		Player.x = random_range (Level->width);
		Player.y = random_range (Level->height);
	    } while (Country->site(Player.x,Player.y).showchar() == CHAOS_SEA);
	    screencheck (Player.y);
	    drawvision (Player.x, Player.y);
	} else {
	    print2 ("You are being drained of experience! Step back? [yn] ");
	    if (ynq2() == 'y') {
		clearmsg();
		print1 ("The vortex calms down, dimishes, and then disappears.");
	    } else {
		Player.xp -= Player.xp / 4;
		dataprint();
		print2 ("The vortex vanishes. Suddenly, there is a clap of thunder!");
		morewait();
		Player.alignment = 0;
		strategic_teleport (1);
	    }
	}
    } else
	print1 ("You step back from the unlikely boulder.");
}

static void l_lawstone (void)
{
    print1 ("This is a stele carved of blueish-green feldspar.");
    print2 ("You feel an aura of serenity rising from it, and your gaze");
    morewait();
    print1 ("is attracted to the bulk of Star Peak to the North-East.");
    if (Player.alignment > 0)
	print2 ("You feel a subtle attraction to the stone.");
    else
	print2 ("You find the stone extremely distasteful to contemplate.");
    morewait();
    clearmsg();
    print1 ("Touch it? [yn] ");
    if (ynq() == 'y') {
	print1 ("A matrix of power flows about you!");
	morewait();
	if (stonecheck (1)) {
	    print2 ("You feel more vigorous!");
	    Player.maxcon = min (Player.maxcon + 10, max (Player.maxcon, 30));
	    dataprint();
	}
    } else
	print1 ("You step back from the strange obelisk.");
}

static void l_voidstone (void)
{
    int i;
    print1 ("This is a grey and uninteresting stone.");
    print2 ("A feeling of nihility emanates from it.");
    morewait();
    clearmsg();
    print1 ("Touch it? [yn] ");
    if (ynq() == 'y') {
	print1 ("You feel negated.");
	morewait();
	Player.mana = 0;
	toggle_item_use (true);
	for (i = 0; i < NUMSTATI; i++)
	    Player.status[i] = 0;
	for (i = 0; i < MAXITEMS; i++)
	    if (Player.has_possession(i)) {
		Player.possessions[i].blessing = 0;
		Player.possessions[i].plus = 0;
		Player.possessions[i].usef = I_NOTHING;
	    }
	toggle_item_use (false);
	calc_melee();
    } else
	print1 ("You back away from the strange rock.");
}

static void l_sacrificestone (void)
{
    int sacrifice = 1;
    int oldmaxhp = Player.maxhp;
    print1 ("You have come on a weathered basaltic block.");
    print2 ("On the top surface is an indentation in human shape.");
    morewait();
    print1 ("You see old rust colored stains in the grain of the stone.");
    print2 ("You sense something awakening. Touch the block? [yn] ");
    if (ynq2() == 'y') {
	print1 ("You sense great pain emanating from the ancient altar.");
	print2 ("Climb on to the block? [yn] ");
	if (ynq2() == 'y') {
	    print1 ("You are stuck fast to the block!");
	    print2 ("You feel your life-force being sucked away!");
	    morewait();
	    print1 ("Hit ESCAPE to try and get up at any moment, SPACE to remain.");
	    do {
		switch (random_range (4)) {
		    case 0:
			print2 ("You feel weaker.");
			break;
		    case 1:
			print2 ("You feel your life fading.");
			break;
		    case 2:
			print2 ("Your energy is vanishing.");
			break;
		    case 3:
			print2 ("You are being drained.");
			break;
		}
		Player.hp -= sacrifice;
		Player.maxhp -= sacrifice / 2;
		sacrifice *= 2;
		dataprint();
		if ((Player.hp < 1) || (Player.maxhp < 1))
		    p_death ("self-sacrifice");
	    } while (stillonblock());
	    print1 ("You manage to wrench yourself off the ancient altar!");
	    print2 ("You leave some skin behind, though....");
	    morewait();
	    if ((oldmaxhp > 10) && (Player.maxhp < 3 * oldmaxhp / 4)) {
		print1 ("A strange red glow arises from the altar.");
		print2 ("The glow surrounds you.... You sense gratitude.");
		Player.pow += sacrifice;
		Player.maxpow += sacrifice / 10;
		dataprint();
	    } else {
		print1 ("You a have a sense of rejection.");
		print2 ("A roil of fetid vapor smokes up from the altar.");
		gain_experience (sacrifice);
	    }
	} else {
	    print1 ("You sense an emotion of pique all around you.");
	    print2 ("You retreat from the strange stone.");
	}
    } else {
	print1 ("You decide discretion to be the better part of valour.");
	print2 ("The stone seems to subside sleepily.");
    }
}

static void l_mindstone (void)
{
    print1 ("You approach a giant crystal of some opaline material.");
    print2 ("Flashes of irridescent light glint from the object.");
    morewait();
    print1 ("You feel your attention being drawn by the intricate crystal.");
    print2 ("Look away from the interesting phenomenon? [yn] ");
    if (ynq2() == 'n') {
	print1 ("Your gaze focuses deeply on the gem....");
	print2 ("The crystal seems to open up and surround you!");
	morewait();
	if (stonecheck (0)) {
	    print1 ("Your mind has been enhanced by the experience!");
	    Player.maxiq = min (Player.maxiq + 10, max (Player.maxiq, 30));
	    dataprint();
	}
    } else {
	print1 ("You manage to wrench your gaze from the odd jewel.");
	print2 ("The light flashes from the crystal diminish in frequency.");
    }
}

void p_movefunction (int movef)
{
    // loc functs above traps should be activated whether levitating or not
    drawvision (Player.x, Player.y);
    sign_print (Player.x, Player.y, false);
    if (Player.status[SHADOWFORM])
	switch (movef) {	// player in shadow form is unable to do most things
	    case L_CHAOS:
		l_chaos();
		break;
	    case L_ABYSS:
		l_abyss();
		break;
	    case L_ARENA_EXIT:
		l_arena_exit();
		break;
	    case L_ENTER_COURT:
		l_enter_court();
		break;
	    case L_ESCALATOR:
		l_escalator();
		break;
	    case L_COLLEGE:
		l_college();
		break;
	    case L_SORCERORS:
		l_sorcerors();
		break;
	    case L_ALTAR:
		l_altar();
		break;
	    case L_TACTICAL_EXIT:
		l_tactical_exit();
		break;
	    case L_HOUSE_EXIT:
		l_house_exit();
		break;
	    case L_HOUSE:
		l_house();
		break;
	    case L_HOVEL:
		l_hovel();
		break;
	    case L_MANSION:
		l_mansion();
		break;
	    case L_COUNTRYSIDE:
		l_countryside();
		break;
	    case L_ORACLE:
		l_oracle();
		break;
	    case L_TEMPLE_WARNING:
		l_temple_warning();
		break;
	    case L_ENTER_CIRCLE:
		l_enter_circle();
		break;
	    case L_CIRCLE_LIBRARY:
		l_circle_library();
		break;
	    case L_TOME1:
		l_tome1();
		break;
	    case L_TOME2:
		l_tome2();
		break;
	    case L_ADEPT:
		l_adept();
		break;
	    case L_VOICE1:
		l_voice1();
		break;
	    case L_VOICE2:
		l_voice2();
		break;
	    case L_VOICE3:
		l_voice3();
		break;
	    case L_VOID:
		l_void();
		break;
	    case L_FIRE_STATION:
		l_fire_station();
		break;
	    case L_EARTH_STATION:
		l_earth_station();
		break;
	    case L_WATER_STATION:
		l_water_station();
		break;
	    case L_AIR_STATION:
		l_air_station();
		break;
	    case L_VOID_STATION:
		l_void_station();
		break;
    } else if ((!Player.status[LEVITATING]) || gamestatusp (MOUNTED) || (Cmd == '@') ||	// @ command activates all effects under player
	       (movef < LEVITATION_AVOIDANCE)) {

	switch (movef) { // miscellaneous
	    case L_NO_OP:
		l_no_op();
		break;
	    case L_HEDGE:
		l_hedge();
		break;
	    case L_WATER:
		l_water();
		break;
	    case L_LIFT:
		l_lift();
		break;
	    case L_LAVA:
		l_lava();
		break;
	    case L_FIRE:
		l_fire();
		break;
	    case L_WHIRLWIND:
		l_whirlwind();
		break;
	    case L_RUBBLE:
		l_rubble();
		break;
	    case L_MAGIC_POOL:
		l_magic_pool();
		break;
	    case L_CHAOS:
		l_chaos();
		break;
	    case L_ABYSS:
		l_abyss();
		break;

	    case L_PORTCULLIS_TRAP:
		l_portcullis_trap();
		break;
	    case L_RAISE_PORTCULLIS:
		l_raise_portcullis();
		break;
	    case L_DROP_EVERY_PORTCULLIS:
		l_drop_every_portcullis();
		break;
	    case L_ARENA_EXIT:
		l_arena_exit();
		break;
	    case L_TRIFID:
		l_trifid();
		break;
	    case L_ENTER_COURT:
		l_enter_court();
		break;
	    case L_ESCALATOR:
		l_escalator();
		break;
	    case L_THRONE:
		l_throne();
		break;

	    case L_TRAP_DART:
		l_trap_dart();
		break;
	    case L_TRAP_SIREN:
		l_trap_siren();
		break;
	    case L_TRAP_PIT:
		l_trap_pit();
		break;
	    case L_TRAP_DOOR:
		l_trap_door();
		break;
	    case L_TRAP_SNARE:
		l_trap_snare();
		break;
	    case L_TRAP_BLADE:
		l_trap_blade();
		break;
	    case L_TRAP_FIRE:
		l_trap_fire();
		break;
	    case L_TRAP_TELEPORT:
		l_trap_teleport();
		break;
	    case L_TRAP_DISINTEGRATE:
		l_trap_disintegrate();
		break;
	    case L_TRAP_SLEEP_GAS:
		l_trap_sleepgas();
		break;
	    case L_TRAP_MANADRAIN:
		l_trap_manadrain();
		break;
	    case L_TRAP_ACID:
		l_trap_acid();
		break;
	    case L_TRAP_ABYSS:
		l_trap_abyss();
		break;

		// door functions
	    case L_BANK:
		l_bank();
		break;
	    case L_ARMORER:
		l_armorer();
		break;
	    case L_CLUB:
		l_club();
		break;
	    case L_GYM:
		l_gym();
		break;
	    case L_BROTHEL:
		l_brothel();
		break;
	    case L_THIEVES_GUILD:
		l_thieves_guild();
		break;
	    case L_COLLEGE:
		l_college();
		break;
	    case L_HEALER:
		l_healer();
		break;
	    case L_STATUE_WAKE:
		l_statue_wake();
		break;
	    case L_CASINO:
		l_casino();
		break;
	    case L_COMMANDANT:
		l_commandant();
		break;
	    case L_DINER:
		l_diner();
		break;
	    case L_CRAP:
		l_crap();
		break;
	    case L_TAVERN:
		l_tavern();
		break;
	    case L_MERC_GUILD:
		l_merc_guild();
		break;
	    case L_ALCHEMIST:
		l_alchemist();
		break;
	    case L_SORCERORS:
		l_sorcerors();
		break;
	    case L_CASTLE:
		l_castle();
		break;
	    case L_ARENA:
		l_arena();
		break;
	    case L_VAULT:
		l_vault();
		break;
	    case L_DPW:
		l_dpw();
		break;
	    case L_LIBRARY:
		l_library();
		break;
	    case L_PAWN_SHOP:
		l_pawn_shop();
		break;
	    case L_CONDO:
		l_condo();
		break;
	    case L_ALTAR:
		l_altar();
		break;
	    case L_TACTICAL_EXIT:
		l_tactical_exit();
		break;
	    case L_HOUSE_EXIT:
		l_house_exit();
		break;
	    case L_SAFE:
		l_safe();
		break;
	    case L_HOUSE:
		l_house();
		break;
	    case L_HOVEL:
		l_hovel();
		break;
	    case L_MANSION:
		l_mansion();
		break;
	    case L_COUNTRYSIDE:
		l_countryside();
		break;
	    case L_ORACLE:
		l_oracle();
		break;
	    case L_ORDER:
		l_order();
		break;
	    case L_CARTOGRAPHER:
		l_cartographer();
		break;

	    case L_TEMPLE_WARNING:
		l_temple_warning();
		break;
	    case L_ENTER_CIRCLE:
		l_enter_circle();
		break;
	    case L_CIRCLE_LIBRARY:
		l_circle_library();
		break;
	    case L_TOME1:
		l_tome1();
		break;
	    case L_TOME2:
		l_tome2();
		break;

	    case L_CHARITY:
		l_charity();
		break;

	    case L_CHAOSTONE:
		l_chaostone();
		break;
	    case L_VOIDSTONE:
		l_voidstone();
		break;
	    case L_BALANCESTONE:
		l_balancestone();
		break;
	    case L_LAWSTONE:
		l_lawstone();
		break;
	    case L_SACRIFICESTONE:
		l_sacrificestone();
		break;
	    case L_MINDSTONE:
		l_mindstone();
		break;

		// challenge functions
	    case L_ADEPT:
		l_adept();
		break;
	    case L_VOICE1:
		l_voice1();
		break;
	    case L_VOICE2:
		l_voice2();
		break;
	    case L_VOICE3:
		l_voice3();
		break;
	    case L_VOID:
		l_void();
		break;
	    case L_FIRE_STATION:
		l_fire_station();
		break;
	    case L_EARTH_STATION:
		l_earth_station();
		break;
	    case L_WATER_STATION:
		l_water_station();
		break;
	    case L_AIR_STATION:
		l_air_station();
		break;
	    case L_VOID_STATION:
		l_void_station();
		break;
	}
	if (movef != L_NO_OP) {
	    resetgamestatus (FAST_MOVE);
	    dataprint();
	}
    }
}

// execute some move function for a monster
void m_movefunction (struct monster *m, int movef)
{
    // loc functs above traps should be activated whether levitating or not
    if (!m_statusp (m, FLYING) && !m_statusp (m, INTANGIBLE))
	switch (movef) {

		// miscellaneous
	    case L_NO_OP:
		m_no_op (m);
		break;
	    case L_WATER:
		m_water (m);
		break;
	    case L_LAVA:
		m_lava (m);
		break;
	    case L_FIRE:
		m_fire (m);
		break;
	    case L_MAGIC_POOL:
		m_water (m);
		break;
	    case L_ABYSS:
		m_abyss (m);
		break;

	    case L_TRAP_DART:
		m_trap_dart (m);
		break;
	    case L_TRAP_PIT:
		m_trap_pit (m);
		break;
	    case L_TRAP_DOOR:
		m_trap_door (m);
		break;
	    case L_TRAP_SNARE:
		m_trap_snare (m);
		break;
	    case L_TRAP_BLADE:
		m_trap_blade (m);
		break;
	    case L_TRAP_FIRE:
		m_trap_fire (m);
		break;
	    case L_TRAP_TELEPORT:
		m_trap_teleport (m);
		break;
	    case L_TRAP_DISINTEGRATE:
		m_trap_disintegrate (m);
		break;
	    case L_TRAP_MANADRAIN:
		m_trap_manadrain (m);
		break;
	    case L_TRAP_SLEEP_GAS:
		m_trap_sleepgas (m);
		break;
	    case L_TRAP_ACID:
		m_trap_acid (m);
		break;
	    case L_TRAP_ABYSS:
		m_trap_abyss (m);
		break;

	    case L_ALTAR:
		m_altar (m);
		break;
	}
}
