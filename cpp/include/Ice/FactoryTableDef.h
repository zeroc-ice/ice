// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_FACTORY_TABLE_DEF_H
#define ICE_FACTORY_TABLE_DEF_H

#include <IceUtil/StaticMutex.h>
#include <IceUtil/Mutex.h>
#include <Ice/UserExceptionFactory.h>
#include <Ice/ObjectFactory.h>
#include <string>
#include <map>

namespace Ice
{

class ICE_API FactoryTableDef : public IceUtil::noncopyable
{
public:

    void addExceptionFactory(const ::std::string&, const Ice::UserExceptionFactoryPtr&);
    Ice::UserExceptionFactoryPtr getExceptionFactory(const ::std::string&) const;
    void removeExceptionFactory(const ::std::string&);

    void addObjectFactory(const ::std::string&, const Ice::ObjectFactoryPtr&);
    Ice::ObjectFactoryPtr getObjectFactory(const ::std::string&) const;
    void removeObjectFactory(const ::std::string&);

private:

    IceUtil::Mutex _m;

    typedef ::std::pair<Ice::UserExceptionFactoryPtr, int> EFPair;
    typedef ::std::map< ::std::string, EFPair> EFTable;
    EFTable _eft;

    typedef ::std::pair<Ice::ObjectFactoryPtr, int> OFPair;
    typedef ::std::map< ::std::string, OFPair> OFTable;
    OFTable _oft;
};

class ICE_API FactoryTableWrapper : IceUtil::noncopyable
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
