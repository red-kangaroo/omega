#include "glob.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/file.h>
#include <fcntl.h>
#include <errno.h>

//----------------------------------------------------------------------

static void displaycryptfile(const char* filestr);

//----------------------------------------------------------------------

FILE* checkfopen (const char* filestring, const char* optionstring)
{
    FILE *fd;
    char response;

    fd = fopen (filestring, optionstring);
    clearmsg();
    while (fd == NULL) {
	print3 ("Warning! Error opening file:");
	nprint3 (filestring);
	print1 (" Abort or Retry? [ar] ");
	do
	    response = (char) mcigetc();
	while ((response != 'a') && (response != 'r'));
	if (response == 'r')
	    fd = fopen (filestring, optionstring);
	else {
	    print2 ("Sorry 'bout that.... Saving character, then quitting.");
	    morewait();
	    save (optionp (COMPRESS_OPTION), TRUE);
	    endgraf();
	    exit (0);
	}
    }
    return (fd);
}

void user_intro (void)
{
    displaycryptfile (OMEGALIB "intro.txt");
    xredraw();
}

void abyss_file (void)
{
    displaycryptfile (OMEGALIB "abyss.txt");
}

void inv_help (void)
{
    displayfile (OMEGALIB "help3.txt");
    xredraw();
}

void combat_help (void)
{
    displayfile (OMEGALIB "help5.txt");
    menuclear();
}

void cityguidefile (void)
{
    displaycryptfile (OMEGALIB "scroll2.txt");
    xredraw();
}

void wishfile (void)
{
    displaycryptfile (OMEGALIB "scroll3.txt");
    xredraw();
}

void adeptfile (void)
{
    displaycryptfile (OMEGALIB "scroll4.txt");
    xredraw();
}

void theologyfile (void)
{
    displaycryptfile (OMEGALIB "scroll1.txt");
    xredraw();
}

void showmotd (void)
{
    displayfile (OMEGALIB "motd.txt");
}

static const char* required_file_list[] = {
    "city.dat", "country.dat", "dlair.dat", "misle.dat", "court.dat",
    "speak.dat", "temple.dat", "abyss.dat", "village1.dat", "village2.dat",
    "village3.dat", "village4.dat", "village5.dat", "village6.dat",
    "home1.dat", "home2.dat", "home3.dat", "arena.dat", "maze1.dat",
    "maze2.dat", "maze3.dat", "maze4.dat", "motd.txt", "circle.dat", NULL
};

static const char* optional_file_list[] = {
    "help1.txt", "help2.txt", "help3.txt", "help4.txt", "help5.txt",
    "help6.txt", "help7.txt", "help8.txt", "help9.txt", "help10.txt",
    "help11.txt", "help12.txt", "help13.txt", "abyss.txt", "scroll1.txt",
    "scroll2.txt", "scroll3.txt", "scroll4.txt", NULL
};

// Checks existence of omega data files
// Returns 1 if OK, 0 if impossible to run, -1 if possible but not OK
int filecheck (void)
{
    int impossible = FALSE, badbutpossible = FALSE;
    int endpos;
    int file;

    strcpy (Str1, OMEGALIB);
    endpos = strlen (Str1);
    for (file = 0; required_file_list[file]; file++) {
	strcpy (&(Str1[endpos]), required_file_list[file]);
	if (access (Str1, R_OK)) {
	    impossible = TRUE;
	    printf ("\nError! File not accessible: %s", Str1);
	}
    }
    for (file = 0; optional_file_list[file]; file++) {
	strcpy (&(Str1[endpos]), optional_file_list[file]);
	if (access (Str1, R_OK)) {
	    badbutpossible = TRUE;
	    printf ("\nWarning! File not accessible: %s", Str1);
	}
    }
    if (impossible) {
	printf ("\nFurther execution is impossible. Sorry.");
	printf ("\nOMEGALIB may be badly #defined in defs.h\n");
	return (0);
    } else if (badbutpossible) {
	printf ("\nFurther execution may cause anomalous behavior.");
	printf ("\nContinue anyhow? [yn] ");
	if (getchar() == 'y')
	    return (-1);
	else
	    return (0);
    } else
	return (1);
}

// display a file given a string name of file
void displayfile (const char* filestr)
{
    FILE *fd = checkfopen (filestr, "r");
    int c, d = ' ';
    clear();
    refresh();
    c = fgetc (fd);
    while ((c != EOF) && ((char) d != 'q') && ((char) d != KEY_ESCAPE)) {
	if (getcury(stdscr) > ScreenLength) {
	    standout();
	    printw ("\n-More-");
	    standend();
	    refresh();
	    d = wgetch (stdscr);
	    clear();
	}
	printw ("%c", (char) c);
	c = fgetc (fd);
    }
    if (((char) d != 'q') && ((char) d != KEY_ESCAPE)) {
	standout();
	printw ("\n-Done-");
	standend();
	refresh();
	getch();
    }
    clear();
    refresh();
    fclose (fd);
}

// display a file given a string name of file
static void displaycryptfile (const char* filestr)
{
    FILE *fd = checkfopen (filestr, "rb");
    int c, d = ' ';
    char key = 100;

    clear();
    refresh();
    c = fgetc (fd);
    while ((c != EOF) && ((char) d != 'q') && ((char) d != KEY_ESCAPE)) {
	if (getcury(stdscr) > ScreenLength) {
	    standout();
	    printw ("\n-More-");
	    standend();
	    refresh();
	    d = wgetch (stdscr);
	    clear();
	}
	key = ((unsigned char) c) ^ key;
	printw ("%c", key);
	c = fgetc (fd);
    }
    if (((char) d != 'q') && ((char) d != KEY_ESCAPE)) {
	standout();
	printw ("\n-Done-");
	standend();
	refresh();
	getch();
    }
    clear();
    refresh();
    fclose (fd);
}

// copy a file given a string name of file
void copyfile (const char* srcstr)
{
    char deststr[80];
    char buffer[STRING_LEN];
    FILE *in, *out;

    print1 ("Enter name of file to create: ");
    strcpy (deststr, msgscanstring());
    if (strlen (deststr) == 0) {
	print2 ("Aborting...");
	morewait();
	return;
    }
    in = checkfopen (srcstr, "rb");
    out = fopen (deststr, "wb");
    if (!out) {
	sprintf (buffer, "Unable to write to file %s - Aborting.", deststr);
	print2 (buffer);
	morewait();
	fclose (in);
	return;
    }
    print2 ("Copying file....");
    while (fgets (buffer, STRING_LEN, in))
	fputs (buffer, out);
    fclose (in);
    fclose (out);
    print3 ("Done.");
}
