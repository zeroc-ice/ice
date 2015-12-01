// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_VIRTUAL_SHARED_H
#define ICE_VIRTUAL_SHARED_H

#include <IceUtil/Config.h>

#ifdef ICE_CPP11_MAPPING

namespace IceInternal
{

class VirtualShared : public ::std::enable_shared_from_this<VirtualShared>
{
public:

    virtual ~VirtualShared() = default;
};

}

#endif

#endif
