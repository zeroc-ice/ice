// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STREAM_F_H
#define ICE_STREAM_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace Ice
{

class InputStream;
ICE_API IceUtil::Shared* upCast(::Ice::InputStream*);
typedef IceInternal::Handle< InputStream > InputStreamPtr;

class OutputStream;
ICE_API IceUtil::Shared* upCast(::Ice::OutputStream*);
typedef IceInternal::Handle< OutputStream > OutputStreamPtr;

}

#endif
