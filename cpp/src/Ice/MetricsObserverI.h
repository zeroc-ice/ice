// Copyright (c) ZeroC, Inc.

#ifndef ICEMX_METRICS_OBSERVER_I_H
#define ICEMX_METRICS_OBSERVER_I_H

#include "Ice/Connection.h"
#include "Ice/Endpoint.h"
#include "Ice/Instrumentation.h"
#include "Ice/Metrics.h"
#include "MetricsAdminI.h"
#include "MetricsFunctional.h"

#include <cassert>
#include <sstream>
#include <stdexcept>

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wweak-vtables"
#endif

namespace IceInternal
{
    class StopWatch
    {
    public:
        StopWatch() = default;

        void start() { _start = std::chrono::steady_clock::now(); }

        std::chrono::microseconds stop()
        {
            assert(isStarted());
            auto duration =
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - _start);
            _start = std::chrono::steady_clock::time_point();
            return duration;
        }

        [[nodiscard]] bool isStarted() const { return _start != std::chrono::steady_clock::time_point(); }

        std::chrono::microseconds delay()
        {
            return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - _start);
        }

    private:
        std::chrono::steady_clock::time_point _start;
    };
}

namespace IceMX
{
    template<typename T> class MetricsHelperT
    {
    public:
        virtual ~MetricsHelperT() = default;

        virtual std::string operator()(const std::string&) const = 0;

        virtual void initMetrics(const std::shared_ptr<T>&) const
        {
            // To be overridden in specialization to initialize state attributes
        }

    protected:
        template<typename Helper> class AttributeResolverT
        {
            class Resolver
            {
            public:
                Resolver(std::string name) : _name(std::move(name)) {}

                virtual ~Resolver() = default;

                virtual std::string operator()(const Helper* h) const = 0;

            protected:
                std::string _name;
            };

        public:
            AttributeResolverT() = default;

            ~AttributeResolverT()
            {
                for (const auto& attribute : _attributes)
                {
                    delete attribute.second;
                }
            }

            std::string operator()(const Helper* helper, const std::string& attribute) const
            {
                auto p = _attributes.find(attribute);
                if (p == _attributes.end())
                {
                    if (attribute == "none")
                    {
                        return "";
                    }
                    if (_default)
                    {
                        return (helper->*_default)(attribute);
                    }
                    throw std::invalid_argument(attribute);
                }
                return (*p->second)(helper);
            }

            void setDefault(std::string (Helper::*memberFn)(const std::string&) const) { _default = memberFn; }

            template<typename Y> void add(const std::string& name, Y Helper::* member)
            {
                _attributes.insert(typename std::map<std::string, Resolver*>::value_type(
                    name,
                    new HelperMemberResolver<Y>(name, member)));
            }

            template<typename Y> void add(const std::string& name, Y (Helper::*memberFn)() const)
            {
                _attributes.insert(typename std::map<std::string, Resolver*>::value_type(
                    name,
                    new HelperMemberFunctionResolver<Y>(name, memberFn)));
            }

            template<typename I, typename O, typename Y>
            void add(const std::string& name, O (Helper::*getFn)() const, Y I::* member)
            {
                _attributes.insert(typename std::map<std::string, Resolver*>::value_type(
                    name,
                    new MemberResolver<I, O, Y>(name, getFn, member)));
            }

            template<typename I, typename O, typename Y>
            void add(const std::string& name, O (Helper::*getFn)() const, Y (I::*memberFn)() const)
            {
                _attributes.insert(typename std::map<std::string, Resolver*>::value_type(
                    name,
                    new MemberFunctionResolver<I, O, Y>(name, getFn, memberFn)));
            }

            //
            // Since C++17 the noexcept-specification is part of the function type and we need a separate
            // overload to handle memberFn being noexcept
            //
            template<typename I, typename O, typename Y>
            void add(const std::string& name, O (Helper::*getFn)() const, Y (I::*memberFn)() const noexcept)
            {
                _attributes.insert(typename std::map<std::string, Resolver*>::value_type(
                    name,
                    new MemberFunctionResolver<I, O, Y>(name, getFn, memberFn)));
            }

        private:
            template<typename Y> class HelperMemberResolver : public Resolver
            {
            public:
                HelperMemberResolver(const std::string& name, Y Helper::* member) : Resolver(name), _member(member) {}

                std::string operator()(const Helper* r) const override { return toString(r->*_member); }

            private:
                Y Helper::* _member;
            };

            template<typename Y> class HelperMemberFunctionResolver : public Resolver
            {
            public:
                HelperMemberFunctionResolver(const std::string& name, Y (Helper::*memberFn)() const)
                    : Resolver(name),
                      _memberFn(memberFn)
                {
                }

                std::string operator()(const Helper* r) const override { return toString((r->*_memberFn)()); }

            private:
                Y (Helper::*_memberFn)() const;
            };

            template<typename I, typename O, typename Y> class MemberResolver : public Resolver
            {
            public:
                MemberResolver(const std::string& name, O (Helper::*getFn)() const, Y I::* member)
                    : Resolver(name),
                      _getFn(getFn),
                      _member(member)
                {
                }

                std::string operator()(const Helper* r) const override
                {
                    O o = (r->*_getFn)();
                    I* v = dynamicCast<I>(IceInternal::ReferenceWrapper<O>::get(o));
                    if (v)
                    {
                        return toString(v->*_member);
                    }
                    else
                    {
                        throw std::invalid_argument(Resolver::_name);
                    }
                }

            private:
                O (Helper::*_getFn)() const;
                Y I::* _member;
            };

            template<typename I, typename O, typename Y> class MemberFunctionResolver : public Resolver
            {
            public:
                MemberFunctionResolver(const std::string& name, O (Helper::*getFn)() const, Y (I::*memberFn)() const)
                    : Resolver(name),
                      _getFn(getFn),
                      _memberFn(memberFn)
                {
                }

                std::string operator()(const Helper* r) const override
                {
                    O o = (r->*_getFn)();
                    I* v = dynamicCast<I>(IceInternal::ReferenceWrapper<O>::get(o));
                    if (v)
                    {
                        return toString((v->*_memberFn)());
                    }
                    else
                    {
                        throw std::invalid_argument(Resolver::_name);
                    }
                }

            private:
                O (Helper::*_getFn)() const;
                Y (I::*_memberFn)() const;
            };

            template<typename I, typename V> static I* dynamicCast(V* v) { return dynamic_cast<I*>(v); }

            template<typename I> static I* dynamicCast(Ice::EndpointInfo* v)
            {
                for (Ice::EndpointInfo* info = v; info; info = info->underlying.get())
                {
                    I* i = dynamic_cast<I*>(info);
                    if (i)
                    {
                        return i;
                    }
                }
                return nullptr;
            }

            template<typename I> static I* dynamicCast(Ice::ConnectionInfo* v)
            {
                for (Ice::ConnectionInfo* info = v; info; info = info->underlying.get())
                {
                    I* i = dynamic_cast<I*>(info);
                    if (i)
                    {
                        return i;
                    }
                }
                return nullptr;
            }

            template<typename I> static std::string toString(const I& v)
            {
                std::ostringstream os;
                os << v;
                return os.str();
            }

            static const std::string toString(const Ice::ObjectPrx& p) { return p->ice_toString(); }

            template<typename Prx, std::enable_if_t<std::is_base_of_v<Ice::ObjectPrx, Prx>, bool> = true>
            static const std::string toString(const std::optional<Prx>& p)
            {
                return p ? toString(p.value()) : "";
            }

            static const std::string& toString(const std::string& s) { return s; }

            static std::string toString(const Ice::EndpointPtr& e) { return e->toString(); }

            static std::string toString(const Ice::ConnectionPtr& e) { return e->toString(); }

            static std::string toString(bool v) { return v ? "true" : "false"; }

            std::map<std::string, Resolver*> _attributes;
            std::string (Helper::*_default)(const std::string&) const = nullptr;
        };
    };

    class Updater
    {
    public:
        virtual void update() = 0;
    };
    using UpdaterPtr = std::shared_ptr<Updater>;

    template<typename T> class UpdaterT final : public Updater
    {
    public:
        UpdaterT(const std::shared_ptr<T>& updater, void (T::*fn)()) : _updater(updater), _fn(fn) {}

        void update() override { (_updater.get()->*_fn)(); }

    private:
        const std::shared_ptr<T> _updater;
        void (T::*_fn)();
    };

    template<typename T> UpdaterPtr newUpdater(const std::shared_ptr<T>& updater, void (T::*fn)())
    {
        if (updater)
        {
            return std::make_shared<UpdaterT<T>>(updater, fn);
        }
        else
        {
            return nullptr;
        }
    }

    template<typename T> class ObserverT : public virtual Ice::Instrumentation::Observer
    {
    public:
        using MetricsType = T;
        using EntryPtrType = typename IceInternal::MetricsMapT<MetricsType>::EntryTPtr;
        using EntrySeqType = std::vector<EntryPtrType>;

        ObserverT() = default;

        void attach() override
        {
            if (!_watch.isStarted())
            {
                _watch.start();
            }
        }

        void detach() override
        {
            std::chrono::microseconds lifetime = _previousDelay + _watch.stop();
            for (const auto& object : _objects)
            {
                object->detach(lifetime.count());
            }
        }

        void failed(const std::string& exceptionName) override
        {
            for (const auto& object : _objects)
            {
                object->failed(exceptionName);
            }
        }

        template<typename Function> void forEach(const Function& func)
        {
            for (const auto& object : _objects)
            {
                object->execute(func);
            }
        }

        void init(const MetricsHelperT<MetricsType>& /*helper*/, EntrySeqType& objects, ObserverT* previous = nullptr)
        {
            _objects.swap(objects);

            if (previous == nullptr)
            {
                return;
            }

            _previousDelay = previous->_previousDelay + previous->_watch.delay();
            //
            // Detach entries from previous observer which are no longer
            // attached to this new observer.
            //
            for (const auto& previousObject : previous->_objects)
            {
                if (find(_objects.begin(), _objects.end(), previousObject) == _objects.end())
                {
                    previousObject->detach(_previousDelay.count());
                }
            }
        }

        EntryPtrType getEntry(IceInternal::MetricsMapT<MetricsType>* map)
        {
            for (const auto& object : _objects)
            {
                if (object->getMap() == map)
                {
                    return object;
                }
            }
            return nullptr;
        }

        template<typename ObserverImpl, typename ObserverMetricsType>
        std::shared_ptr<ObserverImpl>
        getObserver(const std::string& mapName, const MetricsHelperT<ObserverMetricsType>& helper)
        {
            std::vector<typename IceInternal::MetricsMapT<ObserverMetricsType>::EntryTPtr> metricsObjects;
            for (const auto& object : _objects)
            {
                typename IceInternal::MetricsMapT<ObserverMetricsType>::EntryTPtr e =
                    object->getMatching(mapName, helper);
                if (e)
                {
                    metricsObjects.push_back(e);
                }
            }

            if (metricsObjects.empty())
            {
                return nullptr;
            }

            std::shared_ptr<ObserverImpl> obsv = std::make_shared<ObserverImpl>();
            obsv->init(helper, metricsObjects);
            return obsv;
        }

    private:
        EntrySeqType _objects;
        IceInternal::StopWatch _watch;
        std::chrono::microseconds _previousDelay{0};
    };

    template<typename ObserverImplType> class ObserverFactoryT : public Updater
    {
    public:
        using ObserverImplPtrType = std::shared_ptr<ObserverImplType>;
        using MetricsType = typename ObserverImplType::MetricsType;
        using MetricsMapSeqType = std::vector<std::shared_ptr<IceInternal::MetricsMapT<MetricsType>>>;

        ObserverFactoryT(IceInternal::MetricsAdminIPtr metrics, const std::string& name)
            : _metrics(std::move(metrics)),
              _name(name),
              _enabled{false}
        {
            _metrics->registerMap<MetricsType>(name, this);
        }

        ~ObserverFactoryT()
        {
            if (_metrics)
            {
                _metrics->unregisterMap(_name);
            }
        }

        ObserverImplPtrType getObserver(const MetricsHelperT<MetricsType>& helper)
        {
            std::lock_guard lock(_mutex);
            if (!_metrics)
            {
                return nullptr;
            }

            typename ObserverImplType::EntrySeqType metricsObjects;
            for (const auto& map : _maps)
            {
                typename ObserverImplType::EntryPtrType entry = map->getMatching(helper);
                if (entry)
                {
                    metricsObjects.push_back(entry);
                }
            }

            if (metricsObjects.empty())
            {
                return nullptr;
            }

            ObserverImplPtrType obsv = std::make_shared<ObserverImplType>();
            obsv->init(helper, metricsObjects);
            return obsv;
        }

        template<typename ObserverPtrType>
        ObserverImplPtrType getObserver(const MetricsHelperT<MetricsType>& helper, const ObserverPtrType& observer)
        {
            ObserverImplPtrType old = std::dynamic_pointer_cast<ObserverImplType>(observer);

            if (!observer || !old)
            {
                return getObserver(helper);
            }

            std::lock_guard lock(_mutex);
            if (!_metrics)
            {
                return nullptr;
            }

            typename ObserverImplType::EntrySeqType metricsObjects;
            for (const auto& map : _maps)
            {
                typename ObserverImplType::EntryPtrType entry = map->getMatching(helper, old->getEntry(map.get()));
                if (entry)
                {
                    metricsObjects.push_back(entry);
                }
            }
            if (metricsObjects.empty())
            {
                old->detach();
                return nullptr;
            }

            ObserverImplPtrType obsv = std::make_shared<ObserverImplType>();
            obsv->init(helper, metricsObjects, old.get());
            return obsv;
        }

        template<typename SubMapMetricsType>
        void registerSubMap(const std::string& subMap, MetricsMap MetricsType::* member)
        {
            assert(_metrics);
            _metrics->registerSubMap<SubMapMetricsType>(_name, subMap, member);
        }

        [[nodiscard]] bool isEnabled() const { return _enabled != 0; }

        void update() override
        {
            UpdaterPtr updater;
            {
                std::lock_guard lock(_mutex);
                if (!_metrics)
                {
                    return;
                }

                std::vector<IceInternal::MetricsMapIPtr> maps = _metrics->getMaps(_name);
                _maps.clear();
                for (auto& map : maps)
                {
                    _maps.push_back(std::dynamic_pointer_cast<IceInternal::MetricsMapT<MetricsType>>(map));
                    assert(_maps.back());
                }
                _enabled.exchange(_maps.empty() ? 0 : 1);
                updater = _updater;
            }

            if (updater)
            {
                updater->update();
            }
        }

        void setUpdater(const UpdaterPtr& updater)
        {
            std::lock_guard lock(_mutex);
            _updater = updater;
        }

        void destroy()
        {
            std::lock_guard lock(_mutex);
            _metrics = nullptr;
            _maps.clear();
        }

    private:
        IceInternal::MetricsAdminIPtr _metrics;
        const std::string _name;
        MetricsMapSeqType _maps;
        std::atomic<bool> _enabled;
        UpdaterPtr _updater;
        std::mutex _mutex;
    };

    using ObserverI = ObserverT<Metrics>;
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

#endif
