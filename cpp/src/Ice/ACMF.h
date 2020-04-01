//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ACM_F_H
#define ICE_ACM_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace IceInternal
{

class ACMMonitor;
class FactoryACMMonitor;

using ACMMonitorPtr = ::std::shared_ptr<ACMMonitor>;
using FactoryACMMonitorPtr = ::std::shared_ptr<FactoryACMMonitor>;

}

#endif
