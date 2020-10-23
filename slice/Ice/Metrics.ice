//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[cpp:dll-export(ICE_API)]]
[[cpp:doxygen:include(Ice/Ice.h)]]
[[cpp:header-ext(h)]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(Ice)]]

#include <Ice/BuiltinSequences.ice>

[[java:package(com.zeroc)]]

[swift:module(Ice:MX)]
[cs:namespace(ZeroC)]
/// The Ice Management eXtension facility. It provides the {@link IceMX#MetricsAdmin} interface for management clients
/// to retrieve metrics from Ice applications.
module IceMX
{
    /// A dictionary of strings to integers.
    dictionary<string, int> StringIntDict;

    /// The base class for metrics. A metrics object represents a collection of measurements associated to a given a
    /// system.
    class Metrics
    {
        /// The metrics identifier.
        string id = "";

        /// The total number of objects observed by this metrics. This includes the number of currently observed
        /// objects and the number of objects observed in the past.
        long total = 0;

        /// The number of objects currently observed by this metrics.
        int current = 0;

        /// The sum of the lifetime of each observed objects. This does not include the lifetime of objects which are
        /// currently observed, only the objects observed in the past.
        long totalLifetime = 0;

        /// The number of failures observed.
        int failures = 0;
    }

    /// A structure to keep track of failures associated with a given metrics.
    struct MetricsFailures
    {
        /// The identifier of the metrics object associated to the failures.
        string id;

        /// The failures observed for this metrics.
        StringIntDict failures;
    }

    /// A sequence of {@link MetricsFailures}.
    sequence<MetricsFailures> MetricsFailuresSeq;

    /// A metrics map is a sequence of metrics. We use a sequence here instead of a map because the ID of the metrics
    /// is already included in the Metrics class and using sequences of metrics objects is more efficient than using
    /// dictionaries since lookup is not necessary.
    sequence<Metrics?> MetricsMap;

    /// A metrics view is a dictionary of metrics map. The key of the dictionary is the name of the metrics map.
    dictionary<string, MetricsMap> MetricsView;

    /// Raised if a metrics view cannot be found.
    exception UnknownMetricsView
    {
    }

    /// The metrics administrative facet interface. This interface allows remote administrative clients to access
    /// metrics of an application that enabled the Ice administrative facility and configured some metrics views.
    [format(sliced)]
    interface MetricsAdmin
    {
        /// Get the names of enabled and disabled metrics.
        ///
        /// @param disabledViews The names of the disabled views.
        ///
        /// @return The name of the enabled views.
        Ice::StringSeq getMetricsViewNames(out Ice::StringSeq disabledViews);

        /// Enables a metrics view.
        ///
        /// @param name The metrics view name.
        ///
        /// @throws UnknownMetricsView Raised if the metrics view cannot be found.
        void enableMetricsView(string name) throws UnknownMetricsView;

        /// Disable a metrics view.
        ///
        /// @param name The metrics view name.
        ///
        /// @throws UnknownMetricsView Raised if the metrics view cannot be found.
        void disableMetricsView(string name) throws UnknownMetricsView;

        /// Get the metrics objects for the given metrics view. This returns a dictionary of metric maps for each
        /// metrics class configured with the view. The timestamp allows the client to compute averages which are
        /// not dependent of the invocation latency for this operation.
        ///
        /// @param view The name of the metrics view.
        ///
        /// @param timestamp The local time of the process when the metrics objects were retrieved.
        ///
        /// @return The metrics view data.
        ///
        /// @throws UnknownMetricsView Raised if the metrics view cannot be found.
        MetricsView getMetricsView(string view, out long timestamp) throws UnknownMetricsView;

        /// Get the metrics failures associated with the given view and map.
        ///
        /// @param view The name of the metrics view.
        ///
        /// @param map The name of the metrics map.
        ///
        /// @return The metrics failures associated with the map.
        ///
        /// @throws UnknownMetricsView Raised if the metrics view cannot be found.
        MetricsFailuresSeq getMapMetricsFailures(string view, string map) throws UnknownMetricsView;

        /// Get the metrics failure associated for the given metrics.
        ///
        /// @param view The name of the metrics view.
        ///
        /// @param map The name of the metrics map.
        ///
        /// @param id The ID of the metrics.
        ///
        /// @return The metrics failures associated with the metrics.
        ///
        /// @throws UnknownMetricsView Raised if the metrics view cannot be found.
        MetricsFailures getMetricsFailures(string view, string map, string id) throws UnknownMetricsView;
    }

    /// Provides information on the number of threads currently in use and their activity.
    class ThreadMetrics : Metrics
    {
        /// The number of threads which are currently performing socket read or writes.
        int inUseForIO = 0;

        /// The number of threads which are currently calling user code (servant dispatch, AMI callbacks, etc).
        int inUseForUser = 0;

        /// The number of threads which are currently performing other activities. These are all other that are
        /// not counted with {@link #inUseForUser} or {@link #inUseForIO}, such as DNS lookups, garbage collection).
        int inUseForOther = 0;
    }

    /// Provides information on servant dispatch.
    class DispatchMetrics : Metrics
    {
        /// The number of dispatch that failed with a user exception.
        int userException = 0;

        /// The size of the dispatch. This corresponds to the size of the
        /// marshalled input parameters.
        long size = 0;

        /// The size of the dispatch reply. This corresponds to the size of
        /// the marshalled output and return parameters.
        long replySize = 0;
    }

    /// Provides information on child invocations. A child invocation is either remote (sent over an Ice connection) or
    /// collocated. An invocation can have multiple child invocation if it is retried. Child invocation metrics are
    /// embedded within {@link InvocationMetrics}.
    class ChildInvocationMetrics : Metrics
    {
        /// The size of the invocation. This corresponds to the size of the marshalled input parameters.
        long size = 0;

        /// The size of the invocation reply. This corresponds to the size of the marshalled output and return
        /// parameters.
        long replySize = 0;
    }

    /// Provides information on invocations that are collocated. Collocated metrics are embedded within
    /// {@link InvocationMetrics}.
    class CollocatedMetrics : ChildInvocationMetrics
    {
    }

    /// Provides information on invocations that are specifically sent over Ice connections. Remote metrics are
    /// embedded within {@link InvocationMetrics}.
    class RemoteMetrics : ChildInvocationMetrics
    {
    }

    /// Provide measurements for proxy invocations. Proxy invocations can either be sent over the wire or be collocated.
    class InvocationMetrics : Metrics
    {
        /// The number of retries for the invocation(s).
        int retry = 0;

        /// The number of invocations that failed with a user exception.
        int userException = 0;

        /// The children invocation metrics map.
        ///
        /// @see ChildInvocationMetrics
        MetricsMap children;

        /// The remote invocation metrics map.
        /// TODO: Remove once all language mappings implement colocation as a transport
        /// @see RemoteMetrics
        MetricsMap remotes;

        /// The collocated invocation metrics map.
        /// TODO: Remove once all language mappings implement colocation as a transport
        /// @see CollocatedMetrics
        MetricsMap collocated;
    }

    /// Provides information on the data sent and received over Ice connections.
    class ConnectionMetrics : Metrics
    {
        /// The number of bytes received by the connection.
        long receivedBytes = 0;

        /// The number of bytes sent by the connection.
        long sentBytes = 0;
    }
}
