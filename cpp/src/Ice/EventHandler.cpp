//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "EventHandler.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::EventHandler::EventHandler()
    :
#if defined(ICE_USE_IOCP)
      _pending(SocketOperationNone),
      _started(SocketOperationNone),
      _completed(SocketOperationNone),
      _finish(false),
#else
      _disabled(SocketOperationNone),
#endif
      _ready(SocketOperationNone),
      _registered(SocketOperationNone)
{
}

IceInternal::EventHandler::~EventHandler() {}
