// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.Ice.Instrumentation.CommunicatorObserver;
import com.zeroc.Ice.SSL.SSLEngineFactory;

import java.util.Collections;
import java.util.List;
import java.util.function.BiConsumer;

/**
 * Represents a set of options that you can specify when initializing a communicator.
 *
 * @see Communicator#Communicator(InitializationData)
 */
public final class InitializationData implements Cloneable {
    /** Creates an InitializationData with all fields set to {@code null} or the empty list. */
    public InitializationData() {}

    /**
     * Creates and returns a copy of this {@code InitializationData}.
     *
     * @return a copy of this object
     */
    @Override
    public InitializationData clone() {
        // A member-wise copy is safe because the members are immutable.
        InitializationData c = null;
        try {
            c = (InitializationData) super.clone();
        } catch (CloneNotSupportedException ex) {
            assert false;
        }
        return c;
    }

    /**
     * The properties for the communicator.
     * When non-null, this field corresponds to the object returned by {@link Communicator#getProperties()}.
     */
    public Properties properties;

    /** The logger for the communicator. */
    public Logger logger;

    /** The communicator observer used by the Ice runtime. */
    public CommunicatorObserver observer;

    /** {@code threadStart} is called whenever the communicator starts a new thread. */
    public Runnable threadStart;

    /** {@code threadStop} is called whenever a thread created by the communicator is about to be destroyed. */
    public Runnable threadStop;

    /**
     * The custom class loader for the communicator.
     * Applications can supply a custom class loader that the Ice runtime will use when unmarshaling class
     * instances and user exceptions, and when loading plug-ins.
     *
     * <p>If a custom {@link #sliceLoader} is provided, then that will be used to unmarshal class instances and
     * user exceptions, instead of this class loader.
     *
     * <p>If an application does not supply a class loader (or if the application-supplied class loader fails to locate
     * a class), the Ice runtime attempts to load the class using class loaders in the following order:
     * <ul>
     *  <li>the current thread's class loader</li>
     *  <li>the default class loader (that is, by calling Class.forName)</li>
     *  <li>the system class loader</li>
     * </ul>
     */
    public ClassLoader classLoader;

    /**
     * You can control which thread receives operation dispatches and async invocation callbacks by
     * supplying an executor. For example, you can use this execution facility to ensure that all
     * dispatches and invocation callbacks are executed in a GUI event loop thread so that it is
     * safe to invoke directly on GUI objects.
     *
     * <p>The executor is responsible for running the dispatch or async invocation callback on its
     * favorite thread. It must execute the provided {@code Runnable} parameter. The {@code Connection}
     * parameter represents the connection associated with this call.
     */
    public BiConsumer<Runnable, Connection> executor;

    /** The batch request interceptor. */
    public BatchRequestInterceptor batchRequestInterceptor;

    /**
     * The SSL engine factory used to configure the client-side ssl transport. If non-null, all the
     * IceSSL configuration properties are ignored, and any SSL configuration must be done through
     * the SSLEngineFactory.
     */
    public SSLEngineFactory clientSSLEngineFactory;

    /**
     * A list of plug-in factories. The corresponding plug-ins are created during communicator initialization,
     * in order, before all other plug-ins.
     */
    public List<PluginFactory> pluginFactories = Collections.emptyList();

    /**
     * The Slice loader, used to create instances of Slice classes and user exceptions.
     * Applications can supply a custom slice loader that the Ice runtime will use during unmarshaling.
     */
    public SliceLoader sliceLoader;
}
