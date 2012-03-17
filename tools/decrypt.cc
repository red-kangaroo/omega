#include <stdio.h>
#include <string.h>

char cryptkey (const char* fname)
{
    if (!strcmp(fname + strlen(fname) - 4, ".txt"))
	return 100;
    else if (!strncmp(fname, "maze", 4))
	fname = "mazes";
    else if (!strncmp(fname, "villag", 6))
	fname = "village.dat";
    int key = 0;
    for (int pos = 0; fname[pos]; pos++)
	key += 3*(fname[pos] - ' ');
    return (key&0xff);
}

int main (int num_args, const char* const* args)
{
    if (num_args != 2) {
	fprintf(stderr, "Usage: %s (key) < (infile) > (outfile)\nwhere (key) happens to be the name of the file, without any preceding path.\n", args[0]);
	return (-1);
    }
    char key = cryptkey(args[1]);
    for (int c; (c = getchar()) != EOF; putchar(key))
	key = c^key;
    return (0);
}
