// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Logger.h>
#include <Ice/TraceLevels.h>
#include <Ice/SUdpControlChannel.h>
#include <Ice/Cryptor.h>
#include <Ice/Instance.h>

using namespace std;
using namespace Ice;

void ::IceInternal::incRef(::SecureUdp::ControlChannel* p) { p->__incRef(); }
void ::IceInternal::decRef(::SecureUdp::ControlChannel* p) { p->__decRef(); }

SecureUdp::ControlChannel::ControlChannel(IceInternal::SUdpTransceiver* transceiver,
                                          const IceInternal::InstancePtr& instance) :
                          _transceiver(transceiver),
                          _instance(instance),
                          _traceLevels(instance->traceLevels()),
                          _logger(instance->logger())
{
    assert(transceiver);

    _cryptor = new Cryptor();
}

SecureUdp::ControlChannel::~ControlChannel()
{
    unsetTransceiver();
}

void
SecureUdp::ControlChannel::unsetTransceiver()
{
    IceUtil::Mutex::Lock sync(_mutex);
    _transceiver = 0;
}

