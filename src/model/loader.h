#ifndef GABBY_MODEL_LOADER_H_
#define GABBY_MODEL_LOADER_H_

#include <string>
#include <string_view>
#include <unordered_map>

#include "model/model.h"

namespace gabby {
namespace model {

std::unordered_map<std::string, Model> LoadModels(
    const std::string_view models_dir);

}  // namespace model
}  // namespace gabby

#endif  // GABBY_MODEL_LOADER_H_
