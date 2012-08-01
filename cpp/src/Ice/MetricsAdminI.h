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

class MetricsMap : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    class Entry : public Ice::LocalObject, public IceUtil::Mutex
    {
    public:

        Entry(MetricsMap* map, const MetricsObjectPtr& object) : _map(map), _object(object)
        {
        }

        template<typename ObjectHelper, typename MetricsObjectPtrType> MetricsObjectPtrType 
        attach(const ObjectHelper& helper)
        {
            IceUtil::Mutex::Lock sync(*this);
            ++_object->total;
            ++_object->current;

            MetricsObjectPtrType obj = MetricsObjectPtrType::dynamicCast(_object);
            helper.initMetricsObject(obj);
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
            ++_object->failures[exceptionName];
        }

        MetricsObjectPtr
        clone() const
        {
            IceUtil::Mutex::Lock sync(*this);
            // TODO: Fix ice_clone to use a co-variant type.
            return dynamic_cast<MetricsObject*>(_object->ice_clone().get());
        }


        template<typename Function, typename MetricsObjectType> void
        execute(Function func, const MetricsObjectType& obj)
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

        MetricsMap* _map;
        MetricsObjectPtr _object;
    };
    typedef IceUtil::Handle<Entry> EntryPtr;

    MetricsMap(const std::string&, int, const NameValueDict&, const NameValueDict&);

    MetricsObjectSeq getMetricsObjects();

    EntryPtr getMatching(const ObjectHelper&);

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

    void add(const std::string&, const std::string&, int, const NameValueDict&, const NameValueDict&);
    void remove(const std::string&);

    MetricsObjectSeqDict getMetricsObjects();

    MetricsMap::EntryPtr getMatching(const std::string&, const ObjectHelper&) const;

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

    std::vector<MetricsMap::EntryPtr> getMatching(const std::string&, const ObjectHelper&) const;
    void addUpdater(const std::string&, const ObjectObserverUpdaterPtr&);

    virtual MetricsObjectSeqDict getMetricsMaps(const std::string&, const ::Ice::Current&);
    virtual MetricsObjectSeqDictDict getAllMetricsMaps(const ::Ice::Current&);

    virtual void addMapToView(const std::string&, const std::string&, const std::string&, int, const NameValueDict&, 
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
