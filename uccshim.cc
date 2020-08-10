#include "config.h"
#if !USE_UCC
#include "uccshim.h"

//----------------------------------------------------------------------

void memblock::read_file (const char* filename)
{
    int fd = open (filename, O_RDONLY);
    struct stat st;
    fstat (fd, &st);
    resize (st.st_size);
    ssize_t br = read (fd, begin(), size());
    close (fd);
    if (br != size())
	resize (0);
}
void memblock::write_file (const char* filename)
{
    int fd = creat (filename, 0600);
    ssize_t bw = write (fd, begin(), size());
    close (fd);
    if (bw != size())
	perror ("failed to write file");
}

//----------------------------------------------------------------------

static unsigned s_RandState[4] alignas(16) = { 88675123, 123456789, 362436069, 521288629 };

/// Random number generator
unsigned xrand (void)
{
    unsigned t = s_RandState[1] ^ (s_RandState[1] << 11);
    s_RandState[1] = s_RandState[2]; s_RandState[2] = s_RandState[3]; s_RandState[3] = s_RandState[0];
    return s_RandState[0] = s_RandState[0] ^ (s_RandState[0] >> 19) ^ t ^ (t >> 8);
}

/// Initializes random number generator with \p seed
void sxrand (unsigned seed)
{
    for (unsigned i = 0; i < 4; ++i)
	s_RandState[i] = seed = seed*88675123-1;
}

#endif	// if !USE_UCC
