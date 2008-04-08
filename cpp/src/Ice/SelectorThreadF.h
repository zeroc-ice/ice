// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SELECTOR_THREAD_F_H
#define ICE_SELECTOR_THREAD_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class SelectorThread;
ICE_API IceUtil::Shared* upCast(SelectorThread*);
typedef Handle<SelectorThread> SelectorThreadPtr;

}

#endif
