#include "ISEGY.hpp"
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
	CommonSEGY_py.def_readonly_static("default_text_header", &CommonSEGY::default_text_header);
	CommonSEGY_py.def_readonly_static("TEXT_HEADER_SIZE", &CommonSEGY::TEXT_HEADER_SIZE);
	CommonSEGY_py.def_readonly_static("TR_HEADER_SIZE", &CommonSEGY::TR_HEADER_SIZE);
	CommonSEGY_py.def_readonly_static("trace_header_description", &CommonSEGY::trace_header_description);

	py::class_<CommonSEGY::BinaryHeader> BinaryHeader_py(CommonSEGY_py, "BinaryHeader");
	BinaryHeader_py.def("name_as_string", &CommonSEGY::BinaryHeader::name_as_string);
	
	py::enum_<CommonSEGY::BinaryHeader::Name>(BinaryHeader_py, "Name");
	py::enum_<CommonSEGY::TrHdrValueType>(CommonSEGY_py, "TrHdrValueType");

	py::class_<ISEGY> ISEGY_py(m, "ISEGY");
	ISEGY_py.def(py::init<string,
				 vector<map<uint32_t, pair<string, CommonSEGY::TrHdrValueType>>>,
				 vector<pair<string, map<uint32_t, pair<string, CommonSEGY::TrHdrValueType>>>>>(),
				 py::arg("file_name"), py::arg("tr_hdr_over") = vector<map<uint32_t, pair<string, CommonSEGY::TrHdrValueType>>>(),
				 py::arg("add_hdr_map") = vector<pair<string, map<uint32_t, pair<string, CommonSEGY::TrHdrValueType>>>>());
	ISEGY_py.def("read_binary_header", &ISEGY::read_binary_header, "creates ISegy instance internally and returns binary header.");
	ISEGY_py.def("text_headers", &ISEGY::text_headers, "segy text headers getter");
	ISEGY_py.def("trailer_stanzas", &ISEGY::trailer_stanzas, "segy trailer stanzas getter");
	ISEGY_py.def("binary_header", &ISEGY::binary_header, "segy binary header getter");
	ISEGY_py.def("has_trace", &ISEGY::has_trace, "checks for next trace in file");
	ISEGY_py.def("read_header", &ISEGY::read_header, "reads header, skips samples");
	ISEGY_py.def("read_trace", &ISEGY::read_trace, "reads one trace from file");

	py::class_<Trace> Trace_py(m, "Trace");
	Trace_py.def(py::init<unordered_map<string, Trace::Header::Value>, vector<double>>());
	Trace_py.def("header", &Trace::header, "Returns trace header");
	Trace_py.def("samples", &Trace::samples, "Returns trace samples");
	Trace_py.def("samples_as_numpy_array",
			 [](Trace &t) { return py::array_t<double>(t.samples()); },
			 "Returns trace samples as numpy array");

	py::class_<Trace::Header> Header_py(Trace_py, "Header");
	Header_py.def(py::init<unordered_map<string, Trace::Header::Value>>());
	Header_py.def("get", &Trace::Header::get, "Gets header value by specified key");
	Header_py.def("set", &Trace::Header::set, "Sets or adds header value by specified key",
				  py::arg("key"), py::arg("v"));
	Header_py.def("keys", &Trace::Header::keys, "Retruns all keys for header");
}
}
