// Omega is free software, distributed under the MIT license

#include "glob.h"

//----------------------------------------------------------------------

static int itemlist(int itemindex, int num);
static int selectmonster(void);
static void ball(int fx, int fy, int tx, int ty, int dmg, EDamageType dtype);
static void bolt(int fx, int fy, int tx, int ty, int hit, int dmg, EDamageType dtype);

//----------------------------------------------------------------------

// enchant
void enchant (int delta)
{
    int i, used = false;
    long change_cash;

    if (delta < 0) {
	i = random_item();

	const char* your = "Your ";
	if (object_uniqueness(Player.possessions[i]) != COMMON)
	    your = "";

	if (i == ABORT || Player.possessions[i].usef == I_NOTHING || Player.possessions[i].usef == I_NO_OP || Player.possessions[i].usef == I_NORMAL_ARMOR || Player.possessions[i].usef == I_NORMAL_WEAPON
	    || Player.possessions[i].usef == I_NORMAL_SHIELD || Player.possessions[i].objchar == FOOD || Player.possessions[i].objchar == MISSILEWEAPON) {
	    mprint ("You feel fortunate.");
	    morewait();
	} else if (Player.possessions[i].blessing < 0 || (Player.possessions[i].objchar == ARTIFACT && random_range (3))) {
	    mprintf ("%s%s glows, but the glow flickers out...", your, itemid (Player.possessions[i]));
	    morewait();
	} else {
	    used = (Player.possessions[i].used);
	    if (used) {
		Player.possessions[i].used = false;
		item_use (Player.possessions[i]);
	    }
	    mprintf ("%s%s radiates an aura of mundanity!", your, itemid (Player.possessions[i]));
	    morewait();
	    Player.possessions[i].plus = 0;
	    Player.possessions[i].charge = -1;
	    Player.possessions[i].usef = I_NOTHING;
	    if (used) {
		Player.possessions[i].used = true;
		item_use (Player.possessions[i]);
	    }
	}
    } else {
	i = getitem (CASH);
	if (i == ABORT) {
	    mprint ("You feel unlucky.");
	    morewait();
	} else if (i == CASHVALUE) {
	    mprint ("You enchant your money.... What a concept!");
	    change_cash = Player.cash * (random_range (7) - 3) / 6;
	    if (change_cash > 0)
		mprint ("Seems to have been a good idea!");
	    else
		mprint ("Maybe it wasn't such a good idea....");
	    Player.cash += change_cash;
	    morewait();
	} else if (Player.possessions[i].objchar == ARTIFACT) {
	    if (Player.possessions[i].usef != Objects[Player.possessions[i].id].usef) {
		mprint ("It re-acquires its magical aura!");
		Player.possessions[i].usef = Objects[Player.possessions[i].id].usef;
	    } else {
		mprintf ("The enchantment spell enfolds the %s", itemid (Player.possessions[i]));
		mprint ("and the potent enchantment of the Artifact causes a backlash!");
		morewait();
		clearmsg();
		manastorm (Player.x, Player.y, Player.possessions[i].level * 5);
	    }
	} else {
	    if (Player.possessions[i].plus > random_range (20) + 1) {
		mprint ("Uh-oh, the force of the enchantment was too much!");
		mprint ("There is a loud explosion!");
		morewait();
		manastorm (Player.x, Player.y, Player.possessions[i].plus * 5);
		Player.remove_possession (i, 1);
	    } else {
		used = (Player.possessions[i].used);
		if (used) {
		    setgamestatus (SUPPRESS_PRINTING);
		    Player.possessions[i].used = false;
		    item_use (Player.possessions[i]);
		    resetgamestatus (SUPPRESS_PRINTING);
		}
		mprint ("The item shines!");
		morewait();
		Player.possessions[i].plus += delta + 1;
		if (Player.possessions[i].charge > -1)
		    Player.possessions[i].charge += ((delta + 1) * (random_range (10) + 1));
		if (used) {
		    setgamestatus (SUPPRESS_PRINTING);
		    Player.possessions[i].used = true;
		    item_use (Player.possessions[i]);
		    resetgamestatus (SUPPRESS_PRINTING);
		}
	    }
	}
	calc_melee();
    }
}

// bless
void bless (int blessing)
{
    int iidx, used;

    if (blessing < 0) {
	iidx = random_item();
	if (iidx == ABORT) {
	    mprint ("You feel fortunate.");
	    morewait();
	} else {
	    const char* your = "your ";
	    if (object_uniqueness(Player.possessions[iidx]) != COMMON)
		your = "";
	    mprintf ("A foul odor arises from %s%s", your, itemid(Player.possessions[iidx]));
	    morewait();
	    used = (Player.possessions[iidx].used);
	    if (used) {
		setgamestatus (SUPPRESS_PRINTING);
		Player.possessions[iidx].used = false;
		item_use (Player.possessions[iidx]);
		resetgamestatus (SUPPRESS_PRINTING);
	    }
	    Player.possessions[iidx].blessing -= 2;
	    if (Player.possessions[iidx].blessing < 0)
		Player.possessions[iidx].plus = absv (Player.possessions[iidx].plus) - 1;
	    if (used) {
		setgamestatus (SUPPRESS_PRINTING);
		Player.possessions[iidx].used = true;
		item_use (Player.possessions[iidx]);
		resetgamestatus (SUPPRESS_PRINTING);
	    }
	}
    } else {
	iidx = getitem (NULL_ITEM);
	if (iidx == CASHVALUE) {
	    mprint ("Blessing your money has no effect.");
	    morewait();
	} else if (iidx != ABORT) {
	    used = (Player.possessions[iidx].used == true);
	    if (used) {
		setgamestatus (SUPPRESS_PRINTING);
		Player.possessions[iidx].used = false;
		item_use (Player.possessions[iidx]);
		resetgamestatus (SUPPRESS_PRINTING);
	    }
	    mprint ("A pure white light surrounds the item... ");
	    if (Player.possessions[iidx].blessing < 0 - (blessing + 1)) {
		mprint ("which is evil enough to resist the effect of the blessing!");
		morewait();
	    } else if (Player.possessions[iidx].blessing < -1) {
		mprint ("which disintegrates under the influence of the holy aura!");
		morewait();
		Player.itemweight -= Player.possessions[iidx].weight;
		Player.remove_possession (iidx, 1);
	    } else if (Player.possessions[iidx].blessing < blessing + 1) {
		mprint ("which now seems affected by afflatus!");
		morewait();
		Player.possessions[iidx].blessing++;
		Player.possessions[iidx].plus = absv (Player.possessions[iidx].plus) + 1;
	    } else {
		mprint ("The hierolux fades without any appreciable effect....");
		morewait();
	    }
	    if (used && Player.has_possession(iidx)) {
		setgamestatus (SUPPRESS_PRINTING);
		Player.possessions[iidx].used = true;
		item_use (Player.possessions[iidx]);
		resetgamestatus (SUPPRESS_PRINTING);
	    }
	}
    }
    calc_melee();
}

void heal (int amount)
{
    if (amount > -1) {
	mprint ("You feel better.");
	if (Player.hp < Player.maxhp + amount) {
	    Player.hp += random_range (10 * amount) + 1;
	    if (Player.hp > Player.maxhp)
		Player.hp = Player.maxhp + amount;
	}
	Player.status[BLINDED] = 0;
    } else {
	mprint ("You feel unwell.");
	Player.hp -= random_range (10 * absv (amount) + 1);
	if (Player.hp < 0)
	    p_death ("magical disruption");
    }
    dataprint();
}

void fbolt (int fx, int fy, int tx, int ty, int hit, int dmg)
{
    bolt (fx, fy, tx, ty, hit, dmg, FLAME);
}

void lbolt (int fx, int fy, int tx, int ty, int hit, int dmg)
{
    bolt (fx, fy, tx, ty, hit, dmg, ELECTRICITY);
}

// Added 12/30/98 DG
void icebolt (int fx, int fy, int tx, int ty, int hit, int dmg)
{
    bolt (fx, fy, tx, ty, hit, dmg, COLD);
}

void nbolt (int fx, int fy, int tx, int ty, int hit, int dmg)
{
    bolt (fx, fy, tx, ty, hit, dmg, NORMAL_DAMAGE);
}

// from f to t
static void bolt (int fx, int fy, int tx, int ty, int hit, int dmg, EDamageType dtype)
{
    int xx, yy;
    struct monster *target;
    chtype boltchar;
    xx = fx;
    yy = fy;

    switch (dtype) {
	case FLAME:
	    boltchar = ('*' | CLR_LIGHT_RED_BLACK);
	    break;
	case ELECTRICITY:
	    boltchar = ('^' | CLR_LIGHT_BLUE_BLACK);
	    break;
	default:
	case NORMAL_DAMAGE:
	    boltchar = ('!' | CLR_BROWN_BLACK);
	    break;
	case COLD:
	    boltchar = ('o' | CLR_WHITE_BLACK);
	    break;
    }
    clearmsg();

    do_los (boltchar, &xx, &yy, tx, ty);

    if (xx == (int)Player.x && yy == (int)Player.y) {
	if (Player.status[DEFLECTION] > 0)
	    mprint ("The bolt just missed you!");
	else {
	    switch (dtype) {
		case FLAME:
		    mprint ("You were blasted by a firebolt!");
		    p_damage (random_range (dmg), dtype, "a firebolt");
		    break;
		case ELECTRICITY:
		    mprint ("You were zapped by lightning!");
		    p_damage (random_range (dmg), dtype, "a bolt of lightning");
		    break;
		default:
		case NORMAL_DAMAGE:
		    mprint ("You were hit by a missile!");
		    p_damage (random_range (dmg), dtype, "a missile");
		    break;
		case COLD:
		    mprint ("You were hit by an icicle!");
		    p_damage (random_range (dmg), dtype, "an icicle");
		    break;
	    }
	}
    } else if ((target = Level->creature(xx,yy))) {
	if (hitp (hit, target->ac)) {
	    if (target->uniqueness == COMMON) {
		strcpy (Str1, "The ");
		strcat (Str1, target->monstring);
	    } else
		strcpy (Str1, target->monstring);
	    switch (dtype) {
		    // WDT: these sentances really ought to be livened up.  Especially
		    // in full verbose mode.
		case FLAME:
		    strcat (Str1, " was blasted by a firebolt!");
		    break;
		case ELECTRICITY:
		    strcat (Str1, " was zapped by lightning!");
		    break;
		default:
		case NORMAL_DAMAGE:
		    strcat (Str1, " was hit by a missile!");
		    break;
		case COLD:
		    strcat (Str1, " was hit by an icicle!");
		    break;
	    }
	    mprint (Str1);
	    m_status_set (target, HOSTILE);
	    m_damage (target, random_range (dmg), dtype);
	} else {
	    if (target->uniqueness == COMMON) {
		strcpy (Str1, "The ");
		strcat (Str1, target->monstring);
	    } else
		strcpy (Str1, target->monstring);
	    switch (dtype) {
		case FLAME:
		    strcat (Str1, " was missed by a firebolt!");
		    break;
		case ELECTRICITY:
		    strcat (Str1, " was missed by lightning!");
		    break;
		default:
		case NORMAL_DAMAGE:
		    strcat (Str1, " was missed by a missile!");
		    break;
		case COLD:
		    strcat (Str1, " was missed by a flying icicle!");
		    break;
	    }
	    mprint (Str1);
	}
    } else if (Level->site(xx,yy).locchar == HEDGE)
	if (Level->site(xx,yy).p_locf != L_TRIFID) {
	    if ((dtype == FLAME) || (dtype == ELECTRICITY)) {
		mprint ("The hedge is blasted away!");
		Level->site(xx,yy).p_locf = L_NO_OP;
		Level->site(xx,yy).locchar = FLOOR;
		plotspot (xx, yy, true);
		lset (xx, yy, CHANGED);
	    } else
		mprint ("The hedge is unaffected.");
	} else
	    mprint ("The trifid absorbs the energy and laughs!");
    else if (Level->site(xx,yy).locchar == WATER)
	if (dtype == FLAME) {
	    mprint ("The water is vaporised!");
	    Level->site(xx,yy).p_locf = L_NO_OP;
	    Level->site(xx,yy).locchar = FLOOR;
	    lset (xx, yy, CHANGED);
	}
}

void lball (int fx, int fy, int tx, int ty, int dmg)
{
    ball (fx, fy, tx, ty, dmg, ELECTRICITY);
}

void manastorm (int x, int y, int dmg)
{
    ball (x, y, x, y, dmg, UNSTOPPABLE);
}

void snowball (int fx, int fy, int tx, int ty, int dmg)
{
    ball (fx, fy, tx, ty, dmg, COLD);
}

void fball (int fx, int fy, int tx, int ty, int dmg)
{
    ball (fx, fy, tx, ty, dmg, FLAME);
}

// from f to t
static void ball (int fx, int fy, int tx, int ty, int dmg, EDamageType dtype)
{
    int xx, yy, ex, ey, i;
    struct monster *target;
    chtype expchar = ('@' | CLR_LIGHT_PURPLE_BLACK);

    xx = fx;
    yy = fy;

    switch (dtype) {
	default:
	case FLAME:
	    expchar = ('*' | CLR_LIGHT_RED_BLACK);
	    break;
	case COLD:
	    expchar = ('o' | CLR_WHITE_BLACK);
	    break;
	case ELECTRICITY:
	    expchar = ('^' | CLR_LIGHT_BLUE_BLACK);
	    break;
    }

    do_los (expchar, &xx, &yy, tx, ty);
    draw_explosion (expchar, xx, yy);
    for (i = 0; i < 9; i++) {
	ex = xx + Dirs[0][i];
	ey = yy + Dirs[1][i];

	if (ex == (int)Player.x && ey == (int)Player.y) {
	    switch (dtype) {
		default:
		case FLAME:
		    mprint ("You were blasted by a fireball!");
		    p_damage (random_range (dmg), FLAME, "a fireball");
		    break;
		case COLD:
		    mprint ("You were blasted by a snowball!");
		    p_damage (random_range (dmg), COLD, "a snowball");
		    break;
		case ELECTRICITY:
		    mprint ("You were blasted by ball lightning!");
		    p_damage (random_range (dmg), ELECTRICITY, "ball lightning");
		    break;
		case UNSTOPPABLE:
		    mprint ("Oh No! Manastorm!");
		    p_damage (random_range (dmg), UNSTOPPABLE, "a manastorm!");
		    break;
	    }
	}
	if ((target = Level->creature(ex,ey))) {
	    if (los_p (Player.x, Player.y, target->x, target->y)) {
		if (target->uniqueness == COMMON) {
		    strcpy (Str1, "The ");
		    strcat (Str1, target->monstring);
		} else
		    strcpy (Str1, target->monstring);
		switch (dtype) {
		    default:
		    case FLAME:
			strcat (Str1, " was zorched by a fireball!");
			break;
		    case COLD:
			strcat (Str1, " was blasted by a snowball!");
			break;
		    case ELECTRICITY:
			strcat (Str1, " was zapped by ball lightning!");
			break;
		    case UNSTOPPABLE:
			strcat (Str1, " was nuked by a manastorm!");
			break;
		}
		mprint (Str1);
	    }
	    m_status_set (target, HOSTILE);
	    m_damage (target, random_range (dmg), dtype);
	}
	if (Level->site(ex,ey).locchar == HEDGE)
	    if (Level->site(ex,ey).p_locf != L_TRIFID) {
		if ((dtype == FLAME) || (dtype == ELECTRICITY)) {
		    mprint ("The hedge is blasted away!");
		    Level->site(ex,ey).p_locf = L_NO_OP;
		    Level->site(ex,ey).locchar = FLOOR;
		    plotspot (ex, ey, true);
		    lset (ex, ey, CHANGED);
		} else
		    mprint ("The hedge is unaffected.");
	    } else
		mprint ("The trifid absorbs the energy and laughs!");
	else if (Level->site(ex,ey).locchar == WATER)
	    if (dtype == FLAME) {
		mprint ("The water is vaporised!");
		Level->site(ex,ey).p_locf = L_NO_OP;
		Level->site(ex,ey).locchar = FLOOR;
		plotspot (ex, ey, true);
		lset (ex, ey, CHANGED);
	    }
    }
}

void mondet (int blessing)
{
    foreach (m, Level->mlist) {
	if (m->hp <= 0) continue;
	if (blessing > -1)
	    plotmon (&*m);
	else
	    putspot (random_range (Level->width), random_range (Level->height), Monsters[random_range (NUMMONSTERS)].monchar);
    }
    levelrefresh();
    morewait();
    show_screen();
}

void objdet (int blessing)
{
    foreach (i, Level->things) {
	if (blessing < 0)
	    putspot (random_range (Level->width), random_range (Level->height), i->objchar);
	else
	    putspot (i->x, i->y, i->objchar);
    }
    levelrefresh();
    morewait();
    show_screen();
}

void identify (int blessing)
{
    clearmsg();
    if (blessing == 0) {
	mprint ("Identify:");
	int iidx = getitem (NULL_ITEM);
	if (iidx == CASHVALUE)
	    mprint ("Your money is really money.");
	else if (iidx == ABORT)
	    setgamestatus (SKIP_MONSTERS);
	else {
	    learn_object (Player.possessions[iidx]);
	    mprint ("Identified: ");
	    mprint (itemid (Player.possessions[iidx]));
	}
    } else if (blessing < 0) {
	mprint ("You feel forgetful.");
	for (unsigned iidx = 0; iidx < MAXITEMS; iidx++)
	    if (Player.has_possession(iidx))
		forget_object (Player.possessions[iidx]);
    } else {
	mprint ("You feel encyclopaedic.");
	for (unsigned iidx = 0; iidx < MAXITEMS; iidx++)
	    learn_object (Player.possessions[iidx]);
	foreach (i, Player.pack)
	    learn_object(&*i);
    }
    calc_melee();
}

// returns index of random item, ABORT if player carrying none
int random_item (void)
{
    int item = ABORT;
    for (int tries = 0, number = 0; tries < MAXITEMS; tries++) {
	if (Player.has_possession(tries)) {
	    number++;
	    if (random_range (number) == 0)
		item = tries;
	}
    }
    return (item);
}

// various kinds of wishes
void wish (int blessing)
{
    char wishstr[80];
    clearmsg();
    mprint ("What do you wish for? ");
    if (blessing < 0)
	deathprint();
    else
	strcpy (wishstr, msgscanstring());
    if (blessing < 0 || strcmp (wishstr, "Death") == 0) {
	mprint ("As you wish, so shall it be.");
	p_death ("a deathwish");
    }
    if (strcmp (wishstr, "Power") == 0) {
	mprint ("You feel a sudden surge of energy");
	Player.mana = Player.calcmana() * 10;
    } else if (strcmp (wishstr, "Skill") == 0) {
	mprint ("You feel more competent.");
	gain_experience (min (10000U, Player.xp));
    } else if (strcmp (wishstr, "Wealth") == 0) {
	mprint ("You are submerged in shower of gold pieces!");
	Player.cash += 10000;
    } else if (strcmp (wishstr, "Balance") == 0) {
	mprint ("You feel neutral.");
	Player.alignment = 0;
    } else if (strcmp (wishstr, "Chaos") == 0) {
	mprint ("You feel chaotic.");
	Player.alignment -= 100;
    } else if (strcmp (wishstr, "Law") == 0) {
	mprint ("You feel lawful.");
	Player.alignment += 100;
    } else if (strcmp (wishstr, "Location") == 0)
	strategic_teleport (1);
    else if (strcmp (wishstr, "Knowledge") == 0) {
	mprint ("You feel more knowledgeable.");
	ESpell i = (ESpell) random_range (NUMSPELLS);
	if (spell_is_known(i))
	    Spells[i].powerdrain = (max (1, Spells[i].powerdrain / 2));
	else
	    learn_spell(i);
    } else if (strcmp (wishstr, "Health") == 0) {
	mprint ("You feel vigorous");
	Player.food = 40;
	toggle_item_use (true);
	Player.str = max (Player.str, Player.maxstr);
	Player.con = max (Player.con, Player.maxcon);
	Player.agi = max (Player.agi, Player.maxagi);
	Player.dex = max (Player.dex, Player.maxdex);
	Player.iq = max (Player.iq, Player.maxiq);
	Player.pow = max (Player.pow, Player.maxpow);
	Player.maxhp = max<uint16_t> (Player.maxhp, Player.maxcon);
	Player.hp = max<int16_t> (Player.hp, Player.maxhp);
	toggle_item_use (false);
	Player.status[DISEASED] = 0;
	Player.status[POISONED] = 0;
    } else if (strcmp (wishstr, "Destruction") == 0)
	annihilate (gamestatusp (CHEATED));
    else if (strcmp (wishstr, "Acquisition") == 0)
	acquire (gamestatusp (CHEATED));
    else if (strcmp (wishstr, "Summoning") == 0)
	summon (gamestatusp (CHEATED), -1);
    else if (strcmp (wishstr, "Stats") == 0 && gamestatusp (CHEATED)) {
	Player.str = Player.maxstr = Player.con = Player.maxcon = Player.agi = Player.maxagi = Player.dex = Player.maxdex = Player.iq = Player.maxiq = Player.pow = Player.maxpow = 200;
	calc_melee();
    } else
	mprint ("You feel stupid.");
    dataprint();
    showflags();
}

// gain for an item
void acquire (int blessing)
{
    char otype;
    int iidx, id = ABORT;

    if (blessing < 0) {
	iidx = random_item();
	if (iidx == ABORT)
	    mprint ("You feel fortunate.");
	else {
	    mprint ("Smoke drifts out of your pack.... ");
	    mprintf ("Destroyed: %s", itemid (Player.possessions[iidx]));
	    morewait();
	    Player.remove_possession (iidx);
	}
    } else {
	object newthing;
	newthing.id = NO_THING;
	if (gamestatusp (CHEATED))
	    mprint ("Acquire which kind of item: !?][}{)/=%%\\& ");
	else
	    mprint ("Acquire which kind of item: !?][}{)/=%%\\ ");
	otype = mgetc();
	switch (otype) {
	    case (POTION & 0xff):
		if (blessing > 0)
		    id = itemlist (POTIONID, NUMPOTIONS);
		else
		    id = random_range (NUMPOTIONS);
		if (id < 0)
		    mprint ("You feel stupid.");
		else
		    newthing = make_potion (id);
		break;
	    case (SCROLL & 0xff):
		if (blessing > 0)
		    id = itemlist (SCROLLID, NUMSCROLLS);
		else
		    id = random_range (NUMSCROLLS);
		if (id < 0)
		    mprint ("You feel stupid.");
		else
		    newthing = make_scroll (id);
		break;
	    case (RING & 0xff):
		if (blessing > 0)
		    id = itemlist (RINGID, NUMRINGS);
		else
		    id = random_range (NUMRINGS);
		if (id < 0)
		    mprint ("You feel stupid.");
		else
		    newthing = make_ring (id);
		break;
	    case (STICK & 0xff):
		if (blessing > 0)
		    id = itemlist (STICKID, NUMSTICKS);
		else
		    id = random_range (NUMSTICKS);
		if (id < 0)
		    mprint ("You feel stupid.");
		else
		    newthing = make_stick (id);
		break;
	    case (ARMOR & 0xff):
		if (blessing > 0)
		    id = itemlist (ARMORID, NUMARMOR);
		else
		    id = random_range (NUMARMOR);
		if (id < 0)
		    mprint ("You feel stupid.");
		else
		    newthing = make_armor (id);
		break;
	    case (SHIELD & 0xff):
		if (blessing > 0)
		    id = itemlist (SHIELDID, NUMSHIELDS);
		else
		    id = random_range (NUMSHIELDS);
		if (id < 0)
		    mprint ("You feel stupid.");
		else
		    newthing = make_shield (id);
		break;
	    case (WEAPON & 0xff):
		if (blessing > 0)
		    id = itemlist (WEAPONID, NUMWEAPONS);
		else
		    id = random_range (NUMWEAPONS);
		if (id < 0)
		    mprint ("You feel stupid.");
		else
		    newthing = make_weapon (id);
		break;
	    case (BOOTS & 0xff):
		if (blessing > 0)
		    id = itemlist (BOOTID, NUMBOOTS);
		else
		    id = random_range (NUMBOOTS);
		if (id < 0)
		    mprint ("You feel stupid.");
		else
		    newthing = make_boots (id);
		break;
	    case (CLOAK & 0xff):
		if (blessing > 0)
		    id = itemlist (CLOAKID, NUMCLOAKS);
		else
		    id = random_range (NUMCLOAKS);
		if (id < 0)
		    mprint ("You feel stupid.");
		else
		    newthing = make_cloak (id);
		break;
	    case (FOOD & 0xff):
		if (blessing > 0)
		    id = itemlist (FOODID, NUMFOODS);
		else
		    id = random_range (NUMFOODS);
		if (id < 0)
		    mprint ("You feel stupid.");
		else
		    newthing = make_food (id);
		break;
	    case (THING & 0xff):
		if (blessing > 0)
		    id = itemlist (THINGID, NUMTHINGS);
		else
		    id = random_range (NUMTHINGS);
		if (id < 0)
		    mprint ("You feel stupid.");
		else
		    newthing = make_thing (id);
		break;
	    case (ARTIFACT & 0xff):
		if (gamestatusp (CHEATED))
		    id = itemlist (ARTIFACTID, NUMARTIFACTS);
		else
		    id = -1;
		if (id < 0)
		    mprint ("You feel stupid.");
		else
		    newthing = make_artifact (id);
		break;
	    default:
		mprint ("You feel stupid.");
	}
	xredraw();
	if (id != ABORT) {
	    if (blessing > 0)
		learn_object (newthing);
	    gain_item (newthing);
	}
    }
}

void knowledge (int blessing)
{
    if (blessing < 0)
	mprint ("You feel ignorant.");
    else {
	mprint ("You feel knowledgeable!");
	menuclear();
	menuprint ("Current Point Total: ");
	menulongprint (calc_points());
	menuprint ("\nAlignment:");
	if (Player.alignment == 0)
	    menuprint ("Neutral, embodying the Cosmic Balance");
	else if (absv (Player.alignment) < 10)
	    menuprint ("Neutral, tending toward ");
	else if (absv (Player.alignment) < 50)
	    menuprint ("Neutral-");
	else if (absv (Player.alignment) < 100);
	else if (absv (Player.alignment) < 200)
	    menuprint ("Servant of ");
	else if (absv (Player.alignment) < 400)
	    menuprint ("Master of ");
	else if (absv (Player.alignment) < 800)
	    menuprint ("The Essence of ");
	else
	    menuprint ("The Ultimate Avatar of ");
	if (Player.alignment < 0)
	    menuprint ("Chaos\n");
	else if (Player.alignment > 0)
	    menuprint ("Law\n");
	showmenu();
	morewait();
	menuclear();
	menuprint ("Current stati:\n");
	if (Player.status[BLINDED])
	    menuprint ("Blinded\n");
	if (Player.status[SLOWED])
	    menuprint ("Slowed\n");
	if (Player.status[HASTED])
	    menuprint ("Hasted\n");
	if (Player.status[DISPLACED])
	    menuprint ("Displaced\n");
	if (Player.status[SLEPT])
	    menuprint ("Slept\n");
	if (Player.status[DISEASED])
	    menuprint ("Diseased\n");
	if (Player.status[POISONED])
	    menuprint ("Poisoned\n");
	if (Player.status[BREATHING])
	    menuprint ("Breathing\n");
	if (Player.status[INVISIBLE])
	    menuprint ("Invisible\n");
	if (Player.status[REGENERATING])
	    menuprint ("Regenerating\n");
	if (Player.status[VULNERABLE])
	    menuprint ("Vulnerable\n");
	if (Player.status[BERSERK])
	    menuprint ("Berserk\n");
	if (Player.status[IMMOBILE])
	    menuprint ("Immobile\n");
	if (Player.status[ALERT])
	    menuprint ("Alert\n");
	if (Player.status[AFRAID])
	    menuprint ("Afraid\n");
	if (Player.status[ACCURATE])
	    menuprint ("Accurate\n");
	if (Player.status[HERO])
	    menuprint ("Heroic\n");
	if (Player.status[LEVITATING])
	    menuprint ("Levitating\n");
	if (Player.status[TRUESIGHT])
	    menuprint ("Sharp\n");
	if (Player.status[SHADOWFORM])
	    menuprint ("Shadowy\n");
	if (Player.status[ILLUMINATION])
	    menuprint ("Glowing\n");
	if (Player.status[DEFLECTION])
	    menuprint ("Buffered\n");
	if (Player.status[RETURNING])
	    menuprint ("Returning\n");
	showmenu();
	morewait();
	menuclear();
	menuprint ("Immunities:\n");
	if (Player.immune_to (NORMAL_DAMAGE))
	    menuprint ("Normal Damage\n");
	if (Player.immune_to (FLAME))
	    menuprint ("Flame\n");
	if (Player.immune_to (ELECTRICITY))
	    menuprint ("Electricity\n");
	if (Player.immune_to (COLD))
	    menuprint ("Cold\n");
	if (Player.immune_to (POISON))
	    menuprint ("Poison\n");
	if (Player.immune_to (ACID))
	    menuprint ("Acid\n");
	if (Player.immune_to (FEAR))
	    menuprint ("Fear\n");
	if (Player.immune_to (SLEEP))
	    menuprint ("Sleep\n");
	if (Player.immune_to (NEGENERGY))
	    menuprint ("Negative Energies\n");
	if (Player.immune_to (THEFT))
	    menuprint ("Theft\n");
	if (Player.immune_to (GAZE))
	    menuprint ("Gaze\n");
	if (Player.immune_to (INFECTION))
	    menuprint ("Infection\n");
	showmenu();
	morewait();
	menuclear();
	menuprint ("Ranks:\n");
	switch (Player.rank[LEGION]) {
	    case COMMANDANT:
		menuprint ("Commandant of the Legion");
		break;
	    case COLONEL:
		menuprint ("Colonel of the Legion");
		break;
	    case FORCE_LEADER:
		menuprint ("Force Leader of the Legion");
		break;
	    case CENTURION:
		menuprint ("Centurion of the Legion");
		break;
	    case LEGIONAIRE:
		menuprint ("Legionaire");
		break;
	}
	if (Player.rank[LEGION] >= LEGIONAIRE) {
	    menuprint (" (");
	    menunumprint (Player.guildxp[LEGION]);
	    menuprint (" XP).\n");
	}
	switch (Player.rank[ARENA]) {
	    case FORMER_GLADIATOR:
		menuprint ("Ex-gladiator\n");
		break;
	    case CHAMPION:
		menuprint ("Gladiator Champion");
		break;
	    case GLADIATOR:
		menuprint ("Gladiator of the Arena");
		break;
	    case RETIARIUS:
		menuprint ("Retiarius of the Arena");
		break;
	    case BESTIARIUS:
		menuprint ("Bestiarius of the Arena");
		break;
	    case TRAINEE:
		menuprint ("Gladiator Trainee of the Arena");
		break;
	}
	if (Player.rank[ARENA] >= TRAINEE) {
	    menuprint (" (Opponent ");
	    menunumprint (Arena_Opponent);
	    menuprint (")\n");
	}
	switch (Player.rank[COLLEGE]) {
	    case ARCHMAGE:
		menuprint ("Archmage of the Collegium Magii");
		break;
	    case MAGE:
		menuprint ("Collegium Magii: Mage");
		break;
	    case PRECEPTOR:
		menuprint ("Collegium Magii: Preceptor");
		break;
	    case STUDENT:
		menuprint ("Collegium Magii: Student");
		break;
	    case NOVICE:
		menuprint ("Collegium Magii: Novice");
		break;
	}
	if (Player.rank[COLLEGE] >= NOVICE) {
	    menuprint (" (");
	    menunumprint (Player.guildxp[COLLEGE]);
	    menuprint (" XP).\n");
	}
	switch (Player.rank[NOBILITY]) {
	    case DUKE:
		menuprint ("Duke of Rampart");
		break;
	    case LORD:
		menuprint ("Peer of the Realm");
		break;
	    case KNIGHT:
		menuprint ("Order of the Knights of Rampart");
		break;
	    case ESQUIRE:
		menuprint ("Squire of Rampart");
		break;
	    case COMMONER:
		menuprint ("Commoner");
		break;
	    default:
		menuprint ("Lowly Commoner\n");
		break;
	}
	if (Player.rank[NOBILITY] > COMMONER) {
	    menuprint (" (");
	    menunumprint (Player.rank[NOBILITY] - 1);
	    menuprint (ordinal (Player.rank[NOBILITY] - 1));
	    menuprint (" Quest Completed)\n");
	} else if (Player.rank[NOBILITY] == 1) {
	    menuprint (" (1st Quest Undertaken)\n");
	}
	switch (Player.rank[CIRCLE]) {
	    case FORMER_SOURCEROR:
		menuprint ("Former member of the Circle.\n");
		break;
	    case PRIME:
		menuprint ("Prime Sorceror of the Inner Circle");
		break;
	    case HIGHSORCEROR:
		menuprint ("High Sorceror of the Inner Circle");
		break;
	    case SORCEROR:
		menuprint ("Member of the Circle of Sorcerors");
		break;
	    case ENCHANTER:
		menuprint ("Member of the Circle of Enchanters");
		break;
	    case INITIATE:
		menuprint ("Member of the Circle of Initiates");
		break;
	}
	if (Player.rank[CIRCLE] >= INITIATE) {
	    menuprint (" (");
	    menunumprint (Player.guildxp[CIRCLE]);
	    menuprint (" XP).\n");
	}
	switch (Player.rank[ORDER]) {
	    case FORMER_PALADIN:
		menuprint ("Washout from the Order of Paladins\n");
		break;
	    case JUSTICIAR:
		menuprint ("Justiciar of the Order of Paladins");
		break;
	    case PALADIN:
		menuprint ("Paladin of the Order");
		break;
	    case CHEVALIER:
		menuprint ("Chevalier of the Order");
		break;
	    case GUARDIAN:
		menuprint ("Guardian of the Order");
		break;
	    case GALLANT:
		menuprint ("Gallant of the Order");
		break;
	}
	if (Player.rank[ORDER] >= GALLANT) {
	    menuprint (" (");
	    menunumprint (Player.guildxp[ORDER]);
	    menuprint (" XP).\n");
	}
	switch (Player.rank[THIEVES]) {
	    case SHADOWLORD:
		menuprint ("Guild of Thieves: Shadowlord");
		break;
	    case TMASTER:
		menuprint ("Guild of Thieves: Master Thief");
		break;
	    case THIEF:
		menuprint ("Guild of Thieves: Thief");
		break;
	    case ATHIEF:
		menuprint ("Guild of Thieves: Apprentice Thief");
		break;
	    case TMEMBER:
		menuprint ("Guild of Thieves: Candidate Member");
		break;
	}
	if (Player.rank[THIEVES] >= TMEMBER) {
	    menuprint (" (");
	    menunumprint (Player.guildxp[THIEVES]);
	    menuprint (" XP).\n");
	}
	switch (Player.rank[PRIESTHOOD]) {
	    case LAY:
		menuprint ("A lay devotee of ");
		break;
	    case ACOLYTE:
		menuprint ("An Acolyte of ");
		break;
	    case PRIEST:
		menuprint ("A Priest of ");
		break;
	    case SPRIEST:
		menuprint ("A Senior Priest of ");
		break;
	    case HIGHPRIEST:
		menuprint ("The High Priest of ");
		break;
	}
	switch (Player.patron) {
	    case ODIN:
		menuprint ("Odin");
		break;
	    case SET:
		menuprint ("Set");
		break;
	    case ATHENA:
		menuprint ("Athena");
		break;
	    case HECATE:
		menuprint ("Hecate");
		break;
	    case DRUID:
		menuprint ("Druidism");
		break;
	    case DESTINY:
		menuprint ("the Lords of Destiny");
		break;
	}
	if (Player.rank[PRIESTHOOD] >= LAY) {
	    menuprint (" (");
	    menunumprint (Player.guildxp[PRIESTHOOD]);
	    menuprint (" XP).\n");
	}
	if (Player.rank[ADEPT])
	    menuprint ("**************\n*Omegan Adept*\n**************\n");
	showmenu();
	morewait();
	xredraw();
    }
}

// Recreates the current level
void flux (int blessing UNUSED)
{
    mprint ("The universe warps around you!");
    if (Level->environment == E_CITY) {
	mprint ("Sensing dangerous high order magic, the Collegium Magii");
	mprint ("and the Circle of Sorcerors join forces to negate the spell.");
	mprint ("You are zapped by an antimagic ray!!!");
	dispel (-1);
	mprint ("The universe unwarps itself....");
    } else if (Level->IsDungeon()) {
	mprint ("You stagger as the very nature of reality warps!");
	erase_level();
	Level->generated = false;
	mprint ("The fabric of spacetime reknits....");
	change_level (Level->depth - 1, Level->depth, true);
    } else
	mprint ("Odd.... No effect!");
}

//Turns on displacement status for the player
void displace (int blessing)
{
    if (blessing > -1) {
	mprint ("You feel a sense of dislocation.");
	Player.status[DISPLACED] = blessing + random_range (6);
    } else {
	mprint ("You feel vulnerable");
	Player.status[VULNERABLE] += random_range (6) - blessing;
    }
}

void invisible (int blessing)
{
    if (blessing > -1) {
	mprint ("You feel transparent!");
	Player.status[INVISIBLE] += 2 + 5 * blessing;
    } else {
	mprint ("You feel dangerous!");
	Player.status[VULNERABLE] += random_range (10) + 1;
    }
}

void warp (int blessing)
{
    int newlevel;
    if (!Level->IsDungeon())
	mprint ("How strange! No effect....");
    else {
	mprint ("Warp to which level? ");
	newlevel = (int) parsenum();
	if (newlevel >= Level->MaxDepth() || blessing < 0 || newlevel < 1) {
	    mprint ("You have been deflected!");
	    newlevel = random_range (Level->MaxDepth() - 1) + 1;
	}
	mprint ("You dematerialize...");
	change_level (Level->depth, newlevel, false);
    }
    roomcheck();
}

void alert (int blessing)
{
    if (blessing > -1) {
	mprint ("You feel on-the-ball.");
	Player.status[ALERT] += 4 + (5 * blessing);
    } else
	sleep_player (absv (blessing) + 3);
}

void regenerate (int blessing)
{
    if (blessing < 0)
	heal (blessing * 10);
    else {
	mprint ("You feel abnormally healthy.");
	Player.status[REGENERATING] += (blessing + 1) * 50;
    }
}

void haste (int blessing)
{
    if (blessing > -1) {
	if (!Player.status[HASTED])
	    mprint ("The world slows down!");
	else
	    mprint ("Nothing much happens.");
	if (Player.status[SLOWED])
	    Player.status[SLOWED] = 0;
	Player.status[HASTED] += (blessing * 100) + random_range (250);
    } else {
	mprint ("You feel slower.");
	if (Player.status[HASTED] > 0)
	    mprint ("...but the feeling quickly fades.");
	else
	    Player.status[SLOWED] += random_range (250) + 250;
    }
}

void recover_stat (int blessing)
{
    if (blessing < 0) {
	mprint ("You feel a cold surge!");
	switch (random_range (6)) {
	    case 0:
		Player.str = min (Player.str - 1, Player.maxstr - 1);
		break;
	    case 1:
		Player.con = min (Player.con - 1, Player.maxcon - 1);
		break;
	    case 2:
		Player.dex = min (Player.dex - 1, Player.maxdex - 1);
		break;
	    case 3:
		Player.agi = min (Player.agi - 1, Player.maxagi - 1);
		break;
	    case 4:
		Player.iq = min (Player.iq - 1, Player.maxiq - 1);
		break;
	    case 5:
		Player.pow = min (Player.pow - 1, Player.maxpow - 1);
		break;
	}
    } else {
	mprint ("You feel a warm tingle!");
	Player.str = max (Player.str, Player.maxstr);
	Player.con = max (Player.con, Player.maxcon);
	Player.dex = max (Player.dex, Player.maxdex);
	Player.agi = max (Player.agi, Player.maxagi);
	Player.iq = max (Player.iq, Player.maxiq);
	Player.pow = max (Player.pow, Player.maxpow);
	Player.maxhp = max<uint16_t> (Player.maxhp, Player.maxcon);
    }
    calc_melee();
}

void augment (int blessing)
{
    if (blessing < 0) {
	mprint ("You feel a cold surge!");
	switch (random_range (6)) {
	    case 0:
		Player.str = min (Player.str - 1, Player.maxstr - 1);
		break;
	    case 1:
		Player.con = min (Player.con - 1, Player.maxcon - 1);
		break;
	    case 2:
		Player.dex = min (Player.dex - 1, Player.maxdex - 1);
		break;
	    case 3:
		Player.agi = min (Player.agi - 1, Player.maxagi - 1);
		break;
	    case 4:
		Player.iq = min (Player.iq - 1, Player.maxiq - 1);
		break;
	    case 5:
		Player.pow = min (Player.pow - 1, Player.maxpow - 1);
		break;
	}
    } else if (blessing == 0) {
	mprint ("You feel a warm tingle!");
	switch (random_range (6)) {
	    case 0:
		Player.str = max (Player.str + 1, Player.maxstr + 1);
		break;
	    case 1:
		Player.con = max (Player.con + 1, Player.maxcon + 1);
		break;
	    case 2:
		Player.dex = max (Player.dex + 1, Player.maxdex + 1);
		break;
	    case 3:
		Player.agi = max (Player.agi + 1, Player.maxagi + 1);
		break;
	    case 4:
		Player.iq = max (Player.iq + 1, Player.maxiq + 1);
		break;
	    case 5:
		Player.pow = max (Player.pow + 1, Player.maxpow + 1);
		break;
	}
    } else {
	mprint ("You feel a hot flash!");
	Player.str = max (Player.str + 1, Player.maxstr + 1);
	Player.con = max (Player.con + 1, Player.maxcon + 1);
	Player.dex = max (Player.dex + 1, Player.maxdex + 1);
	Player.agi = max (Player.agi + 1, Player.maxagi + 1);
	Player.iq = max (Player.iq + 1, Player.maxiq + 1);
	Player.pow = max (Player.pow + 1, Player.maxpow + 1);
    }
    calc_melee();
}

void breathe (int blessing)
{
    if (blessing > -1) {
	mprint ("Your breath is energized!");
	Player.status[BREATHING] += 6 + blessing;
    } else {
	mprint ("You choke as your lungs fill with water!");
	p_damage (50, UNSTOPPABLE, "drowning");
    }
}

void i_chaos (pob o UNUSED)
{
    if (Player.alignment < 0) {
	Player.alignment -= random_range (20);
	mprint ("You feel deliciously chaotic!");
	gain_experience (absv (Player.alignment) * 10);
    } else {
	mprint ("You feel a sense of inner turmoil!");
	Player.alignment -= random_range (20);
    }
}

void i_law (pob o UNUSED)
{
    if (Player.alignment > 0) {
	Player.alignment += random_range (20);
	mprint ("You feel wonderfully lawful!");
	gain_experience (Player.alignment * 10);
    } else {
	mprint ("You feel a sense of inner constraint!");
	Player.alignment += random_range (20);
    }
}

void sanctify (int blessing)
{
    if (blessing > -1) {
	if (Level->environment == E_TEMPLE)
	    mprint ("Odd, the spell has no effect. I wonder why.");
	else if (Level->site(Player.x,Player.y).locchar == ALTAR)
	    mprint ("This site can't get any holier!");
	else if (Player.patron == 0) {
	    mprint ("The gods are angered!");
	    Level->site(Player.x,Player.y).locchar = LAVA;
	    Level->site(Player.x,Player.y).p_locf = L_LAVA;
	    lset (Player.x, Player.y, CHANGED);
	    p_movefunction (L_LAVA);
	} else {
	    Level->site(Player.x,Player.y).locchar = ALTAR;
	    Level->site(Player.x,Player.y).aux = Player.patron;
	    Level->site(Player.x,Player.y).p_locf = L_ALTAR;
	    lset (Player.x, Player.y, CHANGED);
	    mprint ("You are standing on sacred ground!");
	}
    } else {
	if (Level->site(Player.x,Player.y).locchar == ALTAR) {
	    mprint ("The altar crumbles before your unholy blast....");
	    Level->site(Player.x,Player.y).locchar = FLOOR;
	    Level->site(Player.x,Player.y).p_locf = L_NO_OP;
	    lset (Player.x, Player.y, CHANGED);
	    if (Level->site(Player.x,Player.y).aux == Player.patron) {
		mprint ("Your deity is not amused....");
		p_damage (Player.hp - 1, UNSTOPPABLE, "Divine Wrath");
	    } else if ((Player.patron == ATHENA) || (Player.patron == ODIN)) {
		if ((Level->site(Player.x,Player.y).aux == SET) || (Level->site(Player.x,Player.y).aux == HECATE)) {
		    mprint ("Your deity applauds the eradication of Chaos' taint");
		    gain_experience (1000);
		} else {
		    mprint ("Your deity approves of your action.");
		    gain_experience (100);
		}
	    } else if ((Player.patron == SET) || (Player.patron == HECATE)) {
		if ((Level->site(Player.x,Player.y).aux == ODIN) || (Level->site(Player.x,Player.y).aux == ATHENA)) {
		    mprint ("Your deity applauds the obliteration of Law");
		    gain_experience (1000);
		} else {
		    mprint ("Your deity approves of your action.");
		    gain_experience (100);
		}
	    } else if (Player.patron == DRUID) {
		mprint ("Your attempt to maintain the Balance is applauded....");
		gain_experience (250);
	    } else
		mprint ("Nothing much happens");
	} else
	    mprint ("You feel an aura of unholiness arising from this spot....");
    }
}

void accuracy (int blessing)
{
    if (blessing > -1) {
	mprint ("Everything seems covered in bullseyes!");
	Player.status[ACCURACY] += random_range (5) + 1 + blessing * 5;
	calc_melee();
    } else {
	Player.status[ACCURACY] = 0;
	calc_melee();
	mprint ("Your vision blurs...");
    }
}

// if know id, then summon that monster; else (if < 0) get one.
void summon (int blessing, int id)
{
    int looking = true, x = 0, y = 0;

    if (id < 0) {
	if (blessing > 0) {
	    id = selectmonster();
	    xredraw();
	}
	// for (id ==0) case, see below -- get a "fair" monster
	else if (blessing < 0)
	    id = random_range (NUMMONSTERS);
    }
    for (int i = 0; ((i < 8) && looking); i++) {
	x = Player.x + Dirs[0][i];
	y = Player.y + Dirs[1][i];
	looking = (!inbounds (x, y) || Level->site(x,y).locchar != FLOOR || Level->creature(x,y));
    }

    if (!looking) {
	monster& m = make_site_monster (x, y, (!blessing && id == RANDOM) ? RANDOM : id);
	if (blessing > 0)
	    m_status_reset (m, HOSTILE);
	else if (blessing < 0)
	    m_status_set (m, HOSTILE);
    }
}

static int itemlist (int itemindex, int num)
{
    int i, itemno;

    mprint ("Show ID list? ");
    if (ynq() == 'y') {
	menuclear();
	for (i = 0; i < num; i++) {
	    menunumprint (i + 1);
	    menuprint (":");
	    menuprint (Objects[i + itemindex].truename);
	    menuprint ("\n");
	}
	showmenu();
    }
    mprint ("Item ID? ");
    itemno = (int) parsenum() - 1;
    if ((itemno >= num) || (itemno < 0))
	itemno = ABORT;
    return (itemno);
}

static int selectmonster (void)
{
    int i, itemno;
    mprint ("Show ID list? ");
    if (ynq() == 'y')
	do {
	    clearmsg();
	    mprint ("Summon monster: ");
	    menuclear();
	    for (i = 0; i < NUMMONSTERS; i++) {
		menunumprint (i + 1);
		menuprint (":");
		menuprint (Monsters[i].monstring);
		menuprint ("\n");
	    }
	    showmenu();
	    itemno = (int) parsenum() - 1;
	    if ((itemno < 0) || (itemno > NUMMONSTERS - 1)) {
		mprint ("How about trying a real monster?");
		morewait();
	    }
	} while ((itemno < 0) || (itemno > NUMMONSTERS - 1));
    else
	do {
	    mprint ("Summon monster: ");
	    itemno = (int) parsenum() - 1;
	} while ((itemno < 0) || (itemno > NUMMONSTERS - 1));
    return (itemno);
}

// uncurse all items, cure diseases, and neutralize poison
void cleanse (int blessing)
{
    if (blessing > -1) {
	if (blessing > 0) {
	    for (unsigned i = 0; i < MAXITEMS; i++) {
		if (Player.has_possession(i)) {
		    if ((Player.possessions[i].used) && (Player.possessions[i].blessing < 0)) {
			Player.possessions[i].used = false;
			item_use (&Player.possessions[i]);
			Player.possessions[i].blessing = 0;
			Player.possessions[i].used = true;
			item_use (&Player.possessions[i]);
		    }
		}
	    }
	}
	if (Player.status[POISONED] > 0)
	    Player.status[POISONED] = 0;
	if (Player.status[DISEASED] > 0)
	    Player.status[DISEASED] = 0;
	showflags();
	mprint ("You feel radiant!");
    } else {
	Player.status[POISONED] += 10;
	Player.status[DISEASED] += 10;
	mprint ("You feel besmirched!");
	showflags();
    }
}

void annihilate (int blessing)
{
    if (blessing == 0) {
	mprint ("Lightning strikes flash all around you!!!");
	for (int i = 0; i < 9; i++) {
	    monster* m = Level->creature(Player.x + Dirs[0][i],Player.y + Dirs[1][i]);
	    if (m) m_death (m);
	}
    }
    if (blessing > 0) {
	if (Level->environment == E_COUNTRYSIDE) {
	    clearmsg();
	    mprint ("Bolts of lightning flash down for as far as you can see!!!");
	    morewait();
	    mprint ("There is a rain of small birds and insects from the sky, and you");
	    mprint ("notice that you can't hear any animal noises around here any more...");
	    Player.alignment -= 3;
	} else {
	    mprint ("Thousands of bolts of lightning flash throughout the level!!!");
	    foreach (m, Level->mlist)
		if (m->hp > 0)
		    m_death (&*m);
	}
    } else {
	mprint ("You are hit by a bolt of mystic lightning!");
	p_death ("self-annihilation");
    }
}

void sleep_monster (int blessing)
{
    int x = Player.x, y = Player.y;
    struct monster *target;

    if (blessing == 0)
	setspot (&x, &y);

    if (blessing < 0)
	sleep_player (absv (blessing) + 2);
    else if (blessing > 0) {
	mprint ("A silence pervades the area.");
	foreach (m, Level->mlist) {
	    m_status_set (*m, ASLEEP);
	    m->wakeup = 0;
	}
    } else {
	target = Level->creature(x,y);
	if (target != NULL) {
	    if (target->uniqueness == COMMON) {
		strcpy (Str1, "The ");
		strcat (Str1, target->monstring);
	    } else
		strcpy (Str1, target->monstring);
	    if (!m_immunityp (target, SLEEP)) {
		strcat (Str1, " seems to have fallen asleep.");
		m_status_set (target, ASLEEP);
		target->wakeup = 0;
	    } else
		strcat (Str1, " is bright eyed, and bushy tailed!");
	    mprint (Str1);
	} else
	    mprint ("Nothing to sleep there!");
    }
}

void sleep_player (int amount)
{
    if (Player.status[SLEPT] == 0) {	// prevent player from sleeping forever
	mprint ("You feel sleepy...");
	if (!Player.immune_to (SLEEP)) {
	    Player.status[SLEPT] += random_range (amount * 2) + 2;
	} else
	    mprint ("but you shrug off the momentary lassitude.");
    }
}

void hide (int x, int y)
{
    if (inbounds (x, y)) {
	lset (x, y, SECRET);
	lset (x, y, CHANGED);
	putspot (x, y, WALL);
	mprint ("You feel sneaky.");
    }
}

void clairvoyance (int vision)
{
    int i, j;
    int x = Player.x, y = Player.y;
    mprint ("Clairvoyance... ");
    setspot (&x, &y);
    for (i = x - vision; i < x + vision + 1; i++) {
	for (j = y - vision; j < y + vision + 1; j++) {
	    if (inbounds (i, j)) {
		lreset (i, j, SECRET);
		lset (i, j, CHANGED);
		lset (i, j, SEEN);
		dodrawspot (i, j);
	    }
	}
    }
    levelrefresh();
}

void aggravate (void)
{
    foreach (m, Level->mlist) {
	m_status_reset (*m, ASLEEP);
	m_status_set (*m, HOSTILE);
    }
}

void learnspell (int blessing)
{
    int i, done = false;
    if (blessing < 0) {
	for (i = NUMSPELLS; ((i > -1) && (!done)); i--) {
	    if (spell_is_known (ESpell(i))) {
		done = true;
		learn_object (SCROLL_SPELLS);
		mprint ("You feel forgetful.");
		forget_spell (ESpell(i));
	    }
	}
	if (i == ABORT)
	    mprint ("You feel fortunate.");
    } else {
	learn_object (SCROLL_SPELLS);
	ESpell spell = (ESpell) random_range (NUMSPELLS);
	if ((random_range (4 * Spells[spell].powerdrain) + Spells[spell].powerdrain) < (4 * Player.iq + 8 * Player.level)) {
	    mprintf ("Spell research successful: %s", spellid (spell));
	    if (spell_is_known (spell)) {
		mprint ("...is now easier to cast.");
		Spells[spell].powerdrain = ((int) ((Spells[spell].powerdrain + 1) / 2));
	    } else {
		mprint ("...is added to your repertoire");
		learn_spell (spell);
		gain_experience (Spells[spell].powerdrain * 10);
	    }
	} else
	    mprint ("Spell research unsuccessful.");
    }
}

void amnesia (void)
{
    for (unsigned j = 0; j < Level->height; j++)
	for (unsigned i = 0; i < Level->width; i++)
	    lreset (i, j, SEEN);
    erase_level();
    drawvision (Player.x, Player.y);
}

//affects player only
void level_drain (int levels, const char* source)
{
    int decrement = ((int) (Player.maxhp / (Player.level + 1)));

    Player.level -= levels;

    Player.maxhp -= (levels * decrement);
    Player.hp -= (levels * decrement);

    if ((Player.hp < 1) || (Player.level < 0))
	p_death (source);
}

void disrupt (int x, int y, int amount)
{
    struct monster *target;

    if (x == (int)Player.x && y == (int)Player.y) {
	mprint ("You feel disrupted!");
	p_damage (amount, NORMAL_DAMAGE, "magical disruption");
    } else {
	target = Level->creature(x,y);
	if (target != NULL) {
	    if (target->uniqueness == COMMON) {
		strcpy (Str1, "The ");
		strcat (Str1, target->monstring);
	    } else
		strcpy (Str1, target->monstring);
	    if (!m_immunityp (target, NORMAL_DAMAGE)) {
		strcat (Str1, " was blasted!");
		mprint (Str1);
		m_damage (target, amount, NORMAL_DAMAGE);
		target->wakeup = 0;
	    } else {
		strcat (Str1, " does not seem affected.");
		mprint (Str1);
	    }
	}
    }
}

void disintegrate (int x, int y)
{
    struct monster *target;
    if (!inbounds (x, y))
	mprint ("You feel a sense of wastage.");
    else if (x == (int)Player.x && y == (int)Player.y) {
	if (Player.has_possession(O_CLOAK)) {
	    mprint ("Your cloak disintegrates!");
	    Player.remove_possession (O_CLOAK);
	} else if (Player.has_possession(O_ARMOR)) {
	    mprint ("Your armor disintegrates!");
	    Player.remove_possession (O_ARMOR);
	} else {
	    mprint ("Uh, oh....");
	    mprint ("Zzzap! You've been disintegrated!");
	    p_damage (250, UNSTOPPABLE, "disintegration");
	}
    } else {
	if (!view_los_p (Player.x, Player.y, x, y))
	    setgamestatus (SUPPRESS_PRINTING);
	if ((target = Level->creature(x,y))) {
	    mprintf ("%s disintegrates!", target->name());
	    m_damage (target, 250, UNSTOPPABLE);
	    if (target->hp > 0)
		mprint ("It was partially protected by its armor.");
	} else if (Level->site(x,y).locchar == ALTAR) {
	    mprint ("Zzzzap! the altar seems unaffected...");
	    mprint ("But an angry deity retaliates....");
	    disintegrate (Player.x, Player.y);
	} else if (Level->site(x,y).p_locf == L_TRAP_PIT) {
	    if (Level->IsDungeon() && Level->depth < Level->MaxDepth()) {
		mprint ("A hole is blasted in the base of the pit!");
		Level->site(x,y).locchar = TRAP;
		Level->site(x,y).p_locf = L_TRAP_DOOR;
		Level->site(x,y).aux = S_DISINTEGRATE;
		lset (x, y, CHANGED);
	    } else
		mprint ("The hole just gets deeper....");
	} else if (Level->site(x,y).locchar == FLOOR) {
	    mprint ("You zap a hole in the floor!");
	    Level->site(x,y).locchar = TRAP;
	    Level->site(x,y).p_locf = L_TRAP_PIT;
	    lset (x, y, CHANGED);
	} else if ((Level->site(x,y).locchar == WALL) || (Level->site(x,y).locchar == OPEN_DOOR) || (Level->site(x,y).locchar == CLOSED_DOOR) || (Level->site(x,y).locchar == PORTCULLIS) || (Level->site(x,y).locchar == STATUE)) {
	    mprint ("The site is reduced to rubble!");
	    if (Level->site(x,y).locchar == WALL)
		Level->tunnelcheck();
	    Level->site(x,y).p_locf = L_RUBBLE;
	    Level->site(x,y).locchar = RUBBLE;
	    lreset (x, y, SECRET);
	    lset (x, y, CHANGED);
	} else if ((Level->site(x,y).locchar == RUBBLE) || (Level->site(x,y).locchar == TRAP)) {
	    mprint ("The site is blasted clear!");
	    Level->site(x,y).p_locf = L_NO_OP;
	    Level->site(x,y).locchar = FLOOR;
	    lreset (x, y, SECRET);
	    lset (x, y, CHANGED);
	} else if (Level->site(x,y).locchar == HEDGE) {
	    if (Level->site(x,y).p_locf == L_TRIFID) {
		mprint ("The trifid screams as it disintgrates!");
		gain_experience (50);
		Level->site(x,y).p_locf = L_NO_OP;
		Level->site(x,y).locchar = FLOOR;
		lreset (x, y, SECRET);
		lset (x, y, CHANGED);
	    } else {
		mprint ("The hedge is blasted away!");
		Level->site(x,y).p_locf = L_NO_OP;
		Level->site(x,y).locchar = FLOOR;
		lreset (x, y, SECRET);
		lset (x, y, CHANGED);
	    }
	} else
	    mprint ("The blast has no effect.");
	if (!view_los_p (Player.x, Player.y, x, y))
	    resetgamestatus (SUPPRESS_PRINTING);
	else
	    plotspot (x, y, true);
    }
}

void acid_cloud (void)
{
    mprint ("You are caught in an acid cloud!  ");
    if (Player.has_possession(O_CLOAK)) {
	damage_item (&Player.possessions[O_CLOAK]);
	mprint ("You are burned by acid.");
	p_damage (3, ACID, "an acid cloud");
    } else if (Player.has_possession(O_ARMOR)) {
	mprint ("You are burned by acid.");
	p_damage (3, ACID, "an acid cloud");
	damage_item (&Player.possessions[O_ARMOR]);
    } else if (Player.immune_to (ACID)) {
	mprint ("You resist the effects!");
	return;
    } else {
	mprint ("The acid eats away at your bare skin!");
	p_damage (25, ACID, "an acid cloud");
    }
}

// teleport player
void p_teleport (int type)
{
    int x = Player.x, y = Player.y;
    drawspot (x, y);
    if (type < 0) {
	x = random_range (Level->width);
	y = random_range (Level->height);
	if ((Level->site(x,y).locchar != FLOOR) && (Level->site(x,y).locchar != OPEN_DOOR)) {
	    mprint ("You teleported into a solid object....");
	    mprint ("You are dead!");
	    p_death ("teleportation into a solid object");
	} else {
	    Player.x = x;
	    Player.y = y;
	}
    } else if (type == 0) {
	findspace (&x, &y);
	Player.x = x; Player.y = y;
    } else {
	setspot (&x, &y);
	Player.x = x; Player.y = y;
	if (Level->site(Player.x,Player.y).locchar != FLOOR || Level->creature(Player.x,Player.y)) {
	    mprint ("You feel deflected.");
	    p_teleport (0);
	}
    }
    screencheck (Player.y);
    roomcheck();
}

void p_poison (int toxicity)
{
    mprint ("You feel sick.");
    if (!Player.immune_to (POISON))
	Player.status[POISONED] += toxicity;
    else
	mprint ("The sickness fades!");
    showflags();
}

void apport (int blessing)
{
    int i, idx, x = Player.x, y = Player.y;
    if (blessing > -1) {
	mprint ("Apport from:");
	setspot (&x, &y);
	if (Level->thing(x,y)) {
	    pickup_at (x, y);
	    plotspot (x, y, true);
	} else
	    mprint ("There's nothing there to apport!");
    } else {
	mprint ("You have a sense of loss.");
	for (i = 0; i < absv (blessing); i++) {
	    idx = random_item();
	    if (idx != ABORT) {
		drop_at (x, y, Player.possessions[idx]);
		Player.remove_possession (idx);
	    }
	}
    }
}

void strategic_teleport (int blessing)
{
    // WDT HACK: Game balance issue: the star gem is supposed to be the only
    // way out of the astral plane (including the Circle of Sorcerors).  However,
    // Hy Magic offers the Location wish, and some artifacts grant this
    // as well.  Seems to me that Hy Magic ought to allow it, and nothing
    // else (aside from the Star Gem, of course).
    if ((Level->environment == E_CIRCLE || Level->environment == E_ASTRAL) && !gamestatusp (CHEATED)) {
	mprint ("Some property of this eerie place interferes with the magic!\n");
	return;
    }
    mprint ("Magic portals open up all around you!");
    if (blessing < 0) {
	morewait();
	mprint ("You are dragged into one!");
	change_environment (E_COUNTRYSIDE);
	do {
	    Player.x = random_range (Level->width);
	    Player.y = random_range (Level->height);
	} while (Level->site(Player.x,Player.y).locchar == CHAOS_SEA);
    } else {
	mprint ("Below each portal is a caption. Enter which one:");
	menuclear();
	menuprint ("a: Rampart\n");
	menuprint ("b: Village of Star View\n");
	menuprint ("c: Village of Woodmere\n");
	menuprint ("d: Village of Stormwatch\n");
	menuprint ("e: Village of Thaumaris\n");
	menuprint ("f: Village of Skorch\n");
	menuprint ("g: Village of Whorfen\n");
	menuprint ("h: Temple of the Noose\n");
	menuprint ("i: The Parthenon\n");
	menuprint ("j: Temple of the Black Hand\n");
	menuprint ("k: Temple of the Hidden Moon\n");
	menuprint ("l: WoodHenge\n");
	menuprint ("m: Temple of Destiny\n");
	menuprint ("n: HellWell Volcano\n");
	if (gamestatusp (CHEATED))
	    menuprint ("z: Anywhere\n");
	menuprint ("ANYTHING ELSE: Avoid entering a portal.");
	showmenu();
	switch ((char) mcigetc()) {
	    case 'a':
		change_environment (E_COUNTRYSIDE);
		Player.x = 27;
		Player.y = 19;
		break;
	    case 'b':
		change_environment (E_COUNTRYSIDE);
		Player.x = 56;
		Player.y = 5;
		break;
	    case 'c':
		change_environment (E_COUNTRYSIDE);
		Player.x = 35;
		Player.y = 11;
		break;
	    case 'd':
		change_environment (E_COUNTRYSIDE);
		Player.x = 10;
		Player.y = 40;
		break;
	    case 'e':
		change_environment (E_COUNTRYSIDE);
		Player.x = 7;
		Player.y = 6;
		break;
	    case 'f':
		change_environment (E_COUNTRYSIDE);
		Player.x = 41;
		Player.y = 43;
		break;
	    case 'g':
		change_environment (E_COUNTRYSIDE);
		Player.x = 20;
		Player.y = 41;
		break;
	    case 'h':
		change_environment (E_COUNTRYSIDE);
		Player.x = 22;
		Player.y = 30;
		break;
	    case 'i':
		change_environment (E_COUNTRYSIDE);
		Player.x = 51;
		Player.y = 11;
		break;
	    case 'j':
		change_environment (E_COUNTRYSIDE);
		Player.x = 45;
		Player.y = 45;
		break;
	    case 'k':
		change_environment (E_COUNTRYSIDE);
		Player.x = 19;
		Player.y = 46;
		break;
	    case 'l':
		change_environment (E_COUNTRYSIDE);
		Player.x = 32;
		Player.y = 5;
		break;
	    case 'm':
		change_environment (E_COUNTRYSIDE);
		Player.x = 49;
		Player.y = 59;
		break;
	    case 'n':
		change_environment (E_COUNTRYSIDE);
		Player.x = 30;
		Player.y = 58;
		break;
	    default:
		if (gamestatusp (CHEATED)) {
		    mprint ("Enter environment number: ");
		    change_environment ((EEnvironment) parsenum());
		}
	}
	xredraw();
	if (gamestatusp (LOST)) {
	    mprint ("You know where you are now.");
	    resetgamestatus (LOST);
	    Precipitation = 0;
	}
    }
    setlastxy (Player.x, Player.y);
    screencheck (Player.y);
    drawvision (Player.x, Player.y);
    if (Level->environment == E_COUNTRYSIDE)
	terrain_check (false);
}

void hero (int blessing)
{
    if (blessing > -1) {
	mprint ("You feel super!");
	Player.status[HERO] += random_range (5) + 1 + blessing;
	calc_melee();
    } else {
	Player.status[HERO] = 0;
	calc_melee();
	mprint ("You feel cowardly.");
	level_drain (absv (blessing), "a potion of cowardice");
    }
}

void levitate (int blessing)
{
    if (blessing > -1) {
	if (gamestatusp (MOUNTED))
	    mprint ("You have a strange feeling of lightness in your saddle.");
	else {
	    mprint ("You start to float a few inches above the floor.");
	    mprint ("You discover you can easily control your altitude...");
	    mprint ("(Note use of '@' command may be useful while levitating)");
	    Player.status[LEVITATING] += random_range (5) + 1 + blessing;
	}
    } else
	mprint ("Nothing much happens.");
}

// has effect of switching between 1st level and deepest level attained
void level_return (void)
{
    if (Level->IsDungeon()) {
	mprint ("The vortex of mana carries you off!");
	if (Level->depth > 1)
	    change_level (Level->depth, 1, false);
	else
	    change_level (Level->depth, deepest[Level->environment], false);
    } else if (Level->environment == E_COUNTRYSIDE) {
	mprint ("A mysterious force wafts you back home!");
	Player.x = 27;
	Player.y = 19;
	screencheck (Player.y);
	drawvision (Player.x, Player.y);
	locprint ("Back Outside Rampart.");
    } else
	mprint ("A feeble vortex of magic swirls by and has no further effect.");
}

void cure (int blessing)
{
    int happened = false;
    if (blessing > -1) {
	if (Player.status[DISEASED]) {
	    Player.status[DISEASED] = 0;
	    mprint ("You feel hygienic!");
	    happened = true;
	}
	if (Player.status[POISONED]) {
	    Player.status[POISONED] -= 5 + blessing * 10;
	    if (Player.status[POISONED] > 0)
		mprint ("The effect of the poison has been reduced.");
	    else {
		Player.status[POISONED] = 0;
		mprint ("The poison has been purged from your system.");
	    }
	    happened = true;
	}
	if (Player.status[BLINDED]) {
	    Player.status[BLINDED] = 0;
	    happened = true;
	    mprint ("Cobwebs clear from before your eyes.");
	}
	if (!happened)
	    mprint ("Nothing much happens.");
    } else
	disease (12);
    showflags();
}

void disease (int amount)
{
    mprint ("You feel ill.");
    if (!Player.immunity[INFECTION]) {
	mprint ("You begin to shiver with ague.");
	Player.status[DISEASED] += random_range (amount * 2) + 1;
    } else
	mprint ("The illness fades.");
}

void truesight (int blessing)
{
    if (blessing > -1) {
	Player.status[TRUESIGHT] += random_range (10) + 1;
	mprint ("You feel sharp.");
    } else {
	Player.status[BLINDED] += random_range (10) + 1;
	mprint ("You've been blinded!");
    }
}

void dispel (int blessing)
{
    int i, x = Player.x, y = Player.y;
    monster* target;
    if (blessing > -1) {
	setspot (&x, &y);
	if (x == (int)Player.x && y == (int)Player.y) {
	    for (i = 0; i < MAXITEMS; i++) {
		if (!Player.has_possession(i))
		    continue;
		object& o = Player.possessions[i];
		if (o.used && o.blessing < 0) {
		    if (blessing + 1 + o.blessing >= 0) {
			o.used = false;
			setgamestatus (SUPPRESS_PRINTING);
			item_use (&o);
			resetgamestatus (SUPPRESS_PRINTING);
			mprint ("You hear a sighing sound from");
			mprint (itemid (o));
			o.blessing = 0;
			o.used = true;
			setgamestatus (SUPPRESS_PRINTING);
			item_use (&o);
			resetgamestatus (SUPPRESS_PRINTING);
		    } else {
			mprint ("You hear dark laughter from");
			mprint (itemid (o));
		    }
		}
	    }
	} else if ((target = Level->creature(x,y))) {
	    if (target->level < blessing * 3) {
		target->specialf = M_NO_OP;
		if (target->meleef != M_NO_OP)
		    target->meleef = M_MELEE_NORMAL;
		target->strikef = M_NO_OP;
		target->immunity = 0;
		m_status_reset (*target, M_INVISIBLE);
		m_status_reset (*target, INTANGIBLE);
	    } else
		mprint ("The monster ignores the effect!");
	} else if ((Level->site(x,y).p_locf == L_TRAP_FIRE) || (Level->site(x,y).p_locf == L_STATUE_WAKE) || (Level->site(x,y).p_locf == L_TRAP_TELEPORT) || (Level->site(x,y).p_locf == L_TRAP_DISINTEGRATE)) {
	    Level->site(x,y).p_locf = L_NO_OP;
	    if (Level->site(x,y).locchar == TRAP)
		Level->site(x,y).locchar = FLOOR;
	    lset (x, y, CHANGED);
	} else if (Level->site(x,y).p_locf == L_MAGIC_POOL)
	    Level->site(x,y).p_locf = L_WATER;
	else
	    mprint ("Nothing much seems to happen.");
    } else {
	mprint ("A smell of ozone and positive ions fills the air..");
	if (Player.status[ACCURACY] && (Player.status[ACCURACY] < 1000))
	    Player.status[ACCURACY] = 1;
	if (Player.status[DISPLACED] && (Player.status[DISPLACED] < 1000))
	    Player.status[DISPLACED] = 1;
	if (Player.status[HASTED] && (Player.status[HASTED] < 1000))
	    Player.status[HASTED] = 1;
	if (Player.status[BREATHING] && (Player.status[BREATHING] < 1000))
	    Player.status[BREATHING] = 1;
	if (Player.status[INVISIBLE] && (Player.status[INVISIBLE] < 1000))
	    Player.status[INVISIBLE] = 1;
	if (Player.status[REGENERATING] && (Player.status[REGENERATING] < 1000))
	    Player.status[REGENERATING] = 1;
	if (Player.status[ALERT] && (Player.status[ALERT] < 1000))
	    Player.status[ALERT] = 1;
	if (Player.status[HERO] && (Player.status[HERO] < 1000))
	    Player.status[HERO] = 1;
	if (Player.status[LEVITATING] && (Player.status[LEVITATING] < 1000))
	    Player.status[LEVITATING] = 1;
	if (Player.status[ACCURATE] && (Player.status[ACCURATE] < 1000))
	    Player.status[ACCURATE] = 1;
	if (Player.status[TRUESIGHT] && (Player.status[TRUESIGHT] < 1000))
	    Player.status[TRUESIGHT] = 1;
	tenminute_status_check();
    }
}

void polymorph (int blessing)
{
    int x = Player.x, y = Player.y, newmonster;
    struct monster *m;
    setspot (&x, &y);
    clearmsg();
    if (x == (int)Player.x && y == (int)Player.y) {
	// WDT HACK: shouldn't this use one of the 'getarticle' functions
	// to prevent things like "a elder grue" (should be "an elder grue")?
	mprint ("You enjoy your new life as a");
	mprint (Monsters[random_range (NUMMONSTERS)].monstring);
	mprint ("But your game is over....");
	p_death ("polymorphing oneself");
    } else if ((m = Level->creature(x,y)) == NULL)
	mprint ("Nothing happens.");
    else {
	if (m_immunityp (m, OTHER_MAGIC) || (m->level > random_range (12))) {
	    strcpy (Str1, "The ");
	    strcat (Str1, m->monstring);
	    strcat (Str1, " resists the change!");
	    m_status_set (m, HOSTILE);
	} else {
	    if (blessing < 0) {
		do
		    newmonster = random_range (NUMMONSTERS);
		while ((newmonster == NPC) || (newmonster == MAST_THIEF) || (Monsters[newmonster].level <= m->level) || (Monsters[newmonster].uniqueness != COMMON));
	    } else {
		do
		    newmonster = random_range (NUMMONSTERS);
		while ((newmonster == NPC) || (newmonster == MAST_THIEF) || (Monsters[newmonster].uniqueness != COMMON));
	    }
	    // WDT HACK: most of this could (and should) be implemented by 
	    // the following line: "*m = Monsters[newmonster];".  The exception,
	    // of course, are the parts where the new monster inherits the old
	    // one's abilities.  This would be better because it would be robust
	    // even in the face of additions to the monster structure.
	    m->id = Monsters[newmonster].id;
	    m->hp = max<int16_t> (m->hp, Monsters[newmonster].id);
	    m->speed = Monsters[newmonster].speed;
	    m->hit = Monsters[newmonster].hit;
	    m->ac = Monsters[newmonster].ac;
	    m->dmg = Monsters[newmonster].dmg;
	    m->sense = Monsters[newmonster].sense;
	    m->wakeup = Monsters[newmonster].wakeup;
	    m->level = max (m->level, Monsters[newmonster].level);
	    m->status = Monsters[newmonster].status;
	    m->immunity = (m->immunity | Monsters[newmonster].immunity);
	    m->xpv = max<uint16_t> (m->xpv, Monsters[newmonster].wakeup);
	    m->transformid = Monsters[newmonster].transformid;
	    m->corpsevalue = Monsters[newmonster].corpsevalue;
	    m->corpseweight = Monsters[newmonster].corpseweight;
	    m->monchar = Monsters[newmonster].monchar;
	    m->meleestr = Monsters[newmonster].meleestr;
	    m->monstring = Monsters[newmonster].monstring;
	    m->corpsestr = Monsters[newmonster].corpsestr;
	    m->talkf = Monsters[newmonster].talkf;
	    m->movef = Monsters[newmonster].movef;
	    m->meleef = Monsters[newmonster].meleef;
	    m->strikef = Monsters[newmonster].strikef;
	    m->specialf = Monsters[newmonster].specialf;
	    m_status_set (m, HOSTILE);
	}
    }
}

void hellfire (int x, int y, int blessing)
{
    struct monster *m;
    if (x == (int) Player.x && y == (int) Player.y) {
	mprint ("You have been completely annihilated. Congratulations.");
	p_death ("hellfire");
    } else if ((m = Level->creature(x,y)) == NULL) {
	mprint ("The gods are angry over your waste of power...");
	level_drain (5, "indiscriminate use of hellfire");
    } else {
	mprint ("The monster writhes in the flames...");
	if (blessing < 0) {
	    mprint ("...and appears stronger.");
	    morewait();
	    mprint ("Much stronger.");
	    m->hp += 1000;
	    m->hit += 20;
	    m->dmg += 100;
	    m_status_set (m, HOSTILE);
	} else {
	    if (m->uniqueness == COMMON) {
		mprint ("and is utterly annihilated. Only a greasy spot remains...");
		m->corpsestr = "a greasy spot";
		m->id = 0;
		m->possessions.clear();
	    } else
		mprint ("and dies, cursing your name and the uncaring gods....");
	    m_death (m);
	}
    }
}

void drain (int blessing)
{
    int x = Player.x, y = Player.y;
    struct monster *m;
    setspot (&x, &y);
    mprint ("You begin to drain energy...");
    if (x == (int)Player.x && y == (int)Player.y) {
	mprint ("You drain your own energy....");
	mprint ("Uh, oh, positive feedback....");
	level_drain (Player.level, "self-vampirism");
    } else if ((m = Level->creature(x,y)) != NULL) {
	if ((blessing > -1) && (!m_immunityp (m, NEGENERGY))) {
	    mprint ("The monster seems weaker...");
	    m_damage (m, m->level * m->level, NEGENERGY);
	    m->hit = max (m->hit - m->level, 1);
	    m->dmg = max (m->dmg - m->level * m->level, 1);
	    m->ac = max (m->ac - m->level, 1);
	    m->level = max (1, m->level - 1);
	    mprint ("You feel stronger...");
	    gain_experience (m->level * 5);
	    Player.hp += (m->level * m->level / 2);
	} else {
	    mprint ("The effect reverses itself!");
	    mprint ("The monster seems stronger...");
	    m->hp += Player.level * Player.level;
	    m->hit += Player.level;
	    m->dmg += Player.level * Player.level;
	    m->ac += Player.level;
	    m->level++;
	    mprint ("You feel weaker...");
	    Player.mana = min (0, Player.level * Player.level);
	    level_drain (m->level, "negative energy conflict");
	}
    } else if (blessing < 0) {
	mprint ("You seem to lose energy, instead of gaining it!");
	level_drain (3, "reversed energy drain");
    } else if (Level->site(x,y).locchar == ALTAR) {
	mprint ("The altar collapses in on itself....");
	Level->site(x,y).locchar = ABYSS;
	Level->site(x,y).p_locf = L_ABYSS;
	lset (x, y, CHANGED);
	if (!Player.patron) {
	    mprint ("You drain some theurgic energy from the altar....");
	    gain_experience (40);
	    Player.hp += 20;
	    Player.pow += 2;
	}
	if (Level->site(x,y).aux == Player.patron) {
	    mprint ("Your deity is enraged.");
	    mprint ("You are struck by godsfire.");
	    p_damage (Player.hp - 1, UNSTOPPABLE, "godsfire");
	    mprint ("You feel atheistic.");
	    Player.patron = ATHEISM;
	    Player.rank[PRIESTHOOD] = NOT_A_BELIEVER;
	} else {
	    mprint ("You feel the wrath of a god....");
	    p_damage (random_range (Player.level * 10), UNSTOPPABLE, "divine wrath");
	    if (Player.patron != 0) {
		mprint ("Your deity doesn't seem to mind your action, though.");
		gain_experience (100);
	    }
	}
    } else {
	mprint ("You drain some energy from the ambient megaflow.");
	Player.hp++;
    }
}

void sanctuary (void)
{
    location& l = Level->site(Player.x,Player.y);
    if (Level->environment == E_TEMPLE || Player.patron <= NOT_A_RELIGION)
	mprint ("Odd, the spell has no effect. I wonder why.");
    else if (l.locchar != FLOOR)
	mprint ("There is no space for an altar here.");
    else {
	mprint ("You're standing on sacred ground!");
	l.locchar = ALTAR;
	l.aux = Player.patron;
	l.lstatus |= CHANGED| SEEN| LIT;
    }
}

void shadowform (void)
{
    // WDT HACK: this fix might work, but it seems like the immunity
    // will be FAR too short.  It's obviously better than the old 
    // situation, though...
    if (!Player.status[SHADOWFORM]) {
	mprint ("You feel like a shadow.");
	Player.immunity[NORMAL_DAMAGE]++;
	Player.immunity[ACID]++;
	Player.immunity[THEFT]++;
	Player.immunity[INFECTION]++;
	Player.status[SHADOWFORM] += Player.level;
    } else {
	mprint ("You feel even more shadowy.");
	Player.status[SHADOWFORM] += Player.level;
    }
}

void illuminate (int blessing)
{
    int r = Level->site(Player.x,Player.y).roomnumber;
    if (r < RS_ROOMBASE)
	mprint (blessing >= 0 ? "You see a faint glimmer of light which quickly fades." : "The gloom thickens for a moment.");
    else if (loc_statusp (Player.x, Player.y, LIT))
	mprint (blessing >= 0 ? "A glow surrounds you." : "Nothing much happens.");
    else {
	mprint (blessing >= 0 ? "The room lights up!" : "The room darkens!");
	if (blessing < 0)
	    spreadroomdark (Player.x, Player.y, Level->site(Player.x,Player.y).roomnumber);
	else {
	    Player.status[ILLUMINATION] += blessing + 3;
	    spreadroomlight (Player.x, Player.y, Level->site(Player.x,Player.y).roomnumber);
	}
    }
}

void drain_life (int amount)
{
    amount = absv (amount);
    mprint ("You feel cold!");
    if (Player.immune_to (NEGENERGY))
	mprint ("... but the feeling quickly fades.");
    else {
	if (random_range (2)) {
	    mprint ("The coldness spreads throughout your body...");
	    Player.str -= amount;
	    Player.con -= amount;
	    if ((Player.str < 3) || (Player.con < 3)) {
		mprint ("You suffer a fatal heart attack!!!");
		Player.hp = 0;
		strcpy (Str2, "a coronary");
		p_death (Str2);
	    }
	} else {
	    mprint ("The coldness saps your very soul...");
	    strcpy (Str2, "soul destruction");
	    level_drain (amount, Str2);
	}
    }
}

void inflict_fear (int x, int y)
{
    struct monster *m;
    if ((int)Player.x == x && (int)Player.y == y) {
	mprint ("You shudder with otherworldly dread.");
	if (Player.immunity[FEAR] > 0)
	    mprint ("You brace up and face your fear like a hero!");
	else {
	    mprint ("You panic!");
	    Player.status[AFRAID] += 10;
	}
    } else if ((m = Level->creature(x,y)) != NULL) {
	if (m->uniqueness == COMMON) {
	    strcpy (Str2, "The ");
	    strcat (Str2, m->monstring);
	} else
	    strcpy (Str2, m->monstring);
	m->speed = max (2, m->speed - 1);
	if (m_immunityp (m, FEAR))
	    strcat (Str2, "seems enraged!");
	else {
	    strcat (Str2, "is terrorized!");
	    m_dropstuff (m);
	    if (m_statusp (m, MOBILE))
		m->movef = M_MOVE_SCAREDY;
	}
    } else
	mprint ("A thrill of fear tickles your spine ... and passes.");
}

//Turns on deflection status for the player
void deflection (int blessing)
{
    if (blessing > -1) {
	mprint ("You feel buffered.");
	Player.status[DEFLECTION] = blessing + random_range (6);
    } else {
	mprint ("You feel vulnerable");
	Player.status[VULNERABLE] += random_range (6) - blessing;
    }
}
