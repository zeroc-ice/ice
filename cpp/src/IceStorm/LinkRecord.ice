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

#include <Ice/Identity.ice>
#include <IceStorm/IceStormInternal.ice>

module IceStorm
{

/**
 *
 * Used to store persistent information for Topic federation.
 *
 **/
struct LinkRecord
{
    TopicLink* obj; // The topic link object.
    int cost; // The cost.
    Topic* theTopic;  // The linked topic for getLinkInfoSeq
};

}; // End module IceStorm
