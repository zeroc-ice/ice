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

#include <Ice/Properties.h>
#include <Ice/PropertiesAdmin.h>
#include <Ice/Initialize.h>

#include <Ice/Metrics.h>

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
template<typename T> class MetricsHelperT;

}

namespace IceInternal
{

class MetricsMapI : public IceUtil::Shared
{
public:

    class RegExp : public IceUtil::Shared
    {
    public:
        
        RegExp(const std::string&, const std::string&);
        ~RegExp();

        template<typename T> bool
        match(const IceMX::MetricsHelperT<T>& helper, bool reject)
        {
            std::string value;
            try
            {
                value = helper(_attribute);
            }
            catch(const std::exception&)
            {
                return !reject;
            }
            return match(value);
        }

    private:

        bool match(const std::string&);
        
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

    virtual IceMX::MetricsFailuresSeq getFailures() = 0;
    virtual IceMX::MetricsFailures getFailures(const std::string&) = 0;
    virtual IceMX::MetricsMap getMetrics() const = 0;

    virtual MetricsMapI* clone() const = 0;

    void validateProperties(const std::string&, const Ice::PropertiesPtr&, const std::vector<std::string>&);
    const Ice::PropertyDict& getProperties() const;

protected:

    const Ice::PropertyDict _properties;
    const std::vector<std::string> _groupByAttributes;
    const std::vector<std::string> _groupBySeparators;
    const int _retain;
    const std::vector<RegExpPtr> _accept;
    const std::vector<RegExpPtr> _reject;
};
typedef IceUtil::Handle<MetricsMapI> MetricsMapIPtr;

class MetricsMapFactory : public Ice::LocalObject
{
public:

    MetricsMapFactory(IceMX::Updater*);

    virtual MetricsMapIPtr create(const std::string&, const Ice::PropertiesPtr&) = 0;
    
    void update();

private:
    
    IceMX::Updater* _updater;
};
typedef IceUtil::Handle<MetricsMapFactory> MetricsMapFactoryPtr;

template<class MetricsType> class MetricsMapT : public MetricsMapI, private IceUtil::Mutex
{
public:

    typedef MetricsType T;
    typedef IceInternal::Handle<MetricsType> TPtr;

    typedef IceMX::MetricsMap MetricsType::* SubMapMember;

    class EntryT : public Ice::LocalObject, private IceUtil::Mutex
    {
    public:

        EntryT(MetricsMapT* map, const TPtr& object, const typename std::list<EntryT*>::iterator& p) : 
            _map(map), _object(object), _detachedPos(p)
        {
        }

        void 
        destroy()
        {
            Lock sync(*this);
            _map = 0;
        }

        void  
        failed(const std::string& exceptionName)
        {
            Lock sync(*this);
            ++_object->failures;
            ++_failures[exceptionName];
        }

        IceMX::MetricsFailures 
        getFailures() const
        {
            IceMX::MetricsFailures f;
    
            Lock sync(*this);
            f.id = _object->id;
            f.failures = _failures;
            return f;
        }

        template<typename MemberMetricsType> typename MetricsMapT<MemberMetricsType>::EntryTPtr
        getMatching(const std::string& mapName, const IceMX::MetricsHelperT<MemberMetricsType>& helper)
        {
            MetricsMapIPtr m;
            {
                Lock sync(*this);
                typename std::map<std::string, std::pair<MetricsMapIPtr, SubMapMember> >::iterator p = 
                    _subMaps.find(mapName);
                if(p == _subMaps.end())
                {
                    if(_map == 0)
                    {
                        return 0;
                    }
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
                m = p->second.first;
            }
                
            MetricsMapT<MemberMetricsType>* map = dynamic_cast<MetricsMapT<MemberMetricsType>*>(m.get());
            assert(map);
            return map->getMatching(helper);
        }

        void
        attach(const IceMX::MetricsHelperT<T>& helper)
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

        bool 
        isDetached() const
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

        IceMX::MetricsPtr
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
        IceMX::StringIntDict _failures;
        std::map<std::string, std::pair<MetricsMapIPtr, SubMapMember> > _subMaps;
        typename std::list<EntryT*>::iterator _detachedPos;
    };
    typedef IceUtil::Handle<EntryT> EntryTPtr;

    MetricsMapT(const std::string& mapPrefix,
                const Ice::PropertiesPtr& properties,
                const std::map<std::string, std::pair<SubMapMember, MetricsMapFactoryPtr> >& subMaps) : 
        MetricsMapI(mapPrefix, properties)
    {
        std::vector<std::string> subMapNames;
        typename std::map<std::string, std::pair<SubMapMember, MetricsMapFactoryPtr> >::const_iterator p;
        for(p = subMaps.begin(); p != subMaps.end(); ++p)
        {
            subMapNames.push_back(p->first);
            const std::string subMapsPrefix = mapPrefix + "Map.";
            std::string subMapPrefix = subMapsPrefix + p->first + '.';
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
        validateProperties(mapPrefix, properties, subMapNames);
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

    virtual IceMX::MetricsMap
    getMetrics() const
    {
        IceMX::MetricsMap objects;
        
        Lock sync(*this);
        for(typename std::map<std::string, EntryTPtr>::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
        {
            objects.push_back(p->second->clone());
        }
        return objects;
    }
    
    virtual IceMX::MetricsFailuresSeq
    getFailures()
    {
        IceMX::MetricsFailuresSeq failures;
        
        Lock sync(*this);
        for(typename std::map<std::string, EntryTPtr>::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
        {
            IceMX::MetricsFailures f = p->second->getFailures();
            if(!f.failures.empty())
            {
                failures.push_back(f);
            }
        }
        return failures;
    }
    
    virtual IceMX::MetricsFailures
    getFailures(const std::string& id)
    {
        Lock sync(*this);
        typename std::map<std::string, EntryTPtr>::const_iterator p = _objects.find(id);
        if(p != _objects.end())
        {
            return p->second->getFailures();
        }
        return IceMX::MetricsFailures();
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
    getMatching(const IceMX::MetricsHelperT<T>& helper)
    {
        //
        // Check the accept and reject filters.
        //
        for(std::vector<RegExpPtr>::const_iterator p = _accept.begin(); p != _accept.end(); ++p)
        {
            if(!(*p)->match(helper, false))
            {
                return 0;
            }
        }
        
        for(std::vector<RegExpPtr>::const_iterator p = _reject.begin(); p != _reject.end(); ++p)
        {
            if((*p)->match(helper, true))
            {
                return 0;
            }
        }

        //
        // Compute the key from the GroupBy property.
        //
        std::string key;
        try
        {
            if(_groupByAttributes.size() == 1)
            {
                key = helper(_groupByAttributes.front());
            }
            else
            {
                std::ostringstream os;
                std::vector<std::string>::const_iterator q = _groupBySeparators.begin();
                for(std::vector<std::string>::const_iterator p = _groupByAttributes.begin();
                    p != _groupByAttributes.end(); ++p)
                {
                    os << helper(*p);
                    if(q != _groupBySeparators.end())
                    {
                        os << *q++;
                    }
                }
                key = os.str();
            }
        }
        catch(const std::exception&)
        {
            return 0;
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

    MetricsMapFactoryT(IceMX::Updater* updater) : MetricsMapFactory(updater)
    {
    }

    virtual MetricsMapIPtr
    create(const std::string& mapPrefix, const Ice::PropertiesPtr& properties)
    {
        return new MetricsMapT<MetricsType>(mapPrefix, properties, _subMaps);
    }

    template<class SubMapMetricsType> void
    registerSubMap(const std::string& subMap, IceMX::MetricsMap MetricsType::* member)
    {
        _subMaps[subMap] = make_pair(member, new MetricsMapFactoryT<SubMapMetricsType>(0));
    }

private:

    std::map<std::string, std::pair<IceMX::MetricsMap MetricsType::*, MetricsMapFactoryPtr> > _subMaps;
};

class MetricsViewI : public IceUtil::Shared
{
public:
    
    MetricsViewI(const std::string&);

    bool addOrUpdateMap(const Ice::PropertiesPtr&, const std::string&, const MetricsMapFactoryPtr&, 
                        const Ice::LoggerPtr&);
    bool removeMap(const std::string&);

    IceMX::MetricsView getMetrics();
    IceMX::MetricsFailuresSeq getFailures(const std::string&);
    IceMX::MetricsFailures getFailures(const std::string&, const std::string&);

    std::vector<std::string> getMaps() const;

    MetricsMapIPtr getMap(const std::string&) const;

private:

    const std::string _name;
    std::map<std::string, MetricsMapIPtr> _maps;
};
typedef IceUtil::Handle<MetricsViewI> MetricsViewIPtr;

class MetricsAdminI : public IceMX::MetricsAdmin, public Ice::PropertiesAdminUpdateCallback, private IceUtil::Mutex
{
public:

    MetricsAdminI(const ::Ice::PropertiesPtr&, const Ice::LoggerPtr&);

    void updateViews();

    template<class MetricsType> void 
    registerMap(const std::string& map, IceMX::Updater* updater)
    {
        bool updated;
        MetricsMapFactoryPtr factory;
        {
            Lock sync(*this);
            factory = new MetricsMapFactoryT<MetricsType>(updater);
            _factories[map] = factory;
            updated = addOrUpdateMap(map, factory);
        }
        if(updated)
        {
            factory->update();
        }
    }

    template<class MemberMetricsType, class MetricsType> void
    registerSubMap(const std::string& map, const std::string& subMap, IceMX::MetricsMap MetricsType::* member)
    {
        bool updated;
        IceUtil::Handle<MetricsMapFactoryT<MetricsType> > factory;
        {
            Lock sync(*this);
            
            std::map<std::string, MetricsMapFactoryPtr>::const_iterator p = _factories.find(map);
            if(p == _factories.end())
            {
                return;
            }
            factory = dynamic_cast<MetricsMapFactoryT<MetricsType>*>(p->second.get());
            factory->template registerSubMap<MemberMetricsType>(subMap, member);
            removeMap(map);
            updated = addOrUpdateMap(map, factory);
        }
        if(updated)
        {
            factory->update();
        }
    }

    void unregisterMap(const std::string&);

    virtual Ice::StringSeq getMetricsViewNames(const ::Ice::Current&);
    virtual IceMX::MetricsView getMetricsView(const std::string&, Ice::Long&, const ::Ice::Current&);
    virtual IceMX::MetricsFailuresSeq getMapMetricsFailures(const std::string&, const std::string&, 
                                                            const ::Ice::Current&);
    virtual IceMX::MetricsFailures getMetricsFailures(const std::string&, const std::string&, const std::string&,
                                                      const ::Ice::Current&);

    std::vector<MetricsMapIPtr> getMaps(const std::string&) const;

    const Ice::LoggerPtr& getLogger() const;

    void setProperties(const Ice::PropertiesPtr&);

private:

    void updated(const Ice::PropertyDict&);

    bool addOrUpdateMap(const std::string&, const MetricsMapFactoryPtr&);
    bool removeMap(const std::string&);

    std::map<std::string, MetricsViewIPtr> _views;
    std::map<std::string, MetricsMapFactoryPtr> _factories;

    const Ice::LoggerPtr _logger;
    Ice::PropertiesPtr _properties;
};
typedef IceUtil::Handle<MetricsAdminI> MetricsAdminIPtr;

};

#endif
