// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/Config.h>

#ifdef ICE_OS_WINRT

#include <IceSSL/SSLEngine.h>
#include <IceUtil/Shared.h>

IceUtil::Shared* IceSSL::upCast(IceSSL::WinRTEngine* p) { return p; }

using namespace IceSSL;

WinRTEngine::WinRTEngine(const Ice::CommunicatorPtr& communicator) : SSLEngine(communicator)
{

}

void
WinRTEngine::initialize()
{
}

bool
WinRTEngine::initialized() const
{
	return true;
}

void
WinRTEngine::destroy()
{
}

#endif

