// Omega is free software, distributed under the MIT license

#include <curses.h>
#include "glob.h"

//----------------------------------------------------------------------

static int aux_display_pack(unsigned start_item, unsigned slot);
static bool aux_slottable (const object& o, int slot);
static void drop_from_slot(int slot);
static int get_item_number(pob o);
static int get_to_pack (const object& o);
static bool item_useable (const object& o, int slot);
static unsigned key_to_index (unsigned key);
static int pack_item_cost(int ii);
static void push_pack (const object& o);
static void put_to_pack(int slot);
static void setchargestr(pob obj, char* cstr);
static void setnumstr(pob obj, char* nstr);
static void setplustr(pob obj, char* pstr);
static bool slottable (const object& o, int slot);
static int take_from_pack (int slot);
static void inventory_control(void);

//----------------------------------------------------------------------

// are two objects equal except for their number field?
bool object::operator== (const object& o) const
{
    return (o.id == id && o.plus == plus && o.charge == 0 &&
	    charge == 0 && o.dmg == dmg && o.hit == hit &&
	    o.aux == aux && o.blessing == blessing && o.usef == usef);
}

object::object (int8_t nx, int8_t ny, unsigned tid, uint16_t n)
: number (n == uint16_t(RANDOM) ? 0 : n)
, x(nx)
, y(ny)
{
    operator= (Objects[tid]);
}

//----------------------------------------------------------------------

void player::add_possession (unsigned slot, const object& o)
{
    object& ps = possessions[slot];
    ps = o;
    if (item_useable (ps, slot)) {
	if (ps.number > 1) {
	    pack.push_back (split_item (ps, ps.number-1));
	    ps.number = 1;
	    mprint ("Putting extra items in pack");
	}
	ps.used = true;
	item_use (&ps);
    }
    calc_melee();
}

void player::swap_possessions (unsigned b)
{
    object& ao = possessions[O_UP_IN_AIR];
    object& bo = possessions[b];
    if (!slottable (ao, b))
	return;
    if (cursed(bo) == true + true) {
	mprint ("That object is cursed, you can't remove it");
	return;
    }
    if (ao == bo) {
	bo.number += ao.number;
	ao.number = 0;
	ao.id = NO_THING;
	return;
    }
    if (item_useable(ao, b)) {
	if (!ao.used && ao.number > 1) {
	    pack.push_back (split_item (ao, ao.number-1));
	    ao.number = 1;
	    mprint ("Putting extra items in pack");
	}
	ao.used = true;
	item_use (&ao);
    }
    if (bo.used) {
	bo.used = false;
	item_use (&bo);
    }
    swap (ao, bo);
    calc_melee();
}

void player::remove_possession (unsigned slot, unsigned number)
{
    object& o = possessions[slot];
    if (o.number > number)
	o.number -= number;
    else {
	if (o.used) {
	    o.used = false;
	    item_use (&o);
	}
	o.id = NO_THING;
    }
    calc_melee();
}

void player::remove_possession (object* o, unsigned number)
{
    for (unsigned i = 0; i < possessions.size(); ++i)
	if (&possessions[i] == o)
	    remove_possession (i, number);
}

void player::remove_all_possessions (void)
{
    for (unsigned i = 0; i < possessions.size(); ++i)
	if (has_possession(i))
	    remove_possession(i);
}

//----------------------------------------------------------------------

// gets a legal amount of money or ABORT
long get_money (long limit)
{
    long c;
    mprint ("How much? ");
    c = parsenum();
    if (c > limit) {
	mprint ("Forget it, buddy.");
	return (ABORT);
    } else
	return (c);
}

// pick up from some location x,y
// Lift entire itemlist off ground, pass it to inventory control, which
// may drop things back onto the now null ground
void pickup_at (int x, int y)
{
    resetgamestatus (FAST_MOVE);
    vector<object> pickedup;
    foreach (o, Level->things) {
	if (o->x == x && o->y == y) {
	    pickedup.push_back (*o);
	    --(o = Level->things.erase(o));
	}
    }
    foreach (o, pickedup) {
	clearmsg();
	mprintf ("Pick up: %s [ynq]: ", itemid(*o));
	char response = ynq();
	if (response == 'q')
	    break;
	else if (response == 'y') {
	    gain_item (*o);
	    --(o = pickedup.erase(o));
	}
    }
    Level->things.insert (Level->things.end(), pickedup.begin(), pickedup.end());
}

// WDT -- convert from a char (keypress) to an item index in
// player inventory
// Item identifiers, in this case the letters of the alphabet minus
// any letters already used for commands.  Yes, there are more here
// than could be needed, but I don't want to short myself for later.
static const char inventory_keymap[] = "-abcfghimnoqruvwyz";
static unsigned key_to_index (unsigned key)
{
    for (unsigned i = 0; i < ArraySize(inventory_keymap); i++)
	if (key == (unsigned) inventory_keymap[i])
	    return (i);
    return (O_UP_IN_AIR);
}
char index_to_key (unsigned i)
{
    return (i < ArraySize(inventory_keymap) ? inventory_keymap[i] : '-');
}

// criteria for being able to put some item in some slot
static bool aux_slottable (const object& o, int slot)
{
    return (o.id != NO_THING &&
	    (slot != O_ARMOR || o.objchar == ARMOR) &&
	    (slot != O_SHIELD || o.objchar == SHIELD) &&
	    (slot != O_BOOTS || o.objchar == BOOTS) &&
	    (slot != O_CLOAK || o.objchar == CLOAK) &&
	    (slot < O_RING1 || o.objchar == RING));
}

// put all of o at x,y on Level->depth
// Not necessarily dropped by character; just dropped...
void drop_at (int x, int y, const object& o)
{
    if (Level->environment == E_COUNTRYSIDE)
	return;
    if (Level->site(x,y).p_locf == L_VOID_STATION)
	setgamestatus (PREPARED_VOID);
    else if (Level->site(x,y).locchar != VOID_CHAR && Level->site(x,y).locchar != ABYSS)
	Level->add_thing (x, y, o);
}

// put n of o at x,y on Level->depth
void p_drop_at (int x, int y, const object& o, unsigned n)
{
    if (Level->environment == E_COUNTRYSIDE)
	return;
    if (Level->site(x,y).locchar != VOID_CHAR && Level->site(x,y).locchar != ABYSS) {
	Level->add_thing (x, y, o, n);
	mprintf ("Dropped %s", itemid(Level->thing(x,y)));
    } else if (Level->site(x,y).p_locf == L_VOID_STATION)
	setgamestatus (PREPARED_VOID);
}

// returns a string for identified items
const char* itemid (pob obj)
{
    char tstr[80];
    if (obj->objchar == CASH)
	return (obj->truename);
    setnumstr (obj, tstr);
    strcpy (Str4, tstr);
    if (!object_is_known(obj)) {
	strcat (Str4, obj->objstr);
	return (Str4);
    }
    if (obj->id == AMULET_OF_YENDOR || obj->id == JUGGERNAUT_OF_KARNAK || obj->id == STAR_GEM)
	strcat (Str4, "the ");
    if (obj->usef == I_NOTHING && Objects[obj->id].usef != I_NOTHING)
	strcat (Str4, "disenchanted ");
    if (obj->blessing < 0) {
	strcat (Str4, "cursed ");
	strcat (Str4, obj->cursestr);
    } else if (obj->blessing > 0) {
	strcat (Str4, "blessed ");
	strcat (Str4, obj->truename);
    } else
	strcat (Str4, obj->truename);
    if (obj->number > 1)
	strcat (Str4, "s");
    switch (obj->objchar) {
	case STICK:
	    setchargestr (obj, tstr);
	    strcat (Str4, tstr);
	    break;
	case MISSILEWEAPON:
	case ARMOR:
	case RING:
	case SHIELD:
	case WEAPON:
	    setplustr (obj, tstr);
	    strcat (Str4, tstr);
	    break;
    }
    return (Str4);
}

// return an object's plus as a string
static void setplustr (pob obj, char* pstr)
{
    pstr[0] = ' ';
    pstr[1] = (obj->plus < 0 ? '-' : '+');
    if (absv (obj->plus) < 10) {
	pstr[2] = '0' + absv (obj->plus);
	pstr[3] = 0;
    } else {
	pstr[2] = '0' + absv (obj->plus / 10);
	pstr[3] = '0' + absv (obj->plus % 10);
	pstr[4] = 0;
    }
}

// return an object's number as a string
static void setnumstr (pob obj, char* nstr)
{
    if (obj->number < 2)
	nstr[0] = 0;
    else if (obj->number < 10) {
	nstr[0] = '0' + obj->number;
	nstr[1] = 'x';
	nstr[2] = ' ';
	nstr[3] = 0;
    } else if (obj->number < 41) {
	nstr[0] = '0' + ((int) (obj->number / 10));
	nstr[1] = '0' + (obj->number % 10);
	nstr[2] = 'x';
	nstr[3] = ' ';
	nstr[4] = 0;
    } else
	strcpy (nstr, "lots of ");
}

// return object with charges
static void setchargestr (pob obj, char* cstr)
{
    cstr[0] = ' ';
    cstr[1] = '[';
    if (obj->charge < 0) {
	cstr[2] = 'd';
	cstr[3] = 'e';
	cstr[4] = 'a';
	cstr[5] = 'd';
	cstr[6] = ']';
	cstr[7] = 0;
    } else if (obj->charge < 10) {
	cstr[2] = '0' + obj->charge;
	cstr[3] = ']';
	cstr[4] = 0;
    } else {
	cstr[2] = '0' + ((int) (obj->charge / 10));
	cstr[3] = '0' + (obj->charge % 10);
	cstr[4] = ']';
	cstr[5] = 0;
    }
}

void givemonster (monster& m, const object& o)
{
    // special case -- give gem to LawBringer
    if ((m.id == LAWBRINGER) && (o.id == STAR_GEM)) {
	clearmsg();
	mprint ("The LawBringer accepts the gem reverently.");
	mprint ("He raises it above his head, where it bursts into lambent flame!");
	morewait();
	mprint ("You are bathed in a shimmering golden light.");
	mprint ("You feel embedded in an infinite matrix of ordered energy.");
	morewait();
	Imprisonment = 0;
	if (Player.rank[ORDER] == FORMER_PALADIN) {
	    mprint ("You have been forgiven. You feel like a Paladin....");
	    Player.rank[ORDER] = GALLANT;
	}
	Player.alignment += 200;
	Player.pow = Player.maxpow = Player.pow * 2;
	gain_experience (2000);
	setgamestatus (GAVE_STARGEM);
    } else {
	strcpy (Str3, m.name());
	if (m_statusp (m, GREEDY) || m_statusp (m, NEEDY)) {
	    m.pickup (o);
	    strcat (Str3, " takes your gift");
	    Player.alignment++;
	    if (m_statusp (m, GREEDY) && (true_item_value (o) < (long) m.level * 100))
		strcat (Str3, "...but does not appear satisfied.");
	    else if (m_statusp (m, NEEDY) && (true_item_value (o) < (long) Level->depth * Level->depth))
		strcat (Str3, "...and looks chasteningly at you.");
	    else {
		strcat (Str3, "...and seems happy with it.");
		m_status_reset (m, HOSTILE);
		m_status_reset (m, GREEDY);
		m_status_reset (m, NEEDY);
	    }
	    mprint (Str3);
	} else if (m_statusp (m, HUNGRY)) {
	    if (((m.id == HORSE) && (o.id == FOOD_GRAIN)) ||	// grain
		((m.id != HORSE) && ((o.usef == I_FOOD) || (o.usef == I_POISON_FOOD)))) {
		strcat (Str3, " wolfs down your food ... ");
		m_status_reset (m, HUNGRY);
		m_status_reset (m, HOSTILE);
		if (o.usef == I_POISON_FOOD) {
		    Player.alignment -= 2;
		    strcat (Str3, "...and chokes on the poisoned ration!");
		    m_status_set (m, HOSTILE);
		    m_damage (&m, 100, POISON);
		} else
		    strcat (Str3, "...and now seems satiated.");
		mprint (Str3);
		morewait();
	    } else {
		strcat (Str3, " spurns your offering and leaves it on the ground.");
		mprint (Str3);
		drop_at (m.x, m.y, o);
	    }
	} else {
	    strcat (Str3, " doesn't care for your offering and drops it.");
	    mprint (Str3);
	    drop_at (m.x, m.y, o);
	}
    }
}

// select an item from inventory
// if itype is NULL_ITEM, any kind of item is acceptable.
// if itype is CASH, any kind of item or '$' (cash) is acceptable.
// if itype is FOOD, CORPSE or FOOD is acceptable, but only FOOD is
// listed in the possibilities.
// if itype is any other object type (eg SCROLL, POTION, etc.), only
// that type of item is acceptable or is listed
int getitem (chtype itype)
{
    char invstr[64];
    char key;
    int k = 0, ok = false, drewmenu = false, found = false;

    found = ((itype == NULL_ITEM) || ((itype == CASH) && (Player.cash > 0)));
    invstr[0] = 0;
    foreach (o, Player.possessions) {
	if (o->id == NO_THING) continue;
	if (itype == NULL_ITEM || itype == CASH || o->objchar == itype || (itype == FOOD && o->objchar == CORPSE)) {
	    found = true;
	    invstr[k++] = index_to_key (distance(Player.possessions.begin(),o));
	    invstr[k] = 0;
	}
    }
    if (itype == CASH && found) {
	invstr[k++] = '$';
	invstr[k] = 0;
    }
    if (!found) {
	mprint ("Nothing appropriate.");
	return (ABORT);
    }
    mprintf ("Select an item [%s,?] ", invstr);
    while (!ok) {
	key = (char) mcigetc();
	if (key == '?') {
	    drewmenu = true;
	    display_possessions();
	} else if (key == KEY_ESCAPE)
	    ok = true;
	else if (key == (CASH & 0xff)) {
	    if (itype == CASH)
		ok = true;
	    else {
		mprint ("You cannot select cash now.");
		ok = false;
	    }
	} else if (!strmem (key, invstr) || key_to_index (key) == O_UP_IN_AIR)
	    mprint ("Nope! Try again [? for inventory, ESCAPE to quit]:");
	else
	    ok = true;
    }
    if (drewmenu)
	xredraw();
    if (key == KEY_ESCAPE)
	return (ABORT);
    else if (key == (CASH & 0xff))
	return (CASHVALUE);
    else
	return key_to_index (key);
}

void gain_item (const object& o)
{
    if (object_uniqueness(o) == UNIQUE_MADE)
	set_object_uniqueness (o, UNIQUE_TAKEN);
    if (o.objchar == CASH) {
	mprint ("You gained some cash.");
	Player.cash += o.basevalue;
	dataprint();
    } else if (optionp (PACKADD)) {
	if (!get_to_pack (o)) {
	    Player.possessions[O_UP_IN_AIR] = o;
	    do_inventory_control();
	}
    } else {
	Player.possessions[O_UP_IN_AIR] = o;
	do_inventory_control();
    }
}

// inserts the item at the start of the pack array
static void push_pack (const object& o)
{
    Player.pack.insert (Player.pack.begin(), o);
}

// Adds item to pack list, maybe going into inventory mode if pack is full
static int get_to_pack (const object& o)
{
    if (Player.pack.size() >= MAXPACK) {
	mprint ("Your pack is full.");
	morewait();
	return (false);
    } else {
	push_pack (o);
	mprint ("Putting item in pack.");
	return (true);
    }
}

static int pack_item_cost (int ii)
{
    return (ii > 20 ? 17 : (ii > 15 ? 7 : 2));
}

// WDT -- 'response' must be an index into the pack.
static void use_pack_item (int response, int slot)
{
    unsigned i = pack_item_cost (response);
    if (i > 10) {
	mprint ("You begin to rummage through your pack.");
	morewait();
    }
    if (i > 5) {
	mprint ("You search your pack for the item.");
	morewait();
    }
    mprint ("You take the item from your pack.");
    morewait();
    Command_Duration += i;
    Player.add_possession (slot, Player.pack[response]);
    Player.pack.erase (Player.pack.iat(response));
}

// WDT HACK!  This ought to be in scr.c, along with its companion.  However,
// right now it's only used in the function directly below.
static int aux_display_pack (unsigned start_item, unsigned slot)
{
    unsigned i = start_item, items;
    const char* depth_string;
    if (Player.pack.empty())
	mprint ("Pack is empty.");
    else if (Player.pack.size() <= start_item)
	mprint ("You see the leather at the bottom of the pack.");
    else {
	menuclear();
	items = 0;
	for (i = start_item; i < Player.pack.size() && items < ScreenLength - 5U; i++) {
	    if (aux_slottable (Player.pack[i], slot)) {
		if (pack_item_cost (i) > 10)
		    depth_string = "**";
		else if (pack_item_cost (i) > 5)
		    depth_string = "* ";
		else
		    depth_string = "  ";
		sprintf (Str1, "  %c: %s %s\n", i + 'a', depth_string, itemid (&Player.pack[i]));
		if (items == 0)
		    menuprint ("Items in Pack:\n");
		menuprint (Str1);
		items++;
	    }
	}
	if (items == 0)
	    menuprint ("You see nothing useful for that slot in the pack.");
	else {
	    menuprint ("\n*: Takes some time to reach; **: buried very deeply.");
	}
	showmenu();
    }
    return i;
}

// takes something from pack, puts to slot, 
// or to 'up-in-air', one of which at least must be empty
static int take_from_pack (int slot)
{
    char response, pack_item;
    if (Player.has_possession(slot))
	slot = O_UP_IN_AIR;
    if (Player.has_possession(slot))
	mprint ("slot is not empty!");
    else if (Player.pack.size() < 1)
	mprint ("Pack is empty!");
    else {
	pack_item = 0;
	int quitting = false, ok = true;
	do {
	    ok = true;
	    unsigned last_item = aux_display_pack (pack_item, slot);
	    if (last_item == Player.pack.size() && pack_item == 0)
		mprint ("Enter pack slot letter or ESCAPE to quit.");
	    else if (last_item == Player.pack.size())
		mprint ("Enter pack slot letter, - to go back, or ESCAPE to quit.");
	    else if (pack_item == 0)
		mprint ("Enter pack slot letter, + to see more, or ESCAPE to quit.");
	    else
		mprint ("Enter pack slot letter, + or - to see more, or ESCAPE to quit.");
	    response = mcigetc();
	    if (response == '?') {
		// WDT HACK -- display some help instead.
		mprint ("Help not implemented (sorry).");
		morewait();
		ok = false;
	    } else if (response == KEY_ESCAPE)
		quitting = true;
	    else if (response == '+') {
		if (last_item < Player.pack.size())
		    pack_item = last_item;
		ok = false;
	    } else if (response == '-') {
		// WDT HACK: this _should_ make us page up.  Sadly,
		// I have no way of calculating how much I'll be paging up.
		// This is fixable, but I have no idea how much work...
		pack_item = 0;
		ok = false;
	    } else {
		ok = ((response >= 'a') && (response < char('a' + Player.pack.size())));
		if (ok)
		    ok = slottable (Player.pack[response - 'a'], slot);
	    }
	} while (!ok);
	if (!quitting) {
	    use_pack_item (response - 'a', slot);
	}
    }
    return slot;
}

void do_inventory_control (void)
{
    menuclear();
    display_possessions();
    inventory_control();
}

// inventory_control assumes a few setup things have been done,
// like displaying the slots, loading the O_UP_IN_AIR item, etc.
//
// Each action uses up a little time. If only inspection actions
// are taken, no time is used up.
static void inventory_control (void)
{
    int slot = 0, done = false;
    int response;
    mprint ("Action [d,e,p,s,t,x,>,<,?,ESCAPE]:");
    do {
	display_possessions (slot);
	response = mcigetc();
	switch (response) {
	    case KEY_CTRL|'l':
	    case KEY_CTRL|'r':
		display_possessions (slot);
		break;
	    case 'd':
		if (!Player.has_possession(slot)) {
		    mprint ("Nothing in selected slot!");
		    break;
		}
		drop_from_slot (slot);
		display_possessions (slot);
		Command_Duration++;
		break;
	    case 'p':
		if (Player.has_possession(slot))
		    put_to_pack (slot);
		Command_Duration += 5;
		break;
	    case 's':
		display_pack();
		morewait();
		display_possessions (slot);
		Command_Duration += 5;
		break;
	    case 't':
		take_from_pack (slot);
		Command_Duration += 5;
		break;
	    case 'e':
		if (slot != O_UP_IN_AIR)
		    Player.swap_possessions (slot);
		display_possessions (slot);
		Command_Duration += 2;
		break;
	    case KEY_ENTER:
	    case 'x':
		if (slot != O_UP_IN_AIR)
		    Player.swap_possessions (slot);
		display_possessions (slot);
		Command_Duration += 2;
		done = !Player.has_possession(O_UP_IN_AIR);
		break;
	    case 'j':
	    case '>':
	    case '2':
	    case KEY_DOWN:
		slot = min(slot+1,MAXITEMS-1);
		break;
	    case 'k':
	    case '<':
	    case '8':
	    case KEY_UP:
		slot = max(slot-1,0);
		break;
	    case KEY_HOME:
	    case '-':
		slot = 0;
		break;
	    case KEY_LL:
	    case '+':
		slot = MAXITEMS-1;
		break;
	    case '?':
		menuclear();
		menuprint ("d:\tDrop up-in-air or current item\n");
		menuprint ("e:\tExchange current slot with up-in-air slot\n");
		menuprint ("p:\tPut up-in-air or current item in pack\n");
		menuprint ("s:\tShow contents of pack\n");
		menuprint ("t:\tTake something from pack into the\n\tcurrent or up-in-air slot\n");
		menuprint ("x:\tAs 'e', but exit if up-in-air slot finishes empty\n");
		menuprint (">:\tMove down one slot/item\n");
		menuprint ("<:\tMove up one slot/item\n");
		menuprint ("?:\tDisplay help (this message + help file)\n");
		menuprint ("ESCAPE:\texit\n");
		showmenu();
		clearmsg();
		mprint ("Display full help? (y/n)");
		if (ynq() == 'y') {
		    displayfile (Help_Inventory);
		    xredraw();
		}
		display_possessions (slot);
		break;
	    case KEY_ESCAPE:
		if (Player.has_possession(O_UP_IN_AIR)) {
		    drop_at (Player.x, Player.y, Player.possessions[O_UP_IN_AIR]);
		    Player.remove_possession(O_UP_IN_AIR);
		    mprint ("Object 'up in air' dropped.");
		}
		done = true;
		break;
	    default:
		if (key_to_index (response) > 0)
		    slot = key_to_index (response);
		break;
	}
	calc_melee();
    } while (!done);
    xredraw();
}

// returns some number between 0 and o->number
static int get_item_number (pob o)
{
    int n = 0;
    if (o->number == 1)
	return 1;
    do {
	clearmsg();
	mprintf ("How many? -- max %u: ", o->number);
	n = parsenum();
	if (n > o->number)
	    mprint ("Too many!");
	else if (n < 1)
	    n = 0;
    } while (n > o->number);
    if (n < 1)
	n = 0;
    return (n);
}

static void drop_from_slot (int slot)
{
    if (Player.has_possession(slot)) {
	if (cursed (Player.possessions[slot]) == true + true)
	    mprint ("It sticks to your fingers!");
	else {
	    int n = get_item_number (&Player.possessions[slot]);
	    if (n > 0) {
		p_drop_at (Player.x, Player.y, Player.possessions[slot], n);
		Player.remove_possession (slot, n);
	    } else
		mprint ("Didn't drop anything.");
	}
    } else
	mprint ("Didn't drop anything.");
}

static void put_to_pack (int slot)
{
    if (!Player.has_possession(slot))
	mprint ("Slot is empty!");
    else if (cursed(Player.possessions[slot]) == true + true)
	mprint ("Item is cursed!");
    else {
	int n = get_item_number (&Player.possessions[slot]);
	if (n > 0) {
	    object dropped = split_item(Player.possessions[slot], n);
	    if (Player.pack.size() >= MAXPACK) {
		mprint ("Your pack is full. The item drops to the ground.");
		drop_at (Player.x, Player.y, dropped);
	    } else {
		push_pack (dropped);
		mprint ("Putting item in pack.");
	    }
	    Player.remove_possession (slot, n);
	}
    }
}

/// Returns a copy of \p item with number set to \p n
object split_item (const object& item, unsigned n)
{
    object nitem = item;
    nitem.number = n;
    nitem.used = false;
    return (nitem);
}

// criteria for being able to put some item in some slot
static bool slottable (const object& o, int slot)
{
    if (o.id == NO_THING) {
	return (false);
    } else if (slot == O_ARMOR && o.objchar != ARMOR) {
	mprint ("Only armor can go in the armor slot!");
	return (false);
    } else if (slot == O_SHIELD && o.objchar != SHIELD) {
	mprint ("Only a shield can go in the shield slot!");
	return (false);
    } else if (slot == O_BOOTS && o.objchar != BOOTS) {
	mprint ("Only boots can go in the boots slot!");
	return (false);
    } else if (slot == O_CLOAK && o.objchar != CLOAK) {
	mprint ("Only a cloak can go in the cloak slot!");
	return (false);
    } else if (slot >= O_RING1 && o.objchar != RING) {
	mprint ("Only a ring can go in a ring slot!");
	return (false);
    }
    return (true);
}

// ->;WDT HACK: this is bad factoring.  I want to use this, but it's
// printing SILLY stuff out whether or not an item o can be used in a
// slot. Assumes o can in fact be placed in the slot.
static bool item_useable (const object& o, int slot)
{
    // don't have to check the object in the first if since only armor
    // can go in armor slot, cloak in cloak slot, etc
    if (slot == O_ARMOR || slot == O_CLOAK || slot == O_SHIELD || slot == O_BOOTS || slot >= O_RING1)
	return (true);
    // weapon is useable if it is put in weapon hand or if it is two-handed
    // and put in either hand when the other also holds the weapon
    else if (o.objchar == WEAPON || o.objchar == MISSILEWEAPON) {
	if (twohandedp (o.id) && (slot == O_READY_HAND || slot == O_WEAPON_HAND)) {
	    mprint ("You heft the weapon and find you must use both hands.");
	    morewait();
	    return (true);
	} else
	    return (slot == O_WEAPON_HAND);
    } else
	return (false);
}

// returns false if not cursed, true if cursed but not used, true + true if cursed and used
int cursed (const object& o)
{
    return (o.blessing < 0 ? (o.used == true) + true : false);
}

// returns true if item with id and charge is found in pack or in
// inventory slot. charge is used to differentiate
// corpses instead of aux, which is their food value.
object* find_item (int id)
{
    for (unsigned i = 1; i < MAXITEMS; i++)
	if (Player.possessions[i].id == id)
	    return (&Player.possessions[i]);
    for (unsigned i = 0; i < Player.pack.size(); i++)
	if (Player.pack[i].id == id)
	    return (&Player.pack[i]);
    return (NULL);
}

// returns true if item with id and charge is found in pack or in
// inventory slot. Destroys item. charge is used to differentiate
// corpses instead of aux, which is their food value.
int find_and_remove_item (int id, int chargeval)
{
    for (unsigned i = 1; i < MAXITEMS; i++) {
	if (Player.possessions[i].id == id && (chargeval == -1 || Player.possessions[i].charge == chargeval)) {
	    Player.remove_possession (i);
	    return (true);
	}
    }
    foreach (i, Player.pack) {
	if (i->id != id || (chargeval != -1 && i->charge != chargeval))
	    continue;
	if (--i->number == 0)
	    Player.pack.erase (i);
	return (true);
    }
    return (false);
}

void lose_all_items (void)
{
    mprint ("You notice that you are completely devoid of all possessions.");
    morewait();
    Player.pack.clear();
    Player.remove_all_possessions();
    morewait();
}
