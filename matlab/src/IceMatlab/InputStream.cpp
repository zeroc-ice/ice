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

#define SELF (reinterpret_cast<InputStreamData*>(self))

using namespace std;
using namespace IceMatlab;

namespace
{

struct InputStreamData
{
    std::vector<Ice::Byte> data;
    Ice::InputStream* in;
};

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

}

void*
IceMatlab::createInputStream(const shared_ptr<Ice::Communicator>& communicator, const Ice::EncodingVersion& encoding,
                             vector<Ice::Byte>& data)
{
    auto p = new InputStreamData;
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
Ice_InputStream_setPos(void* self, unsigned int pos)
{
    try
    {
        SELF->in->pos(pos);
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
            auto r = mxCreateUninitNumericMatrix(1, end - start, mxUINT8_CLASS, mxREAL);
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
