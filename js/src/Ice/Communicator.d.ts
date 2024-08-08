//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice" {
    namespace Ice {
        /**
         * The central object in Ice. One or more communicators can be instantiated for an Ice application.
         *
         * @see {@link Logger}
         * @see {@link ObjectAdapter}
         * @see {@link Properties}
         * @see {@link ValueFactory}
         */
        interface Communicator {
            /**
             * Destroy the communicator. This operation calls {@link Communicator#shutdown} implicitly. Calling {@link Communicator#destroy} cleans up
             * memory, and shuts down this communicator's client functionality and destroys all object adapters.
             *
             * @returns A promise that is resolved when the communicator has been destroyed.
             *
             * @see {@link shutdown}
             * @see {@link ObjectAdapter#destroy}
             */
            destroy(): Promise<void>;

            /**
             * Shuts down this communicator's server functionality, which includes the deactivation of all object adapters.
             *
             * After shutdown returns, no new requests are processed. However, requests that have been started before shutdown
             * was called might still be active. You can use {@link Communicator#waitForShutdown} to wait for the completion of all
             * requests.
             *
             * @returns The asynchronous result object for the invocation.
             *
             * @see {@link destroy}
             * @see {@link waitForShutdown}
             * @see {@link ObjectAdapter#deactivate}
             */
            shutdown(): Promise<void>;

            /**
             * Wait until the application has called {@link Communicator#shutdown} (or {@link Communicator#destroy}). On the server side, this
             * operation blocks the calling thread until all currently-executing operations have completed. On the client
             * side, the operation simply blocks until another thread has called {@link Communicator#shutdown} or {@link Communicator#destroy}.
             * A typical use of this operation is to call it from the main thread, which then waits until some other thread
             * calls {@link Communicator#shutdown}. After shut-down is complete, the main thread returns and can do some cleanup work
             * before it finally calls {@link Communicator#destroy} to shut down the client functionality, and then exits the application.
             * @returns The asynchronous result object for the invocation.
             * @see {@link shutdown}
             * @see {@link destroy}
             * @see {@link ObjectAdapter#waitForDeactivate}
             */
            waitForShutdown(): Promise<void>;

            /**
             * Check whether communicator has been shut down.
             *
             * @returns `true` if the communicator has been shut down; `false` otherwise.
             *
             * @see {@link shutdown}
             */
            isShutdown(): boolean;

            /**
             * Convert a proxy string into a proxy.
             *
             * @param str The stringified proxy to convert into a proxy.
             *
             * @returns The proxy, or null if <code>str</code> is an empty string.
             *
             * @see {@link proxyToString}
             */
            stringToProxy(str: string): Ice.ObjectPrx;

            /**
             * Convert a proxy into a string.
             *
             * @param obj The proxy to convert into a proxy string.
             * @returns The proxy string representation, or an empty string if <code>obj</code> is null.
             */
            proxyToString(obj: Ice.ObjectPrx): string;

            /**
             * Convert a set of proxy properties into a proxy.
             *
             * The "base" name supplied in the <code>property</code> argument refers to a property containing a
             * stringified proxy, such as <code>MyProxy=id:tcp -h localhost -p 10000</code>. Additional properties
             * configure local settings for the proxy, such as <code>MyProxy.PreferSecure=1</code>. The "Properties"
             * appendix in the Ice manual describes each of the supported proxy properties.
             *
             * @param property The base property name.
             * @returns The proxy.
             */
            propertyToProxy(property: string): Ice.ObjectPrx;

            /**
             * Convert a proxy into a set of proxy properties.
             *
             * @param proxy The proxy to convert.
             * @param property The base property name.
             * @returns The property set.
             */
            proxyToProperty(proxy: Ice.ObjectPrx, property: string): PropertyDict;

            /**
             * Convert an identity into a string.
             * @param ident The identity to convert into a string.
             * @returns The string representation of the identity.
             */
            identityToString(ident: Identity): string;

            /**
             * Create a new object adapter.
             *
             * The endpoints for the object adapter are taken from the property `<name>.Endpoints`.
             *
             * It is legal to create an object adapter with the empty string as its name. Such an object adapter is
             * accessible via bidirectional connections. Attempts to create a named object adapter for which no
             * configuration can be found raise InitializationException.
             *
             * @param name The object adapter name.
             * @returns The asynchronous result object for the invocation.
             *
             * @see {@link createObjectAdapterWithEndpoints}
             * @see {@link ObjectAdapter}
             * @see {@link Properties}
             */
            createObjectAdapter(name: string): Promise<Ice.ObjectAdapter>;

            /**
             * Create a new object adapter with the given endpoints.
             *
             * This operation sets the property `<name>.Endpoints`, and then calls {@link Communicator#createObjectAdapter}.
             * Calling this operation with an empty name will result in a UUID being generated for the name.
             *
             * @param name The object adapter name.
             * @param endpoints The endpoints for the object adapter.
             * @returns The asynchronous result object for the invocation.
             * @see {@link createObjectAdapter}
             * @see {@link ObjectAdapter}
             * @see {@link Properties}
             */
            createObjectAdapterWithEndpoints(name: string, endpoints: string): Promise<Ice.ObjectAdapter>;

            /**
             * Create a new object adapter with a router.
             *
             * This operation creates a routed object adapter. Calling this operation with an empty name will result
             * in a UUID being generated for the name.
             *
             * @param name The object adapter name.
             * @param router The router.
             * @returns The asynchronous result object for the invocation.
             *
             * @see {@link createObjectAdapter}
             * @see {@link ObjectAdapter}
             * @see {@link Properties}
             */
            createObjectAdapterWithRouter(name: string, router: RouterPrx): Promise<Ice.ObjectAdapter>;

            /**
             * Get the implicit context associated with this communicator.
             *
             * @returns The implicit context associated with this communicator; or null when the property
             * `Ice.ImplicitContext` is not set or is set to None.
             */
            getImplicitContext(): Ice.ImplicitContext;

            /**
             * Get the properties for this communicator.
             *
             * @returns The communicator's properties.
             *
             * @see {@link Properties}
             */
            getProperties(): Ice.Properties;

            /**
             * Get the logger for this communicator.
             *
             * @returns The communicator's logger.
             *
             * @see {@link Logger}
             */
            getLogger(): Ice.Logger;

            /**
             * Get the default router for this communicator.
             *
             * @returns The communicator's default router.
             *
             * @see {@link setDefaultRouter}
             * @see {@link Router}
             */
            getDefaultRouter(): RouterPrx;

            /**
             * Sets the communicator's default router.
             *
             * All newly created proxies will use this default router. To disable the default router, null can be used.
             * Note that this operation has no effect on existing proxies.
             *
             * You can also set a router for an individual proxy by calling {@link ObjectPrx#ice_router} on the proxy.
             *
             * @param router The default router to use for this communicator.
             *
             * @see {@link getDefaultRouter}
             * @see {@link createObjectAdapterWithRouter}
             * @see {@link Router}
             */
            setDefaultRouter(router: RouterPrx | null): void;

            /**
             * Gets the communicator's default locator.
             *
             * @returns The communicator's default locator.
             *
             * @see {@link setDefaultLocator}
             * @see {@link Locator}
             */
            getDefaultLocator(): LocatorPrx;

            /**
             * Set's the communicator default locator.
             *
             * All newly created proxies will use this default locator. To disable the default locator, null can be
             * used. Note that this operation has no effect on existing proxies.
             *
             * You can also set a locator for an individual proxy by calling {@link ObjectPrx#ice_locator} on the
             * proxy.
             *
             * @param locator The default locator to use for this communicator.
             *
             * @see {@link getDefaultLocator}
             * @see {@link Locator}
             */
            setDefaultLocator(locator: LocatorPrx | null): void;

            /**
             * Gets the communicator's value factory manager.
             *
             * @returns The communicator's value factory manager.
             *
             * @see {@link ValueFactoryManager}
             */
            getValueFactoryManager(): Ice.ValueFactoryManager;

            /**
             * Flush any pending batch requests for this communicator.
             *
             * This means all batch requests invoked on fixed proxies for all connections associated with the
             * communicator. Any errors that occur while flushing a connection are ignored.
             *
             * @returns The asynchronous result object for the invocation.
             */
            flushBatchRequests(): Promise<void>;
        }
    }
}
