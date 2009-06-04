// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * The signal policy for Ice.Application signal handling.
 *
 * @see Ice.Application
 **/
public enum SignalPolicy
{
    /**
     * If a signal is received, Ice.Application reacts to the signal
     * by calling <code>destroy</code> or <code>shutdown</code> on
     * the communicator, or by calling a custom shutdown hook installed
     * by the application.
     **/
    HandleSignals,

    /**
     * Any signal that is received is not intercepted and takes the default
     * action.
     **/
    NoSignalHandling
}
