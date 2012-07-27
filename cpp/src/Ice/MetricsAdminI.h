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
#include <Ice/Properties.h>
#include <Ice/Initialize.h>

namespace IceMX
{

class ObjectObserverUpdater;
typedef IceUtil::Handle<ObjectObserverUpdater> ObjectObserverUpdaterPtr;

class ObjectHelper;

class MetricsMap : public IceUtil::Shared
{
    class Entry : public IceUtil::Shared
    {
    public:
        Entry(const MetricsObjectPtr& object) : object(object)
        {
        }
        
        MetricsObjectPtr object;
        IceUtil::Mutex mutex;
    };
    typedef IceUtil::Handle<Entry> EntryPtr;

public:

    MetricsMap(const std::string&, bool, const NameValueDict&, const NameValueDict&);

    void destroy();

    MetricsObjectSeq getMetricsObjects();

    std::pair<MetricsObjectPtr, IceUtil::Mutex*> getMatching(const ObjectHelper&);

private:

    std::vector<std::string> _groupByAttributes;
    std::vector<std::string> _groupBySeparators;
    bool _reap;
    const NameValueDict _accept;
    const NameValueDict _reject;
    std::map<std::string, EntryPtr> _objects;
};
typedef IceUtil::Handle<MetricsMap> MetricsMapPtr;

class MetricsView : public IceUtil::Shared
{
public:
    
    MetricsView();

    void setEnabled(bool enabled)
    {
        _enabled = enabled;
    }

    bool isEnabled() const 
    {
        return _enabled;
    }

    void add(const std::string&, const std::string&, bool, const NameValueDict&, const NameValueDict&);
    void remove(const std::string&);

    MetricsObjectSeqDict getMetricsObjects();

    std::pair<MetricsObjectPtr, IceUtil::Mutex*> getMatching(const std::string&, const ObjectHelper&) const;

    std::vector<std::string> getMaps() const;

private:

    std::map<std::string, MetricsMapPtr> _maps;
    bool _enabled;
};
typedef IceUtil::Handle<MetricsView> MetricsViewPtr;

class MetricsAdminI : public MetricsAdmin, public IceUtil::Mutex
{
public:

    MetricsAdminI(::Ice::InitializationData&);

    std::vector<std::pair<MetricsObjectPtr, IceUtil::Mutex*> > getMatching(const std::string&, 
                                                                           const ObjectHelper&) const;
    void addUpdater(const std::string&, const ObjectObserverUpdaterPtr&);

    virtual MetricsObjectSeqDict getMetricsMaps(const std::string&, const ::Ice::Current&);
    virtual MetricsObjectSeqDictDict getAllMetricsMaps(const ::Ice::Current&);

    virtual void addMapToView(const std::string&, const std::string&, const std::string&, bool, const NameValueDict&, 
                                const NameValueDict&, const ::Ice::Current& = ::Ice::Current());

    virtual void removeMapFromView(const std::string&, const std::string&, const ::Ice::Current&);

    virtual void enableView(const std::string&, const ::Ice::Current&);
    virtual void disableView(const std::string&, const ::Ice::Current&);

private:

    void setViewEnabled(const std::string&, bool);

    std::map<std::string, MetricsViewPtr> _views;
    std::map<std::string, ObjectObserverUpdaterPtr> _updaters;
};
typedef IceUtil::Handle<MetricsAdminI> MetricsAdminIPtr;

};

#endif
