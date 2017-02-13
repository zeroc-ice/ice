// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_RUBY_IMPLICIT_CONTEXT_H
#define ICE_RUBY_IMPLICIT_CONTEXT_H

#include <Config.h>
#include <Ice/ImplicitContext.h>

namespace IceRuby
{

void initImplicitContext(VALUE);
Ice::ImplicitContextPtr getImplicitContext(VALUE);
VALUE createImplicitContext(const Ice::ImplicitContextPtr&);

}

#endif
