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

#ifndef FREEZE_OBJECT_RECORD_ICE
#define FREEZE_OBJECT_RECORD_ICE

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
 * The evictor uses a map of [Ice::Identity] to [ObjectRecord] as its
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
