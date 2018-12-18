// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_RUBY_COMMUNICATOR_H
#define ICE_RUBY_COMMUNICATOR_H

#include <Config.h>
#include <Ice/CommunicatorF.h>

namespace IceRuby
{

void initCommunicator(VALUE);
Ice::CommunicatorPtr getCommunicator(VALUE);
VALUE lookupCommunicator(const Ice::CommunicatorPtr&);

}

#endif
