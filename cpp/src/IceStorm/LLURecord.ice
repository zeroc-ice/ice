// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef LLURECORD_ICE
#define LLURECORD_ICE

[["cpp:header-ext:h"]]

module IceStormElection
{

/** A struct used for marking the last log update. */
struct LogUpdate
{
    /** The generation. */
    long generation;
    /** The iteration within this generation. */
    long iteration;
};

};

#endif
