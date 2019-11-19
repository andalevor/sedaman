#include <vector>
#include <unordered_map>
#include "config.hpp"

using std::make_unique;
using std::move;
using std::pair;
using std::string;
using std::unordered_map;
using std::vector;

namespace sedaman {
class config::impl {
public:
    vector<unordered_map<string, pair<int, int>>> maps;
};

config::~config() = default;
} // namespace sedaman
