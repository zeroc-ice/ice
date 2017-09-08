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
#include <Ice/Optional.h>
#include "icematlab.h"
#include "InputStream.h"
#include "Util.h"

#define SELF (reinterpret_cast<IceMatlab::InputStreamData*>(self))

using namespace std;
using namespace IceMatlab;

namespace
{

template<typename T>
mxArray*
readSeq(Ice::InputStream* in, mxClassID cls)
{
    try
    {
        pair<const T*, const T*> p;
        in->read(p);
        const size_t nelem = p.second - p.first;
        if(nelem > 0)
        {
            mxArray* r = mxCreateUninitNumericMatrix(1, nelem, cls, mxREAL);
            memcpy(reinterpret_cast<T*>(mxGetData(r)), p.first, nelem * sizeof(T));
            return createResultValue(r);
        }
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

template<typename T>
mxArray*
readOpt(Ice::InputStream* in, int tag, mxClassID cls)
{
    try
    {
        Ice::optional<T> opt;
        in->read(tag, opt);
        if(opt.has_value())
        {
            mxArray* r = mxCreateUninitNumericMatrix(1, 1, cls, mxREAL);
            *(reinterpret_cast<T*>(mxGetData(r))) = opt.value();
            return createResultValue(r);
        }
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

template<typename T>
mxArray*
readSeqOpt(Ice::InputStream* in, int tag, mxClassID cls)
{
    try
    {
        Ice::optional<pair<const T*, const T*>> opt;
        in->read(tag, opt);
        const size_t sz = opt.value().second - opt.value().first;
        if(opt.has_value() && sz > 0)
        {
            mxArray* r = mxCreateUninitNumericMatrix(1, sz / sizeof(T), cls, mxREAL);
            memcpy(reinterpret_cast<T*>(mxGetData(r)), opt.value().first, sz);
            return createResultValue(r);
        }
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

}

void*
IceMatlab::createInputStream(const shared_ptr<Ice::Communicator>& communicator, const Ice::EncodingVersion& encoding,
                             vector<Ice::Byte>& data)
{
    InputStreamData* p = new InputStreamData;
    p->data.swap(data);
    p->in = new Ice::InputStream(communicator, encoding, p->data);
    return p;
}

extern "C"
{

EXPORTED_FUNCTION mxArray*
Ice_InputStream__release(void* self)
{
    delete SELF->in;
    delete SELF;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readBool(void* self, unsigned char* v)
{
    try
    {
        bool b;
        SELF->in->read(b);
        *v = b ? 1 : 0;
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readBoolSeq(void* self)
{
    try
    {
        pair<const bool*, const bool*> p;
        SELF->in->read(p);
        const size_t sz = p.second - p.first;
        if(sz > 0)
        {
            mxArray* r = mxCreateLogicalMatrix(1, sz);
            memcpy(mxGetLogicals(r), p.first, sz);
            return createResultValue(r);
        }
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readBoolOpt(void* self, int tag)
{
    try
    {
        Ice::optional<bool> opt;
        SELF->in->read(tag, opt);
        if(opt.has_value())
        {
            return createResultValue(mxCreateLogicalScalar(opt.value()));
        }
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readBoolSeqOpt(void* self, int tag)
{
    try
    {
        Ice::optional<pair<const bool*, const bool*>> opt;
        SELF->in->read(tag, opt);
        const size_t sz = opt.value().second - opt.value().first;
        if(opt.has_value() && sz > 0)
        {
            mxArray* r = mxCreateLogicalMatrix(1, sz);
            memcpy(mxGetLogicals(r), opt.value().first, sz);
            return createResultValue(r);
        }
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readByte(void* self, unsigned char* v)
{
    try
    {
        Ice::Byte b;
        SELF->in->read(b);
        *v = b;
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readByteSeq(void* self)
{
    return readSeq<Ice::Byte>(SELF->in, mxUINT8_CLASS);
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readByteOpt(void* self, int tag)
{
    return readOpt<Ice::Byte>(SELF->in, tag, mxUINT8_CLASS);
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readByteSeqOpt(void* self, int tag)
{
    return readSeqOpt<Ice::Byte>(SELF->in, tag, mxUINT8_CLASS);
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readShort(void* self, short* v)
{
    try
    {
        SELF->in->read(*v);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readShortSeq(void* self)
{
    return readSeq<short>(SELF->in, mxINT16_CLASS);
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readShortOpt(void* self, int tag)
{
    return readOpt<short>(SELF->in, tag, mxINT16_CLASS);
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readShortSeqOpt(void* self, int tag)
{
    return readSeqOpt<short>(SELF->in, tag, mxINT16_CLASS);
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readInt(void* self, int* v)
{
    try
    {
        SELF->in->read(*v);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readIntSeq(void* self)
{
    return readSeq<int>(SELF->in, mxINT32_CLASS);
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readIntOpt(void* self, int tag)
{
    return readOpt<int>(SELF->in, tag, mxINT32_CLASS);
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readIntSeqOpt(void* self, int tag)
{
    return readSeqOpt<int>(SELF->in, tag, mxINT32_CLASS);
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readLong(void* self, long long* v)
{
    try
    {
        SELF->in->read(*v);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readLongSeq(void* self)
{
    return readSeq<long long>(SELF->in, mxINT64_CLASS);
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readLongOpt(void* self, int tag)
{
    return readOpt<long long>(SELF->in, tag, mxINT64_CLASS);
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readLongSeqOpt(void* self, int tag)
{
    return readSeqOpt<long long>(SELF->in, tag, mxINT64_CLASS);
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readFloat(void* self, float* v)
{
    try
    {
        SELF->in->read(*v);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readFloatSeq(void* self)
{
    return readSeq<float>(SELF->in, mxSINGLE_CLASS);
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readFloatOpt(void* self, int tag)
{
    return readOpt<float>(SELF->in, tag, mxSINGLE_CLASS);
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readFloatSeqOpt(void* self, int tag)
{
    return readSeqOpt<float>(SELF->in, tag, mxSINGLE_CLASS);
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readDouble(void* self, double* v)
{
    try
    {
        SELF->in->read(*v);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readDoubleSeq(void* self)
{
    return readSeq<double>(SELF->in, mxDOUBLE_CLASS);
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readDoubleOpt(void* self, int tag)
{
    return readOpt<double>(SELF->in, tag, mxDOUBLE_CLASS);
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readDoubleSeqOpt(void* self, int tag)
{
    return readSeqOpt<double>(SELF->in, tag, mxDOUBLE_CLASS);
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readString(void* self)
{
    string s;
    try
    {
        SELF->in->read(s, false);
        return createResultValue(createStringFromUTF8(s));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readStringSeq(void* self)
{
    try
    {
        vector<string> v;
        SELF->in->read(v, false);
        return createResultValue(createStringList(v));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readStringOpt(void* self, int tag)
{
    try
    {
        Ice::optional<string> opt;
        SELF->in->read(tag, opt);
        if(opt.has_value())
        {
            return createResultValue(createStringFromUTF8(opt.value()));
        }
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readStringSeqOpt(void* self, int tag)
{
    try
    {
        if(SELF->in->readOptional(tag, Ice::OptionalFormat::FSize))
        {
            SELF->in->skip(4);
            return Ice_InputStream_readStringSeq(self);
        }
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_skip(void* self, int n)
{
    try
    {
        SELF->in->skip(n);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_skipSize(void* self)
{
    try
    {
        SELF->in->skipSize();
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_getEncoding(void* self)
{
    try
    {
        return createResultValue(createEncodingVersion(SELF->in->getEncoding()));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readSize(void* self, int* size)
{
    try
    {
        *size = SELF->in->readSize();
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readProxy(void* self, void** r)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> proxy;
        SELF->in->read(proxy);
        if(proxy)
        {
            *r = new shared_ptr<Ice::ObjectPrx>(proxy);
        }
        else
        {
            *r = 0;
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readProxyOpt(void* self, int tag, void** r)
{
    try
    {
        Ice::optional<shared_ptr<Ice::ObjectPrx>> opt;
        SELF->in->readAll({tag}, opt);
        if(opt.has_value())
        {
            *r = new shared_ptr<Ice::ObjectPrx>(opt.value());
        }
        else
        {
            *r = 0;
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_readEnum(void* self, int maxValue, int* r)
{
    try
    {
        *r = SELF->in->readEnum(maxValue);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_pos(void* self, unsigned int* pos)
{
    try
    {
        *pos = SELF->in->pos();
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_size(void* self, unsigned int* sz)
{
    try
    {
        *sz = SELF->in->b.size();
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InputStream_getBytes(void* self, unsigned int start, unsigned int end)
{
    if(end > start)
    {
        try
        {
            mxArray* r = mxCreateUninitNumericMatrix(1, end - start, mxUINT8_CLASS, mxREAL);
            memcpy(reinterpret_cast<Ice::Byte*>(mxGetData(r)), SELF->in->b.begin() + start, end - start);
            return createResultValue(r);
        }
        catch(const std::exception& ex)
        {
            return createResultException(convertException(ex));
        }
    }
    return 0;
}

}
