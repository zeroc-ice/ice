//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_MUTEX_PROTOCOL_H
#define ICE_UTIL_MUTEX_PROTOCOL_H

#include <IceUtil/Config.h>

namespace IceUtil
{

enum MutexProtocol
{
    PrioInherit,
    PrioNone
};

ICE_API MutexProtocol getDefaultMutexProtocol();

} // End namespace IceUtil

#endif
