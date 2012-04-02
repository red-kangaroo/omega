#include "glob.h"

//----------------------------------------------------------------------

static void s_accuracy(void);
static void s_alert(void);
static void s_apport(void);
static void s_bless(void);
static void s_breathe(void);
static void s_clairvoyance(void);
static void s_cure(void);
static void s_desecrate(void);
static void s_disintegrate(void);
static void s_dispel(void);
static void s_disrupt(void);
static void s_drain(void);
static void s_enchant(void);
static void s_fear(void);
static void s_firebolt(void);
static void s_haste(void);
static void s_heal(void);
static void s_hellfire(void);
static void s_hero(void);
static void s_identify(void);
static void s_invisible(void);
static void s_knowledge(void);
static void s_lball(void);
static void s_levitate(void);
static void s_missile(void);
static void s_mondet(void);
static void s_objdet(void);
static void s_polymorph(void);
static void s_regenerate(void);
static void s_restore(void);
static void s_return(void);
static void s_ritual(void);
static void s_sanctify(void);
static void s_sanctuary(void);
static void s_shadowform(void);
static void s_sleep(void);
static void s_summon(void);
static void s_teleport(void);
static void s_truesight(void);
static void s_warp(void);
static void s_wish(void);
static void showknownspells(int first, int last);
static int spellparse(void);

//----------------------------------------------------------------------

static void s_wish (void)
{
    if (random_range (100) > Player.iq + Player.pow + Player.level) {
	mprint ("Your concentration is flawed!");
	mprint ("The spell energy backfires!");
	p_damage (random_range (Spells[S_WISH].powerdrain), UNSTOPPABLE, "a backfired wish spell");
    } else {
	wish (0);
	if (spell_is_known (S_WISH)) {
	    mprint ("The power of the spell is too much for you to withstand!");
	    mprint ("All memory of the spell is expunged from your brain.");
	    forget_spell (S_WISH);
	}
    }
}

static void s_firebolt (void)
{
    int x = Player.x, y = Player.y;
    setspot (&x, &y);
    fbolt (Player.x, Player.y, x, y, Player.dex * 2 + Player.level, Player.level * 10 + 10);
}

static void s_missile (void)
{
    int x = Player.x, y = Player.y;
    setspot (&x, &y);
    nbolt (Player.x, Player.y, x, y, Player.dex * 2 + Player.level, Player.level * 3 + 3);
}

static void s_teleport (void)
{
    p_teleport (0);
}

static void s_disrupt (void)
{
    int x = Player.x, y = Player.y;
    setspot (&x, &y);
    disrupt (x, y, Player.level * 10 + 25);
}

static void s_disintegrate (void)
{
    int x = Player.x, y = Player.y;
    setspot (&x, &y);
    disintegrate (x, y);
}

static void s_sleep (void)
{
    sleep_monster (0);
}

static void s_heal (void)
{
    heal (3);
}

static void s_dispel (void)
{
    dispel ((Player.level + Player.maxpow) / 10);
}

static void s_breathe (void)
{
    breathe (0);
}

static void s_invisible (void)
{
    invisible (0);
}

static void s_warp (void)
{
    warp (1);
}

static void s_enchant (void)
{
    enchant (1);
}

static void s_bless (void)
{
    bless (0);
}

static void s_restore (void)
{
    recover_stat (0);
}

static void s_cure (void)
{
    cure (0);
}

static void s_truesight (void)
{
    truesight (0);
}

static void s_hellfire (void)
{
    int x = Player.x, y = Player.y;
    setspot (&x, &y);
    hellfire (x, y, 0);
}

static void s_knowledge (void)
{
    knowledge (0);
}

static void s_hero (void)
{
    hero (0);
}

// spell takes longer and longer to work deeper into dungeon
static void s_return (void)
{
    mprint ("You hear a whine as your spell begins to charge up.");
    Player.status[RETURNING] = ((Current_Environment == Current_Dungeon) ? difficulty() : 1);
}

static void s_desecrate (void)
{
    sanctify (-1);
}

static void s_haste (void)
{
    haste (0);
}

static void s_summon (void)
{
    summon (0, -1);
}

static void s_sanctuary (void)
{
    sanctuary();
}

static void s_sanctify (void)
{
    sanctify (1);
}

static void s_accuracy (void)
{
    accuracy (0);
}

static void s_fear (void)
{
    int x = Player.x, y = Player.y;
    setspot (&x, &y);
    inflict_fear (x, y);
}

// Has all kinds of effects in different circumstances. Eventually will be more interesting.
static void s_ritual (void)
{
    pob symbol;
    int i, roomno;
    int x, y;

    mprint ("You begin your ritual....");
    mprint ("You enter a deep trance. Time Passes...");
    setgamestatus (SKIP_PLAYER);
    time_clock (FALSE);
    setgamestatus (SKIP_PLAYER);
    time_clock (FALSE);
    setgamestatus (SKIP_PLAYER);
    time_clock (FALSE);
    setgamestatus (SKIP_PLAYER);
    time_clock (FALSE);
    setgamestatus (SKIP_PLAYER);
    time_clock (FALSE);
    if (RitualHour == hour())
	mprint ("Your mental fatigue prevents from completing the ritual!");
    else if (random_range (100) > Player.iq + Player.pow + Player.level)
	mprint ("Your concentration was broken -- the ritual fails!");
    else {
	mprint ("You charge the ritual with magical energy and focus your will.");
	mprint ("Time Passes...");
	setgamestatus (SKIP_PLAYER);
	time_clock (FALSE);
	setgamestatus (SKIP_PLAYER);
	time_clock (FALSE);
	setgamestatus (SKIP_PLAYER);
	time_clock (FALSE);
	setgamestatus (SKIP_PLAYER);
	time_clock (FALSE);
	setgamestatus (SKIP_PLAYER);
	time_clock (FALSE);
	RitualHour = hour();
	// set of random conditions for different ritual effects
	if (Current_Environment == E_CITY) {
	    mprint ("Flowing waves of mystical light congeal all around you.");
	    mprint ("'Like wow, man! Colors!'");
	    mprint ("Appreciative citizens throw you spare change.");
	    Player.cash += random_range (50);
	} else if ((roomno = Level->site[Player.x][Player.y].roomnumber) >= 0) {
	    if (RitualRoom == roomno)
		mprint ("For some reason the ritual doesn't work this time...");
	    else {
		RitualRoom = roomno;
		switch (RitualRoom) {
		    case RS_TREASURE_CHAMBER:
			mprint ("Your spell sets off frenetic growth all around you!");
			for (i = 0; i < 8; i++) {
			    Level->site[Player.x + Dirs[0][i]][Player.y + Dirs[1][i]].locchar = HEDGE;
			    Level->site[Player.x + Dirs[0][i]][Player.y + Dirs[1][i]].p_locf = L_TRIFID;
			    lset (Player.x + Dirs[0][i], Player.y + Dirs[1][i], CHANGED);
			}
			break;
		    case RS_HAREM:
		    case RS_BOUDOIR:
			mprint ("A secret panel opens next to the bed....");
			summon (0, random_range(2) ? INCUBUS : SATYR);
			break;
		    case RS_HIGH_MAGIC_SHRINE:
			mprint ("A storm of mana coaelesces around you.");
			mprint ("You are buffeted by bursts of random magic.");
			p_damage (random_range (Player.pow), UNSTOPPABLE, "high magic");
			mprint ("Continue ritual? Could be dangerous.... [yn] ");
			if (ynq() == 'y')
			    s_wish();
			else
			    mprint ("The mana fades away to nothingness.");
			x = Player.x;
			y = Player.y;
			while (x >= 0 && Level->site[x - 1][y].roomnumber == RS_HIGH_MAGIC_SHRINE)
			    x--;
			while (y >= 0 && Level->site[x][y - 1].roomnumber == RS_HIGH_MAGIC_SHRINE)
			    y--;
			for (i = 0; Level->site[x][y].roomnumber == RS_HIGH_MAGIC_SHRINE;) {
			    Level->site[x][y].roomnumber = RS_ZORCH;
			    x++;
			    i++;
			    if (Level->site[x][y].roomnumber != RS_HIGH_MAGIC_SHRINE) {
				x -= i;
				i = 0;
				y++;
			    }
			}
			lset (Player.x, Player.y, CHANGED);
			break;
		    case RS_MAGIC_LABORATORY:
			mprint ("Your magical activity sets off a latent spell in the lab!");
			cast_spell (random_range (NUMSPELLS));
			break;
		    case RS_PENTAGRAM_ROOM:
			mprint ("A smoky form begins to coalesce....");
			summon (-1, -1);
			mprint ("Fortunately, it seems confined to the pentagram.");
			m_status_reset (Level->mlist->m, MOBILE);
			break;
		    case RS_OMEGA_ROOM:
			mprint ("The Lords of Destiny look upon you....");
			if (Player.level > 10) {
			    mprint ("A curtain of blue flames leaps up from the omega.");
			    morewait();
			    l_adept();
			} else {
			    if (Player.patron == DESTINY) {
				mprint ("Your patrons take pity on you.");
				if ((Player.rank[PRIESTHOOD] < SPRIEST) && (!find_item (&symbol, HOLY_SYMBOL_OF_DESTINY, -1))) {
				    symbol = ((pob) checkmalloc (sizeof (objtype)));
				    *symbol = Objects[HOLY_SYMBOL_OF_DESTINY];
				    learn_object (symbol);
				    symbol->charge = 17;
				    gain_item (symbol);
				    mprint ("You feel uplifted.");
				} else
				    gain_experience (min (1000U, Player.xp));
			    } else if (random_range (3) == 1) {
				mprint ("You feel Fated.");
				gain_experience (Player.level * Player.level * 10);
				Player.hp = max (Player.hp, Player.maxhp);
			    } else if (random_range (2)) {
				mprint ("You feel Doomed.");
				Player.hp = 1;
				Player.mana = 0;
				Player.xp = 0;
			    } else
				mprint ("The Lords of Destiny laugh at you!");
			}
			break;
		    default:
			mprint ("Well, not much effect. Chalk it up to experience.");
			gain_experience (Player.level * 5);
			break;
		}
	    }
	} else {
	    if (RitualRoom == Level->site[Player.x][Player.y].roomnumber)
		mprint ("The ritual fails for some unexplainable reason.");
	    else {
		mprint ("The ritual seems to be generating some spell effect.");
		RitualRoom = Level->site[Player.x][Player.y].roomnumber;
		switch (RitualRoom) {
		    case RS_WALLSPACE:
			shadowform();
			break;
		    case RS_CORRIDOR:
			haste (0);
			break;
		    case RS_PONDS:
			breathe (0);
			break;
		    case RS_ADEPT:
			hero (1);
			break;
		    default:
			mprint ("The ritual doesn't seem to produce any tangible results...");
			gain_experience (Player.level * 6);
		}
	    }
	}
    }
}

static void s_apport (void)
{
    apport (0);
}

static void s_shadowform (void)
{
    shadowform();
}

static void s_alert (void)
{
    alert (0);
}

static void s_regenerate (void)
{
    regenerate (0);
}

static void s_clairvoyance (void)
{
    clairvoyance (10);
}

static void s_drain (void)
{
    drain (0);
}

static void s_levitate (void)
{
    levitate (0);
}

static void s_polymorph (void)
{
    polymorph (0);
}

// lball spell
static void s_lball (void)
{
    int x = Player.x, y = Player.y;
    setspot (&x, &y);
    lball (Player.x, Player.y, x, y, Player.level * 10 + 10);
}

static void s_identify (void)
{
    identify (0);
}

static void s_objdet (void)
{
    objdet (1);
}

static void s_mondet (void)
{
    mondet (1);
}

// select a spell to cast
int getspell (void)
{
    int spell = ABORT - 1;

    do {
	mprint ("Cast Spell: [type spell abbrev, ?, or ESCAPE]: ");
	spell = spellparse();
    } while (spell < ABORT);
    return (spell);
}

const char* spellid (int id)
{
    switch (id) {
	case S_MON_DET:
	    return ("monster detection");
	case S_OBJ_DET:
	    return ("object detection");
	case S_IDENTIFY:
	    return ("identification");
	case S_FIREBOLT:
	    return ("firebolt");
	case S_LBALL:
	    return ("ball lightning");
	case S_SLEEP:
	    return ("sleep");
	case S_DISRUPT:
	    return ("disrupt");
	case S_DISINTEGRATE:
	    return ("disintegrate");
	case S_TELEPORT:
	    return ("teleport");
	case S_MISSILE:
	    return ("magic missile");
	case S_HEAL:
	    return ("healing");
	case S_DISPEL:
	    return ("dispelling");
	case S_BREATHE:
	    return ("breathing");
	case S_INVISIBLE:
	    return ("invisibility");
	case S_WARP:
	    return ("the warp");
	case S_ENCHANT:
	    return ("enchantment");
	case S_BLESS:
	    return ("blessing");
	case S_RESTORE:
	    return ("restoration");
	case S_CURE:
	    return ("curing");
	case S_TRUESIGHT:
	    return ("true sight");
	case S_HELLFIRE:
	    return ("hellfire");
	case S_KNOWLEDGE:
	    return ("self knowledge");
	case S_HERO:
	    return ("heroism");
	case S_RETURN:
	    return ("return");
	case S_DESECRATE:
	    return ("desecration");
	case S_HASTE:
	    return ("haste");
	case S_SUMMON:
	    return ("summoning");
	case S_SANCTUARY:
	    return ("sanctuary");
	case S_ACCURACY:
	    return ("accuracy");
	case S_RITUAL:
	    return ("ritual magic");
	case S_APPORT:
	    return ("apportation");
	case S_SHADOWFORM:
	    return ("shadow form");
	case S_ALERT:
	    return ("alertness");
	case S_REGENERATE:
	    return ("regeneration");
	case S_SANCTIFY:
	    return ("sanctification");
	case S_CLAIRVOYANCE:
	    return ("clairvoyance");
	case S_DRAIN:
	    return ("energy drain");
	case S_LEVITATE:
	    return ("levitate");
	case S_POLYMORPH:
	    return ("polymorph");
	case S_FEAR:
	    return ("fear");
	case S_WISH:
	    return ("wishing");
	default:
	    return ("???");
    }
}

void cast_spell (int spell)
{
    switch (spell) {
	case S_MON_DET:
	    s_mondet();
	    break;
	case S_OBJ_DET:
	    s_objdet();
	    break;
	case S_IDENTIFY:
	    s_identify();
	    break;
	case S_FIREBOLT:
	    s_firebolt();
	    break;
	case S_SLEEP:
	    s_sleep();
	    break;
	case S_LBALL:
	    s_lball();
	    break;
	case S_TELEPORT:
	    s_teleport();
	    break;
	case S_DISRUPT:
	    s_disrupt();
	    break;
	case S_DISINTEGRATE:
	    s_disintegrate();
	    break;
	case S_MISSILE:
	    s_missile();
	    break;
	case S_HEAL:
	    s_heal();
	    break;
	case S_DISPEL:
	    s_dispel();
	    break;
	case S_BREATHE:
	    s_breathe();
	    break;
	case S_INVISIBLE:
	    s_invisible();
	    break;
	case S_WARP:
	    s_warp();
	    break;
	case S_ENCHANT:
	    s_enchant();
	    break;
	case S_BLESS:
	    s_bless();
	    break;
	case S_RESTORE:
	    s_restore();
	    break;
	case S_CURE:
	    s_cure();
	    break;
	case S_TRUESIGHT:
	    s_truesight();
	    break;
	case S_HELLFIRE:
	    s_hellfire();
	    break;
	case S_KNOWLEDGE:
	    s_knowledge();
	    break;
	case S_HERO:
	    s_hero();
	    break;
	case S_RETURN:
	    s_return();
	    break;
	case S_DESECRATE:
	    s_desecrate();
	    break;
	case S_HASTE:
	    s_haste();
	    break;
	case S_SUMMON:
	    s_summon();
	    break;
	case S_SANCTUARY:
	    s_sanctuary();
	    break;
	case S_ACCURACY:
	    s_accuracy();
	    break;
	case S_RITUAL:
	    s_ritual();
	    break;
	case S_APPORT:
	    s_apport();
	    break;
	case S_SHADOWFORM:
	    s_shadowform();
	    break;
	case S_ALERT:
	    s_alert();
	    break;
	case S_REGENERATE:
	    s_regenerate();
	    break;
	case S_SANCTIFY:
	    s_sanctify();
	    break;
	case S_CLAIRVOYANCE:
	    s_clairvoyance();
	    break;
	case S_DRAIN:
	    s_drain();
	    break;
	case S_LEVITATE:
	    s_levitate();
	    break;
	case S_FEAR:
	    s_fear();
	    break;
	case S_POLYMORPH:
	    s_polymorph();
	    break;
	case S_WISH:
	    s_wish();
	    break;
	default:
	    mprint ("Your odd spell fizzles with a small 'sput'.");
	    break;
    }
}

static const char* spell_names[] = {	// alphabetical listing
    "accuracy", "alertness", "apportation", "ball lightning", "blessing",
    "breathing", "clairvoyance", "curing", "desecration", "disintegrate",
    "dispelling", "disrupt", "enchantment", "energy drain", "fear", "firebolt",
    "haste", "healing", "hellfire", "heroism", "identification", "invisibility",
    "levitate", "magic missile", "monster detection", "object detection",
    "polymorph", "regeneration", "restoration", "return", "ritual magic",
    "sanctification", "sanctuary", "self knowledge", "shadow form", "sleep",
    "summoning", "teleport", "the warp", "true sight", "wishing"
};

static const ESpell spell_ids[] = {	// in the same order as spell_names[]
    S_ACCURACY, S_ALERT, S_APPORT, S_LBALL, S_BLESS, S_BREATHE, S_CLAIRVOYANCE,
    S_CURE, S_DESECRATE, S_DISINTEGRATE, S_DISPEL, S_DISRUPT, S_ENCHANT, S_DRAIN,
    S_FEAR, S_FIREBOLT, S_HASTE, S_HEAL, S_HELLFIRE, S_HERO, S_IDENTIFY,
    S_INVISIBLE, S_LEVITATE, S_MISSILE, S_MON_DET, S_OBJ_DET, S_POLYMORPH,
    S_REGENERATE, S_RESTORE, S_RETURN, S_RITUAL, S_SANCTIFY, S_SANCTUARY,
    S_KNOWLEDGE, S_SHADOWFORM, S_SLEEP, S_SUMMON, S_TELEPORT, S_WARP, S_TRUESIGHT,
    S_WISH
};

static void showknownspells (int first, int last)
{
    int i, printed = FALSE;

    menuclear();
    menuprint ("\nPossible Spells:\n");
    for (i = first; i <= last; i++)
	if (spell_is_known (spell_ids[i])) {
	    printed = TRUE;
	    menuprint (spell_names[i]);
	    menuprint (" (");
	    menunumprint (Spells[spell_ids[i]].powerdrain);
	    menuprint (" mana)");
	    menuprint ("\n");
	}
    if (!printed)
	menuprint ("\nNo spells match that prefix!");
    showmenu();
}

static int spellparse (void)
{
    int first, last, pos;
    char byte, prefix[80];
    int found = 0;
    int f, l;

    first = 0;
    while (first < NUMSPELLS && !spell_is_known (spell_ids[first]))
	first++;
    if (first == NUMSPELLS) {
	print1 ("You don't know any spells!");
	return ABORT;
    }
    last = NUMSPELLS - 1;
    pos = 0;
    print2 ("");
    do {
	byte = mgetc();
	if (byte == KEY_BACKSPACE) {
	    if (pos > 0) {
		prefix[--pos] = '\0';
		byte = prefix[pos - 1];
		f = first;
		while (f >= 0 && !strncmp (prefix, spell_names[f], pos)) {
		    if (spell_is_known (spell_ids[f]))
			first = f;
		    f--;
		}
		l = last;
		while (l < NUMSPELLS && !strncmp (prefix, spell_names[l], pos)) {
		    if (spell_is_known (spell_ids[l]))
			last = l;
		    l++;
		}
		if (found)
		    found = 0;
		print2 (prefix);
	    }
	    if (pos == 0) {
		first = 0;
		last = NUMSPELLS - 1;
		found = 0;
		print2 ("");
	    }
	} else if (byte == KEY_ESCAPE) {
	    xredraw();
	    return ABORT;
	} else if (byte == '?')
	    showknownspells (first, last);
	else if (byte != '\n') {
	    if (byte >= 'A' && byte <= 'Z')
		byte += 'a' - 'A';
	    if (found)
		continue;
	    f = first;
	    l = last;
	    while (f < NUMSPELLS && (!spell_is_known(spell_ids[f]) || (int) strlen (spell_names[f]) < pos || spell_names[f][pos] < byte))
		f++;
	    while (l >= 0 && (!spell_is_known(spell_ids[l]) || (int) strlen (spell_names[l]) < pos || spell_names[l][pos] > byte))
		l--;
	    if (l < f)
		continue;
	    prefix[pos++] = byte;
	    prefix[pos] = '\0';
	    nprint2 (prefix + pos - 1);
	    first = f;
	    last = l;
	    if (first == last && !found) {	// unique name
		found = 1;
		nprint2 (spell_names[first] + pos);
	    }
	}
    } while (byte != '\n');
    xredraw();
    if (found)
	return spell_ids[first];
    else {
	print3 ("That is an ambiguous abbreviation!");
	return ABORT;
    }
}
