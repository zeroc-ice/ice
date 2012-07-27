// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

module IceMX
{

class MetricsObject
{
    string id;
    int total = 0;
    int current = 0;
};

sequence<MetricsObject> MetricsObjectSeq;
dictionary<string, MetricsObjectSeq> MetricsObjectSeqDict;
dictionary<string, MetricsObjectSeqDict> MetricsObjectSeqDictDict;
 
dictionary<string, string> NameValueDict;

exception UnknownMetricsView
{
};
 
interface MetricsAdmin
{ 
    /**
     *
     * Get the metrics objects for the given metrics view. This
     * returns a map of metric maps for each metrics class configured
     * with the view.
     *
     **/
    MetricsObjectSeqDict getMetricsMaps(string view)
        throws UnknownMetricsView;

    /**
     *
     * Get all the metrics objects for the given metrics class. This
     * returns a map of metrics view.
     *
     **/
    MetricsObjectSeqDictDict getAllMetricsMaps();

    /**
     *
     * Add a metrics map to the view.
     *
     **/
    void addMapToView(string view, string map, string groupBy, bool reap, NameValueDict acceptFilter, 
                      NameValueDict rejectFilter);

    /**
     *
     * Remove the given map from the given metric view.
     *
     **/
    void removeMapFromView(string view, string map)
        throws UnknownMetricsView;

    /**
     *
     * Enable the collection of metrics for the given view.
     *
     **/
    void enableView(string view)
        throws UnknownMetricsView;

    /**
     *
     * Disable the collection of metrics for the given view.
     *
     **/
    void disableView(string view)
        throws UnknownMetricsView;
};

class ThreadMetricsObject extends MetricsObject
{
    int inUseForIO = 0;
    int inUseForUser = 0;
    int inUseForMisc = 0;
};

class RequestMetricsObject extends MetricsObject
{
    int ok = 0;
    int responseUserException = 0;
    int responseRequestFailedException = 0;
    int responseUnknownException = 0;
    
    long duration = 0;
};

class ConnectionMetricsObject extends MetricsObject
{
   int initializing = 0;
   int holding = 0;
   int active = 0;
   int closing = 0;
   int closed = 0;

   long receivedBytes = 0;
   long receivedTime = 0;

   long sentBytes = 0;
   long sentTime = 0;
};

};