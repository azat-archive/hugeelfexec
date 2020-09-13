// See also https://github.com/abbat/elfexec/blob/master/elfexec.c

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

void perror_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    int memfd;
    int binfd;
    const char *binary_path;
    struct stat binary_stat;
    void *binary_addr;
    void *mem_addr;
    size_t size;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s /path/to/bin args-for-bin\n", argv[0]);
        exit(1);
    }

    binary_path = argv[1];
    if (access(binary_path, R_OK|X_OK))
        perror_exit("access(binary)");
    binfd = open(binary_path, O_RDONLY|O_CLOEXEC);
    if (binfd < 0)
        perror_exit("open(binary)");
    if (fstat(binfd, &binary_stat))
        perror_exit("stat(binary)");
    size = binary_stat.st_size;

    binary_addr = mmap(NULL, size, PROT_READ, MAP_SHARED, binfd, 0);
    if (!binary_addr)
        perror_exit("mmap(binary)");

    // Do not uses MFD_HUGETLB to avoid hugetlbfs setup requirement
    // Will use madvise(MADV_HUGEPAGE) later.
    memfd = memfd_create("hugeelfexec", MFD_CLOEXEC);
    if (!memfd)
        perror_exit("memfd_create");
    if (ftruncate(memfd, size))
        perror_exit("ftruncate(memfd)");
    mem_addr = mmap(NULL, size, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_SHARED, memfd, 0);
    if (!mem_addr)
        perror_exit("mmap(memfd)");

    // Required only if /sys/kernel/mm/transparent_hugepage/shmem_enabled == advise
    if (madvise(mem_addr, size, MADV_HUGEPAGE))
        perror_exit("madvise");

    // Suboptimal
    memcpy(mem_addr, binary_addr, size);

    munmap(mem_addr, size);
    munmap(binary_addr, size);

    if (fexecve(memfd, &argv[1] /* skip "./hugeelfexec" */, environ) == -1)
        perror_exit("fexecve");

    return EXIT_SUCCESS;
}
