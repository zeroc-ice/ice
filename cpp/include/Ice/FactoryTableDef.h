// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_FACTORY_TABLE_DEF_H
#define ICE_FACTORY_TABLE_DEF_H

#include <IceUtil/StaticMutex.h>
#include <IceUtil/Mutex.h>
#include <Ice/UserExceptionFactoryF.h>
#include <Ice/ObjectFactoryF.h>
#include <string>
#include <map>

namespace IceInternal
{

class ICE_API FactoryTableDef : private IceUtil::noncopyable
{
public:

    void addExceptionFactory(const ::std::string&, const IceInternal::UserExceptionFactoryPtr&);
    IceInternal::UserExceptionFactoryPtr getExceptionFactory(const ::std::string&) const;
    void removeExceptionFactory(const ::std::string&);

    void addObjectFactory(const ::std::string&, const Ice::ObjectFactoryPtr&);
    Ice::ObjectFactoryPtr getObjectFactory(const ::std::string&) const;
    void removeObjectFactory(const ::std::string&);

private:

    IceUtil::Mutex _m;

    typedef ::std::pair<IceInternal::UserExceptionFactoryPtr, int> EFPair;
    typedef ::std::map< ::std::string, EFPair> EFTable;
    EFTable _eft;

    typedef ::std::pair<Ice::ObjectFactoryPtr, int> OFPair;
    typedef ::std::map< ::std::string, OFPair> OFTable;
    OFTable _oft;
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
