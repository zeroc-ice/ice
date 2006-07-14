// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GC_COUNTMAP_H
#define ICE_GC_COUNTMAP_H

#include <map>

namespace IceInternal
{

class GCShared;

typedef ::std::map<GCShared*, int> GCCountMap;

}

#endif
