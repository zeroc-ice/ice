// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc"]]

#include <Ice/Identity.ice>

module Freeze
{

/**
 *
 *  Evictors maintain statistics about each object, when using Ice encoding version 1.0.
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
     * The time the object was last saved, in milliseconds relative to <tt>creationTime</tt>.
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
 * ObjectRecord is the value-type for the persistent maps maintained by evictors 
 * when using Ice encoding version is 1.0.
 *
 **/
struct ObjectRecord
{
    /**
     *
     * The servant implementing the object.
     *
     **/
    Object servant;

    /**
     *
     * The statistics for the object. 
     *
     **/
    Statistics stats;
};

};

