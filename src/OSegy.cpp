#include "CommonSegy.hpp"
#include "Exception.hpp"
#include "OSegy.hpp"
#include "util.hpp"
#include <bits/stdint-uintn.h>
#include <functional>

using std::fstream;
using std::function;
using std::make_unique;
using std::string;

namespace sedaman {
class OSegy::Impl {
public:
	Impl(string const& name, string const& revision);
	Impl(string&& name, string&& revision);
	CommonSegy common;
private:
	function<void(char **, uint8_t)> write_u8;
	function<void(char **, uint8_t)> write_i8;
	function<void(char **, uint16_t)> write_u16;
	function<void(char **, uint16_t)> write_i16;
	function<void(char **, uint32_t)> write_u24;
	function<void(char **, uint32_t)> write_i24;
	function<void(char **, uint32_t)> write_u32;
	function<void(char **, uint32_t)> write_i32;
	function<void(char **, uint64_t)> write_u64;
	function<void(char **, uint64_t)> write_i64;
	function<void(char **, double)> write_sample;
	void init_r0();
	void init_r1();
	void init_r20();
	void assign_raw_writers();
};

OSegy::Impl::Impl(string const& name, string const& revision)
	: common { name, fstream::out | fstream::binary }
{
	if (revision == "0")
		init_r0();
	else if (revision == "1")
		init_r1();
	else
		init_r20();
}

OSegy::Impl::Impl(string&& name, string&& revision)
	: common { move(name), fstream::out | fstream::binary }
{
	if (revision == "0")
		init_r0();
	else if (revision == "1")
		init_r1();
	else
		init_r20();
}

void OSegy::Impl::init_r0()
{
	string txt_hdr(CommonSegy::default_text_header, CommonSegy::TEXT_HEADER_SIZE);
	CommonSegy::ascii_to_ebcdic(txt_hdr);
	common.file.write(txt_hdr.c_str(), CommonSegy::TEXT_HEADER_SIZE);
	common.bin_hdr.format_code = 1;
	common.bytes_per_sample = 4;
}

void OSegy::Impl::assign_raw_writers()
{
	write_u8 = [](char** buf, uint8_t val) { write<uint8_t>(buf, val); };
	write_i8 = [](char** buf, int8_t val) { write<int8_t>(buf, val); };
	switch (common.bin_hdr.endianness) {
		case 0x01020304:
			write_u16 = [](char** buf, uint16_t val) { write<uint16_t>(buf, val); };
			write_i16 = [](char** buf, int16_t val) { write<int16_t>(buf, val); };
			write_u24 = [](char** buf, uint32_t val) {
				write<uint16_t>(buf, val);
				write<uint8_t>(buf, val >> 16);
			};
			write_i24 = [](char** buf, int32_t val) {
				write<int16_t>(buf, val);
				write<int8_t>(buf, static_cast<int32_t>(val) >> 16);
			};
			write_u32 = [](char** buf, uint32_t val) { write<uint32_t>(buf, val); };
			write_i32 = [](char** buf, int32_t val) { write<int32_t>(buf, val); };
			write_u64 = [](char** buf, uint64_t val) { write<uint64_t>(buf, val); };
			write_i64 = [](char** buf, int64_t val) { write<int64_t>(buf, val); };
			break;
		case 0:
		case 0x04030201:
			write_u16 = [](char** buf, uint16_t val) { write<uint16_t>(buf, swap(val)); };
			write_i16 = [](char** buf, int16_t val) { write<int16_t>(buf, swap(val)); };
			write_u24 = [](char** buf, uint32_t val) {
				uint32_t tmp = swap(val);
				write<uint16_t>(buf, tmp);
				write<uint8_t>(buf, tmp >> 16);
			};
			write_i24 = [](char** buf, int32_t val) {
				uint32_t tmp = swap(static_cast<uint32_t>(val));
				write<int16_t>(buf, tmp);
				write<int8_t>(buf, tmp >> 16);
			};
			write_u32 = [](char** buf, uint32_t val) { write<uint32_t>(buf, swap(val)); };
			write_i32 = [](char** buf, int32_t val) { write<int32_t>(buf, swap(val)); };
			write_u64 = [](char** buf, uint64_t val) { write<uint64_t>(buf, swap(val)); };
			write_i64 = [](char** buf, int64_t val) { write<int64_t>(buf, swap(val)); };
			break;
		default:
			throw Exception(__FILE__, __LINE__, "unsupported endianness");
	}
}

OSegy::OSegy(string const& name, string const& revision)
	: pimpl(make_unique<Impl>(name, revision))
{
}
OSegy::OSegy(string&& name, string&& revision)
	: pimpl(make_unique<Impl>(move(name), move(revision)))
{
}

OSegy::~OSegy() = default;
} // namespace sedaman
