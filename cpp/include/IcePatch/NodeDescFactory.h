// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PATCH_NODE_DESC_FACTORY_H
#define ICE_PATCH_NODE_DESC_FACTORY_H

#include <Ice/Ice.h>
#include <IcePatch/Node.h>

namespace IcePatch
{

class ICE_PATCH_API NodeDescFactory : public Ice::ObjectFactory
{
public:

    Ice::ObjectPtr create(const std::string&);
    void destroy();
};

}

#endif
