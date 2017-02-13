// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h"]]

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

