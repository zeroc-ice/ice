// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_EXCEPTION_HELPERS_H
#define ICE_EXCEPTION_HELPERS_H

#ifdef ICE_CPP11_MAPPING // C++11 mapping

#include <Ice/InputStream.h>
#include <Ice/OutputStream.h>

namespace Ice
{

class LocalException;

template<typename T, typename B> class LocalExceptionHelper : public IceUtil::ExceptionHelper<T, B>
{
public:

    using IceUtil::ExceptionHelper<T, B>::ExceptionHelper;

    virtual std::string ice_id() const override
    {
        return T::ice_staticId();
    }
};

template<typename T, typename B> class UserExceptionHelper : public IceUtil::ExceptionHelper<T, B>
{
public:

    using IceUtil::ExceptionHelper<T, B>::ExceptionHelper;

    virtual std::string ice_id() const override
    {
        return T::ice_staticId();
    }

protected:

    virtual void _writeImpl(Ice::OutputStream* os) const override
    {
        os->startSlice(T::ice_staticId(), -1, std::is_same<B, Ice::LocalException>::value ? true : false);
        Ice::StreamWriter<T, Ice::OutputStream>::write(os, static_cast<const T&>(*this));
        os->endSlice();
        B::_writeImpl(os);
    }

    virtual void _readImpl(Ice::InputStream* is) override
    {
        is->startSlice();
        Ice::StreamReader<T, ::Ice::InputStream>::read(is, static_cast<T&>(*this));
        is->endSlice();
        B::_readImpl(is);
    }
};

}

#endif // C++11 mapping end

#endif
