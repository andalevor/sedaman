#include "CommonSegy.hpp"

using std::fstream;
using std::ios_base;
using std::move;
using std::string;
using std::valarray;

namespace sedaman {
CommonSegy::CommonSegy(string const& name, ios_base::openmode mode)
    : file_name { name }
    , hdr_buf { valarray<uint8_t>(CommonSegy::TR_HEADER_SIZE) }
{
    fstream fl;
    fl.exceptions(fstream::failbit | fstream::badbit);
    fl.open(file_name, mode);
    file = move(fl);
}

CommonSegy::CommonSegy(string&& name, ios_base::openmode mode)
    : file_name { move(name) }
    , hdr_buf { valarray<uint8_t>(CommonSegy::TR_HEADER_SIZE) }
{
    fstream fl;
    fl.exceptions(fstream::failbit | fstream::badbit);
    fl.open(file_name, mode);
    file = move(fl);
}

static uint8_t constexpr e2a[256] = {
    0x00, 0x01, 0x02, 0x03, 0x9C, 0x09, 0x86, 0x7F, 0x97, 0x8D, 0x8E, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x9D, 0x85, 0x08, 0x87, 0x18, 0x19, 0x92, 0x8F, 0x1C, 0x1D, 0x1E, 0x1F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x0A, 0x17, 0x1B, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x05, 0x06, 0x07,
    0x90, 0x91, 0x16, 0x93, 0x94, 0x95, 0x96, 0x04, 0x98, 0x99, 0x9A, 0x9B, 0x14, 0x15, 0x9E, 0x1A,
    0x20, 0xA0, 0xE2, 0xE4, 0xE0, 0xE1, 0xE3, 0xE5, 0xE7, 0xF1, 0xA2, 0x2E, 0x3C, 0x28, 0x2B, 0x7C,
    0x26, 0xE9, 0xEA, 0xEB, 0xE8, 0xED, 0xEE, 0xEF, 0xEC, 0xDF, 0x21, 0x24, 0x2A, 0x29, 0x3B, 0xAC,
    0x2D, 0x2F, 0xC2, 0xC4, 0xC0, 0xC1, 0xC3, 0xC5, 0xC7, 0xD1, 0xA6, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,
    0xF8, 0xC9, 0xCA, 0xCB, 0xC8, 0xCD, 0xCE, 0xCF, 0xCC, 0x60, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,
    0xD8, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0xAB, 0xBB, 0xF0, 0xFD, 0xFE, 0xB1,
    0xB0, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0xAA, 0xBA, 0xE6, 0xB8, 0xC6, 0xA4,
    0xB5, 0x7E, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0xA1, 0xBF, 0xD0, 0xDD, 0xDE, 0xAE,
    0x5E, 0xA3, 0xA5, 0xB7, 0xA9, 0xA7, 0xB6, 0xBC, 0xBD, 0xBE, 0x5B, 0x5D, 0xAF, 0xA8, 0xB4, 0xD7,
    0x7B, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0xAD, 0xF4, 0xF6, 0xF2, 0xF3, 0xF5,
    0x7D, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0xB9, 0xFB, 0xFC, 0xF9, 0xFA, 0xFF,
    0x5C, 0xF7, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0xB2, 0xD4, 0xD6, 0xD2, 0xD3, 0xD5,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0xB3, 0xDB, 0xDC, 0xD9, 0xDA, 0x9F
};

static uint8_t constexpr a2e[256] = {
    0x00, 0x01, 0x02, 0x03, 0x37, 0x2d, 0x2e, 0x2f, 0x16, 0x05, 0x25, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x3c, 0x3d, 0x32, 0x26, 0x18, 0x19, 0x3f, 0x27, 0x1c, 0x1d, 0x1e, 0x1f,
    0x40, 0x5a, 0x7f, 0x7b, 0x5b, 0x6c, 0x50, 0x7d, 0x4d, 0x5d, 0x5c, 0x4e, 0x6b, 0x60, 0x4b, 0x61,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0x7a, 0x5e, 0x4c, 0x7e, 0x6e, 0x6f,
    0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xba, 0xe0, 0xbb, 0xb0, 0x6d,
    0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xc0, 0x4f, 0xd0, 0xa1, 0x07,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x15, 0x06, 0x17, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x09, 0x0a, 0x1b,
    0x30, 0x31, 0x1a, 0x33, 0x34, 0x35, 0x36, 0x08, 0x38, 0x39, 0x3a, 0x3b, 0x04, 0x14, 0x3e, 0xff,
    0x41, 0xaa, 0x4a, 0xb1, 0x9f, 0xb2, 0x6a, 0xb5, 0xbd, 0xb4, 0x9a, 0x8a, 0x5f, 0xca, 0xaf, 0xbc,
    0x90, 0x8f, 0xea, 0xfa, 0xbe, 0xa0, 0xb6, 0xb3, 0x9d, 0xda, 0x9b, 0x8b, 0xb7, 0xb8, 0xb9, 0xab,
    0x64, 0x65, 0x62, 0x66, 0x63, 0x67, 0x9e, 0x68, 0x74, 0x71, 0x72, 0x73, 0x78, 0x75, 0x76, 0x77,
    0xac, 0x69, 0xed, 0xee, 0xeb, 0xef, 0xec, 0xbf, 0x80, 0xfd, 0xfe, 0xfb, 0xfc, 0xad, 0xae, 0x59,
    0x44, 0x45, 0x42, 0x46, 0x43, 0x47, 0x9c, 0x48, 0x54, 0x51, 0x52, 0x53, 0x58, 0x55, 0x56, 0x57,
    0x8c, 0x49, 0xcd, 0xce, 0xcb, 0xcf, 0xcc, 0xe1, 0x70, 0xdd, 0xde, 0xdb, 0xdc, 0x8d, 0x8e, 0xdf
};

void CommonSegy::ebcdic_to_ascii(string ebcdic)
{
    for (auto from = ebcdic.begin(), end = ebcdic.end(); from != end; ++from)
        *from = e2a[static_cast<uint8_t>(*from)];
}

void CommonSegy::ascii_to_ebcdic(string ascii)
{
    for (auto from = ascii.begin(), end = ascii.end(); from != end; ++from)
        *from = a2e[static_cast<uint8_t>(*from)];
}

static valarray<string> const bin_names = {
    "Job identification number",
    "Line number",
    "Reel number",
    "Number of data traces per ensemble",
    "Number of auxiliary traces per ensemble",
    "Sample interval",
    "Sample interval of original field recording.",
    "Number of samples per data trace",
    "Number of samples per data trace of original field recording",
    "Data sample format code",
    "Ensemble fold",
    "Trace sorting code",
    "Vertical sum code",
    "Sweep frequency at start (Hz)",
    "Sweep frequency at end (Hz)",
    "Sweep length (ms)",
    "Sweep type code",
    "Trace number of sweep channel",
    "Sweep trace taper length in ms at start if tapered",
    "Sweep trace taper length in ms at end",
    "Taper type",
    "Correlated data traces",
    "Binary gain recovered",
    "Amplitude recovery method",
    "Measure system",
    "Impulse signal polarity",
    "Vibratory polarity code",
    "Extended number of data traces per ensemble",
    "Extended number of auxiliary traces per ensemble",
    "Extended number of samples per data trace",
    "Extended sample interval",
    "Extended sample interval of original field recording",
    "Extended number of samples per data trace in original recording",
    "Extended ensemble fold",
    "Integer constant used to detect the byte ordering",
    "Major SEG-Y Format Revision Number",
    "Minor SEG-Y Format Revision Number",
    "Fixed length trace flag",
    "Number of Extended Textual File Header records",
    "Maximum number of additional trace headers",
    "Time basis code",
    "Number of traces in this file",
    "Byte offset of first traces relative to start of file",
    "Number of data trailer stanza records"
};

string const& CommonSegy::BinaryHeader::name_as_string(Name n)
{
    return bin_names[static_cast<decltype(bin_names.size())>(n)];
}

char const* CommonSegy::default_text_header = "C 1 CLIENT                        COMPANY                       CREW NO         "
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
} // namespace sedaman
