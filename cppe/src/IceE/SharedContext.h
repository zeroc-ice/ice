// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_SHARED_CONTEXT_H
#define ICEE_SHARED_CONTEXT_H

#include <IceE/Shared.h>
#include <IceE/Handle.h>
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
