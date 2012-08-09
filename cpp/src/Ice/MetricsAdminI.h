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

class Updater;
typedef IceUtil::Handle<Updater> UpdaterPtr;

class MetricsHelper;
template<typename T> class MetricsHelperT;

typedef std::map<std::string, std::string> NameValueDict;

class MetricsMapI : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    class Entry : public Ice::LocalObject, public IceUtil::Mutex
    {
    public:

        Entry(MetricsMapI* map, const MetricsPtr& object) : _map(map), _object(object)
        {
        }

        template<typename MetricsType> IceInternal::Handle<MetricsType>
        attach(const MetricsHelperT<MetricsType>& helper)
        {
            IceUtil::Mutex::Lock sync(*this);
            ++_object->total;
            ++_object->current;

            IceInternal::Handle<MetricsType> obj = IceInternal::Handle<MetricsType>::dynamicCast(_object);
            assert(obj);
            helper.initMetrics(obj);
            return obj;
        }
        
        void detach(long lifetime)
        {
            bool detached = false;
            {
                IceUtil::Mutex::Lock sync(*this);
                detached = --_object->current == 0;

                _object->totalLifetime += lifetime;
            }
            if(detached)
            {
                _map->detached(this);
            }
        }

        void failed(const std::string& exceptionName)
        {
            IceUtil::Mutex::Lock sync(*this);
            ++_failures[exceptionName];
        }

        MetricsPtr
        clone() const
        {
            IceUtil::Mutex::Lock sync(*this);
            // TODO: Fix ice_clone to use a co-variant type.
            return dynamic_cast<Metrics*>(_object->ice_clone().get());
        }

        MetricsFailures
        getFailures() const
        {
            MetricsFailures f;

            IceUtil::Mutex::Lock sync(*this);
            f.id = _object->id;
            f.failures = _failures;
            return f;
        }


        template<typename Function, typename MetricsType> void
        execute(Function func, const MetricsType& obj)
        {
            IceUtil::Mutex::Lock sync(*this);
            func(obj);
        }

        const std::string& id() const
        {
            return _object->id;
        }

        bool isDetached() const
        {
            IceUtil::Mutex::Lock sync(*this);
            return _object->current == 0;
        }

    private:

        MetricsMapI* _map;
        MetricsPtr _object;
        StringIntDict _failures;
    };
    typedef IceUtil::Handle<Entry> EntryPtr;

    MetricsMapI(const std::string&, int, const NameValueDict&, const NameValueDict&);

    MetricsMap getMetrics();
    MetricsFailuresSeq getFailures();

    EntryPtr getMatching(const MetricsHelper&);

private:

    friend class Entry;
    void detached(Entry*);

    std::vector<std::string> _groupByAttributes;
    std::vector<std::string> _groupBySeparators;
    int _retain;
    const NameValueDict _accept;
    const NameValueDict _reject;

    std::map<std::string, EntryPtr> _objects;
    std::deque<Entry*> _detachedQueue;
};
typedef IceUtil::Handle<MetricsMapI> MetricsMapIPtr;

class MetricsViewI : public IceUtil::Shared
{
public:
    
    MetricsViewI();

    void setEnabled(bool enabled)
    {
        _enabled = enabled;
    }

    bool isEnabled() const 
    {
        return _enabled;
    }

    void add(const std::string&, const std::string&, int, const NameValueDict&, const NameValueDict&);
    void remove(const std::string&);

    MetricsView getMetrics();
    MetricsFailuresSeq getFailures(const std::string&);

    MetricsMapI::EntryPtr getMatching(const MetricsHelper&) const;

    std::vector<std::string> getMaps() const;

private:

    std::map<std::string, MetricsMapIPtr> _maps;
    bool _enabled;
};
typedef IceUtil::Handle<MetricsViewI> MetricsViewIPtr;

class MetricsAdminI : public MetricsAdmin, public IceUtil::Mutex
{
public:

    MetricsAdminI(::Ice::InitializationData&);

    std::vector<MetricsMapI::EntryPtr> getMatching(const MetricsHelper&) const;
    void addUpdater(const std::string&, const UpdaterPtr&);

    virtual Ice::StringSeq getMetricsViewNames(const ::Ice::Current&);
    virtual MetricsView getMetricsView(const std::string&, const ::Ice::Current&);
    virtual MetricsFailuresSeq getMetricsFailures(const std::string&, const std::string&, const ::Ice::Current&);

private:

    virtual void addMapToView(const std::string&, const std::string&, const std::string&, int, const NameValueDict&, 
                                const NameValueDict&, const ::Ice::Current& = ::Ice::Current());

    virtual void removeMapFromView(const std::string&, const std::string&, const ::Ice::Current&);

    void setViewEnabled(const std::string&, bool);

    std::map<std::string, MetricsViewIPtr> _views;
    std::map<std::string, UpdaterPtr> _updaters;
};
typedef IceUtil::Handle<MetricsAdminI> MetricsAdminIPtr;

};

#endif
