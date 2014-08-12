// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/WSAcceptorI.h>
#include <Ice/WSTransceiverI.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::NativeInfoPtr
IceInternal::WSAcceptorI::getNativeInfo()
{
    return _delegate->getNativeInfo();
}

#if defined(ICE_USE_IOCP)
IceInternal::AsyncInfo*
IceInternal::WSAcceptorI::getAsyncInfo(IceInternal::SocketOperation status)
{
    return _delegate->getNativeInfo()->getAsyncInfo(status);
}
#elif defined(ICE_OS_WINRT)
void 
IceInternal::WSAcceptorI::setCompletedHandler(IceInternal::SocketOperationCompletedHandler^ handler)
{
    _delegate->getNativeInfo()->setCompletedHandler(handler);
}
#endif

void
IceInternal::WSAcceptorI::close()
{
    _delegate->close();
}

void
IceInternal::WSAcceptorI::listen()
{
    _delegate->listen();
}

#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
void
IceInternal::WSAcceptorI::startAccept()
{
    _delegate->startAccept();
}

void
IceInternal::WSAcceptorI::finishAccept()
{
    _delegate->finishAccept();
}
#endif

IceInternal::TransceiverPtr
IceInternal::WSAcceptorI::accept()
{
    //
    // WebSocket handshaking is performed in TransceiverI::initialize, since
    // accept must not block.
    //
    return new WSTransceiverI(_instance, _delegate->accept());
}

string
IceInternal::WSAcceptorI::protocol() const
{
    return _delegate->protocol();
}

string
IceInternal::WSAcceptorI::toString() const
{
    return _delegate->toString();
}

IceInternal::WSAcceptorI::WSAcceptorI(const ProtocolInstancePtr& instance, const IceInternal::AcceptorPtr& del) :
    _instance(instance), _delegate(del)
{
}

IceInternal::WSAcceptorI::~WSAcceptorI()
{
}
