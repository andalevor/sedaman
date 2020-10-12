#include "C_API.hpp"
#include "ISEGD.hpp"
#include "ISEGY.hpp"
#include "OSEGYRev0.hpp"
#include "Trace.hpp"
#include <cstring>

using sedaman::CommonSEGY;
using sedaman::ISEGD;
using sedaman::ISEGY;
using sedaman::OSEGYRev0;
using sedaman::Trace;
using std::exception;
using std::get;
using std::holds_alternative;
using std::move;
using std::string;
using std::strncpy;

struct sedaman_CommonSEGY_BinaryHeader {
    CommonSEGY::BinaryHeader hdr;
};

struct sedaman_ISEGY {
    sedaman_ISEGY(char const* file_name)
        : sgy { ISEGY { file_name } }
    {
    }
    ISEGY sgy;
};

struct sedaman_OSEGYRev0 {
    sedaman_OSEGYRev0(char const* file_name)
        : sgy { OSEGYRev0 { file_name } }
    {
    }
    OSEGYRev0 sgy;
};

struct sedaman_ISEGD {
    sedaman_ISEGD(char const* file_name)
        : sgd { ISEGD { file_name } }
    {
    }
    ISEGD sgd;
};

struct sedaman_Trace {
    sedaman_Trace(Trace t)
        : trc { move(t) }
    {
    }
    Trace trc;
};
#include <iostream>
sedaman_ISEGY* sedaman_ISEGY_new(char const* file_name,
    char* err)
{
    try {
        sedaman_ISEGY* result = new sedaman_ISEGY(file_name);
        return result;
    } catch (exception& e) {
        string const& s = e.what();
        strncpy(err, s.data(), 255);
        err[255] = '\0';
        return NULL;
    }
}

void sedaman_ISEGY_delete(sedaman_ISEGY* sgy)
{
    delete sgy;
}

bool sedaman_ISEGY_has_trace(sedaman_ISEGY* sgy)
{
    return sgy->sgy.has_trace();
}

sedaman_Trace* sedaman_ISEGY_read_trace(sedaman_ISEGY* sgy,
    char const** err)
{
    try {
        sedaman_Trace* result = new sedaman_Trace(sgy->sgy.read_trace());
        return result;
    } catch (exception& e) {
        string s = e.what();
        char* new_s = static_cast<char*>(malloc(s.size() + 1));
        strncpy(new_s, s.data(), s.size());
        new_s[s.size()] = '\0';
        *err = new_s;
        return NULL;
    }
}

sedaman_OSEGYRev0* sedaman_OSEGYRev0_new(char const* file_name,
    char const** err)
{
    try {
        sedaman_OSEGYRev0* result = new sedaman_OSEGYRev0(file_name);
        return result;
    } catch (exception& e) {
        string const& s = e.what();
        char* new_s = static_cast<char*>(malloc(s.size() + 1));
        strncpy(new_s, s.data(), s.size());
        new_s[s.size()] = '\0';
        *err = new_s;
        return NULL;
    }
}

void sedaman_OSEGYRev0_delete(sedaman_OSEGYRev0* sgy)
{
    delete sgy;
}

void sedaman_OSEGYRev0_write(sedaman_OSEGYRev0* sgy,
    sedaman_Trace* trc,
    char const** err)
{
    try {
        sgy->sgy.write_trace(trc->trc);
    } catch (exception& e) {
        string s = e.what();
        char* new_s = static_cast<char*>(malloc(s.size() + 1));
        strncpy(new_s, s.data(), s.size());
        new_s[s.size()] = '\0';
        *err = new_s;
    }
}

sedaman_ISEGD* sedaman_ISEGD_new(char const* file_name,
    char const** err)
{
    try {
        sedaman_ISEGD* result = new sedaman_ISEGD(file_name);
        return result;
    } catch (exception& e) {
        string const& s = e.what();
        char* new_s = static_cast<char*>(malloc(s.size() + 1));
        strncpy(new_s, s.data(), s.size());
        new_s[s.size()] = '\0';
        *err = new_s;
        return NULL;
    }
}

void sedaman_ISEGD_delete(sedaman_ISEGD* sgd)
{
    delete sgd;
}

bool sedaman_ISEGD_has_record(sedaman_ISEGD* sgd)
{
    return sgd->sgd.has_record();
}

bool sedaman_ISEGD_has_trace(sedaman_ISEGD* sgd)
{
    return sgd->sgd.has_trace();
}

sedaman_Trace* sedaman_ISEGD_read_trace(sedaman_ISEGD* sgd,
    char const** err)
{
    try {
        sedaman_Trace* result = new sedaman_Trace(sgd->sgd.read_trace());
        return result;
    } catch (exception& e) {
        string s = e.what();
        char* new_s = static_cast<char*>(malloc(s.size() + 1));
        strncpy(new_s, s.data(), s.size());
        new_s[s.size()] = '\0';
        *err = new_s;
        return NULL;
    }
}

void sedaman_Trace_delete(sedaman_Trace* trc)
{
    delete trc;
}

sedaman_Trace_Header_Value_Holder
sedaman_Trace_Header_get_value(sedaman_Trace* t,
    char const* s)
{
    sedaman_Trace_Header_Value_Holder result;
    std::optional<sedaman::Trace::Header::Value> opt = t->trc.header().get(s);
    if (opt) {
        sedaman::Trace::Header::Value val = *opt;
        if (holds_alternative<int8_t>(val)) {
            result.type = I8;
            result.val.i8 = get<int8_t>(val);
        } else if (holds_alternative<int16_t>(val)) {
            result.type = I16;
            result.val.i16 = get<int16_t>(val);
        } else if (holds_alternative<int32_t>(val)) {
            result.type = I32;
            result.val.i32 = get<int32_t>(val);
        } else if (holds_alternative<int64_t>(val)) {
            result.type = I64;
            result.val.i64 = get<int64_t>(val);
        } else if (holds_alternative<uint8_t>(val)) {
            result.type = U8;
            result.val.u8 = get<uint8_t>(val);
        } else if (holds_alternative<uint16_t>(val)) {
            result.type = U16;
            result.val.u16 = get<uint16_t>(val);
        } else if (holds_alternative<uint32_t>(val)) {
            result.type = U32;
            result.val.u32 = get<uint32_t>(val);
        } else if (holds_alternative<uint64_t>(val)) {
            result.type = U64;
            result.val.u64 = get<uint64_t>(val);
        } else if (holds_alternative<float>(val)) {
            result.type = F32;
            result.val.f32 = get<float>(val);
        } else if (holds_alternative<double>(val)) {
            result.type = F64;
            result.val.f64 = get<double>(val);
        }
    } else {
        result.type = NONE;
    }
    return result;
}

double sedaman_Trace_get_sample(sedaman_Trace* trc, size_t num)
{
    return trc->trc.samples()[num];
}

size_t sedaman_Trace_samples_num(sedaman_Trace* trc)
{
    return trc->trc.samples().size();
}
