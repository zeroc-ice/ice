// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_EVICTOR_STORAGE_ICE
#define FREEZE_EVICTOR_STORAGE_ICE

#include <Ice/Identity.ice>

module Freeze
{

/**
 *
 * The evictor maintains statistics about each object.
 *
 **/
struct Statistics
{
    /**
     *
     * The time the object was created, in milliseconds since Jan 1, 1970 0:00.
     *
     **/
    long creationTime;

    /**
     *
     * The time the object was last saved, in milliseconds relative to [creationTime].
     *
     **/
    long lastSaveTime;

    /**
     *
     * The average time between saves, in milliseconds.
     *
     **/
    long avgSaveTime;
};

/**
 *
 * The evictor uses a number of maps [Ice::Identity] to [ObjectRecord] as its
 * persistent storage.
 *
 **/
struct ObjectRecord
{
    Object servant;
    Statistics stats;
};

};

#endif
