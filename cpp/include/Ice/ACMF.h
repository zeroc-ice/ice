// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ACM_F_H
#define ICE_ACM_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class ACMMonitor;
class FactoryACMMonitor;

#ifdef ICE_CPP11_MAPPING
typedef ::std::shared_ptr<ACMMonitor> ACMMonitorPtr;
typedef ::std::shared_ptr<FactoryACMMonitor> FactoryACMMonitorPtr;
#else
ICE_API IceUtil::Shared* upCast(ACMMonitor*);
typedef IceInternal::Handle<ACMMonitor> ACMMonitorPtr;

ICE_API IceUtil::Shared* upCast(FactoryACMMonitor*);
typedef IceInternal::Handle<FactoryACMMonitor> FactoryACMMonitorPtr;
#endif

}

#endif
