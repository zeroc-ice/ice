// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SUdpControlChannel.h>
#include <Ice/Instance.h>

using namespace std;
using namespace Ice;

void IceSecurity::SecureUdp::incRef(ControlChannel* p) { p->__incRef(); }
void IceSecurity::SecureUdp::decRef(ControlChannel* p) { p->__decRef(); }

IceSecurity::SecureUdp::ControlChannel::ControlChannel(const SUdpTransceiverPtr& transceiver,
                                                       const InstancePtr& instance) :
                                       _transceiver(transceiver),
                                       _instance(instance)
{
    assert(transceiver);
}

IceSecurity::SecureUdp::ControlChannel::~ControlChannel() 
{
    unsetTransceiver();
}

void
IceSecurity::SecureUdp::ControlChannel::unsetTransceiver()
{
    IceUtil::Mutex::Lock sync(_mutex);
    _transceiver = 0;
}

