// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef ICE_PATCH_CLIENT_UTIL_H
#define ICE_PATCH_CLIENT_UTIL_H

#include <Ice/Ice.h>
#include <IcePatch/IcePatch.h>
#include <IceUtil/Exception.h>

namespace IcePatch
{

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
