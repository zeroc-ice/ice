// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_REQUEST_QUEUE_ICE
#define ICE_REQUEST_QUEUE_ICE

#include <Ice/ObjectAdapterF.ice>
#include <Ice/Current.ice>

module Ice
{

/**
 *
 * The Request Queue, created and managed by Object Adapters. Request
 * Queues buffer requests, and allow to dispatch all such buffered
 * requests manually in an application-supplied thread.
 *
 * @see ObjectAdapter
 * @see ObjectAdapter::createRequestQueue
 * @see ObjectAdapter::findRequestQueue
 *
 **/
local interface RequestQueue
{
    /**
     *
     * Dispatch all buffered requests. The dispatch thread is the
     * thread that calls this operation. Potential exceptions are not
     * returned to the caller, but handled just in the same manner as
     * if the Ice core would have dispatched the requests.
     *
     **/
    void dispatch();

    /**
     *
     * Destroy this request queue, and remove it from its Object
     * Adapter.
     *
     * @param dispatch If set to true, all requests still buffered in
     * the queue will be dispatched before destruction, so that no
     * requests can get lost. The behavior is the same as if
     * <literal>dispatch()</literal> and
     * <literal>destroy(false)</literal> would be called atomically.
     *
     **/
    void destroy(bool dispatch);
};

};

#endif
