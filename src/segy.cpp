#include "segy.hpp"

using std::make_unique;
using std::move;
using std::string;

namespace sedaman {
static uint8_t constexpr e2a[256] = {
    0x00,0x01,0x02,0x03,0x9C,0x09,0x86,0x7F,0x97,0x8D,0x8E,0x0B,0x0C,0x0D,0x0E,0x0F,
    0x10,0x11,0x12,0x13,0x9D,0x85,0x08,0x87,0x18,0x19,0x92,0x8F,0x1C,0x1D,0x1E,0x1F,
    0x80,0x81,0x82,0x83,0x84,0x0A,0x17,0x1B,0x88,0x89,0x8A,0x8B,0x8C,0x05,0x06,0x07,
    0x90,0x91,0x16,0x93,0x94,0x95,0x96,0x04,0x98,0x99,0x9A,0x9B,0x14,0x15,0x9E,0x1A,
    0x20,0xA0,0xE2,0xE4,0xE0,0xE1,0xE3,0xE5,0xE7,0xF1,0xA2,0x2E,0x3C,0x28,0x2B,0x7C,
    0x26,0xE9,0xEA,0xEB,0xE8,0xED,0xEE,0xEF,0xEC,0xDF,0x21,0x24,0x2A,0x29,0x3B,0xAC,
    0x2D,0x2F,0xC2,0xC4,0xC0,0xC1,0xC3,0xC5,0xC7,0xD1,0xA6,0x2C,0x25,0x5F,0x3E,0x3F,
    0xF8,0xC9,0xCA,0xCB,0xC8,0xCD,0xCE,0xCF,0xCC,0x60,0x3A,0x23,0x40,0x27,0x3D,0x22,
    0xD8,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0xAB,0xBB,0xF0,0xFD,0xFE,0xB1,
    0xB0,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x70,0x71,0x72,0xAA,0xBA,0xE6,0xB8,0xC6,0xA4,
    0xB5,0x7E,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0xA1,0xBF,0xD0,0xDD,0xDE,0xAE,
    0x5E,0xA3,0xA5,0xB7,0xA9,0xA7,0xB6,0xBC,0xBD,0xBE,0x5B,0x5D,0xAF,0xA8,0xB4,0xD7,
    0x7B,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0xAD,0xF4,0xF6,0xF2,0xF3,0xF5,
    0x7D,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,0x50,0x51,0x52,0xB9,0xFB,0xFC,0xF9,0xFA,0xFF,
    0x5C,0xF7,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0xB2,0xD4,0xD6,0xD2,0xD3,0xD5,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0xB3,0xDB,0xDC,0xD9,0xDA,0x9F
};

static char constexpr default_text_header[] =
        "C 1 CLIENT                        COMPANY                       CREW NO         "
        "C 2 LINE            AREA                        MAP ID                          "
        "C 3 REEL NO           DAY-START OF REEL     YEAR      OBSERVER                  "
        "C 4 INSTRUMENT: MFG            MODEL            SERIAL NO                       "
        "C 5 DATA TRACES/RECORD        AUXILIARY TRACES/RECORD         CDP FOLD          "
        "C 6 SAMPLE INTERVAL        SAMPLES/TRACE        BITS/IN     BYTES/SAMPLE        "
        "C 7 RECORDING FORMAT       FORMAT THIS REEL         MEASUREMENT SYSTEM          "
        "C 8 SAMPLE CODE: FLOATING PT     FIXED PT     FIXED PT-GAIN     CORRELATED      "
        "C 9 GAIN  TYPE: FIXED     BINARY     FLOATING POINT     OTHER                   "
        "C10 FILTERS: ALIAS     HZ  NOTCH     HZ  BAND     -     HZ  SLOPE    -    DB/OCT"
        "C11 SOURCE: TYPE            NUMBER/POINT        POINT INTERVAL                  "
        "C12     PATTERN:                           LENGTH        WIDTH                  "
        "C13 SWEEP: START     HZ END      HZ  LENGTH      MS  CHANNEL NO     TYPE        "
        "C14 TAPER: START LENGTH       MS  END LENGTH       MS TYPE                      "
        "C15 SPREAD: OFFSET        MAX DISTANCE        GROUP INTEVAL                     "
        "C16 GEOPHONES: PER GROUP     SPACEING    FREQUENCY     MFG          MODEL       "
        "C17     PATTERN:                           LENGTH        WIDTH                  "
        "C18 TRACES SORTED BY: RECORD     CDP     OTHER                                  "
        "C19 AMPLITUDE RECOVERY: NONE      SPHERICAL DIV       AGC    OTHER              "
        "C20 MAP PROJECTION                      ZONE ID       COORDINATE UNITS          "
        "C21 PROCESSING:                                                                 "
        "C22 PROCESSING:                                                                 "
        "C23                                                                             "
        "C24                                                                             "
        "C25                                                                             "
        "C26                                                                             "
        "C27                                                                             "
        "C28                                                                             "
        "C29                                                                             "
        "C30                                                                             "
        "C31                                                                             "
        "C32                                                                             "
        "C33                                                                             "
        "C34                                                                             "
        "C35                                                                             "
        "C36                                                                             "
        "C37                                                                             "
        "C38                                                                             "
        "C39 SEG-Y_REV2.0                                                                "
        "C40 END TEXTUAL HEADER                                                          ";

class segy::impl {
public:
    impl(string const &file_name, binary_header const &bin_hdr,
         string const &text_hdr)
        : d_file_name(file_name), d_text_header(text_hdr),
          d_bin_hdr(bin_hdr) {}
    impl(string &&file_name, binary_header &&bin_hdr, string &&text_hdr)
        : d_file_name(move(file_name)), d_text_header(move(text_hdr)),
          d_bin_hdr(move(bin_hdr)) {}

    string d_file_name;
    string d_text_header;
    binary_header d_bin_hdr;
};

segy::segy(string const &file_name, binary_header const &bin_hdr,
           string const &text_hdr)
    : pimpl(make_unique<impl>(file_name, bin_hdr, text_hdr)) {}

segy::segy(string &&file_name, binary_header &&bin_hdr,
           string &&text_hdr)
    : pimpl(make_unique<impl>(move(file_name), move(bin_hdr),
                              move(text_hdr))) {}

segy::~segy() = default;

string segy::ebcdic_to_ascii(string &ebcdic)
{
    string result;
    result.reserve((ebcdic.size()));

    for (auto from = ebcdic.begin(), to = result.begin(), end = ebcdic.end();
         from != end; ++from)
        *to = static_cast<char>(e2a[static_cast<uint8_t>(*from)]);

    return result;
}

inline string const &segy::file_name() {return pimpl->d_file_name;}
inline segy::binary_header const &segy::bin_hdr() {return pimpl->d_bin_hdr;}
inline void segy::set_bin_hdr(binary_header const &b_h) {pimpl->d_bin_hdr = b_h;}
inline void segy::set_bin_hdr(binary_header &&b_h) {pimpl->d_bin_hdr = b_h;}
inline string const &segy::text_hdr() {return pimpl->d_text_header;}
inline void segy::set_text_hdr(string const &t_h) {pimpl->d_text_header = t_h;}
inline void segy::set_text_hdr(string &&t_h) {pimpl->d_text_header = t_h;}
} // namespace sedaman
