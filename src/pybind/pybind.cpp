#include "CommonSEGY.hpp"
#include "ISEGY.hpp"
#include "ISEGYSorted1D.hpp"
#include "OSEGY.hpp"
#include "OSEGYRev0.hpp"
#include "OSEGYRev1.hpp"
#include "OSEGYRev2.hpp"
#include "CommonSEGD.hpp"
#include "ISEGD.hpp"
#include "OSEGD.hpp"
#include "OSEGDRev2_1.hpp"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/numpy.h"

namespace sedaman {
namespace py = pybind11;
using std::string;
using std::unordered_map;
using std::vector;
using std::pair;
using std::map;
using std::shared_ptr;

PYBIND11_MODULE(pysedaman, m) {
	py::class_<Trace> Trace_py(m, "Trace");
	Trace_py.def(py::init<unordered_map<string, Trace::Header::Value>,
				 vector<double>>());
	Trace_py.def("header", &Trace::header, "Returns trace header");
	Trace_py.def("samples", &Trace::samples, "Returns trace samples");
	Trace_py.def("samples_as_numpy_array",
				 [](Trace &t) { return py::array_t<double>(t.samples()); },
				 "Returns trace samples as numpy array");
	py::enum_<Trace::Header::ValueType>(Trace_py, "TrHdrValueType")
		.value("int8_t", Trace::Header::ValueType::int8_t)
		.value("uint8_t", Trace::Header::ValueType::uint8_t)
		.value("int16_t", Trace::Header::ValueType::int16_t)
		.value("uint16_t", Trace::Header::ValueType::uint16_t)
		.value("int32_t", Trace::Header::ValueType::int32_t)
		.value("uint32_t", Trace::Header::ValueType::uint32_t)
		.value("int64_t", Trace::Header::ValueType::int64_t)
		.value("uint64_t", Trace::Header::ValueType::uint64_t)
		.value("ibm", Trace::Header::ValueType::ibm)
		.value("ieee_single", Trace::Header::ValueType::ieee_single)
		.value("ieee_double", Trace::Header::ValueType::ieee_double);

	py::class_<Trace::Header> Header_py(Trace_py, "Header");
	Header_py.def(py::init<unordered_map<string, Trace::Header::Value>>());
	Header_py.def("get", &Trace::Header::get,
				  "Gets header value by specified key");
	Header_py.def("set", &Trace::Header::set,
				  "Sets or adds header value by specified key",
				  py::arg("key"), py::arg("v"));
	Header_py.def("keys", &Trace::Header::keys, "Retruns all keys for header");

	py::class_<CommonSEGY> CommonSEGY_py(m, "CommonSEGY");
	CommonSEGY_py.def("ebcdic_to_ascii", &CommonSEGY::ebcdic_to_ascii);
	CommonSEGY_py.def("ascii_to_ebcdic", &CommonSEGY::ascii_to_ebcdic);
	CommonSEGY_py.def_readonly_static("default_text_header",
									  &CommonSEGY::default_text_header);
	CommonSEGY_py.def_readonly_static("TEXT_HEADER_SIZE",
									  &CommonSEGY::TEXT_HEADER_SIZE);
	CommonSEGY_py.def_readonly_static("TR_HEADER_SIZE",
									  &CommonSEGY::TR_HEADER_SIZE);
	CommonSEGY_py.def_readonly_static("trace_header_description",
									  &CommonSEGY::trace_header_description);
	CommonSEGY_py.def_readonly_static("default_trace_header",
									  &CommonSEGY::default_trace_header);

	py::class_<CommonSEGY::BinaryHeader> BinaryHeader_py(CommonSEGY_py,
														 "BinaryHeader");
	BinaryHeader_py.def(py::init<>());
	BinaryHeader_py.def("name_as_string",
					   	&CommonSEGY::BinaryHeader::name_as_string);
	BinaryHeader_py.def_readwrite("job_id", &CommonSEGY::BinaryHeader::job_id);
	BinaryHeader_py.def_readwrite("line_num", &CommonSEGY::BinaryHeader::
								  line_num);
	BinaryHeader_py.def_readwrite("reel_num", &CommonSEGY::BinaryHeader::
								  reel_num);
	BinaryHeader_py.def_readwrite("tr_per_ens", &CommonSEGY::BinaryHeader::
								  tr_per_ens);
	BinaryHeader_py.def_readwrite("aux_per_ens", &CommonSEGY::BinaryHeader::
								  aux_per_ens);
	BinaryHeader_py.def_readwrite("samp_int", &CommonSEGY::BinaryHeader::
								  samp_int);
	BinaryHeader_py.def_readwrite("samp_int_orig", &CommonSEGY::BinaryHeader::
								  samp_int_orig);
	BinaryHeader_py.def_readwrite("samp_per_tr", &CommonSEGY::BinaryHeader::
								  samp_per_tr);
	BinaryHeader_py.def_readwrite("samp_per_tr_orig", &CommonSEGY::
								  BinaryHeader::samp_per_tr_orig);
	BinaryHeader_py.def_readwrite("format_code", &CommonSEGY::BinaryHeader::
								  format_code);
	BinaryHeader_py.def_readwrite("ens_fold", &CommonSEGY::BinaryHeader::
								  ens_fold);
	BinaryHeader_py.def_readwrite("sort_code", &CommonSEGY::BinaryHeader::
								  sort_code);
	BinaryHeader_py.def_readwrite("vert_sum_code", &CommonSEGY::BinaryHeader::
								  vert_sum_code);
	BinaryHeader_py.def_readwrite("sw_freq_at_start", &CommonSEGY::
								  BinaryHeader::sw_freq_at_start);
	BinaryHeader_py.def_readwrite("sw_freq_at_end", &CommonSEGY::
								  BinaryHeader::sw_freq_at_end);
	BinaryHeader_py.def_readwrite("sw_length", &CommonSEGY::BinaryHeader::
								  sw_length);
	BinaryHeader_py.def_readwrite("sw_type_code", &CommonSEGY::BinaryHeader::
								  sw_type_code);
	BinaryHeader_py.def_readwrite("sw_ch_tr_num", &CommonSEGY::BinaryHeader::
								  sw_ch_tr_num);
	BinaryHeader_py.def_readwrite("taper_at_start", &CommonSEGY::BinaryHeader::
								  taper_at_start);
	BinaryHeader_py.def_readwrite("taper_at_end", &CommonSEGY::BinaryHeader::
								  taper_at_end);
	BinaryHeader_py.def_readwrite("taper_type", &CommonSEGY::BinaryHeader::
								  taper_type);
	BinaryHeader_py.def_readwrite("corr_traces", &CommonSEGY::BinaryHeader::
								  corr_traces);
	BinaryHeader_py.def_readwrite("bin_gain_recov", &CommonSEGY::BinaryHeader::
								  bin_gain_recov);
	BinaryHeader_py.def_readwrite("amp_recov_meth", &CommonSEGY::BinaryHeader::
								  amp_recov_meth);
	BinaryHeader_py.def_readwrite("measure_system", &CommonSEGY::BinaryHeader::
								  measure_system);
	BinaryHeader_py.def_readwrite("impulse_sig_pol", &CommonSEGY::
								  BinaryHeader::impulse_sig_pol);
	BinaryHeader_py.def_readwrite("vib_pol_code", &CommonSEGY::BinaryHeader::
								  vib_pol_code);
	BinaryHeader_py.def_readwrite("ext_tr_per_ens", &CommonSEGY::BinaryHeader::
								  ext_tr_per_ens);
	BinaryHeader_py.def_readwrite("ext_aux_per_ens", &CommonSEGY::
								  BinaryHeader::ext_aux_per_ens);
	BinaryHeader_py.def_readwrite("ext_samp_per_tr", &CommonSEGY::
								  BinaryHeader::ext_samp_per_tr);
	BinaryHeader_py.def_readwrite("ext_samp_int", &CommonSEGY::BinaryHeader::
								  ext_samp_int);
	BinaryHeader_py.def_readwrite("ext_samp_int_orig", &CommonSEGY::
								  BinaryHeader::ext_samp_int_orig);
	BinaryHeader_py.def_readwrite("ext_samp_per_tr_orig", &CommonSEGY::
								  BinaryHeader::ext_samp_per_tr_orig);
	BinaryHeader_py.def_readwrite("ext_ens_fold", &CommonSEGY::BinaryHeader::
								  ext_ens_fold);
	BinaryHeader_py.def_readwrite("endianness", &CommonSEGY::BinaryHeader::
								  endianness);
	BinaryHeader_py.def_readwrite("SEGY_rev_major_ver", &CommonSEGY::
								  BinaryHeader::SEGY_rev_major_ver);
	BinaryHeader_py.def_readwrite("SEGY_rev_minor_ver", &CommonSEGY::
								  BinaryHeader::SEGY_rev_minor_ver);
	BinaryHeader_py.def_readwrite("fixed_tr_length", &CommonSEGY::
								  BinaryHeader::fixed_tr_length);
	BinaryHeader_py.def_readwrite("ext_text_headers_num", &CommonSEGY::
								  BinaryHeader::ext_text_headers_num);
	BinaryHeader_py.def_readwrite("max_num_add_tr_headers", &CommonSEGY::
								  BinaryHeader::max_num_add_tr_headers);
	BinaryHeader_py.def_readwrite("time_basis_code", &CommonSEGY::
								  BinaryHeader::time_basis_code);
	BinaryHeader_py.def_readwrite("num_of_tr_in_file", &CommonSEGY::
								  BinaryHeader::num_of_tr_in_file);
	BinaryHeader_py.def_readwrite("byte_off_of_first_tr", &CommonSEGY::
								  BinaryHeader::byte_off_of_first_tr);
	BinaryHeader_py.def_readwrite("num_of_trailer_stanza", &CommonSEGY::
								  BinaryHeader::num_of_trailer_stanza);
	py::enum_<CommonSEGY::BinaryHeader::Name>(BinaryHeader_py, "Name");

	py::class_<ISEGY> ISEGY_py(m, "ISEGY");
	ISEGY_py.def(py::init<string,
				 vector<pair<string, map<uint32_t,
				 pair<string, Trace::Header::ValueType>>>>>(),
				 py::arg("file_name"),
				 py::arg("tr_hdr_map") = CommonSEGY::default_trace_header);
	ISEGY_py.def(py::init<string, CommonSEGY::BinaryHeader,
				 vector<pair<string, map<uint32_t,
				 pair<string, Trace::Header::ValueType>>>>>(),
				 py::arg("file_name"), py::arg("binary_header"),
				 py::arg("tr_hdr_map") = CommonSEGY::default_trace_header);
	ISEGY_py.def("read_binary_header", &ISEGY::read_binary_header,
				 "creates ISegy instance internally and "
				 "returns binary header.");
	ISEGY_py.def("text_headers", [] (ISEGY& s) {
		vector<string> res;
		for(auto h : s.text_headers()) {
			CommonSEGY::ebcdic_to_ascii(h);
			res.push_back(std::move(h));
		}
		return res;},
				 "segy text headers getter");
	ISEGY_py.def("trailer_stanzas", &ISEGY::trailer_stanzas,
				 "segy trailer stanzas getter");
	ISEGY_py.def("binary_header", &ISEGY::binary_header,
				 "segy binary header getter");
	ISEGY_py.def("has_trace", &ISEGY::has_trace,
				 "checks for next trace in file");
	ISEGY_py.def("read_header", &ISEGY::read_header,
				 "reads header, skips samples");
	ISEGY_py.def("read_trace", &ISEGY::read_trace,
				 "reads one trace from file");
	ISEGY_py.def("__next__", [] (ISEGY& s) {
		return s.has_trace() ? s.read_trace() : throw py::stop_iteration();
	});
	ISEGY_py.def("__iter__", [] (ISEGY& s) { return &s; });

    py::class_<ISEGYSorted1D, ISEGY> ISEGYSorted1D_py(m, "ISEGYSorted1D");
    ISEGYSorted1D_py.def(
        py::init<string, string,
                 vector<pair<
                     string,
                     map<uint32_t, pair<string, Trace::Header::ValueType>>>>>(),
        py::arg("file_name"),
		py::arg("hdr_name"),
        py::arg("tr_hdr_map") = CommonSEGY::default_trace_header);
    ISEGYSorted1D_py.def(
        py::init<string, string, CommonSEGY::BinaryHeader,
                 vector<pair<
                     string,
                     map<uint32_t, pair<string, Trace::Header::ValueType>>>>>(),
        py::arg("file_name"),
		py::arg("hdr_name"),
		py::arg("binary_header"),
        py::arg("tr_hdr_map") = CommonSEGY::default_trace_header);
    ISEGYSorted1D_py.def(
        "get_keys", &ISEGYSorted1D::get_keys,
        "Get list of keys which could be used to get headers or traces");
    ISEGYSorted1D_py.def(
        "get_headers", &ISEGYSorted1D::get_headers,
        "Get list of trace headers with given trace header value",
						 py::arg("value"), py::arg("max_num") = 1000000);
    ISEGYSorted1D_py.def("get_traces", &ISEGYSorted1D::get_traces,
                         "Get list of traces with given trace header value",
						 py::arg("value"), py::arg("max_num") = 100000);
	ISEGYSorted1D_py.def("__next__", [] (ISEGYSorted1D& s) {
		return s.has_trace() ? s.read_trace() : throw py::stop_iteration();
	});
	ISEGYSorted1D_py.def("__iter__", [] (ISEGYSorted1D& s) { return &s; });

    py::class_<OSEGY> OSEGY_py(m, "OSEGY");
	OSEGY_py.def("write_trace", &OSEGY::write_trace,
				 "Writes trace to the end of file.",
				 py::arg("trace"));

	py::class_<OSEGYRev0, OSEGY> OSEGYRev0_py(m, "OSEGYRev0");
	OSEGYRev0_py.def(py::init<string, string, CommonSEGY::BinaryHeader, 
					 vector<pair<string, map<uint32_t,
					 pair<string, Trace::Header::ValueType>>>>>(),
					 py::arg("file_name"), py::arg("text_header") = string(),
					 py::arg("bin_header") = CommonSEGY::BinaryHeader(),
					 py::arg("tr_hdr_map") = CommonSEGY::default_trace_header);

	py::class_<OSEGYRev1, OSEGY> OSEGYRev1_py(m, "OSEGYRev1");
	OSEGYRev1_py.def(py::init<string, vector<string>,
					 CommonSEGY::BinaryHeader, vector<pair<string, map<uint32_t,
					 pair<string, Trace::Header::ValueType>>>>>(),
					 py::arg("file_name"),
					 py::arg("text_headers") = vector<string>(),
					 py::arg("bin_header") = CommonSEGY::BinaryHeader(),
					 py::arg("tr_hdr_map") = CommonSEGY::default_trace_header);

	py::class_<OSEGYRev2, OSEGY> OSEGYRev2_py(m, "OSEGYRev2");
	OSEGYRev2_py.def(py::init<string, vector<string>, CommonSEGY::BinaryHeader,
					 vector<string>, vector<pair<string, map<uint32_t,
					 pair<string, Trace::Header::ValueType>>>>>(),
					 py::arg("file_name"),
					 py::arg("text_headers") = vector<string>(),
					 py::arg("bin_header") = CommonSEGY::BinaryHeader(),
					 py::arg("trailer_stanzas") = vector<string>(),
					 py::arg("tr_hdr_map") = CommonSEGY::default_trace_header);

	py::class_<CommonSEGD> CommonSEGD_py(m, "CommonSEGD");
	CommonSEGD_py.def_readonly_static("GEN_HDR_SIZE",
									  &CommonSEGD::GEN_HDR_SIZE);
	CommonSEGD_py.def_readonly_static("GEN_TRLR_SIZE",
									  &CommonSEGD::GEN_TRLR_SIZE);
	CommonSEGD_py.def_readonly_static("CH_SET_HDR_SIZE",
									  &CommonSEGD::CH_SET_HDR_SIZE);
	CommonSEGD_py.def_readonly_static("CH_SET_HDR_R3_SIZE",
									  &CommonSEGD::CH_SET_HDR_R3_SIZE);
	CommonSEGD_py.def_readonly_static("SKEW_BLOCK_SIZE",
									  &CommonSEGD::SKEW_BLOCK_SIZE);
	CommonSEGD_py.def_readonly_static("EXTENDED_HEADER_SIZE",
									  &CommonSEGD::EXTENDED_HEADER_SIZE);
	CommonSEGD_py.def_readonly_static("EXTERNAL_HEADER_SIZE",
									  &CommonSEGD::EXTERNAL_HEADER_SIZE);
	CommonSEGD_py.def_readonly_static("TRACE_HEADER_SIZE",
									  &CommonSEGD::TRACE_HEADER_SIZE);
	CommonSEGD_py.def_readonly_static("TRACE_HEADER_EXT_SIZE",
									  &CommonSEGD::TRACE_HEADER_EXT_SIZE);

	py::class_<CommonSEGD::GeneralHeader>
	   	GeneralHeader_py(CommonSEGD_py, "GeneralHeader");
	GeneralHeader_py.def_readwrite
		("file_number", &CommonSEGD::GeneralHeader::file_number);
	GeneralHeader_py.def_readwrite
		("format_code", &CommonSEGD::GeneralHeader::format_code);
	GeneralHeader_py.def_readwrite
		("gen_const", &CommonSEGD::GeneralHeader::gen_const);
	GeneralHeader_py.def_readwrite
		("year", &CommonSEGD::GeneralHeader::year);
	GeneralHeader_py.def_readwrite
		("add_gen_hdr_blocks", &CommonSEGD::GeneralHeader::add_gen_hdr_blocks);
	GeneralHeader_py.def_readwrite
		("day", &CommonSEGD::GeneralHeader::day);
	GeneralHeader_py.def_readwrite
		("hour", &CommonSEGD::GeneralHeader::hour);
	GeneralHeader_py.def_readwrite
		("minute", &CommonSEGD::GeneralHeader::minute);
	GeneralHeader_py.def_readwrite
		("second", &CommonSEGD::GeneralHeader::second);
	GeneralHeader_py.def_readwrite
		("manufac_code", &CommonSEGD::GeneralHeader::manufac_code);
	GeneralHeader_py.def_readwrite
		("manufac_num", &CommonSEGD::GeneralHeader::manufac_num);
	GeneralHeader_py.def_readwrite
		("bytes_per_scan", &CommonSEGD::GeneralHeader::bytes_per_scan);
	GeneralHeader_py.def_readwrite
		("base_scan_int", &CommonSEGD::GeneralHeader::base_scan_int);
	GeneralHeader_py.def_readwrite
		("polarity", &CommonSEGD::GeneralHeader::polarity);
	GeneralHeader_py.def_readwrite
		("scans_per_block", &CommonSEGD::GeneralHeader::scans_per_block);
	GeneralHeader_py.def_readwrite
		("record_type", &CommonSEGD::GeneralHeader::record_type);
	GeneralHeader_py.def_readwrite
		("record_length", &CommonSEGD::GeneralHeader::record_length);
	GeneralHeader_py.def_readwrite
		("scan_types_per_record",
		 &CommonSEGD::GeneralHeader::scan_types_per_record);
	GeneralHeader_py.def_readwrite
		("channel_sets_per_scan_type",
		 &CommonSEGD::GeneralHeader::channel_sets_per_scan_type);
	GeneralHeader_py.def_readwrite
		("skew_blocks", &CommonSEGD::GeneralHeader::skew_blocks);
	GeneralHeader_py.def_readwrite
		("extended_hdr_blocks",
		 &CommonSEGD::GeneralHeader::extended_hdr_blocks);
	GeneralHeader_py.def_readwrite
		("external_hdr_blocks",
		 &CommonSEGD::GeneralHeader::external_hdr_blocks);
	py::enum_<CommonSEGD::GeneralHeader::Name>(GeneralHeader_py, "Name");
	GeneralHeader_py.def("name_as_string",
						 &CommonSEGD::GeneralHeader::name_as_string);

	py::class_<CommonSEGD::GeneralHeader2>
	   	GeneralHeader2_py(CommonSEGD_py, "GeneralHeader2");
	GeneralHeader2_py.def_readwrite
		("expanded_file_num", &CommonSEGD::GeneralHeader2::expanded_file_num);
	GeneralHeader2_py.def_readwrite
		("ext_ch_sets_per_scan_type",
		 &CommonSEGD::GeneralHeader2::ext_ch_sets_per_scan_type);
	GeneralHeader2_py.def_readwrite
		("extended_hdr_blocks",
		 &CommonSEGD::GeneralHeader2::extended_hdr_blocks);
	GeneralHeader2_py.def_readwrite
		("external_hdr_blocks",
		 &CommonSEGD::GeneralHeader2::external_hdr_blocks);
	GeneralHeader2_py.def_readwrite
		("extended_skew_blocks",
		 &CommonSEGD::GeneralHeader2::extended_skew_blocks);
	GeneralHeader2_py.def_readwrite
		("segd_rev_major", &CommonSEGD::GeneralHeader2::segd_rev_major);
	GeneralHeader2_py.def_readwrite
		("segd_rev_minor", &CommonSEGD::GeneralHeader2::segd_rev_minor);
	GeneralHeader2_py.def_readwrite
		("gen_trailer_num_of_blocks",
		 &CommonSEGD::GeneralHeader2::gen_trailer_num_of_blocks);
	GeneralHeader2_py.def_readwrite
		("ext_record_len", &CommonSEGD::GeneralHeader2::ext_record_len);
	GeneralHeader2_py.def_readwrite
		("record_set_number", &CommonSEGD::GeneralHeader2::record_set_number);
	GeneralHeader2_py.def_readwrite
		("ext_num_add_blks_in_gen_hdr",
		 &CommonSEGD::GeneralHeader2::ext_num_add_blks_in_gen_hdr);
	GeneralHeader2_py.def_readwrite
		("dominant_sampling_int",
		 &CommonSEGD::GeneralHeader2::dominant_sampling_int);
	GeneralHeader2_py.def_readwrite
		("gen_hdr_block_num", &CommonSEGD::GeneralHeader2::gen_hdr_block_num);
	GeneralHeader2_py.def_readwrite
		("sequence_number", &CommonSEGD::GeneralHeader2::sequence_number);
	py::enum_<CommonSEGD::GeneralHeader2::Name>(GeneralHeader2_py, "Name");
	GeneralHeader2_py.def("name_as_string",
 						  &CommonSEGD::GeneralHeader2::name_as_string);

	py::class_<CommonSEGD::AdditionalGeneralHeader>
	   	AdditionalGeneralHeader_py(CommonSEGD_py, "AdditionalGeneralHeader");
	py::enum_<CommonSEGD::AdditionalGeneralHeader::ADD_GEN_HDR_BLKS>
		(AdditionalGeneralHeader_py, "ADD_GEN_HDR_BLKS");
	AdditionalGeneralHeader_py.def("type",
								   &CommonSEGD::AdditionalGeneralHeader::type);

	py::class_<CommonSEGD::GeneralHeaderN, CommonSEGD::AdditionalGeneralHeader>
		GeneralHeaderN_py(CommonSEGD_py, "GeneralHeaderN");
	GeneralHeaderN_py.def_readwrite
		("expanded_file_number",
		 &CommonSEGD::GeneralHeaderN::expanded_file_number);
	GeneralHeaderN_py.def_readwrite
		("sou_line_num", &CommonSEGD::GeneralHeaderN::sou_line_num);
	GeneralHeaderN_py.def_readwrite
		("sou_point_num", &CommonSEGD::GeneralHeaderN::sou_point_num);
	GeneralHeaderN_py.def_readwrite
		("phase_control", &CommonSEGD::GeneralHeaderN::phase_control);
	GeneralHeaderN_py.def_readwrite
		("type_vibrator", &CommonSEGD::GeneralHeaderN::type_vibrator);
	GeneralHeaderN_py.def_readwrite
		("phase_angle", &CommonSEGD::GeneralHeaderN::phase_angle);
	GeneralHeaderN_py.def_readwrite
		("gen_hdr_block_num", &CommonSEGD::GeneralHeaderN::gen_hdr_block_num);
	GeneralHeaderN_py.def_readwrite
		("sou_set_num", &CommonSEGD::GeneralHeaderN::sou_set_num);
	py::enum_<CommonSEGD::GeneralHeaderN::Name>(GeneralHeaderN_py, "Name");
	GeneralHeaderN_py.def("name_as_string",
						  &CommonSEGD::GeneralHeaderN::name_as_string);

	py::class_<CommonSEGD::GeneralHeader3>
	   	GeneralHeader3_py(CommonSEGD_py,"GeneralHeader3");
	GeneralHeader3_py.def_readwrite
		("time_zero", &CommonSEGD::GeneralHeader3::time_zero);
	GeneralHeader3_py.def_readwrite
		("record_size", &CommonSEGD::GeneralHeader3::record_size);
	GeneralHeader3_py.def_readwrite
		("data_size", &CommonSEGD::GeneralHeader3::data_size);
	GeneralHeader3_py.def_readwrite
		("header_size", &CommonSEGD::GeneralHeader3::header_size);
	GeneralHeader3_py.def_readwrite
		("extd_rec_mode", &CommonSEGD::GeneralHeader3::extd_rec_mode);
	GeneralHeader3_py.def_readwrite
		("rel_time_mode", &CommonSEGD::GeneralHeader3::rel_time_mode);
	GeneralHeader3_py.def_readwrite
		("gen_hdr_block_num", &CommonSEGD::GeneralHeader3::gen_hdr_block_num);
	py::enum_<CommonSEGD::GeneralHeader3::Name>(GeneralHeader3_py, "Name");
	GeneralHeader3_py.def("name_as_string",
						  &CommonSEGD::GeneralHeader3::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderVes,
	   	CommonSEGD::AdditionalGeneralHeader>
		   	GeneralHeaderVes_py(CommonSEGD_py, "GeneralHeaderVes");
	GeneralHeaderVes_py.def_property
		("abbr_vessel_crew_name",
		 [] (CommonSEGD::GeneralHeaderVes &h)
		 { return string(h.abbr_vessel_crew_name, 3); },
		 [] (CommonSEGD::GeneralHeaderVes &h, string &s)
		 { strncpy(h.abbr_vessel_crew_name, s.c_str(), 3); });
	GeneralHeaderVes_py.def_property
		("vessel_crew_name",
		 [] (CommonSEGD::GeneralHeaderVes &h)
		 { return string(h.vessel_crew_name, 28); },
		 [] (CommonSEGD::GeneralHeaderVes &h, string &s)
		 { strncpy(h.vessel_crew_name, s.c_str(), 28); });
	GeneralHeaderVes_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderVes::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderVes::Name>(GeneralHeaderVes_py, "Name");
	GeneralHeaderVes_py.def("name_as_string",
  							&CommonSEGD::GeneralHeaderVes::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderSur,
	   	CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderSur_py(CommonSEGD_py, "GeneralHeaderSur");
	GeneralHeaderSur_py.def_property
		("survey_area_name",
		 [] (CommonSEGD::GeneralHeaderSur &h)
		 { return string(h.survey_area_name, 31); },
		 [] (CommonSEGD::GeneralHeaderSur &h, string &s)
		 { strncpy(h.survey_area_name, s.c_str(), 31); });
	GeneralHeaderSur_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderSur::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderSur::Name>(GeneralHeaderSur_py, "Name");
	GeneralHeaderSur_py.def("name_as_string",
  							&CommonSEGD::GeneralHeaderSur::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderCli,
 		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderCli_py(CommonSEGD_py, "GeneralHeaderCli");
	GeneralHeaderCli_py.def_property
		("client_name",
		 [] (CommonSEGD::GeneralHeaderCli &h)
		 { return string(h.client_name, 31); },
		 [] (CommonSEGD::GeneralHeaderCli &h, string &s)
		 { strncpy(h.client_name, s.c_str(), 31); });
	GeneralHeaderCli_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderCli::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderCli::Name>(GeneralHeaderCli_py, "Name");
	GeneralHeaderCli_py.def("name_as_string",
  							&CommonSEGD::GeneralHeaderCli::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderJob,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderJob_py(CommonSEGD_py, "GeneralHeaderJob");
	GeneralHeaderJob_py.def_property
		("abbr_job_id",
		 [] (CommonSEGD::GeneralHeaderJob &h)
		 { return string(h.abbr_job_id, 5); },
		 [] (CommonSEGD::GeneralHeaderJob &h, string &s)
		 { strncpy(h.abbr_job_id, s.c_str(), 5); });
	GeneralHeaderJob_py.def_property
		("job_id",
		 [] (CommonSEGD::GeneralHeaderJob &h)
		 { return string(h.job_id, 26); },
		 [] (CommonSEGD::GeneralHeaderJob &h, string &s)
		 { strncpy(h.job_id, s.c_str(), 26); });
	GeneralHeaderJob_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderJob::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderJob::Name>(GeneralHeaderJob_py, "Name");
	GeneralHeaderJob_py.def("name_as_string",
  							&CommonSEGD::GeneralHeaderJob::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderLin,
	   	CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderLin_py(CommonSEGD_py, "GeneralHeaderLin");
	GeneralHeaderLin_py.def_property
		("line_abbr",
		 [] (CommonSEGD::GeneralHeaderLin &h)
		 { return string(h.line_abbr, 7); },
		 [] (CommonSEGD::GeneralHeaderLin &h, string &s)
		 { strncpy(h.line_abbr, s.c_str(), 7); });
	GeneralHeaderLin_py.def_property
		("line_id",
		 [] (CommonSEGD::GeneralHeaderLin &h)
		 { return string(h.line_id, 24); },
		 [] (CommonSEGD::GeneralHeaderLin &h, string &s)
		 { strncpy(h.line_id, s.c_str(), 24); });
	GeneralHeaderLin_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderLin::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderLin::Name>(GeneralHeaderLin_py, "Name");
	GeneralHeaderLin_py.def("name_as_string",
  							&CommonSEGD::GeneralHeaderLin::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderVib,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderVib_py(CommonSEGD_py, "GeneralHeaderVib");
	GeneralHeaderVib_py.def_readwrite
		("expanded_file_number",
		 &CommonSEGD::GeneralHeaderVib::expanded_file_number);
	GeneralHeaderVib_py.def_readwrite
		("sou_line_num", &CommonSEGD::GeneralHeaderVib::sou_line_num);
	GeneralHeaderVib_py.def_readwrite
		("sou_point_num", &CommonSEGD::GeneralHeaderVib::sou_point_num);
	GeneralHeaderVib_py.def_readwrite
		("sou_point_index", &CommonSEGD::GeneralHeaderVib::sou_point_index);
	GeneralHeaderVib_py.def_readwrite
		("phase_control", &CommonSEGD::GeneralHeaderVib::phase_control);
	GeneralHeaderVib_py.def_readwrite
		("type_vibrator", &CommonSEGD::GeneralHeaderVib::type_vibrator);
	GeneralHeaderVib_py.def_readwrite
		("phase_angle", &CommonSEGD::GeneralHeaderVib::phase_angle);
	GeneralHeaderVib_py.def_readwrite
		("source_id", &CommonSEGD::GeneralHeaderVib::source_id);
	GeneralHeaderVib_py.def_readwrite
		("source_set_num", &CommonSEGD::GeneralHeaderVib::source_set_num);
	GeneralHeaderVib_py.def_readwrite
		("reshoot_idx", &CommonSEGD::GeneralHeaderVib::reshoot_idx);
	GeneralHeaderVib_py.def_readwrite
		("group_idx", &CommonSEGD::GeneralHeaderVib::group_idx);
	GeneralHeaderVib_py.def_readwrite
		("depth_idx", &CommonSEGD::GeneralHeaderVib::depth_idx);
	GeneralHeaderVib_py.def_readwrite
		("offset_crossline", &CommonSEGD::GeneralHeaderVib::offset_crossline);
	GeneralHeaderVib_py.def_readwrite
		("offset_inline", &CommonSEGD::GeneralHeaderVib::offset_inline);
	GeneralHeaderVib_py.def_readwrite
		("size", &CommonSEGD::GeneralHeaderVib::size);
	GeneralHeaderVib_py.def_readwrite
		("offset_depth", &CommonSEGD::GeneralHeaderVib::offset_depth);
	GeneralHeaderVib_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderVib::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderVib::Name>(GeneralHeaderVib_py, "Name");
	GeneralHeaderVib_py.def("name_as_string",
  							&CommonSEGD::GeneralHeaderVib::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderExp,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderExp_py(CommonSEGD_py, "GeneralHeaderExp");
	GeneralHeaderExp_py.def_readwrite
		("expanded_file_number",
		 &CommonSEGD::GeneralHeaderExp::expanded_file_number);
	GeneralHeaderExp_py.def_readwrite
		("sou_line_num", &CommonSEGD::GeneralHeaderExp::sou_line_num);
	GeneralHeaderExp_py.def_readwrite
		("sou_point_num", &CommonSEGD::GeneralHeaderExp::sou_point_num);
	GeneralHeaderExp_py.def_readwrite
		("sou_point_index", &CommonSEGD::GeneralHeaderExp::sou_point_index);
	GeneralHeaderExp_py.def_readwrite
		("depth", &CommonSEGD::GeneralHeaderExp::depth);
	GeneralHeaderExp_py.def_readwrite
		("charge_length", &CommonSEGD::GeneralHeaderExp::charge_length);
	GeneralHeaderExp_py.def_readwrite
		("soil_type", &CommonSEGD::GeneralHeaderExp::soil_type);
	GeneralHeaderExp_py.def_readwrite
		("source_id", &CommonSEGD::GeneralHeaderExp::source_id);
	GeneralHeaderExp_py.def_readwrite
		("source_set_num", &CommonSEGD::GeneralHeaderExp::source_set_num);
	GeneralHeaderExp_py.def_readwrite
		("reshoot_idx", &CommonSEGD::GeneralHeaderExp::reshoot_idx);
	GeneralHeaderExp_py.def_readwrite
		("depth_idx", &CommonSEGD::GeneralHeaderExp::depth_idx);
	GeneralHeaderExp_py.def_readwrite
		("offset_crossline", &CommonSEGD::GeneralHeaderExp::offset_crossline);
	GeneralHeaderExp_py.def_readwrite
		("offset_inline", &CommonSEGD::GeneralHeaderExp::offset_inline);
	GeneralHeaderExp_py.def_readwrite
		("size", &CommonSEGD::GeneralHeaderExp::size);
	GeneralHeaderExp_py.def_readwrite
		("offset_depth", &CommonSEGD::GeneralHeaderExp::offset_depth);
	GeneralHeaderExp_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderExp::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderExp::Name>(GeneralHeaderExp_py, "Name");
	GeneralHeaderExp_py.def("name_as_string",
  							&CommonSEGD::GeneralHeaderExp::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderAir,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderAir_py(CommonSEGD_py, "GeneralHeaderAir");
	GeneralHeaderAir_py.def_readwrite
		("expanded_file_number",
		 &CommonSEGD::GeneralHeaderAir::expanded_file_number);
	GeneralHeaderAir_py.def_readwrite
		("sou_line_num", &CommonSEGD::GeneralHeaderAir::sou_line_num);
	GeneralHeaderAir_py.def_readwrite
		("sou_point_num", &CommonSEGD::GeneralHeaderAir::sou_point_num);
	GeneralHeaderAir_py.def_readwrite
		("sou_point_index", &CommonSEGD::GeneralHeaderAir::sou_point_index);
	GeneralHeaderAir_py.def_readwrite
		("depth", &CommonSEGD::GeneralHeaderAir::depth);
	GeneralHeaderAir_py.def_readwrite
		("air_pressure", &CommonSEGD::GeneralHeaderAir::air_pressure);
	GeneralHeaderAir_py.def_readwrite
		("source_id", &CommonSEGD::GeneralHeaderAir::source_id);
	GeneralHeaderAir_py.def_readwrite
		("source_set_num", &CommonSEGD::GeneralHeaderAir::source_set_num);
	GeneralHeaderAir_py.def_readwrite
		("reshoot_idx", &CommonSEGD::GeneralHeaderAir::reshoot_idx);
	GeneralHeaderAir_py.def_readwrite
		("group_idx", &CommonSEGD::GeneralHeaderAir::group_idx);
	GeneralHeaderAir_py.def_readwrite
		("depth_idx", &CommonSEGD::GeneralHeaderAir::depth_idx);
	GeneralHeaderAir_py.def_readwrite
		("offset_crossline", &CommonSEGD::GeneralHeaderAir::offset_crossline);
	GeneralHeaderAir_py.def_readwrite
		("offset_inline", &CommonSEGD::GeneralHeaderAir::offset_inline);
	GeneralHeaderAir_py.def_readwrite
		("size", &CommonSEGD::GeneralHeaderAir::size);
	GeneralHeaderAir_py.def_readwrite
		("offset_depth", &CommonSEGD::GeneralHeaderAir::offset_depth);
	GeneralHeaderAir_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderAir::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderAir::Name>(GeneralHeaderAir_py, "Name");
	GeneralHeaderAir_py.def("name_as_string",
  							&CommonSEGD::GeneralHeaderAir::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderWat,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderWat_py(CommonSEGD_py, "GeneralHeaderWat");
	GeneralHeaderWat_py.def_readwrite
		("expanded_file_number",
		 &CommonSEGD::GeneralHeaderWat::expanded_file_number);
	GeneralHeaderWat_py.def_readwrite
		("sou_line_num", &CommonSEGD::GeneralHeaderWat::sou_line_num);
	GeneralHeaderWat_py.def_readwrite
		("sou_point_num", &CommonSEGD::GeneralHeaderWat::sou_point_num);
	GeneralHeaderWat_py.def_readwrite
		("sou_point_index", &CommonSEGD::GeneralHeaderWat::sou_point_index);
	GeneralHeaderWat_py.def_readwrite
		("depth", &CommonSEGD::GeneralHeaderWat::depth);
	GeneralHeaderWat_py.def_readwrite
		("air_pressure", &CommonSEGD::GeneralHeaderWat::air_pressure);
	GeneralHeaderWat_py.def_readwrite
		("source_id", &CommonSEGD::GeneralHeaderWat::source_id);
	GeneralHeaderWat_py.def_readwrite
		("source_set_num", &CommonSEGD::GeneralHeaderWat::source_set_num);
	GeneralHeaderWat_py.def_readwrite
		("reshoot_idx", &CommonSEGD::GeneralHeaderWat::reshoot_idx);
	GeneralHeaderWat_py.def_readwrite
		("group_idx", &CommonSEGD::GeneralHeaderWat::group_idx);
	GeneralHeaderWat_py.def_readwrite
		("depth_idx", &CommonSEGD::GeneralHeaderWat::depth_idx);
	GeneralHeaderWat_py.def_readwrite
		("offset_crossline", &CommonSEGD::GeneralHeaderWat::offset_crossline);
	GeneralHeaderWat_py.def_readwrite
		("offset_inline", &CommonSEGD::GeneralHeaderWat::offset_inline);
	GeneralHeaderWat_py.def_readwrite
		("size", &CommonSEGD::GeneralHeaderWat::size);
	GeneralHeaderWat_py.def_readwrite
		("offset_depth", &CommonSEGD::GeneralHeaderWat::offset_depth);
	GeneralHeaderWat_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderWat::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderWat::Name>(GeneralHeaderWat_py, "Name");
	GeneralHeaderWat_py.def("name_as_string",
  							&CommonSEGD::GeneralHeaderWat::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderEle,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderEle_py(CommonSEGD_py, "GeneralHeaderEle");
	GeneralHeaderEle_py.def_readwrite
		("expanded_file_number",
		 &CommonSEGD::GeneralHeaderEle::expanded_file_number);
	GeneralHeaderEle_py.def_readwrite
		("sou_line_num", &CommonSEGD::GeneralHeaderEle::sou_line_num);
	GeneralHeaderEle_py.def_readwrite
		("sou_point_num", &CommonSEGD::GeneralHeaderEle::sou_point_num);
	GeneralHeaderEle_py.def_readwrite
		("sou_point_index", &CommonSEGD::GeneralHeaderEle::sou_point_index);
	GeneralHeaderEle_py.def_readwrite
		("source_type", &CommonSEGD::GeneralHeaderEle::source_type);
	GeneralHeaderEle_py.def_readwrite
		("moment", &CommonSEGD::GeneralHeaderEle::moment);
	GeneralHeaderEle_py.def_readwrite
		("source_id", &CommonSEGD::GeneralHeaderEle::source_id);
	GeneralHeaderEle_py.def_readwrite
		("source_set_num", &CommonSEGD::GeneralHeaderEle::source_set_num);
	GeneralHeaderEle_py.def_readwrite
		("reshoot_idx", &CommonSEGD::GeneralHeaderEle::reshoot_idx);
	GeneralHeaderEle_py.def_readwrite
		("group_idx", &CommonSEGD::GeneralHeaderEle::depth_idx);
	GeneralHeaderEle_py.def_readwrite
		("offset_crossline", &CommonSEGD::GeneralHeaderEle::offset_inline);
	GeneralHeaderEle_py.def_readwrite
		("offset_inline", &CommonSEGD::GeneralHeaderEle::offset_inline);
	GeneralHeaderEle_py.def_readwrite
		("size", &CommonSEGD::GeneralHeaderEle::size);
	GeneralHeaderEle_py.def_readwrite
		("offset_depth", &CommonSEGD::GeneralHeaderEle::offset_depth);
	GeneralHeaderEle_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderEle::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderEle::Name>(GeneralHeaderEle_py, "Name");
	GeneralHeaderEle_py.def("name_as_string",
  							&CommonSEGD::GeneralHeaderEle::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderOth,
	   	CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderOth_py(CommonSEGD_py, "GeneralHeaderOth");
	GeneralHeaderOth_py.def_readwrite
		("expanded_file_number",
		 &CommonSEGD::GeneralHeaderOth::expanded_file_number);
	GeneralHeaderOth_py.def_readwrite
		("sou_line_num", &CommonSEGD::GeneralHeaderOth::sou_line_num);
	GeneralHeaderOth_py.def_readwrite
		("sou_point_num", &CommonSEGD::GeneralHeaderOth::sou_point_num);
	GeneralHeaderOth_py.def_readwrite
		("sou_point_index", &CommonSEGD::GeneralHeaderOth::sou_point_index);
	GeneralHeaderOth_py.def_readwrite
		("source_id", &CommonSEGD::GeneralHeaderOth::source_id);
	GeneralHeaderOth_py.def_readwrite
		("source_set_num", &CommonSEGD::GeneralHeaderOth::source_set_num);
	GeneralHeaderOth_py.def_readwrite
		("reshoot_idx", &CommonSEGD::GeneralHeaderOth::reshoot_idx);
	GeneralHeaderOth_py.def_readwrite
		("group_idx", &CommonSEGD::GeneralHeaderOth::group_idx);
	GeneralHeaderOth_py.def_readwrite
		("depth_idx", &CommonSEGD::GeneralHeaderOth::depth_idx);
	GeneralHeaderOth_py.def_readwrite
		("offset_crossline", &CommonSEGD::GeneralHeaderOth::offset_crossline);
	GeneralHeaderOth_py.def_readwrite
		("offset_inline", &CommonSEGD::GeneralHeaderOth::offset_inline);
	GeneralHeaderOth_py.def_readwrite
		("size", &CommonSEGD::GeneralHeaderOth::size);
	GeneralHeaderOth_py.def_readwrite
		("offset_depth", &CommonSEGD::GeneralHeaderOth::offset_depth);
	GeneralHeaderOth_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderOth::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderOth::Name>(GeneralHeaderOth_py, "Name");
	GeneralHeaderOth_py.def("name_as_string",
  							&CommonSEGD::GeneralHeaderOth::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderAdd,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderAdd_py(CommonSEGD_py, "GeneralHeaderAdd");
	GeneralHeaderAdd_py.def_readwrite
		("time", &CommonSEGD::GeneralHeaderAdd::time);
	GeneralHeaderAdd_py.def_readwrite
		("source_status", &CommonSEGD::GeneralHeaderAdd::source_status);
	GeneralHeaderAdd_py.def_readwrite
		("source_id", &CommonSEGD::GeneralHeaderAdd::source_id);
	GeneralHeaderAdd_py.def_readwrite
		("source_moving", &CommonSEGD::GeneralHeaderAdd::source_moving);
	GeneralHeaderAdd_py.def_property
		("error_description",
		 [] (CommonSEGD::GeneralHeaderAdd &h)
		 { return string(h.error_description, 20); },
		 [] (CommonSEGD::GeneralHeaderAdd &h, string &s)
		 { strncpy(h.error_description, s.c_str(), 20); });
	GeneralHeaderAdd_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderAdd::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderAdd::Name>(GeneralHeaderAdd_py, "Name");
	GeneralHeaderAdd_py.def("name_as_string",
  							&CommonSEGD::GeneralHeaderAdd::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderSaux,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderSaux_py(CommonSEGD_py, "GeneralHeaderSaux");
	GeneralHeaderSaux_py.def_readwrite
		("source_id", &CommonSEGD::GeneralHeaderSaux::source_id);
	GeneralHeaderSaux_py.def_readwrite
		("scan_type_num_1", &CommonSEGD::GeneralHeaderSaux::scan_type_num_1);
	GeneralHeaderSaux_py.def_readwrite
		("ch_set_num_1", &CommonSEGD::GeneralHeaderSaux::ch_set_num_1);
	GeneralHeaderSaux_py.def_readwrite
		("trace_num_1", &CommonSEGD::GeneralHeaderSaux::trace_num_1);
	GeneralHeaderSaux_py.def_readwrite
		("scan_type_num_2", &CommonSEGD::GeneralHeaderSaux::scan_type_num_2);
	GeneralHeaderSaux_py.def_readwrite
		("ch_set_num_2", &CommonSEGD::GeneralHeaderSaux::ch_set_num_2);
	GeneralHeaderSaux_py.def_readwrite
		("trace_num_2", &CommonSEGD::GeneralHeaderSaux::trace_num_2);
	GeneralHeaderSaux_py.def_readwrite
		("scan_type_num_3", &CommonSEGD::GeneralHeaderSaux::scan_type_num_3);
	GeneralHeaderSaux_py.def_readwrite
		("ch_set_num_3", &CommonSEGD::GeneralHeaderSaux::ch_set_num_3);
	GeneralHeaderSaux_py.def_readwrite
		("trace_num_3", &CommonSEGD::GeneralHeaderSaux::trace_num_3);
	GeneralHeaderSaux_py.def_readwrite
		("scan_type_num_4", &CommonSEGD::GeneralHeaderSaux::scan_type_num_4);
	GeneralHeaderSaux_py.def_readwrite
		("ch_set_num_4", &CommonSEGD::GeneralHeaderSaux::ch_set_num_4);
	GeneralHeaderSaux_py.def_readwrite
		("trace_num_4", &CommonSEGD::GeneralHeaderSaux::trace_num_4);
	GeneralHeaderSaux_py.def_readwrite
		("scan_type_num_5", &CommonSEGD::GeneralHeaderSaux::scan_type_num_5);
	GeneralHeaderSaux_py.def_readwrite
		("ch_set_num_5", &CommonSEGD::GeneralHeaderSaux::ch_set_num_5);
	GeneralHeaderSaux_py.def_readwrite
		("trace_num_5", &CommonSEGD::GeneralHeaderSaux::trace_num_5);
	GeneralHeaderSaux_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderSaux::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderSaux::Name>(GeneralHeaderSaux_py,
												   "Name");
	GeneralHeaderSaux_py.def("name_as_string",
 							 &CommonSEGD::GeneralHeaderSaux::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderCoord,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderCoord_py(CommonSEGD_py, "GeneralHeaderCoord");
	GeneralHeaderCoord_py.def_property
		("crs",
		 [] (CommonSEGD::GeneralHeaderCoord &h)
		 { return string(h.crs, 31); },
		 [] (CommonSEGD::GeneralHeaderCoord &h, string &s)
		 { strncpy(h.crs, s.c_str(), 31); });
	GeneralHeaderCoord_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderCoord::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderCoord::Name>(GeneralHeaderCoord_py,
												   "Name");
	GeneralHeaderCoord_py.def("name_as_string",
  							  &CommonSEGD::GeneralHeaderCoord::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderPos1,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderPos1_py(CommonSEGD_py, "GeneralHeaderPos1");
	GeneralHeaderPos1_py.def_readwrite
		("time_of_position", &CommonSEGD::GeneralHeaderPos1::time_of_position);
	GeneralHeaderPos1_py.def_readwrite
		("time_of_measurement",
		 &CommonSEGD::GeneralHeaderPos1::time_of_measurement);
	GeneralHeaderPos1_py.def_readwrite
		("vert_error", &CommonSEGD::GeneralHeaderPos1::vert_error);
	GeneralHeaderPos1_py.def_readwrite
		("hor_error_semi_major",
		 &CommonSEGD::GeneralHeaderPos1::hor_error_semi_major);
	GeneralHeaderPos1_py.def_readwrite
		("hor_error_semi_minor",
		 &CommonSEGD::GeneralHeaderPos1::hor_error_semi_minor);
	GeneralHeaderPos1_py.def_readwrite
		("hor_error_orientation",
		 &CommonSEGD::GeneralHeaderPos1::hor_error_orientation);
	GeneralHeaderPos1_py.def_readwrite
		("position_type", &CommonSEGD::GeneralHeaderPos1::position_type);
	GeneralHeaderPos1_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderPos1::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderPos1::Name>(GeneralHeaderPos1_py,
												   "Name");
	GeneralHeaderPos1_py.def("name_as_string",
 							 &CommonSEGD::GeneralHeaderPos1::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderPos2,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderPos2_py(CommonSEGD_py, "GeneralHeaderPos2");
	GeneralHeaderPos2_py.def_readwrite
		("crs_a_coord1", &CommonSEGD::GeneralHeaderPos2::crs_a_coord1);
	GeneralHeaderPos2_py.def_readwrite
		("crs_a_coord2", &CommonSEGD::GeneralHeaderPos2::crs_a_coord2);
	GeneralHeaderPos2_py.def_readwrite
		("crs_a_coord3", &CommonSEGD::GeneralHeaderPos2::crs_a_coord3);
	GeneralHeaderPos2_py.def_readwrite
		("crs_a_crsref", &CommonSEGD::GeneralHeaderPos2::crs_a_crsref);
	GeneralHeaderPos2_py.def_readwrite
		("pos1_valid", &CommonSEGD::GeneralHeaderPos2::pos1_valid);
	GeneralHeaderPos2_py.def_readwrite
		("pos1_quality", &CommonSEGD::GeneralHeaderPos2::pos1_quality);
	GeneralHeaderPos2_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderPos2::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderPos2::Name>(GeneralHeaderPos2_py,
												   "Name");
	GeneralHeaderPos2_py.def("name_as_string",
 							 &CommonSEGD::GeneralHeaderPos2::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderPos3,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderPos3_py(CommonSEGD_py, "GeneralHeaderPos3");
	GeneralHeaderPos3_py.def_readwrite
		("crs_b_coord1", &CommonSEGD::GeneralHeaderPos3::crs_b_coord1);
	GeneralHeaderPos3_py.def_readwrite
		("crs_b_coord2", &CommonSEGD::GeneralHeaderPos3::crs_b_coord2);
	GeneralHeaderPos3_py.def_readwrite
		("crs_b_coord3", &CommonSEGD::GeneralHeaderPos3::crs_b_coord3);
	GeneralHeaderPos3_py.def_readwrite
		("crs_b_crsref", &CommonSEGD::GeneralHeaderPos3::crs_b_crsref);
	GeneralHeaderPos3_py.def_readwrite
		("pos2_valid", &CommonSEGD::GeneralHeaderPos3::pos2_valid);
	GeneralHeaderPos3_py.def_readwrite
		("pos2_quality", &CommonSEGD::GeneralHeaderPos3::pos2_quality);
	GeneralHeaderPos3_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderPos3::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderPos3::Name>(GeneralHeaderPos3_py,
												   "Name");
	GeneralHeaderPos3_py.def("name_as_string",
 							 &CommonSEGD::GeneralHeaderPos3::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderRel,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderRel_py(CommonSEGD_py, "GeneralHeaderRel");
	GeneralHeaderRel_py.def_readwrite
		("offset_east", &CommonSEGD::GeneralHeaderRel::offset_east);
	GeneralHeaderRel_py.def_readwrite
		("offset_north", &CommonSEGD::GeneralHeaderRel::offset_north);
	GeneralHeaderRel_py.def_readwrite
		("offset_vert", &CommonSEGD::GeneralHeaderRel::offset_vert);
	GeneralHeaderRel_py.def_property
		("description",
		 [] (CommonSEGD::GeneralHeaderRel &h)
		 { return string(h.description, 19); },
		 [] (CommonSEGD::GeneralHeaderRel &h, string &s)
		 { strncpy(h.description, s.c_str(), 19); });
	GeneralHeaderRel_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderRel::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderRel::Name>(GeneralHeaderRel_py,
												  "Name");
	GeneralHeaderRel_py.def("name_as_string",
							&CommonSEGD::GeneralHeaderRel::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderSen,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderSen_py(CommonSEGD_py, "GeneralHeaderSen");
	GeneralHeaderSen_py.def_readwrite
		("instrument_test_time",
		 &CommonSEGD::GeneralHeaderSen::instrument_test_time);
	GeneralHeaderSen_py.def_readwrite
		("sensor_sensitivity",
		 &CommonSEGD::GeneralHeaderSen::sensor_sensitivity);
	GeneralHeaderSen_py.def_readwrite
		("instr_test_result",
		 &CommonSEGD::GeneralHeaderSen::instr_test_result);
	GeneralHeaderSen_py.def_property
		("serial_number",
		 [] (CommonSEGD::GeneralHeaderSen &h)
		 { return string(h.serial_number, 28); },
		 [] (CommonSEGD::GeneralHeaderSen &h, string &s)
		 { strncpy(h.serial_number, s.c_str(), 28); });
	GeneralHeaderSen_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderSen::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderSen::Name>(GeneralHeaderSen_py,
												  "Name");
	GeneralHeaderSen_py.def("name_as_string",
							&CommonSEGD::GeneralHeaderSen::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderSCa,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderSCa_py(CommonSEGD_py, "GeneralHeaderSCa");
	GeneralHeaderSCa_py.def_readwrite
		("freq1", &CommonSEGD::GeneralHeaderSCa::freq1);
	GeneralHeaderSCa_py.def_readwrite
		("amp1", &CommonSEGD::GeneralHeaderSCa::amp1);
	GeneralHeaderSCa_py.def_readwrite
		("phase1", &CommonSEGD::GeneralHeaderSCa::phase1);
	GeneralHeaderSCa_py.def_readwrite
		("freq2", &CommonSEGD::GeneralHeaderSCa::freq1);
	GeneralHeaderSCa_py.def_readwrite
		("amp2", &CommonSEGD::GeneralHeaderSCa::amp1);
	GeneralHeaderSCa_py.def_readwrite
		("phase2", &CommonSEGD::GeneralHeaderSCa::phase1);
	GeneralHeaderSCa_py.def_readwrite
		("calib_applied", &CommonSEGD::GeneralHeaderSCa::calib_applied);
	GeneralHeaderSCa_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderSCa::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderSCa::Name>(GeneralHeaderSCa_py,
												  "Name");
	GeneralHeaderSCa_py.def("name_as_string",
							&CommonSEGD::GeneralHeaderSCa::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderTim,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderTim_py(CommonSEGD_py, "GeneralHeaderTim");
	GeneralHeaderTim_py.def_readwrite
		("time_of_depl", &CommonSEGD::GeneralHeaderTim::time_of_depl);
	GeneralHeaderTim_py.def_readwrite
		("time_of_retr", &CommonSEGD::GeneralHeaderTim::time_of_retr);
	GeneralHeaderTim_py.def_readwrite
		("timer_offset_depl",
		 &CommonSEGD::GeneralHeaderTim::timer_offset_depl);
	GeneralHeaderTim_py.def_readwrite
		("time_offset_retr",
		 &CommonSEGD::GeneralHeaderTim::time_offset_retr);
	GeneralHeaderTim_py.def_readwrite
		("timedrift_corr", &CommonSEGD::GeneralHeaderTim::timedrift_corr);
	GeneralHeaderTim_py.def_readwrite
		("corr_method", &CommonSEGD::GeneralHeaderTim::corr_method);
	py::enum_<CommonSEGD::GeneralHeaderTim::Name>(GeneralHeaderTim_py,
												  "Name");
	GeneralHeaderTim_py.def("name_as_string",
							&CommonSEGD::GeneralHeaderTim::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderElSR,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderElSR_py(CommonSEGD_py, "GeneralHeaderElSR");
	GeneralHeaderElSR_py.def_readwrite
		("equip_dim_x", &CommonSEGD::GeneralHeaderElSR::equip_dim_x);
	GeneralHeaderElSR_py.def_readwrite
		("equip_dim_y", &CommonSEGD::GeneralHeaderElSR::equip_dim_y);
	GeneralHeaderElSR_py.def_readwrite
		("equip_dim_z", &CommonSEGD::GeneralHeaderElSR::equip_dim_z);
	GeneralHeaderElSR_py.def_readwrite
		("pos_term", &CommonSEGD::GeneralHeaderElSR::pos_term);
	GeneralHeaderElSR_py.def_readwrite
		("equip_offset_x", &CommonSEGD::GeneralHeaderElSR::equip_offset_x);
	GeneralHeaderElSR_py.def_readwrite
		("equip_offset_y", &CommonSEGD::GeneralHeaderElSR::equip_offset_y);
	GeneralHeaderElSR_py.def_readwrite
		("equip_offset_z", &CommonSEGD::GeneralHeaderElSR::equip_offset_z);
	GeneralHeaderElSR_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderElSR::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderElSR::Name>(GeneralHeaderElSR_py,
 												   "Name");
	GeneralHeaderElSR_py.def("name_as_string",
 							 &CommonSEGD::GeneralHeaderElSR::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderOri,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderOri_py(CommonSEGD_py, "GeneralHeaderOri");
	GeneralHeaderOri_py.def_readwrite
		("rot_x", &CommonSEGD::GeneralHeaderOri::rot_x);
	GeneralHeaderOri_py.def_readwrite
		("rot_y", &CommonSEGD::GeneralHeaderOri::rot_y);
	GeneralHeaderOri_py.def_readwrite
		("rot_z", &CommonSEGD::GeneralHeaderOri::rot_z);
	GeneralHeaderOri_py.def_readwrite
		("ref_orientation", &CommonSEGD::GeneralHeaderOri::ref_orientation);
	GeneralHeaderOri_py.def_readwrite
		("time_stamp", &CommonSEGD::GeneralHeaderOri::time_stamp);
	GeneralHeaderOri_py.def_readwrite
		("ori_type", &CommonSEGD::GeneralHeaderOri::ori_type);
	GeneralHeaderOri_py.def_readwrite
		("ref_orient_valid", &CommonSEGD::GeneralHeaderOri::ref_orient_valid);
	GeneralHeaderOri_py.def_readwrite
		("rot_applied", &CommonSEGD::GeneralHeaderOri::rot_applied);
	GeneralHeaderOri_py.def_readwrite
		("rot_north_applied",
		 &CommonSEGD::GeneralHeaderOri::rot_north_applied);
	GeneralHeaderOri_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderOri::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderOri::Name>(GeneralHeaderOri_py,
												  "Name");
	GeneralHeaderOri_py.def("name_as_string",
 							 &CommonSEGD::GeneralHeaderOri::name_as_string);

	py::class_<CommonSEGD::GeneralHeaderMeas,
		CommonSEGD::AdditionalGeneralHeader>
			GeneralHeaderMeas_py(CommonSEGD_py, "GeneralHeaderMeas");
	GeneralHeaderMeas_py.def_readwrite
		("timestamp", &CommonSEGD::GeneralHeaderMeas::timestamp);
	GeneralHeaderMeas_py.def_readwrite
		("measurement_value",
		 &CommonSEGD::GeneralHeaderMeas::measurement_value);
	GeneralHeaderMeas_py.def_readwrite
		("maximum_value", &CommonSEGD::GeneralHeaderMeas::maximum_value);
	GeneralHeaderMeas_py.def_readwrite
		("minimum_value", &CommonSEGD::GeneralHeaderMeas::minimum_value);
	GeneralHeaderMeas_py.def_readwrite
		("quantity_class", &CommonSEGD::GeneralHeaderMeas::quantity_class);
	GeneralHeaderMeas_py.def_readwrite
		("unit_of_measure", &CommonSEGD::GeneralHeaderMeas::unit_of_measure);
	GeneralHeaderMeas_py.def_readwrite
		("gen_hdr_block_type",
		 &CommonSEGD::GeneralHeaderMeas::gen_hdr_block_type);
	py::enum_<CommonSEGD::GeneralHeaderMeas::Name>(GeneralHeaderMeas_py,
 												   "Name");
	GeneralHeaderMeas_py.def("name_as_string",
 							 &CommonSEGD::GeneralHeaderMeas::name_as_string);

	py::class_<CommonSEGD::ChannelSetHeader>
		ChannelSetHeader_py (CommonSEGD_py, "ChannelSetHeader");
	ChannelSetHeader_py.def_readwrite
		("scan_type_number", &CommonSEGD::ChannelSetHeader::scan_type_number);
	ChannelSetHeader_py.def_readwrite
		("channel_set_number",
		 &CommonSEGD::ChannelSetHeader::channel_set_number);
	ChannelSetHeader_py.def_readwrite
		("channel_type", &CommonSEGD::ChannelSetHeader::channel_type);
	ChannelSetHeader_py.def_readwrite
		("channel_set_start_time",
		 &CommonSEGD::ChannelSetHeader::channel_set_start_time);
	ChannelSetHeader_py.def_readwrite
		("channel_set_end_time",
		 &CommonSEGD::ChannelSetHeader::channel_set_end_time);
	ChannelSetHeader_py.def_readwrite
		("descale_multiplier",
		 &CommonSEGD::ChannelSetHeader::descale_multiplier);
	ChannelSetHeader_py.def_readwrite
		("number_of_channels",
		 &CommonSEGD::ChannelSetHeader::number_of_channels);
	ChannelSetHeader_py.def_readwrite
		("subscans_per_ch_set",
		 &CommonSEGD::ChannelSetHeader::subscans_per_ch_set);
	ChannelSetHeader_py.def_readwrite
		("channel_gain", &CommonSEGD::ChannelSetHeader::channel_gain);
	ChannelSetHeader_py.def_readwrite
		("alias_filter_freq",
		 &CommonSEGD::ChannelSetHeader::alias_filter_freq);
	ChannelSetHeader_py.def_readwrite
		("alias_filter_slope",
		 &CommonSEGD::ChannelSetHeader::alias_filter_slope);
	ChannelSetHeader_py.def_readwrite
		("low_cut_filter_freq",
		 &CommonSEGD::ChannelSetHeader::low_cut_filter_freq);
	ChannelSetHeader_py.def_readwrite
		("low_cut_filter_slope",
		 &CommonSEGD::ChannelSetHeader::low_cut_filter_slope);
	ChannelSetHeader_py.def_readwrite
		("first_notch_filter",
		 &CommonSEGD::ChannelSetHeader::first_notch_filter);
	ChannelSetHeader_py.def_readwrite
		("second_notch_filter",
		 &CommonSEGD::ChannelSetHeader::second_notch_filter);
	ChannelSetHeader_py.def_readwrite
		("third_notch_filter",
		 &CommonSEGD::ChannelSetHeader::third_notch_filter);
	ChannelSetHeader_py.def_readwrite
		("ext_ch_set_num", &CommonSEGD::ChannelSetHeader::ext_ch_set_num);
	ChannelSetHeader_py.def_readwrite
		("trc_hdr_ext", &CommonSEGD::ChannelSetHeader::trc_hdr_ext);
	ChannelSetHeader_py.def_readwrite
		("vert_stack", &CommonSEGD::ChannelSetHeader::vert_stack);
	ChannelSetHeader_py.def_readwrite
		("streamer_no", &CommonSEGD::ChannelSetHeader::streamer_no);
	ChannelSetHeader_py.def_readwrite
		("array_forming", &CommonSEGD::ChannelSetHeader::array_forming);
	ChannelSetHeader_py.def
		("number_of_samples",
		 &CommonSEGD::ChannelSetHeader::number_of_samples);
	ChannelSetHeader_py.def
		("samp_int", &CommonSEGD::ChannelSetHeader::samp_int);
	ChannelSetHeader_py.def
		("filter_phase", &CommonSEGD::ChannelSetHeader::filter_phase);
	ChannelSetHeader_py.def
		("filter_delay", &CommonSEGD::ChannelSetHeader::filter_delay);
	ChannelSetHeader_py.def
		("description", &CommonSEGD::ChannelSetHeader::description);
	py::enum_<CommonSEGD::ChannelSetHeader::Name>(ChannelSetHeader_py,
												  "Name");
	ChannelSetHeader_py.def("name_as_string",
							&CommonSEGD::ChannelSetHeader::name_as_string);

	py::class_<ISEGD> ISEGD_py(m, "ISEGD");
	ISEGD_py.def(py::init<string>());
	ISEGD_py.def("general_header", &ISEGD::general_header,
				 "Returns general header");
	ISEGD_py.def("general_header2", &ISEGD::general_header2,
				 "Returns general header 2 if there is one.");
	ISEGD_py.def("general_headerN", &ISEGD::general_headerN,
				 "Returns list with genreral headers N.");
	ISEGD_py.def("general_header3", &ISEGD::general_header3,
				 "Returns general header 3 if there is one.");
	ISEGD_py.def("general_header_vessel_crew_id",
				 &ISEGD::general_header_vessel_crew_id,
				 "Returns general header vessel crew id.");
	ISEGD_py.def("general_header_survey_name",
				 &ISEGD::general_header_survey_name,
				 "Returns general header survey survey name.");
	ISEGD_py.def("general_header_client_name",
				 &ISEGD::general_header_survey_name,
				 "Returns general header client name.");
	ISEGD_py.def("general_header_job_id", &ISEGD::general_header_job_id,
				 "Returns general header job id.");
	ISEGD_py.def("general_header_line_id", &ISEGD::general_header_line_id,
				 "Returns general header line id.");
	ISEGD_py.def("general_header_vibrator_info",
				 &ISEGD::general_header_vibrator_info,
				 "Returns general header vibrator source info.");
	ISEGD_py.def("general_header_explosive_info",
				 &ISEGD::general_header_airgun_info,
				 "Returns general header explosive source info.");
	ISEGD_py.def("general_header_airgun_info",
				 &ISEGD::general_header_airgun_info,
				 "Returns general header airgun source info.");
	ISEGD_py.def("general_header_watergun_info",
				 &ISEGD::general_header_watergun_info,
				 "Returns general header watergun source info.");
	ISEGD_py.def("general_header_electromagnetic_info",
				 &ISEGD::general_header_electromagnetic_info,
				 "Returns general header electromagnetic source info.");
	ISEGD_py.def("general_header_other_source_info",
				 &ISEGD::general_header_other_source_info,
				 "Returns general header other source type info.");
	ISEGD_py.def("general_header_add_source_info",
				 &ISEGD::general_header_add_source_info,
				 "Returns general header additional source type info.");
	ISEGD_py.def("general_header_sou_aux_chan_ref",
				 &ISEGD::general_header_sou_aux_chan_ref,
				 "Returns general header source auxiliary channel reference.");
	ISEGD_py.def("general_header_sen_info_hdr_ext_blk",
				 &ISEGD::general_header_sen_info_hdr_ext_blk,
				 "Returns general header sensor info extension block.");
	ISEGD_py.def("general_header_sen_calib_blk",
				 &ISEGD::general_header_sen_calib_blk,
				 "Returns general header sensor calibration block.");
	ISEGD_py.def("general_header_time_drift_blk",
				 &ISEGD::general_header_time_drift_blk,
				 "Returns general header time frift block.");
	ISEGD_py.def("general_header_elemag_src_rec_desc_blk",
				 &ISEGD::general_header_elemag_src_rec_desc_blk,
				 "Returns general header electromagnetic source receiver "
				 "description block.");
	ISEGD_py.def("general_header_position_blk_1",
				 &ISEGD::general_header_position_blk_1,
				 "Returns general header position block 1.");
	ISEGD_py.def("general_header_position_blk_2",
				 &ISEGD::general_header_position_blk_2,
				 "Returns general header position block 2.");
	ISEGD_py.def("general_header_position_blk_3",
				 &ISEGD::general_header_position_blk_3,
				 "Returns general header position block 3.");
	ISEGD_py.def("general_header_coord_ref_blk",
				 &ISEGD::general_header_coord_ref_blk,
				 "Returns general header coordinate reference system block.");
	ISEGD_py.def("general_header_relative_pos_blk",
				 &ISEGD::general_header_relative_pos_blk,
				 "Returns general header relative position block.");
	ISEGD_py.def("general_header_orient_hdr_blk",
				 &ISEGD::general_header_orient_hdr_blk,
				 "Returns general header orientation header block.");
	ISEGD_py.def("general_header_measurement_blk",
				 &ISEGD::general_header_measurement_blk,
				 "Returns general header measurement block.");
	ISEGD_py.def("channel_set_headers", &ISEGD::channel_set_headers,
				 "Returns list of lists of channel set headers.");
	ISEGD_py.def("has_record", &ISEGD::has_record,
				 "Returns true if there if something to read.");
	ISEGD_py.def("has_trace", &ISEGD::has_trace,
				 "Returns true if there are traces to read in current record");
	ISEGD_py.def("read_trace", &ISEGD::read_trace,
				 "Returns trace");
	ISEGD_py.def("__next__", [] (ISEGD &s)
				 { return s.has_record() ? s.read_trace() :
					 throw py::stop_iteration(); });
	ISEGD_py.def("__iter__", [] (ISEGD &s) { return &s; });

	py::class_<OSEGD> OSEGD_py(m, "OSEGD");
	OSEGD_py.def("write_trace", &OSEGD::write_trace);

	py::class_<OSEGDRev2_1, OSEGD> OSEGDRev2_1_py(m, "OSEGDRev2_1");
	OSEGDRev2_1_py.def
		(py::init<string, CommonSEGD::GeneralHeader,
		 CommonSEGD::GeneralHeader2,
		 vector<vector<CommonSEGD::ChannelSetHeader>>,
		 vector<shared_ptr<CommonSEGD::AdditionalGeneralHeader>>>(),
		 py::arg("file_name"), py::arg("gh"), py::arg("gh2"),
		 py::arg("ch_sets"), py::arg("add_ghs") =
		 vector<shared_ptr<CommonSEGD::AdditionalGeneralHeader>>());
}
}
