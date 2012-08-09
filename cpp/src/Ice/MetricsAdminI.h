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

        Entry(MetricsMapI* map, const MetricsPtr& object) : _object(object), _map(map)
        {
        }

        template<typename T> IceInternal::Handle<T>
        attach(const MetricsHelperT<T>& helper)
        {
            IceUtil::Mutex::Lock sync(*this);
            ++_object->total;
            ++_object->current;
            IceInternal::Handle<T> obj = IceInternal::Handle<T>::dynamicCast(_object);
            assert(obj);
            helper.initMetrics(obj);
            return obj;
        }

        void 
        failed(const std::string& exceptionName)
        {
            IceUtil::Mutex::Lock sync(*this);
            ++_failures[exceptionName];
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
        
        void 
        detach(long lifetime)
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

        virtual MetricsPtr
        clone() const
        {
            IceUtil::Mutex::Lock sync(*this);
            // TODO: Fix ice_clone to use a co-variant type.
            return dynamic_cast<Metrics*>(_object->ice_clone().get());
        }

        const std::string& 
        id() const
        {
            return _object->id;
        }

        bool 
        isDetached() const
        {
            IceUtil::Mutex::Lock sync(*this);
            return _object->current == 0;
        }

        virtual Entry*
        getMatching(const std::string&, const MetricsHelper&)
        {
            return 0;
        }

    protected:

        MetricsPtr _object;

    private:

        MetricsMapI* _map;
        StringIntDict _failures;
    };
    typedef IceUtil::Handle<Entry> EntryPtr;

    MetricsMapI(const std::string&, const Ice::PropertiesPtr&);
    MetricsMapI(const MetricsMapI&);

    MetricsFailuresSeq getFailures();
    MetricsMap getMetrics() const;
    EntryPtr getMatching(const MetricsHelper&);

protected:

    virtual EntryPtr newEntry(const MetricsPtr& object)
    {
        return new Entry(this, object);
    }

private:

    friend class Entry;
    void detached(Entry*);

    std::vector<std::string> _groupByAttributes;
    std::vector<std::string> _groupBySeparators;
    const int _retain;
    const NameValueDict _accept;
    const NameValueDict _reject;

    std::map<std::string, EntryPtr> _objects;
    std::deque<Entry*> _detachedQueue;
};
typedef IceUtil::Handle<MetricsMapI> MetricsMapIPtr;

class MetricsMapFactory : public IceUtil::Shared
{
public:

    virtual MetricsMapIPtr create(const std::string&, const Ice::PropertiesPtr&) = 0;
};
typedef IceUtil::Handle<MetricsMapFactory> MetricsMapFactoryPtr;

template<class MetricsType> class MetricsMapT : public MetricsMapI
{
public:

    typedef MetricsType T;
    typedef IceInternal::Handle<MetricsType> TPtr;

    typedef MetricsMap MetricsType::*SubMapMember;

    class EntryT : public MetricsMapI::Entry
    {
    public:

        EntryT(MetricsMapT* map, const TPtr& object) : Entry(map, object), _map(map)
        {
        }

        virtual Entry*
        getMatching(const std::string& mapName, const MetricsHelper& helper)
        {
            typename std::map<std::string, std::pair<MetricsMapIPtr, SubMapMember> >::iterator p = 
                _subMaps.find(mapName);
            if(p == _subMaps.end())
            {
                std::pair<MetricsMapIPtr, SubMapMember> map = _map->createSubMap(mapName);
                if(map.first)
                {
                    p = _subMaps.insert(make_pair(mapName, map)).first;
                }
            }
            if(p == _subMaps.end())
            {
                return 0;
            }            
            return p->second.first->getMatching(helper).get();
        }

        virtual MetricsPtr
        clone() const
        {
            IceUtil::Mutex::Lock sync(*this);
            TPtr metrics = TPtr::dynamicCast(_object->ice_clone());
            for(typename std::map<std::string, std::pair<MetricsMapIPtr, SubMapMember> >::const_iterator p =
                    _subMaps.begin(); p != _subMaps.end(); ++p)
            {
                metrics.get()->*p->second.second = p->second.first->getMetrics();
            }
            return metrics;
        }

    private:

        std::map<std::string, std::pair<MetricsMapIPtr, SubMapMember> > _subMaps;
        MetricsMapT* _map;
    };
    typedef IceUtil::Handle<EntryT> EntryTPtr;

    MetricsMapT(const std::string& mapPrefix, 
                const Ice::PropertiesPtr& properties,
                const std::map<std::string, SubMapMember>& subMaps) : 
        MetricsMapI(mapPrefix, properties)
    {
        for(typename std::map<std::string, SubMapMember>::const_iterator p = subMaps.begin(); p != subMaps.end(); ++p)
        {
            const std::string subMapsPrefix = mapPrefix + ".Map.";
            std::string subMapPrefix = subMapsPrefix + p->first;
            if(properties->getPropertyAsInt(subMapPrefix) == 0 || 
               properties->getPropertiesForPrefix(subMapPrefix).empty())
            {
                if(properties->getPropertiesForPrefix(subMapsPrefix).empty())
                {
                    subMapPrefix = mapPrefix;
                }
                else
                {
                    continue; // This sub-map isn't configured.
                }
            }
            _subMaps.insert(std::make_pair(p->first, 
                                           std::make_pair(new MetricsMapI(subMapPrefix, properties), p->second)));
        }
    }

    std::pair<MetricsMapIPtr, SubMapMember>
    createSubMap(const std::string& subMapName)
    {
        typename std::map<std::string, std::pair<MetricsMapIPtr, SubMapMember> >::const_iterator p =
            _subMaps.find(subMapName);
        if(p != _subMaps.end())
        {
            return std::make_pair(new MetricsMapI(*p->second.first), p->second.second);
        }
        return std::make_pair(MetricsMapIPtr(), static_cast<SubMapMember>(0));
    }

protected:

    virtual EntryPtr newEntry(const MetricsPtr& object)
    {
        return new EntryT(this, TPtr::dynamicCast(object));
    }

    std::map<std::string, std::pair<MetricsMapIPtr, SubMapMember> > _subMaps;
};

class MetricsViewI : public IceUtil::Shared
{
public:
    
    MetricsViewI(bool);

    void setEnabled(bool enabled)
    {
        _enabled = enabled;
    }

    bool isEnabled() const 
    {
        return _enabled;
    }

    void add(const std::string&, const MetricsMapIPtr&);
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

    MetricsAdminI(const ::Ice::PropertiesPtr&);

    std::vector<MetricsMapI::EntryPtr> getMatching(const MetricsHelper&) const;

    void addUpdater(const std::string&, const UpdaterPtr&);
    void addFactory(const std::string&, const MetricsMapFactoryPtr&);

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
    std::map<std::string, MetricsMapFactoryPtr> _factories;

    Ice::PropertiesPtr _properties;
};
typedef IceUtil::Handle<MetricsAdminI> MetricsAdminIPtr;

};

#endif
