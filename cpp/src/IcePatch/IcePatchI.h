// **********************************************************************
//
// Copyright (c) 2002
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

    Ice::ByteSeq readMD5(const Ice::Current&) const;

    const Ice::ObjectAdapterPtr _adapter;
    const Ice::LoggerPtr _fileTraceLogger;
    const IceUtil::Time _busyTimeout;
};

class DirectoryI : virtual public Directory,
		   virtual public FileI
{
public:
    
    DirectoryI(const Ice::ObjectAdapterPtr&);

    virtual FileDescPtr describe(const Ice::Current&) const;
    virtual FileDescSeq getContents(const Ice::Current&) const;
};

class RegularI : virtual public Regular,
                 virtual public FileI
{
public:
    
    RegularI(const Ice::ObjectAdapterPtr&);

    virtual FileDescPtr describe(const Ice::Current&) const;
    virtual Ice::Int getBZ2Size(const Ice::Current&) const;
    virtual Ice::ByteSeq getBZ2(Ice::Int, Ice::Int, const Ice::Current&) const;
    virtual Ice::ByteSeq getBZ2MD5(Ice::Int, const Ice::Current&) const;
};

}

#endif
