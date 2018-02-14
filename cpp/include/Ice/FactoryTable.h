// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_FACTORYTABLE_H
#define ICE_FACTORYTABLE_H

#include <IceUtil/Mutex.h>
#include <Ice/UserExceptionFactory.h>
#include <Ice/ValueFactory.h>

namespace Ice
{

class ICE_API CompactIdResolver : public IceUtil::Shared
{
public:

    virtual ::std::string resolve(Ice::Int) const = 0;
};
typedef IceUtil::Handle<CompactIdResolver> CompactIdResolverPtr;

}

namespace IceInternal
{

class ICE_API FactoryTable : private IceUtil::noncopyable
{
public:

    void addExceptionFactory(const ::std::string&, ICE_IN(ICE_DELEGATE(::Ice::UserExceptionFactory)));
    ICE_DELEGATE(::Ice::UserExceptionFactory) getExceptionFactory(const ::std::string&) const;
    void removeExceptionFactory(const ::std::string&);

    void addValueFactory(const ::std::string&, ICE_IN(ICE_DELEGATE(::Ice::ValueFactory)));
    ICE_DELEGATE(::Ice::ValueFactory) getValueFactory(const ::std::string&) const;
    void removeValueFactory(const ::std::string&);

    void addTypeId(int, const ::std::string&);
    std::string getTypeId(int) const;
    void removeTypeId(int);

private:

    IceUtil::Mutex _m;

    typedef ::std::pair< ICE_DELEGATE(::Ice::UserExceptionFactory), int> EFPair;
    typedef ::std::map< ::std::string, EFPair> EFTable;
    EFTable _eft;

    typedef ::std::pair< ICE_DELEGATE(::Ice::ValueFactory), int> VFPair;
    typedef ::std::map< ::std::string, VFPair> VFTable;
    VFTable _vft;

    typedef ::std::pair< ::std::string, int> TypeIdPair;
    typedef ::std::map<int, TypeIdPair> TypeIdTable;
    TypeIdTable _typeIdTable;
};

}

#endif
