// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#define EXPORT_FCNS

#include <Ice/Ice.h>
#include "icematlab.h"
#include "Util.h"

#define SELF (reinterpret_cast<Ice::OutputStream*>(self))

using namespace std;
using namespace IceMatlab;


extern "C"
{

EXPORTED_FUNCTION mxArray*
Ice_OutputStream__release(void* self)
{
    delete SELF;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeBool(void* self, unsigned char v)
{
    try
    {
        SELF->write(v == 1);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeBoolOpt(void* self, int tag, unsigned char v)
{
    try
    {
        SELF->writeAll({tag}, IceUtil::Optional<bool>(v == 1));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeBoolSeq(void* self, const unsigned char* v, int size)
{
    const bool* p = reinterpret_cast<const bool*>(v);
    try
    {
        SELF->write(p, p + size);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeBoolSeqOpt(void* self, int tag, const unsigned char* v, int size)
{
    const bool* p = reinterpret_cast<const bool*>(v);
    try
    {
        if(SELF->writeOptional(tag, Ice::OptionalFormat::VSize))
        {
            SELF->write(p, p + size);
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeByte(void* self, unsigned char v)
{
    try
    {
        SELF->write(static_cast<Ice::Byte>(v));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeByteOpt(void* self, int tag, unsigned char v)
{
    try
    {
        SELF->writeAll({tag}, IceUtil::Optional<Ice::Byte>(static_cast<Ice::Byte>(v)));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeByteSeq(void* self, const unsigned char* v, int size)
{
    const Ice::Byte* p = reinterpret_cast<const Ice::Byte*>(v);
    try
    {
        SELF->write(p, p + size);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeByteSeqOpt(void* self, int tag, const unsigned char* v, int size)
{
    const Ice::Byte* p = reinterpret_cast<const Ice::Byte*>(v);
    try
    {
        if(SELF->writeOptional(tag, Ice::OptionalFormat::VSize))
        {
            SELF->write(p, p + size);
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeShort(void* self, short v)
{
    try
    {
        SELF->write(v);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeShortOpt(void* self, int tag, short v)
{
    try
    {
        SELF->writeAll({tag}, IceUtil::Optional<short>(v));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeShortSeq(void* self, const short* v, int size)
{
    try
    {
        SELF->write(v, v + size);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeShortSeqOpt(void* self, int tag, const short* v, int size)
{
    try
    {
        if(SELF->writeOptional(tag, Ice::OptionalFormat::VSize))
        {
            SELF->write(v, v + size);
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeInt(void* self, int v)
{
    try
    {
        SELF->write(v);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeIntOpt(void* self, int tag, int v)
{
    try
    {
        SELF->writeAll({tag}, IceUtil::Optional<int>(v));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeIntSeq(void* self, const int* v, int size)
{
    try
    {
        SELF->write(v, v + size);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeIntSeqOpt(void* self, int tag, const int* v, int size)
{
    try
    {
        if(SELF->writeOptional(tag, Ice::OptionalFormat::VSize))
        {
            SELF->write(v, v + size);
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeLong(void* self, long long v)
{
    try
    {
        SELF->write(v);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeLongOpt(void* self, int tag, long long v)
{
    try
    {
        SELF->writeAll({tag}, IceUtil::Optional<long long>(v));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeLongSeq(void* self, const long long* v, int size)
{
    try
    {
        SELF->write(v, v + size);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeLongSeqOpt(void* self, int tag, const long long* v, int size)
{
    try
    {
        if(SELF->writeOptional(tag, Ice::OptionalFormat::VSize))
        {
            SELF->write(v, v + size);
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeFloat(void* self, float v)
{
    try
    {
        SELF->write(v);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeFloatOpt(void* self, int tag, float v)
{
    try
    {
        SELF->writeAll({tag}, IceUtil::Optional<float>(v));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeFloatSeq(void* self, const float* v, int size)
{
    try
    {
        SELF->write(v, v + size);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeFloatSeqOpt(void* self, int tag, const float* v, int size)
{
    try
    {
        if(SELF->writeOptional(tag, Ice::OptionalFormat::VSize))
        {
            SELF->write(v, v + size);
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeDouble(void* self, double v)
{
    try
    {
        SELF->write(v);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeDoubleOpt(void* self, int tag, double v)
{
    try
    {
        SELF->writeAll({tag}, IceUtil::Optional<double>(v));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeDoubleSeq(void* self, const double* v, int size)
{
    try
    {
        SELF->write(v, v + size);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeDoubleSeqOpt(void* self, int tag, const double* v, int size)
{
    try
    {
        if(SELF->writeOptional(tag, Ice::OptionalFormat::VSize))
        {
            SELF->write(v, v + size);
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeString(void* self, mxArray* str)
{
    try
    {
        SELF->write(getStringFromUTF16(str), false);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeStringOpt(void* self, int tag, mxArray* str)
{
    try
    {
        if(SELF->writeOptional(tag, Ice::OptionalFormat::VSize))
        {
            SELF->write(getStringFromUTF16(str), false);
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeStringSeq(void* self, mxArray* v)
{
    try
    {
        if(mxIsEmpty(v))
        {
            SELF->writeSize(0);
        }
        else if(!mxIsCell(v))
        {
            throw invalid_argument("value is not a cell array");
        }
        else
        {
            size_t m = mxGetM(v);
            size_t n = mxGetN(v);
            if(m != 1)
            {
                throw invalid_argument("cell array must be a 1xN matrix");
            }
            SELF->writeSize(n);
            for(size_t i = 0; i < n; ++i)
            {
                mxArray* e = mxGetCell(v, i);
                SELF->write(getStringFromUTF16(e), false);
            }
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeStringSeqOpt(void* self, int tag, mxArray* v)
{
    try
    {
        if(SELF->writeOptional(tag, Ice::OptionalFormat::FSize))
        {
            Ice::OutputStream::size_type pos = SELF->startSize();
            mxArray* ex = Ice_OutputStream_writeStringSeq(self, v);
            if(ex)
            {
                return ex;
            }
            SELF->endSize(pos);
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeSize(void* self, int size)
{
    try
    {
        SELF->writeSize(size);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeProxy(void* self, void* proxy)
{
    shared_ptr<Ice::ObjectPrx> p;
    if(proxy)
    {
        p = *(reinterpret_cast<shared_ptr<Ice::ObjectPrx>*>(proxy));
    }
    try
    {
        SELF->writeProxy(p);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeProxyOpt(void* self, int tag, void* proxy)
{
    assert(proxy); // Wrapper only calls this function for a non-nil value.
    shared_ptr<Ice::ObjectPrx> p = *(reinterpret_cast<shared_ptr<Ice::ObjectPrx>*>(proxy));
    try
    {
        SELF->writeAll({tag}, IceUtil::Optional<shared_ptr<Ice::ObjectPrx>>(p));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeEnum(void* self, int val, int maxValue)
{
    try
    {
        SELF->writeEnum(val, maxValue);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_startEncapsulation(void* self)
{
    try
    {
        SELF->startEncapsulation();
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_startEncapsulationWithFormat(void* self, Ice_FormatType t)
{
    Ice::FormatType type = static_cast<Ice::FormatType>(t);
    try
    {
        SELF->startEncapsulation(SELF->getEncoding(), type);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_endEncapsulation(void* self)
{
    try
    {
        SELF->endEncapsulation();
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_startSlice(void* self, const char* typeId, int compactId, unsigned char last)
{
    try
    {
        SELF->startSlice(typeId, compactId, last == 1);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_endSlice(void* self)
{
    try
    {
        SELF->endSlice();
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_writeOptional(void* self, int tag, Ice_OptionalFormat f, unsigned char* supportsOptionals)
{
    Ice::OptionalFormat fmt = static_cast<Ice::OptionalFormat>(f);
    try
    {
        *supportsOptionals = SELF->writeOptional(tag, fmt);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_startSize(void* self, unsigned int* pos)
{
    try
    {
        *pos = static_cast<unsigned int>(SELF->startSize());
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_OutputStream_endSize(void* self, unsigned int pos)
{
    try
    {
        SELF->endSize(pos);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

}
