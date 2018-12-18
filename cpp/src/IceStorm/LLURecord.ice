// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
}

}
