//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Instrumentation.h"

Ice::Instrumentation::Observer::~Observer() {}

Ice::Instrumentation::ThreadObserver::~ThreadObserver() {}

Ice::Instrumentation::ConnectionObserver::~ConnectionObserver() {}

Ice::Instrumentation::DispatchObserver::~DispatchObserver() {}

Ice::Instrumentation::ChildInvocationObserver::~ChildInvocationObserver() {}

Ice::Instrumentation::RemoteObserver::~RemoteObserver() {}

Ice::Instrumentation::CollocatedObserver::~CollocatedObserver() {}

Ice::Instrumentation::InvocationObserver::~InvocationObserver() {}

Ice::Instrumentation::ObserverUpdater::~ObserverUpdater() {}

Ice::Instrumentation::CommunicatorObserver::~CommunicatorObserver() {}
