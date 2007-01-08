// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_PERF_H
#define ICE_GRID_PERF_H

#include <IceGrid/PerfTypes.h>
#include <Ice/Ice.h>

namespace IcePerf
{

//
// Obtain performance data for the local system/processes
//
ICE_GRID_API PerfDataSeq getPerfData(const Ice::StringSeq&);

}

#endif
