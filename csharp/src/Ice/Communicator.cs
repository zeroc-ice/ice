// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Net.Security;
using Ice.Internal;

namespace Ice;

public sealed class Communicator : IDisposable
{
    /// <summary>
    /// Gets a task that completes when the communicator's shutdown completes. This task always completes successfully.
    /// </summary>
    /// <remarks>The shutdown of a communicator completes when all its incoming connections are closed. Awaiting this
    /// task is equivalent to calling <see cref="waitForShutdown" />.</remarks>
    /// <seealso cref="shutdown" />
    public Task shutdownCompleted
    {
        get
        {
            // It would be much nicer to wait asynchronously but doing so requires significant refactoring.
            var tcs = new TaskCompletionSource(); // created "on demand", when the user calls shutdownCompleted
            _ = Task.Run(() =>
            {
                waitForShutdown();
                tcs.SetResult();
            });
            return tcs.Task;
        }
    }

    internal Instance instance { get; }

    private const string _flushBatchRequests_name = "flushBatchRequests";

    /// <summary>
    /// Disposes this communicator. It's an alias for <see cref="destroy"/>.
    /// </summary>
    public void Dispose() => destroy();

    /// <summary>
    /// Destroys the communicator. This operation calls shutdown implicitly. Calling destroy cleans up memory, and shuts
    /// down this communicator's client functionality and destroys all object adapters. Subsequent calls to destroy are
    /// ignored.
    /// </summary>
    public void destroy() => instance.destroy();

    /// <summary>
    /// Shuts down this communicator: call <see cref="ObjectAdapter.deactivate"/> on all object adapters created by
    /// this communicator. Shutting down a communicator has no effect on outgoing connections.
    /// </summary>
    public void shutdown()
    {
        try
        {
            instance.objectAdapterFactory().shutdown();
        }
        catch (CommunicatorDestroyedException)
        {
            // Ignore
        }
    }

    /// <summary>
    /// Waits until the application has called <see cref="shutdown" /> (or <see cref="destroy" />).
    /// On the server side, this method blocks the calling thread until all currently-executing dispatches have
    /// completed. On the client side, the method simply blocks until another thread has called shutdown or destroy.
    /// A typical use of this method is to call it from the main thread, which then waits until some other thread
    /// calls shutdown. After shut-down is complete, the main thread returns and can do some cleanup work
    /// before it finally calls destroy to shut down the client functionality, and then exits the application.
    /// </summary>
    public void waitForShutdown()
    {
        try
        {
            instance.objectAdapterFactory().waitForShutdown();
        }
        catch (CommunicatorDestroyedException)
        {
            // Ignore
        }
    }

    /// <summary>
    /// Checks whether communicator has been shut down.
    /// </summary>
    /// <returns>True if the communicator has been shut down; false otherwise.</returns>
    public bool isShutdown()
    {
        try
        {
            return instance.objectAdapterFactory().isShutdown();
        }
        catch (CommunicatorDestroyedException)
        {
            return true;
        }
    }

    /// <summary>
    /// Converts a stringified proxy into a proxy.
    /// For example, "MyCategory/MyObject:tcp -h some_host -p 10000" creates a proxy that refers to the Ice object
    /// having an identity with a name "MyObject" and a category "MyCategory", with the server running on host
    /// "some_host", port 10000. If the stringified proxy does not parse correctly, this method throws ParseException.
    /// Refer to the Ice manual for a detailed description of the syntax supported by stringified proxies.
    /// </summary>
    /// <param name="str">The stringified proxy to convert into a proxy.</param>
    /// <returns>The proxy, or null if str is an empty string.</returns>
    public ObjectPrx? stringToProxy(string str)
    {
        Reference? reference = instance.referenceFactory().create(str, "");
        return reference is not null ? new ObjectPrxHelper(reference) : null;
    }

    /// <summary>
    /// Converts a proxy into a string.
    /// </summary>
    /// <param name="proxy">The proxy to convert into a stringified proxy.</param>
    /// <returns>The stringified proxy, or an empty string if <paramref name="proxy" /> is null.</returns>
    public string proxyToString(ObjectPrx? proxy) =>
        proxy is null ? "" : ((ObjectPrxHelperBase)proxy).iceReference().ToString();

    /// <summary>
    /// Converts a set of proxy properties into a proxy. The "base" name supplied in the property argument refers to a
    /// property containing a stringified proxy, such as "MyProxy=id:tcp -h localhost -p 10000". Additional properties
    /// configure local settings for the proxy, such as MyProxy.PreferSecure=1. The "Properties" appendix in the Ice
    /// manual describes each of the supported proxy properties.
    /// </summary>
    /// <param name="property">The base property name.</param>
    /// <returns>The proxy.</returns>
    public ObjectPrx? propertyToProxy(string property)
    {
        string proxy = instance.initializationData().properties!.getProperty(property);
        Reference? reference = instance.referenceFactory().create(proxy, property);
        return reference is not null ? new ObjectPrxHelper(reference) : null;
    }

    /// <summary>
    /// Converts a proxy to a set of proxy properties.
    /// </summary>
    /// <param name="proxy">The proxy.</param>
    /// <param name="prefix">The base property name.</param>
    /// <returns>The property set.</returns>
    public Dictionary<string, string> proxyToProperty(ObjectPrx proxy, string prefix) =>
        ((ObjectPrxHelperBase)proxy).iceReference().toProperty(prefix);

    /// <summary>
    /// Converts an identity into a string.
    /// </summary>
    /// <param name="ident">The identity to convert into a string.</param>
    /// <returns>The "stringified" identity.</returns>
    public string identityToString(Identity ident) => Util.identityToString(ident, instance.toStringMode());

    /// <summary>Creates a new object adapter. The endpoints for the object adapter are taken from the property
    /// name.Endpoints. It is legal to create an object adapter with the empty string as its name. Such an object
    /// adapter is accessible via bidirectional connections or by collocated invocations that originate from the
    /// same communicator as is used by the adapter. Attempts to create a named object adapter for which no
    /// configuration can be found raise InitializationException.</summary>
    /// <param name="name">The object adapter name.</param>
    /// <param name="serverAuthenticationOptions">The authentication options used by the SSL transport. Pass null
    /// if the adapter doesn't have any secure endpoints or if the SSL transport is configured using IceSSL properties.
    /// When <paramref name="serverAuthenticationOptions"/> is set to a non-null value, all IceSSL properties are
    /// ignored, and all the required configuration must be set using the <see cref="SslServerAuthenticationOptions"/>
    /// object.
    /// </param>
    /// <returns>The new object adapter.</returns>
    public ObjectAdapter createObjectAdapter(
        string name,
        SslServerAuthenticationOptions? serverAuthenticationOptions = null) =>
        instance.objectAdapterFactory().createObjectAdapter(name, null, serverAuthenticationOptions);

    /// <summary>Creates a new object adapter with endpoints. This method sets the property name.Endpoints, and
    /// then calls createObjectAdapter. It is provided as a convenience function. Calling this operation with an
    /// empty name will result in a UUID being generated for the name.</summary>
    /// <param name="name">The object adapter name.</param>
    /// <param name="endpoints">The endpoints for the object adapter.</param>
    /// <param name="serverAuthenticationOptions">The authentication options used by the SSL transport. Pass null
    /// if the adapter doesn't have any secure endpoints or if the SSL transport is configured using IceSSL properties.
    /// When <paramref name="serverAuthenticationOptions"/> is set to a non-null value, all IceSSL properties are
    /// ignored, and all the required configuration must be set using the <see cref="SslServerAuthenticationOptions"/>
    /// object.</param>
    /// <returns>The new object adapter.</returns>
    public ObjectAdapter createObjectAdapterWithEndpoints(
        string name,
        string endpoints,
        SslServerAuthenticationOptions? serverAuthenticationOptions = null)
    {
        if (name.Length == 0)
        {
            name = Guid.NewGuid().ToString();
        }

        getProperties().setProperty(name + ".Endpoints", endpoints);
        return instance.objectAdapterFactory().createObjectAdapter(name, null, serverAuthenticationOptions);
    }

    /// <summary>
    /// Creates a new object adapter with a router.
    /// This method creates a routed object adapter. Calling this operation with an empty name will result in a UUID
    /// being generated for the name.
    /// </summary>
    /// <param name="name">The object adapter name.</param>
    /// <param name="router">The router.</param>
    /// <returns>The new object adapter.</returns>
    public ObjectAdapter createObjectAdapterWithRouter(string name, RouterPrx router)
    {
        if (name.Length == 0)
        {
            name = Guid.NewGuid().ToString();
        }

        //
        // We set the proxy properties here, although we still use the proxy supplied.
        //
        Dictionary<string, string> properties = proxyToProperty(router, name + ".Router");
        foreach (KeyValuePair<string, string> entry in properties)
        {
            getProperties().setProperty(entry.Key, entry.Value);
        }

        return instance.objectAdapterFactory().createObjectAdapter(name, router, serverAuthenticationOptions: null);
    }

    /// <summary>
    /// Gets the object adapter that is associated by default with new outgoing connections created by this
    /// communicator. This method returns null unless you set a non-null default object adapter using
    /// <see cref="setDefaultObjectAdapter" />.
    /// </summary>
    /// <returns>The object adapter associated by default with new outgoing connections.</returns>
    /// <seealso cref="Connection.getAdapter" />
    public ObjectAdapter? getDefaultObjectAdapter() => instance.outgoingConnectionFactory().getDefaultObjectAdapter();

    /// <summary>
    /// Sets the object adapter that will be associated with new outgoing connections created by this communicator. This
    /// method has no effect on existing outgoing connections, or on incoming connections.
    /// </summary>
    /// <param name="adapter">The object adapter to associate with new outgoing connections.</param>
    /// <seealso cref="Connection.setAdapter" />
    public void setDefaultObjectAdapter(ObjectAdapter? adapter) =>
        instance.outgoingConnectionFactory().setDefaultObjectAdapter(adapter);

    /// <summary>
    /// Gets the implicit context associated with this communicator.
    /// </summary>
    /// <returns>The implicit context associated with this communicator; returns null when the property
    /// Ice.ImplicitContext is not set or is set to None.</returns>
    public ImplicitContext getImplicitContext() => instance.getImplicitContext();

    /// <summary>
    /// Gets the properties for this communicator.
    /// </summary>
    /// <returns>This communicator's properties.</returns>
    public Properties getProperties() => instance.initializationData().properties!;

    /// <summary>
    /// Gets the logger for this communicator.
    /// </summary>
    /// <returns>This communicator's logger.</returns>
    public Logger getLogger() => instance.initializationData().logger!;

    /// <summary>
    /// Gets the observer resolver object for this communicator.
    /// </summary>
    /// <returns>This communicator's observer resolver object.</returns>
    public Instrumentation.CommunicatorObserver? getObserver() => instance.initializationData().observer;

    /// <summary>
    /// Gets the default router for this communicator.
    /// </summary>
    /// <returns>The default router for this communicator.</returns>
    public RouterPrx? getDefaultRouter() => instance.referenceFactory().getDefaultRouter();

    /// <summary>
    /// Sets a default router for this communicator.
    /// All newly created proxies will use this default router. To disable the default router, null can be used. Note
    /// that this method has no effect on existing proxies. You can also set a router for an individual proxy by calling
    /// <see cref="ObjectPrx.ice_router(RouterPrx?)" /> on the proxy.
    /// </summary>
    /// <param name="router">The default router to use for this communicator.</param>
    public void setDefaultRouter(RouterPrx? router) => instance.setDefaultRouter(router);

    /// <summary>
    /// Gets the default locator for this communicator.
    /// </summary>
    /// <returns>The default locator for this communicator.</returns>
    public LocatorPrx? getDefaultLocator() => instance.referenceFactory().getDefaultLocator();

    /// <summary>
    /// Sets a default Ice locator for this communicator.
    /// All newly created proxy and object adapters will use this default locator. To disable the default locator, null
    /// can be used. Note that this method has no effect on existing proxies or object adapters.
    /// You can also set a locator for an individual proxy by calling <see cref="ObjectPrx.ice_locator(LocatorPrx?)" />
    /// on the proxy, or for an object adapter by calling <see cref="ObjectAdapter.setLocator(LocatorPrx)" /> on the
    /// object adapter.
    /// </summary>
    /// <param name="locator">The default locator to use for this communicator.</param>
    public void setDefaultLocator(LocatorPrx? locator) => instance.setDefaultLocator(locator);

    /// <summary>
    /// Gets the plug-in manager for this communicator.
    /// </summary>
    /// <returns>This communicator's plug-in manager.</returns>
    public PluginManager getPluginManager() => instance.pluginManager();

    /// <summary>
    /// Gets the value factory manager for this communicator.
    /// </summary>
    /// <returns>This communicator's value factory manager.</returns>
    public ValueFactoryManager getValueFactoryManager() => instance.initializationData().valueFactoryManager!;

    /// <summary>
    /// Flushes any pending batch requests for this communicator.
    /// This means all batch requests invoked on fixed proxies for all connections associated with the communicator.
    /// Any errors that occur while flushing a connection are ignored.
    /// </summary>
    /// <param name="compress">Specifies whether or not the queued batch requests should be compressed before being sent
    /// over the wire.</param>
    public void flushBatchRequests(CompressBatch compress)
    {
        try
        {
            var completed = new FlushBatchTaskCompletionCallback();
            var outgoing = new CommunicatorFlushBatchAsync(instance, completed);
            outgoing.invoke(_flushBatchRequests_name, compress, true);
            completed.Task.Wait();
        }
        catch (AggregateException ex)
        {
            throw ex.InnerException!;
        }
    }

    public Task flushBatchRequestsAsync(
        CompressBatch compress,
        IProgress<bool>? progress = null,
        CancellationToken cancel = default)
    {
        var completed = new FlushBatchTaskCompletionCallback(progress, cancel);
        var outgoing = new CommunicatorFlushBatchAsync(instance, completed);
        outgoing.invoke(_flushBatchRequests_name, compress, false);
        return completed.Task;
    }

    /// <summary>
    /// Adds the Admin object with all its facets to the provided object adapter.
    /// If Ice.Admin.ServerId is set and the provided object adapter has a Locator, createAdmin registers the Admin's
    /// Process facet with the Locator's LocatorRegistry. createAdmin must only be called once; subsequent calls raise
    /// InitializationException.
    /// </summary>
    /// <param name="adminAdapter">The object adapter used to host the Admin object; if null and Ice.Admin.Endpoints is
    /// set, create, activate and use the Ice.Admin object adapter.</param>
    /// <param name="adminId">The identity of the Admin object.</param>
    /// <returns>A proxy to the main ("") facet of the Admin object.</returns>
    public ObjectPrx createAdmin(ObjectAdapter adminAdapter, Identity adminId) =>
        instance.createAdmin(adminAdapter, adminId);

    /// <summary>
    /// Gets a proxy to the main facet of the Admin object.
    /// getAdmin also creates the Admin object and creates and activates the Ice.Admin object adapter to host this
    /// Admin object if Ice.Admin.Endpoints is set. The identity of the Admin object created by getAdmin is
    /// {value of Ice.Admin.InstanceName}/admin, or {UUID}/admin when  Ice.Admin.InstanceName is not set. If
    /// Ice.Admin.DelayCreation is 0 or not set, getAdmin is called  by the communicator initialization, after
    /// initialization of all plugins.
    /// </summary>
    /// <returns>A proxy to the main ("") facet of the Admin object, or a null proxy if no Admin object is configured.
    /// </returns>
    public ObjectPrx? getAdmin() => instance.getAdmin();

    /// <summary>
    /// Adds a new facet to the Admin object.
    /// Adding a servant with a facet that is already registered throws AlreadyRegisteredException.
    /// </summary>
    /// <param name="servant">The servant that implements the new Admin facet.</param>
    /// <param name="facet">The name of the new Admin facet.</param>
    public void addAdminFacet(Object servant, string facet) => instance.addAdminFacet(servant, facet);

    /// <summary>
    /// Removes the following facet to the Admin object.
    /// Removing a facet that was not previously registered throws <see cref="NotRegisteredException" />.
    /// </summary>
    /// <param name="facet">The name of the Admin facet.</param>
    /// <returns>The servant associated with this Admin facet.</returns>
    public Object removeAdminFacet(string facet) => instance.removeAdminFacet(facet);

    /// <summary>
    /// Returns a facet of the Admin object.
    /// </summary>
    /// <param name="facet">The name of the Admin facet.</param>
    /// <returns>The servant associated with this Admin facet, or null if no facet is registered with the given name.
    /// </returns>
    public Object? findAdminFacet(string facet) => instance.findAdminFacet(facet);

    /// <summary>
    /// Returns a map of all facets of the Admin object.
    /// </summary>
    /// <returns>A collection containing all the facet names and servants of the Admin object.</returns>
    public Dictionary<string, Object> findAllAdminFacets() => instance.findAllAdminFacets();

    internal Communicator(InitializationData initData)
    {
        instance = new Instance();
        instance.initialize(this, initData);
    }

    internal void finishSetup(ref string[] args)
    {
        try
        {
            instance.finishSetup(ref args, this);
        }
        catch
        {
            instance.destroy();
            throw;
        }
    }
}

/// <summary>
/// The output mode for xxxToString method such as identityToString and proxyToString.
/// The actual encoding format for
/// the string is the same for all modes: you don't need to specify an encoding format or mode when reading such a
/// string.
/// </summary>
public enum ToStringMode
{
    /// <summary>
    /// Characters with ordinal values greater than 127 are kept as-is in the resulting string.
    /// Non-printable ASCII
    /// characters with ordinal values 127 and below are encoded as \\t, \\n (etc.) or \\unnnn.
    /// </summary>
    Unicode,

    /// <summary>
    /// Characters with ordinal values greater than 127 are encoded as universal character names in the resulting
    /// string: \\unnnn for BMP characters and \\Unnnnnnnn for non-BMP characters.
    /// Non-printable ASCII characters
    /// with ordinal values 127 and below are encoded as \\t, \\n (etc.) or \\unnnn.
    /// </summary>
    ASCII,

    /// <summary>
    /// Characters with ordinal values greater than 127 are encoded as a sequence of UTF-8 bytes using octal escapes.
    /// Characters with ordinal values 127 and below are encoded as \\t, \\n (etc.) or an octal escape. Use this mode
    /// to generate strings compatible with Ice 3.6 and earlier.
    /// </summary>
    Compat
}
