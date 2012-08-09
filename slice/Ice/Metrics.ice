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
     * The sum of the lifetime of each observed objects. This doesn't
     * include the lifetime of objects wich are currently observed.
     *
     **/
    long totalLifetime = 0;
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
 * efficient than using dictionaries since lookup isn't necessary.
 *
 **/
sequence<Metrics> MetricsMap;

/**
 *
 * A metrics view is a ditcionary of metrics map. The key of the
 * dictionary is the name of the metrics map.
 *
 **/
dictionary<string, MetricsMap> MetricsView;

/**
 *
 * Raised if a metrics view can't be found.
 * 
 **/
exception UnknownMetricsView
{
};
 
interface MetricsAdmin
{
    /**
     *
     * Get the name of enabled metrics views.
     *
     * @return The names of the metrics view currently enabled.
     *
     **/
    Ice::StringSeq getMetricsViewNames();

    /**
     *
     * Get the metrics objects for the given metrics view. This
     * returns a map of metric maps for each metrics class configured
     * with the view.
     *
     * @param view The name of the metrics view to retrieve.
     *
     * @throws UnknownMetricsView Raised if the metrics view can't be
     * found.
     *
     * @return The metrics view data.
     *
     **/
    MetricsView getMetricsView(string view)
        throws UnknownMetricsView;

    /**
     *
     * Get the metrics failure associated with the given view and map.
     *
     * @throws UnknownMetricsView Raised if the metrics view can't be
     * found.
     *
     * @param view The name of the metrics view.
     *
     * @param map The name of the metrics map.
     *
     * @return The metrics failures associated with the map.
     *
     **/
    MetricsFailuresSeq getMetricsFailures(string view, string map)
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
     * Number of connections waiting connection validation.
     * 
     **/
    int validating = 0;

    /**
     *
     * Number of connections in the holding state.
     * 
     **/
    int holding = 0;

    /**
     *
     * Number of active connection. An active connection is a
     * connection that can receive and dispatch messages.
     *
     **/
    int active = 0;
    
    /**
     *
     * Number of connections currently closing as a result of a
     * graceful shutdown.
     *
     **/
    int closing = 0;

    /**
     *
     * Number of closed connection which are waiting to be
     * destroyed. A connection might be in the closed state but not
     * destroyed if it's still waiting for user dispatch or callbacks
     * to return.
     *
     **/
    int closed = 0;

    /**
     *
     * The number of bytes received by the connection. 
     *
     **/
    long receivedBytes = 0;

    /**
     *
     * The time spent receiving bytes.
     *
     **/
    long receivedTime = 0;
    
    /**
     *
     * The number of bytes sent by the connection.
     *
     **/
    long sentBytes = 0;

    /**
     *
     * The time spent sending bytes.
     *
     **/
    long sentTime = 0;
};

};
