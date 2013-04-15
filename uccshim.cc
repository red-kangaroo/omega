#include "config.h"
#if !USE_UCC
#include "uccshim.h"

void memblock::read_file (const char* filename)
{
    int fd = open (filename, O_RDONLY);
    struct stat st;
    fstat (fd, &st);
    resize (st.st_size);
    ssize_t br = read (fd, begin(), size());
    close (fd);
    if (br != size())
	throw runtime_error ("failed to read file");
}
void memblock::write_file (const char* filename)
{
    int fd = creat (filename, 0600);
    ssize_t bw = write (fd, begin(), size());
    close (fd);
    if (bw != size())
	throw runtime_error ("failed to write file");
}

#endif	// if !USE_UCC
