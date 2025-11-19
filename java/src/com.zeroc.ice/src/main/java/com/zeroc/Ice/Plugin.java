// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Represents a communicator plug-in. A plug-in generally adds a feature to a communicator,
 * such as support for an additional transport. The communicator loads its plug-ins in two stages:
 * the first stage creates the plug-ins, and the second stage calls {@link Plugin#initialize} on each one.
 */
public interface Plugin {
    /** Performs any necessary initialization steps. */
    void initialize();

    /** Destroys this plugin. This method is called when the communicator is destroyed. */
    void destroy();
}
