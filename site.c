#include "glob.h"
#include <unistd.h>

/* the bank; can be broken into (!) */
void l_bank (void)
{
    int done = FALSE, valid = FALSE;
    long amount;
    char response;
    char passwd[64];
    print1 ("First Bank of Omega: Autoteller Carrel.");

    if (gamestatusp (BANK_BROKEN))
	print2 ("You see a damaged autoteller.");
    else {
	print2 ("The proximity sensor activates the autoteller as you approach.");
	morewait ();
	clearmsg ();
	while (!done) {
	    print1 ("Current Balance: ");
	    mlongprint (Balance);
	    nprint1 ("Au. ");
	    nprint1 (" Enter command (? for help) > ");
	    response = mgetc ();
	    if (response == '?') {
		menuclear ();
		menuprint ("?: This List.\n");
		if (strcmp (Password, "") == 0)
		    menuprint ("O: Open an account.\n");
		else {
		    menuprint ("P: Enter password.\n");
		    menuprint ("D: Deposit.\n");
		    menuprint ("W: Withdraw\n");
		}
		menuprint ("X: eXit\n");
		showmenu ();
		morewait ();
		xredraw ();
		continue;
	    } else if ((response == 'P') && (strcmp (Password, "") != 0)) {
		clearmsg ();
		print1 ("Password: ");
		strcpy (passwd, msgscanstring ());
		valid = (strcmp (passwd, Password) == 0);
		if (!valid) {
		    done = TRUE;
		    menuclear ();
		    menuprint ("Alert! Alert! Invalid Password!\n");
		    menuprint ("The police are being summoned!\n");
		    menuprint ("Please wait for the police to arrive....\n\n");
		    menuprint ("----Hit space bar to continue----\n");
		    showmenu ();
		    response = menugetc ();
		    if (response == ' ') {
			Player.alignment += 5;
			xredraw ();
			print1 ("Ah ha! Trying to rob the bank, eh?");
			print2 ("Take him away, boys!");
			morewait ();
			send_to_jail ();
		    } else {
			Player.alignment -= 5;
			menuclear ();
			sleep (4);
			menuprint ("^@^@^@^@^@00AD1203BC0F0000FFFFFFFFFFFF\n");
			menuprint ("Interrupt in _get_space. Illegal Character.\n");
			showmenu ();
			sleep (4);
			menuprint ("Aborting _police_alert.....\n");
			menuprint ("Attempting reboot.....\n");
			showmenu ();
			sleep (4);
			menuprint ("Warning: Illegal shmop at _count_cash.\n");
			menuprint ("Warning: Command Buffer NOT CLEARED\n");
			showmenu ();
			sleep (4);
			menuprint ("Reboot Complete. Execution Continuing.\n");
			menuprint ("Withdrawing: 4294967297 Au.\n");
			menuprint ("Warning: Arithmetic Overflow in _withdraw\n");
			showmenu ();
			sleep (4);
			menuprint ("Yo mama. Core dumped.\n");
			showmenu ();
			sleep (4);
			xredraw ();
			clearmsg ();
			print1 ("The cash machine begins to spew gold pieces!");
			print2 ("You pick up your entire balance and then some!");
			Player.cash += Balance + 1000 + random_range (3000);
			Balance = 0;
			setgamestatus (BANK_BROKEN);
		    }
		} else
		    print2 ("Password accepted. Working.");
	    } else if ((response == 'D') && valid) {
		clearmsg ();
		print1 ("Amount: ");
		amount = get_money (Player.cash);
		if (amount < 1)
		    print3 ("Transaction aborted.");
		else if (amount > Player.cash)
		    print3 ("Deposit too large -- transaction aborted.");
		else {
		    print2 ("Transaction accomplished.");
		    Balance += amount;
		    Player.cash -= amount;
		}
	    } else if ((response == 'W') && valid) {
		clearmsg ();
		print1 ("Amount: ");
		amount = get_money (Balance);
		if (amount < 1)
		    print3 ("Transaction aborted.");
		else if (amount > Balance)
		    print3 ("Withdrawal too large -- transaction aborted.");
		else {
		    print2 ("Transaction accomplished.");
		    Balance -= amount;
		    Player.cash += amount;
		}
	    } else if (response == 'X') {
		clearmsg ();
		print1 ("Bye!");
		done = TRUE;
	    } else if ((response == 'O') && (strcmp (Password, "") == 0)) {
		clearmsg ();
		print1 ("Opening new account.");
		nprint1 (" Please enter new password: ");
		strcpy (Password, msgscanstring ());
		if (strcmp (Password, "") == 0) {
		    print3 ("Illegal to use null password -- aborted.");
		    done = TRUE;
		} else {
		    print2 ("Password validated; account saved.");
		    valid = TRUE;
		}
	    } else
		print3 (" Illegal command.");
	    dataprint ();
	}
    }
    xredraw ();
}

void l_armorer (void)
{
    int done = FALSE;
    char action;
    if (hour () == 12)
	print3 ("Unfortunately, this is Julie's lunch hour -- try again later.");
    else if (nighttime ())
	print3 ("It seems that Julie keeps regular business hours.");
    else {
	while (!done) {
	    clearmsg ();
	    print1 ("Julie's: Buy Armor, Weapons, or Leave [a,w,ESCAPE] ");
	    action = mgetc ();
	    if (action == ESCAPE)
		done = TRUE;
	    else if (action == 'a')
		buyfromstock (ARMORID, 10);
	    else if (action == 'w')
		buyfromstock (WEAPONID, 23);
	}
    }
    xredraw ();
}

void buyfromstock (int base, int numitems)
{
    int i;
    char item;
    pob newitem;

    print2 ("Purchase which item? [ESCAPE to quit] ");
    menuclear ();
    for (i = 0; i < numitems; i++) {
	strcpy (Str4, " :");
	Str4[0] = i + 'a';
	strcat (Str4, Objects[base + i].objstr);
	menuprint (Str4);
	menuprint ("\n");
    }
    showmenu ();
    item = ' ';
    while ((item != ESCAPE) && ((item < 'a') || (item >= 'a' + numitems)))
	item = mgetc ();
    if (item != ESCAPE) {
	i = item - 'a';
	newitem = ((pob) checkmalloc (sizeof (objtype)));
	*newitem = Objects[base + i];
	newitem->known = 2;
	clearmsg ();
	print1 ("I can let you have it for ");
	mlongprint (2 * true_item_value (newitem));
	nprint1 ("Au. Buy it? [yn] ");
	if (ynq1 () == 'y') {
	    if (Player.cash < 2 * true_item_value (newitem)) {
		print2 ("Why not try again some time you have the cash?");
		free ((char *) newitem);
	    } else {
		Player.cash -= 2 * true_item_value (newitem);
		dataprint ();
		gain_item (newitem);
	    }
	} else
	    free ((char *) newitem);
    }
}

void l_club (void)
{
#define hinthour club_hinthour
    char response;

    print1 ("Rampart Explorers' Club.");
    if (!gamestatusp (CLUB_MEMBER)) {
	if (Player.level < 2)
	    print3 ("Only reknowned adventurers need apply.");
	else {
	    print2 ("Dues are 100Au. Pay it? [yn] ");
	    if (ynq2 () == 'y') {
		if (Player.cash < 100)
		    print3 ("Beat it, or we'll blackball you!");
		else {
		    print1 ("Welcome to the club! You are taught the spell of Return.");
		    print2 ("When cast on the first level of a dungeon it");
		    morewait ();
		    clearmsg ();
		    print1 ("will transport you down to the lowest level");
		    print2 ("you have explored, and vice versa.");
		    Spells[S_RETURN].known = TRUE;
		    Player.cash -= 100;
		    setgamestatus (CLUB_MEMBER);
		}
	    } else
		print2 ("OK, but you're missing out on our benefits....");
	}
    } else {
	print2 ("Shop at the club store or listen for rumors [sl] ");
	do
	    response = (char) mcigetc ();
	while ((response != 's') && (response != 'l') && (response != ESCAPE));
	if (response == 'l') {
	    if (hinthour == hour ())
		print2 ("You don't hear anything useful.");
	    else {
		print1 ("You overhear a conversation....");
		hint ();
		hinthour = hour ();
	    }
	} else if (response == 's') {
	    buyfromstock (THINGID + 7, 2);
	    xredraw ();
	} else if (response == ESCAPE)
	    print2 ("Be seeing you, old chap!");
    }
}

#undef hinthour

void l_gym (void)
{
    int done = TRUE;
    int trained = 0;
    clearmsg ();
    do {
	print1 ("The Rampart Gymnasium");
	if ((Gymcredit > 0) || (Player.rank[ARENA])) {
	    nprint1 ("-- Credit: ");
	    mlongprint (Gymcredit);
	    nprint1 ("Au.");
	}
	done = FALSE;
	menuclear ();
	menuprint ("Train for 2000 Au. Choose:\n");
	menuprint ("\na: work out in the weight room");
	menuprint ("\nb: use our gymnastics equipment");
	menuprint ("\nc: take our new anaerobics course");
	menuprint ("\nd: enroll in dance lessons.");
	menuprint ("\nESCAPE: Leave this place.");
	showmenu ();
	switch (mgetc ()) {
	    case 'a':
		gymtrain (&(Player.maxstr), &(Player.str));
		break;
	    case 'b':
		gymtrain (&(Player.maxdex), &(Player.dex));
		break;
	    case 'c':
		gymtrain (&(Player.maxcon), &(Player.con));
		break;
	    case 'd':
		gymtrain (&(Player.maxagi), &(Player.agi));
		break;
	    case ESCAPE:
		clearmsg ();
		if (trained == 0)
		    print1 ("Well, it's your body you're depriving!");
		else if (trained < 3)
		    print1 ("You towel yourself off, and find the exit.");
		else
		    print1 ("A refreshing bath, and you're on your way.");
		done = TRUE;
		break;
	    default:
		trained--;
		break;
	}
	trained++;
    } while (!done);
    xredraw ();
    calc_melee ();
}

void l_healer (void)
{
    print1 ("Rampart Healers. Member RMA.");
    morewait ();
    clearmsg ();
    print1 ("a: Heal injuries (50 crowns)");
    print2 ("b: Cure disease (250 crowns)");
    print3 ("ESCAPE: Leave these antiseptic alcoves.");
    switch ((char) mcigetc ()) {
	case 'a':
	    healforpay ();
	    break;
	case 'b':
	    cureforpay ();
	    break;
	default:
	    print3 ("OK, but suppose you have Acute Satyriasis?");
	    break;
    }
}

void statue_random (int x, int y)
{
    pob item;
    int i, j;
    switch (random_range (difficulty () + 3) - 1) {
	default:
	    l_statue_wake ();
	    break;
	case 0:
	    print1 ("The statue crumbles with a clatter of gravel.");
	    Level->site[x][y].locchar = RUBBLE;
	    Level->site[x][y].p_locf = L_RUBBLE;
	    plotspot (x, y, TRUE);
	    lset (x, y, CHANGED);
	    break;
	case 1:
	    print1 ("The statue stoutly resists your attack.");
	    break;
	case 2:
	    print1 ("The statue crumbles with a clatter of gravel.");
	    Level->site[x][y].locchar = RUBBLE;
	    Level->site[x][y].p_locf = L_RUBBLE;
	    plotspot (x, y, TRUE);
	    lset (x, y, CHANGED);
	    make_site_treasure (x, y, difficulty ());
	    break;
	case 3:
	    print1 ("The statue hits you back!");
	    p_damage (random_range (difficulty () * 5), UNSTOPPABLE, "a statue");
	    break;
	case 4:
	    print1 ("The statue looks slightly pained. It speaks:");
	    morewait ();
	    clearmsg ();
	    hint ();
	    break;
	case 5:
	    if ((Current_Environment == Current_Dungeon) || (Current_Environment == E_CITY)) {
		print1 ("You hear the whirr of some mechanism.");
		print2 ("The statue glides smoothly into the floor!");
		/* WDT HACK: I shouldn't be making this choice on a level
		 * where no stairs can be (or perhaps I should, and I should
		 * implement a bonus level!). */
		Level->site[x][y].locchar = STAIRS_DOWN;
		Level->site[x][y].p_locf = L_NO_OP;
		lset (x, y, CHANGED | STOPS);
	    }
	    break;
	case 6:
	    print1 ("The statue was covered with contact cement!");
	    print2 ("You can't move....");
	    Player.status[IMMOBILE] += random_range (6) + 2;
	    break;
	case 7:
	    print1 ("A strange radiation emanates from the statue!");
	    dispel (-1);
	    break;
	case 8:		/* I think this is particularly evil. Heh heh. */
	    if (Player.possessions[O_WEAPON_HAND] != NULL) {
		print1 ("Your weapon sinks deeply into the statue and is sucked away!");
		item = Player.possessions[O_WEAPON_HAND];
		conform_lost_object (Player.possessions[O_WEAPON_HAND]);
		item->blessing = -1 - abs (item->blessing);
		drop_at (x, y, item);
	    }
	    break;
	case 9:
	    print1 ("The statue extends an arm. Beams of light illuminate the level!");
	    for (i = 0; i < WIDTH; i++)
		for (j = 0; j < LENGTH; j++) {
		    lset (i, j, SEEN);
		    if (loc_statusp (i, j, SECRET)) {
			lreset (i, j, SECRET);
			lset (i, j, CHANGED);
		    }
		}
	    show_screen ();
	    break;
    }
}

void l_statue_wake (void)
{
    int i;
    int x = Player.x, y = Player.y;
    for (i = 0; i < 9; i++)
	wake_statue (x + Dirs[0][i], y + Dirs[1][i], TRUE);
}

void wake_statue (int x, int y, int first)
{
    int i;
    pml tml;
    if (Level->site[x][y].locchar == STATUE) {
	if (!first)
	    mprint ("Another statue awakens!");
	else
	    mprint ("A statue springs to life!");
	Level->site[x][y].locchar = FLOOR;
	lset (x, y, CHANGED);
	tml = ((pml) checkmalloc (sizeof (mltype)));
	tml->m = (Level->site[x][y].creature = m_create (x, y, 0, difficulty () + 1));
	m_status_set (Level->site[x][y].creature, HOSTILE);
	tml->next = Level->mlist;
	Level->mlist = tml;
	for (i = 0; i < 8; i++)
	    wake_statue (x + Dirs[0][i], y + Dirs[1][i], FALSE);
    }
}

void l_casino (void)
{
    int i, done = FALSE, a, b, c, match;
    char response;
    print1 ("Rampart Mithril Nugget Casino.");
    if (random_range (10) == 1)
	print2 ("Casino closed due to Grand Jury investigation.");
    else {
	while (!done) {
	    morewait ();
	    clearmsg ();
	    print1 ("a: Drop 100Au in the slots.");
	    print2 ("b: Risk 1000Au  at roulette.");
	    print3 ("ESCAPE: Leave this green baize hall.");
	    response = (char) mcigetc ();
	    if (response == 'a') {
		if (Player.cash < 100)
		    print3 ("No credit, jerk.");
		else {
		    Player.cash -= 100;
		    dataprint ();
		    for (i = 0; i < 20; i++) {
			if (i == 19)
			    sleep (1);
			else
			    usleep (250000);
			a = random_range (10);
			b = random_range (10);
			c = random_range (10);
			clearmsg1 ();
			mprint (slotstr (a));
			mprint (slotstr (b));
			mprint (slotstr (c));
		    }
		    if (winnings > 0)
			do {
			    a = random_range (10);
			    b = random_range (10);
			    c = random_range (10);
			} while ((a == b) || (a == c) || (b == c));
		    else {
			a = random_range (10);
			b = random_range (10);
			c = random_range (10);
		    }
		    clearmsg ();
		    mprint (slotstr (a));
		    mprint (slotstr (b));
		    mprint (slotstr (c));
		    if ((a == b) && (a == c)) {
			print3 ("Jackpot Winner!");
			winnings += (a + 2) * (b + 2) * (c + 2) * 5;
			Player.cash += (a + 2) * (b + 2) * (c + 2) * 5;
			dataprint ();
		    } else if (a == b) {
			print3 ("Winner!");
			Player.cash += (a + 2) * (b + 2) * 5;
			dataprint ();
			winnings += (a + 2) * (b + 2) * 5;
		    } else if (a == c) {
			print3 ("Winner!");
			Player.cash += (a + 2) * (c + 2) * 5;
			dataprint ();
			winnings += (a + 2) * (c + 2) * 5;
		    } else if (c == b) {
			print3 ("Winner!");
			Player.cash += (c + 2) * (b + 2) * 5;
			dataprint ();
			winnings += (c + 2) * (b + 2) * 5;
		    } else {
			print3 ("Loser!");
			winnings -= 100;
		    }
		}
	    } else if (response == 'b') {
		if (Player.cash < 1000)
		    mprint ("No credit, jerk.");
		else {
		    Player.cash -= 1000;
		    dataprint ();
		    print1 ("Red or Black? [rb]");
		    do
			response = (char) mcigetc ();
		    while ((response != 'r') && (response != 'b'));
		    match = (response == 'r' ? 0 : 1);
		    for (i = 0; i < 20; i++) {
			if (i == 19)
			    sleep (1);
			else
			    usleep (250000);
			a = random_range (37);
			b = a % 2;
			if (a == 0)
			    print1 (" 0 ");
			else if (a == 1)
			    print1 (" 0 - 0 ");
			else {
			    print1 ((b == 0) ? "Red " : "Black ");
			    mnumprint (a - 1);
			}
		    }
		    if (winnings > 0)
			do {
			    a = random_range (37);
			    b = a % 2;
			} while (b == match);
		    else {
			a = random_range (37);
			b = a % 2;
		    }
		    if (a == 0)
			print1 (" 0 ");
		    else if (a == 1)
			print1 (" 0 - 0 ");
		    else {
			print1 ((b == 0) ? "Red " : "Black ");
			mnumprint (a - 1);
		    }
		    if ((a > 1) && (b == match)) {
			print3 (" Winner!");
			winnings += 1000;
			Player.cash += 2000;
			dataprint ();
		    } else {
			print3 (" Loser!");
			winnings -= 1000;
			dataprint ();
		    }
		}
	    } else if (response == ESCAPE)
		done = TRUE;
	}
    }
}

void l_commandant (void)
{
    int num;
    pob food;
    print1 ("Commandant Sonder's Rampart-fried Lyzzard partes. Open 24 hrs.");
    print2 ("Buy a bucket! Only 5 Au. Make a purchase? [yn] ");
    if (ynq2 () == 'y') {
	clearmsg ();
	print1 ("How many? ");
	num = (int) parsenum ();
	if (num < 1)
	    print3 ("Cute. Real cute.");
	else if (num * 5 > Player.cash)
	    print3 ("No handouts here, mac!");
	else {
	    Player.cash -= num * 5;
	    food = ((pob) checkmalloc (sizeof (objtype)));
	    *food = Objects[FOODID + 0];	/* food ration */
	    food->number = num;
	    if (num == 1)
		print2 ("There you go, mac! One Lyzzard Bucket, coming up.");
	    else
		print2 ("A passel of Lyzzard Buckets, for your pleasure.");
	    morewait ();
	    gain_item (food);
	}
    } else
	print2 ("Don't blame the Commandant if you starve!");
}

void l_diner (void)
{
    print1 ("The Rampart Diner. All you can eat, 25Au.");
    print2 ("Place an order? [yn] ");
    if (ynq2 () == 'y') {
	if (Player.cash < 25)
	    mprint ("TANSTAAFL! Now git!");
	else {
	    Player.cash -= 25;
	    dataprint ();
	    Player.food = 44;
	    foodcheck ();
	}
    }
}

void l_crap (void)
{
    print1 ("Les Crapeuleaux. (****) ");
    if ((hour () < 17) || (hour () > 23))
	print2 ("So sorry, we are closed 'til the morrow...");
    else {
	print2 ("May I take your order? [yn] ");
	if (ynq2 () == 'y') {
	    if (Player.cash < 1000)
		print2 ("So sorry, you have not the funds for dinner.");
	    else {
		print2 ("Hope you enjoyed your tres expensive meal, m'sieur...");
		Player.cash -= 1000;
		dataprint ();
		Player.food += 8;
		foodcheck ();
	    }
	}
    }
}

void l_tavern (void)
{
#define hinthour tavern_hinthour
    char response;
    print1 ("The Centaur and Nymph -- J. Riley, prop.");
    if (nighttime ()) {
	menuclear ();
	menuprint ("Riley says: Whataya have?\n\n");
	menuprint ("a: Pint of Riley's ultra-dark 1Au\n");
	menuprint ("b: Shot of Tullimore Dew 10Au\n");
	menuprint ("c: Round for the House. 100Au\n");
	menuprint ("d: Bed and Breakfast. 25Au\n");
	menuprint ("ESCAPE: Leave this comfortable haven.\n");
	showmenu ();
	do
	    response = (char) mcigetc ();
	while ((response != 'a') && (response != 'b') && (response != 'c') && (response != 'd') && (response != ESCAPE));
	switch (response) {
	    case 'a':
		if (Player.cash < 1)
		    print2 ("Aw hell, have one on me.");
		else {
		    Player.cash -= 1;
		    dataprint ();
		    if (hinthour != hour ()) {
			if (random_range (3)) {
			    print1 ("You overhear a rumor...");
			    hint ();
			} else
			    print1 ("You don't hear much of interest.");
			hinthour = hour ();
		    } else
			print1 ("You just hear the same conversations again.");
		}
		break;
	    case 'b':
		if (Player.cash < 10)
		    print2 ("I don't serve the Dew on no tab, buddy!");
		else {
		    Player.cash -= 10;
		    print1 ("Ahhhhh....");
		    if (Player.status[POISONED] || Player.status[DISEASED])
			print2 ("Phew! That's, er, smooth stuff!");
		    Player.status[POISONED] = 0;
		    Player.status[DISEASED] = 0;
		    showflags ();
		}
		break;
	    case 'c':
		if (Player.cash < 100) {
		    print1 ("Whatta feeb!");
		    print2 ("Outta my establishment.... Now!");
		    p_damage (random_range (20), UNSTOPPABLE, "Riley's right cross");
		    morewait ();
		} else {
		    Player.cash -= 100;
		    dataprint ();
		    print1 ("'What a guy!'");
		    morewait ();
		    print2 ("'Hey, thanks, fella.'");
		    morewait ();
		    print3 ("'Make mine a double...'");
		    morewait ();
		    clearmsg ();
		    switch (random_range (4)) {
			case 0:
			    print1 ("'You're a real pal. Say, have you heard.... ");
			    hint ();
			    break;
			case 1:
			    print1 ("A wandering priest of Dionysus blesses you...");
			    if ((Player.patron == ODIN) || (Player.patron == ATHENA))
				Player.alignment++;
			    else if ((Player.patron == HECATE) || (Player.patron == SET))
				Player.alignment--;
			    else if (Player.alignment > 0)
				Player.alignment--;
			    else
				Player.alignment++;
			    break;
			case 2:
			    print1 ("A thirsty bard promises to put your name in a song!");
			    gain_experience (20);
			    break;
			case 3:
			    print1 ("Riley draws you a shot of his 'special reserve'");
			    print2 ("Drink it [yn]?");
			    if (ynq2 () == 'y') {
				if (Player.con < random_range (20)) {
				    print1 ("<cough> Quite a kick!");
				    print2 ("You feel a fiery warmth in your tummy....");
				    Player.con++;
				    Player.maxcon++;
				} else
				    print2 ("You toss it back nonchalantly.");
			    }
		    }
		}
		break;
	    case 'd':
		if (Player.cash < 25)
		    print2 ("Pay in advance, mac!");
		else {
		    Player.cash -= 25;
		    print2 ("How about a shot o' the dew for a nightcap?");
		    morewait ();
		    Time += (6 + random_range (4)) * 60;
		    Player.status[POISONED] = 0;
		    Player.status[DISEASED] = 0;
		    Player.food = 40;
		    /* reduce temporary stat gains to max stat levels */
		    toggle_item_use (TRUE);
		    Player.str = min (Player.str, Player.maxstr);
		    Player.con = min (Player.con, Player.maxcon);
		    Player.agi = min (Player.agi, Player.maxagi);
		    Player.dex = min (Player.dex, Player.maxdex);
		    Player.iq = min (Player.iq, Player.maxiq);
		    Player.pow = min (Player.pow, Player.maxpow);
		    toggle_item_use (FALSE);
		    timeprint ();
		    dataprint ();
		    showflags ();
		    print1 ("The next day.....");
		    if (hour () > 10)
			print2 ("Oh my! You overslept!");
		}
		break;
	    default:
		print2 ("So? Just looking? Go on!");
		break;
	}
    } else
	print2 ("The pub don't open til dark, fella.");
    xredraw ();
}

#undef hinthour

void l_alchemist (void)
{
    int i, done = FALSE, mlevel;
    char response;
    pob obj;
    print1 ("Ambrosias' Potions et cie.");
    if (nighttime ())
	print2 ("Ambrosias doesn't seem to be in right now.");
    else
	while (!done) {
	    morewait ();
	    clearmsg ();
	    print1 ("a: Sell monster components.");
	    print2 ("b: Pay for transformation.");
	    print3 ("ESCAPE: Leave this place.");
	    response = (char) mcigetc ();
	    if (response == 'a') {
		clearmsg ();
		done = TRUE;
		i = getitem (CORPSE);
		if ((i != ABORT) && (Player.possessions[i] != NULL)) {
		    obj = Player.possessions[i];
		    if (Monsters[obj->charge].transformid == -1) {
			print1 ("I don't want such a thing.");
			if (obj->basevalue > 0)
			    print2 ("You might be able to sell it to someone else, though.");
		    } else {
			clearmsg ();
			print1 ("I'll give you ");
			mnumprint (obj->basevalue / 3);
			nprint1 ("Au for it. Take it? [yn] ");
			if (ynq1 () == 'y') {
			    Player.cash += (obj->basevalue / 3);
			    conform_lost_objects (1, obj);
			} else
			    print2 ("Well, keep the smelly old thing, then!");
		    }
		} else
		    print2 ("So nu?");
	    } else if (response == 'b') {
		clearmsg ();
		done = TRUE;
		i = getitem (CORPSE);
		if ((i != ABORT) && (Player.possessions[i] != NULL)) {
		    obj = Player.possessions[i];
		    if (Monsters[obj->charge].transformid == -1)
			print1 ("Oy vey! You want me to transform such a thing?");
		    else {
			mlevel = Monsters[obj->charge].level;
			print1 ("It'll cost you ");
			mnumprint (max (10, obj->basevalue * 2));
			nprint1 ("Au for the transformation. Pay it? [yn] ");
			if (ynq1 () == 'y') {
			    if (Player.cash < max (10, obj->basevalue * 2))
				print2 ("You can't afford it!");
			    else {
				print1 ("Voila! A tap of the Philosopher's Stone...");
				Player.cash -= max (10, obj->basevalue * 2);
				*obj = Objects[Monsters[obj->charge].transformid];
				if ((obj->id >= STICKID) && (obj->id < STICKID + NUMSTICKS))
				    obj->charge = 20;
				if (obj->plus == 0)
				    obj->plus = mlevel;
				if (obj->blessing == 0)
				    obj->blessing = 1;
			    }
			} else
			    print2 ("I don't need your business, anyhow.");
		    }
		} else
		    print2 ("So nu?");
	    } else if (response == ESCAPE)
		done = TRUE;
	}
}

void l_dpw (void)
{
    print1 ("Rampart Department of Public Works.");
    if (Date - LastDay < 7)
	print2 ("G'wan! Get a job!");
    else if (Player.cash < 100) {
	print2 ("Do you want to go on the dole? [yn] ");
	if (ynq2 () == 'y') {
	    print1 ("Well, ok, but spend it wisely.");
	    morewait ();
	    print1 ("Please enter your name for our records:");
	    strcpy (Str1, msgscanstring ());
	    if (Str1[0] >= 'a' && Str1[0] <= 'z')
		Str1[0] += 'A' - 'a';
	    if (Str1[0] == '\0')
		print1 ("Maybe you should come back when you've learned to write.");
	    else if (strcmp (Player.name, Str1) != 0) {
		print3 ("Aha! Welfare Fraud! It's off to gaol for you, lout!");
		morewait ();
		send_to_jail ();
	    } else {
		print2 ("Here's your handout, layabout!");
		LastDay = Date;
		Player.cash = 99;
		dataprint ();
	    }
	}
    } else
	print2 ("You're too well off for us to help you!");
}

void l_library (void)
{
    char response;
    int studied = FALSE;
    int done = FALSE, fee = 1000;
    print1 ("Rampart Public Library.");
    if (nighttime ())
	print2 ("CLOSED");
    else {
	morewait ();
	print1 ("Library Research Fee: 1000Au.");
	if (Player.maxiq < 18) {
	    print2 ("The Rampart student aid system has arranged a grant!");
	    morewait ();
	    clearmsg ();
	    print1 ("Your revised fee is: ");
	    mnumprint (fee = max (50, 1000 - (18 - Player.maxiq) * 125));
	    nprint1 ("Au.");
	}
	morewait ();
	while (!done) {
	    print1 ("Pay the fee? [yn] ");
	    if (ynq1 () == 'y') {
		if (Player.cash < fee) {
		    print2 ("No payee, No studee.");
		    done = TRUE;
		} else {
		    Player.cash -= fee;
		    do {
			studied = TRUE;
			dataprint ();
			menuclear ();
			menuprint ("Peruse a scroll:\n");
			menuprint ("a: Omegan Theology\n");
			menuprint ("b: Guide to Rampart\n");
			menuprint ("c: High Magick\n");
			menuprint ("d: Odd Uncatalogued Document\n");
			menuprint ("e: Attempt Advanced Research\n");
			menuprint ("ESCAPE: Leave this font of learning.\n");
			showmenu ();
			response = (char) mcigetc ();
			if (response == 'a') {
			    print1 ("You unfurl an ancient, yellowing scroll...");
			    morewait ();
			    theologyfile ();
			} else if (response == 'b') {
			    print1 ("You unroll a slick four-color document...");
			    morewait ();
			    cityguidefile ();
			} else if (response == 'c') {
			    print1 ("This scroll is written in a strange magical script...");
			    morewait ();
			    wishfile ();
			} else if (response == 'd') {
			    print1 ("You find a strange document, obviously misfiled");
			    print2 ("under the heading 'acrylic fungus painting technique'");
			    morewait ();
			    adeptfile ();
			} else if (response == 'e') {
			    if (random_range (30) > Player.iq) {
				print2 ("You feel more knowledgeable!");
				Player.iq++;
				Player.maxiq++;
				dataprint ();
				if (Player.maxiq < 19 && fee != max (50, 1000 - (18 - Player.maxiq) * 125)) {
				    morewait ();
				    clearmsg ();
				    print1 ("Your revised fee is: ");
				    mnumprint (fee = max (50, 1000 - (18 - Player.maxiq) * 125));
				    nprint1 ("Au.");
				    morewait ();
				}
			    } else {
				clearmsg1 ();
				print1 ("You find advice in an ancient tome: ");
				morewait ();
				hint ();
				morewait ();
			    }
			} else if (response == ESCAPE) {
			    done = TRUE;
			    print1 ("That was an expensive browse...");
			} else
			    studied = FALSE;
		    } while (!studied);
		}
		xredraw ();
	    } else {
		done = TRUE;
		if (studied)
		    print2 ("Come back anytime we're open, 7am to 8pm.");
		else
		    print2 ("You philistine!");
	    }
	}
    }
}

void l_pawn_shop (void)
{
    int i, j, k, limit, number, done = FALSE;
    char item, action;

    if (nighttime ())
	print1 ("Shop Closed: Have a Nice (K)Night");
    else {
	limit = min (5, Date - Pawndate);
	Pawndate = Date;
	for (k = 0; k < limit; k++) {
	    if (Pawnitems[0] != NULL) {
		if (Objects[Pawnitems[0]->id].uniqueness > UNIQUE_UNMADE)
		    Objects[Pawnitems[0]->id].uniqueness = UNIQUE_UNMADE;
		/* could turn up anywhere, really :) */
		free ((char *) Pawnitems[0]);
		Pawnitems[0] = NULL;
	    }
	    for (i = 0; i < PAWNITEMS - 1; i++)
		Pawnitems[i] = Pawnitems[i + 1];
	    Pawnitems[PAWNITEMS - 1] = NULL;
	    for (i = 0; i < PAWNITEMS; i++)
		if (Pawnitems[i] == NULL)
		    do {
			if (Pawnitems[i] != NULL)
			    free (Pawnitems[i]);
			Pawnitems[i] = create_object (5);
			Pawnitems[i]->known = 2;
		    } while ((Pawnitems[i]->objchar == CASH) || (Pawnitems[i]->objchar == ARTIFACT) || (true_item_value (Pawnitems[i]) <= 0));
	}
	while (!done) {
	    print1 ("Knight's Pawn Shop:");
	    print2 ("Buy item, Sell item, sell Pack contents, Leave [b,s,p,ESCAPE] ");
	    menuclear ();
	    for (i = 0; i < PAWNITEMS; i++)
		if (Pawnitems[i] != NULL) {
		    strcpy (Str3, " :");
		    Str3[0] = i + 'a';
		    strcat (Str3, itemid (Pawnitems[i]));
		    menuprint (Str3);
		    menuprint ("\n");
		}
	    showmenu ();
	    action = (char) mcigetc ();
	    if (action == ESCAPE)
		done = TRUE;
	    else if (action == 'b') {
		print2 ("Purchase which item? [ESCAPE to quit] ");
		item = ' ';
		while ((item != ESCAPE) && ((item < 'a') || (item >= 'a' + PAWNITEMS)))
		    item = (char) mcigetc ();
		if (item != ESCAPE) {
		    i = item - 'a';
		    if (Pawnitems[i] == NULL)
			print3 ("No such item!");
		    else if (true_item_value (Pawnitems[i]) <= 0) {
			print1 ("Hmm, how did that junk get on my shelves?");
			print2 ("I'll just remove it.");
			free ((char *) Pawnitems[i]);
			Pawnitems[i] = NULL;
		    } else {
			clearmsg ();
			print1 ("The low, low, cost is: ");
			mlongprint (Pawnitems[i]->number * true_item_value (Pawnitems[i]));
			nprint1 (" Buy it? [ynq] ");
			if (ynq1 () == 'y') {
			    if (Player.cash < Pawnitems[i]->number * true_item_value (Pawnitems[i])) {
				print2 ("No credit! Gwan, Beat it!");
				morewait ();
			    } else {
				Player.cash -= Pawnitems[i]->number * true_item_value (Pawnitems[i]);
				Objects[Pawnitems[i]->id].known = 1;
				gain_item (Pawnitems[i]);
				Pawnitems[i] = NULL;
			    }
			}
		    }
		}
	    } else if (action == 's') {
		menuclear ();
		print2 ("Sell which item: ");
		i = getitem (NULL_ITEM);
		if ((i != ABORT) && (Player.possessions[i] != NULL)) {
		    if (cursed (Player.possessions[i])) {
			print1 ("No loans on cursed items! I been burned before....");
			morewait ();
		    } else if (true_item_value (Player.possessions[i]) <= 0) {
			print1 ("That looks like a worthless piece of junk to me.");
			morewait ();
		    } else {
			clearmsg ();
			print1 ("You can get ");
			mlongprint (item_value (Player.possessions[i]) / 2);
			nprint1 ("Au each. Sell [yn]? ");
			if (ynq1 () == 'y') {
			    number = getnumber (Player.possessions[i]->number);
			    if ((number >= Player.possessions[i]->number) && Player.possessions[i]->used) {
				Player.possessions[i]->used = FALSE;
				item_use (Player.possessions[i]);
			    }
			    Player.cash += number * item_value (Player.possessions[i]) / 2;
			    free ((char *) Pawnitems[0]);
			    for (j = 0; j < PAWNITEMS - 1; j++)
				Pawnitems[j] = Pawnitems[j + 1];
			    Pawnitems[PAWNITEMS - 1] = ((pob) checkmalloc (sizeof (objtype)));
			    *(Pawnitems[PAWNITEMS - 1]) = *(Player.possessions[i]);
			    Pawnitems[PAWNITEMS - 1]->number = number;
			    Pawnitems[PAWNITEMS - 1]->known = 2;
			    dispose_lost_objects (number, Player.possessions[i]);
			    dataprint ();
			}
		    }
		}
	    } else if (action == 'p') {
		for (i = 0; i < Player.packptr; i++) {
		    if (Player.pack[i]->blessing > -1 && true_item_value (Player.pack[i]) > 0) {
			clearmsg ();
			print1 ("Sell ");
			nprint1 (itemid (Player.pack[i]));
			nprint1 (" for ");
			mlongprint (item_value (Player.pack[i]) / 2);
			nprint1 ("Au each? [yn] ");
			if (ynq1 () == 'y') {
			    number = getnumber (Player.pack[i]->number);
			    if (number > 0) {
				Player.cash += number * item_value (Player.pack[i]) / 2;
				free ((char *) Pawnitems[0]);
				for (j = 0; j < PAWNITEMS - 1; j++)
				    Pawnitems[j] = Pawnitems[j + 1];
				Pawnitems[PAWNITEMS - 1] = ((pob) checkmalloc (sizeof (objtype)));
				*(Pawnitems[PAWNITEMS - 1]) = *(Player.pack[i]);
				Pawnitems[PAWNITEMS - 1]->number = number;
				Pawnitems[PAWNITEMS - 1]->known = 2;
				Player.pack[i]->number -= number;
				if (Player.pack[i]->number < 1) {
				    free ((char *) Player.pack[i]);
				    Player.pack[i] = NULL;
				}
				dataprint ();
			    }
			}
		    }
		}
		fixpack ();
	    }
	}
    }
    calc_melee ();
    xredraw ();
}

void l_condo (void)
{
    pol ol, prev = NULL;
    int i, done = FALSE, over = FALSE, weeksleep = FALSE;
    char response;

    if (!gamestatusp (SOLD_CONDO)) {
	print1 ("Rampart Arms. Weekly Rentals and Purchases");
	print2 ("Which are you interested in [r,p, or ESCAPE] ");
	response = mgetc ();
	if (response == 'p') {
	    print2 ("Only 50,000Au. Buy it? [yn] ");
	    if (ynq2 () == 'y') {
		if (Player.cash < 50000)
		    print3 ("No mortgages, buddy.");
		else {
		    setgamestatus (SOLD_CONDO);
		    Player.cash -= 50000;
		    dataprint ();
		    print2 ("You are the proud owner of a luxurious condo penthouse.");
		    Condoitems = NULL;
		}
	    }
	} else if (response == 'r') {
	    print2 ("Weekly Rental, 1000Au. Pay for it? [yn] ");
	    if (ynq2 () == 'y') {
		if (Player.cash < 1000)
		    print2 ("Hey, pay the rent or out you go....");
		else {
		    weeksleep = TRUE;
		    Player.cash -= 1000;
		    dataprint ();
		}
	    }
	} else
	    print2 ("Please keep us in mind for your housing needs.");
    } else {
	while (!done) {
	    menuclear ();
	    menuprint ("Home Sweet Home\n");
	    menuprint ("a: Leave items in your safe.\n");
	    menuprint ("b: Retrieve items.\n");
	    menuprint ("c: Take a week off to rest.\n");
	    menuprint ("d: Retire permanently.\n");
	    menuprint ("ESCAPE: Leave this place.\n");
	    showmenu ();
	    response = (char) mcigetc ();
	    if (response == 'a') {
		i = getitem (NULL_ITEM);
		if (i != ABORT) {
		    if (Player.possessions[i]->blessing < 0)
			print2 ("The item just doesn't want to be stored away...");
		    else {
			ol = ((pol) checkmalloc (sizeof (oltype)));
			ol->thing = Player.possessions[i];
			ol->next = Condoitems;
			Condoitems = ol;
			conform_unused_object (Player.possessions[i]);
			Player.possessions[i] = NULL;
		    }
		}
	    } else if (response == 'b') {
		ol = Condoitems;
		while ((ol != NULL) && (!over)) {
		    print1 ("Retrieve ");
		    nprint1 (itemid (ol->thing));
		    nprint1 (" [ynq] ");
		    response = (char) mcigetc ();
		    if (response == 'y') {
			gain_item (ol->thing);
			if (ol == Condoitems)
			    Condoitems = Condoitems->next;
			else if (prev != NULL)
			    prev->next = ol->next;
		    } else if (response == 'q')
			over = TRUE;
		    prev = ol;
		    ol = ol->next;
		}
	    } else if (response == 'c') {
		weeksleep = TRUE;
		print1 ("You take a week off to rest...");
		morewait ();
	    } else if (response == 'd') {
		clearmsg ();
		print1 ("You sure you want to retire, now? [yn] ");
		if (ynq1 () == 'y') {
		    p_win ();
		}
	    } else if (response == ESCAPE)
		done = TRUE;
	}
	xredraw ();
    }
    if (weeksleep) {
	clearmsg ();
	print1 ("Taking a week off to rest...");
	morewait ();
	toggle_item_use (TRUE);
	Player.hp = Player.maxhp;
	Player.str = Player.maxstr;
	Player.agi = Player.maxagi;
	Player.con = Player.maxcon;
	Player.dex = Player.maxdex;
	Player.iq = Player.maxiq;
	Player.pow = Player.maxpow;
	for (i = 0; i < NUMSTATI; i++)
	    if (Player.status[i] < 1000)
		Player.status[i] = 0;
	toggle_item_use (FALSE);
	Player.food = 36;
	print2 ("You're once again fit and ready to continue your adventure.");
	Time += 60 * 24 * 7;
	Date += 7;
	moon_check ();
	timeprint ();
    }
}

void gymtrain (int *maxstat, int *stat)
{
    if (Gymcredit + Player.cash < 2000)
	print2 ("You can't afford our training!");
    else {
	if (Gymcredit > 2000)
	    Gymcredit -= 2000;
	else {
	    Player.cash -= (2000 - Gymcredit);
	    Gymcredit = 0;
	}
	print2 ("Sweat. Sweat. ");
	if ((*maxstat < 30) && ((*maxstat < random_range (30)) || (random_range (3) == 1))) {
	    nprint2 ("The training pays off!");
	    (*maxstat)++;
	    (*stat)++;
	} else {
	    nprint2 ("You feel the healthy glow of a good workout.");
	    if (*stat < *maxstat) {
		(*stat)++;
		print3 ("A feeling of rehabilitation washes through you.");
	    }
	}
    }
    dataprint ();
}

void healforpay (void)
{
    if (Player.cash < 50)
	print2 ("You can't afford to be healed!");
    else {
	Player.cash -= 50;
	Player.hp += 20 + random_range (20);
	if (Player.hp > Player.maxhp)
	    Player.hp = Player.maxhp;
	print2 ("Another medical marvel....");
    }
    calc_melee ();
}

void cureforpay (void)
{
    if (Player.cash < 250)
	print2 ("You can't afford to be cured!");
    else {
	Player.cash -= 250;
	Player.status[DISEASED] = 0;
	print2 ("Quarantine lifted....");
	showflags ();
    }
}

void pacify_guards (void)
{
    pml ml;

    for (ml = Level->mlist; ml != NULL; ml = ml->next)
	if ((ml->m->id == GUARD) ||	/*guard */
	    ((ml->m->id == HISCORE_NPC) && (ml->m->aux2 == 15))) {	/* justiciar */
	    m_status_reset (ml->m, HOSTILE);
	    ml->m->specialf = M_NO_OP;
	    if (ml->m->id == GUARD && ml->m->hp > 0 && ml->m->aux1 > 0) {
		if (Level->site[ml->m->x][ml->m->y].creature == ml->m)
		    Level->site[ml->m->x][ml->m->y].creature = NULL;
		ml->m->x = ml->m->aux1;
		ml->m->y = ml->m->aux2;
		Level->site[ml->m->x][ml->m->y].creature = ml->m;
	    } else if (ml->m->id == HISCORE_NPC && ml->m->hp > 0 && Current_Environment == E_CITY) {
		if (Level->site[ml->m->x][ml->m->y].creature == ml->m)
		    Level->site[ml->m->x][ml->m->y].creature = NULL;
		ml->m->x = 40;
		ml->m->y = 62;
		Level->site[ml->m->x][ml->m->y].creature = ml->m;
	    }
	}
    if (Current_Environment == E_CITY)
	Level->site[40][60].p_locf = L_ORDER;	/* undoes action in alert_guards */
}

void send_to_jail (void)
{
    if (Player.rank[ORDER] > 0) {
	print1 ("A member of the Order of Paladins sent to jail!");
	print2 ("It cannot be!");
	morewait ();
	print1 ("You are immediately expelled permanently from the Order!");
	print2 ("Your name is expunged from the records....");
	Player.rank[ORDER] = -1;
    } else if (gamestatusp (DESTROYED_ORDER))
	print1 ("The destruction of the Order of Paladins has negated the law!");
    else if ((Current_Environment != E_CITY) && (Last_Environment != E_CITY))
	print1 ("Fortunately, there is no jail around here, so you are freed!");
    else {
	pacify_guards ();
	if (((Current_Environment == E_HOUSE) || (Current_Environment == E_MANSION) || (Current_Environment == E_HOVEL)) && (Last_Environment == E_CITY)) {
	    setgamestatus (SUPPRESS_PRINTING);
	    change_environment (E_CITY);
	    resetgamestatus (SUPPRESS_PRINTING);
	}
	if (Current_Environment == E_CITY) {
	    if (gamestatusp (UNDEAD_GUARDS)) {
		print1 ("You are taken to a weirdly deserted chamber where an undead");
		print2 ("Magistrate presides over a court of ghosts and haunts.");
		morewait ();
		print1 ("'Mr. Foreman, what is the verdict?'");
		print2 ("'Guilty as charged, your lordship.'");
		morewait ();
		clearmsg ();
		print1 ("'Guilty...");
		morewait ();
		nprint1 ("Guilty...");
		morewait ();
		nprint1 ("Guilty...");
		morewait ();
		nprint1 ("Guilty...'");
		clearmsg ();
		print1 ("The members of the court close in around, fingers pointing.");
		print2 ("You feel insubstantial hands closing around your throat....");
		print3 ("You feel your life draining away!");
		while (Player.level > 0) {
		    Player.level--;
		    Player.xp /= 2;
		    Player.hp /= 2;
		    dataprint ();
		}
		Player.maxhp = Player.maxcon;
		morewait ();
		print1 ("You are finally released, a husk of your former self....");
		Player.x = 58;
		Player.y = 40;
		screencheck (58);
	    } else if (Player.alignment + random_range (200) < 0) {
		print1 ("Luckily for you, a smooth-tongued advocate from the");
		print2 ("Rampart Chaotic Liberties Union gets you off!");
		Player.x = 58;
		Player.y = 40;
		screencheck (58);
	    } else
		switch (Imprisonment++) {
		    case 0:
			print1 ("The Magistrate sternly reprimands you.");
			print2 ("As a first-time offender, you are given probation.");
			Player.y = 58;
			Player.x = 40;
			screencheck (58);
			break;
		    case 1:
			print1 ("The Magistrate expresses shame for your conduct.");
			print2 ("You are thrown in jail!");
			morewait ();
			repair_jail ();
			Player.y = 54;
			Player.x = 37 + (2 * random_range (4));
			screencheck (54);
			l_portcullis_trap ();
			break;
		    default:
			print1 ("The Magistrate renders summary judgement.");
			print2 ("You are sentenced to prison!");
			morewait ();
			print1 ("The guards recognize you as a 'three-time-loser'");
			print2 ("...and beat you up a little to teach you a lesson.");
			p_damage (random_range (Imprisonment * 10), UNSTOPPABLE, "police brutality");
			morewait ();
			repair_jail ();
			Player.y = 54;
			Player.x = 37 + (2 * random_range (4));
			screencheck (54);
			l_portcullis_trap ();
		}
	}
    }
}

void l_adept (void)
{
    print1 ("You see a giant shimmering gate in the form of an omega.");
    if (!gamestatusp (ATTACKED_ORACLE)) {
	if (Player.str + Player.con + Player.iq + Player.pow < 100)
	    print2 ("A familiar female voice says: I would not advise this now....");
	else
	    print2 ("A familiar female voice says: Go for it!");
	morewait ();
	clearmsg ();
    }
    print2 ("Enter the mystic portal? [yn] ");
    if (ynq2 () != 'y') {
	if (Player.level > 100) {
	    print1 ("The Lords of Destiny spurn your cowardice....");
	    Player.xp = 0;
	    Player.level = 0;
	    Player.hp = Player.maxhp = Player.con;
	    Player.mana = calcmana ();
	    print2 ("You suddenly feel very inexperienced.");
	    dataprint ();
	}
    } else {
	clearmsg ();
	print1 ("You pass through the portal.");
	morewait ();
	drawomega ();
	print1 ("Like wow man! Colors! ");
	if (Player.patron != DESTINY) {
	    print2 ("Strange forces try to tear you apart!");
	    p_damage (random_range (200), UNSTOPPABLE, "a vortex of chaos");
	} else
	    print2 ("Some strange force shields you from a chaos vortex!");
	morewait ();
	print1 ("Your head spins for a moment....");
	print2 ("and clears....");
	morewait ();
	Player.hp = Player.maxhp;
	Player.mana = calcmana ();
	change_environment (E_ABYSS);
    }
}

void l_trifid (void)
{
    int damage = 0, stuck = TRUE;
    print1 ("The hedge comes alive with a surge of alien growth!");
    while (stuck) {
	dataprint ();
	damage += Level->depth / 2 + 1;
	print2 ("Razor-edged vines covered in suckers attach themselves to you.");
	morewait ();
	if (find_and_remove_item (THINGID + 6, -1)) {
	    print1 ("Thinking fast, you toss salt water on the trifid...");
	    print2 ("The trifid disintegrates with a frustrated sigh.");
	    Level->site[Player.x][Player.y].locchar = FLOOR;
	    Level->site[Player.x][Player.y].p_locf = L_NO_OP;
	    lset (Player.x, Player.y, CHANGED);
	    gain_experience (1000);
	    stuck = FALSE;
	} else {
	    p_damage (damage, UNSTOPPABLE, "a trifid");
	    morewait ();
	    print1 ("You are entangled in tendrils...");
	    menuclear ();
	    menuprint ("a: Try to break free.\n");
	    menuprint ("b: Hang limp and hope the tendrils uncoil.\n");
	    menuprint ("c: Pray for assistance.\n");
	    menuprint ("d: Attempt to bargain with the hedge.\n");
	    menuprint ("e: Click your heels together and wish for escape.\n");
	    menuprint ("ANYTHING ELSE: writhe and scream hopelessly.\n");
	    showmenu ();
	    switch (menugetc ()) {
		case 'a':
		    if (Player.str > random_range (200)) {
			print1 ("Amazing! You're now free.");
			print2 ("The trifid writhes hungrily at you.");
			stuck = FALSE;
		    } else
			print1 ("Well, THAT didn't work.");
		    break;
		case 'b':
		    print1 ("Well, at least you're facing your fate with dignity.");
		    break;
		case 'c':
		    if ((Player.patron == DRUID) && (Player.rank[PRIESTHOOD] > random_range (5))) {
			print1 ("A shaft of golden light bathes the alien plant");
			print2 ("which grudginly lets you go....");
			stuck = FALSE;
		    } else
			print1 ("You receive no divine aid as yet.");
		    break;
		case 'd':
		    print1 ("The hedge doesn't answer your entreaties.");
		    break;
		case 'e':
		    print1 ("You forgot your ruby slippers, stupid.");
		    break;
		default:
		    print1 ("The hedge enjoys your camp play-acting....");
		    break;
	    }
	}
    }
    xredraw ();
}

void l_vault (void)
{
    print1 ("You come to a thick vault door with a complex time lock.");
    if ((hour () == 23)) {
	print2 ("The door is open.");
	Level->site[12][56].locchar = FLOOR;
    } else {
	print2 ("The door is closed.");
	Level->site[12][56].locchar = WALL;
	morewait ();
	clearmsg ();
	print1 ("Try to crack it? [yn] ");
	if (ynq1 () == 'y') {
	    if (random_range (100) < Player.rank[THIEVES] * Player.rank[THIEVES]) {
		print2 ("The lock clicks open!!!");
		gain_experience (5000);
		Level->site[12][56].locchar = FLOOR;
	    } else {
		print2 ("Uh, oh, set off the alarm.... The castle guard arrives....");
		morewait ();
		if (Player.rank[NOBILITY] == DUKE) {
		    clearmsg ();
		    print1 ("\"Ah, just testing us, your Grace?  I hope we're up to scratch.\"");
		    morewait ();
		} else
		    send_to_jail ();
	    }
	} else
	    print2 ("Good move.");
    }
}

void l_brothel (void)
{
    char response;
    print1 ("You come to a heavily reinforced inner door.");
    print2 ("A sign reads `The House of the Eclipse'");
    morewait ();
    clearmsg ();
    print1 ("Try to enter? [yn] ");
    if (ynq1 () == 'y') {
	menuclear ();
	menuprint ("a:knock on the door.\n");
	menuprint ("b:try to pick the lock.\n");
	menuprint ("c:bash down the door.\n");
	menuprint ("ESCAPE: Leave this house of ill repute.\n");
	showmenu ();
	do
	    response = menugetc ();
	while ((response != 'a') && (response != 'b') && (response != 'c') && (response != ESCAPE));
	xredraw ();
	if (response == 'a') {
	    if (!nighttime ())
		print2 ("There is no reponse.");
	    else {
		print1 ("A window opens in the door.");
		print2 ("`500Au, buddy. For the night.' pay it? [yn] ");
		if (ynq2 () == 'y') {
		    if (Player.cash < 500) {
			print1 ("`What, no roll?!'");
			print2 ("The bouncer bounces you a little and lets you go.");
			p_damage (25, UNSTOPPABLE, "da bouncer");
		    } else {
			Player.cash -= 500;
			print1 ("You are ushered into an opulently appointed hall.");
			print2 ("After an expensive dinner (takeout from Les Crapuleux)");
			morewait ();
			if (Player.preference == 'n') {
			    switch (random_range (4)) {
				case 0:
				    print1 ("you spend the evening playing German Whist with");
				    break;
				case 1:
				    print1 ("you spend the evening discussing philosophy with");
				    break;
				case 2:
				    print1 ("you spend the evening playing chess against");
				    break;
				case 3:
				    print1 ("you spend the evening telling your adventures to");
			    }
			    print2 ("various employees of the House of the Eclipse.");
			} else {
			    print1 ("you spend an enjoyable and educational evening with");
			    if (Player.preference == 'm' || (Player.preference == 'y' && random_range (2)))
				switch (random_range (4)) {
				    case 0:
					print2 ("Skarn the Insatiable, a satyr.");
					break;
				    case 1:
					print2 ("Dryden the Defanged, an incubus.");
					break;
				    case 2:
					print2 ("Gorgar the Equipped, a centaur.");
					break;
				    case 3:
					print2 ("Hieronymus, the mendicant priest of Eros.");
					break;
			    } else
				switch (random_range (4)) {
				    case 0:
					print2 ("Noreen the Nymph (omaniac)");
					break;
				    case 1:
					print2 ("Angelface, a recanted succubus.");
					break;
				    case 2:
					print2 ("Corporal Sue of the City Guard (moonlighting).");
					break;
				    case 3:
					print2 ("Sheena the Queena the Jungle, a wereleopard.");
					break;
				}
			}
			morewait ();
			if (hour () > 12)
			    Time += ((24 - hour ()) + 8) * 60;
			else {
			    Time += ((9 - hour ()) * 60);
			    Date++;
			}
			Player.food = 40;
			Player.status[DISEASED] = 0;
			Player.status[POISONED] = 0;
			Player.hp = Player.maxhp;
			/* reduce temporary stat gains to max stat levels */
			toggle_item_use (TRUE);
			Player.str = min (Player.str, Player.maxstr);
			Player.con = min (Player.con, Player.maxcon);
			Player.agi = min (Player.agi, Player.maxagi);
			Player.dex = min (Player.dex, Player.maxdex);
			Player.iq = min (Player.iq, Player.maxiq);
			Player.pow = min (Player.pow, Player.maxpow);
			toggle_item_use (FALSE);
			if (Player.preference == 'n')
			    Player.iq++;	/* whatever :-) */
			else
			    Player.con++;
			gain_experience (100);
			timeprint ();
			dataprint ();
			showflags ();
			morewait ();
			clearmsg ();
			if (Player.preference == 'n')
			    print1 ("You arise refreshed the next morning...");
			else
			    print1 ("You arise, tired but happy, the next morning...");
		    }
		} else
		    print2 ("What are you, some kinda prude?");
	    }
	} else if (response == 'b') {
	    if (nighttime ()) {
		print1 ("As you fumble at the lock, the door opens....");
		print2 ("The bouncer tosses you into the street.");
	    } else
		print1 ("The door appears to be bolted and barred from behind.");
	} else if (response == 'c') {
	    if (nighttime ()) {
		print1 ("As you charge toward the door it opens....");
		print2 ("Yaaaaah! Thud!");
		morewait ();
		print1 ("You run past the startled bouncer into a wall.");
		p_damage (20, UNSTOPPABLE, "a move worthy of Clouseau");
		print2 ("The bouncer tosses you into the street.");
	    } else {
		print1 ("Ouch! The door resists your efforts.");
		p_damage (1, UNSTOPPABLE, "a sturdy door");
		morewait ();
		print1 ("You hear an irritated voice from inside:");
		print2 ("'Keep it down out there! Some of us are trying to sleep!'");
	    }
	}
    }
}

/* if signp is true, always print message, otherwise do so only sometimes */
void sign_print (int x, int y, int signp)
{
    if ((Level->site[x][y].p_locf >= CITYSITEBASE) && (Level->site[x][y].p_locf < CITYSITEBASE + NUMCITYSITES))
	CitySiteList[Level->site[x][y].p_locf - CITYSITEBASE][0] = TRUE;
    switch (Level->site[x][y].p_locf) {
	case L_CHARITY:
	    print1 ("You notice a sign: The Rampart Orphanage And Hospice For The Needy.");
	    break;
	case L_MANSION:
	    print1 ("You notice a sign:");
	    print2 ("This edifice protected by DeathWatch Devices, Ltd.");
	    morewait ();
	    break;
	case L_GRANARY:
	    print1 ("You notice a sign:");
	    print2 ("Public Granary: Entrance Strictly Forbidden.");
	    break;
	case L_PORTCULLIS:
	    if (Level->site[x][y].locchar == FLOOR)
		print1 ("You see a groove in the floor and slots above you.");
	    break;
	case L_STABLES:
	    print1 ("You notice a sign:");
	    print2 ("Village Stables");
	    break;
	case L_COMMONS:
	    print1 ("You notice a sign:");
	    print2 ("Village Commons: No wolves allowed.");
	    break;
	case L_MAZE:
	    print1 ("You notice a sign:");
	    print2 ("Hedge maze closed for trifid extermination.");
	    break;
	case L_BANK:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("First Bank of Omega: Autoteller Carrel.");
	    }
	    break;
	case L_TEMPLE:
	    print1 ("You see the ornate portico of the Rampart Pantheon");
	    break;
	case L_ARMORER:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Julie's Armor of Proof and Weapons of Quality");
	    }
	    break;
	case L_CLUB:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Rampart Explorers' Club.");
	    }
	    break;
	case L_GYM:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("The Rampart Gymnasium, (affil. Rampart Coliseum).");
	    }
	    break;
	case L_HEALER:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Rampart Healers. Member RMA.");
	    }
	    break;
	case L_CASINO:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Rampart Mithril Nugget Casino.");
	    }
	    break;
	case L_SEWER:
	    print1 ("A sewer entrance. You don't want to go down THERE, do you?");
	    break;
	case L_COMMANDANT:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Commandant Sonder's Rampart-fried Lyzzard partes. Open 24 hrs.");
	    }
	    break;
	case L_DINER:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("The Rampart Diner. All you can eat, 25Au.");
	    }
	    break;
	case L_CRAP:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Les Crapeuleaux. (****)");
	    }
	    break;
	case L_TAVERN:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("The Centaur and Nymph -- J. Riley, prop.");
	    }
	    break;
	case L_ALCHEMIST:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Ambrosias' Potions et cie.");
	    }
	    break;
	case L_DPW:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Rampart Department of Public Works.");
	    }
	    break;
	case L_LIBRARY:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Rampart Public Library.");
	    }
	    break;
	case L_CONDO:
	    if (signp) {
		print1 ("You notice a sign:");
		if (gamestatusp (SOLD_CONDO))
		    print2 ("Home Sweet Home");
		else
		    print2 ("Luxury Condominium For Sale; Inquire Within");
	    }
	    break;
	case L_PAWN_SHOP:
	    if (signp) {
		print1 ("You notice a sign:");
		print2 ("Knight's Pawn Shop.");
	    }
	    break;
	case L_CEMETARY:
	    print1 ("You notice a sign:");
	    print2 ("Rampart City Cemetary. Closed -- Full.");
	    break;
	case L_GARDEN:
	    print1 ("You notice a sign:");
	    print2 ("Rampart Botanical Gardens---Do not deface statues.");
	    break;
	case L_JAIL:
	    print1 ("You notice a sign:");
	    print2 ("Rampart City Gaol -- always room for more.");
	    break;
	case L_ORACLE:
	    print1 ("You notice a sign:");
	    print2 ("The Oracle of the Cyan Flames");
	    morewait ();
	    break;
    }
}

void l_countryside (void)
{
    if (optionp (CONFIRM)) {
	clearmsg ();
	print1 ("Do you really want to return to the countryside? ");
	if (ynq1 () != 'y')
	    return;
    }
    change_environment (E_COUNTRYSIDE);
}

void l_oracle (void)
{
    char response;
    if (gamestatusp (ATTACKED_ORACLE) && (!gamestatusp (COMPLETED_ASTRAL))) {
	print1 ("You come before a blue crystal dais. You see a broken mirror.");
	print2 ("Look in the mirror? [yn] ");
	if (ynq2 () == 'y') {
	    print1 ("A strange force rips you from your place....");
	    Player.hp = 1;
	    print2 ("You feel drained....");
	    dataprint ();
	    print3 ("You find yourself in a weird flickery maze.");
	    change_environment (E_ASTRAL);
	}
    } else {
	print1 ("You come before a blue crystal dais. There is a bell and a mirror.");
	print2 ("Ring the bell [b], look in the mirror [m], or leave [ESCAPE] ");
	do
	    response = (char) mcigetc ();
	while ((response != 'b') && (response != 'm') && (response != ESCAPE));
	if (response == 'b') {
	    print1 ("The ringing note seems to last forever.");
	    print2 ("You notice a robed figure in front of you....");
	    morewait ();
	    print1 ("The oracle doffs her cowl. Her eyes glitter with blue fire!");
	    print2 ("Attack her? [yn] ");
	    if (ynq2 () == 'y') {
		setgamestatus (ATTACKED_ORACLE);
		print1 ("The oracle deftly avoids your attack.");
		print2 ("She sneers at you and vanishes.");
	    } else {
		print2 ("She stares at you...and speaks:");
		if (!gamestatusp (SPOKE_TO_DRUID)) {
		    print3 ("'The ArchDruid speaks wisdom in his forest shrine.'");
		} else if (!gamestatusp (COMPLETED_CAVES)) {
		    print3 ("'Thou mayest find aught of interest in the caves to the South.'");
		} else if (!gamestatusp (COMPLETED_SEWERS)) {
		    print3 ("'Turn thy attention to the abyssal depths of the city.'");
		} else if (!gamestatusp (COMPLETED_CASTLE)) {
		    print3 ("'Explorest thou the depths of the Castle of the ArchMage.'");
		} else if (!gamestatusp (COMPLETED_ASTRAL)) {
		    morewait ();
		    print1 ("'Journey to the Astral Plane and meet the Gods' servants.'");
		    print2 ("The oracle holds out her hand. Do you take it? [yn] ");
		    if (ynq2 () == 'y') {
			print1 ("'Beware: Only the Star Gem can escape the Astral Plane.'");
			print2 ("A magic portal opens behind the oracle. She leads you");
			morewait ();
			print1 ("through a sequence of special effects that would have");
			print2 ("IL&M technicians cursing in awe and deposits you in an");
			morewait ();
			clearmsg ();
			print1 ("odd looking room whose walls seem strangely insubstantial.");
			gain_experience (5000);
			change_environment (E_ASTRAL);
		    } else
			print3 ("You detect the hint of a sneer from the oracle.");
		} else if (!gamestatusp (COMPLETED_VOLCANO)) {
		    print3 ("'The infernal maw may yield its secrets to thee now.'");
		} else if (!gamestatusp (COMPLETED_CHALLENGE)) {
		    print3 ("'The challenge of adepthood yet awaits thee.'");
		} else {
		    morewait ();
		    print1 ("'My lord: Thou hast surpassed my tutelage forever.");
		    print2 ("Fare thee well.'");
		    print3 ("The oracle replaces her hood and seems to fade away....");
		}
	    }
	} else if (response == 'm') {
	    print1 ("You seem to see yourself. Odd....");
	    knowledge (1);
	} else
	    print2 ("You leave this immanent place.");
    }
}

void l_mansion (void)
{
    print1 ("Enter the mansion? [yn] ");
    if (ynq1 () == 'y')
	change_environment (E_MANSION);
}

void l_house (void)
{
    print1 ("Enter the house? [yn] ");
    if (ynq1 () == 'y')
	change_environment (E_HOUSE);
}

void l_hovel (void)
{
    print1 ("Enter the hovel? [yn] ");
    if (ynq1 () == 'y')
	change_environment (E_HOVEL);
}

void l_safe (void)
{
    char response;
    pob newitem;
    int attempt = 0;
    print1 ("You have discovered a safe!");
    print2 ("Pick the lock [p], Force the door [f], or ignore [ESCAPE]");
    do
	response = (char) mcigetc ();
    while ((response != 'p') && (response != 'f') && (response != ESCAPE));
    if (response == 'p')
	attempt = (2 * Player.dex + Player.rank[THIEVES] * 10 - random_range (100)) / 10;
    else if (response == 'f')
	attempt = (Player.dmg - random_range (100)) / 10;
    if (attempt > 0) {
	Player.alignment -= 4;
	gain_experience (50);
	print2 ("The door springs open!");
	Level->site[Player.x][Player.y].locchar = FLOOR;
	Level->site[Player.x][Player.y].p_locf = L_NO_OP;
	lset (Player.x, Player.y, CHANGED);
	if (random_range (2) == 1) {
	    print1 ("You find:");
	    do {
		newitem = create_object (difficulty ());
		print2 (itemid (newitem));
		morewait ();
		gain_item (newitem);
	    } while (random_range (3) == 1);
	} else
	    print2 ("The safe was empty (awwwww....)");
    } else {
	print3 ("Your attempt at burglary failed!");
	if (attempt == -1) {
	    print1 ("A siren goes off! You see flashing red lights everywhere!");
	    morewait ();
	    if (Last_Environment == E_CITY) {
		print2 ("The city guard shows up! They collar you in no time flat!");
		change_environment (E_CITY);
		morewait ();
		send_to_jail ();
	    }
	} else if (attempt == -2) {
	    print1 ("There is a sudden flash!");
	    p_damage (random_range (25), FLAME, "a safe");
	    print2 ("The safe has self-destructed.");
	    Level->site[Player.x][Player.y].locchar = RUBBLE;
	    Level->site[Player.x][Player.y].p_locf = L_RUBBLE;
	    lset (Player.x, Player.y, CHANGED);
	} else if (attempt == -3) {
	    print1 ("The safe jolts you with electricity!");
	    lball (Player.x, Player.y, Player.x, Player.y, 30);
	} else if (attempt < -3) {
	    print1 ("You are hit by an acid spray!");
	    if (Player.possessions[O_CLOAK] != NULL) {
		print2 ("Your cloak is destroyed!");
		conform_lost_object (Player.possessions[O_CLOAK]);
		p_damage (10, ACID, "a safe");
	    } else if (Player.possessions[O_ARMOR] != NULL) {
		print2 ("Your armor corrodes!");
		Player.possessions[O_ARMOR]->dmg -= 3;
		Player.possessions[O_ARMOR]->hit -= 3;
		Player.possessions[O_ARMOR]->aux -= 3;
		p_damage (10, ACID, "a safe");
	    } else {
		print2 ("The acid hits your bare flesh!");
		p_damage (random_range (100), ACID, "a safe");
	    }
	}
    }
}

void l_cartographer (void)
{
    int i, j, x, y;
    print1 ("Ye Olde Mappe Shoppe.");
    print2 ("Map of the local area: 500Au. Buy it? [yn] ");
    if (ynq2 () == 'y') {
	if (Player.cash < 500)
	    print3 ("Cursed be cheapskates! May you never find an aid station....");
	else {
	    print3 ("You now have the local area mapped.");
	    Player.cash -= 500;
	    dataprint ();
	    switch (Villagenum) {
		case 1:
		    x = 56;
		    y = 5;
		    break;
		default:
		case 2:
		    x = 35;
		    y = 11;
		    break;
		case 3:
		    x = 10;
		    y = 40;
		    break;
		case 4:
		    x = 7;
		    y = 6;
		    break;
		case 5:
		    x = 40;
		    y = 43;
		    break;
		case 6:
		    x = 20;
		    y = 41;
		    break;
	    }
	    for (i = x - 15; i <= x + 15; i++)
		for (j = y - 15; j <= y + 15; j++)
		    if ((i >= 0) && (i < 64) && (j >= 0) && (j < 64)) {
			if (Country[i][j].current_terrain_type != Country[i][j].base_terrain_type) {
			    c_set (i, j, CHANGED);
			    Country[i][j].current_terrain_type = Country[i][j].base_terrain_type;
			}
			c_set (i, j, SEEN);
		    }
	}
    } else
	print3 ("Don't blame me if you get lost....");
}

void l_charity (void)
{
    long donation;
    print2 ("'Greetings, friend. Do you wish to make a donation?' [yn] ");
    if (ynq2 () != 'y')
	print3 ("'Pinchpurse!'");
    else {
	clearmsg ();
	print1 ("How much can you give? ");
	donation = parsenum ();
	if (donation < 1)
	    print2 ("'Go stick your head in a pig.'");
	else if (donation > Player.cash)
	    print2 ("'I'm afraid you're charity is bigger than your purse!'");
	else if (donation < max (100, Player.level * Player.level * 100)) {
	    print2 ("'Oh, can't you do better than that?'");
	    print3 ("'Well, I guess we'll take it....'");
	    if (Player.alignment < 10)
		Player.alignment++;
	    Player.cash -= donation;
	} else {
	    print2 ("'Oh thank you kindly, friend, and bless you!'");
	    Player.cash -= donation;
	    Player.alignment += 5;
	}
    }
    dataprint ();
}
