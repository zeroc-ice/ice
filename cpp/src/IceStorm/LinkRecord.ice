// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
    TopicLink* obj; // The topic link object.
    int cost; // The cost.
    Topic* theTopic;  // The linked topic for getLinkInfoSeq
};

/** Dictionary of link name to link record. */
dictionary<string, LinkRecord> LinkRecordDict;

}; // End module IceStorm

#endif
