// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_VALUE_H
#define ICE_VALUE_H

#ifdef ICE_CPP11_MAPPING // C++11 mapping

#include <Ice/ValueF.h>
#include <Ice/SlicedDataF.h>

#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>

namespace Ice
{

class ICE_API Value
{
public:

    // See "Rule of zero" at http://en.cppreference.com/w/cpp/language/rule_of_three
    // The virtual dtor is actually not stricly necessary since Values are always stored
    // in std::shared_ptr

    Value() = default;
    Value(const Value&) = default;
    Value(Value&&) = default;
    Value& operator=(const Value&) = default;
    Value& operator=(Value&&) = default;
    virtual ~Value() = default;

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();

    virtual std::string ice_id() const;
    static const std::string& ice_staticId();

    std::shared_ptr<Value> ice_clone() const;

    virtual std::shared_ptr<SlicedData> ice_getSlicedData() const;

    virtual void _iceWrite(Ice::OutputStream*) const;
    virtual void _iceRead(Ice::InputStream*);

protected:

    virtual std::shared_ptr<Value> _iceCloneImpl() const = 0;
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
        return std::static_pointer_cast<T>(_iceCloneImpl());
    }

    virtual std::string ice_id() const override
    {
        return T::ice_staticId();
    }

protected:

    virtual std::shared_ptr<Value> _iceCloneImpl() const override
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
