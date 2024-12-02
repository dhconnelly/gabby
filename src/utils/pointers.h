#ifndef GABBY_UTILS_POINTERS_H_
#define GABBY_UTILS_POINTERS_H_

#include <cstdint>
#include <cstdio>
#include <memory>
#include <string_view>

namespace gabby {

using OwnedStream = std::unique_ptr<FILE, decltype(&fclose)>;

OwnedStream Fopen(const char *name, const char *mode);

using OwnedFd = std::unique_ptr<int, void (*)(int *)>;

OwnedFd Own(int fd);

OwnedFd Open(const char *name, int flags);

struct MmapDeleter {
    size_t size;
    void operator()(uint8_t *p);
};

using OwnedMmap = std::unique_ptr<uint8_t, MmapDeleter>;

OwnedMmap Mmap(size_t size, OwnedFd fd);

}  // namespace gabby

#endif  // GABBY_UTILS_POINTERS_H_
