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


#ifndef ICE_ACCEPTOR_F_H
#define ICE_ACCEPTOR_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class Acceptor;
ICE_PROTOCOL_API void incRef(Acceptor*);
ICE_PROTOCOL_API void decRef(Acceptor*);
typedef Handle<Acceptor> AcceptorPtr;

}

#endif
