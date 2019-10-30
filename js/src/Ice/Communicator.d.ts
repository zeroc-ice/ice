//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

export namespace Ice
{
    /**
     * The central object in Ice. One or more communicators can be
     * instantiated for an Ice application. Communicator instantiation
     * is language-specific, and not specified in Slice code.
     * @see Logger
     * @see ObjectAdapter
     * @see Properties
     * @see ValueFactory
     */
    interface Communicator
    {
        /**
         * Destroy the communicator. This operation calls {@link #shutdown}
         * implicitly.  Calling {@link #destroy} cleans up memory, and shuts down
         * this communicator's client functionality and destroys all object
         * adapters. Subsequent calls to {@link #destroy} are ignored.
         * @return @returns The asynchronous result object for the invocation.
         * @see #shutdown
         * @see ObjectAdapter#destroy
         */
        destroy():AsyncResultBase<void>;
        /**
         * Shuts down this communicator's server functionality, which
         * includes the deactivation of all object adapters. Attempts to use a
         * deactivated object adapter raise ObjectAdapterDeactivatedException.
         * Subsequent calls to shutdown are ignored.
         *
         * After shutdown returns, no new requests are processed. However, requests
         * that have been started before shutdown was called might still be active.
         * You can use {@link #waitForShutdown} to wait for the completion of all
         * requests.
         * @return @returns The asynchronous result object for the invocation.
         * @see #destroy
         * @see #waitForShutdown
         * @see ObjectAdapter#deactivate
         */
        shutdown():AsyncResultBase<void>;
        /**
         * Wait until the application has called {@link #shutdown} (or {@link #destroy}).
         * On the server side, this operation blocks the calling thread
         * until all currently-executing operations have completed.
         * On the client side, the operation simply blocks until another
         * thread has called {@link #shutdown} or {@link #destroy}.
         *
         * A typical use of this operation is to call it from the main thread,
         * which then waits until some other thread calls {@link #shutdown}.
         * After shut-down is complete, the main thread returns and can do some
         * cleanup work before it finally calls {@link #destroy} to shut down
         * the client functionality, and then exits the application.
         * @return @returns The asynchronous result object for the invocation.
         * @see #shutdown
         * @see #destroy
         * @see ObjectAdapter#waitForDeactivate
         */
        waitForShutdown():AsyncResultBase<void>;
        /**
         * Check whether communicator has been shut down.
         * @return True if the communicator has been shut down; false otherwise.
         * @see #shutdown
         */
        isShutdown():boolean;
        /**
         * Convert a stringified proxy into a proxy. For example,
         * <code>MyCategory/MyObject:tcp -h some_host -p
         * 10000</code> creates a proxy that refers to the Ice object
         * having an identity with a name "MyObject" and a category
         * "MyCategory", with the server running on host "some_host", port
         * 10000. If the stringified proxy does not parse correctly, the
         * operation throws one of ProxyParseException, EndpointParseException,
         * or IdentityParseException. Refer to the Ice manual for a detailed
         * description of the syntax supported by stringified proxies.
         * @param str The stringified proxy to convert into a proxy.
         * @return The proxy, or nil if <code>str</code> is an empty string.
         * @see #proxyToString
         */
        stringToProxy(str:string):Ice.ObjectPrx;
        /**
         * Convert a proxy into a string.
         * @param obj The proxy to convert into a stringified proxy.
         * @return The stringified proxy, or an empty string if
         * <code>obj</code> is nil.
         * @see #stringToProxy
         */
        proxyToString(obj:Ice.ObjectPrx):string;
        /**
         * Convert a set of proxy properties into a proxy. The "base"
         * name supplied in the <code>property</code> argument refers to a
         * property containing a stringified proxy, such as
         * <code>MyProxy=id:tcp -h localhost -p 10000</code>. Additional
         * properties configure local settings for the proxy, such as
         * <code>MyProxy.PreferSecure=1</code>. The "Properties"
         * appendix in the Ice manual describes each of the supported
         * proxy properties.
         * @param property The base property name.
         * @return The proxy.
         */
        propertyToProxy(property:string):Ice.ObjectPrx;
        /**
         * Convert a proxy to a set of proxy properties.
         * @param proxy The proxy.
         * @param property The base property name.
         * @return The property set.
         */
        proxyToProperty(proxy:Ice.ObjectPrx, property:string):PropertyDict;
        /**
         * Convert a string into an identity. If the string does not parse
         * correctly, the operation throws IdentityParseException.
         * @param str The string to convert into an identity.
         * @return The identity.
         * @see #identityToString
         *
         * @deprecated stringToIdentity() is deprecated, use the static stringToIdentity() method instead.
         */
        stringToIdentity(str:string):Identity;
        /**
         * Convert an identity into a string.
         * @param ident The identity to convert into a string.
         * @return The "stringified" identity.
         * @see #stringToIdentity
         */
        identityToString(ident:Identity):string;
        /**
         * Create a new object adapter. The endpoints for the object
         * adapter are taken from the property <code><em>name</em>.Endpoints</code>.
         *
         * It is legal to create an object adapter with the empty string as
         * its name. Such an object adapter is accessible via bidirectional
         * connections or by collocated invocations that originate from the
         * same communicator as is used by the adapter.
         *
         * Attempts to create a named object adapter for which no configuration
         * can be found raise InitializationException.
         * @param name The object adapter name.
         * @return @returns The asynchronous result object for the invocation.
         * @see #createObjectAdapterWithEndpoints
         * @see ObjectAdapter
         * @see Properties
         */
        createObjectAdapter(name:string):AsyncResultBase<Ice.ObjectAdapter>;
        /**
         * Create a new object adapter with endpoints. This operation sets
         * the property <code><em>name</em>.Endpoints</code>, and then calls
         * {@link #createObjectAdapter}. It is provided as a convenience
         * function.
         *
         * Calling this operation with an empty name will result in a
         * UUID being generated for the name.
         * @param name The object adapter name.
         * @param endpoints The endpoints for the object adapter.
         * @return @returns The asynchronous result object for the invocation.
         * @see #createObjectAdapter
         * @see ObjectAdapter
         * @see Properties
         */
        createObjectAdapterWithEndpoints(name:string, endpoints:string):AsyncResultBase<Ice.ObjectAdapter>;
        /**
         * Create a new object adapter with a router. This operation
         * creates a routed object adapter.
         *
         * Calling this operation with an empty name will result in a
         * UUID being generated for the name.
         * @param name The object adapter name.
         * @param rtr The router.
         * @return @returns The asynchronous result object for the invocation.
         * @see #createObjectAdapter
         * @see ObjectAdapter
         * @see Properties
         */
        createObjectAdapterWithRouter(name:string, rtr:RouterPrx):AsyncResultBase<Ice.ObjectAdapter>;
        /**
         * Add an object factory to this communicator. Installing a
         * factory with an id for which a factory is already registered
         * throws AlreadyRegisteredException.
         *
         * When unmarshaling an Ice object, the Ice run time reads the
         * most-derived type id off the wire and attempts to create an
         * instance of the type using a factory. If no instance is created,
         * either because no factory was found, or because all factories
         * returned nil, the behavior of the Ice run time depends on the
         * format with which the object was marshaled:
         *
         * If the object uses the "sliced" format, Ice ascends the class
         * hierarchy until it finds a type that is recognized by a factory,
         * or it reaches the least-derived type. If no factory is found that
         * can create an instance, the run time throws NoValueFactoryException.
         *
         * If the object uses the "compact" format, Ice immediately raises
         * NoValueFactoryException.
         *
         * The following order is used to locate a factory for a type:
         *
         * <ol>
         *
         * <li>The Ice run-time looks for a factory registered
         * specifically for the type.</li>
         *
         * <li>If no instance has been created, the Ice run-time looks
         * for the default factory, which is registered with an empty type id.
         * </li>
         *
         * <li>If no instance has been created by any of the preceding
         * steps, the Ice run-time looks for a factory that may have been
         * statically generated by the language mapping for non-abstract classes.
         * </li>
         *
         * </ol>
         * @param factory The factory to add.
         * @param id The type id for which the factory can create instances, or
         * an empty string for the default factory.
         * @see #findObjectFactory
         * @see ObjectFactory
         * @see ValueFactoryManager#add
         *
         * @deprecated addObjectFactory() is deprecated, use ValueFactoryManager::add() instead.
         */
        addObjectFactory(factory:Ice.ObjectFactory, id:string):void;
        /**
         * Find an object factory registered with this communicator.
         * @param id The type id for which the factory can create instances,
         * or an empty string for the default factory.
         * @return The object factory, or null if no object factory was
         * found for the given id.
         * @see #addObjectFactory
         * @see ObjectFactory
         * @see ValueFactoryManager#find
         *
         * @deprecated findObjectFactory() is deprecated, use ValueFactoryManager::find() instead.
         */
        findObjectFactory(id:string):Ice.ObjectFactory;
        /**
         * Get the implicit context associated with this communicator.
         * @return The implicit context associated with this communicator;
         * returns null when the property Ice.ImplicitContext is not set
         * or is set to None.
         */
        getImplicitContext():Ice.ImplicitContext;
        /**
         * Get the properties for this communicator.
         * @return This communicator's properties.
         * @see Properties
         */
        getProperties():Ice.Properties;
        /**
         * Get the logger for this communicator.
         * @return This communicator's logger.
         * @see Logger
         */
        getLogger():Ice.Logger;
        /**
         * Get the default router this communicator.
         * @return The default router for this communicator.
         * @see #setDefaultRouter
         * @see Router
         */
        getDefaultRouter():RouterPrx;
        /**
         * Set a default router for this communicator. All newly
         * created proxies will use this default router. To disable the
         * default router, null can be used. Note that this
         * operation has no effect on existing proxies.
         *
         * You can also set a router for an individual proxy
         * by calling the operation <code>ice_router</code> on the proxy.
         * @param rtr The default router to use for this communicator.
         * @see #getDefaultRouter
         * @see #createObjectAdapterWithRouter
         * @see Router
         */
        setDefaultRouter(rtr:RouterPrx):void;
        /**
         * Get the default locator this communicator.
         * @return The default locator for this communicator.
         * @see #setDefaultLocator
         * @see Locator
         */
        getDefaultLocator():LocatorPrx;
        /**
         * Set a default Ice locator for this communicator. All newly
         * created proxy and object adapters will use this default
         * locator. To disable the default locator, null can be used.
         * Note that this operation has no effect on existing proxies or
         * object adapters.
         *
         * You can also set a locator for an individual proxy by calling the
         * operation <code>ice_locator</code> on the proxy, or for an object adapter
         * by calling {@link ObjectAdapter#setLocator} on the object adapter.
         * @param loc The default locator to use for this communicator.
         * @see #getDefaultLocator
         * @see Locator
         * @see ObjectAdapter#setLocator
         */
        setDefaultLocator(loc:LocatorPrx):void;
        /**
         * Get the value factory manager for this communicator.
         * @return This communicator's value factory manager.
         * @see ValueFactoryManager
         */
        getValueFactoryManager():Ice.ValueFactoryManager;
        /**
         * Flush any pending batch requests for this communicator.
         * This means all batch requests invoked on fixed proxies
         * for all connections associated with the communicator.
         * Any errors that occur while flushing a connection are ignored.
         * @param compress Specifies whether or not the queued batch requests
         * should be compressed before being sent over the wire.
         * @return @returns The asynchronous result object for the invocation.
         */
        flushBatchRequests(compress:CompressBatch):AsyncResultBase<void>;
    }

    /**
     * The output mode for xxxToString method such as identityToString and proxyToString.
     * The actual encoding format for the string is the same for all modes: you
     * don't need to specify an encoding format or mode when reading such a string.
     */
    class ToStringMode
    {
        /**
         * Characters with ordinal values greater than 127 are kept as-is in the resulting string.
         * Non-printable ASCII characters with ordinal values 127 and below are encoded as \\t, \\n (etc.)
         * or \\unnnn.
         */
        static readonly Unicode:ToStringMode;
        /**
         * Characters with ordinal values greater than 127 are encoded as universal character names in
         * the resulting string: \\unnnn for BMP characters and \\Unnnnnnnn for non-BMP characters.
         * Non-printable ASCII characters with ordinal values 127 and below are encoded as \\t, \\n (etc.)
         * or \\unnnn.
         */
        static readonly ASCII:ToStringMode;
        /**
         * Characters with ordinal values greater than 127 are encoded as a sequence of UTF-8 bytes using
         * octal escapes. Characters with ordinal values 127 and below are encoded as \\t, \\n (etc.) or
         * an octal escape. Use this mode to generate strings compatible with Ice 3.6 and earlier.
         */
        static readonly Compat:ToStringMode;
        
        static valueOf(value:number):ToStringMode;
        equals(other:any):boolean;
        hashCode():number;
        toString():string;
        
        readonly name:string;
        readonly value:number;
    }
}
