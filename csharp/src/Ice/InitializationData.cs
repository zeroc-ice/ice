// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Collections.Immutable;
using System.Net.Security;

namespace Ice;

/// <summary>
/// Represents a set of options that you can specify when initializing a communicator.
/// </summary>
public sealed record class InitializationData
{
    /// <summary>
    /// The properties for the communicator.
    /// </summary>
    /// <remarks>When non-null, this corresponds to the object returned by the <see cref="Communicator.getProperties"/>
    /// method.</remarks>
    public Properties? properties { get; set; }

    /// <summary>
    /// The logger for the communicator.
    /// </summary>
    public Logger? logger { get; set; }

    /// <summary>
    /// The communicator observer used by the Ice runtime.
    /// </summary>
    public Instrumentation.CommunicatorObserver? observer { get; set; }

    /// <summary>
    /// A function that the communicator calls when it starts a new thread.
    /// </summary>
    public Action? threadStart { get; set; }

    /// <summary>
    /// A function that the communicator calls when it destroys a thread.
    /// </summary>
    public Action? threadStop { get; set; }

    /// <summary>
    /// A function that the communicator calls to execute dispatches and async invocation callbacks.
    /// </summary>
    public Action<Action, Connection?>? executor { get; set; }

    /// <summary>
    /// The batch request interceptor, which is called by the Ice runtime to enqueue a batch request.
    /// </summary>
    public Action<BatchRequest, int, int>? batchRequestInterceptor { get; set; }

    /// <summary>
    /// The authentication options for SSL client connections. When set, the SSL transport ignores all IceSSL
    /// configuration properties and uses these options.
    /// </summary>
    public SslClientAuthenticationOptions? clientAuthenticationOptions { get; set; }

    /// <summary>
    /// A list of plug-in factories. The corresponding plug-ins are created during communicator initialization,
    /// in order, before all other plug-ins.
    /// </summary>
    public IList<PluginFactory> pluginFactories { get; set; } = ImmutableList<PluginFactory>.Empty;

    /// <summary>
    /// The Slice loader, used to unmarshal Slice classes and exceptions.
    /// </summary>
    public SliceLoader? sliceLoader { get; set; }
}
