// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef LINK_INFO_ICE
#define LINK_INFO_ICE

#include <IceStorm/IceStormInternal.ice>

module IceStorm
{

/**
 *
 * Used to store persistent information for Topic federation.
 *
 **/
struct LinkDB
{
    TopicLink* obj;
    LinkInfo info;
};

}; // End module IceStorm

#endif
