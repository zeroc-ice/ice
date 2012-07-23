// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_METRICSADMIN_I_H
#define ICE_METRICSADMIN_I_H

#include <Ice/Metrics.h>
#include <Ice/Stats.h>

#include <Ice/MetricsObserverI.h>

namespace IceMetrics
{

class MetricsHelper
{
public:

    virtual std::string operator()(const std::string&) const = 0;

    virtual MetricsObjectPtr newMetricsObject() const = 0;
};

class MetricsMap : public IceUtil::Shared, IceUtil::Mutex
{
public:

    struct Entry
    {
        MetricsObjectPtr object;
        MetricsMapPtr map;

        bool operator<(const Entry& e)
        {
            return object < e.object;
        }
    };
    
    MetricsMap(const std::string&, const NameValueDict&, const NameValueDict&);

    void destroy();

    MetricsObjectSeq getMetricsObjects() const;

    MetricsObjectPtr getMatching(const MetricsHelper&);

private:

    bool match(const std::string&, const std::string&) const;

    const std::vector<std::string> _groupByAttributes;
    const std::vector<std::string> _groupBySeparators;
    const NameValueDict _accept;
    const NameValueDict _reject;
    std::map<std::string, MetricsObjectPtr> _objects;
};
typedef IceUtil::Handle<MetricsMap> MetricsMapPtr;

class MetricsView : public IceUtil::Shared
{
public:

    MetricsView();

    void add(const std::string&, const std::string&, const NameValueDict&, const NameValueDict&);
    void remove(const std::string&);

    MetricsObjectSeqDict getMetricsObjects() const;

    MetricsObjectPtr getMatching(const std::string&, const MetricsHelper&) const;

private:

    std::map<std::string, MetricsMapPtr> _maps;
};
typedef IceUtil::Handle<MetricsView> MetricsViewPtr;

class MetricsAdminI : public MetricsAdmin, public IceUtil::Mutex
{
public:

    MetricsObjectSeq getMatching(const std::string&, const MetricsHelper&) const;
    void addUpdater(const std::string&, const ObjectObserverUpdaterPtr&);

    virtual MetricsObjectSeqDict getMetrics(const std::string&, const Ice::Current&);
    virtual MetricsObjectSeqDictDict getAllMetrics(const Ice::Current&);

    virtual void addClassToView(const std::string&, const std::string&, const std::string&, const NameValueDict&, 
                                const NameValueDict&, const Ice::Current&);

    virtual void removeClassFromView(const std::string&, const std::string&, const Ice::Current&);

private:

    std::map<std::string, MetricsViewPtr> _views;
    std::map<std::string, ObjectObserverUpdaterPtr> _updaters;
};

};

#endif
