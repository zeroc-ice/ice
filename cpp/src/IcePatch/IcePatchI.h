// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
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

class InfoI : public Info
{
public:

    InfoI(const Ice::ObjectAdapterPtr&);

    Ice::Long getStamp(const Ice::Current&) const;

private:

    const Ice::ObjectAdapterPtr _adapter;
    const IceUtil::Time _busyTimeout;
};

class FileI : virtual public File
{
public:

    FileI(const Ice::ObjectAdapterPtr&);

protected:

    const Ice::ObjectAdapterPtr _adapter;
    const Ice::LoggerPtr _logger;
    const Ice::Int _traceLevel;
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
