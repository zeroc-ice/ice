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
#include <Ice/PropertiesAdmin.h>
#include <Ice/Initialize.h>

#ifdef _MSC_VER
#  define ICE_CPP11_REGEXP
#endif

#ifdef ICE_CPP11_REGEXP
#  include <regex>
#else
#  include <regex.h>
#endif

#include <list>

namespace IceMX
{

class Updater;
typedef IceUtil::Handle<Updater> UpdaterPtr;

class MetricsHelper;
template<typename T> class MetricsHelperT;

class MetricsMapI : public IceUtil::Shared
{
public:

    class RegExp : public IceUtil::Shared
    {
    public:
        
        RegExp(const std::string&, const std::string&);
        ~RegExp();

        bool match(const MetricsHelper&);

    private:
        const std::string _attribute;
#ifdef ICE_CPP11_REGEXP
        std::regex _regex;
#else
        regex_t _preg;
#endif        
    };
    typedef IceUtil::Handle<RegExp> RegExpPtr;

    MetricsMapI(const std::string&, const Ice::PropertiesPtr&);
    MetricsMapI(const MetricsMapI&);

    virtual MetricsFailuresSeq getFailures() = 0;
    virtual MetricsFailures getFailures(const std::string&) = 0;
    virtual MetricsMap getMetrics() const = 0;
    virtual MetricsMapI* clone() const = 0;

    const Ice::PropertyDict& getMapProperties() const
    {
        return _properties;
    }

protected:

    const Ice::PropertyDict _properties;
    std::vector<std::string> _groupByAttributes;
    std::vector<std::string> _groupBySeparators;
    const int _retain;
    const std::vector<RegExpPtr> _accept;
    const std::vector<RegExpPtr> _reject;
};
typedef IceUtil::Handle<MetricsMapI> MetricsMapIPtr;

class MetricsMapFactory : public IceUtil::Shared
{
public:

    virtual MetricsMapIPtr create(const std::string&, const Ice::PropertiesPtr&) = 0;
};
typedef IceUtil::Handle<MetricsMapFactory> MetricsMapFactoryPtr;

template<class MetricsType> class MetricsMapT : public MetricsMapI, private IceUtil::Mutex
{
public:

    typedef MetricsType T;
    typedef IceInternal::Handle<MetricsType> TPtr;

    typedef MetricsMap MetricsType::* SubMapMember;

    class EntryT : public Ice::LocalObject, protected IceUtil::Mutex
    {
    public:

        EntryT(MetricsMapT* map, const TPtr& object, const typename std::list<EntryT*>::iterator& p) : 
            _map(map), _object(object), _detachedPos(p)
        {
        }

        void destroy()
        {
            Lock sync(*this);
            _map = 0;
        }

        void  failed(const std::string& exceptionName)
        {
            Lock sync(*this);
            ++_object->failures;
            ++_failures[exceptionName];
        }

        MetricsFailures getFailures() const
        {
            MetricsFailures f;
    
            Lock sync(*this);
            f.id = _object->id;
            f.failures = _failures;
            return f;
        }

        template<typename MemberMetricsType> typename MetricsMapT<MemberMetricsType>::EntryTPtr
        getMatching(const std::string& mapName, const MetricsHelperT<MemberMetricsType>& helper)
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

            MetricsMapT<MemberMetricsType>* map = dynamic_cast<MetricsMapT<MemberMetricsType>*>(p->second.first.get());
            assert(map);
            return map->getMatching(helper);
        }

        void
        attach(const MetricsHelperT<T>& helper)
        {
            Lock sync(*this);
            ++_object->total;
            ++_object->current;
            helper.initMetrics(_object);
        }

        void 
        detach(Ice::Long lifetime)
        {
            MetricsMapT* map;
            {
                Lock sync(*this);
                _object->totalLifetime += lifetime;
                if(--_object->current > 0)
                {
                    return;
                }
                map = _map;
            }
            if(map)
            {
                map->detached(this);
            }
        }

        bool isDetached() const
        {
            Lock sync(*this);
            return _object->current == 0;
        }

        template<typename Function> void
        execute(Function func)
        {
            Lock sync(*this);
            func(_object);
        }

        MetricsPtr
        clone() const
        {
            Lock sync(*this);
            TPtr metrics = TPtr::dynamicCast(_object->ice_clone());
            for(typename std::map<std::string, std::pair<MetricsMapIPtr, SubMapMember> >::const_iterator p =
                    _subMaps.begin(); p != _subMaps.end(); ++p)
            {
                metrics.get()->*p->second.second = p->second.first->getMetrics();
            }
            return metrics;
        }

    private:

        friend class MetricsMapT;
        MetricsMapT* _map;
        TPtr _object;
        StringIntDict _failures;
        std::map<std::string, std::pair<MetricsMapIPtr, SubMapMember> > _subMaps;
        typename std::list<EntryT*>::iterator _detachedPos;
    };
    typedef IceUtil::Handle<EntryT> EntryTPtr;

    MetricsMapT(const std::string& mapPrefix,
                const Ice::PropertiesPtr& properties,
                const std::map<std::string, std::pair<SubMapMember, MetricsMapFactoryPtr> >& subMaps) : 
        MetricsMapI(mapPrefix, properties)
    {
        typename std::map<std::string, std::pair<SubMapMember, MetricsMapFactoryPtr> >::const_iterator p;
        for(p = subMaps.begin(); p != subMaps.end(); ++p)
        {
            const std::string subMapsPrefix = mapPrefix + "Map.";
            std::string subMapPrefix = subMapsPrefix + p->first;
            if(properties->getPropertiesForPrefix(subMapPrefix).empty())
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
                                           std::make_pair(p->second.first, 
                                                          p->second.second->create(subMapPrefix, properties))));
        }
    }

    MetricsMapT(const MetricsMapT& other) : MetricsMapI(other)
    {
    }

    ~MetricsMapT()
    {
        for(typename std::map<std::string, EntryTPtr>::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
        {
            p->second->destroy();
        }
    }

    virtual MetricsMap
    getMetrics() const
    {
        MetricsMap objects;
        
        Lock sync(*this);
        for(typename std::map<std::string, EntryTPtr>::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
        {
            objects.push_back(p->second->clone());
        }
        return objects;
    }
    
    virtual MetricsFailuresSeq
    getFailures()
    {
        MetricsFailuresSeq failures;
        
        Lock sync(*this);
        for(typename std::map<std::string, EntryTPtr>::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
        {
            MetricsFailures f = p->second->getFailures();
            if(!f.failures.empty())
            {
                failures.push_back(f);
            }
        }
        return failures;
    }
    
    virtual MetricsFailures
    getFailures(const std::string& id)
    {
        Lock sync(*this);
        typename std::map<std::string, EntryTPtr>::const_iterator p = _objects.find(id);
        if(p != _objects.end())
        {
            return p->second->getFailures();
        }
        return MetricsFailures();
    }


    std::pair<MetricsMapIPtr, SubMapMember>
    createSubMap(const std::string& subMapName)
    {
        typename std::map<std::string, std::pair<SubMapMember, MetricsMapIPtr> >::const_iterator p =
            _subMaps.find(subMapName);
        if(p != _subMaps.end())
        {
            return std::make_pair(p->second.second->clone(), p->second.first);
        }
        return std::make_pair(MetricsMapIPtr(), static_cast<SubMapMember>(0));
    }

    EntryTPtr 
    getMatching(const MetricsHelperT<T>& helper)
    {
        //
        // Check the accept and reject filters.
        //
        for(std::vector<RegExpPtr>::const_iterator p = _accept.begin(); p != _accept.end(); ++p)
        {
            if(!(*p)->match(helper))
            {
                return 0;
            }
        }
        
        for(std::vector<RegExpPtr>::const_iterator p = _reject.begin(); p != _reject.end(); ++p)
        {
            if((*p)->match(helper))
            {
                return 0;
            }
        }

        //
        // Compute the key from the GroupBy property.
        //
        std::string key;
        if(_groupByAttributes.size() == 1)
        {
            key = helper(_groupByAttributes.front());
        }
        else
        {
            std::ostringstream os;
            std::vector<std::string>::const_iterator q = _groupBySeparators.begin();
            for(std::vector<std::string>::const_iterator p = _groupByAttributes.begin(); p != _groupByAttributes.end();
                ++p)
            {
                os << helper(*p);
                if(q != _groupBySeparators.end())
                {
                    os << *q++;
                }
            }
            key = os.str();
        }

        //
        // Lookup the metrics object.
        // 
        Lock sync(*this);
        typename std::map<std::string, EntryTPtr>::const_iterator p = _objects.find(key);
        if(p == _objects.end())
        {
            p = _objects.insert(make_pair(key, newEntry(key))).first;
        }
        return p->second;
    }
    
private:

    virtual EntryTPtr newEntry(const std::string& id)
    {
        TPtr t = new T();
        t->id = id;
        return new EntryT(this, t, _detachedQueue.end());
    }

    virtual MetricsMapI* clone() const
    {
        return new MetricsMapT<MetricsType>(*this);
    }

    void detached(EntryT* entry)
    {
        if(_retain == 0)
        {
            return;
        }

        Lock sync(*this);
        assert(static_cast<int>(_detachedQueue.size()) <= _retain);

        // If the entry is already detached and in the queue, just move it to the back.
        if(entry->_detachedPos != _detachedQueue.end())
        {
            _detachedQueue.splice(_detachedQueue.end(), _detachedQueue, entry->_detachedPos);
            entry->_detachedPos = --_detachedQueue.end();
            return;
        }

        // Otherwise, compress the queue by removing entries which are no longer detached.
        if(static_cast<int>(_detachedQueue.size()) == _retain)
        {
            // Remove entries which are no longer detached
            typename std::list<EntryT*>::iterator p = _detachedQueue.begin();
            while(p != _detachedQueue.end())
            {
                if(!(*p)->isDetached())
                {
                    (*p)->_detachedPos = _detachedQueue.end();
                    p = _detachedQueue.erase(p);
                }
                else
                {
                    ++p;
                }
            }
        }

        // If there's still no room, remove the oldest entry (at the front).
        if(static_cast<int>(_detachedQueue.size()) == _retain)
        {
            _objects.erase(_detachedQueue.front()->_object->id);
            _detachedQueue.pop_front();
        }

        // Add the entry at the back of the queue.
        _detachedQueue.push_back(entry);
        entry->_detachedPos = --_detachedQueue.end();
    }

    friend class EntryT;
    
    std::map<std::string, EntryTPtr> _objects;
    std::list<EntryT*> _detachedQueue;
    std::map<std::string, std::pair<SubMapMember, MetricsMapIPtr> > _subMaps;
};

template<class MetricsType> class MetricsMapFactoryT : public MetricsMapFactory
{
public:

    virtual MetricsMapIPtr
    create(const std::string& mapPrefix, const Ice::PropertiesPtr& properties)
    {
        return new MetricsMapT<MetricsType>(mapPrefix, properties, _subMaps);
    }

    template<class SubMapMetricsType> void
    registerSubMap(const std::string& subMap, MetricsMap MetricsType::* member)
    {
        _subMaps[subMap] = make_pair(member, new MetricsMapFactoryT<SubMapMetricsType>());
    }

private:

    std::map<std::string, std::pair<MetricsMap MetricsType::*, MetricsMapFactoryPtr> > _subMaps;
};

class MetricsViewI : public IceUtil::Shared
{
public:
    
    MetricsViewI(const std::string&);

    void update(const Ice::PropertiesPtr&, const std::map<std::string, MetricsMapFactoryPtr>&, std::set<std::string>&);

    MetricsView getMetrics();
    MetricsFailuresSeq getFailures(const std::string&);
    MetricsFailures getFailures(const std::string&, const std::string&);

    std::vector<std::string> getMaps() const;

    MetricsMapIPtr getMap(const std::string&) const;

private:

    const std::string _name;
    std::map<std::string, MetricsMapIPtr> _maps;
};
typedef IceUtil::Handle<MetricsViewI> MetricsViewIPtr;

class MetricsAdminI : public MetricsAdmin, private IceUtil::Mutex
{
public:

    MetricsAdminI(const ::Ice::PropertiesPtr&, const Ice::LoggerPtr&);

    void addUpdater(const std::string&, const UpdaterPtr&);
    void updateViews();

    template<class MetricsType> void 
    registerMap(const std::string& map)
    {
        _factories[map] = new MetricsMapFactoryT<MetricsType>();
    }

    template<class MemberMetricsType, class MetricsType> void
    registerSubMap(const std::string& map, const std::string& subMap, MetricsMap MetricsType::* member)
    {
        std::map<std::string, MetricsMapFactoryPtr>::const_iterator p = _factories.find(map);
        assert(p != _factories.end());

        MetricsMapFactoryT<MetricsType>* factory = dynamic_cast<MetricsMapFactoryT<MetricsType>*>(p->second.get());
        factory->template registerSubMap<MemberMetricsType>(subMap, member);
    }

    virtual Ice::StringSeq getMetricsViewNames(const ::Ice::Current&);
    virtual MetricsView getMetricsView(const std::string&, const ::Ice::Current&);
    virtual MetricsFailuresSeq getMapMetricsFailures(const std::string&, const std::string&, const ::Ice::Current&);
    virtual MetricsFailures getMetricsFailures(const std::string&, const std::string&, const std::string&,
                                               const ::Ice::Current&);

    std::vector<MetricsMapIPtr> getMaps(const std::string&) const;

    const Ice::LoggerPtr& getLogger() const;

private:

    std::map<std::string, MetricsViewIPtr> _views;
    std::map<std::string, UpdaterPtr> _updaters;
    std::map<std::string, MetricsMapFactoryPtr> _factories;

    const Ice::PropertiesPtr _properties;
    const Ice::LoggerPtr _logger;
};
typedef IceUtil::Handle<MetricsAdminI> MetricsAdminIPtr;

};

#endif
