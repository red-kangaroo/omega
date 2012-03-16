#include "glob.h"

/*               Revised function                   */
/* WDT: code contributed by David J. Robertson */
/* consider one monster's action */
void m_pulse (struct monster *m)
{
    int range = distance (m->x, m->y, Player.x, Player.y);
    int STRIKE = FALSE;
    pol prev;

    if (Time % 10 == 0)
	if (m->hp < Monsters[m->id].hp)
	    m->hp++;

    if ((!m_statusp (m, AWAKE)) && (range <= m->wakeup)) {
	m_status_set (m, AWAKE);
	resetgamestatus (FAST_MOVE);
    }

    if (m_statusp (m, AWAKE)) {
	if (m_statusp (m, WANDERING)) {
	    if (m_statusp (m, MOBILE))
		m_random_move (m);
	    if (range <= m->sense && (m_statusp (m, HOSTILE) || m_statusp (m, NEEDY)))
		m_status_reset (m, WANDERING);
	} else {		/* not wandering */

	    if (m_statusp (m, HOSTILE))
		if ((range > 2) && (range < m->sense) && (random_range (2) == 1))
		    if (los_p (m->x, m->y, Player.x, Player.y) && (Player.status[INVISIBLE] == 0)) {
			STRIKE = TRUE;
			monster_strike (m);
		    }

	    if ((m_statusp (m, HOSTILE) || m_statusp (m, NEEDY))
		&& (range > 1) && m_statusp (m, MOBILE) && (!STRIKE || (random_range (2) == 1)))
		monster_move (m);
	    else if (m_statusp (m, HOSTILE) && (range == 1)) {
		resetgamestatus (FAST_MOVE);
		tacmonster (m);
	    }
	}
	/* if monster is greedy, picks up treasure it finds */
	if (m_statusp (m, GREEDY) && (m->hp > 0))
	    while (Level->site[m->x][m->y].things != NULL) {
		m_pickup (m, Level->site[m->x][m->y].things->thing);
		prev = Level->site[m->x][m->y].things;
		Level->site[m->x][m->y].things = Level->site[m->x][m->y].things->next;
		free ((char *) prev);
	    }
	/* prevents monsters from casting spells from other side of dungeon */
	if ((range < max (5, m->level)) && (m->hp > 0) && (random_range (2) == 1))
	    monster_special (m);
    }
}

/* like m_normal_move, but can open doors */
void m_smart_move (struct monster *m)
{
    m_simple_move (m);
}

/* not very smart, but not altogether stupid movement */
void m_normal_move (struct monster *m)
{
    m_simple_move (m);
}

/* used by both m_normal_move and m_smart_move */
void m_simple_move (struct monster *m)
{
    int dx = sign (Player.x - m->x);
    int dy = sign (Player.y - m->y);

    erase_monster (m);
    if (m->hp < Monsters[m->id].hp / 4) {
	dx = -dx;
	dy = -dy;
	m->movef = M_MOVE_SCAREDY;
	if (m->uniqueness == COMMON) {
	    strcpy (Str2, "The ");
	    strcat (Str2, m->monstring);
	} else
	    strcpy (Str2, m->monstring);
	if (m->possessions != NULL) {
	    strcat (Str2, " drops its treasure and flees!");
	    m_dropstuff (m);
	} else
	    strcat (Str2, " flees!");
	mprint (Str2);
	m->speed = min (2, m->speed - 1);
    }
    if ((!m_statusp (m, HOSTILE) && !m_statusp (m, NEEDY)) || (Player.status[INVISIBLE] > 0))
	m_random_move (m);
    else {
	if (m_unblocked (m, m->x + dx, m->y + dy))
	    movemonster (m, m->x + dx, m->y + dy);
	else if (dx == 0) {
	    if (m_unblocked (m, m->x + 1, m->y + dy))
		movemonster (m, m->x + 1, m->y + dy);
	    else if (m_unblocked (m, m->x - 1, m->y + dy))
		movemonster (m, m->x - 1, m->y + dy);
	}

	else if (dy == 0) {
	    if (m_unblocked (m, m->x + dx, m->y + 1))
		movemonster (m, m->x + dx, m->y + 1);
	    else if (m_unblocked (m, m->x + dx, m->y - 1))
		movemonster (m, m->x + dx, m->y - 1);
	}

	else {
	    if (m_unblocked (m, m->x + dx, m->y))
		movemonster (m, m->x + dx, m->y);
	    else if (m_unblocked (m, m->x, m->y + dy))
		movemonster (m, m->x, m->y + dy);
	}
    }
}

void m_move_animal (struct monster *m)
{
    if (m_statusp (m, HOSTILE))
	m_normal_move (m);
    else
	m_scaredy_move (m);
}

/* same as simple move except run in opposite direction */
void m_scaredy_move (struct monster *m)
{
    int dx = -sign (Player.x - m->x);
    int dy = -sign (Player.y - m->y);
    erase_monster (m);
    if (Player.status[INVISIBLE])
	m_random_move (m);
    else {
	if (m_unblocked (m, m->x + dx, m->y + dy))
	    movemonster (m, m->x + dx, m->y + dy);
	else if (dx == 0) {
	    if (m_unblocked (m, m->x + 1, m->y + dy))
		movemonster (m, m->x + 1, m->y + dy);
	    else if (m_unblocked (m, m->x - 1, m->y + dy))
		movemonster (m, m->x - 1, m->y + dy);
	}

	else if (dy == 0) {
	    if (m_unblocked (m, m->x + dx, m->y + 1))
		movemonster (m, m->x + dx, m->y + 1);
	    else if (m_unblocked (m, m->x + dx, m->y - 1))
		movemonster (m, m->x + dx, m->y - 1);
	}

	else {
	    if (m_unblocked (m, m->x + dx, m->y))
		movemonster (m, m->x + dx, m->y);
	    else if (m_unblocked (m, m->x, m->y + dy))
		movemonster (m, m->x, m->y + dy);
	}
    }
}

/* for spirits (and earth creatures) who can ignore blockages because
   either they are noncorporeal or they can move through stone */
void m_spirit_move (struct monster *m)
{
    int dx = sign (Player.x - m->x);
    int dy = sign (Player.y - m->y);
    erase_monster (m);
    if (m->hp < Monsters[m->id].hp / 6) {
	dx = -dx;
	dy = -dy;
    }

    if (Player.status[INVISIBLE] > 0 || !m_unblocked (m, m->x + dx, m->y + dy))
	m_random_move (m);
    else
	movemonster (m, m->x + dx, m->y + dy);
}

/* fluttery dumb movement */
void m_flutter_move (struct monster *m)
{
    int trange, range = distance (m->x, m->y, Player.x, Player.y);
    int i, tx, ty, nx = m->x, ny = m->y;
    erase_monster (m);
    if (Player.status[INVISIBLE] > 0)
	m_random_move (m);
    else {
	for (i = 0; i < 8; i++) {
	    tx = m->x + Dirs[0][i];
	    ty = m->y + Dirs[1][i];
	    trange = distance (tx, ty, Player.x, Player.y);
	    if (m->hp < Monsters[m->id].hp / 6) {
		if ((trange > range) && m_unblocked (m, tx, ty)) {
		    range = trange;
		    nx = tx;
		    ny = ty;
		}
	    } else if ((trange <= range) && m_unblocked (m, tx, ty)) {
		range = trange;
		nx = tx;
		ny = ty;
	    }
	}
	movemonster (m, nx, ny);
    }
}

void m_follow_move (struct monster *m)
{
    if (!m_statusp (m, HOSTILE))
	m_normal_move (m);
    else
	m_scaredy_move (m);
}

/* allows monsters to fall into pools, revealed traps, etc */
void m_confused_move (struct monster *m)
{
    int i, nx, ny, done = FALSE;
    erase_monster (m);
    for (i = 0; ((i < 8) && (!done)); i++) {
	nx = m->x + random_range (3) - 1;
	ny = m->y + random_range (3) - 1;
	if (unblocked (nx, ny) && ((nx != Player.x) || (ny != Player.y))) {
	    done = TRUE;
	    movemonster (m, nx, ny);
	}
    }
}

void m_random_move (struct monster *m)
{
    int i, nx, ny, done = FALSE;
    erase_monster (m);
    for (i = 0; ((i < 8) && (!done)); i++) {
	nx = m->x + random_range (3) - 1;
	ny = m->y + random_range (3) - 1;
	if (m_unblocked (m, nx, ny) && ((nx != Player.x) || (ny != Player.y))) {
	    done = TRUE;
	    movemonster (m, nx, ny);
	}
    }
}

/* monster removed from play */
void m_vanish (struct monster *m)
{
    if (m->uniqueness == COMMON) {
	strcpy (Str2, "The ");
	strcat (Str2, m->monstring);
    } else
	strcpy (Str2, m->monstring);
    strcat (Str2, " vanishes in the twinkling of an eye!");
    mprint (Str2);
    Level->site[m->x][m->y].creature = NULL;
    erase_monster (m);
    m->hp = -1;			/* signals "death" -- no credit to player, though */
}

/* monster still in play */
void m_teleport (struct monster *m)
{
    erase_monster (m);
    if (m_statusp (m, AWAKE)) {
	Level->site[m->x][m->y].creature = NULL;
	putspot (m->x, m->y, getspot (m->x, m->y, FALSE));
	findspace (&(m->x), &(m->y), -1);
	Level->site[m->x][m->y].creature = m;
    }
}

void m_move_leash (struct monster *m)
{
    m_simple_move (m);
    if (m->aux1 == 0) {
	m->aux1 = m->x;
	m->aux2 = m->y;
    } else if (distance (m->x, m->y, m->aux1, m->aux2) > 5) {
	if (Level->site[m->aux1][m->aux2].creature != NULL) {
	    if (los_p (Player.x, Player.y, m->aux1, m->aux2)) {
		/* some other monster is where the chain starts */
		if (Level->site[m->aux1][m->aux2].creature->uniqueness == COMMON) {
		    strcpy (Str1, "The ");
		    strcat (Str1, Level->site[m->aux1][m->aux2].creature->monstring);
		} else
		    strcpy (Str1, Level->site[m->aux1][m->aux2].creature->monstring);
		strcat (Str1, " releases the dog's chain!");
		mprint (Str1);
	    }
	    m->movef = M_MOVE_NORMAL;
	    /* otherwise, we'd lose either the dog or the other monster. */
	} else if (los_p (Player.x, Player.y, m->x, m->y)) {
	    mprint ("You see the dog jerked back by its chain!");
	    plotspot (m->x, m->y, FALSE);
	} else
	    mprint ("You hear a strangled sort of yelp!");
	Level->site[m->x][m->y].creature = NULL;
	m->x = m->aux1;
	m->y = m->aux2;
	Level->site[m->x][m->y].creature = m;
    }
}

/* actually make a move */
void movemonster (struct monster *m, int newx, int newy)
{
    if (Level->site[newx][newy].creature != NULL)
	return;
    if (Level->site[m->x][m->y].creature == m)
	Level->site[m->x][m->y].creature = NULL;
    m->x = newx;
    m->y = newy;
    Level->site[m->x][m->y].creature = m;
    m_movefunction (m, Level->site[m->x][m->y].p_locf);
}

/* The druid's altar is in the northern forest */
void m_talk_druid (struct monster *m)
{
    int i;
    pml curr;

    if (!m_statusp (m, HOSTILE)) {
	print1 ("The Archdruid raises a hand in greeting.");
	if (!gamestatusp (SPOKE_TO_DRUID)) {
	    setgamestatus (SPOKE_TO_DRUID);
	    morewait ();
	    print1 ("The Archdruid congratulates you on reaching his sanctum.");
	    print2 ("You feel competent.");
	    morewait ();
	    gain_experience (300);
	    if (Player.patron == DRUID) {
		print1 ("The Archdruid conveys to you the wisdom of nature....");
		print2 ("You feel like a sage.");
		morewait ();
		for (i = 0; i < NUMRANKS; i++) {
		    if (Player.guildxp[i] > 0)
			Player.guildxp[i] += 300;
		}
	    }
	}
	mprint ("Do you request a ritual of neutralization? [yn] ");
	if (ynq () == 'y') {
	    if (Phase / 2 == 6 || Phase / 2 == 0) {	/* full or new moon */
		mprint ("\"Unfortunately, I cannot perform a ritual of balance on");
		if (Phase / 2 == 6)
		    mprint ("this lawful day.\"");
		else
		    mprint ("this chaotic day.\"");
	    } else if (Phase / 2 == 3 || Phase / 2 == 9) {	/* half moon */
		mprint ("You take part in today's holy celebration of balance...");
		Player.alignment = 0;
		Player.mana = calcmana ();
		if (Player.patron == DRUID)
		    gain_experience (200);	/* if a druid wants to spend 2 days */
		Time += 60;	/* celebrating for 1600 xp, why not? */
		hourly_check ();
		Time += 60;
		hourly_check ();
		Time += 60;
		hourly_check ();
		Time += 60;
		hourly_check ();
		Time += 60;
		hourly_check ();
		Time += 60;
		hourly_check ();
	    } else {
		mprint ("The ArchDruid conducts a sacred rite of balance...");
		if (Player.patron == DRUID) {
		    Player.alignment = 0;
		    Player.mana = calcmana ();
		} else
		    Player.alignment -= Player.alignment * max (0, 10 - Player.level) / 10;
		/* the higher level the character is, the more set in his/her ways */
		Time += 60;
		hourly_check ();
	    }
	    dataprint ();
	}
    } else {
	mprint ("The ArchDruid looks at you and cries: 'Unclean! Unclean!'");
	disrupt (Player.x, Player.y, 100);
	mprint ("This seems to have satiated his desire for vengeance.");
	mprint ("'Have you learned your lesson?' The ArchDruid asks. [yn] ");
	if (ynq ()) {
	    mprint ("'I certainly hope so!' says the ArchDruid.");
	    for (curr = Level->mlist; curr; curr = curr->next)
		m_status_reset (curr->m, HOSTILE);
	    m_vanish (m);
	} else {
	    mprint ("'Idiot.' mutters the ArchDruid.");
	    p_damage (500, UNSTOPPABLE, "the ArchDruid's Vengeance");
	}
    }
}

void m_talk_silent (struct monster *m)
{

    int reply = random_range (4);

    if (m->uniqueness == COMMON) {
	strcpy (Str2, "The ");
	strcat (Str2, m->monstring);
    } else
	strcpy (Str2, m->monstring);
    switch (reply) {
	case 0:
	    strcat (Str2, " does not reply. ");
	    break;
	case 1:
	    strcat (Str2, " shrugs silently. ");
	    break;
	case 2:
	    strcat (Str2, " hold a finger to his mouth. ");
	    break;
	case 3:
	    strcat (Str2, " glares at you but says nothing. ");
	    break;
    }
    mprint (Str2);
}

void m_talk_stupid (struct monster *m)
{

    int reply = random_range (4);
    if (m->uniqueness == COMMON) {
	strcpy (Str2, "The ");
	strcat (Str2, m->monstring);
    } else
	strcpy (Str2, m->monstring);
    switch (reply) {
	case 0:
	    strcat (Str2, " looks at you with mute incomprehension.");
	    break;
	case 1:
	    strcat (Str2, " growls menacingly and ignores you.");
	    break;
	case 2:
	    strcat (Str2, " does not seem to have heard you.");
	    break;
	case 3:
	    strcat (Str2, " tries to pretend it didn't hear you.");
	    break;
    }
    mprint (Str2);
}

void m_talk_greedy (struct monster *m)
{

    int reply = random_range (4);
    if (m->uniqueness == COMMON) {
	strcpy (Str2, "The ");
	strcat (Str2, m->monstring);
    } else
	strcpy (Str2, m->monstring);
    switch (reply) {
	case 0:
	    strcat (Str2, " says: Give me a treasure.... ");
	    break;
	case 1:
	    strcat (Str2, " says: Stand and deliver, knave! ");
	    break;
	case 2:
	    strcat (Str2, " says: Your money or your life! ");
	    break;
	case 3:
	    strcat (Str2, " says: Yield or Die! ");
	    break;
    }
    mprint (Str2);
}

void m_talk_hungry (struct monster *m)
{

    int reply = random_range (4);
    if (m->uniqueness == COMMON) {
	strcpy (Str2, "The ");
	strcat (Str2, m->monstring);
    } else
	strcpy (Str2, m->monstring);
    switch (reply) {
	case 0:
	    strcat (Str2, " says: I hunger, foolish adventurer! ");
	    break;
	case 1:
	    strcat (Str2, " drools menacingly at you. ");
	    break;
	case 2:
	    strcat (Str2, " says: You're invited to be lunch! ");
	    break;
	case 3:
	    strcat (Str2, " says: Feeed Meee! ");
	    break;
    }
    mprint (Str2);
}

void m_talk_guard (struct monster *m)
{
    if (m_statusp (m, HOSTILE)) {
	print1 ("'Surrender in the name of the Law!'");
	print2 ("Do it? [yn] ");
	if (ynq2 () == 'y') {
	    Player.alignment++;
	    if (Current_Environment == E_CITY) {
		print1 ("Go directly to jail. Do not pass go, do not collect 200Au.");
		print2 ("You are taken to the city gaol.");
		morewait ();
		send_to_jail ();
		drawvision (Player.x, Player.y);
	    } else {
		clearmsg ();
		print1 ("Mollified, the guard disarms you and sends you away.");
		dispose_lost_objects (1, Player.possessions[O_WEAPON_HAND]);
		pacify_guards ();
	    }
	} else {
	    clearmsg ();
	    print1 ("All right, you criminal scum, you asked for it!");
	}
    } else if (Player.rank[ORDER] > 0)
	print1 ("'Greetings comrade! May you always tread the paths of Law.'");
    else
	print1 ("Move it right along, stranger!");
}

void m_talk_mp (struct monster *m UNUSED)
{
    mprint ("The mendicant priest asks you to spare some treasure for the needy");
}

void m_talk_titter (struct monster *m)
{

    if (m->uniqueness == COMMON) {
	strcpy (Str2, "The ");
	strcat (Str2, m->monstring);
    } else
	strcpy (Str2, m->monstring);
    strcat (Str2, " titters obscenely at you.");
    mprint (Str2);
}

void m_talk_ninja (struct monster *m UNUSED)
{
    mprint ("The black-garbed figure says apologetically:");
    mprint ("'Situree simasita, wakarimasen.'");
}

void m_talk_thief (struct monster *m)
{
    if (Player.rank[THIEVES]) {
	if (m->level == 2)
	    m->monstring = "sneak thief";
	else
	    m->monstring = "master thief";
	print1 ("The cloaked figure makes a gesture which you recognize...");
	print2 ("...the thieves' guild recognition signal!");
	print3 ("'Sorry, mate, thought you were a mark....'");
	morewait ();
	m_vanish (m);
    } else
	m_talk_man (m);

}

void m_talk_assassin (struct monster *m)
{
    m->monstring = "master assassin";
    print1 ("The ominous figure does not reply, but hands you an embossed card:");
    print2 ("'Guild of Assassins Ops are forbidden to converse with targets.'");
}

void m_talk_im (struct monster *m)
{
    if (strcmp (m->monstring, "itinerant merchant") != 0) {
	m->monstring = "itinerant merchant";
    }
    if (m->possessions == NULL)
	mprint ("The merchant says: Alas! I have nothing to sell!");
    else {
	m->possessions->thing->known = 2;
	clearmsg ();
	mprint ("I have a fine");
	mprint (itemid (m->possessions->thing));
	mprint ("for only");
	mlongprint (max (10, 4 * true_item_value (m->possessions->thing)));
	mprint ("Au.");
	mprint ("Want it? [yn] ");
	if (ynq () == 'y') {
	    if (Player.cash < (max (10, 4 * true_item_value (m->possessions->thing)))) {
		if (Player.alignment > 10) {
		    mprint ("Well, I'll let you have it for what you've got.");
		    Player.cash = 0;
		    gain_item (m->possessions->thing);
		    m->possessions = NULL;
		} else
		    mprint ("Beat it, you deadbeat!");
	    } else {
		mprint ("Here you are. Have a good day.");
		Player.cash -= max (10, (4 * item_value (m->possessions->thing)));
		gain_item (m->possessions->thing);
		m->possessions = NULL;
	    }
	} else
	    mprint ("Well then, I must be off. Good day.");
	m_vanish (m);
    }
}

void m_talk_man (struct monster *m)
{

    if (m->uniqueness == COMMON) {
	strcpy (Str2, "The ");
	strcat (Str2, m->monstring);
    } else
	strcpy (Str2, m->monstring);
    switch (random_range (5)) {
	case 0:
	    strcat (Str2, " asks you for the way home.");
	    break;
	case 1:
	    strcat (Str2, " wishes you a pleasant day.");
	    break;
	case 2:
	    strcat (Str2, " sneers at you contemptuously.");
	    break;
	case 3:
	    strcat (Str2, " smiles and nods.");
	    break;
	case 4:
	    strcat (Str2, " tells you a joke.");
	    break;
    }
    mprint (Str2);
}

void m_talk_evil (struct monster *m)
{

    if (m->uniqueness == COMMON) {
	strcpy (Str2, "The ");
	strcat (Str2, m->monstring);
    } else
	strcpy (Str2, m->monstring);
    switch (random_range (14)) {
	case 0:
	    strcat (Str2, " says: 'THERE CAN BE ONLY ONE!'");
	    break;
	case 1:
	    strcat (Str2, " says: 'Prepare to die, Buckwheat!'");
	    break;
	case 2:
	    strcat (Str2, " says: 'Time to die!'");
	    break;
	case 3:
	    strcat (Str2, " says: 'There will be no mercy.'");
	    break;
	case 4:
	    strcat (Str2, " insults your mother-in-law.");
	    break;
	case 5:
	    strcat (Str2, " says: 'Kurav tu ando mul!'");
	case 6:
	    strcat (Str2, " says: '!va al infierno!'");
	    break;
	case 7:
	    strcat (Str2, " says: 'dame desu, nee.'");
	    break;
	case 8:
	    strcat (Str2, " spits on your rug and calls your cat a bastard.");
	    break;
	case 9:
	    strcat (Str2, " snickers malevolently and draws a weapon.");
	    break;
	case 10:
	    strcat (Str2, " sends 'rm -r *' to your shell!");
	    break;
	case 11:
	    strcat (Str2, " tweaks your nose and cackles evilly.");
	    break;
	case 12:
	    strcat (Str2, " thumbs you in the eyes.");
	    break;
	case 13:
	    strcat (Str2, " kicks you in the groin.");
	    break;
    }
    mprint (Str2);
}

void m_talk_robot (struct monster *m)
{
    if (m->uniqueness == COMMON) {
	strcpy (Str2, "The ");
	strcat (Str2, m->monstring);
    } else
	strcpy (Str2, m->monstring);
    switch (random_range (4)) {
	case 0:
	    strcat (Str2, " says: 'exterminate...Exterminate...EXTERMINATE!!!'");
	    break;
	case 1:
	    strcat (Str2, " says: 'Kill ... Crush ... Destroy'");
	    break;
	case 2:
	    strcat (Str2, " says: 'Danger -- Danger'");
	    break;
	case 3:
	    strcat (Str2, " says: 'Yo Mama -- core dumped.'");
	    break;
    }
    mprint (Str2);
}

void m_talk_slithy (struct monster *m UNUSED)
{
    mprint ("It can't talk -- it's too slithy!");
}

void m_talk_mimsy (struct monster *m UNUSED)
{
    mprint ("It can't talk -- it's too mimsy!");
}

void m_talk_burble (struct monster *m)
{

    if (m->uniqueness == COMMON) {
	strcpy (Str2, "The ");
	strcat (Str2, m->monstring);
    } else
	strcpy (Str2, m->monstring);
    strcat (Str2, " burbles hatefully at you.");
    mprint (Str2);
}

void m_talk_beg (struct monster *m)
{
    if (m->uniqueness == COMMON) {
	strcpy (Str2, "The ");
	strcat (Str2, m->monstring);
    } else
	strcpy (Str2, m->monstring);
    strcat (Str2, " asks you for alms.");
    mprint (Str2);
}

void m_talk_hint (struct monster *m)
{
    if (m->uniqueness == COMMON) {
	strcpy (Str2, "The ");
	strcat (Str2, m->monstring);
    } else
	strcpy (Str2, m->monstring);
    if (m_statusp (m, HOSTILE)) {
	strcat (Str2, " only sneers at you. ");
	mprint (Str2);
    } else {
	strcat (Str2, " whispers in your ear: ");
	mprint (Str2);
	hint ();
	m->talkf = M_TALK_SILENT;
    }
}

void m_talk_gf (struct monster *m)
{
    mprint ("The good fairy glints: Would you like a wish?");
    if (ynq () == 'y') {
	mprint ("The good fairy glows: Are you sure?");
	if (ynq () == 'y') {
	    mprint ("The good fairy radiates: Really really sure?");
	    if (ynq () == 'y') {
		mprint ("The good fairy beams: I mean, like, sure as sure can be?");
		if (ynq () == 'y') {
		    mprint ("The good fairy dazzles: You don't want a wish, right?");
		    if (ynq () == 'y')
			mprint ("The good fairy laughs: I thought not.");
		    else
			wish (0);
		}
	    }
	}
    }
    mprint ("In a flash of sweet-smelling light, the fairy vanishes....");
    Player.hp = max (Player.hp, Player.maxhp);
    Player.mana = max (Player.mana, calcmana ());
    mprint ("You feel mellow.");
    m_vanish (m);
}

void m_talk_ef (struct monster *m)
{
    mprint ("The evil fairy roils: Eat my pixie dust!");
    mprint ("She waves her black-glowing wand, which screams thinly....");
    m->movef = M_MOVE_SMART;
    m->meleef = M_MELEE_POISON;
    m->specialf = M_SP_THIEF;
    acquire (-1);
    bless (-1);
    sleep_player (m->level / 2);
    summon (-1, -1);
    summon (-1, -1);
    summon (-1, -1);
    summon (-1, -1);
}

void m_talk_seductor (struct monster *m)
{
    if (m->uniqueness == COMMON) {
	strcpy (Str2, "The ");
	strcat (Str2, m->monstring);
    } else
	strcpy (Str2, m->monstring);
    if (Player.preference == 'n') {
	strcat (Str2, " notices your disinterest and leaves with a pout.");
	mprint (Str2);
    } else {
	strcat (Str2, " beckons seductively...");
	mprint (Str2);
	mprint ("Flee? [yn] ");
	if (ynq () == 'y') {
	    mprint ("You feel stupid.");
	} else {
	    strcpy (Str2, "The ");
	    strcat (Str2, m->monstring);
	    strcat (Str2, " shows you a good time....");
	    mprint (Str2);
	    gain_experience (500);
	    Player.con++;
	}
    }
    m_vanish (m);
}

void m_talk_demonlover (struct monster *m)
{
    if (m->uniqueness == COMMON) {
	strcpy (Str2, "The ");
	strcat (Str2, m->monstring);
    } else
	strcpy (Str2, m->monstring);
    if (Player.preference == 'n') {
	strcat (Str2, " notices your disinterest and changes with a snarl...");
	mprint (Str2);
	morewait ();
    } else {
	strcat (Str2, " beckons seductively...");
	mprint (Str2);
	mprint ("Flee? [yn] ");
	if (ynq () == 'y')
	    mprint ("You feel fortunate....");
	else {
	    if (m->uniqueness == COMMON) {
		strcpy (Str2, "The ");
		strcat (Str2, m->monstring);
	    } else
		strcpy (Str2, m->monstring);
	    strcat (Str2, " shows you a good time....");
	    mprint (Str2);
	    morewait ();
	    mprint ("You feel your life energies draining...");
	    level_drain (random_range (3) + 1, "a demon's kiss");
	    morewait ();
	}
    }
    m->talkf = M_TALK_EVIL;
    m->meleef = M_MELEE_SPIRIT;
    m->specialf = M_SP_DEMON;

    if ((m->monchar & 0xff) == 's') {
	m->monchar = 'I' | CLR (RED);
	m->monstring = "incubus";
    } else {
	m->monchar = 'S' | CLR (RED);
	m->monstring = "succubus";
    }
    if (m->uniqueness == COMMON) {
	strcpy (Str2, "The ");
	strcat (Str2, m->monstring);
    } else
	strcpy (Str2, m->monstring);
    strcat (Str2, " laughs insanely.");
    mprint (Str2);
    mprint ("You now notice the fangs, claws, batwings...");
}

void m_talk_horse (struct monster *m)
{
    if (m_statusp (m, HOSTILE))
	mprint ("The horse neighs angrily at you.");
    else if (m_statusp (m, HUNGRY))
	mprint ("The horse noses curiously at your pack.");
    else if (gamestatusp (MOUNTED))
	mprint ("The horse and your steed don't seem to get along.");
    else if (Current_Environment == Current_Dungeon)
	mprint ("The horse shies; maybe he doesn't like the dungeon air....");
    else {
	mprint ("The horse lets you pat his nose. Want to ride him? [yn] ");
	if (ynq () == 'y') {
	    m->hp = -1;
	    Level->site[m->x][m->y].creature = NULL;
	    putspot (m->x, m->y, getspot (m->x, m->y, FALSE));
	    setgamestatus (MOUNTED);
	    calc_melee ();
	    mprint ("You are now equitating!");
	}
    }
}

void m_talk_hyena (struct monster *m UNUSED)
{
    mprint ("The hyena only laughs at you...");
}

void m_talk_parrot (struct monster *m UNUSED)
{
    mprint ("Polly wanna cracker?");
}

void m_talk_servant (struct monster *m)
{
    int target, x = Player.x, y = Player.y;
    if (m->id == SERV_LAW) {
	target = SERV_CHAOS;
	mprint ("The Servant of Law pauses in thought for a moment.");
	mprint ("You are asked: Are there any Servants of Chaos hereabouts? [yn] ");
    } else {
	target = SERV_LAW;
	mprint ("The Servant of Chaos grins mischievously at you.");
	mprint ("You are asked: Are there any Servants of Law hereabouts? [yn] ");
    }
    if (ynq () == 'y') {
	print1 ("Show me.");
	show_screen ();
	drawmonsters (TRUE);
	setspot (&x, &y);
	if (Level->site[x][y].creature != NULL) {
	    if (Level->site[x][y].creature->id == target) {
		mprint ("The Servant launches itself towards its enemy.");
		mprint ("In a blaze of combat, the Servants annihilate each other!");
		gain_experience (m->xpv);
		m_death (Level->site[x][y].creature);
		Level->site[m->x][m->y].creature = NULL;
		m->x = x;
		m->y = y;
		Level->site[x][y].creature = m;
		m_death (Level->site[x][y].creature);
	    } else
		mprint ("Right. Tell me about it. Idiot!");
	} else
	    mprint ("Right. Tell me about it. Idiot!");
    } else
	mprint ("The servant shrugs and turns away.");
}

void m_talk_animal (struct monster *m)
{
    if (m->uniqueness == COMMON) {
	strcpy (Str2, "The ");
	strcat (Str2, m->monstring);
    } else
	strcpy (Str2, m->monstring);
    mprint (Str2);
    mprint ("shows you a scholarly paper by Dolittle, D. Vet.");
    mprint ("which demonstrates that animals don't have speech centers");
    mprint ("complex enough to communicate in higher languages.");
    mprint ("It giggles softly to itself and takes back the paper.");
}

void m_talk_scream (struct monster *m)
{
    mprint ("A thinly echoing scream reaches your ears....");
    morewait ();
    mprint ("You feel doomed....");
    morewait ();
    mprint ("A bird appears and flies three times widdershins around your head.");
    summon (-1, QUAIL);
    m->talkf = M_TALK_EVIL;
}

void m_talk_archmage (struct monster *m)
{
    if (m_statusp (m, HOSTILE)) {
	mprint ("The Archmage ignores your attempt at conversation");
	mprint ("and concentrates on his spellcasting....");
    } else if (Current_Environment == E_COURT) {
	mprint ("The Archmage congratulates you on getting this far.");
	mprint ("He invites you to attempt the Throne of High Magic");
	mprint ("but warns you that it is important to wield the Sceptre");
	mprint ("before sitting on the throne.");
	if (Level->site[m->x][m->y].p_locf == L_THRONE) {
	    mprint ("The Archmage smiles and makes an arcane gesture....");
	    m_vanish (m);
	}
    } else {
	mprint ("The Archmage tells you to find him again in his");
	mprint ("Magical Court at the base of his castle in the mountains");
	mprint ("of the far North-East; if you do he will give you some");
	mprint ("important information.");
    }
}

void m_talk_merchant (struct monster *m)
{
    if (!m_statusp (m, HOSTILE)) {
	if (Current_Environment == E_VILLAGE) {
	    mprint ("The merchant asks you if you want to buy a horse for 250GP.");
	    mprint ("Pay the merchant? [yn] ");
	    if (ynq () == 'y') {
		if (Player.cash < 250)
		    mprint ("The merchant says: 'Come back when you've got the cash!'");
		else {
		    Player.cash -= 250;
		    mprint ("The merchant takes your money and tells you to select");
		    mprint ("any horse you want in the stables.");
		    mprint ("He says: 'You'll want to get to know him before trying to");
		    mprint ("ride him. By the way, food for the horse is not included.'");
		    mprint ("The merchant runs off toward the bank, cackling gleefully.");
		    m_vanish (m);
		}
	    } else
		mprint ("The merchant tells you to stop wasting his time.");
	} else {
	    mprint ("The merchant tells you to visit his stables at his village");
	    mprint ("for a great deal on a horse.");
	}
    } else {
	mprint ("The merchant ignores you and screams:");
	mprint ("'Help! Help! I'm being oppressed!'");
    }
}

void m_talk_prime (struct monster *m)
{
    if (!m_statusp (m, HOSTILE)) {
	if (Current_Environment == E_CIRCLE) {
	    print1 ("The Prime nods brusquely at you, removes a gem from his");
	    print2 ("sleeve, places it on the floor, and vanishes wordlessly.");
	    morewait ();
	    m_dropstuff (m);
	    m_vanish (m);
	} else {
	    print1 ("The Prime makes an intricate gesture, which leaves behind");
	    print2 ("glowing blue sparks... He winks mischievously at you....");
	    if (Player.rank[CIRCLE] > 0) {
		morewait ();
		print1 ("The blue sparks strike you! You feel enhanced!");
		print2 ("You feel more experienced....");
		Player.pow += Player.rank[CIRCLE];
		Player.mana += calcmana ();
		gain_experience (1000);
		m_vanish (m);
	    }
	}
    } else
	m_talk_evil (m);
}

/* give object o to monster m */
void m_pickup (struct monster *m, struct object *o)
{
    pol tmp = ((pol) checkmalloc (sizeof (oltype)));
    tmp->thing = o;
    tmp->next = m->possessions;
    m->possessions = tmp;
}

void m_dropstuff (struct monster *m)
{
    pol tmp = m->possessions;
    if (tmp != NULL) {
	while (tmp->next != NULL)
	    tmp = tmp->next;

	tmp->next = Level->site[m->x][m->y].things;
	Level->site[m->x][m->y].things = m->possessions;
	m->possessions = NULL;
    }
}

void m_firebolt (struct monster *m)
{
    fbolt (m->x, m->y, Player.x, Player.y, m->hit, m->dmg);
}

void m_nbolt (struct monster *m)
{
    nbolt (m->x, m->y, Player.x, Player.y, m->hit, m->dmg);
}

void m_lball (struct monster *m)
{
    lball (m->x, m->y, Player.x, Player.y, m->dmg);
}

void m_fireball (struct monster *m)
{
    fball (m->x, m->y, Player.x, Player.y, m->dmg);
}

void m_snowball (struct monster *m)
{
    snowball (m->x, m->y, Player.x, Player.y, m->dmg);
}

void m_blind_strike (struct monster *m)
{

    pml ml;
    if ((Player.status[BLINDED] == 0) && los_p (m->x, m->y, Player.x, Player.y) && (distance (m->x, m->y, Player.x, Player.y) < 5)) {
	if (m->uniqueness == COMMON) {
	    strcpy (Str2, "The ");
	    strcat (Str2, m->monstring);
	} else
	    strcpy (Str2, m->monstring);
	strcat (Str2, " gazes at you menacingly");
	mprint (Str2);
	if (!p_immune (GAZE)) {
	    mprint ("You've been blinded!");
	    Player.status[BLINDED] = random_range (4) + 1;
	    for (ml = Level->mlist; ml != NULL; ml = ml->next)
		plotspot (ml->m->x, ml->m->y, FALSE);
	} else
	    mprint ("You gaze steadily back....");
    }
}

void m_strike_sonic (struct monster *m)
{
    if (m->uniqueness == COMMON) {
	strcpy (Str2, "The ");
	strcat (Str2, m->monstring);
    } else
	strcpy (Str2, m->monstring);
    strcat (Str2, " screams at you!");
    mprint (Str2);
    p_damage (m->dmg, OTHER_MAGIC, "a sonic blast");
}


void m_sp_mp (struct monster *m)
{
    if (m->attacked && (random_range (3) == 1)) {
	mprint ("You feel cursed!");
	p_damage (10, UNSTOPPABLE, "a mendicant priest's curse");
	m_vanish (m);
    } else if (!m_statusp (m, NEEDY)) {
	mprint ("The mendicant priest makes a mystical gesture....");
	mprint ("You feel impressed...");
	Player.alignment += 5;
	if (Player.alignment > 20)
	    Player.hp = max (Player.hp, Player.maxhp);
	m_vanish (m);
    }
}

void m_sp_ng (struct monster *m)
{
    if (distance (m->x, m->y, Player.x, Player.y) < 2)
	if ((random_range (5) == 1) || (Player.status[VULNERABLE] > 0)) {
	    mprint ("The night gaunt grabs you and carries you off!");
	    mprint ("Its leathery wings flap and flap, and it giggles insanely.");
	    mprint ("It tickles you cunningly to render you incapable of escape.");
	    mprint ("Finally, it deposits you in a strange place.");
	    p_teleport (0);
	}
}

void m_sp_poison_cloud (struct monster *m)
{
    if (distance (m->x, m->y, Player.x, Player.y) < 3) {
	mprint ("A cloud of poison gas surrounds you!");
	if (Player.status[BREATHING] > 0)
	    mprint ("You can breathe freely, however.");
	else
	    p_poison (7);
    }
}

void m_sp_explode (struct monster *m)
{
    if ((distance (Player.x, Player.y, m->x, m->y) < 2) && (m->hp > 0) && (m->hp < Monsters[m->id].hp))
	fball (m->x, m->y, m->x, m->y, m->hp);
}

void m_sp_demon (struct monster *m)
{
    if (random_range (2)) {
	if ((m->id != INCUBUS) &&	/*succubi don't give fear */
	    los_p (m->x, m->y, Player.x, Player.y) && (random_range (30) > Player.level + 10) && (Player.status[AFRAID] == 0)) {
	    mprint ("You are stricken with fear!");
	    if (!p_immune (FEAR))
		Player.status[AFRAID] += m->level;
	    else
		mprint ("You master your reptile brain and stand fast.");
	} else
	    m_sp_spell (m);
    }
    if ((m->hp < (m->level * 5)) && (m->hp > 1)) {
	mprint ("The demon uses its waning lifeforce to summon help!");
	m->hp = 1;
	int mid = NIGHT_GAUNT;
	switch (m->level) {
	    case 4:
	    case 5:
		mid = L_FDEMON;
		break;		/* lesser frost demon */
	    case 6:
		mid = FROST_DEMON;
		break;
	    case 7:
		mid = OUTER_DEMON;
		break;		/* outer circle demon */
	    case 8:
		mid = DEMON_SERP;
		break;		/* demon serpent */
	    case 9:
		mid = INNER_DEMON;
		break;		/* inner circle demon */
	}
	summon (-1, mid);
	summon (-1, mid);
    }
}

void m_sp_acid_cloud (struct monster *m)
{
    if (m_statusp (m, HOSTILE) && (distance (m->x, m->y, Player.x, Player.y) < 3))
	acid_cloud ();
}

void m_sp_escape (struct monster *m)
{
    if (m_statusp (m, HOSTILE))
	m_vanish (m);
}

void m_sp_ghost (struct monster *m)
{
    if (m_statusp (m, HOSTILE)) {
	mprint ("The ghost moans horribly....");
	p_damage (1, FEAR, "a ghost-inspired heart attack");
	mprint ("You've been terrorized!");
	if (!p_immune (FEAR))
	    Player.status[AFRAID] += m->level;
	else
	    mprint ("You master your reptile brain and stand fast.");
    }
}

/* random spell cast by monster */
void m_sp_spell (struct monster *m)
{
    char action[80];
    if (m_statusp (m, HOSTILE) && los_p (Player.x, Player.y, m->x, m->y)) {
	if (m->uniqueness == COMMON)
	    strcpy (action, "The ");
	else
	    strcpy (action, "");
	strcat (action, m->monstring);
	strcat (action, " casts a spell...");
	mprint (action);
	if (!magic_resist (m->level))
	    switch (random_range (m->level + 7)) {
		case 0:
		    nbolt (m->x, m->y, Player.x, Player.y, m->hit, 10);
		    break;
		case 1:
		    mprint ("It seems stronger...");
		    m->hp += random_range (m->level * m->level);
		    break;
		case 2:
		    haste (-1);
		    break;
		case 3:
		    cure (-1);
		    break;
		case 4:
		    /* WDT: I'd like to make this (and "case 5" below) dependant on
		     * the monster's IQ in some way -- dumb but powerful monsters
		     * deserve what they get :).  No rush. */
		    if (m_immunityp (m, ELECTRICITY) || distance (m->x, m->y, Player.x, Player.y) > 2)
			lball (m->x, m->y, Player.x, Player.y, 20);
		    else
			lbolt (m->x, m->y, Player.x, Player.y, m->hit, 20);
		    break;
		case 5:
		    if (m_immunityp (m, COLD) || distance (m->x, m->y, Player.x, Player.y) > 2)
			snowball (m->x, m->y, Player.x, Player.y, 30);
		    else
			icebolt (m->x, m->y, Player.x, Player.y, m->hit, 30);
		    break;
		case 6:
		    enchant (-1);
		    break;
		case 7:
		    bless (0 - m->level);
		    break;
		case 8:
		    p_poison (m->level);
		    break;
		case 9:
		    sleep_player (m->level / 2);
		    break;
		case 10:
		    fbolt (m->x, m->y, Player.x, Player.y, m->hit * 3, 50);
		    break;
		case 11:
		    acquire (0 - m->level);
		    break;
		case 12:
		    dispel (-1);
		    break;
		case 13:
		    disrupt (Player.x, Player.y, 50);
		    break;
		case 14:
		    if (m->uniqueness == COMMON) {
			strcpy (Str2, "a ");
			strcat (Str2, m->monstring);
		    } else
			strcpy (Str2, m->monstring);
		    level_drain (m->level, Str2);
		    break;
		case 15:
		case 16:
		    disintegrate (Player.x, Player.y);
		    break;
	    }
    }
}

/* monsters with this have some way to hide, camouflage, etc until they 
   attack */
void m_sp_surprise (struct monster *m)
{
    if (m->attacked) {
	if (m_statusp (m, HOSTILE) && (!Player.status[TRUESIGHT]) && m_statusp (m, M_INVISIBLE)) {
	    m->monchar = Monsters[m->id].monchar;
	    if (!Player.status[ALERT]) {
		switch (random_range (4)) {
		    case 0:
			mprint ("You are surprised by a sudden treacherous attack!");
			break;
		    case 1:
			mprint ("You are shocked out of your reverie by the scream of battle!");
			break;
		    case 2:
			mprint ("Suddenly, from out of the shadows, a surprise attack!");
			break;
		    case 3:
			mprint ("A shriek of hatred causes you to momentarily freeze up!");
			break;
		}
		morewait ();
		setgamestatus (SKIP_PLAYER);
		m_status_reset (m, M_INVISIBLE);
	    } else {
		mprint ("You alertly sense the presence of an attacker!");
		m_status_reset (m, M_INVISIBLE);
	    }
	}
    }
}

void m_sp_whistleblower (struct monster *m)
{
    if (m_statusp (m, HOSTILE)) {
	alert_guards ();
	m->specialf = M_MELEE_NORMAL;
    }
}

void m_sp_seductor (struct monster *m)
{
    if (m_statusp (m, HOSTILE)) {
	if (m->uniqueness == COMMON) {
	    strcpy (Str2, "The ");
	    strcat (Str2, m->monstring);
	} else
	    strcpy (Str2, m->monstring);
	strcat (Str2, " runs away screaming for help....");
	mprint (Str2);
	m_vanish (m);
	summon (-1, -1);
	summon (-1, -1);
	summon (-1, -1);
    } else if (distance (Player.x, Player.y, m->x, m->y) < 2)
	m_talk_seductor (m);

}

void m_sp_demonlover (struct monster *m)
{
    if (distance (Player.x, Player.y, m->x, m->y) < 2)
	m_talk_demonlover (m);
}

void m_sp_eater (struct monster *m)
{
    if (Player.rank[COLLEGE])
	m_status_set (m, HOSTILE);
    if (m_statusp (m, HOSTILE))
	if (los_p (m->x, m->y, Player.x, Player.y)) {
	    mprint ("A strange numbing sensation comes over you...");
	    morewait ();
	    Player.mana = Player.mana / 2;
	    if (random_range (4))
		enchant (-1);
	    else
		dispel (-1);
	    Player.pow--;
	    if (--Player.pow < 1)
		p_death ("the Eater of Magic");
	}
    if (m->hp < 10) {
	mprint ("The Eater explodes in a burst of mana!");
	manastorm (m->x, m->y, 1000);
    }
}

void m_sp_dragonlord (struct monster *m)
{
    if (m_statusp (m, HOSTILE)) {
	if (distance (m->x, m->y, Player.x, Player.y) < 2) {
	    if (!Player.status[IMMOBILE]) {
		mprint ("A gust of wind from the Dragonlord's wings knocks you down!");
		p_damage (25, NORMAL_DAMAGE, "a gust of wind");
		setgamestatus (SKIP_PLAYER);
		Player.status[IMMOBILE] += 2;
	    } else if (!Constriction) {
		mprint ("The Dragonlord grabs you with his tail!");
		Constriction = 25;
		Player.status[IMMOBILE] += 1;
	    } else if (random_range (2)) {
		mprint ("The coils squeeze tighter and tighter...");
		p_damage (Constriction, NORMAL_DAMAGE, "the Dragonlord");
		Player.status[IMMOBILE] += 1;
		Constriction *= 2;
	    } else {
		mprint ("The Dragonlord hurls you to the ground!");
		p_damage (2 * Constriction, NORMAL_DAMAGE, "the Dragonlord");
		Constriction = 0;
	    }
	    m_sp_spell (m);
	} else {
	    Constriction = 0;
	    if (view_los_p (m->x, m->y, Player.x, Player.y)) {
		if ((!Player.immunity[FEAR]) && (!Player.status[AFRAID])) {
		    mprint ("You are awestruck at the sight of the Dragonlord.");
		    Player.status[AFRAID] += 5;
		}
		if (random_range (3)) {
		    m_sp_spell (m);
		    m_sp_spell (m);
		}
	    }
	}
    } else if (distance (m->x, m->y, Player.x, Player.y) < 2)
	mprint ("You are extremely impressed at the sight of the Dragonlord.");
}

void m_sp_blackout (struct monster *m)
{
    if ((distance (m->x, m->y, Player.x, Player.y) < 4) && (Player.status[BLINDED] == 0)) {
	mprint ("The fungus emits a burst of black spores. You've been blinded!");
	if (Player.status[TRUESIGHT] > 0)
	    mprint ("The blindness quickly passes.");
	else
	    Player.status[BLINDED] += 4;
    }
    if (loc_statusp (m->x, m->y, LIT)) {
	mprint ("The fungus chirps.... ");
	mprint ("The area is plunged into darkness.");
	torch_check ();
	torch_check ();
	torch_check ();
	torch_check ();
	torch_check ();
	torch_check ();
	spreadroomdark (m->x, m->y, Level->site[m->x][m->y].roomnumber);
	levelrefresh ();
    }
}

void m_sp_bogthing (struct monster *m)
{
    if (Player.status[IMMOBILE] && (distance (Player.x, Player.y, m->x, m->y) < 2)) {
	if (!Player.status[AFRAID]) {
	    mprint ("As the bogthing touches you, you feel a frisson of terror....");
	    if (Player.immunity[FEAR])
		mprint ("which you shake off.");
	    else
		Player.status[AFRAID] += 2;
	} else {
	    mprint ("The bogthing's touch causes you scream in agony!");
	    p_damage (50, UNSTOPPABLE, "fright");
	    mprint ("Your struggles grow steadily weaker....");
	    Player.con--;
	    Player.str--;
	    if ((Player.con < 3) || (Player.str < 3))
		p_death ("congestive heart failure");
	}
    }
}

void m_sp_were (struct monster *m)
{
    int mid;
    if (m_statusp (m, HOSTILE) || (Phase == 6)) {
	do
	    mid = random_range (ML9 - NML_0) + ML1;
	/* log npc, 0th level npc, high score npc or were-creature */
	while (mid == NPC || mid == ZERO_NPC || mid == HISCORE_NPC || mid == WEREHUMAN || (Monsters[mid].uniqueness != COMMON) || (!m_statusp (&(Monsters[mid]), MOBILE)) || (!m_statusp (&(Monsters[mid]), HOSTILE))
	    );
	m->id = Monsters[mid].id;
	m->hp += Monsters[mid].hp;
	m->status |= Monsters[mid].status;
	m->ac = Monsters[mid].ac;
	m->dmg = Monsters[mid].dmg;
	m->speed = Monsters[mid].speed;
	m->immunity |= Monsters[mid].immunity;
	m->xpv += Monsters[mid].xpv;
	m->corpseweight = Monsters[mid].corpseweight;
	m->monchar = Monsters[mid].monchar;
	m->talkf = Monsters[mid].talkf;
	m->meleef = Monsters[mid].meleef;
	m->strikef = Monsters[mid].strikef;
	m->specialf = Monsters[mid].specialf;
	strcpy (Str1, "were-");
	strcat (Str1, Monsters[mid].monstring);
	strcpy (Str2, "dead were-");
	strcat (Str2, Monsters[mid].monstring);
	m->monstring = salloc (Str1);
	m->corpsestr = salloc (Str2);
	m->immunity += pow2 (NORMAL_DAMAGE);
	if (los_p (m->x, m->y, Player.x, Player.y))
	    mprint ("You witness a hideous transformation!");
	else
	    mprint ("You hear a distant howl.");
    }
}

void m_sp_servant (struct monster *m)
{
    if ((m->id == SERV_LAW) && (Player.alignment < 0))
	m_status_set (m, HOSTILE);
    else if ((m->id == SERV_CHAOS) && (Player.alignment > 0))
	m_status_set (m, HOSTILE);
}

void m_sp_av (struct monster *m)
{
    if (Player.mana > 0) {
	mprint ("You feel a sudden loss of mana!");
	Player.mana -= (max (0, 10 - distance (m->x, m->y, Player.x, Player.y)));
	dataprint ();
    }
}

void m_sp_lw (struct monster *m)
{
    if (random_range (2)) {
	if (Level->site[m->x][m->y].locchar == FLOOR) {
	    Level->site[m->x][m->y].locchar = LAVA;
	    Level->site[m->x][m->y].p_locf = L_LAVA;
	    lset (m->x, m->y, CHANGED);
	} else if (Level->site[m->x][m->y].locchar == WATER) {
	    Level->site[m->x][m->y].locchar = FLOOR;
	    Level->site[m->x][m->y].p_locf = L_NO_OP;
	    lset (m->x, m->y, CHANGED);
	}
    }
}

void m_sp_angel (struct monster *m)
{
    int mid, hostile = FALSE;
    switch (m->aux1) {
	case ATHENA:
	case ODIN:
	    hostile = ((Player.patron == HECATE) || (Player.patron == SET));
	    break;
	case SET:
	case HECATE:
	    hostile = ((Player.patron == ODIN) || (Player.patron == ATHENA));
	    break;
	case DESTINY:
	    hostile = (Player.patron != DESTINY);
	    break;
    }
    if (hostile)
	m_status_set (m, HOSTILE);
    if (m_statusp (m, HOSTILE)) {
	mprint ("The angel summons a heavenly host!");
	switch (m->level) {
	    case 9:
		mid = HIGH_ANGEL;
		break;
	    case 8:
		mid = ANGEL;
		break;
	    default:
	    case 6:
		mid = PHANTOM;
		break;
	}
	summon (-1, mid);
	summon (-1, mid);
	summon (-1, mid);
	/* prevent angel from summoning infinitely */
	m->specialf = M_NO_OP;
    }
}

/* Could completely fill up level */
void m_sp_swarm (struct monster *m)
{
    if (random_range (5) == 1) {
	if (view_los_p (m->x, m->y, Player.x, Player.y))
	    mprint ("The swarm expands!");
	else
	    mprint ("You hear an aggravating humming noise.");
	summon (-1, SWARM);
    }
}

/* raise nearby corpses from the dead.... */
void m_sp_raise (struct monster *m)
{
    int x, y;
    pol t;
    for (x = m->x - 2; x <= m->x + 2; x++)
	for (y = m->y - 2; y <= m->y + 2; y++)
	    if (inbounds (x, y))
		if (Level->site[x][y].things != NULL)
		    if (Level->site[x][y].things->thing->id == CORPSEID) {
			mprint ("The Zombie Overlord makes a mystical gesture...");
			summon (-1, Level->site[x][y].things->thing->charge);
			t = Level->site[x][y].things;
			Level->site[x][y].things = Level->site[x][y].things->next;
			free ((char *) t);
		    }
}

void m_sp_mb (struct monster *m)
{
    if (distance (m->x, m->y, Player.x, Player.y) == 1) {
	mprint ("The manaburst explodes!");
	if (m_statusp (m, HOSTILE)) {
	    mprint ("You get blasted!");
	    p_damage (random_range (100), UNSTOPPABLE, "a manaburst");
	    mprint ("You feel cold all over!");
	    Player.pow -= 3;
	    Player.iq--;
	    Player.con--;
	    Player.str -= 2;
	    Player.dex--;
	    Player.agi--;
	    dispel (-1);
	} else {
	    mprint ("You feel toasty warm inside!");
	    Player.pow++;
	    Player.mana = max (Player.mana, calcmana ());
	    Player.hp = max (Player.hp, ++Player.maxhp);
	}
	m->hp = 0;
    }
}

void m_sp_mirror (struct monster *m)
{
    int i, x, y;
    if (view_los_p (m->x, m->y, Player.x, Player.y)) {
	if (random_range (20) + 6 < m->level) {
	    summon (-1, m->id);
	    mprint ("You hear the sound of a mirror shattering!");
	} else
	    for (i = 0; i < 5; i++) {
		x = m->x + random_range (13) - 6;
		y = m->y + random_range (13) - 6;
		if (inbounds (x, y)) {
		    Level->site[x][y].showchar = m->monchar;
		    putspot (x, y, m->monchar);
		}
	    }
    }
}

void m_illusion (struct monster *m)
{
    int i = random_range (NUMMONSTERS);
    if (i == NPC || i == HISCORE_NPC || i == ZERO_NPC)
	i = m->id;		/* can't imitate NPC */
    if (Player.status[TRUESIGHT]) {
	m->monchar = Monsters[m->id].monchar;
	m->monstring = Monsters[m->id].monstring;
    } else if (random_range (5) == 1) {
	m->monchar = Monsters[i].monchar;
	m->monstring = Monsters[i].monstring;
    }
}

void m_huge_sounds (struct monster *m)
{
    if (m_statusp (m, AWAKE) && (!los_p (m->x, m->y, Player.x, Player.y)) && (random_range (10) == 1))
	mprint ("The dungeon shakes!");
}

void m_thief_f (struct monster *m)
{
    int i = random_item ();
    if (random_range (3) == 1) {
	if (distance (Player.x, Player.y, m->x, m->y) < 2) {
	    if (p_immune (THEFT) || (Player.level > (m->level * 2) + random_range (20)))
		mprint ("You feel secure.");
	    else {
		if (i == ABORT)
		    mprint ("You feel fortunate.");
		else if ((Player.possessions[i]->used) || (Player.dex < m->level * random_range (10))) {
		    mprint ("You feel a sharp tug.... You hold on!");
		} else {
		    mprint ("You feel uneasy for a moment.");
		    if (m->uniqueness == COMMON) {
			strcpy (Str2, "The ");
			strcat (Str2, m->monstring);
		    } else
			strcpy (Str2, m->monstring);
		    strcat (Str2, " suddenly runs away for some reason.");
		    mprint (Str2);
		    m_teleport (m);
		    m->movef = M_MOVE_SCAREDY;
		    m->specialf = M_MOVE_SCAREDY;
		    m_pickup (m, Player.possessions[i]);
		    conform_unused_object (Player.possessions[i]);
		    Player.possessions[i] = NULL;
		}
	    }
	}
    }
}

void m_summon (struct monster *m)
{
    if ((distance (Player.x, Player.y, m->x, m->y) < 2) && (random_range (3) == 1)) {
	summon (0, -1);
	summon (0, -1);
    }
}

void m_aggravate (struct monster *m)
{

    if (m_statusp (m, HOSTILE)) {
	if (m->uniqueness == COMMON) {
	    strcpy (Str2, "The ");
	    strcat (Str2, m->monstring);
	} else
	    strcpy (Str2, m->monstring);
	strcat (Str2, " emits an irritating humming sound.");
	mprint (Str2);
	aggravate ();
	m_status_reset (m, HOSTILE);
    }
}

void m_sp_merchant (struct monster *m)
{
    pml ml;
    if (m_statusp (m, HOSTILE))
	if (Current_Environment == E_VILLAGE) {
	    mprint ("The merchant screams: 'Help! Murder! Guards! Help!'");
	    mprint ("You hear the sound of police whistles and running feet.");
	    for (ml = Level->mlist; ml != NULL; ml = ml->next) {
		m_status_set (ml->m, AWAKE);
		m_status_set (ml->m, HOSTILE);
	    }
	    m->specialf = M_NO_OP;
	}
}

/* The special function of the various people in the court of the archmage */
/* and the sorcerors' circle */
void m_sp_court (struct monster *m)
{
    pml ml;
    if (m_statusp (m, HOSTILE)) {
	mprint ("A storm of spells hits you!");
	for (ml = Level->mlist; ml != NULL; ml = ml->next) {
	    m_status_set (ml->m, HOSTILE);
	    m_sp_spell (ml->m);
	    if (ml->m->specialf == M_SP_COURT)
		ml->m->specialf = M_SP_SPELL;
	}
    }
}

/* The special function of the dragons in the dragons' lair */
void m_sp_lair (struct monster *m)
{
    pml ml;
    if (m_statusp (m, HOSTILE)) {
	mprint ("You notice a number of dragons waking up....");
	mprint ("You are struck by a quantity of firebolts.");
	morewait ();
	for (ml = Level->mlist; ml != NULL; ml = ml->next)
	    if (ml->m->hp > 0 && ml->m->specialf == M_SP_LAIR) {
		m_status_set (ml->m, HOSTILE);
		fbolt (ml->m->x, ml->m->y, Player.x, Player.y, 100, 100);
		if (ml->m->id == DRAGON_LORD)
		    ml->m->specialf = M_SP_DRAGONLORD;
		else
		    ml->m->specialf = M_STRIKE_FBOLT;
	    }
    }
}

void m_sp_prime (struct monster *m)
{
    if (m_statusp (m, HOSTILE)) {
	mprint ("The prime sorceror gestures and a pentacular gate opens!");
	mprint ("You are surrounded by demons!");
	summon (-1, DEMON_PRINCE);
	summon (-1, DEMON_PRINCE);
	summon (-1, DEMON_PRINCE);
	summon (-1, DEMON_PRINCE);
    }
    m->specialf = M_SP_SPELL;
}

void m_damage (struct monster *m, int dmg, int dtype)
{
    m_status_set (m, AWAKE);
    m_status_set (m, HOSTILE);
    if (m_immunityp (m, dtype)) {
	if (los_p (Player.x, Player.y, m->x, m->y)) {
	    char buf[80];
	    if (m->uniqueness != COMMON)
		strcpy (buf, m->monstring);
	    else {
		strcpy (buf, "The ");
		strcat (buf, m->monstring);
	    }
	    strcat (buf, " ignores the attack!");
	    mprint (buf);
	}
    } else if ((m->hp -= dmg) < 1)
	m_death (m);
}

void m_death (struct monster *m)
{
    pob corpse;
    pml ml;
    int x, y, found = FALSE;
    pol curr, prev = NULL;

    m->hp = -1;
    if (los_p (Player.x, Player.y, m->x, m->y)) {
	gain_experience (m->xpv);
	calc_melee ();
	char buf[80];
	if (m->uniqueness != COMMON)
	    strcpy (buf, m->monstring);
	else {
	    strcpy (buf, "The ");
	    strcat (buf, m->monstring);
	}
	strcat (buf, " is dead! ");
	mprint (buf);
    }
    m_dropstuff (m);
    if (m->id == DEATH) {	/* Death */
	mprint ("Death lies sprawled out on the ground......");
	mprint ("Death laughs ironically and gets back to its feet.");
	mprint ("It gestures and another scythe appears in its hands.");
	switch (random_range (10)) {
	    case 0:
		mprint ("Death performs a little bow and goes back on guard.");
		break;
	    case 1:
		mprint ("'A hit! A palpable hit!' Death goes back on the attack.");
		break;
	    case 2:
		mprint ("'Ah, if only it could be so simple!' snickers Death.");
		break;
	    case 3:
		mprint ("'You think Death can be slain?  What a jest!' says Death.");
		break;
	    case 4:
		mprint ("'Your point is well taken.' says Death, attacking again.");
		break;
	    case 5:
		mprint ("'Oh, come now, stop delaying the inevitable.' says Death.");
		break;
	    case 6:
		mprint ("'Your destiny ends here with me.' says Death, scythe raised.");
		break;
	    case 7:
		mprint ("'I almost felt that.' says Death, smiling.");
		break;
	    case 8:
		mprint ("'Timeo Mortis?' asks Death quizzically, 'Not me!'");
		break;
	    case 9:
		mprint ("Death sighs theatrically. 'They never learn.'");
		break;
	}
	strengthen_death (m);
    } else {
	Level->site[m->x][m->y].creature = NULL;
	if (m == Arena_Monster)
	    Arena_Victory = TRUE;	/* won this round of arena combat */
	if (random_range (2) || (m->uniqueness != COMMON)) {
	    corpse = ((pob) checkmalloc (sizeof (objtype)));
	    make_corpse (corpse, m);
	    drop_at (m->x, m->y, corpse);
	}
	plotspot (m->x, m->y, FALSE);
	switch (m->id) {
	    case HISCORE_NPC:
		switch (m->aux2) {
		    case 0:
			mprint ("You hear a faroff dirge. You feel a sense of triumph.");
			break;
		    case 1:
		    case 2:
		    case 3:
		    case 4:
		    case 5:
		    case 6:
			mprint ("You hear a faroff sound like angels crying....");
			strcpy (Priest[m->aux2], nameprint ());
			Priestbehavior[m->aux2] = 2933;
			break;
		    case 7:
			mprint ("A furtive figure dashes out of the shadows, takes a look at");
			mprint ("the corpse, and runs away!");
			strcpy (Shadowlord, nameprint ());
			Shadowlordbehavior = 2912;
			break;
		    case 8:
			mprint ("An aide-de-camp approaches, removes the corpse's insignia,");
			mprint ("and departs.");
			strcpy (Commandant, nameprint ());
			Commandantbehavior = 2912;
			break;
		    case 9:
			mprint ("An odd glow surrounds the corpse, and slowly fades.");
			strcpy (Archmage, nameprint ());
			Archmagebehavior = 2933;
			break;
		    case 10:
			mprint ("A demon materializes, takes a quick look at the corpse,");
			mprint ("and teleports away with a faint popping noise.");
			strcpy (Prime, nameprint ());
			Primebehavior = 2932;
			break;
		    case 11:
			mprint ("A sports columnist rushes forward and takes a quick photo");
			mprint ("of the corpse and rushes off muttering about a deadline.");
			strcpy (Champion, nameprint ());
			Championbehavior = 2913;
			break;
		    case 12:
			mprint ("You hear a fanfare in the distance, and feel dismayed.");
			strcpy (Duke, nameprint ());
			Dukebehavior = 2911;
			break;
		    case 13:
			if (Player.alignment > 10)
			    mprint ("You feel smug.");
			else if (Player.alignment < 10)
			    mprint ("You feel ashamed.");
			strcpy (Chaoslord, nameprint ());
			Chaoslordbehavior = 2912;
			break;
		    case 14:
			if (Player.alignment < 10)
			    mprint ("You feel smug.");
			else if (Player.alignment > 10)
			    mprint ("You feel ashamed.");
			strcpy (Lawlord, nameprint ());
			Lawlordbehavior = 2911;
			break;
		    case 15:
			/* just a tad complicated. Promote a new justiciar if any
			   guards are left in the city, otherwise Destroy the Order! */
			Player.alignment -= 100;
			if (!gamestatusp (DESTROYED_ORDER)) {
			    curr = Level->site[m->x][m->y].things;
			    while (curr && curr->thing->id != THINGID + 16) {
				prev = curr;
				curr = curr->next;
			    }
			    strcpy (Justiciar, nameprint ());
			    Justiciarbehavior = 2911;
			    mprint ("In the distance you hear a trumpet. A Servant of Law");
			    /* promote one of the city guards to be justiciar */
			    ml = City->mlist;
			    while ((!found) && (ml != NULL)) {
				found = ((ml->m->id == GUARD) && (ml->m->hp > 0));
				if (!found)
				    ml = ml->next;
			    }
			    if (ml != NULL) {
				if (curr) {
				    mprint ("materializes, sheds a tear, picks up the badge, and leaves.");
				    m_pickup (ml->m, curr->thing);
				    if (prev)
					prev->next = curr->next;
				    else
					Level->site[m->x][m->y].things = curr->next;
				    free (curr);
				} else
				    mprint ("materializes, sheds a tear, and leaves.");
				mprint ("A new justiciar has been promoted!");
				x = ml->m->x;
				y = ml->m->y;
				make_hiscore_npc (ml->m, 15);
				ml->m->x = x;
				ml->m->y = y;
				ml->m->click = (Tick + 1) % 60;
				m_status_reset (ml->m, AWAKE);
				m_status_reset (ml->m, HOSTILE);
			    } else {
				mprint ("materializes, sheds a tear, and leaves.");
				morewait ();
			    }
			    alert_guards ();
			    /* will cause order to be destroyed if no guards or justiciar */
			} else {
			    mprint ("A Servant of Chaos materializes, grabs the corpse,");
			    mprint ("snickers a bit, and vanishes.");
			}
			break;
		}
		save_hiscore_npc (m->aux2);
		break;
	    case GUARD:	/* guard */
		Player.alignment -= 10;
		if ((Current_Environment == E_CITY) || (Current_Environment == E_VILLAGE))
		    alert_guards ();
		break;
	    case GOBLIN_KING:
		if (!gamestatusp (ATTACKED_ORACLE)) {
		    mprint ("You seem to hear a woman's voice from far off:");
		    mprint ("'Well done! Come to me now....'");
		}
		setgamestatus (COMPLETED_CAVES);
		break;		/* gob king */
	    case GREAT_WYRM:
		if (!gamestatusp (ATTACKED_ORACLE)) {
		    mprint ("A female voice sounds from just behind your ear:");
		    mprint ("'Well fought! I have some new advice for you....'");
		}
		setgamestatus (COMPLETED_SEWERS);
		break;		/*grt worm */
	    case EATER:
		setgamestatus (KILLED_EATER);
		break;
	    case LAWBRINGER:
		setgamestatus (KILLED_LAWBRINGER);
		break;
	    case DRAGON_LORD:
		setgamestatus (KILLED_DRAGONLORD);
		break;
	    case DEMON_EMP:
		setgamestatus (COMPLETED_VOLCANO);
		if (!gamestatusp (ATTACKED_ORACLE)) {
		    mprint ("You feel a soft touch on your shoulder...");
		    mprint ("You turn around but there is no one there!");
		    mprint ("You turn back and see a note: 'See me soon.'");
		    mprint ("The note vanishes in a burst of blue fire!");
		}
		break;
	    case ELEM_MASTER:
		if (!gamestatusp (ATTACKED_ORACLE)) {
		    mprint ("Words appear before you, traced in blue flame!");
		    mprint ("'Return to the Prime Plane via the Circle of Sorcerors....'");
		}
		break;		/* elem mast */
	}
	switch (m->specialf) {
	    case M_SP_COURT:
	    case M_SP_LAIR:
		m_status_set (m, HOSTILE);
		monster_action (m, m->specialf);
	}
    }
}

void monster_move (struct monster *m)
{
    monster_action (m, m->movef);
}

void monster_strike (struct monster *m)
{
    if (player_on_sanctuary ())
	print1 ("The aegis of your deity protects you!");
    else {
	/* It's lawful to wait to be attacked */
	if (m->attacked == 0)
	    Player.alignment++;
	m->attacked++;
	monster_action (m, m->strikef);
    }
}

void monster_special (struct monster *m)
{
    /* since many special functions are really attacks, cancel them
       all if on sanctuary */
    if (!player_on_sanctuary ())
	monster_action (m, m->specialf);
}

void monster_talk (struct monster *m)
{
    monster_action (m, m->talkf);
}

void monster_action (struct monster *m, int action)
{
    int meleef;
    if ((action >= M_MELEE_NORMAL) && (action < M_MOVE_NORMAL)) {
	/* kluge allows multiple attack forms */
	if (distance (m->x, m->y, Player.x, Player.y) < 2) {
	    meleef = m->meleef;
	    m->meleef = action;
	    tacmonster (m);
	    m->meleef = meleef;
	}
    } else
	switch (action) {

	    case M_NO_OP:
		m_no_op (m);
		break;

	    case M_MOVE_NORMAL:
		m_normal_move (m);
		break;
	    case M_MOVE_FLUTTER:
		m_flutter_move (m);
		break;
	    case M_MOVE_FOLLOW:
		m_follow_move (m);
		break;
	    case M_MOVE_TELEPORT:
		m_teleport (m);
		break;
	    case M_MOVE_RANDOM:
		m_random_move (m);
		break;
	    case M_MOVE_SMART:
		m_smart_move (m);
		break;
	    case M_MOVE_SPIRIT:
		m_spirit_move (m);
		break;
	    case M_MOVE_CONFUSED:
		m_confused_move (m);
		break;
	    case M_MOVE_SCAREDY:
		m_scaredy_move (m);
		break;
	    case M_MOVE_ANIMAL:
		m_move_animal (m);
		break;
	    case M_MOVE_LEASH:
		m_move_leash (m);
		break;

	    case M_STRIKE_MISSILE:
		m_nbolt (m);
		break;
	    case M_STRIKE_FBOLT:
		m_firebolt (m);
		break;
	    case M_STRIKE_LBALL:
		m_lball (m);
		break;
	    case M_STRIKE_FBALL:
		m_fireball (m);
		break;
	    case M_STRIKE_SNOWBALL:
		m_snowball (m);
		break;
	    case M_STRIKE_BLIND:
		m_blind_strike (m);
		break;
	    case M_STRIKE_SONIC:
		m_strike_sonic (m);
		break;

	    case M_TALK_HORSE:
		m_talk_horse (m);
		break;
	    case M_TALK_THIEF:
		m_talk_thief (m);
		break;
	    case M_TALK_STUPID:
		m_talk_stupid (m);
		break;
	    case M_TALK_SILENT:
		m_talk_silent (m);
		break;
	    case M_TALK_HUNGRY:
		m_talk_hungry (m);
		break;
	    case M_TALK_GREEDY:
		m_talk_greedy (m);
		break;
	    case M_TALK_TITTER:
		m_talk_titter (m);
		break;
	    case M_TALK_MP:
		m_talk_mp (m);
		break;
	    case M_TALK_IM:
		m_talk_im (m);
		break;
	    case M_TALK_MAN:
		m_talk_man (m);
		break;
	    case M_TALK_ROBOT:
		m_talk_robot (m);
		break;
	    case M_TALK_EVIL:
		m_talk_evil (m);
		break;
	    case M_TALK_GUARD:
		m_talk_guard (m);
		break;
	    case M_TALK_MIMSY:
		m_talk_mimsy (m);
		break;
	    case M_TALK_SLITHY:
		m_talk_slithy (m);
		break;
	    case M_TALK_BURBLE:
		m_talk_burble (m);
		break;
	    case M_TALK_BEG:
		m_talk_beg (m);
		break;
	    case M_TALK_HINT:
		m_talk_hint (m);
		break;
	    case M_TALK_EF:
		m_talk_ef (m);
		break;
	    case M_TALK_GF:
		m_talk_gf (m);
		break;
	    case M_TALK_SEDUCTOR:
		m_talk_seductor (m);
		break;
	    case M_TALK_DEMONLOVER:
		m_talk_demonlover (m);
		break;
	    case M_TALK_NINJA:
		m_talk_ninja (m);
		break;
	    case M_TALK_ASSASSIN:
		m_talk_assassin (m);
		break;
	    case M_TALK_SERVANT:
		m_talk_servant (m);
		break;
	    case M_TALK_ANIMAL:
		m_talk_animal (m);
		break;
	    case M_TALK_SCREAM:
		m_talk_scream (m);
		break;
	    case M_TALK_PARROT:
		m_talk_parrot (m);
		break;
	    case M_TALK_HYENA:
		m_talk_hyena (m);
		break;
	    case M_TALK_DRUID:
		m_talk_druid (m);
		break;
	    case M_TALK_ARCHMAGE:
		m_talk_archmage (m);
		break;
	    case M_TALK_MERCHANT:
		m_talk_merchant (m);
		break;
	    case M_TALK_PRIME:
		m_talk_prime (m);
		break;

	    case M_SP_BOG:
		m_sp_bogthing (m);
		break;
	    case M_SP_WERE:
		m_sp_were (m);
		break;
	    case M_SP_WHISTLEBLOWER:
		m_sp_whistleblower (m);
		break;
	    case M_SP_MERCHANT:
		m_sp_merchant (m);
		break;
	    case M_SP_SURPRISE:
		m_sp_surprise (m);
		break;
	    case M_SP_MP:
		m_sp_mp (m);
		break;
	    case M_SP_THIEF:
		m_thief_f (m);
		break;
	    case M_SP_DEMONLOVER:
		m_sp_demonlover (m);
		break;
	    case M_SP_AGGRAVATE:
		m_aggravate (m);
		break;
	    case M_SP_POISON_CLOUD:
		m_sp_poison_cloud (m);
		break;
	    case M_SP_HUGE:
		m_huge_sounds (m);
		break;
	    case M_SP_SUMMON:
		m_summon (m);
		break;
	    case M_SP_ILLUSION:
		m_illusion (m);
		break;
	    case M_SP_ESCAPE:
		m_sp_escape (m);
		break;
	    case M_SP_FLUTTER:
		m_flutter_move (m);
		break;
	    case M_SP_EXPLODE:
		m_sp_explode (m);
		break;
	    case M_SP_DEMON:
		m_sp_demon (m);
		break;
	    case M_SP_ACID_CLOUD:
		m_sp_acid_cloud (m);
		break;
	    case M_SP_GHOST:
		m_sp_ghost (m);
		break;
	    case M_SP_SPELL:
		m_sp_spell (m);
		break;
	    case M_SP_SEDUCTOR:
		m_sp_seductor (m);
		break;
	    case M_SP_EATER:
		m_sp_eater (m);
		break;
	    case M_SP_DRAGONLORD:
		m_sp_dragonlord (m);
		break;
	    case M_SP_BLACKOUT:
		m_sp_blackout (m);
		break;
	    case M_SP_SWARM:
		m_sp_swarm (m);
		break;
	    case M_SP_ANGEL:
		m_sp_angel (m);
		break;
	    case M_SP_SERVANT:
		m_sp_servant (m);
		break;
	    case M_SP_AV:
		m_sp_av (m);
		break;
	    case M_SP_LW:
		m_sp_lw (m);
		break;
	    case M_SP_MB:
		m_sp_mb (m);
		break;
	    case M_SP_RAISE:
		m_sp_raise (m);
		break;
	    case M_SP_MIRROR:
		m_sp_mirror (m);
		break;
	    case M_SP_COURT:
		m_sp_court (m);
		break;
	    case M_SP_LAIR:
		m_sp_lair (m);
		break;
	    case M_SP_PRIME:
		m_sp_prime (m);
		break;
	}
}

/* makes one of the highscore npcs */
void make_hiscore_npc (pmt npc, int npcid)
{
    int st = -1;
    pob ob;
    *npc = Monsters[HISCORE_NPC];
    npc->aux2 = npcid;
    /* each of the high score npcs can be created here */
    switch (npcid) {
	case 0:
	    strcpy (Str2, Hiscorer);
	    determine_npc_behavior (npc, Hilevel, Hibehavior);
	    break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	    strcpy (Str2, Priest[npcid]);
	    determine_npc_behavior (npc, Priestlevel[npcid], Priestbehavior[npcid]);
	    st = ARTIFACTID + 13 + npcid;	/* appropriate holy symbol... */
	    Objects[st].uniqueness = UNIQUE_MADE;
	    if (npcid == DRUID)
		npc->talkf = M_TALK_DRUID;
	    if (Player.patron == npcid)
		m_status_reset (npc, HOSTILE);
	    break;
	case 7:
	    strcpy (Str2, Shadowlord);
	    determine_npc_behavior (npc, Shadowlordlevel, Shadowlordbehavior);
	    break;
	case 8:
	    strcpy (Str2, Commandant);
	    determine_npc_behavior (npc, Commandantlevel, Commandantbehavior);
	    if (Player.rank[LEGION])
		m_status_reset (npc, HOSTILE);
	    break;
	case 9:
	    strcpy (Str2, Archmage);
	    determine_npc_behavior (npc, Archmagelevel, Archmagebehavior);
	    st = ARTIFACTID + 9;	/* kolwynia */
	    npc->talkf = M_TALK_ARCHMAGE;
	    m_status_reset (npc, WANDERING);
	    m_status_reset (npc, HOSTILE);
	    break;
	case 10:
	    strcpy (Str2, Prime);
	    determine_npc_behavior (npc, Primelevel, Primebehavior);
	    npc->talkf = M_TALK_PRIME;
	    npc->specialf = M_SP_PRIME;
	    if (Player.alignment < 0)
		m_status_reset (npc, HOSTILE);
	    break;
	case 11:
	    strcpy (Str2, Champion);
	    determine_npc_behavior (npc, Championlevel, Championbehavior);
	    if (Player.rank[ARENA])
		m_status_reset (npc, HOSTILE);
	    break;
	case 12:
	    strcpy (Str2, Duke);
	    determine_npc_behavior (npc, Dukelevel, Dukebehavior);
	    break;
	case 13:
	    strcpy (Str2, Chaoslord);
	    determine_npc_behavior (npc, Chaoslordlevel, Chaoslordbehavior);
	    if (Player.alignment < 0 && random_range (2))
		m_status_reset (npc, HOSTILE);
	    break;
	case 14:
	    strcpy (Str2, Lawlord);
	    determine_npc_behavior (npc, Lawlordlevel, Lawlordbehavior);
	    if (Player.alignment > 0)
		m_status_reset (npc, HOSTILE);
	    break;
	case 15:
	    strcpy (Str2, Justiciar);
	    determine_npc_behavior (npc, Justiciarlevel, Justiciarbehavior);
	    st = THINGID + 16;	/* badge */
	    npc->talkf = M_TALK_GUARD;
	    npc->specialf = M_SP_WHISTLEBLOWER;
	    m_status_reset (npc, WANDERING);
	    m_status_reset (npc, HOSTILE);
	    break;
    }
    if (st > -1 && Objects[st].uniqueness == UNIQUE_MADE) {
	ob = ((pob) checkmalloc (sizeof (objtype)));
	*ob = Objects[st];
	m_pickup (npc, ob);
    }
    npc->monstring = salloc (Str2);
    char buf[80];
    strcpy (buf, "The body of ");
    strcat (buf, Str2);
    npc->corpsestr = salloc (buf);
}

/* sets npc behavior given level and behavior code */
void determine_npc_behavior (pmt npc, int level, int behavior)
{
    int combatype, competence, talktype;
    npc->hp = (level + 1) * 20;
    npc->status = AWAKE + MOBILE + WANDERING;
    combatype = (behavior % 100) / 10;
    competence = (behavior % 1000) / 100;
    talktype = behavior / 1000;
    npc->level = competence;
    if (npc->level < 2 * difficulty ())
	npc->status += HOSTILE;
    npc->xpv = npc->level * 20;
    switch (combatype) {
	case 1:		/* melee */
	    npc->meleef = M_MELEE_NORMAL;
	    npc->dmg = competence * 5;
	    npc->hit = competence * 3;
	    npc->speed = 3;
	    break;
	case 2:		/*missile */
	    npc->meleef = M_MELEE_NORMAL;
	    npc->strikef = M_STRIKE_MISSILE;
	    npc->dmg = competence * 3;
	    npc->hit = competence * 2;
	    npc->speed = 4;
	    break;
	case 3:		/* spellcasting */
	    npc->meleef = M_MELEE_NORMAL;
	    npc->dmg = competence;
	    npc->hit = competence;
	    npc->specialf = M_SP_SPELL;
	    npc->speed = 6;
	    break;
	case 4:		/* thievery */
	    npc->meleef = M_MELEE_NORMAL;
	    npc->dmg = competence;
	    npc->hit = competence;
	    npc->specialf = M_SP_THIEF;
	    npc->speed = 3;
	    break;
	case 5:		/* flee */
	    npc->dmg = competence;
	    npc->hit = competence;
	    npc->meleef = M_MELEE_NORMAL;
	    npc->specialf = M_MOVE_SCAREDY;
	    npc->speed = 3;
	    break;
    }
    if (npc->talkf == M_TALK_MAN)
	switch (talktype) {
	    case 1:
		npc->talkf = M_TALK_EVIL;
		break;
	    case 2:
		npc->talkf = M_TALK_MAN;
		break;
	    case 3:
		npc->talkf = M_TALK_HINT;
		break;
	    case 4:
		npc->talkf = M_TALK_BEG;
		break;
	    case 5:
		npc->talkf = M_TALK_SILENT;
		break;
	    default:
		mprint ("Say Whutt? (npc talk weirdness)");
		break;
	}
    npc->uniqueness = UNIQUE_MADE;
}

/* makes an ordinary npc (maybe undead) */
void make_log_npc (struct monster *npc)
{
    int i, n;
    int behavior, status, level;
    FILE *fd;

    /* in case the log file is null */
    behavior = 2718;
    level = 1;
    status = 2;
    strcpy (Str2, "Malaprop the Misnamed");

    strcpy (Str1, Omegalib);
    strcat (Str1, "omega.log");
    fd = checkfopen (Str1, "r");
    n = 1;
    while (fgets (Str1, STRING_LEN, fd)) {
	if (random_range (n) == 0) {	/* this algo. from Knuth 2 - cute, eh? */
	    sscanf (Str1, "%d %d %d", &status, &level, &behavior);
	    for (i = 0; (Str1[i] < 'a' || Str1[i] > 'z') && (Str1[i] < 'A' || Str1[i] > 'Z'); i++);
	    strcpy (Str2, Str1 + i);
	    Str2[strlen (Str2) - 1] = '\0';	/* 'cos fgets reads in the \n */
	}
	n++;
    }
    fclose (fd);
    npc->hp = level * 20;
    if (status == 1) {
	if (level < 3) {
	    *npc = Monsters[GHOST];
	    strcpy (Str1, "ghost named ");
	} else if (level < 7) {
	    *npc = Monsters[HAUNT];
	    strcpy (Str1, "haunt named ");
	} else if (level < 12) {
	    *npc = Monsters[SPECTRE];
	    strcpy (Str1, "spectre named ");
	} else {
	    *npc = Monsters[LICHE];
	    strcpy (Str1, "lich named ");
	}
	strcat (Str1, Str2);
	npc->monstring = salloc (Str1);
	strcpy (Str3, "the mortal remains of ");
	strcat (Str3, Str2);
	npc->corpsestr = salloc (Str3);
    } else {
	npc->monstring = salloc (Str2);
	strcpy (Str3, "the corpse of ");
	strcat (Str3, Str2);
	npc->corpsestr = salloc (Str3);
    }
    determine_npc_behavior (npc, level, behavior);
}

void m_trap_dart (struct monster *m)
{
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	if (m->uniqueness != COMMON)
	    strcpy (Str1, m->monstring);
	else {
	    strcpy (Str1, "The ");
	    strcat (Str1, m->monstring);
	}
	strcat (Str1, " was hit by a dart!");
	mprint (Str1);
	Level->site[m->x][m->y].locchar = TRAP;
	lset (m->x, m->y, CHANGED);
    }
    m_damage (m, difficulty () * 2, NORMAL_DAMAGE);
}

void m_trap_pit (struct monster *m)
{
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	if (m->uniqueness != COMMON)
	    strcpy (Str1, m->monstring);
	else {
	    strcpy (Str1, "The ");
	    strcat (Str1, m->monstring);
	}
	strcat (Str1, " fell into a pit!");
	mprint (Str1);
	Level->site[m->x][m->y].locchar = TRAP;
	lset (m->x, m->y, CHANGED);
    }
    if (!m_statusp (m, INTANGIBLE))
	m_status_reset (m, MOBILE);
    m_damage (m, difficulty () * 5, NORMAL_DAMAGE);

}

void m_trap_door (struct monster *m)
{
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	if (m->uniqueness != COMMON)
	    strcpy (Str1, m->monstring);
	else {
	    strcpy (Str1, "The ");
	    strcat (Str1, m->monstring);
	}
	strcat (Str1, " fell into a trap door!");
	mprint (Str1);
	Level->site[m->x][m->y].locchar = TRAP;
	lset (m->x, m->y, CHANGED);
    }
    m_vanish (m);
}

void m_trap_abyss (struct monster *m)
{
    char buf[80];
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	if (m->uniqueness != COMMON)
	    strcpy (buf, m->monstring);
	else {
	    strcpy (buf, "The ");
	    strcat (buf, m->monstring);
	}
	strcat (buf, " fell into the infinite abyss!");
	mprint (buf);
	Level->site[m->x][m->y].locchar = ABYSS;
	lset (m->x, m->y, CHANGED);
	Level->site[m->x][m->y].p_locf = L_ABYSS;
	lset (m->x, m->y, CHANGED);
    }
    setgamestatus (SUPPRESS_PRINTING);
    m_vanish (m);
    resetgamestatus (SUPPRESS_PRINTING);
}

void m_trap_snare (struct monster *m)
{
    char buf[80];
    Level->site[m->x][m->y].locchar = TRAP;
    lset (m->x, m->y, CHANGED);
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	if (m->uniqueness != COMMON)
	    strcpy (buf, m->monstring);
	else {
	    strcpy (buf, "The ");
	    strcat (buf, m->monstring);
	}
	strcat (buf, " was caught in a snare!");
	mprint (buf);
    }
    if (!m_statusp (m, INTANGIBLE))
	m_status_reset (m, MOBILE);
}

void m_trap_blade (struct monster *m)
{
    char buf[80];
    Level->site[m->x][m->y].locchar = TRAP;
    lset (m->x, m->y, CHANGED);
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	if (m->uniqueness != COMMON)
	    strcpy (buf, m->monstring);
	else {
	    strcpy (buf, "The ");
	    strcat (buf, m->monstring);
	}
	strcat (buf, " was hit by a blade trap!");
	mprint (buf);
    }
    m_damage (m, (difficulty () + 1) * 7 - Player.defense, NORMAL_DAMAGE);
}

void m_trap_fire (struct monster *m)
{
    char buf[80];
    Level->site[m->x][m->y].locchar = TRAP;
    lset (m->x, m->y, CHANGED);
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	if (m->uniqueness != COMMON)
	    strcpy (buf, m->monstring);
	else {
	    strcpy (buf, "The ");
	    strcat (buf, m->monstring);
	}
	strcat (buf, " was hit by a fire trap!");
	mprint (buf);
    }
    m_damage (m, (difficulty () + 1) * 5, FLAME);
}

void m_fire (struct monster *m)
{
    char buf[80];
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	if (m->uniqueness != COMMON)
	    strcpy (buf, m->monstring);
	else {
	    strcpy (buf, "The ");
	    strcat (buf, m->monstring);
	}
	strcat (buf, " was blasted by fire!");
	mprint (buf);
    }
    m_damage (m, random_range (100), FLAME);
}

void m_trap_teleport (struct monster *m)
{
    char buf[80];
    Level->site[m->x][m->y].locchar = TRAP;
    lset (m->x, m->y, CHANGED);
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	if (m->uniqueness != COMMON)
	    strcpy (buf, m->monstring);
	else {
	    strcpy (buf, "The ");
	    strcat (buf, m->monstring);
	}
	strcat (buf, " walked into a teleport trap!");
	mprint (buf);
    }
    m_teleport (m);
}

void m_trap_disintegrate (struct monster *m)
{
    char buf[80];
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	if (m->uniqueness != COMMON)
	    strcpy (buf, m->monstring);
	else {
	    strcpy (buf, "The ");
	    strcat (buf, m->monstring);
	}
	strcat (buf, " walked into a disintegration trap!");
	mprint (buf);
	Level->site[m->x][m->y].locchar = TRAP;
	lset (m->x, m->y, CHANGED);
    }
    disintegrate (m->x, m->y);
}

void m_trap_sleepgas (struct monster *m)
{
    char buf[80];
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	if (m->uniqueness != COMMON)
	    strcpy (buf, m->monstring);
	else {
	    strcpy (buf, "The ");
	    strcat (buf, m->monstring);
	}
	strcat (buf, " walked into a sleepgas trap!");
	mprint (buf);
	Level->site[m->x][m->y].locchar = TRAP;
	lset (m->x, m->y, CHANGED);
    }
    if (!m_immunityp (m, SLEEP))
	m_status_reset (m, AWAKE);
}

void m_trap_acid (struct monster *m)
{
    char buf[80];
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	if (m->uniqueness != COMMON)
	    strcpy (buf, m->monstring);
	else {
	    strcpy (buf, "The ");
	    strcat (buf, m->monstring);
	}
	strcat (buf, " walked into an acid bath trap!");
	mprint (buf);
	Level->site[m->x][m->y].locchar = TRAP;
	lset (m->x, m->y, CHANGED);
    }
    m_damage (m, random_range (difficulty () * difficulty ()), ACID);
}

void m_trap_manadrain (struct monster *m)
{
    char buf[80];
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	if (m->uniqueness != COMMON)
	    strcpy (buf, m->monstring);
	else {
	    strcpy (buf, "The ");
	    strcat (buf, m->monstring);
	}
	strcat (buf, " walked into a manadrain trap!");
	mprint (buf);
	Level->site[m->x][m->y].locchar = TRAP;
	lset (m->x, m->y, CHANGED);
    }
    if (m->specialf == M_SP_SPELL)
	m->specialf = M_NO_OP;
}

void m_water (struct monster *m)
{
    char buf[80];
    if ((!m_statusp (m, INTANGIBLE)) && (!m_statusp (m, SWIMMING)) && (!m_statusp (m, ONLYSWIM))) {
	if (los_p (m->x, m->y, Player.x, Player.y)) {
	    if (m->uniqueness != COMMON)
		strcpy (buf, m->monstring);
	    else {
		strcpy (buf, "The ");
		strcat (buf, m->monstring);
	    }
	    strcat (buf, " drowned!");
	    mprint (buf);
	}
	m_death (m);
    }
}

void m_abyss (struct monster *m)
{
    char buf[80];
    if (los_p (m->x, m->y, Player.x, Player.y)) {
	if (m->uniqueness != COMMON)
	    strcpy (buf, m->monstring);
	else {
	    strcpy (buf, "The ");
	    strcat (buf, m->monstring);
	}
	strcat (buf, " fell into the infinite abyss!");
	mprint (buf);
    }
    m_vanish (m);
}

void m_lava (struct monster *m)
{
    char buf[80];
    if ((!m_immunityp (m, FLAME)) || ((!m_statusp (m, SWIMMING)) && (!m_statusp (m, ONLYSWIM)))) {
	if (los_p (m->x, m->y, Player.x, Player.y)) {
	    if (m->uniqueness != COMMON)
		strcpy (buf, m->monstring);
	    else {
		strcpy (buf, "The ");
		strcat (buf, m->monstring);
	    }
	    strcat (buf, " died in a pool of lava!");
	    mprint (buf);
	}
	m_death (m);
    }
}

void m_altar (struct monster *m)
{
    int visible = view_los_p (Player.x, Player.y, m->x, m->y);
    int reaction = 0;
    int altar = Level->site[m->x][m->y].aux;

    if (visible) {
	char buf[80];
	if (m->uniqueness != COMMON)
	    strcpy (buf, m->monstring);
	else {
	    strcpy (buf, "The ");
	    strcat (buf, m->monstring);
	}
	strcat (buf, " walks next to an altar...");
	mprint (buf);
    }
    if (!m_statusp (m, HOSTILE))
	reaction = 0;
    else if (m->id == HISCORE_NPC && m->aux2 == altar)
	reaction = 1;		/* high priest of same deity */
    else if ((m->id == ANGEL || m->id == HIGH_ANGEL || m->id == ARCHANGEL) && m->aux1 == altar)
	reaction = 1;		/* angel of same deity */
    else if (altar == Player.patron)
	reaction = -1;		/* friendly deity will zap hostile monster */
    else if (((Player.patron == ODIN || Player.patron == ATHENA) && (altar == SET || altar == HECATE)) || ((Player.patron == SET || Player.patron == HECATE) && (altar == ODIN || altar == ATHENA)))
	reaction = 1;		/* hostile deity will help hostile monster */
    switch (reaction) {
	case -1:
	    if (visible) {
		mprint ("Your deity is angry!");
		mprint ("A bolt of godsfire strikes the monster....");
	    }
	    disrupt (m->x, m->y, Player.rank[PRIESTHOOD] * 50);
	    break;
	case 1:
	    if (visible) {
		mprint ("The deity of the altar smiles on the monster....");
		mprint ("A shaft of light zaps the altar...");
	    }
	    m->hp = Monsters[m->id].hp * 2;
	    break;
	default:
	    if (visible)
		mprint ("but nothing much seems to happen");
	    break;
    }
}

char *mantype (void)
{
    switch (random_range (20)) {
	case 0:
	    return "janitor";
	case 1:
	    return "beggar";
	case 2:
	    return "barbarian";
	case 3:
	    return "hairdresser";
	case 4:
	    return "accountant";
	case 5:
	    return "lawyer";
	case 6:
	    return "indian chief";
	case 7:
	    return "tinker";
	case 8:
	    return "tailor";
	case 9:
	    return "soldier";
	case 10:
	    return "spy";
	case 11:
	    return "doctor";
	case 12:
	    return "miner";
	case 13:
	    return "noble";
	case 14:
	    return "serf";
	case 15:
	    return "neer-do-well";
	case 16:
	    return "vendor";
	case 17:
	    return "dilettante";
	case 18:
	    return "surveyor";
	default:
	case 19:
	    return "jongleur";
    }
}

void strengthen_death (struct monster *m)
{
    pol ol = ((pol) checkmalloc (sizeof (oltype)));
    pob scythe = ((pob) checkmalloc (sizeof (objtype)));
    m->xpv += min (10000, m->xpv + 1000);
    m->hit += min (1000, m->hit + 10);
    m->dmg = min (10000, m->dmg * 2);
    m->ac += min (1000, m->ac + 10);
    m->speed = max (m->speed - 1, 1);
    m->movef = M_MOVE_SMART;
    m->hp = min (100000, 100 + m->dmg * 10);
    *scythe = Objects[WEAPONID + 39];
    ol->thing = scythe;
    ol->next = NULL;
    m->possessions = ol;
}

void m_no_op (struct monster *m UNUSED)
{
}
