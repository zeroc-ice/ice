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
