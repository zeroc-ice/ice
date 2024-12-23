//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

import com.zeroc.Ice.SSL.SSLEngineFactory;

/**
 * A class that encapsulates data to initialize a communicator.
 *
 * @see Util#initialize
 * @see Properties
 * @see Logger
 */
public final class InitializationData implements Cloneable {
    /** Creates an instance with all members set to <code>null</code>. */
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
    public com.zeroc.Ice.Instrumentation.CommunicatorObserver observer;

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
    public java.util.function.BiConsumer<Runnable, Connection> executor;

    /** The batch request interceptor. */
    public BatchRequestInterceptor batchRequestInterceptor;

    /** The value factory manager. */
    public ValueFactoryManager valueFactoryManager;

    /**
     * The SSL engine factory used to configure the client-side ssl transport. If non-null all the
     * Ice.SSL configuration properties are ignored, and any SSL configuration must be done through
     * the SSLEngineFactory.
     */
    public SSLEngineFactory clientSSLEngineFactory;

    /**
     * A user-supplied function used to loads resources (e.g., key stores, trust stores) given a
     * resource identifier or path. If this field is non-null, the provided function is used to load
     * resources. Otherwise, the default loader (using the classpath and file system) is used.
     *
     * <p>If the function itself returns {@code null} when attempting to load a particular resource,
     * this class will automatically fall back to the default resource loader.
     */
    public java.util.function.Function<String, java.io.InputStream> resourceLoader;
}
