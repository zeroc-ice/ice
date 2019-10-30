//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * A communicator plug-in. A plug-in generally adds a feature to a
 * communicator, such as support for a protocol.
 *
 * The communicator loads its plug-ins in two stages: the first stage
 * creates the plug-ins, and the second stage invokes {@link Plugin#initialize} on
 * each one.
 **/
public interface Plugin
{
    /**
     * Perform any necessary initialization steps.
     **/
    void initialize();

    /**
     * Called when the communicator is being destroyed.
     **/
    void destroy();
}
