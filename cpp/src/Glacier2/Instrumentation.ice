// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h"]]

#include <Ice/Instrumentation.ice>

module Glacier2
{

module Instrumentation
{

local interface SessionObserver extends Ice::Instrumentation::Observer
{
    /**
     *
     * Notification of a forwarded request. This also implies removing
     * the event from the queue.
     *
     * @param client True if client request, false if server request.
     *
     **/
    void forwarded(bool client);

    /**
     *
     * Notification of a queued request.
     *
     * @param client True if client request, false if server request.
     *
     **/
    void queued(bool client);

    /**
     *
     * Notification of a overridden request. This implies adding and
     * removing an event to the queue.
     *
     * @param client True if client request, false if server request.
     *
     **/
    void overridden(bool client);

    /**
     *
     * Notification of a routing table size change.
     * 
     * @param delta The size adjustement.
     *
     **/
    void routingTableSize(int delta);
};

/**
 *
 * The ObserverUpdater interface is implemented by Glacier2 and an
 * instance of this interface is provided on initialization to the
 * RouterObserver object.
 *
 * This interface can be used by add-ins imlementing the
 * RouterObserver interface to update the obsevers of observed
 * objects.
 *
 **/
local interface ObserverUpdater
{
    /**
     *
     * Update the router sessions.
     *
     * When called, this method goes through all the sessions and for
     * each sesssion RouterObserver::getSessionObserver is
     * called. The implementation of getSessionObserver has the
     * possibility to return an updated observer if necessary.
     * 
     **/
    void updateSessionObservers();
};

/**
 *
 * The router observer interface used by Glacier2 to obtain and update
 * observers for its observeable objects. This interface should be
 * implemented by add-ins that wish to observe Glacier2 objects in
 * order to collect statistics.
 *
 **/
local interface RouterObserver
{
    /**
     *
     * This method should return an observer for the given session.
     *
     * @param id The id of the session (the user id or the SSL DN).
     *
     * @param con The connection associated to the session.
     *
     * @param routingTableSize The size of the routing table for this
     * session.
     *
     * @param old The previous observer, only set when updating an
     * existing observer.
     *
     **/
    SessionObserver getSessionObserver(string id, Ice::Connection con, int routingTableSize, SessionObserver old);

    /**
     *
     * Glacier2 calls this method on initialization. The add-in
     * implementing this interface can use this object to get Glacier2
     * to re-obtain observers for topics and subscribers.
     *
     * @param updater The observer updater object.
     *
     **/
    void setObserverUpdater(ObserverUpdater updater);
};

};
    
};

