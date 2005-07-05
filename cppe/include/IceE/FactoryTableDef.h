// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_FACTORY_TABLE_DEF_H
#define ICEE_FACTORY_TABLE_DEF_H

#include <IceE/UserExceptionFactoryF.h>

#include <IceE/StaticMutex.h>
#include <IceE/Mutex.h>

namespace IceE
{

class ICEE_API FactoryTableDef : private IceE::noncopyable
{
public:

    void addExceptionFactory(const ::std::string&, const IceEInternal::UserExceptionFactoryPtr&);
    IceEInternal::UserExceptionFactoryPtr getExceptionFactory(const ::std::string&) const;
    void removeExceptionFactory(const ::std::string&);

private:

    IceE::Mutex _m;

    typedef ::std::pair<IceEInternal::UserExceptionFactoryPtr, int> EFPair;
    typedef ::std::map< ::std::string, EFPair> EFTable;
    EFTable _eft;
};

class ICEE_API FactoryTableWrapper : private IceE::noncopyable
{
public:

    friend class FactoryTable;

    FactoryTableWrapper();
    ~FactoryTableWrapper();

private:

    void initialize();
    void finalize();
    static IceE::StaticMutex _m;
    static int _initCount;
};

extern ICEE_API FactoryTableWrapper factoryTableWrapper;

}

#endif
