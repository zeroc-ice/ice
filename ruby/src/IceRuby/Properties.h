//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
