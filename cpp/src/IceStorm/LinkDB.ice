// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
