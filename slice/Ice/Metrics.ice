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
    int total;
    int current;
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
    void addMapToView(string view, string map, string groupBy, NameValueDict acceptFilter, NameValueDict rejectFilter);

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

module Ice
{

class ThreadPoolThreadMetricsObject extends MetricsObject
{
   int inUse;
   int inUseForIO;
};
  
class RequestMetricsObject extends MetricsObject
{
   int responseOK;
   int responseUserException;
   int responseSytemException;
   int failure;

   long marshalTime;
   long unmarshalTime;
   long time;
};
  
class ConnectionMetricsObject extends MetricsObject
{
   int initializing;
   int holding;
   int active;
   int closing;
   int closed;

   long receivedBytes;
   long receivedTime;

   long sentBytes;
   long sentTime;
};

};

};