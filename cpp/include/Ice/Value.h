// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_VALUE_H
#define ICE_VALUE_H

#ifdef ICE_CPP11_MAPPING // C++11 mapping

#include <Ice/ValueF.h>

#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>

namespace Ice
{

class ICE_API Value
{
public:

    virtual ~Value() = default;

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();

    virtual void _iceWrite(Ice::OutputStream*) const;
    virtual void _iceRead(Ice::InputStream*);

    virtual std::string ice_id() const;
    static const std::string& ice_staticId();

    std::shared_ptr<Value> ice_clone() const;

protected:

    virtual std::shared_ptr<Value> cloneImpl() const = 0;

    virtual void _iceWriteImpl(Ice::OutputStream*) const {}
    virtual void _iceReadImpl(Ice::InputStream*) {}
};

template<typename T, typename Base> class ValueHelper : public Base
{
public:

    using Base::Base;

    ValueHelper() = default;

    std::shared_ptr<T> ice_clone() const
    {
        return std::static_pointer_cast<T>(cloneImpl());
    }

    virtual std::string ice_id() const override
    {
        return T::ice_staticId();
    }

protected:

    virtual std::shared_ptr<Value> cloneImpl() const override
    {
        return std::make_shared<T>(static_cast<const T&>(*this));
    }

    virtual void _iceWriteImpl(Ice::OutputStream* os) const override
    {
        os->startSlice(T::ice_staticId(), -1, std::is_same<Base, Ice::Value>::value ? true : false);
        Ice::StreamWriter<T, Ice::OutputStream>::write(os, static_cast<const T&>(*this));
        os->endSlice();
        Base::_iceWriteImpl(os);
    }

    virtual void _iceReadImpl(Ice::InputStream* is) override
    {
        is->startSlice();
        Ice::StreamReader<T, Ice::InputStream>::read(is, static_cast<T&>(*this));
        is->endSlice();
        Base::_iceReadImpl(is);
    }
};

}
#endif // C++11 mapping end

#endif
