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

    /**
     * Applications that make use of compact type IDs to conserve space when marshaling class
     * instances, and also use the streaming API to extract such classes, can intercept the
     * translation between compact type IDs and their corresponding string type IDs by installing a
     * compact ID resolver in <code>InitializationData</code> The parameter id represents the
     * compact ID; the returned value is the type ID such as <code>"::Module::Class"</code>, or an
     * empty string if the compact ID is unknown.
     */
    public java.util.function.IntFunction<String> compactIdResolver;

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
}
