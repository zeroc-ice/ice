// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef LINK_RECORD_ICE
#define LINK_RECORD_ICE

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
    TopicLink* obj;
    int cost;

    //
    // Only for Topic::getLinkInfoSeq()
    //
    Topic* theTopic; 
};

dictionary<string, LinkRecord> LinkRecordDict;

}; // End module IceStorm

#endif
