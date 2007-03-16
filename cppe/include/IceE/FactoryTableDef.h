// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_FACTORY_TABLE_DEF_H
#define ICEE_FACTORY_TABLE_DEF_H

#include <IceE/UserExceptionFactoryF.h>

#include <IceE/StaticMutex.h>
#include <IceE/Mutex.h>

namespace IceInternal
{

class ICE_API FactoryTableDef : private IceUtil::noncopyable
{
public:

    void addExceptionFactory(const ::std::string&, const IceInternal::UserExceptionFactoryPtr&);
    IceInternal::UserExceptionFactoryPtr getExceptionFactory(const ::std::string&) const;
    void removeExceptionFactory(const ::std::string&);

private:

    IceUtil::Mutex _m;

    typedef ::std::pair<IceInternal::UserExceptionFactoryPtr, int> EFPair;
    typedef ::std::map< ::std::string, EFPair> EFTable;
    EFTable _eft;
};

class ICE_API FactoryTableWrapper : private IceUtil::noncopyable
{
public:

    friend class FactoryTable;

    FactoryTableWrapper();
    ~FactoryTableWrapper();

private:

    void initialize();
    void finalize();
    static IceUtil::StaticMutex _m;
    static int _initCount;
};

extern ICE_API FactoryTableWrapper factoryTableWrapper;

}

#endif
