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

namespace IcePatch
{

ICE_PATCH_API std::string pathToName(const std::string&);

class ICE_PATCH_API ProgressCB : public IceUtil::Mutex
{
public:

    ProgressCB();
    virtual ~ProgressCB();
    void abort();
    bool isAborted();

    virtual void startDownload(Ice::Int, Ice::Int) = 0;
    virtual void updateDownload(Ice::Int, Ice::Int) = 0;
    virtual void finishedDownload(Ice::Int) = 0;

    virtual void startUncompress(Ice::Int, Ice::Int) = 0;
    virtual void updateUncompress(Ice::Int, Ice::Int) = 0;
    virtual void finishedUncompress(Ice::Int) = 0;

 private:

    bool _aborted;
};

ICE_PATCH_API void getRegular(const IcePatch::RegularPrx&, ProgressCB&);

}

#endif
