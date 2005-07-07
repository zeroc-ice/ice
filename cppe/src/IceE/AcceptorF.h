// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_ACCEPTOR_F_H
#define ICEE_ACCEPTOR_F_H

#include <IceE/Handle.h>

namespace IceInternal
{

class Acceptor;
ICEE_API void incRef(Acceptor*);
ICEE_API void decRef(Acceptor*);
typedef Handle<Acceptor> AcceptorPtr;

}

#endif
