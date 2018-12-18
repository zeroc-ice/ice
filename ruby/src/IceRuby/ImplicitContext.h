// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
