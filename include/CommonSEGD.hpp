///
/// \file CommonSEGD.hpp
/// \brief header file with CommonSEGD class declaration
///
/// \author andalevor
///
/// \date   2020/05/04
///

#ifndef SEDAMAN_COMMON_SEGD_HPP
#define SEDAMAN_COMMON_SEGD_HPP

#include <array>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

///
/// \namespace sedaman
/// \brief General namespace for sedaman library.
///
namespace sedaman {
///
/// \class CommonSEGD
/// \brief Class with common SEGD parts.
/// Holds common data and members for ISEGD and OSEGD classes.
/// \see ISEGD
/// \see OSEGD
///
class CommonSEGD {
public:
    ///
    /// \class GeneralHeader
    /// \brief Class for first general header
    ///
    class GeneralHeader {
    public:
        int file_number;
        int format_code;
        long long gen_const;
        int year;
        int add_gen_hdr_blocks;
        int day;
        int hour;
        int minute;
        int second;
        int manufac_code;
        int manufac_num;
        long bytes_per_scan;
        double base_scan_int;
        int polarity;
        long scans_per_block;
        int record_type;
        int record_length;
        int scan_types_per_record;
        int channel_sets_per_scan_type;
        int skew_blocks;
        int extended_hdr_blocks;
        int external_hdr_blocks;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            FILE_NUMBER,
            FORMAT_CODE,
            GENERAL_CONSTANTS,
            YEAR,
            GENERAL_HEADER_BLOCKS,
            DAY,
            HOUR,
            MINUTE,
            SECOND,
            MANUFACTURERS_CODE,
            MANUFACTURERS_NUMBER,
            BYTES_PER_SCAN,
            BASE_SCAN_INTERVAL,
            POLARITY,
            SCANS_PER_BLOCK,
            RECORD_TYPE,
            RECORD_LENGTH,
            SCAN_TYPES_PER_RECORD,
            CHANNEL_SETS_PER_SCAN_TYPE,
            SKEW_BLOCKS,
            EXTENDED_HEADER_BLOCKS,
            EXTERNAL_HEADER_BLOCKS
        };
        static char const* name_as_string(Name n);
    } general_header;
    ///
    /// \class GeneralHeader
    /// \brief Class for second general header
    /// Mandatory from SEGD rev 1
    ///
    class GeneralHeader2 {
    public:
        uint32_t expanded_file_num;
        uint16_t ext_ch_sets_per_scan_type;
        uint32_t extended_hdr_blocks;
        uint32_t external_hdr_blocks;
        uint16_t extended_skew_blocks;
        uint8_t segd_rev_major;
        uint8_t segd_rev_minor;
        uint32_t gen_trailer_num_of_blocks;
        uint32_t ext_record_len;
        uint16_t record_set_number;
        uint16_t ext_num_add_blks_in_gen_hdr;
        uint32_t dominant_sampling_int;
        uint8_t gen_hdr_block_num;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            EXPANDED_FILE_NUMBER,
            EXT_CH_SETS_PER_SCAN_TYPE,
            EXTEDNDED_HEADER_BLOCKS,
            EXTERNAL_HEADER_BLOCKS,
            EXTENDED_SKEW_BLOCKS,
            SEGD_REVISION_MAJOR,
            SEGD_REVISION_MINOR,
            GEN_TRAILER_NUMBER_OF_BLOCKS,
            EXTENDED_RECORD_LENGTH,
            RECORD_SET_NUMBER,
            EXT_NUM_ADD_BLKS_IN_GEN_HDR,
            DOMINANT_SAMPLING_INT,
            GEN_HEADER_BLOCK_NUM
        };
        static char const* name_as_string(Name n);
    } general_header2;
    ///
    /// \class GeneralHeader
    /// \brief Class for N-th general header
    /// Optional from SEGD rev 1 till 2
    ///
    class GeneralHeaderN {
    public:
        uint32_t expanded_file_number;
        double sou_line_num;
        double sou_point_num;
        uint8_t sou_point_index;
        uint8_t phase_control;
        uint8_t type_vibrator;
        int16_t phase_angle;
        uint8_t gen_hdr_block_num;
        uint8_t sou_set_num;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            EXPANDED_FILE_NUMBER,
            SOURCE_LINE_NUMBER,
            SOURCE_POINT_NUMBER,
            SOURCE_POINT_INDEX,
            PHASE_CONTROL,
            TYPE_VIBRATOR,
            PHASE_ANGLE,
            GEN_HEADER_BLOCK_NUM,
            SOURCE_SET_NUMBER
        };
        static char const* name_as_string(Name n);
    } general_headerN;
    ///
    /// \class GeneralHeader
    /// \brief Class for second general header
    /// Mandatory from SEGD rev 3
    ///
    class GeneralHeader3 {
    public:
        uint64_t time_zero;
        uint64_t record_size;
        uint64_t data_size;
        uint32_t header_size;
        uint8_t extd_rec_mode;
        uint8_t rel_time_mode;
        uint8_t gen_hdr_block_num;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            TIME_ZERO,
            RECORD_SIZE,
            DATA_SIZE,
            HEADER_SIZE,
            EXTD_REC_MODE,
            REL_TIME_MODE,
            GEN_HEADER_BLOCK_NUM
        };
        static char const* name_as_string(Name n);
    } general_header3;
    ///
    /// \class GeneralHeaderVes
    /// \brief Class for general header vessel\crew information
    ///
    class GeneralHeaderVes {
    public:
        char abbr_vessel_crew_name[3];
        char vessel_crew_name[28];
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            ABBR_VESSEL_OR_CREW_NAME,
            VESSEL_OR_CREW_NAME,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_ves;
    ///
    /// \class GeneralHeaderSur
    /// \brief Class for general header for Survea Area Name
    ///
    class GeneralHeaderSur {
    public:
        char survey_area_name[31];
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            SURVEY_ARE_NAME,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_sur;
    ///
    /// \class GeneralHeaderCli
    /// \brief Class for general header for Client Name
    ///
    class GeneralHeaderCli {
    public:
        char client_name[31];
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            CLIENT_NAME,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_cli;
    ///
    /// \class GeneralHeaderJob
    /// \brief Class for general header for Job ID
    ///
    class GeneralHeaderJob {
    public:
        char abbr_job_id[5];
        char job_id[26];
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            ABBR_JOB_ID,
            JOB_ID,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_job;
    ///
    /// \class GeneralHeaderLin
    /// \brief Class for general header for Line ID
    ///
    class GeneralHeaderLin {
    public:
        char line_abbr[7];
        char line_id[24];
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            LINE_ABBR,
            LINE_ID,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_lin;
    ///
    /// \class GeneralHeaderVib
    /// \brief Class for general header for Vibrator source information
    ///
    class GeneralHeaderVib {
    public:
        uint32_t expanded_file_number;
        double sou_line_num;
        double sou_point_num;
        uint8_t sou_point_index;
        uint8_t phase_control;
        uint8_t type_vibrator;
        int16_t phase_angle;
        uint8_t source_id;
        uint8_t source_set_num;
        uint8_t reshoot_idx;
        uint8_t group_idx;
        uint8_t depth_idx;
        uint16_t offset_crossline;
        uint16_t offset_inline;
        uint16_t size;
        uint16_t offset_depth;
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            EXPANDED_FILE_NUMBER,
            SOURCE_LINE_NUMBER,
            SOURCE_POINT_NUMBER,
            SOURCE_POINT_INDEX,
            PHASE_CONTROL,
            TYPE_VIBRATOR,
            PHASE_ANGLE,
            SOURCE_ID,
            SOURCE_SET_NUM,
            RESHOOT_INDEX,
            GROUP_INDEX,
            DEPTH_INDEX,
            OFFSET_CROSSLINE,
            OFFSET_INLINE,
            SIZE,
            OFFSET_DEPTH,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_vib;
    ///
    /// \class GeneralHeaderExp
    /// \brief Class for general header for Explosive source information
    ///
    class GeneralHeaderExp {
    public:
        uint32_t expanded_file_number;
        double sou_line_num;
        double sou_point_num;
        uint8_t sou_point_index;
        uint16_t depth;
        uint8_t charge_length;
        uint8_t soil_type;
        uint8_t source_id;
        uint8_t source_set_num;
        uint8_t reshoot_idx;
        uint8_t group_idx;
        uint8_t depth_idx;
        uint16_t offset_crossline;
        uint16_t offset_inline;
        uint16_t size;
        uint16_t offset_depth;
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            EXPANDED_FILE_NUMBER,
            SOURCE_LINE_NUMBER,
            SOURCE_POINT_NUMBER,
            SOURCE_POINT_INDEX,
            DEPTH,
            CHARGE_LENGTH,
            SOIL_TYPE,
            SOURCE_ID,
            SOURCE_SET_NUM,
            RESHOOT_INDEX,
            GROUP_INDEX,
            DEPTH_INDEX,
            OFFSET_CROSSLINE,
            OFFSET_INLINE,
            SIZE,
            OFFSET_DEPTH,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_exp;
    ///
    /// \class GeneralHeaderAir
    /// \brief Class for general header for Airgun source information
    ///
    class GeneralHeaderAir {
    public:
        uint32_t expanded_file_number;
        double sou_line_num;
        double sou_point_num;
        uint8_t sou_point_index;
        uint16_t depth;
        uint16_t air_pressure;
        uint8_t source_id;
        uint8_t source_set_num;
        uint8_t reshoot_idx;
        uint8_t group_idx;
        uint8_t depth_idx;
        uint16_t offset_crossline;
        uint16_t offset_inline;
        uint16_t size;
        uint16_t offset_depth;
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            EXPANDED_FILE_NUMBER,
            SOURCE_LINE_NUMBER,
            SOURCE_POINT_NUMBER,
            SOURCE_POINT_INDEX,
            DEPTH,
            AIR_PRESSURE,
            SOURCE_ID,
            SOURCE_SET_NUM,
            RESHOOT_INDEX,
            GROUP_INDEX,
            DEPTH_INDEX,
            OFFSET_CROSSLINE,
            OFFSET_INLINE,
            SIZE,
            OFFSET_DEPTH,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_air;
    ///
    /// \class GeneralHeaderWat
    /// \brief Class for general header for Watergun source information
    ///
    class GeneralHeaderWat {
    public:
        uint32_t expanded_file_number;
        double sou_line_num;
        double sou_point_num;
        uint8_t sou_point_index;
        uint16_t depth;
        uint16_t air_pressure;
        uint8_t source_id;
        uint8_t source_set_num;
        uint8_t reshoot_idx;
        uint8_t group_idx;
        uint8_t depth_idx;
        uint16_t offset_crossline;
        uint16_t offset_inline;
        uint16_t size;
        uint16_t offset_depth;
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            EXPANDED_FILE_NUMBER,
            SOURCE_LINE_NUMBER,
            SOURCE_POINT_NUMBER,
            SOURCE_POINT_INDEX,
            DEPTH,
            AIR_PRESSURE,
            SOURCE_ID,
            SOURCE_SET_NUM,
            RESHOOT_INDEX,
            GROUP_INDEX,
            DEPTH_INDEX,
            OFFSET_CROSSLINE,
            OFFSET_INLINE,
            SIZE,
            OFFSET_DEPTH,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_wat;
    ///
    /// \class GeneralHeaderEle
    /// \brief Class for general header for Electromagnetic source information
    ///
    class GeneralHeaderEle {
    public:
        uint32_t expanded_file_number;
        double sou_line_num;
        double sou_point_num;
        uint8_t sou_point_index;
        uint8_t source_type;
        uint32_t moment;
        uint8_t source_id;
        uint8_t source_set_num;
        uint8_t reshoot_idx;
        uint8_t group_idx;
        uint8_t depth_idx;
        uint16_t offset_crossline;
        uint16_t offset_inline;
        uint16_t size;
        uint16_t offset_depth;
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            EXPANDED_FILE_NUMBER,
            SOURCE_LINE_NUMBER,
            SOURCE_POINT_NUMBER,
            SOURCE_POINT_INDEX,
            SOURCE_TYPE,
            MOMENT,
            SOURCE_ID,
            SOURCE_SET_NUM,
            RESHOOT_INDEX,
            GROUP_INDEX,
            DEPTH_INDEX,
            OFFSET_CROSSLINE,
            OFFSET_INLINE,
            SIZE,
            OFFSET_DEPTH,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_ele;
    ///
    /// \class GeneralHeaderOth
    /// \brief Class for general header for Other source information
    ///
    class GeneralHeaderOth {
    public:
        uint32_t expanded_file_number;
        double sou_line_num;
        double sou_point_num;
        uint8_t sou_point_index;
        uint8_t source_id;
        uint8_t source_set_num;
        uint8_t reshoot_idx;
        uint8_t group_idx;
        uint8_t depth_idx;
        uint16_t offset_crossline;
        uint16_t offset_inline;
        uint16_t size;
        uint16_t offset_depth;
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            EXPANDED_FILE_NUMBER,
            SOURCE_LINE_NUMBER,
            SOURCE_POINT_NUMBER,
            SOURCE_POINT_INDEX,
            SOURCE_ID,
            SOURCE_SET_NUM,
            RESHOOT_INDEX,
            GROUP_INDEX,
            DEPTH_INDEX,
            OFFSET_CROSSLINE,
            OFFSET_INLINE,
            SIZE,
            OFFSET_DEPTH,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_oth;
    ///
    /// \class GeneralHeaderAdd
    /// \brief Class for general header for Additional source information
    ///
    class GeneralHeaderAdd {
    public:
        uint64_t time;
        uint8_t source_status;
        uint8_t source_id;
        uint8_t source_moving;
        char error_description[20];
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            TIME,
            SOURCCE_STATUS,
            SOURCE_ID,
            SOURCE_MOVING,
            ERROR_DESCRIPTION,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_add;
    ///
    /// \class GeneralHeaderSaux
    /// \brief Class for general header for Source Auxiliary information
    ///
    class GeneralHeaderSaux {
    public:
        uint8_t source_id;
        uint8_t scan_type_num_1;
        uint16_t ch_set_num_1;
        uint32_t trace_num_1;
        uint8_t scan_type_num_2;
        uint16_t ch_set_num_2;
        uint32_t trace_num_2;
        uint8_t scan_type_num_3;
        uint16_t ch_set_num_3;
        uint32_t trace_num_3;
        uint8_t scan_type_num_4;
        uint16_t ch_set_num_4;
        uint32_t trace_num_4;
        uint8_t scan_type_num_5;
        uint16_t ch_set_num_5;
        uint32_t trace_num_5;
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            SOURCE_ID,
            SCAN_TYPE_NUM_1,
            CH_SET_NUM_1,
            TRACE_NUM_1,
            SCAN_TYPE_NUM_2,
            CH_SET_NUM_2,
            TRACE_NUM_2,
            SCAN_TYPE_NUM_3,
            CH_SET_NUM_3,
            TRACE_NUM_3,
            SCAN_TYPE_NUM_4,
            CH_SET_NUM_4,
            TRACE_NUM_4,
            SCAN_TYPE_NUM_5,
            CH_SET_NUM_5,
            TRACE_NUM_5,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_saux;
    ///
    /// \class GeneralHeaderCoord
    /// \brief Class for general header for coordinate reference system identification
    ///
    class GeneralHeaderCoord {
    public:
        char crs[31];
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            COORD_REF_SYS,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_coord;
    ///
    /// \class GeneralHeaderPos1
    /// \brief Class for general header for position blocks 1 identification
    ///
    class GeneralHeaderPos1 {
    public:
        uint64_t time_of_position;
        uint64_t time_of_measurement;
        uint32_t vert_error;
        uint32_t hor_error_semi_major;
        uint32_t hor_error_semi_minor;
        uint16_t hor_error_orientation;
        uint8_t position_type;
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            TIME_OF_POSITION,
            TIME_OF_MEASUREMENT,
            VERTICAL_ERROR,
            HOR_ERR_SEMI_MAJOR,
            HOR_ERR_SEMI_MINOR,
            HOR_ERR_ORIENTATION,
            POSITION_TYPE,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_pos1;
    ///
    /// \class GeneralHeaderPos2
    /// \brief Class for general header for position blocks 2 identification
    ///
    class GeneralHeaderPos2 {
    public:
        uint64_t crs_a_coord1;
        uint64_t crs_a_coord2;
        uint64_t crs_a_coord3;
        uint16_t crs_a_crsref;
        uint8_t pos1_valid;
        uint8_t pos1_quality;
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            CRS_A_COORD1,
            CRS_A_COORD2,
            CRS_A_COORD3,
            CRS_A_CRSREF,
            POS1_VALID,
            POS1_QUALITY,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_pos2;
    ///
    /// \class GeneralHeaderPos3
    /// \brief Class for general header for position blocks 3 identification
    ///
    class GeneralHeaderPos3 {
    public:
        uint64_t crs_b_coord1;
        uint64_t crs_b_coord2;
        uint64_t crs_b_coord3;
        uint16_t crs_b_crsref;
        uint8_t pos2_valid;
        uint8_t pos2_quality;
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            CRS_B_COORD1,
            CRS_B_COORD2,
            CRS_B_COORD3,
            CRS_B_CRSREF,
            POS2_VALID,
            POS2_QUALITY,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_pos3;
    ///
    /// \class GeneralHeaderRel
    /// \brief Class for general header for relative position identification
    ///
    class GeneralHeaderRel {
    public:
        uint32_t offset_east;
        uint32_t offset_north;
        uint32_t offset_vert;
        char description[19];
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            OFFSET_EAST,
            OFFSET_NORTH,
            OFFSET_VERT,
            DESCRIPTION,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_rel;
    ///
    /// \class GeneralHeaderSen
    /// \brief Class for general header for sensor info
    ///
    class GeneralHeaderSen {
    public:
        uint64_t instrument_test_time;
        uint32_t sensor_sensitivity;
        uint8_t instr_test_result;
        char serial_number[28];
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            INSTRUMENT_TEST_TIME,
            SENSOR_SENSITIVITY,
            INSTR_TEST_RESULT,
            SERIAL_NUMBER,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_sen;
    ///
    /// \class GeneralHeaderSCa
    /// \brief Class for general header for sensor calibration
    ///
    class GeneralHeaderSCa {
    public:
        uint32_t freq1;
        uint32_t amp1;
        uint32_t phase1;
        uint32_t freq2;
        uint32_t amp2;
        uint32_t phase2;
        uint8_t calib_applied;
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            FREQUENCY_1,
            AMPLITUDE_1,
            PHASE_1,
            FREQUENCY_2,
            AMPLITUDE_2,
            PHASE_2,
            ALIBRATION_APPLIED,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_sca;
    ///
    /// \class GeneralHeaderTim
    /// \brief Class for general header for time drift
    ///
    class GeneralHeaderTim {
    public:
        uint64_t time_of_depl;
        uint64_t time_of_retr;
        uint32_t timer_offset_depl;
        uint32_t time_offset_retr;
        uint8_t timedrift_corr;
        uint8_t corr_method;
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            TIME_OF_DEPLOYMENT,
            TIME_OF_RETRIEVAL,
            TIMER_OFFSET_DEPLOYMENT,
            TIME_OFFSET_RETRIEVAL,
            TIMEDRIFT_CORRECTED,
            CORRECTION_METHOD,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_tim;
    ///
    /// \class GeneralHeaderElSR
    /// \brief Class for general header for electomagnetic src/recv desc block
    ///
    class GeneralHeaderElSR {
    public:
        uint32_t equip_dim_x;
        uint32_t equip_dim_y;
        uint32_t equip_dim_z;
        uint8_t pos_term;
        uint32_t equip_offset_x;
        uint32_t equip_offset_y;
        uint32_t equip_offset_z;
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            EQUIPMENT_DIMENTION_X,
            EQUIPMENT_DIMENTION_Y,
            EQUIPMENT_DIMENTION_Z,
            POSITIVE_TERMINAL,
            EQUIPMENT_OFFSET_X,
            EQUIPMENT_OFFSET_Y,
            EQUIPMENT_OFFSET_Z,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_elsr;
    ///
    /// \class GeneralHeaderOri
    /// \brief Class for general header for orientation block
    ///
    class GeneralHeaderOri {
    public:
        uint32_t rot_x;
        uint32_t rot_y;
        uint32_t rot_z;
        uint32_t ref_orientation;
        uint64_t time_stamp;
        uint8_t ori_type;
        uint8_t ref_orient_valid;
        uint8_t rot_applied;
        uint8_t rot_north_applied;
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            ROTATION_X_AXIS,
            ROTATION_Y_AXIS,
            ROTATION_Z_AXIS,
            REFERENCE_ORIENT,
            TIME_STAMP,
            ORIENT_TYPE,
            REF_ORIENT_VALID,
            ROTATION_APPLIED,
            ROT_NORTH,
            APPLIED,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_ori;
    ///
    /// \class GeneralHeaderMeas
    /// \brief Class for general header for measurement block
    ///
    class GeneralHeaderMeas {
    public:
        uint64_t timestamp;
        uint32_t measurement_value;
        uint32_t maximum_value;
        uint32_t minimum_value;
        uint16_t quantity_class;
        uint16_t unit_of_measure;
        uint16_t measurement_description;
        uint8_t gen_hdr_block_type;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            TIMESTAMP,
            MEASUREMENT_VALUE,
            MAXIMUM_VALUE,
            MINIMUM_VALUE,
            QUANTITY_CLASS,
            UNIT_OF_MEASURE,
            MEASUREMENT_DESCRIPTION,
            GEN_HEADER_TYPE
        };
        static char const* name_as_string(Name n);
    } general_header_meas;
    class ChannelSetHeader {
    public:
        explicit ChannelSetHeader(CommonSEGD& com);
        int scan_type_number;
        uint16_t channel_set_number;
        uint8_t channel_type;
        uint32_t channel_set_start_time;
        uint32_t channel_set_end_time;
        double descale_multiplier;
        uint32_t number_of_channels;
        int subscans_per_ch_set;
        int channel_gain;
        int alias_filter_freq;
        int alias_filter_slope;
        int low_cut_filter_freq;
        int low_cut_filter_slope;
        int first_notch_filter;
        int second_notch_filter;
        int third_notch_filter;
        uint16_t ext_ch_set_num;
        int ext_hdr_flag;
        int trc_hdr_ext;
        uint8_t vert_stack;
        uint8_t streamer_no;
        uint8_t array_forming;
        std::optional<uint32_t> number_of_samples();
        std::optional<uint32_t> samp_int();
        std::optional<uint8_t> filter_phase();
        std::optional<uint8_t> physical_unit();
        std::optional<uint32_t> filter_delay();
        std::optional<std::array<char, 27>> description();
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            SCAN_TYPE_NUMBER,
            CHANNEL_SET_NUMBER,
            CHANNEL_TYPE,
            CHANNEL_SET_START_TIME,
            CHANNEL_SET_END_TIME,
            DESCALE_MULTIPLIER,
            NUMBER_OF_CHANNELS,
            SUBSCANS_PER_CH_SET,
            CHANNEL_GAIN,
            ALIAS_FILTER_FREQ,
            ALIAS_FILTER_SLOPE,
            LOW_CUT_FILTER_FREQ,
            LOW_CUT_FILTER_SLOPE,
            FIRST_NOTCH_FILTER,
            SECOND_NOTCH_FILTER,
            THIRD_NOTCH_FILTER,
            EXT_CH_SET_NUM,
            EXT_HDR_FLAG,
            TRC_HDR_EXT,
            VERT_STACK,
            STREAMER_NO,
            ARRAY_FORMING,
            NUMBER_OF_SAMPLES,
            SAMPLE_INTERVAL,
            FILTER_PHASE,
            PHYSICAL_UNIT,
            FILTER_DELAY,
            DESCRIPTION
        };
        static char const* name_as_string(Name n);

    private:
        uint8_t segd_rev_major;
        uint32_t p_number_of_samples;
        uint32_t p_samp_int;
        uint8_t p_filter_phase;
        uint8_t p_physical_unit;
        uint32_t p_filter_delay;
        std::array<char, 27> p_description;
    };
    static constexpr int GEN_HDR_SIZE = 32;
    static constexpr int CH_SET_HDR_SIZE = 32;
    static constexpr int CH_SET_HDR_R3_SIZE = 96;
    ///
    /// \pa m file_name Name of file.
    /// \param mode Choose input or output.
    ///
    CommonSEGD(std::string file_name, std::fstream::openmode mode);
    std::string file_name;
    std::fstream file;
    std::vector<char> gen_hdr_buf;
    std::vector<char> ch_set_hdr_buf;
    std::vector<std::vector<ChannelSetHeader>> ch_sets;
};
} // namespace sedaman

#endif // SEDAMAN_COMMON_SEGD_HPP
