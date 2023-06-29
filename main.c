// Original source: https://stackoverflow.com/a/51932253/3977812
// License: CC-BY-SA-4.0
// Copyright https://stackoverflow.com/users/1475978/nominal-animal

#define  _POSIX_C_SOURCE  200809L
#define  _GNU_SOURCE

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static inline int read_from(const int fd, void *const to, const size_t len, const off_t offset) {
    char *p = (char *) to;
    char *const q = (char *) to + len;
    ssize_t n;

    if (lseek(fd, offset, SEEK_SET) != offset)
        return errno = EIO;

    while (p < q) {
        n = read(fd, p, (size_t) (q - p));
        if (n > 0)
            p += n;
        else if (n != -1)
            return errno = EIO;
        else if (errno != EINTR)
            return errno;
    }

    return 0;
}

static inline int write_to(const int fd, const void *const from, const size_t len, const off_t offset) {
    const char *const q = (const char *) from + len;
    const char *p = (const char *) from;
    ssize_t n;

    if (lseek(fd, offset, SEEK_SET) != offset)
        return errno = EIO;

    while (p < q) {
        n = write(fd, p, (size_t) (q - p));
        if (n > 0)
            p += n;
        else if (n != -1)
            return errno = EIO;
        else if (errno != EINTR)
            return errno;
    }

    if (fdatasync(fd) != 0)
        return errno;

    return 0;
}

int main(int argc, char *argv[]) {
    unsigned long tests, n, merrs = 0, werrs = 0;
    size_t page;
    long *map, data[2];
    int fd;
    char *fname = "test.dat";

    tests = 5;
    fprintf(stderr, "Starting %lu tests with filename %s\n", tests, fname);

    /* Create the file. */
    page = sysconf(_SC_PAGESIZE);
    fd = open(fname, O_RDWR | O_CREAT | O_EXCL, 0644);
    if (fd == -1) {
        fprintf(stderr, "%s: Cannot create file: %s.\n", fname, strerror(errno));
        return EXIT_FAILURE;
    }
    if (ftruncate(fd, page) == -1) {
        fprintf(stderr, "%s: Cannot resize file: %s.\n", fname, strerror(errno));
        unlink(fname);
        return EXIT_FAILURE;
    }

    /* Map it. */
    map = mmap(NULL, page, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        fprintf(stderr, "%s: Cannot map file: %s.\n", fname, strerror(errno));
        unlink(fname);
        close(fd);
        return EXIT_FAILURE;
    }

    /* Test loop. */
    for (n = 0; n < tests; n++) {

        /* Update map. */
        map[0] = (long) (n + 1);
        map[1] = (long) (~n);

        msync(map, 2 * sizeof map[0], MAP_SYNC);

        /* Check the file contents. */
        if (read_from(fd, data, sizeof data, 0)) {
            fprintf(stderr, "read_from() failed: %s.\n", strerror(errno));
            munmap(map, page);
            unlink(fname);
            close(fd);
            return EXIT_FAILURE;
        }
        werrs += (data[0] != (long) (n + 1) || data[1] != (long) (~n));

        /* Update data. */
        data[0] = (long) (n * 386131);
        data[1] = (long) (n * -257);
        if (write_to(fd, data, sizeof data, 0)) {
            fprintf(stderr, "write_to() failed: %s.\n", strerror(errno));
            munmap(map, page);
            unlink(fname);
            close(fd);
            return EXIT_FAILURE;
        }
        merrs += (map[0] != (long) (n * 386131) || map[1] != (long) (n * -257));
    }

    munmap(map, page);
    unlink(fname);
    close(fd);

    if (!werrs && !merrs)
        fprintf(stderr, "[PASS] No errors detected.\n");
    else {
        if (werrs)
            fprintf(stderr,"Detected %lu times (%.3f%%) when file contents were incorrect.\n",
                   werrs, 100.0 * (double) werrs / (double) tests);
        if (merrs)
            fprintf(stderr,"Detected %lu times (%.3f%%) when mapping was incorrect.\n",
                   merrs, 100.0 * (double) merrs / (double) tests);
    }

    return EXIT_SUCCESS;
}
