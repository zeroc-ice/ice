// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_PLATFORM_INFO_H
#define ICE_GRID_PLATFORM_INFO_H

#include <IceGrid/Internal.h>

#ifdef _WIN32
#   include <pdh.h> // Performance data helper API
#   include <deque>
#endif

namespace IceGrid
{

class PlatformInfo
{
public:

    PlatformInfo();
    ~PlatformInfo();

    NodeInfo getNodeInfo() const;
    LoadInfo getLoadInfo();

private:

    NodeInfo _info;

#if defined(_WIN32)
    HQUERY _query;
    HCOUNTER _counter;
    std::deque<int> _usages1;
    std::deque<int> _usages5;
    std::deque<int> _usages15;
    int _last1Total;
    int _last5Total;
    int _last15Total;
#elif defined(_AIX)
    int _kmem;
#endif

};


};

#endif
