// Omega is free software, distributed under the MIT license

#include "glob.h"

//----------------------------------------------------------------------

static void answer_prayer(void);
static int check_sacrilege(int deity);
static void hp_req_print(void);
static void hp_req_test(void);
static int increase_priest_rank(int deity);
static void make_hp(pob o);

//----------------------------------------------------------------------

// prayer occurs at altars, hence name of function
void l_altar (void)
{
    int i, deity;
    char response;

    if (Level->environment == E_COUNTRYSIDE)
	deity = DRUID;
    else
	deity = Level->site(Player.x,Player.y).aux;

    switch (deity) {
	default:
	    mprint ("This rude altar has no markings.");
	    break;
	case ODIN:
	    mprint ("This granite altar is graven with a gallows.");
	    break;
	case SET:
	    mprint ("This sandstone altar has a black hand drawn on it.");
	    break;
	case HECATE:
	    mprint ("This silver altar is inlaid with a black crescent moon.");
	    break;
	case ATHENA:
	    mprint ("This golden altar is inscribed with an owl.");
	    break;
	case DESTINY:
	    mprint ("This crystal altar is in the form of an omega.");
	    break;
	case DRUID:
	    mprint ("This oaken altar is ornately engraved with leaves.");
	    break;
    }
    mprint ("Worship at this altar? [yn] ");
    if (ynq() == 'y') {
	if (Player.rank[PRIESTHOOD] == NOT_A_BELIEVER)
	    increase_priest_rank (deity);
	else if (!check_sacrilege (deity)) {
	    if (Blessing)
		mprint ("You have a sense of immanence.");
	    mprint ("Request a Blessing, Sacrifice an item, or just Pray [b,s,p] ");
	    do
		response = (char) mcigetc();
	    while ((response != 'b') && (response != 's') && (response != 'p') && (response != KEY_ESCAPE));
	    if (response == 'b') {
		mprint ("You beg a heavenly benefice.");
		mprint ("You hear a gong resonating throughout eternity....");
		morewait();
		if (Blessing) {
		    mprint ("A shaft of lucent radiance lances down from the heavens!");
		    mprint ("You feel uplifted....");
		    morewait();
		    gain_experience (Player.rank[PRIESTHOOD] * Player.rank[PRIESTHOOD] * 50);
		    cleanse (1);
		    heal (10);
		    bless (1);
		    Blessing = false;
		    increase_priest_rank (deity);
		} else {
		    mprint ("Your ardent plea is ignored.");
		    mprint ("You feel ashamed.");
		    Player.xp -= (Player.xp / 4);
		}
		calc_melee();
	    } else if (response == 's') {
		mprint ("Which item to Sacrifice?");
		i = getitem (NULL_ITEM);
		if (i == ABORT)
		    i = 0;
		if (!Player.has_possession(i)) {
		    mprint ("You have insulted your deity!");
		    mprint ("Not a good idea, as it turns out...");
		    dispel (-1);
		    p_damage (Player.hp - 1, UNSTOPPABLE, "a god's pique");
		} else if (true_item_value (Player.possessions[i]) > (long) (Player.rank[PRIESTHOOD] * Player.rank[PRIESTHOOD] * Player.rank[PRIESTHOOD] * 50)) {
		    mprint ("With a burst of blue flame, your offering vanishes!");
		    Player.remove_possession (i, 1);
		    mprint ("A violet nimbus settles around your head and slowly fades.");
		    morewait();
		    Blessing = true;
		} else {
		    mprint ("A darkling glow envelopes your offering!");
		    mprint ("The glow slowly fades....");
		    morewait();
		    setgamestatus (SUPPRESS_PRINTING);
		    object o = Player.possessions[i];
		    Player.remove_possession(i);
		    o.blessing = -1 - absv (o.blessing);
		    Player.add_possession (i, o);
		    resetgamestatus (SUPPRESS_PRINTING);
		}
	    } else if (response == 'p') {
		if (deity != Player.patron)
		    mprint ("Nothing seems to happen.");
		else if (!increase_priest_rank (deity))
		    answer_prayer();
	    }
	}
    }
}

static int check_sacrilege (int deity)
{
    int sacrilege = false;
    if ((Player.patron != deity) && (Player.patron > 0)) {
	sacrilege = true;
	Player.pow--;
	Player.maxpow--;
	switch (Player.patron) {
	    case ODIN:
		mprint ("Odin notices your lack of faith! ");
		morewait();
		if (deity == ATHENA) {
		    mprint ("However, Athena intercedes on your behalf.");
		    sacrilege = false;
		} else {
		    mprint ("You are struck by a thunderbolt!");
		    p_damage (Player.level * 5, UNSTOPPABLE, "Odin's wrath");
		    if (Player.hp > 0) {
			morewait();
			mprint ("The bolt warps your feeble frame....");
			Player.maxcon = Player.maxcon / 2;
			Player.con = min (Player.con, Player.maxcon);
			Player.maxstr = Player.maxstr / 2;
			Player.con = min (Player.str, Player.maxstr);
		    }
		}
		morewait();
		break;
	    case SET:
		mprint ("Set notices your lack of faith! ");
		morewait();
		if (deity == HECATE) {
		    mprint ("But since you pray to a friendly deity,");
		    mprint ("Set decides not to punish you.");
		    sacrilege = false;
		} else {
		    mprint ("You are blasted by a shaft of black fire!");
		    p_damage (Player.level * 5, UNSTOPPABLE, "Set's anger");
		    if (Player.hp > 0) {
			morewait();
			mprint ("You are wreathed in clouds of smoke.");
			Player.remove_all_possessions();
			morewait();
			mprint ("You feel Set's Black Hand on your heart....");
			Player.con = Player.maxcon = Player.maxcon / 4;
		    }
		}
		morewait();
		break;
	    case HECATE:
		mprint ("Hecate notices your lack of faith! ");
		morewait();
		if (deity == SET) {
		    mprint ("But ignores the affront since she likes Set.");
		    sacrilege = false;
		} else {
		    mprint ("You are zapped by dark moonbeams!");
		    p_damage (Player.level * 5, UNSTOPPABLE, "Hecate's malice");
		    if (Player.hp > 0) {
			mprint ("The beams leach you of magical power!");
			Player.maxpow = Player.maxpow / 5;
			Player.pow = min (Player.pow, Player.maxpow);
			forget_all_spells();
		    }
		}
		morewait();
		break;
	    case ATHENA:
		mprint ("Athena notices your lack of faith! ");
		morewait();
		if (deity == ODIN) {
		    mprint ("But lets you off this time since Odin is also Lawful.");
		    sacrilege = false;
		} else {
		    mprint ("You are zorched by godsfire!");
		    if (Player.hp > 0) {
			morewait();
			mprint ("The fire burns away your worldly experience!");
			Player.level = 0;
			Player.xp = 0;
			Player.maxhp = Player.hp = Player.con;
			mprint ("Your power is reduced by the blast!!!");
			Player.pow = Player.maxpow = Player.maxpow / 3;
			Player.mana = min (Player.mana, Player.calcmana());
		    }
		}
		morewait();
		break;
	    case DESTINY:
		mprint ("The Lords of Destiny ignore your lack of faith.");
		sacrilege = false;
		morewait();
		break;
	    case DRUID:
		mprint ("Your treachery to the ArchDruid has been noted.");
		if (random_range (2) == 1)
		    Player.alignment += 40;
		else
		    Player.alignment -= 40;
		morewait();
		break;
	}
	if (sacrilege) {
	    Player.patron = 0;
	    Player.rank[PRIESTHOOD] = NOT_A_BELIEVER;
	}
    }
    return sacrilege;
}

static int increase_priest_rank (int deity)
{
    if (Player.rank[PRIESTHOOD] == NOT_A_BELIEVER)
	switch (deity) {
	    default:
		mprint ("Some nameless god blesses you....");
		Player.hp = max<int16_t> (Player.hp, Player.maxhp);
		morewait();
		mprint ("The altar crumbles to dust and blows away.");
		Level->site(Player.x,Player.y).locchar = FLOOR;
		Level->site(Player.x,Player.y).p_locf = L_NO_OP;
		lset (Player.x, Player.y, CHANGED);
		break;
	    case ODIN:
		if (Player.alignment > 0) {
		    mprint ("Odin hears your prayer!");
		    mprintf ("%s personally blesses you. You are now a lay devotee of Odin.", Priest[ODIN]);
		    Player.patron = ODIN;
		    Player.rank[PRIESTHOOD] = LAY;
		    Player.guildxp[PRIESTHOOD] = 1;
		    morewait();
		    learnclericalspells (ODIN, LAY);
		} else
		    mprint ("Odin ignores you.");
		break;
	    case SET:
		if (Player.alignment < 0) {
		    mprint ("Set hears your prayer!");
		    mprintf ("%s personally blesses you. You are now a lay devotee of Set.", Priest[SET]);
		    Player.patron = SET;
		    Player.rank[PRIESTHOOD] = LAY;
		    Player.guildxp[PRIESTHOOD] = 1;
		    morewait();
		    learnclericalspells (SET, LAY);
		} else
		    mprint ("Set ignores you.");
		break;
	    case ATHENA:
		if (Player.alignment > 0) {
		    mprint ("Athena hears your prayer!");
		    mprintf ("%s personally blesses you. You are now a lay devotee of Athena.", Priest[ATHENA]);
		    Player.patron = ATHENA;
		    Player.rank[PRIESTHOOD] = LAY;
		    Player.guildxp[PRIESTHOOD] = 1;
		    morewait();
		    learnclericalspells (ATHENA, LAY);
		} else
		    mprint ("Athena ignores you.");
		break;
	    case HECATE:
		if (Player.alignment < 0) {
		    mprint ("Hecate hears your prayer!");
		    mprintf ("%s personally blesses you. You are now a lay devotee of Hecate.", Priest[HECATE]);
		    Player.patron = HECATE;
		    Player.rank[PRIESTHOOD] = LAY;
		    Player.guildxp[PRIESTHOOD] = 1;
		    morewait();
		    learnclericalspells (HECATE, LAY);
		} else
		    mprint ("Hecate ignores you.");
		break;
	    case DRUID:
		if (absv (Player.alignment) < 10) {
		    mprintf ("%s personally blesses you. You are now a lay devotee of the Druids.", Priest[DRUID]);
		    Player.patron = DRUID;
		    Player.rank[PRIESTHOOD] = LAY;
		    Player.guildxp[PRIESTHOOD] = 1;
		    morewait();
		    learnclericalspells (DRUID, LAY);
		} else {
		    mprint ("You hear a voice....");
		    morewait();
		    mprint ("'Only those who embody the Balance may become Druids.'");
		}
		break;
	    case DESTINY:
		mprint ("The Lords of Destiny could hardly care less.");
		mprint ("But you can consider yourself now to be a lay devotee.");
		Player.patron = DESTINY;
		Player.rank[PRIESTHOOD] = LAY;
		Player.guildxp[PRIESTHOOD] = 1;
		break;
    } else if (deity == Player.patron) {
	if (((deity == ODIN || deity == ATHENA) && Player.alignment < 1) || ((deity == SET || deity == HECATE) && Player.alignment > 1) || (deity == DRUID && absv(Player.alignment) > 10)) {
	    mprint ("You have swerved from the One True Path!");
	    mprint ("Your deity is greatly displeased...");
	    Player.xp -= Player.level * Player.level;
	    Player.xp = max (0U, Player.xp);
	} else if (Player.rank[PRIESTHOOD] == HIGHPRIEST)
	    return 0;
	else if (Player.rank[PRIESTHOOD] == SPRIEST) {
	    if (Player.level > Priestlevel[deity])
		hp_req_test();
	    else
		return 0;
	} else if (Player.rank[PRIESTHOOD] == PRIEST) {
	    if (Player.guildxp[PRIESTHOOD] >= 4000) {
		mprint ("An heavenly fanfare surrounds you!");
		mprint ("Your deity raises you to the post of Senior Priest.");
		hp_req_print();
		Player.rank[PRIESTHOOD] = SPRIEST;
		morewait();
		learnclericalspells (deity, SPRIEST);
	    } else
		return 0;
	} else if (Player.rank[PRIESTHOOD] == ACOLYTE) {
	    if (Player.guildxp[PRIESTHOOD] >= 1500) {
		mprint ("A trumpet sounds in the distance.");
		mprint ("Your deity raises you to the post of Priest.");
		Player.rank[PRIESTHOOD] = PRIEST;
		morewait();
		learnclericalspells (deity, PRIEST);
	    } else
		return 0;
	} else if (Player.rank[PRIESTHOOD] == LAY) {
	    if (Player.guildxp[PRIESTHOOD] >= 400) {
		mprint ("A mellifluous chime sounds from above the altar.");
		mprint ("Your deity raises you to the post of Acolyte.");
		Player.rank[PRIESTHOOD] = ACOLYTE;
		morewait();
		learnclericalspells (deity, ACOLYTE);
	    } else
		return 0;
	}
    }
    return 1;
}

static void answer_prayer (void)
{
    clearmsg();
    switch (random_range (12)) {
	case 0:
	    mprint ("You have a revelation!");
	    break;
	case 1:
	    mprint ("You feel pious.");
	    break;
	case 2:
	    mprint ("A feeling of sanctity comes over you.");
	    break;
	default:
	    mprint ("Nothing unusual seems to happen.");
	    break;
    }
}

static void hp_req_test (void)
{
    pob o;
    switch (Player.patron) {
	case ODIN:
	    if ((o = find_item (HOLY_SYMBOL_OF_SET)))
		make_hp (o);
	    else
		hp_req_print();
	    break;
	case SET:
	    if ((o = find_item (HOLY_SYMBOL_OF_ODIN)))
		make_hp (o);
	    else
		hp_req_print();
	    break;
	case ATHENA:
	    if ((o = find_item (HOLY_SYMBOL_OF_HECATE)))
		make_hp (o);
	    else
		hp_req_print();
	    break;
	case HECATE:
	    if ((o = find_item (HOLY_SYMBOL_OF_ATHENA)))
		make_hp (o);
	    else
		hp_req_print();
	    break;
	case DRUID:
	    if ((o = find_item (HOLY_SYMBOL_OF_ODIN)))
		make_hp (o);
	    else if ((o = find_item (HOLY_SYMBOL_OF_SET)))
		make_hp (o);
	    else if ((o = find_item (HOLY_SYMBOL_OF_ATHENA)))
		make_hp (o);
	    else if ((o = find_item (HOLY_SYMBOL_OF_HECATE)))
		make_hp (o);
	    else
		hp_req_print();
	    break;
	case DESTINY:
	    if ((o = find_item (HOLY_SYMBOL_OF_DESTINY)))
		make_hp (o);
	    else
		hp_req_print();
	    break;
    }
}

static void hp_req_print (void)
{
    morewait();
    switch (Player.patron) {
	case ODIN:
	    mprintf ("To advance further, you must obtain the Holy Symbol of %s", Priest[SET]);
	    mprint ("who may be found in the main Temple of Set.");
	    break;
	case SET:
	    mprintf ("To advance further, you must obtain the Holy Symbol of %s", Priest[ODIN]);
	    mprint ("who may be found in the main Temple of Odin.");
	    break;
	case ATHENA:
	    mprintf ("To advance further, you must obtain the Holy Symbol of %s", Priest[HECATE]);
	    mprint ("who may be found in the main Temple of Hecate.");
	    break;
	case HECATE:
	    mprintf ("To advance further, you must obtain the Holy Symbol of %s", Priest[ATHENA]);
	    mprint ("who may be found in the main Temple of Athena.");
	    break;
	case DRUID:
	    mprint ("any of the aligned priests who may be found in their main Temples.");
	    break;
	case DESTINY:
	    mprintf ("To advance further, you must obtain the Holy Symbol of %s", Priest[DESTINY]);
	    mprint ("who may be found in the main Temple of Destiny.");
	    break;
    }
}

static void make_hp (pob o)
{
    mprint ("A full-scale heavenly choir chants 'Hallelujah' all around you!");
    mprint ("You notice a change in the symbol you carry....");
    *o = Objects[HOLY_SYMBOL_OF_ODIN-1+Player.patron];
    learn_object (o);
    o->charge = 17;		// random hack to convey bit that symbol is functional
    morewait();
    if (Player.patron == DRUID)
	mprint ("Your deity raises you to the post of ArchDruid!");
    else
	mprint ("Your deity raises you to the post of High Priest!");
    mprint ("You feel holy.");
    Player.rank[PRIESTHOOD] = HIGHPRIEST;
    morewait();
    learnclericalspells (Player.patron, HIGHPRIEST);
}
