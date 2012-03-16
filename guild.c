#include "glob.h"

void l_merc_guild (void)
{
    pob newitem;

    print1 ("Legion of Destiny, Mercenary Guild, Inc.");
    if (nighttime ())
	print2 ("The barracks are under curfew right now.");
    else {
	print2 ("You enter Legion HQ, ");
	if (Player.rank[LEGION] == COMMANDANT) {
	    nprint2 ("Your aide follows you to the staff room.");
	    morewait ();
	    clearmsg ();
	}
	if (Player.rank[LEGION] > 0) {
	    nprint2 ("and report to your commander.");
	    morewait ();
	}
	switch (Player.rank[LEGION]) {
	    case 0:
		nprint2 ("and see the Recruiting Centurion.");
		morewait ();
		print2 ("Do you wish to join the Legion? [yn] ");
		if (ynq2 () == 'y') {
		    clearmsg ();
		    if (Player.rank[ARENA] > 0) {
			print1 ("The Centurion checks your record, and gets angry:");
			print2 ("'The legion don't need any Arena Jocks. Git!'");
		    } else if (Player.rank[ORDER] > 0) {
			print1 ("The Centurion checks your record, and is amused:");
			print2 ("'A paladin in the ranks? You must be joking.'");
		    } else if (Player.con < 12) {
			print1 ("The Centurion looks you over, sadly.");
			print2 ("You are too fragile to join the legion.");
		    } else if (Player.str < 10) {
			print1 ("The Centurion looks at you contemptuously.");
			print2 ("Your strength is too low to pass the physical!");
		    } else {
			print1 ("You are tested for strength and stamina...");
			morewait ();
			nprint1 (" and you pass!");
			print2 ("Commandant ");
			nprint2 (Commandant);
			nprint2 (" shakes your hand.");
			morewait ();
			print2 ("The Legion pays you a 500Au induction fee.");
			morewait ();
			print1 ("You are also issued a shortsword and leather.");
			print2 ("You are now a Legionaire.");
			morewait ();
			clearmsg ();
			newitem = ((pob) checkmalloc (sizeof (objtype)));
			*newitem = Objects[WEAPONID + 1];	/* shortsword */
			gain_item (newitem);
			newitem = ((pob) checkmalloc (sizeof (objtype)));
			*newitem = Objects[ARMORID + 1];	/* leather */
			gain_item (newitem);
			Player.cash += 500;
			Player.rank[LEGION] = LEGIONAIRE;
			Player.guildxp[LEGION] = 1;
			Player.str++;
			Player.con++;
			Player.maxstr++;
			Player.maxcon++;
		    }
		}
		break;
	    case COMMANDANT:
		print1 ("You find the disposition of your forces satisfactory.");
		break;
	    case COLONEL:
		if ((Player.level > Commandantlevel) && find_and_remove_item (CORPSEID, DEMON_EMP)) {
		    print1 ("You liberated the Demon Emperor's Regalia!");
		    morewait ();
		    clearmsg ();
		    print1 ("The Legion is assembled in salute to you!");
		    print2 ("The Regalia is held high for all to see and admire.");
		    morewait ();
		    clearmsg ();
		    print1 ("Commandant ");
		    nprint1 (Commandant);
		    nprint1 (" promotes you to replace him,");
		    print2 ("and announces his own overdue retirement.");
		    morewait ();
		    clearmsg ();
		    print1 ("You are the new Commandant of the Legion!");
		    print2 ("The Emperor's Regalia is sold for a ridiculous sum.");
		    strcpy (Commandant, Player.name);
		    Commandantlevel = Player.level;
		    morewait ();
		    Commandantbehavior = fixnpc (4);
		    save_hiscore_npc (8);
		    clearmsg ();
		    print1 ("You now know the Spell of Regeneration.");
		    Spells[S_REGENERATE].known = TRUE;
		    Player.rank[LEGION] = COMMANDANT;
		    Player.maxstr += 2;
		    Player.str += 2;
		    Player.maxcon += 2;
		    Player.con += 2;
		    print2 ("Your training is complete. You get top salary.");
		    Player.cash += 20000;
		} else if (Player.level <= Commandantlevel) {
		    clearmsg ();
		    print1 ("Your CO expresses satisfaction with your progress.");
		    print2 ("But your service record does not yet permit promotion.");
		} else {
		    clearmsg ();
		    print1 ("Why do you come empty handed?");
		    print2 ("You must return with the Regalia of the Demon Emperor!");
		}
		break;
	    case FORCE_LEADER:
		clearmsg ();
		print1 ("Your CO expresses satisfaction with your progress.");
		if (Player.guildxp[LEGION] < 4000)
		    print2 ("But your service record does not yet permit promotion.");
		else {
		    print2 ("You have been promoted to Legion Colonel!");
		    morewait ();
		    print1 ("Your next promotion is contingent on the return of");
		    print2 ("the Regalia of the Demon Emperor.");
		    morewait ();
		    print1 ("The Demon Emperor holds court at the base of a volcano");
		    print2 ("to the far south, in the heart of a swamp.");
		    morewait ();
		    clearmsg ();
		    print1 ("You have been taught the spell of heroism!");
		    Spells[S_HERO].known = TRUE;
		    Player.rank[LEGION] = COLONEL;
		    Player.maxstr++;
		    Player.str++;
		    Player.maxcon++;
		    Player.con++;
		    print2 ("You are given advanced training, and a raise.");
		    Player.cash += 10000;
		}
		break;
	    case CENTURION:
		clearmsg ();
		print1 ("Your CO expresses satisfaction with your progress.");
		if (Player.guildxp[LEGION] < 1500)
		    print2 ("But your service record does not yet permit promotion.");
		else {
		    print2 ("You are now a Legion Force-Leader!");
		    Player.rank[LEGION] = FORCE_LEADER;
		    Player.maxstr++;
		    Player.str++;
		    morewait ();
		    clearmsg ();
		    print1 ("You receive more training, and bonus pay.");
		    Player.cash += 5000;
		}
		break;
	    case LEGIONAIRE:
		clearmsg ();
		print1 ("Your CO expresses satisfaction with your progress.");
		if (Player.guildxp[LEGION] < 400)
		    print2 ("But your service record does not yet permit promotion.");
		else {
		    print2 ("You are promoted to Legion Centurion!");
		    morewait ();
		    clearmsg ();
		    print1 ("You get advanced training, and a higher salary.");
		    Player.rank[LEGION] = CENTURION;
		    Player.maxcon++;
		    Player.con++;
		    Player.cash += 2000;
		}
		break;
	}
    }
}

void l_castle (void)
{
    pob o;
    int x, y;

    if (Player.level < 3) {
	print1 ("You can't possibly enter the castle, you nobody!");
	print2 ("Come back when you are famous.");
    } else {
	print1 ("You are ushered into the castle.");
	if (Player.rank[NOBILITY] < DUKE) {
	    print2 ("His Grace, ");
	    nprint2 (Duke);
	    nprint2 ("-- Duke of Rampart! <fanfare>");
	    morewait ();
	    clearmsg ();
	}
	if (Player.rank[NOBILITY] == 0) {
	    print1 ("Well, sirrah, wouldst embark on a quest? [yn] ");
	    if (ynq1 () == 'y') {
		print2 ("Splendid. Bring me the head of the Goblin King.");
		Player.rank[NOBILITY] = COMMONER;
	    } else {
		print1 ("You scoundrel! Guards! Take this blackguard away!");
		morewait ();
		p_damage (25, UNSTOPPABLE, "castle guards for lese majeste");
		send_to_jail ();
	    }
	} else if (Player.rank[NOBILITY] == COMMONER) {
	    if (find_and_remove_item (CORPSEID, GOBLIN_KING)) {
		print1 ("Good job, sirrah! I promote you to the rank of esquire.");
		Player.rank[NOBILITY] = ESQUIRE;
		gain_experience (100);
		print2 ("Now that you have proved yourself true, another quest!");
		morewait ();
		print1 ("Bring to me a Holy Defender!");
		print2 ("One is said to be in the possession of the Great Wyrm");
		morewait ();
		clearmsg ();
		print1 ("in the depths of the sewers below the city.");
	    } else
		print2 ("Do not return until you achieve the quest, caitiff!");
	} else if (Player.rank[NOBILITY] == ESQUIRE) {
	    if (find_and_remove_item (WEAPONID + 34, -1)) {
		print1 ("My thanks, squire. In return, I dub thee knight!");
		Player.rank[NOBILITY] = KNIGHT;
		gain_experience (1000);
		print2 ("If thou wouldst please me further...");
		morewait ();
		print1 ("Bring me a suit of dragonscale armor.");
		print2 ("You might have to kill a dragon to get one....");
	    } else
		print2 ("Greetings, squire. My sword? What, you don't have it?");
	} else if (Player.rank[NOBILITY] == KNIGHT) {
	    if (find_and_remove_item (ARMORID + 12, -1)) {
		print1 ("Thanks, good sir knight.");
		print2 ("Here are letters patent to a peerage!");
		Player.rank[NOBILITY] = LORD;
		gain_experience (10000);
		morewait ();
		print1 ("If you would do me a final service...");
		print2 ("I require the Orb of Mastery. If you would be so kind...");
		morewait ();
		print1 ("By the way, you might find the Orb in the possession");
		print2 ("Of the Elemental Master on the Astral Plane");
	    } else
		print2 ("Your quest is not yet complete, sir knight.");
	} else if (Player.rank[NOBILITY] == LORD) {
	    if (find_item (&o, ARTIFACTID + 0, -1)) {
		print1 ("My sincerest thanks, my lord.");
		print2 ("You have proved yourself a true paragon of chivalry");
		morewait ();
		print1 ("I abdicate the Duchy in your favor....");
		print2 ("Oh, you can keep the Orb, by the way....");
		Player.rank[NOBILITY] = DUKE;
		gain_experience (10000);
		strcpy (Duke, Player.name);
		morewait ();
		Dukebehavior = fixnpc (4);
		save_hiscore_npc (12);
		for (y = 52; y < 63; y++)
		    for (x = 2; x < 52; x++) {
			if (Level->site[x][y].p_locf == L_TRAP_SIREN) {
			    Level->site[x][y].p_locf = L_NO_OP;
			    lset (x, y, CHANGED);
			}
			if (x >= 12 && loc_statusp (x, y, SECRET)) {
			    lreset (x, y, SECRET);
			    lset (x, y, CHANGED);
			}
			if (x >= 20 && x <= 23 && y == 56) {
			    Level->site[x][y].locchar = FLOOR;
			    lset (x, y, CHANGED);
			}
		    }

	    } else
		print2 ("I didn't really think you were up to the task....");
	}
    }
}

void l_arena (void)
{
    char response;
    pob newitem;
    int i, prize, monsterlevel;
    char *melee = NULL;

    print1 ("Rampart Coliseum");
    if (Player.rank[ARENA] == 0) {
	print2 ("Enter the games, or Register as a Gladiator? [e,r,ESCAPE] ");
	do
	    response = (char) mcigetc ();
	while ((response != 'e') && (response != 'r') && (response != ESCAPE));
    } else {
	print2 ("Enter the games? [yn] ");
	response = ynq2 ();
	if (response == 'y')
	    response = 'e';
	else
	    response = ESCAPE;
    }
    if (response == 'r') {
	if (Player.rank[ARENA] > 0)
	    print2 ("You're already a gladiator....");
	else if (Player.rank[ORDER] > 0)
	    print2 ("We don't let Paladins into our Guild.");
	else if (Player.rank[LEGION] > 0)
	    print2 ("We don't train no stinkin' mercs!");
	else if (Player.str < 13)
	    print2 ("Yer too weak to train!");
	else if (Player.agi < 12)
	    print2 ("Too clumsy to be a gladiator!");
	else {
	    print1 ("Ok, yer now an Arena Trainee.");
	    print2 ("Here's a wooden sword, and a shield");
	    morewait ();
	    clearmsg ();
	    newitem = ((pob) checkmalloc (sizeof (objtype)));
	    *newitem = Objects[WEAPONID + 17];	/* club */
	    gain_item (newitem);
	    newitem = ((pob) checkmalloc (sizeof (objtype)));
	    *newitem = Objects[SHIELDID + 2];	/* shield */
	    gain_item (newitem);
	    Player.rank[ARENA] = TRAINEE;
	    Arena_Opponent = 3;
	    morewait ();
	    clearmsg ();
	    print1 ("You've got 5000Au credit at the Gym.");
	    Gymcredit += 5000;
	}
    } else if (response == 'e') {
	print1 ("OK, we're arranging a match....");
	morewait ();
	Arena_Monster = ((pmt) checkmalloc (sizeof (montype)));
	Arena_Victory = FALSE;
	switch (Arena_Opponent) {
	    case 0:
		*Arena_Monster = Monsters[GEEK];
		break;
	    case 1:
		*Arena_Monster = Monsters[HORNET];
		break;
	    case 2:
		*Arena_Monster = Monsters[HYENA];
		break;
	    case 3:
		*Arena_Monster = Monsters[GOBLIN];
		break;
	    case 4:
		*Arena_Monster = Monsters[GRUNT];
		break;
	    case 5:
		*Arena_Monster = Monsters[TOVE];
		break;
	    case 6:
		*Arena_Monster = Monsters[APPR_NINJA];
		break;
	    case 7:
		*Arena_Monster = Monsters[SALAMANDER];
		break;
	    case 8:
		*Arena_Monster = Monsters[ANT];
		break;
	    case 9:
		*Arena_Monster = Monsters[MANTICORE];
		break;
	    case 10:
		*Arena_Monster = Monsters[SPECTRE];
		break;
	    case 11:
		*Arena_Monster = Monsters[BANDERSNATCH];
		break;
	    case 12:
		*Arena_Monster = Monsters[LICHE];
		break;
	    case 13:
		*Arena_Monster = Monsters[AUTO_MAJOR];
		break;
	    case 14:
		*Arena_Monster = Monsters[JABBERWOCK];
		break;
	    case 15:
		*Arena_Monster = Monsters[JOTUN];
		break;
	    default:
		if ((Player.rank[ARENA] < 5) && (Player.rank[ARENA] > 0)) {
		    strcpy (Str1, Champion);
		    strcat (Str1, ", the arena champion");
		    *Arena_Monster = Monsters[HISCORE_NPC];
		    Arena_Monster->monstring = salloc (Str1);
		    strcpy (Str2, "The corpse of ");
		    strcat (Str2, Str1);
		    Arena_Monster->corpsestr = salloc (Str2);
		    Arena_Monster->level = 20;
		    Arena_Monster->hp = Championlevel * Championlevel * 5;
		    Arena_Monster->hit = Championlevel * 4;
		    Arena_Monster->ac = Championlevel * 3;
		    Arena_Monster->dmg = 100 + Championlevel * 2;
		    Arena_Monster->xpv = Championlevel * Championlevel * 5;
		    Arena_Monster->speed = 3;
		    melee = Arena_Monster->meleestr = (char *) checkmalloc (30 * sizeof (char));
		    strcpy (Arena_Monster->meleestr, "");
		    for (i = 0; i < Championlevel / 5; i++)
			strcat (Arena_Monster->meleestr, "L?R?");
		    m_status_set (Arena_Monster, MOBILE);
		    m_status_set (Arena_Monster, HOSTILE);
		} else {
		    do
			i = random_range (ML9 - ML0) + ML0;
		    while (i == NPC || i == HISCORE_NPC || i == ZERO_NPC || (Monsters[i].uniqueness != COMMON) || (Monsters[i].dmg == 0));
		    *Arena_Monster = Monsters[i];
		}
		break;
	}
	monsterlevel = Arena_Monster->level;
	if (Arena_Monster->level != 20) {
	    strcpy (Str1, nameprint ());
	    strcat (Str1, " the ");
	    strcat (Str1, Arena_Monster->monstring);
	    Arena_Monster->monstring = salloc (Str1);
	    strcpy (Str2, "The corpse of ");
	    strcat (Str2, Str1);
	    Arena_Monster->corpsestr = salloc (Str2);
	}
	Arena_Monster->uniqueness = UNIQUE_MADE;
	print1 ("You have a challenger: ");
	print2 (Arena_Monster->monstring);
	Arena_Monster->attacked = TRUE;
	m_status_set (Arena_Monster, HOSTILE);
	morewait ();
	clearmsg ();
	change_environment (E_ARENA);
	print1 ("Let the battle begin....");

	time_clock (TRUE);
	while (Current_Environment == E_ARENA)
	    time_clock (FALSE);

	/* WDT -- Sheldon Simms points out that these objects are not
	 * wastes of space; on the contrary, they can be carried out of the
	 * arena.  Freeing them was causing subtle and hard to find problems.
	 * However, not freeing them is causing a couple of tiny memory leaks.
	 * This should be fixed, probably by modifying the object destruction
	 * procedures to account for this case.  I'm not really concerned. */
	/* David Given has proposed a nicer solution, but it still causes a
	 * memory leak.  Obviously, we need a special field just for names
	 * in the monster struct.  Yadda yadda -- I'll mmark this with a 
	 * HACK!, and comme back to it later. */
	/* can not free the corpse string... it is referenced in the */
	/* corpse string of the corpse object.  */
	/* Unfortunately, this will cause a memory leak, but I don't see */
	/* any way to avoid it.  This fixes the munged arena corpse names */
	/* problem. -DAG */
	/* free(corpse); */

	if (melee)
	    free (melee);
	if (!Arena_Victory) {
	    print1 ("The crowd boos your craven behavior!!!");
	    if (Player.rank[ARENA] > 0) {
		print2 ("You are thrown out of the Gladiator's Guild!");
		morewait ();
		clearmsg ();
		if (Gymcredit > 0)
		    print1 ("Your credit at the gym is cut off!");
		Gymcredit = 0;
		Player.rank[ARENA] = -1;
	    }
	} else {
	    Arena_Opponent++;
	    if (monsterlevel == 20) {
		print1 ("The crowd roars its approval!");
		if (Player.rank[ARENA]) {
		    print2 ("You are the new Arena Champion!");
		    Championlevel = Player.level;
		    strcpy (Champion, Player.name);
		    Player.rank[ARENA] = 5;
		    morewait ();
		    Championbehavior = fixnpc (4);
		    save_hiscore_npc (11);
		    print1 ("You are awarded the Champion's Spear: Victrix!");
		    morewait ();
		    newitem = ((pob) checkmalloc (sizeof (objtype)));
		    *newitem = Objects[WEAPONID + 35];
		    gain_item (newitem);

		} else {
		    print1 ("As you are not an official gladiator,");
		    nprint1 ("you are not made Champion.");
		    morewait ();
		}
	    }
	    morewait ();
	    clearmsg ();
	    print1 ("Good fight! ");
	    nprint1 ("Your prize is: ");
	    prize = max (25, monsterlevel * 50);
	    if (Player.rank[ARENA] > 0)
		prize *= 2;
	    mnumprint (prize);
	    nprint1 ("Au.");
	    Player.cash += prize;
	    if ((Player.rank[ARENA] < 4) && (Arena_Opponent > 5) && (Arena_Opponent % 3 == 0)) {
		if (Player.rank[ARENA] > 0) {
		    Player.rank[ARENA]++;
		    morewait ();
		    print1 ("You've been promoted to a stronger class!");
		    print2 ("You are also entitled to additional training.");
		    Gymcredit += Arena_Opponent * 1000;
		}
	    }
	}
	xredraw ();
    } else
	clearmsg ();
}

void l_thieves_guild (void)
{
    int fee, count, i, number, done = FALSE, dues = 1000;
    char c, action;
    pob lockpick;
    print1 ("You have penetrated to the Lair of the Thieves' Guild.");
    if (!nighttime ())
	print2 ("There aren't any thieves around in the daytime.");
    else {
	if ((Player.rank[THIEVES] == TMASTER) && (Player.level > Shadowlordlevel) && find_and_remove_item (THINGID + 16, -1)) {
	    print2 ("You nicked the Justiciar's Badge!");
	    morewait ();
	    print1 ("The Badge is put in a place of honor in the Guild Hall.");
	    print2 ("You are now the Shadowlord of the Thieves' Guild!");
	    morewait ();
	    print1 ("Who says there's no honor among thieves?");
	    strcpy (Shadowlord, Player.name);
	    Shadowlordlevel = Player.level;
	    morewait ();
	    Shadowlordbehavior = fixnpc (4);
	    save_hiscore_npc (7);
	    clearmsg ();
	    print1 ("You learn the Spell of Shadowform.");
	    Spells[S_SHADOWFORM].known = TRUE;
	    morewait ();
	    clearmsg ();
	    Player.rank[THIEVES] = SHADOWLORD;
	    Player.maxagi += 2;
	    Player.maxdex += 2;
	    Player.agi += 2;
	    Player.dex += 2;
	}
	while (!done) {
	    menuclear ();
	    if (Player.rank[THIEVES] == 0)
		menuprint ("a: Join the Thieves' Guild.\n");
	    else
		menuprint ("b: Raise your Guild rank.\n");
	    menuprint ("c: Get an item identified.\n");
	    if (Player.rank[THIEVES] > 0)
		menuprint ("d: Fence an item.\n");
	    menuprint ("ESCAPE: Leave this Den of Iniquity.");
	    showmenu ();
	    action = mgetc ();
	    if (action == ESCAPE)
		done = TRUE;
	    else if (action == 'a') {
		done = TRUE;
		if (Player.rank[THIEVES] > 0)
		    print2 ("You are already a member!");
		else if (Player.alignment > 10)
		    print2 ("You are too lawful to be a thief!");
		else {
		    dues += dues * (12 - Player.dex) / 9;
		    dues += Player.alignment * 5;
		    dues = max (100, dues);
		    clearmsg ();
		    mprint ("Dues are");
		    mnumprint (dues);
		    mprint (" Au. Pay it? [yn] ");
		    if (ynq1 () == 'y') {
			if (Player.cash < dues) {
			    print1 ("You can't cheat the Thieves' Guild!");
			    print2 ("... but the Thieves' Guild can cheat you....");
			    Player.cash = 0;
			} else {
			    print1 ("Shadowlord ");
			    nprint1 (Shadowlord);
			    print2 ("enters your name into the roll of the Guild.");
			    morewait ();
			    clearmsg ();
			    print1 ("As a special bonus, you get a free lockpick.");
			    print2 ("You are taught the spell of Object Detection.");
			    morewait ();
			    Spells[S_OBJ_DET].known = TRUE;
			    lockpick = ((pob) checkmalloc (sizeof (objtype)));
			    *lockpick = Objects[THINGID + 2];	/* lock pick */
			    gain_item (lockpick);
			    Player.cash -= dues;
			    dataprint ();
			    Player.guildxp[THIEVES] = 1;
			    Player.rank[THIEVES] = TMEMBER;
			    Player.maxdex++;
			    Player.dex++;
			    Player.agi++;
			    Player.maxagi++;
			}
		    }
		}
	    } else if (action == 'b') {
		if (Player.rank[THIEVES] == 0)
		    print2 ("You are not even a member!");
		else if (Player.rank[THIEVES] == SHADOWLORD)
		    print2 ("You can't get any higher than this!");
		else if (Player.rank[THIEVES] == TMASTER) {
		    if (Player.level <= Shadowlordlevel)
			print2 ("You are not experienced enough to advance.");
		    else
			print2 ("You must bring back the Justiciar's Badge!");
		} else if (Player.rank[THIEVES] == THIEF) {
		    if (Player.guildxp[THIEVES] < 4000)
			print2 ("You are not experienced enough to advance.");
		    else {
			print1 ("You are now a Master Thief of the Guild!");
			print2 ("You are taught the Spell of Apportation.");
			morewait ();
			print1 ("To advance to the next level you must return with");
			print2 ("the badge of the Justiciar (cursed be his name).");
			morewait ();
			clearmsg ();
			print1 ("The Justiciar's office is just south of the gaol.");
			Spells[S_APPORT].known = TRUE;
			Player.rank[THIEVES] = TMASTER;
			Player.maxagi++;
			Player.maxdex++;
			Player.agi++;
			Player.dex++;
		    }
		} else if (Player.rank[THIEVES] == ATHIEF) {
		    if (Player.guildxp[THIEVES] < 1500)
			print2 ("You are not experienced enough to advance.");
		    else {
			print1 ("You are now a ranking Thief of the Guild!");
			print2 ("You learn the Spell of Invisibility.");
			Spells[S_INVISIBLE].known = TRUE;
			Player.rank[THIEVES] = THIEF;
			Player.agi++;
			Player.maxagi++;
		    }
		} else if (Player.rank[THIEVES] == TMEMBER) {
		    if (Player.guildxp[THIEVES] < 400)
			print2 ("You are not experienced enough to advance.");
		    else {
			print1 ("You are now an Apprentice Thief!");
			print2 ("You are taught the Spell of Levitation.");
			Spells[S_LEVITATE].known = TRUE;
			Player.rank[THIEVES] = ATHIEF;
			Player.dex++;
			Player.maxdex++;
		    }
		}
	    } else if (action == 'c') {
		if (Player.rank[THIEVES] == 0) {
		    print1 ("RTG, Inc, Appraisers. Identification Fee: 50Au/item.");
		    fee = 50;
		} else {
		    fee = 5;
		    print1 ("The fee is 5Au per item.");
		}
		print2 ("Identify one item, or all possessions? [ip] ");
		if ((char) mcigetc () == 'i') {
		    if (Player.cash < fee)
			print2 ("Try again when you have the cash.");
		    else {
			Player.cash -= fee;
			dataprint ();
			identify (0);
		    }
		} else {
		    count = 0;
		    for (i = 1; i < MAXITEMS; i++)
			if (Player.possessions[i] != NULL)
			    if (Player.possessions[i]->known < 2)
				count++;
		    for (i = 0; i < Player.packptr; i++)
			if (Player.pack[i] != NULL)
			    if (Player.pack[i]->known < 2)
				count++;
		    clearmsg ();
		    print1 ("The fee will be: ");
		    mnumprint (max (count * fee, fee));
		    nprint1 ("Au. Pay it? [yn] ");
		    if (ynq1 () == 'y') {
			if (Player.cash < max (count * fee, fee))
			    print2 ("Try again when you have the cash.");
			else {
			    Player.cash -= max (count * fee, fee);
			    dataprint ();
			    identify (1);
			}
		    }
		}
	    } else if (action == 'd') {
		if (Player.rank[THIEVES] == 0)
		    print2 ("Fence? Who said anything about a fence?");
		else {
		    print1 ("Fence one item or go through pack? [ip] ");
		    if ((char) mcigetc () == 'i') {
			i = getitem (NULL_ITEM);
			if ((i == ABORT) || (Player.possessions[i] == NULL))
			    print2 ("Huh, Is this some kind of set-up?");
			else if (Player.possessions[i]->blessing < 0)
			    print2 ("I don't want to buy a cursed item!");
			else {
			    clearmsg ();
			    print1 ("I'll give you ");
			    mlongprint (2 * item_value (Player.possessions[i]) / 3);
			    nprint1 ("Au each. OK? [yn] ");
			    if (ynq1 () == 'y') {
				number = getnumber (Player.possessions[i]->number);
				if ((number >= Player.possessions[i]->number) && Player.possessions[i]->used) {
				    Player.possessions[i]->used = FALSE;
				    item_use (Player.possessions[i]);
				}
				Player.cash += number * 2 * item_value (Player.possessions[i]) / 3;
				/* Fenced artifacts could turn up anywhere, really... */
				if (Objects[Player.possessions[i]->id].uniqueness > UNIQUE_UNMADE)
				    Objects[Player.possessions[i]->id].uniqueness = UNIQUE_UNMADE;
				dispose_lost_objects (number, Player.possessions[i]);
				dataprint ();
			    } else
				print2 ("Hey, gimme a break, it was a fair price!");
			}
		    } else {
			for (i = 0; i < Player.packptr; i++) {
			    if (Player.pack[i]->blessing > -1) {
				clearmsg ();
				print1 ("Sell ");
				nprint1 (itemid (Player.pack[i]));
				nprint1 (" for ");
				mlongprint (2 * item_value (Player.pack[i]) / 3);
				nprint1 ("Au each? [ynq] ");
				if ((c = ynq1 ()) == 'y') {
				    number = getnumber (Player.pack[i]->number);
				    Player.cash += 2 * number * item_value (Player.pack[i]) / 3;
				    Player.pack[i]->number -= number;
				    if (Player.pack[i]->number < 1) {
					/* Fenced an artifact?  You just might see it again. */
					if (Objects[Player.pack[i]->id].uniqueness > UNIQUE_UNMADE)
					    Objects[Player.pack[i]->id].uniqueness = UNIQUE_UNMADE;
					free ((char *) Player.pack[i]);
					Player.pack[i] = NULL;
				    }
				    dataprint ();
				} else if (c == 'q')
				    break;
			    }
			}
			fixpack ();
		    }
		}
	    }
	}
    }
    xredraw ();
}

void l_college (void)
{
    char action;
    int done = FALSE, enrolled = FALSE;
    print1 ("The Collegium Magii. Founded 16937, AOF.");
    if (nighttime ())
	print2 ("The Registration desk is closed at night....");
    else {
	while (!done) {
	    if ((Player.rank[COLLEGE] == MAGE) && (Player.level > Archmagelevel) && find_and_remove_item (CORPSEID, EATER)) {
		print1 ("You brought back the heart of the Eater of Magic!");
		morewait ();
		print1 ("The Heart is sent to the labs for analysis.");
		print2 ("The Board of Trustees appoints you Archmage!");
		morewait ();
		clearmsg ();
		strcpy (Archmage, Player.name);
		Archmagelevel = Player.level;
		Player.rank[COLLEGE] = ARCHMAGE;
		Player.maxiq += 5;
		Player.iq += 5;
		Player.maxpow += 5;
		Player.pow += 5;
		morewait ();
		Archmagebehavior = fixnpc (4);
		save_hiscore_npc (9);
	    }
	    menuclear ();
	    menuprint ("May we help you?\n\n");
	    menuprint ("a: Enroll in the College.\n");
	    menuprint ("b: Raise your College rank.\n");
	    menuprint ("c: Do spell research.\n");
	    menuprint ("ESCAPE: Leave these hallowed halls.\n");
	    showmenu ();
	    action = mgetc ();
	    if (action == ESCAPE)
		done = TRUE;
	    else if (action == 'a') {
		if (Player.rank[COLLEGE] > 0)
		    print2 ("You are already enrolled!");
		else if (Player.iq < 13)
		    print2 ("Your low IQ renders you incapable of being educated.");
		else if (Player.rank[CIRCLE] > 0)
		    print2 ("Sorcery and our Magic are rather incompatable, no?");
		else {
		    if (Player.iq > 17) {
			print2 ("You are given a scholarship!");
			morewait ();
			enrolled = TRUE;
		    } else {
			print1 ("Tuition is 1000Au. ");
			nprint1 ("Pay it? [yn] ");
			if (ynq1 () == 'y') {
			    if (Player.cash < 1000)
				print2 ("You don't have the funds!");
			    else {
				Player.cash -= 1000;
				enrolled = TRUE;
				dataprint ();
			    }
			}
		    }
		    if (enrolled) {
			print1 ("Archmage ");
			nprint1 (Archmage);
			nprint1 (" greets you and congratulates you on your acceptance.");
			print2 ("You are now enrolled in the Collegium Magii!");
			morewait ();
			print1 ("You are now a Novice.");
			print2 ("You may research 1 spell, for your intro class.");
			Spellsleft = 1;
			Player.rank[COLLEGE] = INITIATE;
			Player.guildxp[COLLEGE] = 1;
			Player.maxiq += 1;
			Player.iq += 1;
			Player.maxpow += 1;
			Player.pow += 1;
		    }
		}
	    } else if (action == 'b') {
		if (Player.rank[COLLEGE] == 0)
		    print2 ("You have not even been initiated, yet!");
		else if (Player.rank[COLLEGE] == ARCHMAGE)
		    print2 ("You are at the pinnacle of mastery in the Collegium.");
		else if (Player.rank[COLLEGE] == MAGE) {
		    if (Player.level <= Archmagelevel)
			print2 ("You are not experienced enough to advance.");
		    else
			print2 ("You must return with the heart of the Eater of Magic!");
		} else if (Player.rank[COLLEGE] == PRECEPTOR) {
		    if (Player.guildxp[COLLEGE] < 4000)
			print2 ("You are not experienced enough to advance.");
		    else {
			print1 ("You are now a Mage of the Collegium Magii!");
			print2 ("You may research 6 spells for postdoctoral research.");
			Spellsleft += 6;
			morewait ();
			print1 ("To become Archmage, you must return with the");
			print2 ("heart of the Eater of Magic");
			morewait ();
			clearmsg ();
			print1 ("The Eater may be found on a desert isle somewhere.");
			Player.rank[COLLEGE] = MAGE;
			Player.maxiq += 2;
			Player.iq += 2;
			Player.maxpow += 2;
			Player.pow += 2;
		    }
		} else if (Player.rank[COLLEGE] == STUDENT) {
		    if (Player.guildxp[COLLEGE] < 1500)
			print2 ("You are not experienced enough to advance.");
		    else {
			print1 ("You are now a Preceptor of the Collegium Magii!");
			print2 ("You are taught the basics of ritual magic.");
			morewait ();
			clearmsg ();
			print1 ("Your position allows you to research 4 spells.");
			Spellsleft += 4;
			Spells[S_RITUAL].known = TRUE;
			Player.rank[COLLEGE] = PRECEPTOR;
			Player.maxiq += 1;
			Player.iq += 1;
			Player.maxpow += 1;
			Player.pow += 1;
		    }
		} else if (Player.rank[COLLEGE] == NOVICE) {
		    if (Player.guildxp[COLLEGE] < 400)
			print2 ("You are not experienced enough to advance.");
		    else {
			print1 ("You are now a Student at the Collegium Magii!");
			print2 ("You are taught the spell of identification.");
			morewait ();
			clearmsg ();
			print1 ("Thesis research credit is 2 spells.");
			Spellsleft += 2;
			Spells[S_IDENTIFY].known = TRUE;
			Player.rank[COLLEGE] = STUDENT;
			Player.maxiq += 1;
			Player.iq += 1;
			Player.maxpow += 1;
			Player.pow += 1;
		    }
		}
	    } else if (action == 'c') {
		clearmsg ();
		if (Spellsleft > 0) {
		    print1 ("Research permitted: ");
		    mnumprint (Spellsleft);
		    nprint1 (" Spells.");
		    morewait ();
		}
		if (Spellsleft < 1) {
		    print1 ("Extracurricular Lab fee: 2000 Au. ");
		    nprint1 ("Pay it? [yn] ");
		    if (ynq1 () == 'y') {
			if (Player.cash < 2000)
			    print1 ("Try again when you have the cash.");
			else {
			    Player.cash -= 2000;
			    dataprint ();
			    Spellsleft = 1;
			}
		    }
		}
		if (Spellsleft > 0) {
		    learnspell (0);
		    Spellsleft--;
		}
	    }
	}
    }
    xredraw ();
}

void l_sorcerors (void)
{
    char action;
    int done = FALSE, fee = 3000;
    long total;
    print1 ("The Circle of Sorcerors.");
    if (Player.rank[CIRCLE] == -1) {
	print2 ("Fool! Didn't we tell you to go away?");
	Player.mana = 0;
	dataprint ();
    } else
	while (!done) {
	    if ((Player.rank[CIRCLE] == HIGHSORCEROR) && (Player.level > Primelevel) && find_and_remove_item (CORPSEID, LAWBRINGER)) {
		print2 ("You obtained the Crown of the Lawgiver!");
		morewait ();
		print1 ("The Crown is ritually sacrificed to the Lords of Chaos.");
		print2 ("You are now the Prime Sorceror of the Inner Circle!");
		strcpy (Prime, Player.name);
		Primelevel = Player.level;
		morewait ();
		Primebehavior = fixnpc (4);
		save_hiscore_npc (10);
		clearmsg ();
		print1 ("You learn the Spell of Disintegration!");
		morewait ();
		clearmsg ();
		Spells[S_DISINTEGRATE].known = TRUE;
		Player.rank[CIRCLE] = PRIME;
		Player.maxpow += 10;
		Player.pow += 10;
	    }
	    menuclear ();
	    menuprint ("May we help you?\n\n");
	    menuprint ("a: Become an Initiate of the Circle.\n");
	    menuprint ("b: Raise your rank in the Circle.\n");
	    menuprint ("c: Restore mana points\n");
	    menuprint ("ESCAPE: Leave these Chambers of Power.\n");
	    showmenu ();
	    action = mgetc ();
	    if (action == ESCAPE)
		done = TRUE;
	    else if (action == 'a') {
		if (Player.rank[CIRCLE] > 0)
		    print2 ("You are already an initiate!");
		else if (Player.alignment > 0)
		    print2 ("You may not join -- you reek of Law!");
		else if (Player.rank[COLLEGE] != 0)
		    print2 ("Foolish Mage!  You don't have the right attitude to Power!");
		else {
		    fee += Player.alignment * 100;
		    fee += fee * (12 - Player.pow) / 9;
		    fee = max (100, fee);
		    clearmsg ();
		    mprint ("For you, there is an initiation fee of");
		    mnumprint (fee);
		    mprint (" Au.");
		    print2 ("Pay it? [yn] ");
		    if (ynq2 () == 'y') {
			if (Player.cash < fee)
			    print3 ("Try again when you have the cash!");
			else {
			    print1 ("Prime Sorceror ");
			    nprint1 (Prime);
			    print2 ("conducts your initiation into the circle of novices.");
			    morewait ();
			    clearmsg ();
			    print1 ("You learn the Spell of Magic Missiles.");
			    Spells[S_MISSILE].known = TRUE;
			    Player.cash -= fee;
			    dataprint ();
			    Player.rank[CIRCLE] = INITIATE;
			    Player.guildxp[CIRCLE] = 1;
			    Player.maxpow++;
			    Player.pow++;
			}
		    }
		}
	    } else if (action == 'b') {
		if (Player.rank[CIRCLE] == 0)
		    print2 ("You have not even been initiated, yet!");
		else if (Player.alignment > -1) {
		    print1 ("Ahh! You have grown too lawful!!!");
		    print2 ("You are hereby blackballed from the Circle!");
		    Player.rank[CIRCLE] = -1;
		    morewait ();
		    clearmsg ();
		    print1 ("A pox upon thee!");
		    if (!Player.immunity[INFECTION])
			Player.status[DISEASED] += 100;
		    print2 ("And a curse on your possessions!");
		    morewait ();
		    clearmsg ();
		    acquire (-1);
		    clearmsg ();
		    enchant (-1);
		    bless (-1);
		    print3 ("Die, false sorceror!");
		    p_damage (25, UNSTOPPABLE, "a sorceror's curse");
		    done = TRUE;
		} else if (Player.rank[CIRCLE] == PRIME) {
		    print2 ("You are at the pinnacle of mastery in the Circle.");
		} else if (Player.rank[CIRCLE] == HIGHSORCEROR) {
		    if (Player.level <= Primelevel)
			print2 ("You are not experienced enough to advance.");
		    else
			print2 ("You must return with the Crown of the LawBringer!");
		} else if (Player.rank[CIRCLE] == SORCEROR) {
		    if (Player.guildxp[CIRCLE] < 4000)
			print2 ("You are not experienced enough to advance.");
		    else {
			print1 ("You are now a High Sorceror of the Inner Circle!");
			print2 ("You learn the Spell of Disruption!");
			morewait ();
			clearmsg ();
			print1 ("To advance you must return with the LawBringer's Crown!");
			print2 ("The LawBringer resides on Star Peak.");
			Spells[S_DISRUPT].known = TRUE;
			Player.rank[CIRCLE] = HIGHSORCEROR;
			Player.maxpow += 5;
			Player.pow += 5;
		    }
		} else if (Player.rank[CIRCLE] == ENCHANTER) {
		    if (Player.guildxp[CIRCLE] < 1500)
			print2 ("You are not experienced enough to advance.");
		    else {
			print1 ("You are now a member of the Circle of Sorcerors!");
			print2 ("You learn the Spell of Ball Lightning!");
			Spells[S_LBALL].known = TRUE;
			Player.rank[CIRCLE] = SORCEROR;
			Player.maxpow += 2;
			Player.pow += 2;
		    }
		} else if (Player.rank[CIRCLE] == INITIATE) {
		    if (Player.guildxp[CIRCLE] < 400)
			print2 ("You are not experienced enough to advance.");
		    else {
			print1 ("You are now a member of the Circle of Enchanters!");
			print2 ("You learn the Spell of Firebolts.");
			Spells[S_FIREBOLT].known = TRUE;
			Player.rank[CIRCLE] = ENCHANTER;
			Player.maxpow += 2;
			Player.pow += 2;
		    }
		}
	    } else if (action == 'c') {
		done = TRUE;
		fee = Player.level * 100;
		if (Player.rank[CIRCLE])
		    fee = fee / 2;
		clearmsg ();
		print1 ("That will be: ");
		mnumprint (fee);
		nprint1 ("Au. Pay it? [yn] ");
		if (ynq1 () == 'y') {
		    if (Player.cash < fee)
			print2 ("Begone, deadbeat, or face the wrath of the Circle!");
		    else {
			Player.cash -= fee;
			total = calcmana ();
			while (Player.mana < total) {
			    Player.mana++;
			    dataprint ();
			}
			print2 ("Have a sorcerous day, now!");
		    }
		} else
		    print2 ("Be seeing you!");
	    }
	}
    xredraw ();
}

void l_order (void)
{
    pob newitem;
    pml ml;
    print1 ("The Headquarters of the Order of Paladins.");
    morewait ();
    if ((Player.rank[ORDER] == PALADIN) && (Player.level > Justiciarlevel) && gamestatusp (GAVE_STARGEM) && Player.alignment > 300) {
	print1 ("You have succeeded in your quest!");
	morewait ();
	print1 ("The previous Justiciar steps down in your favor.");
	print2 ("You are now the Justiciar of Rampart and the Order!");
	strcpy (Justiciar, Player.name);
	for (ml = Level->mlist; ml && (ml->m->id != HISCORE_NPC || ml->m->aux2 != 15); ml = ml->next)
	    /* just scan for current Justicar */ ;
	if (ml) {
	    Level->site[ml->m->x][ml->m->y].creature = NULL;
	    erase_monster (ml->m);
	    ml->m->hp = -1;	/* signals "death" -- no credit to player, though */
	}
	Justiciarlevel = Player.level;
	morewait ();
	Justiciarbehavior = fixnpc (4);
	save_hiscore_npc (15);
	clearmsg ();
	print1 ("You are awarded a blessed shield of deflection!");
	morewait ();
	newitem = ((pob) checkmalloc (sizeof (objtype)));
	*newitem = Objects[SHIELDID + 7];	/* shield of deflection */
	newitem->blessing = 9;
	gain_item (newitem);
	morewait ();
	Player.rank[ORDER] = JUSTICIAR;
	Player.maxstr += 5;
	Player.str += 5;
	Player.maxpow += 5;
	Player.pow += 5;
    }
    if (Player.alignment < 1) {
	if (Player.rank[ORDER] > 0) {
	    print1 ("You have been tainted by chaos!");
	    print2 ("You are stripped of your rank in the Order!");
	    morewait ();
	    Player.rank[ORDER] = -1;
	    send_to_jail ();
	} else
	    print1 ("Get thee hence, minion of chaos!");
    } else if (Player.rank[ORDER] == -1)
	print1 ("Thee again?  Get thee hence, minion of chaos!");
    else if (Player.rank[ORDER] == 0) {
	if (Player.rank[ARENA] != 0)
	    print1 ("We do not accept bloodstained gladiators into our Order.");
	else if (Player.rank[LEGION] != 0)
	    print1 ("Go back to your barracks, mercenary!");
	else {
	    print1 ("Dost thou wish to join our Order? [yn] ");
	    if (ynq1 () == 'y') {
		print1 ("Justiciar ");
		nprint1 (Justiciar);
		nprint1 (" welcomes you to the Order.");
		print2 ("'Mayest thou always follow the sublime path of Law.'");
		morewait ();
		print1 ("You are now a Gallant in the Order.");
		print2 ("You are given a horse and a blessed spear.");
		morewait ();
		Player.rank[ORDER] = GALLANT;
		Player.guildxp[ORDER] = 1;
		setgamestatus (MOUNTED);
		newitem = ((pob) checkmalloc (sizeof (objtype)));
		*newitem = Objects[WEAPONID + 19];	/* spear */
		newitem->blessing = 9;
		newitem->plus = 1;
		newitem->known = 2;
		gain_item (newitem);
	    }
	}
    } else {
	print1 ("'Welcome back, Paladin.'");
	if (!gamestatusp (MOUNTED)) {
	    print2 ("You are given a new steed.");
	    setgamestatus (MOUNTED);
	}
	morewait ();
	clearmsg ();
	if ((Player.hp < Player.maxhp) || (Player.status[DISEASED]) || (Player.status[POISONED]))
	    print1 ("Your wounds are treated by a medic.");
	cleanse (0);
	Player.hp = Player.maxhp;
	Player.food = 40;
	print2 ("You get a hot meal from the refectory.");
	morewait ();
	clearmsg ();
	if (Player.rank[ORDER] == PALADIN) {
	    if (Player.level <= Justiciarlevel)
		print2 ("You are not experienced enough to advance.");
	    else if (Player.alignment < 300)
		print2 ("You are not sufficiently Lawful as yet to advance.");
	    else
		print2 ("You must give the Star Gem to the LawBringer.");
	} else if (Player.rank[ORDER] == CHEVALIER) {
	    if (Player.guildxp[ORDER] < 4000)
		print2 ("You are not experienced enough to advance.");
	    else if (Player.alignment < 200)
		print2 ("You are not sufficiently Lawful as yet to advance.");
	    else {
		print1 ("You are made a Paladin of the Order!");
		print2 ("You learn the Spell of Heroism and get Mithril Plate!");
		morewait ();
		newitem = ((pob) checkmalloc (sizeof (objtype)));
		*newitem = Objects[ARMORID + 11];	/* mithril plate armor */
		newitem->blessing = 9;
		newitem->known = 2;
		gain_item (newitem);
		morewait ();
		clearmsg ();
		print1 ("To advance you must rescue the Star Gem and return it");
		print2 ("to its owner, the LawBringer, who resides on Star Peak.");
		morewait ();
		print1 ("The Star Gem was stolen by the cursed Prime Sorceror,");
		print2 ("whose headquarters may be found beyond the Astral Plane.");
		morewait ();
		print1 ("The Oracle will send you to the Astral Plane if you");
		print2 ("prove yourself worthy to her.");
		morewait ();
		Spells[S_HERO].known = TRUE;
		Player.rank[ORDER] = PALADIN;
	    }
	} else if (Player.rank[ORDER] == GUARDIAN) {
	    if (Player.guildxp[ORDER] < 1500)
		print2 ("You are not experienced enough to advance.");
	    else if (Player.alignment < 125)
		print2 ("You are not yet sufficiently Lawful to advance.");
	    else {
		Player.rank[ORDER] = CHEVALIER;
		print1 ("You are made a Chevalier of the Order!");
		print2 ("You are given a Mace of Disruption!");
		morewait ();
		clearmsg ();
		newitem = ((pob) checkmalloc (sizeof (objtype)));
		*newitem = Objects[WEAPONID + 25];	/* mace of disruption */
		newitem->known = 2;
		gain_item (newitem);
	    }
	} else if (Player.rank[ORDER] == GALLANT) {
	    if (Player.guildxp[ORDER] < 400)
		print2 ("You are not experienced enough to advance.");
	    else if (Player.alignment < 50)
		print2 ("You are not Lawful enough to advance.");
	    else {
		print1 ("You are made a Guardian of the Order of Paladins!");
		print2 ("You are given a Holy Hand Grenade (of Antioch).");
		morewait ();
		print1 ("You hear a nasal monotone in the distance....");
		print2 ("'...and the number of thy counting shall be 3...'");
		morewait ();
		clearmsg ();
		Player.rank[ORDER] = GUARDIAN;
		newitem = ((pob) checkmalloc (sizeof (objtype)));
		*newitem = Objects[ARTIFACTID + 7];	/* holy hand grenade. */
		newitem->known = 2;
		gain_item (newitem);
	    }
	}
    }
}
