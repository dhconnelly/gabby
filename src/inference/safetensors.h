#ifndef GABBY_INFERENCE_SAFETENSORS_H_
#define GABBY_INFERENCE_SAFETENSORS_H_

#include <filesystem>
#include <stdexcept>

#include "json/json.h"
#include "utils/pointers.h"

namespace gabby {
namespace inference {

class Safetensors {
public:
    static Safetensors LoadFile(const std::filesystem::path& path);
    const json::ValuePtr header() const { return header_; }

private:
    Safetensors(OwnedMmap mem, json::ValuePtr header, size_t data_offset)
        : mem_(std::move(mem)), header_(header), data_offset_(data_offset) {}

    OwnedMmap mem_;
    json::ValuePtr header_;
    size_t data_offset_ = 0;
};

}  // namespace inference
}  // namespace gabby

#endif  // GABBY_INFERENCE_SAFETENSORS_H_
