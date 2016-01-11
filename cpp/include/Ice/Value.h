// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_VALUE_H
#define ICE_VALUE_H

#ifdef ICE_CPP11_MAPPING // C++11 mapping

#include <Ice/ValueF.h>
#include <Ice/StreamF.h>

namespace IceInternal
{

class BasicStream;

}

namespace Ice
{

class ICE_API Value
{
public:
    
    virtual ~Value() = default;
    
    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();

    virtual void __write(IceInternal::BasicStream*) const;
    virtual void __read(IceInternal::BasicStream*);
    
    virtual const std::string& ice_id() const;
    static const std::string& ice_staticId();
    
    std::shared_ptr<Value> ice_clone() const;

protected:

    virtual std::shared_ptr<Value> cloneImpl() const = 0;

    virtual void __writeImpl(IceInternal::BasicStream*) const {}
    virtual void __readImpl(IceInternal::BasicStream*) {}
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
    
    virtual const std::string& ice_id() const override
    {
        return T::ice_staticId();
    }

protected:

    virtual std::shared_ptr<Value> cloneImpl() const
    {
        return std::make_shared<T>(static_cast<const T&>(*this));
    }
};

ICE_API void ice_writeObject(const OutputStreamPtr&, const std::shared_ptr<Value>&);
ICE_API void ice_readObject(const InputStreamPtr&, std::shared_ptr<Value>&);

}
#endif // C++11 mapping end

#endif
