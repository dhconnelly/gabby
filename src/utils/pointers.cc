#include "utils/pointers.h"

#include <fcntl.h>
#include <sys/mman.h>

#include <cerrno>

#include "utils/logging.h"

namespace gabby {

OwnedStream Fopen(const char *name, const char *mode) {
    std::unique_ptr<FILE, decltype(&fclose)> f(fopen(name, mode), fclose);
    if (f.get() == nullptr) throw SystemError(errno);
    return std::move(f);
}

OwnedFd Own(int fd) {
    return OwnedFd(new int(fd), [](int *fdp) {
        if (fdp && *fdp >= 0) {
            close(*fdp);
            delete fdp;
        }
    });
}

OwnedFd Open(const char *name, int flags) {
    int fd = open(name, flags);
    if (fd < 0) throw SystemError(errno);
    return Own(fd);
}

void MmapDeleter::operator()(uint8_t *p) { munmap(p, size); }

OwnedMmap Mmap(size_t size, OwnedFd fd) {
    void *data = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, *fd.release(), 0);
    return OwnedMmap(static_cast<uint8_t *>(data), MmapDeleter{.size = size});
}

}  // namespace gabby
