// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
