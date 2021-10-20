///
/// @file ISEGD.hpp
/// @author Andrei Voronin (andalevor@gmail.com)
/// \brief 
/// @version 0.1
/// \date 2020-05-04
/// 
/// @copyright Copyright (c) 2020
/// 
///
#ifndef SEDAMAN_ISEGD_HPP
#define SEDAMAN_ISEGD_HPP

#include "CommonSEGD.hpp"
#include "Trace.hpp"
///
/// \brief General namespace for sedaman library.
/// \namespace sedaman
/// 
///
namespace sedaman {
///
/// \brief Class for SEGD reading.
/// Defines methods to read information from SEGD files.
/// \class ISegd
/// 
///
class ISEGD {
public:
    ///
    /// \brief Construct a new ISEGD object
    /// 
    /// \param file_name Name of file to read from.
    /// \param trc_hdr_ext Could be used to read trace header extensions.
    ///
    ISEGD(std::string file_name, std::vector<std::map<uint32_t,
    std::pair<std::string, Trace::Header::ValueType>>> trc_hdr_ext = {});
    ///
    /// \brief General header getter
    /// 
    /// \return CommonSEGD::GeneralHeader 
    ///
    CommonSEGD::GeneralHeader general_header();
    ///
    /// \brief General header 2 getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeader2> 
    ///
    std::optional<CommonSEGD::GeneralHeader2> general_header2();
    ///
    /// \brief General header N getter
    /// 
    /// \return std::vector<CommonSEGD::GeneralHeaderN> 
    ///
    std::vector<CommonSEGD::GeneralHeaderN> general_headerN();
    ///
    /// \brief General header 3 getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeader3> 
    ///
    std::optional<CommonSEGD::GeneralHeader3> general_header3();
    ///
    /// \brief General header vessel crew id getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderVes> 
    ///
    std::optional<CommonSEGD::GeneralHeaderVes>
   	general_header_vessel_crew_id();
    ///
    /// \brief General header survey area name getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderSur> 
    ///
    std::optional<CommonSEGD::GeneralHeaderSur> general_header_survey_name();
    ///
    /// \brief General header client name getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderCli> 
    ///
    std::optional<CommonSEGD::GeneralHeaderCli> general_header_client_name();
    ///
    /// \brief General header job id getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderJob> 
    ///
    std::optional<CommonSEGD::GeneralHeaderJob> general_header_job_id();
    ///
    /// \brief General header line id getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderLin> 
    ///
    std::optional<CommonSEGD::GeneralHeaderLin> general_header_line_id();
    /// \brief General header vibrator source info getter
    /// \return return third general header or std::nullopt if there is no
	///                such header in file
    std::optional<CommonSEGD::GeneralHeaderVib> general_header_vibrator_info();
    ///
    /// \brief General header explosive source info getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderExp> 
    ///
    std::optional<CommonSEGD::GeneralHeaderExp>
	   	general_header_explosive_info();
    ///
    /// \brief General header airgun source info getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderAir> 
    ///
    std::optional<CommonSEGD::GeneralHeaderAir> general_header_airgun_info();
    ///
    /// \brief General header watergun source info getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderWat> 
    ///
    std::optional<CommonSEGD::GeneralHeaderWat> general_header_watergun_info();
    ///
    /// \brief General header electromagnetic source info getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderEle> 
    ///
    std::optional<CommonSEGD::GeneralHeaderEle>
   	general_header_electromagnetic_info();
    ///
    /// \brief General header other source type info getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderOth> 
    ///
    std::optional<CommonSEGD::GeneralHeaderOth>
   	general_header_other_source_info();
    ///
    /// \brief General header additional source type info getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderAdd> 
    ///
    std::optional<CommonSEGD::GeneralHeaderAdd>
   	general_header_add_source_info();
    ///
    /// \brief General header source auxiliary channel reference getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderSaux> 
    ///
    std::optional<CommonSEGD::GeneralHeaderSaux>
	   	general_header_sou_aux_chan_ref();
    ///
    /// \brief General header sensor info header extension block getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderSen> 
    ///
    std::optional<CommonSEGD::GeneralHeaderSen>
   	general_header_sen_info_hdr_ext_blk();
    ///
    /// \brief General header sensor calibration block getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderSCa> 
    ///
    std::optional<CommonSEGD::GeneralHeaderSCa> general_header_sen_calib_blk();
    ///
    /// \brief General header time drift block getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderTim> 
    ///
    std::optional<CommonSEGD::GeneralHeaderTim>
   	general_header_time_drift_blk();
    ///
    /// \brief General header electromagnetic source receiver description
	/// block getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderElSR> 
    ///
    std::optional<CommonSEGD::GeneralHeaderElSR>
   	general_header_elemag_src_rec_desc_blk();
    ///
    /// \brief General header position block 1 getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderPos1> 
    ///
    std::optional<CommonSEGD::GeneralHeaderPos1>
   	general_header_position_blk_1();
    ///
    /// \brief General header position block 2 getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderPos2> 
    ///
    std::optional<CommonSEGD::GeneralHeaderPos2>
   	general_header_position_blk_2();
    ///
    /// \brief General header position block 3 getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderPos3> 
    ///
    std::optional<CommonSEGD::GeneralHeaderPos3>
   	general_header_position_blk_3();
    ///
    /// \brief General header coordinate reference system block getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderCoord> 
    ///
    std::optional<CommonSEGD::GeneralHeaderCoord>
   	general_header_coord_ref_blk();
    ///
    /// \brief General header relative position block getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderRel> 
    ///
    std::optional<CommonSEGD::GeneralHeaderRel>
   	general_header_relative_pos_blk();
    ///
    /// \brief General header orientation header block getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderOri> 
    ///
    std::optional<CommonSEGD::GeneralHeaderOri>
   	general_header_orient_hdr_blk();
    ///
    /// \brief General header measurement block getter
    /// 
    /// \return std::optional<CommonSEGD::GeneralHeaderMeas> 
    ///
    std::optional<CommonSEGD::GeneralHeaderMeas>
   	general_header_measurement_blk();
    ///
    /// \brief Extended headers getter
    /// 
    /// \return std::vector<std::vector<char>> 
    ///
    std::vector<std::vector<char>> extended_headers();
    ///
    /// \brief External headers getter
    /// 
    /// \return std::vector<std::vector<char>> 
    ///
    std::vector<std::vector<char>> external_headers();
    ///
    /// \brief Channel set headers getter
    /// 
    /// \return std::vector<std::vector<CommonSEGD::ChannelSetHeader>> const& 
    ///
    std::vector<std::vector<CommonSEGD::ChannelSetHeader>> const&
	   	channel_set_headers();
    ///
    /// \brief Return true if there are record to read.
    /// 
    /// \return true 
    /// \return false 
    ///
    bool has_record();
    ///
    /// \brief Return true if there are traces to read in current record
    /// 
    /// \return true 
    /// \return false 
    ///
    bool has_trace();
    ///
    /// \brief Reads trace
    /// 
    /// \return Trace 
    ///
    Trace read_trace();
    ~ISEGD();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_ISEGD_HPP
