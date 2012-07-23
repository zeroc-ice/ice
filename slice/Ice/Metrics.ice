// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

module IceMetrics
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
    MetricsObjectSeqDict getMetrics(string view)
        throws UnknownMetricsView;

    /**
     *
     * Get all the metrics objects for the given metrics class. This
     * returns a map of metrics view.
     *
     **/
    MetricsObjectSeqDictDict getAllMetrics();

    /**
     *
     * Monitor a new metrics class with the given view.
     *
     **/
    void addClassToView(string name, string cl, string groupBy, NameValueDict acceptFilter, NameValueDict rejectFilter);

    /**
     *
     * No longer monitor the given metrics class with the given view.
     *
     **/
    void removeClassFromView(string name, string cl)
        throws UnknownMetricsView;
};

local interface MetricsViewProvider
{
     void addView(string name, string value, NameValueDict accept, NameValueDict reject);
     void removeView(string name);
     MetricsObjectSeq getView(string view);
};
 
local interface MetricsViewProviderManager
{
     void add(string cl, MetricsViewProvider provider);
     void remove(string cl);
};
 
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