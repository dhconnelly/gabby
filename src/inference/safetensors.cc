#include "inference/safetensors.h"

#include <fcntl.h>

#include <cstdint>
#include <cstdio>
#include <memory>

#include "json/json.h"
#include "json/parser.h"
#include "utils/logging.h"

namespace gabby {
namespace inference {

/* static */
Safetensors Safetensors::LoadFile(const std::filesystem::path& path) {
    // format: https://github.com/huggingface/safetensors
    size_t file_size = std::filesystem::file_size(path);
    OwnedFd fd = Open(path.c_str(), O_RDONLY);
    OwnedMmap data = Mmap(file_size, std::move(fd));

    // get header size
    uint64_t header_size = 0;
    for (int i = 0; i < 8; i++) {
        header_size |= data.get()[i] << (8 * i);
    }
    LOG(DEBUG) << "header size: " << header_size;

    // get header
    std::string s(data.get() + 8, data.get() + 8 + header_size);
    json::ValuePtr header = json::Parse(s);
    LOG(DEBUG) << "header: " << *header;

    return Safetensors(std::move(data), header, 8 + header_size);
}

}  // namespace inference
}  // namespace gabby
