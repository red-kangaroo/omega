#include "glob.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/file.h>
#include <fcntl.h>
#include <errno.h>

//----------------------------------------------------------------------

void user_intro (void)
{
    displayfile (Data_Intro);
    xredraw();
}

void abyss_file (void)
{
    displayfile (Data_AbyssIntro);
}

void inv_help (void)
{
    displayfile (Help_Inventory);
    xredraw();
}

void combat_help (void)
{
    displayfile (Help_Combat);
    menuclear();
}

void cityguidefile (void)
{
    displayfile (Data_ScrollRampartGuide);
    xredraw();
}

void wishfile (void)
{
    displayfile (Data_ScrollHiMagick);
    xredraw();
}

void adeptfile (void)
{
    displayfile (Data_ScrollAdept);
    xredraw();
}

void theologyfile (void)
{
    displayfile (Data_ScrollReligion);
    xredraw();
}

void showmotd (void)
{
    displayfile (Data_Title);
}

// display a file given a string name of file
void displayfile (const char* filestr)
{
    clear();
    refresh();
    const char* ld = filestr;
    char d;
    for (d = ' '; *ld && d != 'q' && d != KEY_ESCAPE;) {
	if (getcury(stdscr) > ScreenLength) {
	    standout();
	    printw ("\n-More-");
	    standend();
	    refresh();
	    d = getch();
	    clear();
	}
	addch (*ld++);
    }
    if ((char) d != 'q' && (char) d != KEY_ESCAPE) {
	standout();
	printw ("\n-Done-");
	standend();
	refresh();
	getch();
    }
    clear();
    refresh();
}
