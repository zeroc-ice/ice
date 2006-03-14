// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class PlatformInfo
{
public:

    PlatformInfo(const Ice::CommunicatorPtr&, const TraceLevelsPtr&);
    ~PlatformInfo();

    NodeInfo getNodeInfo() const;
    LoadInfo getLoadInfo();
    std::string getHostname() const;
    std::string getDataDir() const;

private:

#if defined(_WIN32)
    void initQuery();
#endif    

    const TraceLevelsPtr _traceLevels;
    NodeInfo _info;
    std::string _hostname;

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
