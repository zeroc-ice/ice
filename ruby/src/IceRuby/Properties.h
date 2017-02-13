// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_RUBY_PROPERTIES_H
#define ICE_RUBY_PROPERTIES_H

#include <Config.h>
#include <Ice/PropertiesF.h>

namespace IceRuby
{

void initProperties(VALUE);
Ice::PropertiesPtr getProperties(VALUE);
VALUE createProperties(const Ice::PropertiesPtr&);

}

#endif
