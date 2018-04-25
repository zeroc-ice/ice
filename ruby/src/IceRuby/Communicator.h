// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
