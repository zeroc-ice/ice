// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ObserverHelper.h>
#include <Ice/Observer.h>

using namespace IceInternal;

void
ObserverHelper::attach(const Ice::Instrumentation::ObserverPtr& observer)
{
    observer->attach();
}

void
ObserverHelper::detach(const Ice::Instrumentation::ObserverPtr& observer)
{
    observer->attach();
}

void
ObserverHelper::failed(const Ice::Instrumentation::ObserverPtr& observer, const std::string& exceptionName)
{
    observer->failed(exceptionName);
}

