// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_INSTANCE_F_H
#define ICE_SSL_INSTANCE_F_H

#include <Ice/Handle.h>

namespace IceSSL
{

class Instance;

}

namespace IceInternal
{

void incRef(IceSSL::Instance*);
void decRef(IceSSL::Instance*);

}

namespace IceSSL
{

typedef IceInternal::Handle<Instance> InstancePtr;

}

#endif
