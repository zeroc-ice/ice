// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    FileI(const Ice::ObjectAdapterPtr&, const std::string&);

    virtual Ice::SliceChecksumDict getSliceChecksums(const Ice::Current&) const;

protected:

    Ice::ByteSeq readMD5(const Ice::Current&) const;

    const Ice::ObjectAdapterPtr _adapter;
    const Ice::LoggerPtr _fileTraceLogger;
    const IceUtil::Time _busyTimeout;
    const std::string _dir;
};

class DirectoryI : virtual public Directory,
		   virtual public FileI
{
public:

    DirectoryI(const Ice::ObjectAdapterPtr&, const std::string&);

    virtual FileDescPtr describe(const Ice::Current&) const;
    virtual FileDescSeq getContents(const Ice::Current&) const;
    virtual Ice::Long getTotal(const Ice::ByteSeq&, const Ice::Current&) const;
};

class RegularI : virtual public Regular,
                 virtual public FileI
{
public:

    RegularI(const Ice::ObjectAdapterPtr&, const std::string&);

    virtual FileDescPtr describe(const Ice::Current&) const;
    virtual Ice::Int getBZ2Size(const Ice::Current&) const;
    virtual Ice::ByteSeq getBZ2(Ice::Int, Ice::Int, const Ice::Current&) const;
    virtual Ice::ByteSeq getBZ2MD5(Ice::Int, const Ice::Current&) const;
};

}

#endif
