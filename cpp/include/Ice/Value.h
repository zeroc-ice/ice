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

/**
 * The base class for instances of Slice classes.
 * \headerfile Ice/Ice.h
 */
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

    /**
     * The Ice run time invokes this method prior to marshaling an object's data members. This allows a subclass
     * to override this method in order to validate its data members.
     */
    virtual void ice_preMarshal();

    /**
     * The Ice run time invokes this method vafter unmarshaling an object's data members. This allows a
     * subclass to override this method in order to perform additional initialization.
     */
    virtual void ice_postUnmarshal();

    /**
     * Obtains the Slice type ID of the most-derived class supported by this object.
     * @return The type ID.
     */
    virtual std::string ice_id() const;

    /**
     * Obtains the Slice type ID of this type.
     * @return The return value is always "::Ice::Object".
     */
    static const std::string& ice_staticId();

    /**
     * Returns a shallow copy of the object.
     * @return The cloned value.
     */
    std::shared_ptr<Value> ice_clone() const;

    /**
     * Obtains the sliced data associated with this instance.
     * @return The sliced data if the value has a preserved-slice base class and has been sliced during
     * unmarshaling of the value, nil otherwise.
     */
    virtual std::shared_ptr<SlicedData> ice_getSlicedData() const;

    /// \cond STREAM
    virtual void _iceWrite(Ice::OutputStream*) const;
    virtual void _iceRead(Ice::InputStream*);
    /// \endcond

protected:

    /// \cond INTERNAL
    virtual std::shared_ptr<Value> _iceCloneImpl() const = 0;
    /// \endcond

    /// \cond STREAM
    virtual void _iceWriteImpl(Ice::OutputStream*) const {}
    virtual void _iceReadImpl(Ice::InputStream*) {}
    /// \endcond
};

/// \cond INTERNAL
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
/// \endcond

}
#endif // C++11 mapping end

#endif
