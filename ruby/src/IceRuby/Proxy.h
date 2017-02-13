// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_RUBY_PROXY_H
#define ICE_RUBY_PROXY_H

#include <Config.h>
#include <Ice/ProxyF.h>
#include <Ice/CommunicatorF.h>

namespace IceRuby
{

void initProxy(VALUE);
VALUE createProxy(const Ice::ObjectPrx&, VALUE = Qnil);
Ice::ObjectPrx getProxy(VALUE);
bool checkProxy(VALUE);

}

#endif
