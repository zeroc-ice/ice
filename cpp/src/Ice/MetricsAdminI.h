// Copyright (c) ZeroC, Inc.

#ifndef ICE_METRICS_ADMIN_I_H
#define ICE_METRICS_ADMIN_I_H

#include "Ice/Config.h"
#include "Ice/Initialize.h"
#include "Ice/Metrics.h"
#include "Ice/Properties.h"

#include <cassert>
#include <list>
#include <mutex>
#include <regex>

namespace IceMX
{
    class Updater;
    template<typename T> class MetricsHelperT;
}

namespace IceInternal
{
    class ICE_API MetricsMapI;
    using MetricsMapIPtr = std::shared_ptr<MetricsMapI>;

    class ICE_API MetricsMapI : public std::enable_shared_from_this<MetricsMapI>
    {
    public:
        class ICE_API RegExp
        {
        public:
            RegExp(std::string, const std::string&);

            template<typename T> bool match(const IceMX::MetricsHelperT<T>& helper, bool reject)
            {
                std::string value;
                try
                {
                    value = helper(_attribute);
                }
                catch (const std::exception&)
                {
                    return !reject;
                }
                return match(value);
            }

        private:
            bool match(const std::string&);

            const std::string _attribute;

            std::regex _regex;
        };
        using RegExpPtr = std::shared_ptr<RegExp>;

        virtual ~MetricsMapI();

        MetricsMapI(const std::string&, const Ice::PropertiesPtr&);
        MetricsMapI(const MetricsMapI&);

        virtual void destroy() = 0;

        virtual IceMX::MetricsFailuresSeq getFailures() = 0;
        virtual IceMX::MetricsFailures getFailures(const std::string&) = 0;
        [[nodiscard]] virtual IceMX::MetricsMap getMetrics() const = 0;

        [[nodiscard]] virtual MetricsMapIPtr clone() const = 0;

        [[nodiscard]] const Ice::PropertyDict& getProperties() const;

    protected:
        const Ice::PropertyDict _properties;
        const std::vector<std::string> _groupByAttributes;
        const std::vector<std::string> _groupBySeparators;
        const int _retain;
        const std::vector<RegExpPtr> _accept;
        const std::vector<RegExpPtr> _reject;
    };

    class ICE_API MetricsMapFactory
    {
    public:
        virtual ~MetricsMapFactory();

        MetricsMapFactory(IceMX::Updater*);

        virtual MetricsMapIPtr create(const std::string&, const Ice::PropertiesPtr&) = 0;

        void update();

    private:
        IceMX::Updater* _updater;
    };
    using MetricsMapFactoryPtr = std::shared_ptr<MetricsMapFactory>;

    template<class MetricsType> class MetricsMapT : public MetricsMapI
    {
    public:
        using T = MetricsType;
        using TPtr = std::shared_ptr<MetricsType>;
        using MetricsMapTPtr = std::shared_ptr<MetricsMapT>;

        using SubMapMember = IceMX::MetricsMap MetricsType::*;

        class EntryT;
        using EntryTPtr = std::shared_ptr<EntryT>;

        class EntryT : public std::enable_shared_from_this<EntryT>
        {
        public:
            EntryT(MetricsMapTPtr map, TPtr object, const typename std::list<EntryTPtr>::iterator& p)
                : _map(std::move(map)),
                  _object(std::move(object)),
                  _detachedPos(p)
            {
            }

            ~EntryT()
            {
                assert(_object->total > 0);
                for (const auto& subMap : _subMaps)
                {
                    subMap.second.first->destroy(); // Break cyclic reference counts.
                }
            }

            void failed(const std::string& exceptionName)
            {
                std::lock_guard lock(_map->_mutex);
                ++_object->failures;
                ++_failures[exceptionName];
            }

            template<typename MemberMetricsType>
            typename MetricsMapT<MemberMetricsType>::EntryTPtr
            getMatching(const std::string& mapName, const IceMX::MetricsHelperT<MemberMetricsType>& helper)
            {
                MetricsMapIPtr m;
                {
                    std::lock_guard lock(_map->_mutex);
                    auto p = _subMaps.find(mapName);
                    if (p == _subMaps.end())
                    {
                        std::pair<MetricsMapIPtr, SubMapMember> map = _map->createSubMap(mapName);
                        if (map.first)
                        {
                            p = _subMaps.insert(make_pair(mapName, map)).first;
                        }
                    }
                    if (p == _subMaps.end())
                    {
                        return nullptr;
                    }
                    m = p->second.first;
                }

                auto map = std::dynamic_pointer_cast<MetricsMapT<MemberMetricsType>>(m);
                assert(map);
                return map->getMatching(helper);
            }

            void detach(std::int64_t lifetime)
            {
                std::lock_guard lock(_map->_mutex);
                _object->totalLifetime += lifetime;
                if (--_object->current == 0)
                {
                    _map->detached(this->shared_from_this());
                }
            }

            template<typename Function> void execute(Function func)
            {
                std::lock_guard lock(_map->_mutex);
                func(_object);
            }

            MetricsMapT* getMap() { return _map.get(); }

        private:
            [[nodiscard]] IceMX::MetricsFailures getFailures() const
            {
                IceMX::MetricsFailures f;
                f.id = _object->id;
                f.failures = _failures;
                return f;
            }

            [[nodiscard]] IceMX::MetricsPtr clone() const
            {
                TPtr metrics = std::dynamic_pointer_cast<T>(_object->ice_clone());
                for (auto p = _subMaps.begin(); p != _subMaps.end(); ++p)
                {
                    metrics.get()->*p->second.second = p->second.first->getMetrics();
                }
                return metrics;
            }

            [[nodiscard]] bool isDetached() const { return _object->current == 0; }

            void attach(const IceMX::MetricsHelperT<T>& helper)
            {
                ++_object->total;
                ++_object->current;
                helper.initMetrics(_object);
            }

            friend class MetricsMapT;
            MetricsMapTPtr _map;
            TPtr _object;
            IceMX::StringIntDict _failures;
            std::map<std::string, std::pair<MetricsMapIPtr, SubMapMember>> _subMaps;
            typename std::list<EntryTPtr>::iterator _detachedPos;
        };

        MetricsMapT(
            const std::string& mapPrefix,
            const Ice::PropertiesPtr& properties,
            const std::map<std::string, std::pair<SubMapMember, MetricsMapFactoryPtr>>& subMaps)
            : MetricsMapI(mapPrefix, properties)
        {
            std::vector<std::string> subMapNames;
            typename std::map<std::string, std::pair<SubMapMember, MetricsMapFactoryPtr>>::const_iterator p;
            for (p = subMaps.begin(); p != subMaps.end(); ++p)
            {
                subMapNames.push_back(p->first);
                const std::string subMapsPrefix = mapPrefix + "Map.";
                std::string subMapPrefix = subMapsPrefix + p->first + '.';
                if (properties->getPropertiesForPrefix(subMapPrefix).empty())
                {
                    if (properties->getPropertiesForPrefix(subMapsPrefix).empty())
                    {
                        subMapPrefix = mapPrefix;
                    }
                    else
                    {
                        continue; // This sub-map isn't configured.
                    }
                }
                _subMaps.insert(std::make_pair(
                    p->first,
                    std::make_pair(p->second.first, p->second.second->create(subMapPrefix, properties))));
            }
        }

        MetricsMapT(const MetricsMapT& other) : MetricsMapI(other) {}

        std::shared_ptr<MetricsMapT> shared_from_this()
        {
            return std::static_pointer_cast<MetricsMapT>(MetricsMapI::shared_from_this());
        }

        void destroy() override
        {
            std::lock_guard lock(_mutex);
            _destroyed = true;
            _objects.clear();       // Break cyclic reference counts
            _detachedQueue.clear(); // Break cyclic reference counts
        }

        [[nodiscard]] IceMX::MetricsMap getMetrics() const override
        {
            IceMX::MetricsMap objects;

            std::lock_guard lock(_mutex);
            for (const auto& object : _objects)
            {
                objects.push_back(object.second->clone());
            }
            return objects;
        }

        IceMX::MetricsFailuresSeq getFailures() override
        {
            IceMX::MetricsFailuresSeq failures;

            std::lock_guard lock(_mutex);
            for (const auto& object : _objects)
            {
                IceMX::MetricsFailures f = object.second->getFailures();
                if (!f.failures.empty())
                {
                    failures.push_back(f);
                }
            }
            return failures;
        }

        IceMX::MetricsFailures getFailures(const std::string& id) override
        {
            std::lock_guard lock(_mutex);
            auto p = _objects.find(id);
            if (p != _objects.end())
            {
                return p->second->getFailures();
            }
            return {};
        }

        std::pair<MetricsMapIPtr, SubMapMember> createSubMap(const std::string& subMapName)
        {
            auto p = _subMaps.find(subMapName);
            if (p != _subMaps.end())
            {
                return std::pair<MetricsMapIPtr, SubMapMember>(
                    p->second.second->clone()->shared_from_this(),
                    p->second.first);
            }
            return std::pair<MetricsMapIPtr, SubMapMember>(MetricsMapIPtr(nullptr), static_cast<SubMapMember>(nullptr));
        }

        EntryTPtr getMatching(const IceMX::MetricsHelperT<T>& helper, const EntryTPtr& previous = EntryTPtr())
        {
            //
            // Check the accept and reject filters.
            //
            for (const auto& filter : _accept)
            {
                if (!filter->match(helper, false))
                {
                    return nullptr;
                }
            }

            for (const auto& filter : _reject)
            {
                if (filter->match(helper, true))
                {
                    return nullptr;
                }
            }

            //
            // Compute the key from the GroupBy property.
            //
            std::string key;
            try
            {
                if (_groupByAttributes.size() == 1)
                {
                    key = helper(_groupByAttributes.front());
                }
                else
                {
                    std::ostringstream os;
                    auto q = _groupBySeparators.begin();
                    for (auto p = _groupByAttributes.begin(); p != _groupByAttributes.end(); ++p)
                    {
                        os << helper(*p);
                        if (q != _groupBySeparators.end())
                        {
                            os << *q++;
                        }
                    }
                    key = os.str();
                }
            }
            catch (const std::exception&)
            {
                return nullptr;
            }

            //
            // Lookup the metrics object.
            //
            std::lock_guard lock(_mutex);
            if (_destroyed)
            {
                return nullptr;
            }

            if (previous && previous->_object->id == key)
            {
                assert(_objects[key] == previous);
                return previous;
            }

            auto p = _objects.find(key);
            if (p == _objects.end())
            {
                TPtr t = std::make_shared<T>();
                t->id = key;

                p = _objects
                        .insert(typename std::map<std::string, EntryTPtr>::value_type(
                            key,
                            std::make_shared<EntryT>(shared_from_this(), t, _detachedQueue.end())))
                        .first;
            }
            p->second->attach(helper);
            return p->second;
        }

    private:
        [[nodiscard]] MetricsMapIPtr clone() const override
        {
            return std::make_shared<MetricsMapT<MetricsType>>(*this);
        }

        void detached(const EntryTPtr& entry)
        {
            // This is called with the map mutex locked.

            if (_retain == 0 || _destroyed)
            {
                return;
            }

            assert(static_cast<int>(_detachedQueue.size()) <= _retain);

            // If the entry is already detached and in the queue, just move it to the back.
            if (entry->_detachedPos != _detachedQueue.end())
            {
                if (entry->_detachedPos != --_detachedQueue.end()) // If not already at the end
                {
                    _detachedQueue.splice(_detachedQueue.end(), _detachedQueue, entry->_detachedPos);
                    entry->_detachedPos = --_detachedQueue.end();
                }
                return;
            }

            // Otherwise, compress the queue by removing entries which are no longer detached.
            if (static_cast<int>(_detachedQueue.size()) == _retain)
            {
                // Remove entries which are no longer detached
                auto p = _detachedQueue.begin();
                while (p != _detachedQueue.end())
                {
                    if (!(*p)->isDetached())
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
            if (static_cast<int>(_detachedQueue.size()) == _retain)
            {
                _objects.erase(_detachedQueue.front()->_object->id);
                _detachedQueue.pop_front();
            }

            // Add the entry at the back of the queue.
            entry->_detachedPos = _detachedQueue.insert(_detachedQueue.end(), entry);
            assert(entry->_detachedPos != _detachedQueue.end());
            return;
        }

        friend class EntryT;

        bool _destroyed{false};
        std::map<std::string, EntryTPtr> _objects;
        std::list<EntryTPtr> _detachedQueue;
        std::map<std::string, std::pair<SubMapMember, MetricsMapIPtr>> _subMaps;
        mutable std::mutex _mutex;
    };

    template<class MetricsType> class MetricsMapFactoryT : public MetricsMapFactory
    {
    public:
        MetricsMapFactoryT(IceMX::Updater* updater) : MetricsMapFactory(updater) {}

        MetricsMapIPtr create(const std::string& mapPrefix, const Ice::PropertiesPtr& properties) override
        {
            return std::make_shared<MetricsMapT<MetricsType>>(mapPrefix, properties, _subMaps);
        }

        template<class SubMapMetricsType>
        void registerSubMap(const std::string& subMap, IceMX::MetricsMap MetricsType::* member)
        {
            _subMaps[subMap] = std::pair<IceMX::MetricsMap MetricsType::*, MetricsMapFactoryPtr>(
                member,
                std::make_shared<MetricsMapFactoryT<SubMapMetricsType>>(nullptr));
        }

    private:
        std::map<std::string, std::pair<IceMX::MetricsMap MetricsType::*, MetricsMapFactoryPtr>> _subMaps;
    };

    class MetricsViewI
    {
    public:
        MetricsViewI(std::string);

        void destroy();

        bool addOrUpdateMap(
            const Ice::PropertiesPtr&,
            const std::string&,
            const MetricsMapFactoryPtr&,
            const Ice::LoggerPtr&);
        bool removeMap(const std::string&);

        IceMX::MetricsView getMetrics();
        IceMX::MetricsFailuresSeq getFailures(const std::string&);
        IceMX::MetricsFailures getFailures(const std::string&, const std::string&);

        [[nodiscard]] std::vector<std::string> getMaps() const;

        [[nodiscard]] MetricsMapIPtr getMap(const std::string&) const;

    private:
        const std::string _name;
        std::map<std::string, MetricsMapIPtr> _maps;
    };
    using MetricsViewIPtr = std::shared_ptr<MetricsViewI>;

    class ICE_API MetricsAdminI : public IceMX::MetricsAdmin
    {
    public:
        MetricsAdminI(Ice::PropertiesPtr, Ice::LoggerPtr);
        ~MetricsAdminI() override;

        void destroy();

        void updateViews();

        template<class MetricsType> void registerMap(const std::string& map, IceMX::Updater* updater)
        {
            bool updated;
            MetricsMapFactoryPtr factory;
            {
                std::lock_guard lock(_mutex);
                factory = std::make_shared<MetricsMapFactoryT<MetricsType>>(updater);
                _factories[map] = factory;
                updated = addOrUpdateMap(map, factory);
            }
            if (updated)
            {
                factory->update();
            }
        }

        template<class MemberMetricsType, class MetricsType>
        void registerSubMap(const std::string& map, const std::string& subMap, IceMX::MetricsMap MetricsType::* member)
        {
            bool updated;
            std::shared_ptr<MetricsMapFactoryT<MetricsType>> factory;
            {
                std::lock_guard lock(_mutex);
                auto p = _factories.find(map);
                if (p == _factories.end())
                {
                    return;
                }
                factory = std::dynamic_pointer_cast<MetricsMapFactoryT<MetricsType>>(p->second);

                // use 'template' keyword to treat 'registerSubMap' as a dependent template name
                factory->template registerSubMap<MemberMetricsType>(subMap, member);
                removeMap(map);
                updated = addOrUpdateMap(map, factory);
            }
            if (updated)
            {
                factory->update();
            }
        }

        void unregisterMap(const std::string&);

        Ice::StringSeq getMetricsViewNames(Ice::StringSeq&, const Ice::Current&) override;

        void updated(const Ice::PropertyDict&);

        void enableMetricsView(std::string, const Ice::Current&) override;
        void disableMetricsView(std::string, const Ice::Current&) override;
        IceMX::MetricsView getMetricsView(std::string, std::int64_t&, const Ice::Current&) override;
        IceMX::MetricsFailuresSeq getMapMetricsFailures(std::string, std::string, const Ice::Current&) override;
        IceMX::MetricsFailures getMetricsFailures(std::string, std::string, std::string, const Ice::Current&) override;
        [[nodiscard]] std::vector<MetricsMapIPtr> getMaps(const std::string&) const;

        [[nodiscard]] const Ice::LoggerPtr& getLogger() const;

    private:
        MetricsViewIPtr getMetricsView(const std::string&);

        bool addOrUpdateMap(const std::string&, const MetricsMapFactoryPtr&);
        bool removeMap(const std::string&);

        std::map<std::string, MetricsViewIPtr> _views;
        std::set<std::string> _disabledViews;
        std::map<std::string, MetricsMapFactoryPtr> _factories;

        const Ice::LoggerPtr _logger;
        Ice::PropertiesPtr _properties;
        mutable std::mutex _mutex;
    };

    using MetricsAdminIPtr = std::shared_ptr<MetricsAdminI>;

};

#endif
