//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_INSTRUMENTATION_F_H
#define ICE_INSTRUMENTATION_F_H

namespace Ice::Instrumentation
{

class Observer;
class CommunicatorObserver;

using ObserverPtr = ::std::shared_ptr<Observer>;
using CommunicatorObserverPtr = ::std::shared_ptr<CommunicatorObserver>;

}

#endif
