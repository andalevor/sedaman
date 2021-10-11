#include "CommonSEGY.hpp"
#include "Exception.hpp"
#include "Trace.hpp"

using std::fstream;
using std::ios_base;
using std::map;
using std::move;
using std::pair;
using std::string;
using std::to_string;
using std::vector;

namespace sedaman {

static void check_add_tr_hdr_map(CommonSEGY* com);

CommonSEGY::CommonSEGY(string name, ios_base::openmode mode, BinaryHeader bh,
    vector<pair<string, map<uint32_t, pair<string, Trace::Header::ValueType>>>>
   	add_hdr_map)
    : binary_header { move(bh) }
    , file_name { move(name) }
    , hdr_buf {}
    , add_tr_hdr_map { move(add_hdr_map) }
{
    fstream fl;
    fl.exceptions(fstream::failbit | fstream::badbit);
    fl.open(file_name, mode);
    file = move(fl);
    check_add_tr_hdr_map(this);
}

void check_add_tr_hdr_map(CommonSEGY* com)
{
    int first = 1, size = 0;
    uint32_t prev = 0;
    if (com->binary_header.max_num_add_tr_headers &&
	   	com->add_tr_hdr_map.size() != 
           static_cast<decltype(com->add_tr_hdr_map.size())>
           						(com->binary_header.max_num_add_tr_headers))
        throw Exception(__FILE__, __LINE__,
            "number of additional trace headers not equal to max number of "
		   	"headers in binary header");
    for (auto& p : com->add_tr_hdr_map) {
        if (p.first.size() != 8)
            throw Exception(__FILE__, __LINE__,
						   	"additional trace header name must have "
						   	"8 bytes length");
        for (auto& i : p.second) {
            if (first) {
                prev = i.first;
                first = 0;
            } else {
                switch (i.second.second) {
                case Trace::Header::ValueType::int8_t:
                    size = 1;
                    break;
                case Trace::Header::ValueType::uint8_t:
                    size = 1;
                    break;
                case Trace::Header::ValueType::int16_t:
                    size = 2;
                    break;
                case Trace::Header::ValueType::uint16_t:
                    size = 2;
                    break;
                case Trace::Header::ValueType::int24_t:
                    size = 3;
                    break;
                case Trace::Header::ValueType::uint24_t:
                    size = 3;
                    break;
                case Trace::Header::ValueType::int32_t:
                    size = 4;
                    break;
                case Trace::Header::ValueType::uint32_t:
                    size = 4;
                    break;
                case Trace::Header::ValueType::int64_t:
                    size = 8;
                    break;
                case Trace::Header::ValueType::uint64_t:
                    size = 8;
                    break;
                case Trace::Header::ValueType::ibm:
                    size = 4;
                    break;
                case Trace::Header::ValueType::ieee_single:
                    size = 4;
                    break;
                case Trace::Header::ValueType::ieee_double:
                    size = 8;
                    break;
                default:
                    throw Exception(__FILE__, __LINE__,
									"impossible, unexpected type in "
									"TrHdrValueType");
                }
                if (i.first - prev < static_cast<uint32_t>(size))
                    throw Exception(__FILE__, __LINE__,
									string("overlapping type/offset in "
                                    "additional trace headers map: ") +
								   	to_string(i.first));
                if (i.first + size > 232)
                    throw Exception(__FILE__, __LINE__,
                        string("last 8 bytes should be used for header name"));
                prev = i.first;
            }
        }
    }
}

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

static uint8_t constexpr a2e[256] = {
0x00,0x01,0x02,0x03,0x37,0x2d,0x2e,0x2f,0x16,0x05,0x25,0x0b,0x0c,0x0d,0x0e,0x0f,
0x10,0x11,0x12,0x13,0x3c,0x3d,0x32,0x26,0x18,0x19,0x3f,0x27,0x1c,0x1d,0x1e,0x1f,
0x40,0x5a,0x7f,0x7b,0x5b,0x6c,0x50,0x7d,0x4d,0x5d,0x5c,0x4e,0x6b,0x60,0x4b,0x61,
0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0x7a,0x5e,0x4c,0x7e,0x6e,0x6f,
0x7c,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,
0xd7,0xd8,0xd9,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xba,0xe0,0xbb,0xb0,0x6d,
0x79,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x91,0x92,0x93,0x94,0x95,0x96,
0x97,0x98,0x99,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xc0,0x4f,0xd0,0xa1,0x07,
0x20,0x21,0x22,0x23,0x24,0x15,0x06,0x17,0x28,0x29,0x2a,0x2b,0x2c,0x09,0x0a,0x1b,
0x30,0x31,0x1a,0x33,0x34,0x35,0x36,0x08,0x38,0x39,0x3a,0x3b,0x04,0x14,0x3e,0xff,
0x41,0xaa,0x4a,0xb1,0x9f,0xb2,0x6a,0xb5,0xbd,0xb4,0x9a,0x8a,0x5f,0xca,0xaf,0xbc,
0x90,0x8f,0xea,0xfa,0xbe,0xa0,0xb6,0xb3,0x9d,0xda,0x9b,0x8b,0xb7,0xb8,0xb9,0xab,
0x64,0x65,0x62,0x66,0x63,0x67,0x9e,0x68,0x74,0x71,0x72,0x73,0x78,0x75,0x76,0x77,
0xac,0x69,0xed,0xee,0xeb,0xef,0xec,0xbf,0x80,0xfd,0xfe,0xfb,0xfc,0xad,0xae,0x59,
0x44,0x45,0x42,0x46,0x43,0x47,0x9c,0x48,0x54,0x51,0x52,0x53,0x58,0x55,0x56,0x57,
0x8c,0x49,0xcd,0xce,0xcb,0xcf,0xcc,0xe1,0x70,0xdd,0xde,0xdb,0xdc,0x8d,0x8e,0xdf
};

void CommonSEGY::ebcdic_to_ascii(string& ebcdic)
{
    for (auto from = ebcdic.begin(), end = ebcdic.end(); from != end; ++from)
        *from = e2a[static_cast<uint8_t>(*from)];
}

void CommonSEGY::ascii_to_ebcdic(string& ascii)
{
    for (auto from = ascii.begin(), end = ascii.end(); from != end; ++from)
        *from = a2e[static_cast<uint8_t>(*from)];
}

static char const* bin_names[] = {
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

char const* CommonSEGY::BinaryHeader::name_as_string(Name n)
{
    return bin_names[static_cast<int>(n)];
}

map<string, string> CommonSEGY::trace_header_description = {
    { "TRC_SEQ_LINE", "Trace sequence number within line." },
    { "TRC_SEQ_SGY", "Trace sequence number within SEGY file." },
    { "FFID", "Original field record number." },
    { "CHAN", "Trace number within the original field record." },
	{ "ESP", "Energy source point number. Used when more than one record "
		"occures at the same effective surface location." },
    { "ENS_NO", "Ensemble number (i.e. CDP, CMP, CRP, etc.)." },
    { "SEQ_NO", "Trace number within the ensemble." },
    { "TRACE_ID", "Trace identification code." },
    { "VERT_SUM", "Number of vertically summed traces yielding this trace." },
	{ "HOR_SUM", "Number of horizontally stacked traces yielding "
		"this trace." },
    { "DATA_USE", "Data use: 1 = Production, 2 = Test" },
	{ "OFFSET", "Distance from center of the source point to the "
		"center of the receiver group." },
    { "R_ELEV", "Elevation of receiver group." },
    { "S_ELEV", "Surface elevation at source location." },
    { "S_DEPTH", "Source depth below surface." },
    { "R_DATUM", "Seismic Datum elevation at receiver group." },
    { "S_DATUM", "Seismic Datum elevation at source." },
    { "S_WATER", "Water column height at source location." },
    { "R_WATER", "Water column height at reseiver group location." },
    { "ELEV_SCALAR", "Scalar to be aplied to all elevations and depths." },
    { "COORD_SCALAR", "Scalar to be applied to all coordinates." },
    { "SOU_X", "Source coordinate X." },
    { "SOU_Y", "Source coordinate Y." },
    { "REC_X", "Group coordinate X." },
    { "REC_Y", "Group coordinate Y." },
	{ "COORD_UNITS", "Coordinate units. (1,3,4): length; "
		"decimal degrees; DMS." },
    { "WEATH_VEL", "Weathering velocity." },
    { "SUBWEATH_VEL", "Subweathering velocity." },
    { "S_UPHOLE", "Uphole time at source in milliseconds." },
    { "R_UPHOLE", "Uphole time at group in milliseconds." },
    { "S_STAT", "Source static correction in milliseconds." },
    { "R_STAT", "Receiver static correction in milliseconds." },
    { "TOT_STAT", "Total static applied in milliseconds." },
	{ "LAG_A", "Lag time A. Time in ms between end of trace header and "
		"time break." },
	{ "LAG_B", "Lag time B. Time in ms between time break and "
		"initiation of source." },
	{ "DELAY_TIME", "Delay recording time. Time in ms between initiation of "
		"source andstart of recording" },
    { "MUTE_START", "Mute time. Start time in milliseconds." },
    { "MUTE_END", "Mute time. End time in milliseconds." },
    { "SAMP_NUM", "Number of samples in this trace." },
    { "SAMP_INT", "Sample interval for this trace." },
	{ "GAIN_TYPE", "Gain type of field instruments: "
		"1 = fixed, 2 = binary, 3 = floating point." },
    { "GAIN_CONST", "Instrument gain constant (dB)." },
    { "INIT_GAIN", "Instrument early or initial gain (dB)." },
    { "CORRELATED", "Correlated: 1 = no, 2 = yes." },
    { "SW_START", "Sweep frequency at start (Hz)." },
    { "SW_END", "Sweep frequency at end (Hz)." },
    { "SW_LENGTH", "Sweep length in milliseconds." },
    { "SW_TYPE", "Sweep type: 1 = linear, 2 = parabolic, 3 = exponential." },
    { "SW_TAPER_START", "Sweep trace taper length at start in milliseconds." },
    { "SW_TAPER_END", "Sweep trace taper length at end in milliseconds." },
    { "TAPER_TYPE", "Taper type: 1 = linear, 2 = cos^2, 3 = other." },
    { "ALIAS_FILT_FREQ", "Alias filter frequency (Hz), if used." },
    { "ALIAS_FILT_SLOPE", "Alias filter slope (dB/octave)." },
    { "NOTCH_FILT_FREQ", "Notch filter frequency (Hz), if used." },
    { "NOTCH_FILT_SLOPE", "Notch filter slope (dB/octave)." },
    { "LOW_CUT_FREQ", "Low-cut frequency (Hz), if used." },
    { "HIGH_CUT_FREQ", "High-cut frequency (Hz), if used." },
    { "LOW_CUT_SLOPE", "Low-cut slope (dB/octave)." },
    { "HIGH_CUT_SLOPE", "High-cut slope (dB/octave)." },
    { "YEAR", "Year data recorded." },
    { "DAY", "Day of year." },
    { "HOUR", "Year data recorded." },
    { "MINUTE", "Minute of hour." },
    { "SECOND", "Second of minute." },
	{ "TIME_BASIS_CODE", "Time basis code. 1 = local, 2 = GMT, 4 = UTC, "
		"5 = GPS" },
    { "TRACE_WEIGHT", "Trace weighting factor. Defined as 2^-N." },
    { "GROUP_NUM_ROLL", "Geophone group number of roll switch position one." },
	{ "GROUP_NUM_FIRST", "Geophone group number of trace number one "
		"within original field record." },
	{ "GROUP_NUM_LAST", "Geophone group number of last trace within original "
		"field record." },
    { "GAP_SIZE", "Gap size (total number of groups dropped)." },
	{ "OVER_TRAVEL", "Over travel associated with taper at beginning or "
		"end of line." },
    { "CDP_X", "X coordinate of ensemble (CDP) position." },
    { "CDP_Y", "Y coordinate of ensemble (CDP) position." },
    { "INLINE", "In-line number (for 3D)." },
    { "XLINE", "Cross-line number (for 3D)." },
	{ "SP_NUM", "Shotpoint number. Shotpoint number refers to the source "
		"location nearest to CDP location" },
    { "SP_NUM_SCALAR", "Scalar to be applied to the SP_NUM." },
	{ "TR_VAL_UNIT", "Trace value measurement unit. 1 = Pa, 2 = V, 3 = mV, "
		"4 = A, 5 = m, 6 = m/s, 7 = m/s/s, 8 = N, 9 = W" },
	{ "TRANS_CONST", "Transduction Constant. Transforms trace samples to the "
		"Transduction Units" },
	{ "TRANS_UNITS", "Transduction Units. 1 = Pa, 2 = V, 3 = mV, 4 = A, "
		"5 = m, 6 = m/s, 7 = m/s/s, 8 = N, 9 = W" },
	{ "DEVICE_ID", "Device/Trace Identifier. The unit number or id of the "
		"device associated with trace data." },
	{ "TIME_SCALAR", "Scalar to be applied to upholes, statics, lags, "
		"delay and mutes." },
	{ "SOURCE_TYPE", "Defines the type and the orientation of "
		"energy source." },
	{ "SOU_V_DIR", "Source Energy Direction with respect to "
		"source orientation. Vertical." },
	{ "SOU_X_DIR", "Source Energy Direction with respect to "
		"source orientation. Cross-line." },
	{ "SOU_I_DIR", "Source Energy Direction with respect to "
		"source orientation. In-line." },
	{ "SOURCE_MEASUREMENT", "Source Measurement. Describes the "
		"source effort used to generate the trace." },
	{ "SOU_MEAS_UNIT", "The unit used for the Source Measurement. "
		"1 = J, 2 = kW, 3 = Pa, 4 = Bar, 5 = N, 6 = kg." },
	{ "R_DEPTH", "Receiver group depth below the surface locacation of "
		"receiver group." },
    { "NANO_SECOND", "Nanoseconds to add to Second of minute." },
	{ "CABLE_NUM", "Cable number for multi-cable acquisition or "
		"Recording Device/Sensor ID number." },
	{ "ADD_TRC_HDR_NUM", "Number of additional trace header extension "
		"blocks including this one." },
	{ "LAST_TRC_FLAG", "Last trace flag. 1 = in ensemble, 2 = in line, "
		"4 = in file, 8 = in survey." },
};

char const* CommonSEGY::default_text_header =
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
} // namespace sedaman
