// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SHARED_CONTEXT_H
#define ICE_SHARED_CONTEXT_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <string>
#include <map>

namespace Ice
{
typedef ::std::map< ::std::string, ::std::string> Context;
}

namespace IceInternal
{

class SharedContext : public IceUtil::Shared
{
public:

    SharedContext()
    {
    }
    
    SharedContext(const Ice::Context& val) :
        _val(val)
    {
    }

    inline const Ice::Context& getValue()
    {
        return _val;
    }

private:

    Ice::Context _val;
};
typedef IceUtil::Handle<SharedContext> SharedContextPtr;
}

#endif
