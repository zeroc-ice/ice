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

#ifndef ICE_SSL_API
#   ifdef ICE_SSL_API_EXPORTS
#       define ICE_SSL_API ICE_DECLSPEC_EXPORT
#    else
#       define ICE_SSL_API ICE_DECLSPEC_IMPORT
#    endif
#endif

namespace IceSSL
{

class Instance;

}

namespace IceInternal
{

ICE_SSL_API void incRef(IceSSL::Instance*);
ICE_SSL_API void decRef(IceSSL::Instance*);

}

namespace IceSSL
{

typedef IceInternal::Handle<Instance> InstancePtr;

}

#endif
