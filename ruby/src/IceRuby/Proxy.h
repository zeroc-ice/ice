// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
