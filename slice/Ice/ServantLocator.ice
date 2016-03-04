// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc"]]

#include <Ice/ObjectAdapterF.ice>
#include <Ice/Current.ice>

["objc:prefix:ICE"]
module Ice
{

/**
 *
 * A servant locator is called by an object adapter to
 * locate a servant that is not found in its active servant map.
 *
 * @see ObjectAdapter
 * @see ObjectAdapter#addServantLocator
 * @see ObjectAdapter#findServantLocator
 *
 **/
local interface ServantLocator
{
    /**
     *
     * Called before a request is dispatched if a
     * servant cannot be found in the object adapter's active servant
     * map. Note that the object adapter does not automatically insert
     * the returned servant into its active servant map. This must be
     * done by the servant locator implementation, if this is desired.
     *
     * <tt>locate</tt> can throw any user exception. If it does, that exception
     * is marshaled back to the client. If the Slice definition for the
     * corresponding operation includes that user exception, the client
     * receives that user exception; otherwise, the client receives
     * {@link UnknownUserException}.
     *
     * If <tt>locate</tt> throws any exception, the Ice run time does <em>not</em>
     * call <tt>finished</tt>.
     *
     * <p class="Note">If you call <tt>locate</tt> from your own code, you
     * must also call <tt>finished</tt> when you have finished using the
     * servant, provided that <tt>locate</tt> returned a non-null servant;
     * otherwise, you will get undefined behavior if you use
     * servant locators such as the Freeze Evictor.
     *
     * @param curr Information about the current operation for which
     * a servant is required.
     *
     * @param cookie A "cookie" that will be passed to <tt>finished</tt>.
     *
     * @return The located servant, or null if no suitable servant has
     * been found.
     *
     * @see ObjectAdapter
     * @see Current
     * @see #finished
     *
     **/
    ["UserException"] Object locate(Current curr, out LocalObject cookie);

    /**
     *
     * Called by the object adapter after a request has been
     * made. This operation is only called if <tt>locate</tt> was called
     * prior to the request and returned a non-null servant. This
     * operation can be used for cleanup purposes after a request.
     *
     * <tt>finished</tt> can throw any user exception. If it does, that exception
     * is marshaled back to the client. If the Slice definition for the
     * corresponding operation includes that user exception, the client
     * receives that user exception; otherwise, the client receives
     * {@link UnknownUserException}.
     *
     * If both the operation and <tt>finished</tt> throw an exception, the
     * exception thrown by <tt>finished</tt> is marshaled back to the client.
     *
     * @param curr Information about the current operation call for
     * which a servant was located by <tt>locate</tt>.
     *
     * @param servant The servant that was returned by <tt>locate</tt>.
     *
     * @param cookie The cookie that was returned by <tt>locate</tt>.
     *
     * @see ObjectAdapter
     * @see Current
     * @see #locate
     *
     **/
    ["UserException"] void finished(Current curr, Object servant, LocalObject cookie);

    /**
     *
     * Called when the object adapter in which this servant locator is
     * installed is destroyed.
     *
     * @param category Indicates for which category the servant locator
     * is being deactivated.
     *
     * @see ObjectAdapter#destroy
     * @see Communicator#shutdown
     * @see Communicator#destroy
     *
     **/
    void deactivate(string category);
};

};

