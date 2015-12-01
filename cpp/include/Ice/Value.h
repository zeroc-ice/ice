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

protected:
    
    virtual void __writeImpl(IceInternal::BasicStream*) const {}
    virtual void __readImpl(IceInternal::BasicStream*) {}
};

template<typename T>
struct ICE_API InterfaceTraits
{    
    static const int compactId = -1;
    static const std::string staticId;
};

ICE_API void ice_writeObject(const OutputStreamPtr&, const ValuePtr&);
ICE_API void ice_readObject(const InputStreamPtr&, ValuePtr&);

}
#endif // C++11 mapping end

#endif
