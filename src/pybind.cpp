#include "ISEGY.hpp"
#include "OSEGY.hpp"
#include "OSEGYRev0.hpp"
#include "OSEGYRev1.hpp"
#include "OSEGYRev2.hpp"
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

PYBIND11_MODULE(pysedaman, m) {
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

	py::class_<CommonSEGY::BinaryHeader> BinaryHeader_py(CommonSEGY_py,
														 "BinaryHeader");
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
	py::enum_<CommonSEGY::TrHdrValueType>(CommonSEGY_py, "TrHdrValueType");

	py::class_<ISEGY> ISEGY_py(m, "ISEGY");
	ISEGY_py.def(py::init<string,
				 vector<map<uint32_t,
				 pair<string, CommonSEGY::TrHdrValueType>>>,
				 vector<pair<string, map<uint32_t,
				 pair<string, CommonSEGY::TrHdrValueType>>>>>(),
				 py::arg("file_name"), py::arg("tr_hdr_over") =
				 vector<map<uint32_t,
				 pair<string, CommonSEGY::TrHdrValueType>>>(),
				 py::arg("add_hdr_map") = vector<pair<string,
				 map<uint32_t, pair<string, CommonSEGY::TrHdrValueType>>>>());
	ISEGY_py.def("read_binary_header", &ISEGY::read_binary_header,
				 "creates ISegy instance internally and "
				 "returns binary header.");
	ISEGY_py.def("text_headers", &ISEGY::text_headers,
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

	py::class_<OSEGY> OSEGY_py(m, "OSEGY");
	OSEGY_py.def("write_trace", &OSEGY::write_trace,
				 "Writes trace to the end of file.",
				 py::arg("trace"));

	py::class_<OSEGYRev0, OSEGY> OSEGYRev0_py(m, "OSEGYRev0");
	OSEGYRev0_py.def(py::init<string, string, CommonSEGY::BinaryHeader>(),
					 py::arg("file_name"), py::arg("text_header") = string(),
					 py::arg("bin_header") = CommonSEGY::BinaryHeader());

	py::class_<Trace> Trace_py(m, "Trace");
	Trace_py.def(py::init<unordered_map<string, Trace::Header::Value>,
				 vector<double>>());
	Trace_py.def("header", &Trace::header, "Returns trace header");
	Trace_py.def("samples", &Trace::samples, "Returns trace samples");
	Trace_py.def("samples_as_numpy_array",
				 [](Trace &t) { return py::array_t<double>(t.samples()); },
				 "Returns trace samples as numpy array");

	py::class_<Trace::Header> Header_py(Trace_py, "Header");
	Header_py.def(py::init<unordered_map<string, Trace::Header::Value>>());
	Header_py.def("get", &Trace::Header::get,
				  "Gets header value by specified key");
	Header_py.def("set", &Trace::Header::set,
				  "Sets or adds header value by specified key",
				  py::arg("key"), py::arg("v"));
	Header_py.def("keys", &Trace::Header::keys, "Retruns all keys for header");
}
}
