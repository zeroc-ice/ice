// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PATCH_CLIENT_UTIL_H
#define ICE_PATCH_CLIENT_UTIL_H

#include <Ice/Ice.h>
#include <IcePatch/IcePatch.h>
#include <IceUtil/Exception.h>

namespace IcePatch
{

class ICE_PATCH_API AbortException : public IceUtil::Exception
{
public:
    AbortException(const char*, int);
    virtual std::string ice_name() const;
    virtual IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class ICE_PATCH_API ProgressCB
{
public:
    virtual void startDownload(Ice::Int, Ice::Int) = 0;
    virtual void updateDownload(Ice::Int, Ice::Int) = 0;
    virtual void finishedDownload(Ice::Int) = 0;

    virtual void startUncompress(Ice::Int, Ice::Int) = 0;
    virtual void updateUncompress(Ice::Int, Ice::Int) = 0;
    virtual void finishedUncompress(Ice::Int) = 0;
};

ICE_PATCH_API std::string pathToName(const std::string&);
ICE_PATCH_API void getRegular(const IcePatch::RegularPrx&, ProgressCB&);

}

#endif
