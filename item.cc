#include "glob.h"

//----------------------------------------------------------------------

static const char* grotname(void);
static void i_accuracy(pob o);
static void i_acquire(pob o);
static void i_alert(pob o);
static void i_antioch(pob o);
static void i_apport(pob o);
static void i_augment(pob o);
static void i_azoth(pob o);
static void i_bless(pob o);
static void i_breathing(pob o);
static void i_charge(pob o);
static void i_clairvoyance(struct object *o);
static void i_corpse(pob o);
static void i_crystal(pob o);
static void i_cure(pob o);
static void i_death(pob o);
static void i_defend(pob o);
static void i_deflect(pob o);
static void i_demonblade(pob o);
static void i_desecrate(pob o);
static void i_disintegrate(pob o);
static void i_dispel(pob o);
static void i_displace(pob o);
static void i_disrupt(pob o);
static void i_enchant(pob o);
static void i_enchantment(pob o);
static void i_fear(pob o);
static void i_fear_resist(pob o);
static void i_fireball(pob o);
static void i_firebolt(pob o);
static void i_flux(pob o);
static void i_food(pob o);
static void i_heal(pob o);
static void i_helm(pob o);
static void i_hero(pob o);
static void i_hide(pob o);
static void i_id(pob o);
static void i_illuminate(pob o);
static void i_immune(pob o);
static void i_invisible(pob o);
static void i_jane_t(pob o);
static void i_juggernaut(pob o);
static void i_key(pob o);
static void i_knowledge(pob o);
static void i_kolwynia(pob o);
static void i_lball(pob o);
static void i_lbolt(pob o);
static void i_lembas(pob o);
static void i_levitate(pob o);
static void i_life(pob o);
static void i_lightsabre(pob o);
static void i_mace_disrupt(pob o);
static void i_missile(pob o);
static void i_mondet(pob o);
static void i_neutralize_poison(pob o);
static void i_no_op(pob o);
static void i_normal_armor(pob o);
static void i_normal_shield(pob o);
static void i_normal_weapon(pob o);
static void i_nothing(pob o);
static void i_objdet(pob o);
static void i_orbair(pob o);
static void i_orbdead(pob o);
static void i_orbearth(pob o);
static void i_orbfire(pob o);
static void i_orbmastery(pob o);
static void i_orbwater(pob o);
static void i_pepper_food(pob o);
static void i_perm_accuracy(pob o);
static void i_perm_agility(pob o);
static void i_perm_breathing(pob o);
static void i_perm_burden(pob o);
static void i_perm_deflect(pob o);
static void i_perm_displace(pob o);
static void i_perm_energy_resist(pob o);
static void i_perm_fear_resist(pob o);
static void i_perm_fire_resist(pob o);
static void i_perm_gaze_immune(pob o);
static void i_perm_hero(pob o);
static void i_perm_illuminate(pob o);
static void i_perm_invisible(pob o);
static void i_perm_knowledge(pob o);
static void i_perm_levitate(pob o);
static void i_perm_negimmune(pob o);
static void i_perm_poison_resist(pob o);
static void i_perm_protection(pob o);
static void i_perm_regenerate(pob o);
static void i_perm_speed(pob o);
static void i_perm_strength(pob o);
static void i_perm_truesight(pob o);
static void i_pick(pob o);
static void i_planes(pob o);
static void i_poison_food(pob o);
static void i_polymorph(pob o);
static void i_pow(pob o);
static void i_raise_portcullis(pob o);
static void i_regenerate(pob o);
static void i_restore(pob o);
static void i_sceptre(pob o);
static void i_sleep_other(pob o);
static void i_sleep_self(pob o);
static void i_snowball(pob o);
static void i_speed(pob o);
static void i_spells(pob o);
static void i_stargem(pob o);
static void i_stim(pob o);
static void i_summon(pob o);
static void i_symbol(pob o);
static void i_teleport(pob o);
static void i_trap(pob o);
static void i_truesight(pob o);
static void i_victrix(pob o);
static void i_warp(pob o);
static void i_wish(pob o);
static int itemblessing(void);
static int itemcharge(void);
static int itemplus(void);
static int orbcheck(int element);

//----------------------------------------------------------------------

object create_object (int itemlevel)
{
    object o;
    do {
	int r = random_range (135);
	if (r < 20)		o = make_thing();
	else if (r < 40)	o = make_food();
	else if (r < 50)	o = make_scroll();
	else if (r < 60)	o = make_potion();
	else if (r < 70)	o = make_weapon();
	else if (r < 80)	o = make_armor();
	else if (r < 90)	o = make_shield();
	else if (r < 100)	o = make_stick();
	else if (r < 110)	o = make_boots();
	else if (r < 120)	o = make_cloak();
	else if (r < 130)	o = make_ring();
	else			o = make_artifact();
	// not ok if object is too good for level, or if unique and already made
	// 1/100 chance of finding object if too good for level
    } while (object_uniqueness(o) >= UNIQUE_MADE && (o.level >= itemlevel + random_range(3) || random_range(100)));
    if (object_uniqueness(o) == UNIQUE_UNMADE)
	set_object_uniqueness (o, UNIQUE_MADE);
    return (o);
}

object make_cash (int level)
{
    object o = Objects[CASHID];
    o.basevalue = random_range (level * level + 10) + 1;	// aux is AU value
    return (o);
}

object make_food (int id)
{
    return (Objects[FOODID + (id == RANDOM ? random_range(NUMFOODS) : id)]);
}

object make_corpse (const monster& m)
{
    object o = Objects[CORPSEID];
    o.charge = m.id;
    o.weight = m.corpseweight;
    o.basevalue = m.corpsevalue;
    learn_object (o.id);
    o.objstr = m.corpsestr;
    o.truename = o.cursestr = o.objstr;
    if (m_statusp (m, EDIBLE)) {
	o.usef = I_FOOD;
	o.aux = 6;
    } else if (m_statusp (m, POISONOUS))
	o.usef = I_POISON_FOOD;
    // Special corpse-eating effects
    else {
	switch (m.id) {
	    case TSETSE:
	    case TORPOR:	o.usef = I_SLEEP_SELF; break;
	    case NASTY:		o.usef = I_INVISIBLE; break;
	    case BLIPPER:	o.usef = I_TELEPORT; break;
	    case EYE:		o.usef = I_CLAIRVOYANCE; break;
	    case FUZZY:		o.usef = I_DISPLACE; break;
	    case SERV_LAW:	o.usef = I_CHAOS; break;
	    case SERV_CHAOS:	o.usef = I_LAW; break;
	    case ASTRAL_VAMP:	o.usef = I_ENCHANT; break;
	    case MANABURST:	o.usef = I_SPELLS; break;
	    case RAKSHASA:	o.usef = I_TRUESIGHT; break;
	    case COMA:		o.usef = I_ALERT; break;
	}
    }
    return (o);
}

object make_ring (int id)
{
    object o = Objects[RINGID + (id == RANDOM ? random_range(NUMRINGS) : id)];
    if (o.blessing == 0)
	o.blessing = itemblessing();
    if (o.plus == 0)
	o.plus = itemplus() + 1;
    if (o.blessing < 0)
	o.plus = RANDOM - absv (o.plus);
    return (o);
}

object make_thing (int id)
{
    object o = Objects[THINGID + (id == RANDOM ? random_range(NUMTHINGS) : id)];
    if (strcmp (o.objstr, "grot") == 0)
	o.truename = o.cursestr = o.objstr = grotname();
    return (o);
}

object make_scroll (int id)
{
    object o = Objects[SCROLLID + (id == RANDOM ? random_range(NUMSCROLLS) : id)];
    if (o.id == SCROLL_SPELLS) // if a scroll of spells, aux is the spell id in Spells
	o.aux = random_range (NUMSPELLS);
    return (o);
}

object make_potion (int id)
{
    object o = Objects[POTIONID + (id == RANDOM ? random_range(NUMPOTIONS) : id)];
    if (o.plus == 0)
	o.plus = itemplus();
    return (o);
}

object make_weapon (int id)
{
    object o = Objects[WEAPONID + (id == RANDOM ? random_range(NUMWEAPONS) : id)];
    if (id == WEAPON_BOLT-WEAPONID || id == WEAPON_ARROW-WEAPONID)
	o.number = random_range(20) + 1;
    if (o.blessing == 0)
	o.blessing = itemblessing();
    if (o.plus == 0) {
	o.plus = itemplus();
	if (o.blessing < 0)
	    o.plus = RANDOM - absv (o.plus);
	else if (o.blessing > 0)
	    o.plus = 1 + absv (o.plus);
    }
    return (o);
}

object make_shield (int id)
{
    object o = Objects[SHIELDID + (id == RANDOM ? random_range(NUMSHIELDS) : id)];
    if (o.plus == 0)
	o.plus = itemplus();
    if (o.blessing == 0)
	o.blessing = itemblessing();
    if (o.blessing < 0)
	o.plus = RANDOM - absv (o.plus);
    else if (o.blessing > 0)
	o.plus = 1 + absv (o.plus);
    return (o);
}

object make_armor (int id)
{
    object o = Objects[ARMORID + (id == RANDOM ? random_range(NUMARMOR) : id)];
    if (o.plus == 0)
	o.plus = itemplus();
    if (o.blessing == 0)
	o.blessing = itemblessing();
    if (o.blessing < 0)
	o.plus = RANDOM - absv (o.plus);
    else if (o.blessing > 0)
	o.plus = 1 + absv (o.plus);
    return (o);
}

object make_cloak (int id)
{
    object o = Objects[CLOAKID + (id == RANDOM ? random_range(NUMCLOAKS) : id)];
    if (o.blessing == 0)
	o.blessing = itemblessing();
    return (o);
}

object make_boots (int id)
{
    object o = Objects[BOOTID + (id == RANDOM ? random_range(NUMBOOTS) : id)];
    if (o.blessing == 0)
	o.blessing = itemblessing();
    return (o);
}

object make_stick (int id)
{
    object o = Objects[STICKID + (id == RANDOM ? random_range(NUMSTICKS) : id)];
    o.charge = itemcharge();
    if (o.blessing == 0)
	o.blessing = itemblessing();
    return (o);
}

object make_artifact (int id)
{
    if (id == RANDOM)
	do { id = random_range (NUMARTIFACTS); } while (object_uniqueness(id) >= UNIQUE_MADE);
    return (Objects[ARTIFACTID + id]);
}

static const char* grotname (void)
{
    static const char _grots[] =
	"pot lid\0"
	"mound of offal\0"
	"sword that was broken\0"
	"salted snail\0"
	"key\0"
	"toadstool\0"
	"greenish spindle\0"
	"tin soldier\0"
	"broken yo-yo\0"
	"NYC subway map\0"
	"Nixon's the One! button\0"
	"beer can (empty)\0"
	"golden bejewelled falcon\0"
	"hamster cage\0"
	"wooden nickel\0"
	"three-dollar bill\0"
	"rosebud\0"
	"water pistol\0"
	"shattered skull\0"
	"jawbone of an ass";
    return (zstrn (_grots, random_range(20), 20));
}

static int itemplus (void)
{
    int p = 0;
    while (random_range (2) == 0)
	p++;
    return (p);
}

static int itemcharge (void)
{
    return (random_range (20) + 1);
}

static int itemblessing (void)
{
    switch (random_range (10)) {
	case 0:
	case 1: return (-1 - random_range (10));
	case 8:
	case 9: return (1 + random_range (10));
	default: return (0);
    }
}

bool twohandedp (int id)
{
    return (id == WEAPON_GREAT_SWORD || id == WEAPON_GREAT_AXE ||
	    id == WEAPON_QUARTERSTAFF || id == WEAPON_HALBERD ||
	    id == WEAPON_LONGBOW || id == WEAPON_CROSSBOW ||
	    id == WEAPON_DESECRATOR || id == WEAPON_GOBLIN_HEWER ||
	    id == WEAPON_GIANT_CLUB || id == WEAPON_SCYTHE_OF_DEATH);
}

// general item functions
static void i_no_op (pob o UNUSED)
{
}

static void i_nothing (pob o UNUSED)
{
}

//  scroll functions
static void i_knowledge (pob o)
{
    if (o->blessing > -1)
	learn_object (o);
    knowledge (o->blessing);
}

static void i_jane_t (pob o)
{
    int volume = random_range (6);
    int j = 0, k = 0;
    char v;

    learn_object (o);
    print1 ("Jane's Guide to the World's Treasures: ");
    switch (volume) {
	case 0:
	    nprint1 ("SCROLLS");
	    j = SCROLLID;
	    k = POTIONID;
	    break;
	case 1:
	    nprint1 ("POTIONS");
	    j = POTIONID;
	    k = WEAPONID;
	    break;
	case 2:
	    nprint1 ("CLOAKS");
	    j = CLOAKID;
	    k = BOOTID;
	    break;
	case 3:
	    nprint1 ("BOOTS");
	    j = BOOTID;
	    k = RINGID;
	    break;
	case 4:
	    nprint1 ("RINGS");
	    j = RINGID;
	    k = STICKID;
	    break;
	case 5:
	    nprint1 ("STICKS");
	    j = STICKID;
	    k = ARTIFACTID;
	    break;
    }
    menuclear();
    menuprint ("You could probably now recognise:\n");
    for (int i = j; i < k; i++) {
	learn_object (i);
	v = Objects[i].truename[0];
	if ((v >= 'A' && v <= 'Z') || volume == 3)
	    sprintf (Str1, "   %s\n", Objects[i].truename);
	else if (v == 'a' || v == 'e' || v == 'i' || v == 'o' || v == 'u')
	    sprintf (Str1, "   an %s\n", Objects[i].truename);
	else
	    sprintf (Str1, "   a %s\n", Objects[i].truename);
	menuprint (Str1);
    }
    showmenu();
    morewait();
    xredraw();
}

static void i_flux (pob o)
{
    if (o->blessing > -1)
	learn_object (o);
    flux (o->blessing);
}

// enchantment
static void i_enchant (pob o)
{
    if (o->blessing > -1)
	learn_object (o);
    enchant (o->blessing < 0 ? -1 - o->plus : o->plus + 1);
}

// scroll of clairvoyance
static void i_clairvoyance (struct object *o)
{
    if (o->blessing > -1)
	learn_object (o);
    if (o->blessing < 0)
	amnesia();
    else
	clairvoyance (5 + o->blessing * 5);
}

static void i_acquire (pob o)
{
    if (o->blessing > -1)
	learn_object (o);
    int blessing = o->blessing;
    *o = Objects[SCROLL_BLANK];
    acquire (blessing);
}

static void i_teleport (pob o)
{
    if (o->blessing > -1)
	learn_object (o);
    p_teleport (o->blessing);
}

static void i_spells (pob o)
{
    if (o->blessing > -1)
	learn_object (o);
    mprint ("A scroll of spells.");
    morewait();
    learnspell (o->blessing);
}

// scroll of blessing
static void i_bless (pob o)
{
    learn_object (o);
    bless (o->blessing);
}

// scroll of wishing
static void i_wish (pob o)
{
    if (o->blessing > -1)
	learn_object (o);
    wish (o->blessing);
    *o = Objects[SCROLL_BLANK];
}

// scroll of displacement
static void i_displace (pob o)
{
    if (o->blessing > -1)
	learn_object (o);
    displace (o->blessing);
}

// scroll of deflection
static void i_deflect (pob o)
{
    if (o->blessing > -1)
	learn_object (o);
    deflection (o->blessing);
}

// scroll of identification
static void i_id (pob o)
{
    if (o->blessing > -1)
	learn_object (o);
    identify (o->blessing);
}

// potion functions

// potion of healing
static void i_heal (pob o)
{
    if (o->blessing > -1) {
	learn_object (o);
	heal (1 + o->plus);
    } else
	heal (-1 - absv (o->plus));
}

// potion of monster detection
static void i_mondet (pob o)
{
    if (o->blessing > -1)
	learn_object (o);
    mondet (o->blessing);
}

// potion of object detection
static void i_objdet (pob o)
{

    if (o->blessing > -1)
	learn_object (o);
    objdet (o->blessing);
}

// potion of neutralize poison
static void i_neutralize_poison (pob o)
{
    if (o->blessing > -1) {
	learn_object (o);
	mprint ("You feel vital!");
	Player.status[POISONED] = 0;
    } else
	p_poison (random_range (20) + 5);
}

// potion of sleep
static void i_sleep_self (pob o)
{
    sleep_player (6);
    learn_object (o);
}

// potion of speed
static void i_speed (pob o)
{
    if (o->blessing > -1)
	learn_object (o);
    haste (o->blessing);
}

// potion of restoration
static void i_restore (pob o)
{
    if (o->blessing > -1)
	learn_object (o);
    recover_stat (o->blessing);
}

static void i_augment (pob o)
{
    if (o->blessing > -1)
	learn_object (o);
    augment (o->blessing);
}

static void i_azoth (pob o)
{
    if (o->plus < 0) {
	mprint ("The mercury was poisonous!");
	p_poison (25);
    } else if (o->plus == 0) {
	mprint ("The partially enchanted azoth makes you sick!");
	Player.con = ((int) (Player.con / 2));
	calc_melee();
    } else if (o->blessing < 1) {
	mprint ("The unblessed azoth warps your soul!");
	Player.pow = Player.maxpow = ((int) (Player.maxpow / 2));
	level_drain (random_range (10), "cursed azoth");
    } else {
	mprint ("The azoth fills you with cosmic power!");
	if (Player.str > Player.maxstr * 2) {
	    mprint ("The power rages out of control!");
	    p_death ("overdose of azoth");
	} else {
	    heal (10);
	    cleanse (1);
	    Player.mana = calcmana() * 3;
	    toggle_item_use (TRUE);
	    Player.str = (Player.maxstr++) * 3;
	    toggle_item_use (FALSE);
	}
    }
}

static void i_regenerate (pob o)
{
    regenerate (o->blessing);
}

// boots functions
static void i_perm_speed (pob o)
{
    if (o->blessing > -1) {
	if (o->used) {
	    learn_object (o);
	    if (Player.status[SLOWED] > 0)
		Player.status[SLOWED] = 0;
	    mprint ("The world slows down!");
	    Player.status[HASTED] += 1500;
	} else {
	    Player.status[HASTED] -= 1500;
	    if (Player.status[HASTED] < 1)
		mprint ("The world speeds up again.");
	}
    } else {
	if (o->used) {
	    if (Player.status[HASTED] > 0) {
		Player.status[HASTED] = 0;
	    }
	    mprint ("You feel slower.");
	    Player.status[SLOWED] += 1500;
	} else {
	    Player.status[SLOWED] -= 1500;
	    if (Player.status[SLOWED] < 1)
		mprint ("You feel quicker again.");
	}
    }
}

// cloak functions
static void i_perm_displace (pob o)
{
    if (o->blessing > -1) {
	if (o->used) {
	    mprint ("You feel dislocated.");
	    Player.status[DISPLACED] += 1500;
	} else {
	    Player.status[DISPLACED] -= 1500;
	    if (Player.status[DISPLACED] < 1) {
		mprint ("You feel a sense of position.");
		Player.status[DISPLACED] = 0;
	    }
	}
    } else {
	if (o->used) {
	    mprint ("You have a forboding of bodily harm!");
	    Player.status[VULNERABLE] += 1500;
	} else {
	    Player.status[VULNERABLE] -= 1500;
	    if (Player.status[VULNERABLE] < 1) {
		mprint ("You feel less endangered.");
		Player.status[VULNERABLE] = 0;
	    }
	}
    }
}

static void i_perm_negimmune (pob o)
{
    if (o->blessing > -1) {
	if (o->used) {
	    Player.immunity[NEGENERGY]++;
	} else
	    Player.immunity[NEGENERGY]--;
    } else if (o->used)
	level_drain (absv (o->blessing), "cursed cloak of level drain");
}

// food functions

static void i_food (pob o UNUSED)
{
    switch (random_range (5)) {
	case 0:
	    mprint ("That tasted horrible!");
	    break;
	case 1:
	    mprint ("Yum!");
	    break;
	case 2:
	    mprint ("How nauseous!");
	    break;
	case 3:
	    mprint ("Can I have some more? Please?");
	    break;
	case 4:
	    mprint ("Your mouth feels like it is growing hair!");
	    break;
    }
}

static void i_stim (pob o)
{
    mprint ("You feel Hyper!");
    i_speed (o);
    Player.str += 3;
    Player.con -= 1;
    calc_melee();
}

static void i_pow (pob o UNUSED)
{
    mprint ("You feel a surge of mystic power!");
    Player.mana = 2 * calcmana();
}

static void i_poison_food (pob o UNUSED)
{
    mprint ("This food was contaminated with cyanide!");
    p_poison (random_range (20) + 5);
}

static void i_pepper_food (pob o UNUSED)
{
    mprint ("You innocently start to chew the szechuan pepper.....");
    morewait();
    mprint ("hot.");
    morewait();
    mprint ("Hot.");
    morewait();
    mprint ("Hot!");
    morewait();
    mprint ("HOT!!!!!!");
    morewait();
    p_damage (1, UNSTOPPABLE, "a szechuan pepper");
    mprint ("Your sinuses melt and run out your ears.");
    mprint ("Your mouth and throat seem to be permanently on fire.");
    mprint ("You feel much more awake now....");
    Player.immunity[SLEEP]++;
}

static void i_lembas (pob o UNUSED)
{
    heal (10);
    cleanse (0);
    Player.food = 40;
}

static void i_cure (pob o)
{
    cure (o->blessing);
}

static void i_immune (pob o)
{
    if (o->blessing > 0) {
	mprint ("You feel a sense of innoculation");
	Player.immunity[INFECTION]++;
	cure (o->blessing);
    }
}

static void i_breathing (pob o)
{

    if (o->blessing > -1)
	learn_object (o);
    breathe (o->blessing);
}

static void i_invisible (pob o)
{
    if (o->blessing > -1)
	learn_object (o);
    invisible (o->blessing);
}

static void i_perm_invisible (pob o)
{
    if (o->blessing > -1)
	learn_object (o);
    if (o->used) {
	if (o->blessing > -1) {
	    mprint ("You feel transparent!");
	    Player.status[INVISIBLE] += 1500;
	} else {
	    mprint ("You feel a forboding of bodily harm!");
	    Player.status[VULNERABLE] += 1500;
	}
    } else {
	if (o->blessing > -1) {
	    Player.status[INVISIBLE] -= 1500;
	    if (Player.status[INVISIBLE] < 1) {
		mprint ("You feel opaque again.");
		Player.status[INVISIBLE] = 0;
	    }
	} else {
	    Player.status[VULNERABLE] -= 1500;
	    if (Player.status[VULNERABLE] < 1) {
		mprint ("You feel less endangered now.");
		Player.status[VULNERABLE] = 0;
	    }
	}
    }
}

static void i_warp (pob o)
{
    if (o->blessing > -1)
	learn_object (o);
    warp (o->blessing);
}

static void i_alert (pob o)
{
    if (o->blessing > -1) {
	learn_object (o);
	alert (o->blessing);
    }
}

static void i_charge (pob o)
{
    int i;
    if (o->blessing > -1)
	learn_object (o);
    mprint ("A scroll of charging.");
    mprint ("Charge: ");
    i = getitem (STICK);
    if (i != ABORT) {
	if (o->blessing < 0) {
	    mprint ("The stick glows black!");
	    Player.possessions[i].charge = 0;
	} else {
	    mprint ("The stick glows blue!");
	    Player.possessions[i].charge += (random_range (10) + 1) * (o->blessing + 1);
	    if (Player.possessions[i].charge > 99)
		Player.possessions[i].charge = 99;
	}
    }
}

static void i_fear_resist (pob o)
{
    if (o->blessing > -1) {
	learn_object (o);
	if (Player.status[AFRAID] > 0) {
	    mprint ("You feel stauncher now.");
	    Player.status[AFRAID] = 0;
	}
    } else if (!p_immune (FEAR)) {
	mprint ("You panic!");
	Player.status[AFRAID] += random_range (100);
    }
}

// use a thieves pick
static void i_pick (pob o)
{
    int dir;
    int ox, oy;
    o->used = FALSE;
    if (!object_is_known(o) && Player.rank[THIEVES] == NOT_A_THIEF) {
	mprint ("You have no idea what do with a piece of twisted metal.");
	return;
    }
    learn_object (o);
    mprint ("Pick lock:");
    dir = getdir();
    if (dir == ABORT) {
	resetgamestatus (SKIP_MONSTERS);
	return;
    }
    ox = Player.x + Dirs[0][dir];
    oy = Player.y + Dirs[1][dir];
    if ((Level->site(ox,oy).locchar != CLOSED_DOOR) || loc_statusp (ox, oy, SECRET)) {
	mprint ("You can't unlock that!");
	resetgamestatus (SKIP_MONSTERS);
    } else if (Level->site(ox,oy).aux == LOCKED) {
	if (Level->depth == MaxDungeonLevels - 1)
	    mprint ("The lock is too complicated for you!!!");
	else if (Level->depth * 2 + random_range (50) < Player.dex + Player.level + Player.rank[THIEVES] * 10) {
	    mprint ("You picked the lock!");
	    Level->site(ox,oy).aux = UNLOCKED;
	    lset (ox, oy, CHANGED);
	    gain_experience (max (3, Level->depth));
	} else
	    mprint ("You failed to pick the lock.");
    } else
	mprint ("That door is already unlocked!");
}

// use a magic key
static void i_key (pob o)
{
    int dir;
    int ox, oy;
    o->used = FALSE;
    mprint ("Unlock door: ");
    dir = getdir();
    if (dir == ABORT)
	resetgamestatus (SKIP_MONSTERS);
    else {
	ox = Player.x + Dirs[0][dir];
	oy = Player.y + Dirs[1][dir];
	if ((Level->site(ox,oy).locchar != CLOSED_DOOR) || loc_statusp (ox, oy, SECRET)) {
	    mprint ("You can't unlock that!");
	    resetgamestatus (SKIP_MONSTERS);
	} else if (Level->site(ox,oy).aux == LOCKED) {
	    mprint ("The lock clicks open!");
	    Level->site(ox,oy).aux = UNLOCKED;
	    lset (ox, oy, CHANGED);
	    o->blessing--;
	    if ((o->blessing < 0) || (Level->depth == MaxDungeonLevels - 1)) {
		mprint ("The key disintegrates!");
		Player.remove_possession (o, 1);
	    } else
		mprint ("Your key glows faintly.");
	} else
	    mprint ("That door is already unlocked!");
    }
}

static void i_corpse (pob o)
{
    // WDT HACK: there are some comments in this function which need
    // to be backed up with assert().
    // object's charge holds the former monster id
    switch (o->charge) {
	case MEND_PRIEST:
	case ITIN_MERCH:
	case GUARD:
	case NPC:
	case MERCHANT:
	case ZERO_NPC:
	case HISCORE_NPC:
	case APPR_NINJA:
	case SNEAK_THIEF:
	case BRIGAND:
	case GENIN:
	case MAST_THIEF:
	case CHUNIN:
	case JONIN:		// cannibalism
	    mprint ("Yechh! How could you! You didn't even cook him, first!");
	    if (Player.alignment > 0)
		Player.food = 25;
	    Player.food += 8;
	    Player.alignment -= 10;
	    foodcheck();
	    break;
	case FNORD:		// fnord
	    mprint ("You feel illuminated!");
	    Player.iq++;
	    break;
	case DENEBIAN:		// denebian slime devil
	    mprint ("I don't believe this. You ate Denebian Slime?");
	    mprint ("You deserve a horrible wasting death, uncurable by any means!");
	    break;
	case DRAGONETTE:	// can't get here... i_usef changed to I_FOOD
	    mprint ("Those dragon-steaks were fantastic!");
	    Player.food = 24;
	    foodcheck();
	    break;
	case BEHEMOTH:
	    mprint ("You feel infinitely more virile now.");
	    Player.str = max (Player.str, Player.maxstr + 10);
	    Player.food = 24;
	    foodcheck();
	    break;
	case INVIS_SLAY:
	    mprint ("Guess what? You're invisible.");
	    if (Player.status[INVISIBLE] < 1000)
		Player.status[INVISIBLE] = 666;
	    Player.food += 6;
	    foodcheck();
	    break;
	case UNICORN:
	    mprint ("You ATE a unicorn's horn?!?!?");
	    Player.immunity[POISON] = 1000;
	    break;
	case HORNET:		// can't get here... i_usef changed to I_POISON_FOOD
	case GRUNT:
	case TSETSE:		// can't get here... i_usef changed to I_SLEEP_SELF
	case AGGRAVATOR:
	case BLIPPER:		// can't get here... i_usef changed to I_TELEPORT
	case GOBLIN:
	case GEEK:
	case NIGHT_GAUNT:	// can't get here... i_usef changed to I_POISON_FOOD
	case TOVE:
	case TASMANIAN:
	case JUBJUB:		// can't get here... i_usef changed to I_FOOD
	case CATEAGLE:
	    mprint ("Well, you forced it down. Not much nutrition, though.");
	    Player.food++;
	    foodcheck();
	    break;
	case SEWER_RAT:
	case PHANTASTICON:	// can't get here... i_usef changed to I_POISON_FOOD
	case EYE:		// can't get here... i_usef changed to I_CLAIRVOYANCE
	case NASTY:		// can't get here... i_usef changed to I_INVISIBLE
	case CATOBLEPAS:
	case HAUNT:		// can't get here... i_usef changed to I_POISON_FOOD
	case ROUS:
	case DEATHSTAR:	// can't get here... i_usef changed to I_POISON_FOOD
	case EATER:
	    mprint ("Oh, yuck. The 'food' seems to be tainted.");
	    mprint ("You feel very sick. You throw up.");
	    Player.food = min (Player.food, 4);
	default:
	    mprint ("It proved completely inedible, but you tried anyhow.");
    }
}

static void i_accuracy (pob o)
{
    learn_object (o);
    accuracy (o->blessing);
}

static void i_perm_accuracy (pob o)
{
    learn_object (o);
    if ((o->used) && (o->blessing > -1)) {
	Player.status[ACCURATE] += 1500;
	mprint ("You feel skillful and see bulls' eyes everywhere.");
    } else {
	Player.status[ACCURATE] -= 1500;
	if (Player.status[ACCURATE] < 1) {
	    Player.status[ACCURATE] = 0;
	    calc_melee();
	    mprint ("Your vision blurs....");
	}
    }
}

static void i_hero (pob o)
{
    learn_object (o);
    hero (o->blessing);
}

static void i_perm_hero (pob o)
{
    learn_object (o);
    if (o->used) {
	if (o->blessing > -1) {
	    Player.status[HERO] += 1500;
	    calc_melee();
	    mprint ("You feel super!");
	} else {
	    Player.status[HERO] = 0;
	    calc_melee();
	    if (!Player.immunity[FEAR]) {
		Player.status[AFRAID] += 1500;
		mprint ("You feel cowardly....");
	    }
	}
    } else {
	if (o->blessing > -1) {
	    Player.status[HERO] -= 1500;
	    if (Player.status[HERO] < 1) {
		calc_melee();
		mprint ("You feel less super now.");
		Player.status[HERO] = 0;
	    }
	} else {
	    Player.status[AFRAID] -= 1500;
	    if (Player.status[AFRAID] < 1) {
		mprint ("You finally conquer your fear.");
		Player.status[AFRAID] = 0;
	    }
	}
    }
}

static void i_levitate (pob o)
{
    learn_object (o);
    levitate (o->blessing);
}

static void i_perm_levitate (pob o)
{
    learn_object (o);
    if (o->blessing > -1) {
	if (o->used) {
	    Player.status[LEVITATING] += 1400;
	    mprint ("You start to float a few inches above the floor");
	    mprint ("You find you can easily control your altitude");
	} else {
	    Player.status[LEVITATING] -= 1500;
	    if (Player.status[LEVITATING] < 1) {
		Player.status[LEVITATING] = 0;
		mprint ("You sink to the floor.");
	    }
	}
    } else
	i_perm_burden (o);
}

static void i_perm_protection (pob o)
{
    if (o->used) {
	if (o->blessing > -1)
	    Player.status[PROTECTION] += absv (o->plus) + 1;
	else
	    Player.status[PROTECTION] -= absv (o->plus) + 1;
    } else {
	if (o->blessing > -1)
	    Player.status[PROTECTION] -= absv (o->plus) + 1;
	else
	    Player.status[PROTECTION] += absv (o->plus) + 1;
    }
    calc_melee();
}

static void i_perm_agility (pob o)
{
    learn_object (o);
    if (o->used) {
	if (o->blessing > -1)
	    Player.agi += absv (o->plus) + 1;
	else
	    Player.agi -= absv (o->plus) + 1;
    } else {
	if (o->blessing > -1)
	    Player.agi -= absv (o->plus) + 1;
	else
	    Player.agi += absv (o->plus) + 1;
    }
    calc_melee();
}

static void i_truesight (pob o)
{
    learn_object (o);
    truesight (o->blessing);
}

static void i_perm_truesight (pob o)
{
    learn_object (o);
    if (o->used) {
	if (o->blessing > -1) {
	    Player.status[TRUESIGHT] += 1500;
	    mprint ("You feel sharp!");
	} else {
	    Player.status[BLINDED] += 1500;
	    mprint ("You've been blinded!");
	}
    } else {
	if (o->blessing > -1) {
	    Player.status[TRUESIGHT] -= 1500;
	    if (Player.status[TRUESIGHT] < 1) {
		mprint ("You feel less keen now.");
		Player.status[TRUESIGHT] = 0;
	    }
	} else {
	    Player.status[BLINDED] -= 1500;
	    if (Player.status[BLINDED] < 1) {
		mprint ("You can see again!");
		Player.status[BLINDED] = 0;
	    }
	}
    }
}

static void i_illuminate (pob o)
{
    learn_object (o);
    illuminate (o->blessing);
}

static void i_perm_illuminate (pob o)
{
    learn_object (o);
    if (o->used)
	Player.status[ILLUMINATION] += 1500;
    else
	Player.status[ILLUMINATION] = max (0, Player.status[ILLUMINATION] - 1500);
}

static void i_trap (pob o)
{
    learn_object (o);

    if ((Level->site(Player.x,Player.y).locchar != FLOOR) || (Level->site(Player.x,Player.y).p_locf != L_NO_OP))
	mprint ("Your attempt fails.");
    else if (!object_is_known(o)) {
	mprint ("Fiddling with the thing, you have a small accident....");
	p_movefunction (o->aux);
    } else {
	mprint ("You successfully set a trap at your location.");
	Level->site(Player.x,Player.y).p_locf = o->aux;
	lset (Player.x, Player.y, CHANGED);
    }
    Player.remove_possession (o, 1);
}

static void i_raise_portcullis (pob o)
{
    l_raise_portcullis();
    mprint ("The box beeps once and explodes in your hands!");
    Player.remove_possession (o, 1);
}

// ring functions
static void i_perm_knowledge (pob o)
{
    learn_object (o);
    if (o->used)
	knowledge (o->blessing);
}

static void i_perm_strength (pob o)
{
    learn_object (o);
    if (o->used) {
	if (o->blessing > -1)
	    Player.str += absv (o->plus) + 1;
	else
	    Player.str -= absv (o->plus) + 1;
    } else {
	if (o->blessing > -1)
	    Player.str -= absv (o->plus) + 1;
	else
	    Player.str += absv (o->plus) + 1;
    }
    calc_melee();
}

static void i_perm_burden (pob o)
{
    int i;

    if (o->used) {
	o->weight = 1000;
	mprint ("You feel heavier.");
    } else {
	o->weight = 1;
	mprint ("Phew. What a relief.");
    }
    Player.itemweight = 0;
    for (i = 0; i < MAXITEMS; i++) {
	if (Player.has_possession(i))
	    Player.itemweight += (Player.possessions[i].weight * Player.possessions[i].number);
    }
}

static void i_perm_gaze_immune (pob o)
{
    if (o->used)
	Player.immunity[GAZE]++;
    else
	Player.immunity[GAZE]--;
}

static void i_perm_fire_resist (pob o)
{
    if (o->used)
	Player.immunity[FLAME]++;
    else
	Player.immunity[FLAME]--;
}

static void i_perm_poison_resist (pob o)
{
    if (o->used) {
	if (o->blessing < 0) {
	    Player.immunity[POISON] = 0;
	    p_poison (100);
	} else {
	    Player.immunity[POISON]++;
	    if (Player.status[POISONED] > 0) {
		mprint ("You feel much better now.");
		Player.status[POISONED] = 0;
	    }
	}
    } else {
	Player.status[POISONED] = 0;
	Player.immunity[POISON]--;
    }
}

static void i_perm_regenerate (pob o)
{
    learn_object (o);
    if (o->used) {
	mprint ("You seem abnormally healthy.");
	Player.status[REGENERATING] += 1500;
    } else {
	Player.status[REGENERATING] -= 1500;
	if (Player.status[REGENERATING] < 1) {
	    mprint ("Your vitality is back to normal");
	    Player.status[REGENERATING] = 0;
	}
    }
}

// armor functions

static void i_normal_armor (pob o)
{
    if (o->used)
	mprint ("You put on your suit of armor.");
}

static void i_perm_energy_resist (pob o)
{
    if (o->used) {
	Player.immunity[FLAME]++;
	Player.immunity[COLD]++;
	Player.immunity[ELECTRICITY]++;
    } else {
	Player.immunity[FLAME]--;
	Player.immunity[COLD]--;
	Player.immunity[ELECTRICITY]--;
    }
}

static void i_perm_fear_resist (pob o)
{
    if (o->used) {
	Player.immunity[FEAR]++;
	if (o->blessing < 0) {
	    Player.status[BERSERK] += 1500;
	    mprint ("You feel blood-simple!");
	}
    } else {
	Player.immunity[FEAR]--;
	if (o->blessing < 0) {
	    Player.status[BERSERK] -= 1500;
	    if (Player.status[BERSERK] < 1) {
		mprint ("You feel less rabid now.");
		Player.status[BERSERK] = 0;
	    }
	}
    }
}

static void i_perm_breathing (pob o)
{
    learn_object (o);
    if (o->blessing > -1) {
	if (o->used) {
	    mprint ("Your breath is energized!");
	    Player.status[BREATHING] += 1500;
	} else {
	    Player.status[BREATHING] -= 1500;
	    if (Player.status[BREATHING] < 1) {
		mprint ("Your breathing is back to normal.");
		Player.status[BREATHING] = 0;
	    }
	}
    } else if (o->used) {
	Player.status[BREATHING] = 0;
	p_drown();
	print1 ("Water pours from the broken suit.");
    }
}

// weapons functions

void weapon_acidwhip (int dmgmod, pob o UNUSED, struct monster *m)
{
    if ((random_range (2) == 1) && (!m_immunityp (m, NORMAL_DAMAGE))) {
	mprint ("You entangle the monster!");
	m_status_reset (m, MOBILE);
    }
    p_hit (m, Player.dmg + dmgmod, ACID);
}

void weapon_scythe (int dmgmod UNUSED, pob o UNUSED, struct monster *m)
{
    mprint ("Slice!");
    m_death (m);
    if (!Player.rank[ADEPT]) {
	mprint ("Ooops!");
	mprint ("You accidentally touch yourself on the backswing....");
	p_death ("the Scythe of Death");
    }
}

void weapon_demonblade (int dmgmod, pob o, struct monster *m)
{
    if (o->blessing > -1) {
	mprint ("Demonblade disintegrates with a soft sigh.");
	mprint ("You stop foaming at the mouth.");
	Player.status[BERSERK] = 0;
	Player.remove_possession (o, 1);
    } else if (m->specialf == M_SP_DEMON) {
	mprint ("The demon flees in terror before your weapon!");
	m_vanish (m);
    } else if (m->meleef != M_MELEE_SPIRIT) {
	if (m->level > random_range (10)) {
	    if (Player.hp < Player.maxhp)
		Player.hp = min (Player.maxhp, Player.hp + m->hp);
	    Player.str++;
	    if (Player.pow < Player.maxpow)
		Player.pow = min (Player.maxpow, Player.pow + m->level);
	    m_death (m);
	    mprint ("You feel a surge of raw power from Demonblade!");
	} else
	    p_hit (m, Player.dmg + dmgmod, NORMAL_DAMAGE);

    } else {
	mprint ("Demonblade howls as it strikes the spirit!");
	if (random_range (10) == 1) {
	    mprint ("... and shatters into a thousand lost fragments!");
	    morewait();
	    p_damage (50, UNSTOPPABLE, "Demonblade exploding");
	    Player.remove_possession (o, 1);
	} else {
	    mprint ("You feel your lifeforce draining....");
	    p_damage (25, UNSTOPPABLE, "a backlash of negative energies");
	    Player.str -= 3;
	    Player.con -= 3;
	    if (Player.str < 1 || Player.con < 1)
		p_death ("a backlash of negative energies");
	}
    }
}

void weapon_lightsabre (int dmgmod UNUSED, pob o, struct monster *m)
{
    if (!object_is_known(o)) {
	mprint ("Fumbling with the cylinder, you press the wrong stud....");
	p_damage (100, UNSTOPPABLE, "fumbling with a lightsabre");
	learn_object (o);
    } else {
	// test prevents confusing immunity messages....
	if (!m_immunityp (m, NORMAL_DAMAGE)) {
	    mprint ("Vzzzzmmm!");
	    m_damage (m, 20, NORMAL_DAMAGE);
	}
	if ((m->hp > 0) && (!m_immunityp (m, FLAME))) {
	    mprint ("Zzzzap!");
	    m_damage (m, 20, FLAME);
	}
    }
}

void weapon_tangle (int dmgmod, pob o UNUSED, struct monster *m)
{
    if ((random_range (2) == 1) && (!m_immunityp (m, NORMAL_DAMAGE))) {
	mprint ("You entangle the monster!");
	m_status_reset (m, MOBILE);
    }
    p_hit (m, Player.dmg + dmgmod, NORMAL_DAMAGE);
}

// if wielding a bow, add bow damage to arrow damage
void weapon_arrow (int dmgmod, pob o, struct monster *m)
{
    if (Player.possessions[O_WEAPON_HAND].id == WEAPON_LONGBOW)
	p_hit (m, Player.dmg + o->plus + o->dmg + dmgmod, NORMAL_DAMAGE);
    else
	p_hit (m, o->plus + o->dmg + dmgmod, NORMAL_DAMAGE);
}

// if wielding a crossbow, add bow damage to arrow damage
void weapon_bolt (int dmgmod, pob o, struct monster *m)
{
    if (Player.possessions[O_WEAPON_HAND].id == WEAPON_CROSSBOW &&
	Player.possessions[O_WEAPON_HAND].aux == LOADED) {
	p_hit (m, Player.dmg + o->plus + o->dmg + dmgmod, NORMAL_DAMAGE);
	Player.possessions[O_WEAPON_HAND].aux = UNLOADED;
    } else
	p_hit (m, o->plus + o->dmg, NORMAL_DAMAGE);
}

void weapon_mace_disrupt (int dmgmod, pob o UNUSED, struct monster *m)
{
    if (m->meleef == M_MELEE_SPIRIT) {
	mprint ("The monster crumbles away to dust!");
	m_death (m);
    } else
	p_hit (m, Player.dmg + dmgmod, UNSTOPPABLE);
}

void weapon_normal_hit (int dmgmod, pob o UNUSED, struct monster *m)
{
    p_hit (m, Player.dmg + dmgmod, NORMAL_DAMAGE);
}

// will be updated eventually
void weapon_bare_hands (int dmgmod, struct monster *m)
{
    p_hit (m, Player.dmg + dmgmod, NORMAL_DAMAGE);
}

static void i_demonblade (pob o)
{
    if (o->used) {
	learn_object (o);
	mprint ("Demonblade's fangs open and bury themselves in your wrist!");
	mprint ("You hear evil laughter in the distance....");
	mprint ("You begin to foam at the mouth!");
	Player.status[BERSERK] = 1500;
    } else {
	mprint ("You hear a sound like a demon cursing.");
	mprint ("You feel less like biting your shield.");
	Player.status[BERSERK] = 0;
    }
}

static void i_normal_weapon (pob o)
{
    if (o->used)
	mprint ("You ready your weapon for battle.");
}

static void i_lightsabre (pob o)
{
    if (o->used)
	mprint ("You feel one with the Force.");
    else
	mprint ("You feel out of touch with the Force.");
}

static void i_mace_disrupt (pob o UNUSED)
{
    mprint ("That's a damned heavy mace!");
}

void weapon_vorpal (int dmgmod, pob o, struct monster *m)
{
    if ((random_range (10) < 3) && (!m_immunityp (m, NORMAL_DAMAGE))) {
	learn_object (o);
	if (random_range (2) == 1)
	    mprint ("One Two! One Two! And through and through!");
	else
	    mprint ("Your vorpal blade goes snicker-snack!");
	m_death (m);
    } else
	weapon_normal_hit (dmgmod, o, m);
}

void weapon_desecrate (int dmgmod, pob o, struct monster *m)
{
    learn_object (o);
    if (Player.alignment < 0) {
	mprint ("Your opponent screams in agony!");
	p_hit (m, Player.dmg + dmgmod, UNSTOPPABLE);
	Player.alignment--;
	if (Player.hp < Player.maxhp) {
	    mprint ("You feel a thrill of power surging up your blade!");
	    Player.hp = min (Player.maxhp, Player.hp + Player.dmg + dmgmod);
	}
    } else {
	mprint ("Your blade turns in your hands and hits you!");
	mprint ("You hear evil laughter....");
	level_drain (Player.dmg, "the sword Desecrator");
	Player.alignment -= 10;
	mprint ("A strange force spreads from the wound throughout your body...");
	mprint ("You feel much more chaotic now.");
    }
}

void weapon_firestar (int dmgmod, pob o, struct monster *m)
{
    if (random_range (3) == 1) {
	learn_object (o);
	fball (Player.x, Player.y, Player.x, Player.y, max (Player.dmg, 25));
    }
    if (m->hp > 0)
	weapon_normal_hit (dmgmod, o, m);
}

void weapon_defend (int dmgmod, pob o, struct monster *m)
{
    if ((Player.alignment < 0) && (o->blessing > 0)) {
	mprint ("The Holy Defender screams in your hands....");
	mprint ("You stagger before the sound of its rage....");
	p_damage (50, UNSTOPPABLE, "a pissed-off Holy Defender");
	mprint ("The weapon finally quiets. It seems less holy now.");
	o->truename = o->cursestr;
	Player.status[PROTECTION] -= (o->hit);
	o->plus = 0 - absv (o->plus);
	o->blessing = -1;
    }
    if ((o->blessing > 0) && ((m->specialf == M_SP_DEMON) || (m->meleef == M_MELEE_SPIRIT))) {
	mprint ("Your opponent shies back before your holy weapon!");
	m->hit = 0;
	m->speed *= 2;
    }
    weapon_normal_hit (dmgmod, o, m);
}

void weapon_victrix (int dmgmod, pob o, struct monster *m)
{
    if (m->meleef == M_MELEE_SPIRIT) {
	mprint ("Your opponent dissipates in a harmless cloud of vapors...");
	m_death (m);
    } else
	weapon_normal_hit (dmgmod, o, m);
}

static void i_defend (pob o)
{
    learn_object (o);
    if (o->used) {
	mprint ("You feel under an aegis!");
	Player.status[PROTECTION] += o->hit;
    } else
	Player.status[PROTECTION] -= o->hit;
}

static void i_victrix (pob o)
{
    learn_object (o);
    o->blessing = absv (o->blessing);
    if (o->used) {
	Player.immunity[POISON]++;
	Player.immunity[FEAR]++;
	Player.immunity[INFECTION]++;
    } else {
	Player.immunity[POISON]--;
	Player.immunity[FEAR]--;
	Player.immunity[INFECTION]--;
    }
}

static void i_desecrate (pob o)
{
    learn_object (o);
    if (o->blessing > 0) {
	mprint ("How weird, a blessed desecrator... ");
	mprint ("The structure of reality cannot permit such a thing....");
	Player.remove_possession (o, 1);
    } else if (Level->site(Player.x,Player.y).locchar == ALTAR)
	sanctify (-1);
}

// shield functions
static void i_normal_shield (pob o)
{
    if (o->used)
	mprint ("You sling your shield across a forearm.");
}

static void i_perm_deflect (pob o)
{
    learn_object (o);
    if (o->blessing > -1) {
	if (o->used) {
	    mprint ("You feel buffered.");
	    Player.status[DEFLECTION] += 1500;
	} else {
	    Player.status[DEFLECTION] -= 1500;
	    if (Player.status[DEFLECTION] < 1) {
		mprint ("You feel less defended");
		Player.status[DEFLECTION] = 0;
	    }
	}
    } else {
	if (o->used) {
	    mprint ("You feel naked.");
	    Player.status[VULNERABLE] += 1500;
	    Player.status[DEFLECTION] = 0;
	} else {
	    Player.status[VULNERABLE] -= 1500;
	    if (Player.status[VULNERABLE] < 1) {
		mprint ("You feel less vulnerable");
		Player.status[VULNERABLE] = 0;
	    }
	}
    }
}

// amulet of the planes
static void i_planes (pob o UNUSED)
{
    if (Player.mana < 1)
	print1 ("The amulet spits some multicolored sparks.");
    else {
	print1 ("You focus mana into the amulet....");
	Player.mana = max (0, Player.mana - 100);
	dataprint();
	morewait();
	strategic_teleport (1);
    }
}

// the sceptre of high magic
static void i_sceptre (pob o UNUSED)
{
    if (HiMagicUse == Date)
	print1 ("The Sceptre makes a sort of dull 'thut' noise.");
    else if (Current_Environment == E_CIRCLE || Current_Environment == E_ASTRAL) {
	HiMagicUse = Date;	// WDT: this looks like it's a good place to use the batteries.
	print1 ("The Sceptre warps strangely for a second, and then subsides.");
	morewait();
	print2 ("You smell ozone.");	// WDT: explain the battery use.
    } else {
	HiMagicUse = Date;
	print1 ("With a shriek of tearing aether, a magic portal appears!");
	print2 ("Step through? [yn] ");
	if (ynq() == 'y')
	    change_environment (E_COURT);
	print1 ("The sceptre seems to subside. You hear a high whine, as of");
	print2 ("capacitors beginning to recharge.");
	morewait();
    }
}

// the star gem
static void i_stargem (pob o)
{
    if (StarGemUse == Date) {
	print1 ("The Star Gem glints weakly as if to say:");
	print2 ("'You have used me overmuch.'");
	print3 ("and it vanishes a puff of regret.");
	set_object_uniqueness (o, UNIQUE_UNMADE);
	// it's now out there, somewhere
	Player.remove_possession (o, 1);
    } else {
	StarGemUse = Date;
	if (o->blessing < 1) {
	    print1 ("The Star Gem shines brightly and emits a musical tone.");
	    print2 ("You see a dark cloud roil away from it.");
	    morewait();
	    o->blessing = 10;
	}
	print1 ("The star gem flares with golden light!");
	morewait();
	if (Player.status[ILLUMINATION] < 1000) {
	    print1 ("Interesting, you seem to be permanently accompanied");
	    print2 ("by a friendly lambent glow....");
	    morewait();
	    Player.status[ILLUMINATION] = 1500;
	}
	print1 ("You suddenly find yourself whisked away by some unknown force!");
	morewait();
	setgamestatus (COMPLETED_ASTRAL);
	change_environment (E_COUNTRYSIDE);
	Player.x = 61;
	Player.y = 3;
	screencheck (3);
	drawvision (Player.x, Player.y);
	locprint ("Star Peak");
	Country[Player.x][Player.y].current_terrain_type = Country[Player.x][Player.y].base_terrain_type;
	c_set (Player.x, Player.y, CHANGED);
	print2 ("The Star Gem's brilliance seems to fade.");
    }
}

// wand of fear
static void i_fear (pob o)
{
    int x = Player.x, y = Player.y;
    learn_object (o);
    setspot (&x, &y);
    if (o->blessing < 0) {
	x = Player.x;
	y = Player.y;
    }
    inflict_fear (x, y);
}

static void i_juggernaut (pob o)
{
    int d, x = Player.x, y = Player.y;
    int seen = 1, not_seen = 0;
    int tunneled = 0;

    print1 ("You activate the Juggernaut of Karnak!");
    if (!object_is_known(o)) {
	print2 ("Uh, oh, it's coming this way!");
	p_death ("the Juggernaut of Karnak");
	return;
    }
    d = getdir();
    if (d == ABORT)
	print2 ("You deactivate the Juggernaut before it escapes.");
    else {
	print1 ("Vroom! ");
	while (inbounds (x + Dirs[0][d], y + Dirs[1][d])) {
	    x += Dirs[0][d];
	    y += Dirs[1][d];
	    if (!view_unblocked (x, y) || offscreen (y))
		seen = 0;
	    if (Level->site(x,y).locchar == WALL)
		tunneled++;
	    if (Level->site(x,y).locchar != WATER && Level->site(x,y).locchar != VOID_CHAR && Level->site(x,y).locchar != ABYSS && Level->site(x,y).locchar != SPACE && Level->site(x,y).locchar != LAVA) {
		Level->site(x,y).locchar = FLOOR;
		Level->site(x,y).p_locf = L_NO_OP;
	    }
	    lreset (x, y, SECRET);
	    lset (x, y, CHANGED);
	    if (Level->creature(x,y) != NULL) {
		if (seen)
		    nprint1 ("Splat! ");
		else
		    not_seen++;
		setgamestatus (SUPPRESS_PRINTING);
		m_death (Level->creature(x,y));
		resetgamestatus (SUPPRESS_PRINTING);
	    }
	    plotspot (x, y, FALSE);
	    omshowcursor (x, y);
	}
	if (not_seen > 6)
	    print2 ("You hear many distant screams...");
	else if (not_seen > 3)
	    print2 ("You hear several distant screams...");
	else if (not_seen > 1)
	    print2 ("You hear a couple of distant screams...");
	else if (not_seen == 1)
	    print2 ("You hear a distant scream...");
	gain_experience (1000);
	Player.remove_possession (o, 1);
	Level->tunnelled += tunneled - 1;
	tunnelcheck();
    }
}

static void i_symbol (pob o)
{
    if (!object_is_known(o))
	print1 ("Nothing seems to happen.");
    // if o->charge != 17, then symbol was stolen from own high priest!
    else if ((o->aux != Player.patron) || (o->charge != 17)) {
	print1 ("You invoke the deity...");
	print2 ("...who for some reason seems rather annoyed at you...");
	print3 ("You are enveloped in Godsfire!");
	morewait();
	for (; Player.hp > 1; Player.hp--) {
	    dataprint();
	    morewait();
	    Player.remove_all_possessions();
	    Player.mana = 0;
	}
    } else if (SymbolUseHour == hour()) {
	print1 ("Your deity frowns upon this profligate use of power...");
	print2 ("Shazam! A bolt of Godsfire! Your symbol shatters!");
	Player.remove_possession (o, 1);
	Player.hp = 1;
	dataprint();
    } else {
	print1 ("A mystic flow of theurgic energy courses through your body!");
	SymbolUseHour = hour();
	cleanse (1);
	heal (10);
	Player.mana = max (Player.mana, calcmana());
    }
}

static void i_crystal (pob o)
{
    if (!object_is_known(o)) {
	print1 ("You can't figure out how to activate this orb.");
	return;
    }
    print1 ("You gaze into your crystal ball.");
    if (ViewHour == hour())
	print2 ("All you get is Gilligan's Island reruns.");
    else if (o->blessing < 0 || Player.iq + Player.level < random_range(30)) {
	ViewHour = hour();
	print2 ("Weird interference patterns from the crystal fog your mind....");
	amnesia();
    } else {
	ViewHour = hour();
	print2 ("You sense the presence of life...");
	mondet (1);
	morewait();
	print2 ("You sense the presence of objects...");
	objdet (1);
	morewait();
	print2 ("You begin to see visions of things beyond your ken....");
	hint();
    }
}

static void i_antioch (pob o)
{
    int x = Player.x, y = Player.y;
    int count;
    if (!object_is_known(o)) {
	print1 ("Ka-Boom!");
	print2 ("You seem to have annihilated yourself.");
	p_death ("the Holy Hand-Grenade of Antioch");
    } else {
	print1 ("Bring out the Holy Hand-Grenade of Antioch!");
	setspot (&x, &y);
	print2 ("Ok, you pull the pin.....");
	morewait();
	print1 ("What do you count up to? ");
	count = (int) parsenum();
	if (count < 3 && Level->creature(x,y)) {
	    print1 ("`Three shall be the number of thy counting....");
	    print2 ("And the number of thy counting shall be three.'");
	    print3 ("Your target picks up the grenade and throws it back!");
	    morewait();
	    clearmsg();
	    print1 ("Ka-Boom!");
	    p_death ("the Holy Hand-Grenade of Antioch");
	} else if (count > 3) {
	    print1 ("`Three shall be the number of thy counting.");
	    print2 ("And the number of thy counting shall be three.'");
	    morewait();
	    clearmsg();
	    print1 ("Ka-Boom!");
	    p_death ("the Holy Hand-Grenade of Antioch");
	} else {
	    print1 ("Ka-Boom!");
	    gain_experience (1000);
	    Level->site(x,y).locchar = TRAP;
	    Level->site(x,y).p_locf = L_TRAP_DOOR;
	    lset (x, y, CHANGED);
	    if (Level->creature(x,y)) {
		m_death (Level->creature(x,y));
		print2 ("You are covered with gore.");
	    }
	    Level->remove_things (x, y);
	}
    }
    Player.remove_possession (o, 1);
}

static void i_kolwynia (pob o)
{
    if (!object_is_known(o)) {
	print1 ("You destroy youself with a mana storm. How sad.");
	p_death ("Kolwynia, The Key That Was Lost");
    } else {
	gain_experience (5000);
	print1 ("You seem to have gained complete mastery of magic.");
	Player.pow = Player.maxpow = 2 * Player.maxpow;
	learn_all_spells();
    }
    Player.remove_possession (o, 1);
}

static void i_enchantment (pob o)
{
    char response;
    if (ZapHour == hour())
	print1 ("The staff doesn't seem to have recharged yet.");
    else if (!object_is_known(o)) {
	ZapHour = hour();
	print1 ("You blast the staff backwards....");
	dispel (-1);
    } else {
	ZapHour = hour();
	print1 ("Zap with white or black end [wb] ");
	do
	    response = (char) mcigetc();
	while ((response != 'w') && (response != 'b'));
	print2 ("The staff discharges!");
	if (response == 'w')
	    enchant (o->blessing * 2 + 1);
	else
	    dispel (o->blessing);
    }
}

static void i_helm (pob o)
{
    if (HelmHour == hour())
	print1 ("The helm doesn't seem to have recharged yet.");
    else if (!object_is_known(o)) {
	HelmHour = hour();
	print1 ("You put the helm on backwards....");
	p_teleport (-1);
    } else {
	HelmHour = hour();
	print1 ("Your environment fades.... and rematerializes.");
	p_teleport (o->blessing);
    }
}

static void i_death (pob o UNUSED)
{
    clearmsg();
    print1 ("Bad move...");
    p_death ("the Potion of Death");
}

static void i_life (pob o)
{
    clearmsg();
    print1 ("Good move.");
    Player.hp = Player.maxhp = 2 * Player.maxhp;
    Player.remove_possession (o, 1);
}

// f = fire, w = water, e = earth, a = air, m = mastery
static int orbcheck (int element)
{
    char response;
    print1 ("The orb begins to glow with increasing intensity!");
    print2 ("You have the feeling you need to do something more....");
    morewait();
    print1 ("Burn it in fire [f] ");
    print2 ("Douse it with water [w] ");
    morewait();
    print1 ("Smash it against the earth [e] ");
    print2 ("Toss is through the air [a] ");
    morewait();
    print1 ("Mix the above actions, doing them in sequence [m] ");
    do {
	print2 ("Which one [f,w,e,a,m] ");
	response = (char) mcigetc();
    } while ((response != 'f') && (response != 'w') && (response != 'e') && (response != 'a') && (response != 'm'));
    return (response == element);
}

// orb functions
static void i_orbfire (pob o)
{
    if (!orbcheck ('f')) {
	print1 ("Bad choice!");
	print2 ("The Orb of Fire blasts you!");
	fball (Player.x, Player.y, Player.x, Player.y, 250);
	learn_object (o);
    } else {
	print1 ("The Orb of Fire flares a brilliant red!");
	learn_spell (S_FIREBOLT);
	gain_experience (10000);
	Player.immunity[FLAME] += 100;
	print2 ("You feel fiery!");
	o->plus = 100;
	o->blessing = 100;
	i_firebolt (o);
    }
    *o = Objects[ORB_BURNT_OUT];
}

static void i_orbwater (pob o)
{
    if (!orbcheck ('w')) {
	print1 ("A serious mistake!");
	print2 ("The Orb of Water blasts you!");
	heal (-250);
	learn_object (o);
    } else {
	print1 ("The Orb of Water pulses a deep green!");
	learn_spell (S_DISRUPT);
	Player.immunity[POISON] += 100;
	gain_experience (10000);
	print2 ("You feel wet!");
	o->plus = 100;
	o->blessing = 100;
	i_disrupt (o);
    }
    *o = Objects[ORB_BURNT_OUT];
}

static void i_orbearth (pob o)
{
    if (!orbcheck ('e')) {
	print1 ("What a moron!");
	print2 ("The Orb of Earth blasts you!");
	Player.con -= 10;
	if (Player.con < 3)
	    p_death ("congestive heart failure");
	else {
	    print3 ("Your possessions disintegrate!");
	    Player.remove_all_possessions();
	    Player.pack.clear();
	    learn_object (o);
	}
    } else {
	print1 ("The Orb of Earth emanates a brownish aura!");
	learn_spell (S_DISINTEGRATE);
	gain_experience (10000);
	Player.immunity[NEGENERGY] += 100;
	print2 ("You feel earthy!");
	o->plus = 100;
	o->blessing = 100;
	i_disintegrate (o);
    }
    *o = Objects[ORB_BURNT_OUT];
}

static void i_orbair (pob o)
{
    if (!orbcheck ('a')) {
	print1 ("You lose!");
	print2 ("The Orb of Air blasts you!");
	lball (Player.x, Player.y, Player.x, Player.y, 100);
	learn_object (o);
    } else {
	print1 ("The Orb of Air flashes blue!");
	learn_spell (S_LBALL);
	gain_experience (10000);
	print2 ("You feel airy!");
	Player.immunity[ELECTRICITY] += 100;
	o->plus = 100;
	o->blessing = 100;
	i_invisible (o);
	i_lbolt (o);
    }
    *o = Objects[ORB_BURNT_OUT];
}

static void i_orbmastery (pob o)
{

    if (!orbcheck ('m')) {
	print1 ("A fatal error!");
	print2 ("The Orb of Mastery blasts you to cinders!");
	p_death ("playing with the Orb of Mastery");
	learn_object (o);
    } else if ((find_and_remove_item (ORB_OF_FIRE, -1)) && (find_and_remove_item (ORB_OF_EARTH, -1)) && (find_and_remove_item (ORB_OF_AIR, -1)) && (find_and_remove_item (ORB_OF_WATER, -1))) {
	print1 ("The Orb of Mastery radiates rainbow colors!");
	print2 ("You feel godlike.");
	Player.iq = Player.maxiq = 2 * Player.maxiq;
	Player.pow = Player.maxpow = 2 * Player.maxpow;
	Player.str = Player.maxstr = 2 * Player.maxstr;
	Player.dex = Player.maxdex = 2 * Player.maxdex;
	Player.con = Player.maxcon = 2 * Player.maxcon;
	Player.agi = Player.maxagi = 2 * Player.maxagi;
	dataprint();
	morewait();
	print1 ("You have been imbued with a cosmic power....");
	morewait();
	wish (1);
	morewait();
	clearmsg();
	print2 ("You feel much more experienced.");
	gain_experience (20000);
	*o = Objects[ORB_BURNT_OUT];
    } else {
	print1 ("The Orb of Mastery's power is unbalanced!");
	print2 ("The Orb of Mastery blasts you to cinders!");
	p_death ("playing with the Orb of Mastery");
    }
}

static void i_orbdead (pob o UNUSED)
{
    int i;
    print1 ("The burnt-out orb drains all your energy!");
    forget_all_spells();
    print2 ("You feel not at all like a mage.");
    for (i = 0; i < MAXITEMS; i++) {
	if (Player.has_possession(i)) {
	    Player.possessions[i].plus = 0;
	    if (Player.possessions[i].usef > 100)
		Player.possessions[i].usef = I_NOTHING;
	}
    }
    print3 ("A storm of mundanity surounds you!");
    level_drain (Player.level - 1, "a Burnt-out Orb");
    Player.mana = 0;
    Player.pow -= 10;
}

static void i_dispel (pob o)
{
    dispel ((o->blessing > -1) ? o->blessing + random_range (3) : o->blessing);
}

// stick functions

// wand of apportation
static void i_apport (pob o)
{
    learn_object (o);
    apport (o->blessing);
}

// staff of firebolts
static void i_firebolt (pob o)
{
    int x = Player.x, y = Player.y;
    learn_object (o);
    setspot (&x, &y);
    if (o->blessing < 0) {
	x = Player.x;
	y = Player.y;
    }
    fbolt (Player.x, Player.y, x, y, Player.dex * 2 + Player.level, 75);
}

static void i_disintegrate (pob o)
{
    int x = Player.x, y = Player.y;
    learn_object (o);
    setspot (&x, &y);
    if (o->blessing < 0) {
	x = Player.x;
	y = Player.y;
    }
    disintegrate (x, y);
}

static void i_disrupt (pob o)
{
    int x = Player.x, y = Player.y;
    learn_object (o);
    setspot (&x, &y);
    if (o->blessing < 0) {
	x = Player.x;
	y = Player.y;
    }
    disrupt (x, y, 100);
}

// staff of lightning bolts
static void i_lbolt (pob o)
{
    int x = Player.x, y = Player.y;
    learn_object (o);
    setspot (&x, &y);
    if (o->blessing < 0) {
	x = Player.x;
	y = Player.y;
    }
    lbolt (Player.x, Player.y, x, y, Player.dex * 2 + Player.level, 75);
}

// wand of magic missiles
static void i_missile (pob o)
{
    int x = Player.x, y = Player.y;
    learn_object (o);
    setspot (&x, &y);
    if (o->blessing < 0) {
	x = Player.x;
	y = Player.y;
    }
    nbolt (Player.x, Player.y, x, y, Player.dex * 2 + Player.level, 20);
}

// wand of fire balls
static void i_fireball (pob o)
{
    int x = Player.x, y = Player.y;
    learn_object (o);
    setspot (&x, &y);
    if (o->blessing < 0) {
	x = Player.x;
	y = Player.y;
    }
    fball (Player.x, Player.y, x, y, 35);
}

// wand of snowballs
static void i_snowball (pob o)
{
    int x = Player.x, y = Player.y;
    learn_object (o);
    setspot (&x, &y);
    if (o->blessing < 0) {
	x = Player.x;
	y = Player.y;
    }
    snowball (Player.x, Player.y, x, y, 20);
}

// wand of lightning balls
static void i_lball (pob o)
{
    int x = Player.x, y = Player.y;
    learn_object (o);
    setspot (&x, &y);
    if (o->blessing < 0) {
	x = Player.x;
	y = Player.y;
    }
    lball (Player.x, Player.y, x, y, 50);
}

// staff of sleep
static void i_sleep_other (pob o)
{
    learn_object (o);
    sleep_monster (o->blessing);
}

// rod of summoning
// rod of summoning now always summons as if cursed
static void i_summon (pob o)
{
    learn_object (o);
    summon (-1, -1);
}

static void i_hide (pob o)
{
    int x = Player.x, y = Player.y;
    learn_object (o);
    setspot (&x, &y);
    hide (x, y);
}

static void i_polymorph (pob o)
{
    learn_object (o);
    polymorph (o->blessing);
}

void item_use (struct object *o)
{
    clearmsg();
    switch (o->usef) {
	default:
	case I_NO_OP:			i_no_op (o); break;
	case I_NOTHING:			i_nothing (o); break;
	case I_SPELLS:			i_spells (o); break;
	case I_BLESS:			i_bless (o); break;
	case I_ACQUIRE:			i_acquire (o); break;
	case I_ENCHANT:			i_enchant (o); break;
	case I_TELEPORT:		i_teleport (o); break;
	case I_WISH:			i_wish (o); break;
	case I_CLAIRVOYANCE:		i_clairvoyance (o); break;
	case I_DISPLACE:		i_displace (o); break;
	case I_ID:			i_id (o); break;
	case I_JANE_T:			i_jane_t (o); break;
	case I_FLUX:			i_flux (o); break;
	case I_WARP:			i_warp (o); break;
	case I_ALERT:			i_alert (o); break;
	case I_CHARGE:			i_charge (o); break;
	case I_KNOWLEDGE:		i_knowledge (o); break;
	case I_LAW:			i_law (o); break;
	case I_HINT:			hint(); break;
	case I_HERO:			i_hero (o); break;
	case I_TRUESIGHT:		i_truesight (o); break;
	case I_ILLUMINATE:		i_illuminate (o); break;
	case I_DEFLECT:			i_deflect (o); break;
	case I_HEAL:			i_heal (o); break;
	case I_OBJDET:			i_objdet (o); break;
	case I_MONDET:			i_mondet (o); break;
	case I_SLEEP_SELF:		i_sleep_self (o); break;
	case I_NEUTRALIZE_POISON:	i_neutralize_poison (o); break;
	case I_RESTORE:			i_restore (o); break;
	case I_SPEED:			i_speed (o); break;
	case I_AZOTH:			i_azoth (o); break;
	case I_AUGMENT:			i_augment (o); break;
	case I_REGENERATE:		i_regenerate (o); break;
	case I_INVISIBLE:		i_invisible (o); break;
	case I_BREATHING:		i_breathing (o); break;
	case I_FEAR_RESIST:		i_fear_resist (o); break;
	case I_CHAOS:			i_chaos (o); break;
	case I_ACCURACY:		i_accuracy (o); break;
	case I_LEVITATION:		i_levitate (o); break;
	case I_CURE:			i_cure (o); break;
	case I_FIREBOLT:		i_firebolt (o); break;
	case I_LBOLT:			i_lbolt (o); break;
	case I_MISSILE:			i_missile (o); break;
	case I_SLEEP_OTHER:		i_sleep_other (o); break;
	case I_FIREBALL:		i_fireball (o); break;
	case I_LBALL:			i_lball (o); break;
	case I_SNOWBALL:		i_snowball (o); break;
	case I_SUMMON:			i_summon (o); break;
	case I_HIDE:			i_hide (o); break;
	case I_DISRUPT:			i_disrupt (o); break;
	case I_DISINTEGRATE:		i_disintegrate (o); break;
	case I_APPORT:			i_apport (o); break;
	case I_DISPEL:			i_dispel (o); break;
	case I_POLYMORPH:		i_polymorph (o); break;
	case I_FEAR:			i_fear (o); break;
	case I_FOOD:			i_food (o); break;
	case I_LEMBAS:			i_lembas (o); break;
	case I_STIM:			i_stim (o); break;
	case I_POW:			i_pow (o); break;
	case I_IMMUNE:			i_immune (o); break;
	case I_POISON_FOOD:		i_poison_food (o); break;
	case I_CORPSE:			i_corpse (o); break;
	case I_PEPPER_FOOD:		i_pepper_food (o); break;
	case I_PERM_SPEED:		i_perm_speed (o); break;
	case I_PERM_HERO:		i_perm_hero (o); break;
	case I_PERM_LEVITATE:		i_perm_levitate (o); break;
	case I_PERM_AGILITY:		i_perm_agility (o); break;
	case I_SCEPTRE:			i_sceptre (o); break;
	case I_PLANES:			i_planes (o); break;
	case I_STARGEM:			i_stargem (o); break;
	case I_SYMBOL:			i_symbol (o); break;
	case I_ORBMASTERY:		i_orbmastery (o); break;
	case I_ORBFIRE:			i_orbfire (o); break;
	case I_ORBWATER:		i_orbwater (o); break;
	case I_ORBEARTH:		i_orbearth (o); break;
	case I_ORBAIR:			i_orbair (o); break;
	case I_ORBDEAD:			i_orbdead (o); break;
	case I_CRYSTAL:			i_crystal (o); break;
	case I_LIFE:			i_life (o); break;
	case I_DEATH:			i_death (o); break;
	case I_ANTIOCH:			i_antioch (o); break;
	case I_HELM:			i_helm (o); break;
	case I_KOLWYNIA:		i_kolwynia (o); break;
	case I_ENCHANTMENT:		i_enchantment (o); break;
	case I_JUGGERNAUT:		i_juggernaut (o); break;
	case I_PERM_DISPLACE:		i_perm_displace (o); break;
	case I_PERM_NEGIMMUNE:		i_perm_negimmune (o); break;
	case I_PERM_INVISIBLE:		i_perm_invisible (o); break;
	case I_PERM_PROTECTION:		i_perm_protection (o); break;
	case I_PERM_ACCURACY:		i_perm_accuracy (o); break;
	case I_PERM_TRUESIGHT:		i_perm_truesight (o); break;
	case I_PERM_BURDEN:		i_perm_burden (o); break;
	case I_PERM_STRENGTH:		i_perm_strength (o); break;
	case I_PERM_GAZE_IMMUNE:	i_perm_gaze_immune (o); break;
	case I_PERM_FIRE_RESIST:	i_perm_fire_resist (o); break;
	case I_PERM_POISON_RESIST:	i_perm_poison_resist (o); break;
	case I_PERM_REGENERATE:		i_perm_regenerate (o); break;
	case I_PERM_KNOWLEDGE:		i_perm_knowledge (o); break;
	case I_NORMAL_ARMOR:		i_normal_armor (o); break;
	case I_PERM_FEAR_RESIST:	i_perm_fear_resist (o); break;
	case I_PERM_ENERGY_RESIST:	i_perm_energy_resist (o); break;
	case I_PERM_BREATHING:		i_perm_breathing (o); break;
	case I_NORMAL_WEAPON:		i_normal_weapon (o); break;
	case I_LIGHTSABRE:		i_lightsabre (o); break;
	case I_DEMONBLADE:		i_demonblade (o); break;
	case I_DESECRATE:		i_desecrate (o); break;
	case I_MACE_DISRUPT:		i_mace_disrupt (o); break;
	case I_DEFEND:			i_defend (o); break;
	case I_VICTRIX:			i_victrix (o); break;
	case I_PICK:			i_pick (o); break;
	case I_KEY:			i_key (o); break;
	case I_PERM_ILLUMINATE:		i_perm_illuminate (o); break;
	case I_TRAP:			i_trap (o); break;
	case I_RAISE_PORTCULLIS:	i_raise_portcullis (o); break;
	case I_NORMAL_SHIELD:		i_normal_shield (o); break;
	case I_PERM_DEFLECT:		i_perm_deflect (o); break;
    }
}
