// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/BuiltinSequences.ice>

module IceMX
{

dictionary<string, int> StringIntDict;

/**
 *
 * The base class for metrics.
 *
 **/ 
class Metrics
{
    /**
     *
     * The metrics identifier.
     *
     **/
    string id;
    
    /**
     *
     * The total number of objects that were observed by this metrics.
     *
     **/
    long total = 0;
    
    /**
     *
     * The current number of objects observed by this metrics.
     *
     **/
    int current = 0;

    /**
     *
     * The sum of the lifetime of each observed objects. This does not
     * include the lifetime of objects which are currently observed.
     *
     **/
    long totalLifetime = 0;

    /**
     *
     * The number of failures observed.
     *
     **/
    int failures = 0;
};

struct MetricsFailures
{
    /**
     *
     * The metrics id.
     *
     **/
    string id;

    /**
     *
     * The failures observed for this metrics.
     *
     **/
    StringIntDict failures;
};
sequence<MetricsFailures> MetricsFailuresSeq;

/**
 *
 * A metrics map is a sequence of metrics. We use a sequence here
 * instead of a map because the ID of the metrics is already included
 * in the Metrics class and using sequences of metrics objects is more
 * efficient than using dictionaries since lookup is not necessary.
 *
 **/
sequence<Metrics> MetricsMap;

/**
 *
 * A metrics view is a dictionary of metrics map. The key of the
 * dictionary is the name of the metrics map.
 *
 **/
dictionary<string, MetricsMap> MetricsView;

/**
 *
 * Raised if a metrics view cannot be found.
 * 
 **/
exception UnknownMetricsView
{
};


/**
 *
 * The MetricsAdmin facet interface.
 *
 **/ 
interface MetricsAdmin
{
    /**
     *
     * Get the names of enabled and disabled metrics.
     *
     * @param disabledViews The names of the disabled views.
     *
     * @return The name of the enabled views.
     *
     **/
    Ice::StringSeq getMetricsViewNames(out Ice::StringSeq disabledViews);

    /**
     *
     * Enables a metrics view.
     *
     * @param name The metrics view name.
     *
     **/
    void enableMetricsView(string name)
        throws UnknownMetricsView;

    /**
     *
     * Disable a metrics view.
     *
     * @param name The metrics view name.
     *
     **/
    void disableMetricsView(string name)
        throws UnknownMetricsView;

    /**
     *
     * Get the metrics objects for the given metrics view. This
     * returns a map of metric maps for each metrics class configured
     * with the view. The timestamp allows the client to compute
     * averages which are not dependent of the invocation latency for
     * this operation.
     *
     * @param view The name of the metrics view to retrieve.
     *
     * @param timestamp The local time of the process when the metrics
     * object were retrieved.
     *
     * @return The metrics view data.
     *
     * @throws UnknownMetricsView Raised if the metrics view cannot be
     * found.
     *
     **/
    MetricsView getMetricsView(string view, out long timestamp)
        throws UnknownMetricsView;

    /**
     *
     * Get the metrics failures associated with the given view and map.
     *
     * @param view The name of the metrics view.
     *
     * @param map The name of the metrics map.
     *
     * @return The metrics failures associated with the map.
     *
     * @throws UnknownMetricsView Raised if the metrics view cannot be
     * found.
     *
     **/
    MetricsFailuresSeq getMapMetricsFailures(string view, string map)
        throws UnknownMetricsView;

    /**
     *
     * Get the metrics failure associated for the given metrics.
     *
     * @param view The name of the metrics view.
     *
     * @param map The name of the metrics map.
     *
     * @param id The ID of the metrics.
     *
     * @return The metrics failures associated with the metrics.
     *
     * @throws UnknownMetricsView Raised if the metrics view cannot be
     * found.
     *
     **/
    MetricsFailures getMetricsFailures(string view, string map, string id)
        throws UnknownMetricsView;
};

/**
 *
 * Thread metrics.
 *
 **/
class ThreadMetrics extends Metrics
{
    /**
     *
     * Number of threads which are currently performing socket read or
     * writes.
     *
     **/
    int inUseForIO = 0;

    /**
     *
     * Number of threads which are currently calling user code
     * (servant dispatch, AMI callbacks, etc).
     *
     **/
    int inUseForUser = 0;
    
    /**
     *
     * Number of threads which are currently performing other
     * activities than the activities listed above (among others, this
     * is for example DNS lookups, garbage collection).
     *
     **/
    int inUseForOther = 0;
};

class DispatchMetrics extends Metrics
{
    /**
     *
     * Number of dispatch that failed with a user exception.
     *
     **/
    int userException = 0;
};

/**
 *
 * Invocation metrics.
 *
 **/
class InvocationMetrics extends Metrics
{
    /**
     *
     * Number of retries.
     *
     **/
    int retry = 0;

    /**
     *
     * Number of invocations that failed with a user exception.
     *
     **/
    int userException = 0;

    /**
     *
     * Remote invocations metrics map.
     *
     **/
    MetricsMap remotes;
};

/**
 *
 * Connection metrics.
 *
 **/
class ConnectionMetrics extends Metrics
{
    /**
     *
     * The number of bytes received by the connection. 
     *
     **/
    long receivedBytes = 0;

    /**
     *
     * The number of bytes sent by the connection.
     *
     **/
    long sentBytes = 0;
};

};
