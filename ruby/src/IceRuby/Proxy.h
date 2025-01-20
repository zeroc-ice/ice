// Copyright (c) ZeroC, Inc.

#ifndef ICE_RUBY_PROXY_H
#define ICE_RUBY_PROXY_H

#include "Config.h"

#include "Ice/CommunicatorF.h"
#include "Ice/ProxyF.h"

#include <memory>

namespace IceRuby
{
    void initProxy(VALUE);
    VALUE createProxy(Ice::ObjectPrx, VALUE = Qnil);
    Ice::ObjectPrx getProxy(VALUE);
    bool checkProxy(VALUE);
}

#endif
