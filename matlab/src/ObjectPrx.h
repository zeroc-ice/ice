// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Proxy.h>

namespace IceMatlab
{

void* createProxy(std::shared_ptr<Ice::ObjectPrx>);
std::shared_ptr<Ice::ObjectPrx> getProxy(void*);

}
