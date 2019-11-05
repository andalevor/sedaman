#include "isegy.hpp"

using std::make_unique;
using std::move;
using std::string;

namespace sedaman {
class isegy::impl {
public:
};

isegy::isegy(string const &file_name)
    : segy(file_name), pimpl(make_unique<impl>()) {}

isegy::isegy(string &&file_name)
    : segy(move(file_name)), pimpl(make_unique<impl>()) {}

isegy::~isegy() = default;
}
