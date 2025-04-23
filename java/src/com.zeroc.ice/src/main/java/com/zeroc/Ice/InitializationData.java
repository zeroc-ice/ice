// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.Ice.Instrumentation.CommunicatorObserver;
import com.zeroc.Ice.SSL.SSLEngineFactory;

import java.util.Collections;
import java.util.List;
import java.util.function.BiConsumer;

/**
 * A class that encapsulates data to initialize a communicator.
 *
 * @see Util#initialize
 * @see Properties
 * @see Logger
 */
public final class InitializationData implements Cloneable {
    /** Creates an instance with all members set to <code>null</code> or empty. */
    public InitializationData() {}

    /** Creates and returns a copy of this object. */
    @Override
    public InitializationData clone() {
        //
        // A member-wise copy is safe because the members are immutable.
        //
        InitializationData c = null;
        try {
            c = (InitializationData) super.clone();
        } catch (CloneNotSupportedException ex) {
            assert false;
        }
        return c;
    }

    /** The properties for the communicator. */
    public Properties properties;

    /** The logger for the communicator. */
    public Logger logger;

    /** The communicator observer used by the Ice run-time. */
    public CommunicatorObserver observer;

    /** threadStart is called whenever the communicator starts a new thread. */
    public Runnable threadStart;

    /**
     * threadStop is called whenever a thread created by the communicator is about to be destroyed.
     */
    public Runnable threadStop;

    /** The custom class loader for the communicator. */
    public ClassLoader classLoader;

    /**
     * You can control which thread receives operation dispatches and async invocation callbacks by
     * supplying an executor. For example, you can use this execution facility to ensure that all
     * dispatches and invocation callbacks are executed in a GUI event loop thread so that it is
     * safe to invoke directly on GUI objects.
     *
     * <p>The executor is responsible for running the dispatch or async invocation callback on its
     * favorite thread. It must execute the the provided <code>Runnable</code> parameter. The con
     * parameter represents the connection associated with this call.
     */
    public BiConsumer<Runnable, Connection> executor;

    /** The batch request interceptor. */
    public BatchRequestInterceptor batchRequestInterceptor;

    /**
     * The SSL engine factory used to configure the client-side ssl transport. If non-null all the
     * Ice.SSL configuration properties are ignored, and any SSL configuration must be done through
     * the SSLEngineFactory.
     */
    public SSLEngineFactory clientSSLEngineFactory;

    /**
     * The plug-in factories. The corresponding plug-ins are created during communicator initialization,
     * in order, before all other plug-ins.
     */
    public List<PluginFactory> pluginFactories = Collections.emptyList();

    /**
     * The Slice loader, used to unmarshal Slice classes and exceptions.
     */
    public SliceLoader sliceLoader;
}
