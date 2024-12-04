#include "inference/config.h"

#include "json/parser.h"
#include "utils/logging.h"

namespace gabby {
namespace inference {

namespace fs = std::filesystem;

std::unique_ptr<InferenceConfig> LoadConfig(const std::filesystem::path& dir) {
    LOG(DEBUG) << "loading model from: " << dir;
    auto config = json::ParseFile(dir / "config.json");
    auto gen_config = json::ParseFile(dir / "generation_config.json");
    auto special_tokens_map = json::ParseFile(dir / "special_tokens_map.json");
    auto tok_config = json::ParseFile(dir / "tokenizer_config.json");
    auto tok = json::ParseFile(dir / "tokenizer.json");
    auto tensors = Safetensors::LoadFile(dir / "model.safetensors");
    LOG(DEBUG) << "successfully loaded model";
    return std::unique_ptr<InferenceConfig>(new InferenceConfig{
        .config = config,
        .gen_config = gen_config,
        .special_tokens_map = special_tokens_map,
        .tok_config = tok_config,
        .tok = tok,
        .tensors = std::move(tensors),
    });
}

constexpr std::string_view kUserRelativeSnapshotDir =
    ".cache/huggingface/hub/models--meta-llama--Llama-3.2-1B-Instruct/"
    "snapshots";

fs::path FindDefaultModelDir() {
    const char* home = getenv("HOME");
    if (home == nullptr) throw std::runtime_error("env var HOME is unset");

    fs::path snapshots_dir(home);
    snapshots_dir /= kUserRelativeSnapshotDir;
    fs::directory_iterator contents;
    try {
        contents = fs::directory_iterator(snapshots_dir);
    } catch (fs::filesystem_error& err) {
        throw std::runtime_error(std::format("can't access model dir at {}: {}",
                                             snapshots_dir.string(),
                                             err.what()));
    }

    auto it = fs::begin(contents);
    if (it == fs::end(contents)) {
        throw std::runtime_error(
            std::format("no snapshots found in {}", snapshots_dir.string()));
    }

    return *it;
}
}  // namespace inference
}  // namespace gabby
