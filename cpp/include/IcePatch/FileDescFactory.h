// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PATCH_FILE_DESC_FACTORY_H
#define ICE_PATCH_FILE_DESC_FACTORY_H

#include <Ice/Ice.h>
#include <IcePatch/IcePatch.h>

namespace IcePatch
{

class ICE_PATCH_API FileDescFactory : public Ice::ObjectFactory
{
public:

    Ice::ObjectPtr create(const std::string&);
    void destroy();
};

}

#endif
