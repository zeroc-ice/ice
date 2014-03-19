// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceWS/AcceptorI.h>
#include <IceWS/Instance.h>
#include <IceWS/TransceiverI.h>

using namespace std;
using namespace Ice;
using namespace IceWS;

IceInternal::NativeInfoPtr
IceWS::AcceptorI::getNativeInfo()
{
    return _delegate->getNativeInfo();
}

#ifdef ICE_USE_IOCP
IceInternal::AsyncInfo*
IceWS::AcceptorI::getAsyncInfo(IceInternal::SocketOperation status)
{
    return _delegate->getNativeInfo()->getAsyncInfo(status);
}
#endif

void
IceWS::AcceptorI::close()
{
    _delegate->close();
}

void
IceWS::AcceptorI::listen()
{
    _delegate->listen();
}

#ifdef ICE_USE_IOCP
void
IceWS::AcceptorI::startAccept()
{
    _delegate->startAccept();
}

void
IceWS::AcceptorI::finishAccept()
{
    _delegate->finishAccept();
}
#endif

IceInternal::TransceiverPtr
IceWS::AcceptorI::accept()
{
    //
    // WebSocket handshaking is performed in TransceiverI::initialize, since
    // accept must not block.
    //
    return new TransceiverI(_instance, _delegate->accept());
}

string
IceWS::AcceptorI::protocol() const
{
    return _delegate->protocol();
}

string
IceWS::AcceptorI::toString() const
{
    return _delegate->toString();
}

IceWS::AcceptorI::AcceptorI(const InstancePtr& instance, const IceInternal::AcceptorPtr& del) :
    _instance(instance), _delegate(del)
{
}

IceWS::AcceptorI::~AcceptorI()
{
}
