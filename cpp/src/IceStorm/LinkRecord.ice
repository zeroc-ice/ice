// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
    TopicUpstreamLink* upstream; // The upstream link (for recreating
				 // the upstream link object).
    Topic* theTopic;  // The linked topic for getLinkInfoSeq
};

/** Dictionary of link name to link record. */
dictionary<string, LinkRecord> LinkRecordDict;

/** Sequence of TopicUpstreamLink objects. */
sequence<TopicUpstreamLink*> TopicUpstreamLinkPrxSeq;

}; // End module IceStorm

#endif
