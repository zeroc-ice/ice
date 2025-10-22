// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Collections.Immutable;
using System.Net.Security;

namespace Ice;

/// <summary>
/// A class that encapsulates data to initialize a communicator.
/// </summary>
public sealed record class InitializationData
{
    /// <summary>
    /// Gets or sets the properties for the communicator.
    /// </summary>
    /// <remarks>When not null, this is the object returned by <see cref="Communicator.getProperties"/>.</remarks>
    public Properties? properties { get; set; }

    /// <summary>
    /// Gets or sets the logger for the communicator. The communicator does not take ownership of the logger; the
    /// caller remains responsible for disposing it.
    /// </summary>
    public Logger? logger { get; set; }

    /// <summary>
    /// Gets or sets the communicator observer used by the Ice run-time.
    /// </summary>
    public Instrumentation.CommunicatorObserver? observer { get; set; }

    /// <summary>
    /// Gets or sets the thread start hook for the communicator.
    /// </summary>
    /// <value>
    /// The Ice runtime calls this hook for each new thread it creates. The call is made by the newly-started thread.
    /// </value>
    public Action? threadStart { get; set; }

    /// <summary>
    /// Gets or sets the thread stop hook for the communicator.
    /// </summary>
    /// <value>
    /// The Ice runtime calls stop before it destroys a thread. The call is made by thread that is about to be
    /// destroyed.
    /// </value>
    public Action? threadStop { get; set; }

    /// <summary>
    /// Gets or sets the executor for the communicator.
    /// </summary>
    public Action<Action, Connection?>? executor { get; set; }

    /// <summary>
    /// Gets or sets the batch request interceptor.
    /// </summary>
    public Action<BatchRequest, int, int>? batchRequestInterceptor { get; set; }

    /// <summary>
    /// Gets or sets the <see cref="SslClientAuthenticationOptions"/> used by the client-side ssl transport.
    /// </summary>
    public SslClientAuthenticationOptions? clientAuthenticationOptions { get; set; }

    /// <summary>
    /// Gets or sets the plug-in factories. The corresponding plug-ins are created during communicator initialization,
    /// in order, before all other plug-ins.
    /// </summary>
    public IList<PluginFactory> pluginFactories { get; set; } = ImmutableList<PluginFactory>.Empty;

    /// <summary>
    /// Gets or sets the Slice loader. The Slice loader is used to unmarshal Slice classes and exceptions.
    /// </summary>
    public SliceLoader? sliceLoader { get; set; }
}
