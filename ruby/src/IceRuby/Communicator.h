// Copyright (c) ZeroC, Inc.

#ifndef ICE_RUBY_COMMUNICATOR_H
#define ICE_RUBY_COMMUNICATOR_H

#include "Config.h"
#include "Ice/CommunicatorF.h"
#include "Ice/SliceLoader.h"

namespace IceRuby
{
    void initCommunicator(VALUE);
    Ice::CommunicatorPtr getCommunicator(VALUE);

    VALUE lookupCommunicator(const Ice::CommunicatorPtr&);
    Ice::SliceLoaderPtr lookupSliceLoader(const Ice::CommunicatorPtr&);
}

#endif
