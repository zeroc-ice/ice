// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PATCH_ICE_PATCH_I_H
#define ICE_PATCH_ICE_PATCH_I_H

#include <Ice/Ice.h>
#include <IceUtil/RWRecMutex.h>
#include <IcePatch/IcePatch.h>

namespace IcePatch
{

class FileI : virtual public File
{
public:

    FileI(const Ice::ObjectAdapterPtr&);

protected:

    Ice::ObjectAdapterPtr _adapter;
    Ice::LoggerPtr _logger;
    Ice::Int _traceLevel;
    IceUtil::Time _busyTimeout;
    static IceUtil::RWRecMutex _globalMutex;
};

class DirectoryI : virtual public Directory,
		   virtual public FileI
{
public:
    
    DirectoryI(const Ice::ObjectAdapterPtr&);

    virtual FileDescPtr describe(const Ice::Current&);
    virtual FileDescSeq getContents(const Ice::Current&);
};

class RegularI : virtual public Regular,
                 virtual public FileI
{
public:
    
    RegularI(const Ice::ObjectAdapterPtr&);

    virtual FileDescPtr describe(const Ice::Current&);
    virtual Ice::Int getBZ2Size(const Ice::Current&);
    virtual Ice::ByteSeq getBZ2(Ice::Int, Ice::Int, const Ice::Current&);
};

}

#endif
