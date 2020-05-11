#include "CommonSegd.hpp"
#include "Exception.hpp"

using std::fstream;
using std::make_unique;
using std::move;
using std::string;
using std::vector;

namespace sedaman {
class CommonSegd::Impl {
public:
    explicit Impl(string name)
        : file_name(move(name))
        , gen_hdr_buf(CommonSegd::GeneralHeader::SIZE)
    {
    }
    string file_name;
    fstream file;
    GeneralHeader gen_hdr;
    vector<char> gen_hdr_buf;
};

CommonSegd::CommonSegd(string name, fstream::openmode mode)
    : pimpl(make_unique<Impl>(move(name)))
{
    fstream fl;
    fl.exceptions(fstream::failbit | fstream::badbit);
    fl.open(pimpl->file_name, mode);
    pimpl->file = move(fl);
}

CommonSegd::~CommonSegd() = default;

CommonSegd::GeneralHeader& CommonSegd::p_general_header() { return pimpl->gen_hdr; }
vector<char>& CommonSegd::p_gen_hdr_buf() { return pimpl->gen_hdr_buf; }
} //sedaman
