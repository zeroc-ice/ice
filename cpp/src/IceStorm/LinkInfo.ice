// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
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
 * Use to store persistent information on Topic federation.
 *
 **/
struct LinkInfo
{
    TopicLink* obj;
    int cost;
};

}; // End module IceStorm

#endif
