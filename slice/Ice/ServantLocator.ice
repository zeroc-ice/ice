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
     * {@link #locate} can throw any user exception. If it does, that exception
     * is marshaled back to the client. If the Slice definition for the
     * corresponding operation includes that user exception, the client
     * receives that user exception; otherwise, the client receives
     * {@link UnknownUserException}.
     *
     * If {@link #locate} throws any exception, the Ice run time does <em>not</em>
     * call {@link finished}.
     *
     * <p class="Note">If you call {@link #locate} from your own code, you
     * must also call {@link #finished} when you have finished using the
     * servant, provided that {@link #locate} returned a non-null servant;
     * otherwise, you will get undefined behavior if you use
     * servant locators such as the {@link Freeze.Evictor}.
     *
     * @param curr Information about the current operation for which
     * a servant is required.
     *
     * @param cookie A "cookie" that will be passed to {@link #finished}.
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
     * made. This operation is only called if {@link #locate} was called
     * prior to the request and returned a non-null servant. This
     * operation can be used for cleanup purposes after a request.
     *
     * {@link finished} can throw any user exception. If it does, that exception
     * is marshaled back to the client. If the Slice definition for the
     * corresponding operation includes that user exception, the client
     * receives that user exception; otherwise, the client receives
     * {@link UnknownUserException}.
     *
     * If both the operation and {@link #finished} throw an exception, the
     * exception thrown by {@link #finished} is marshaled back to the client.
     *
     * @param curr Information about the current operation call for
     * which a servant was located by {@link #locate}.
     *
     * @param servant The servant that was returned by {@link #locate}.
     *
     * @param cookie The cookie that was returned by {@link #locate}.
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

