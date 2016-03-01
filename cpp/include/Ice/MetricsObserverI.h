// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEMX_METRICSOBSERVER_I_H
#define ICEMX_METRICSOBSERVER_I_H

#include <IceUtil/StopWatch.h>
#include <IceUtil/Atomic.h>

#include <Ice/Instrumentation.h>
#include <Ice/Endpoint.h>
#include <Ice/Connection.h>

#include <Ice/Metrics.h>
#include <Ice/MetricsAdminI.h>
#include <Ice/MetricsFunctional.h>

#include <stdexcept>

namespace IceMX
{

template<typename T> class MetricsHelperT
{
public:

    virtual ~MetricsHelperT()
    {
    }

    virtual std::string operator()(const std::string&) const = 0;

    virtual void initMetrics(const IceInternal::Handle<T>&) const
    {
        // To be overriden in specialization to initialize state attributes
    }

protected:

    template<typename Helper> class AttributeResolverT
    {
        class Resolver
        {
        public:

            Resolver(const std::string& name) : _name(name)
            {
            }

            virtual ~Resolver()
            {
            }

            virtual std::string operator()(const Helper* h) const = 0;

        protected:

            std::string _name;
        };

    public:

        AttributeResolverT() : _default(0)
        {
        }

        ~AttributeResolverT()
        {
            for(typename std::map<std::string, Resolver*>::iterator p = _attributes.begin(); p != _attributes.end();++p)
            {
                delete p->second;
            }
        }

        std::string operator()(const Helper* helper, const std::string& attribute) const
        {
            typename std::map<std::string, Resolver*>::const_iterator p = _attributes.find(attribute);
            if(p == _attributes.end())
            {
                if(attribute == "none")
                {
                    return "";
                }
                if(_default)
                {
                    return (helper->*_default)(attribute);
                }
                throw std::invalid_argument(attribute);
            }
            return (*p->second)(helper);
        }

        void
        setDefault(std::string (Helper::*memberFn)(const std::string&) const)
        {
            _default = memberFn;
        }

        template<typename Y> void
        add(const std::string& name, Y Helper::*member)
        {
            _attributes.insert(typename std::map<std::string,
                               Resolver*>::value_type(name, new HelperMemberResolver<Y>(name, member)));
        }

        template<typename Y> void
        add(const std::string& name, Y (Helper::*memberFn)() const)
        {
            _attributes.insert(typename std::map<std::string,
                               Resolver*>::value_type(name, new HelperMemberFunctionResolver<Y>(name, memberFn)));
        }

        template<typename I, typename O, typename Y> void
        add(const std::string& name, O (Helper::*getFn)() const, Y I::*member)
        {
            _attributes.insert(typename std::map<std::string,
                               Resolver*>::value_type(name, new MemberResolver<I, O, Y>(name, getFn, member)));
        }

        template<typename I, typename O, typename Y> void
        add(const std::string& name, O (Helper::*getFn)() const, Y (I::*memberFn)() const)
        {
            _attributes.insert(typename std::map<std::string,
                               Resolver*>::value_type(name, new MemberFunctionResolver<I, O, Y>(name, getFn,
                                                                                                memberFn)));
        }

    private:

        template<typename Y> class HelperMemberResolver : public Resolver
        {
        public:

            HelperMemberResolver(const std::string& name, Y Helper::*member) : Resolver(name), _member(member)
            {
            }

            virtual std::string operator()(const Helper* r) const
            {
                return toString(r->*_member);
            }

        private:

            Y Helper::*_member;
        };

        template<typename Y> class HelperMemberFunctionResolver : public Resolver
        {
        public:

            HelperMemberFunctionResolver(const std::string& name, Y (Helper::*memberFn)() const) :
                Resolver(name), _memberFn(memberFn)
            {
            }

            virtual std::string operator()(const Helper* r) const
            {
                return toString((r->*_memberFn)());
            }

        private:

            Y (Helper::*_memberFn)() const;
        };


        template<typename I, typename O, typename Y> class MemberResolver : public Resolver
        {
        public:

            MemberResolver(const std::string& name, O (Helper::*getFn)() const, Y I::*member) :
                Resolver(name), _getFn(getFn), _member(member)
            {
            }

            virtual std::string operator()(const Helper* r) const
            {
                O o = (r->*_getFn)();
                I* v = dynamic_cast<I*>(IceInternal::ReferenceWrapper<O>::get(o));
                if(v)
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
            Y I::*_member;
        };

        template<typename I, typename O, typename Y> class MemberFunctionResolver : public Resolver
        {
        public:

            MemberFunctionResolver(const std::string& name, O (Helper::*getFn)() const, Y (I::*memberFn)() const) :
                Resolver(name), _getFn(getFn), _memberFn(memberFn)
            {
            }

            virtual std::string operator()(const Helper* r) const
            {
                O o = (r->*_getFn)();
                I* v = dynamic_cast<I*>(IceInternal::ReferenceWrapper<O>::get(o));
                if(v)
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

        template<typename I> static std::string
        toString(const I& v)
        {
            std::ostringstream os;
            os << v;
            return os.str();
        }

        static const std::string&
        toString(const std::string& s)
        {
            return s;
        }

        static std::string
        toString(const ::Ice::EndpointPtr& e)
        {
            return e->toString();
        }

        static std::string
        toString(const ::Ice::ConnectionPtr& e)
        {
            return e->toString();
        }

        static std::string
        toString(bool v)
        {
            return v ? "true" : "false";
        }

        std::map<std::string, Resolver*> _attributes;
        std::string (Helper::*_default)(const std::string&) const;
    };
};

class Updater : public IceUtil::Shared
{
public:

    virtual void update() = 0;
};
typedef IceUtil::Handle<Updater> UpdaterPtr;

template<typename T> class UpdaterT : public Updater
{
public:

    UpdaterT(T* updater, void (T::*fn)()) :
        _updater(updater), _fn(fn)
    {
    }

    virtual void update()
    {
        (_updater.get()->*_fn)();
    }

private:

    const IceUtil::Handle<T> _updater;
    void (T::*_fn)();
};

template<typename T> Updater*
newUpdater(const IceInternal::Handle<T>& updater, void (T::*fn)())
{
    if(updater)
    {
        return new UpdaterT<T>(updater.get(), fn);
    }
    else
    {
        return 0;
    }
}

template<typename T> class ObserverT : virtual public ::Ice::Instrumentation::Observer
{
public:

    typedef T MetricsType;
    typedef typename IceInternal::MetricsMapT<MetricsType>::EntryTPtr EntryPtrType;
    typedef std::vector<EntryPtrType> EntrySeqType;

    ObserverT() : _previousDelay(0)
    {
    }

    virtual void
    attach()
    {
        if(!_watch.isStarted())
        {
            _watch.start();
        }
    }

    virtual void
    detach()
    {
        ::Ice::Long lifetime = _previousDelay + _watch.stop();
        for(typename EntrySeqType::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
        {
            (*p)->detach(lifetime);
        }
    }

    virtual void
    failed(const std::string& exceptionName)
    {
        for(typename EntrySeqType::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
        {
            (*p)->failed(exceptionName);
        }
    }

    template<typename Function> void
    forEach(const Function& func)
    {
        for(typename EntrySeqType::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
        {
            (*p)->execute(func);
        }
    }

    void
    init(const MetricsHelperT<MetricsType>& /*helper*/, EntrySeqType& objects, ObserverT* previous = 0)
    {
        _objects.swap(objects);

        if(previous == 0)
        {
            return;
        }

        _previousDelay = previous->_previousDelay + previous->_watch.delay();

        //
        // Detach entries from previous observer which are no longer
        // attached to this new observer.
        //
        for(typename EntrySeqType::const_iterator p = previous->_objects.begin(); p != previous->_objects.end(); ++p)
        {
            if(find(_objects.begin(), _objects.end(), *p) == _objects.end())
            {
                (*p)->detach(_previousDelay);
            }
        }
    }

    EntryPtrType
    getEntry(IceInternal::MetricsMapT<MetricsType>* map)
    {
        for(typename EntrySeqType::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
        {
            if((*p)->getMap() == map)
            {
                return *p;
            }
        }
        return 0;
    }

    template<typename ObserverImpl, typename ObserverMetricsType> IceInternal::Handle<ObserverImpl>
    getObserver(const std::string& mapName, const MetricsHelperT<ObserverMetricsType>& helper)
    {
        std::vector<typename IceInternal::MetricsMapT<ObserverMetricsType>::EntryTPtr> metricsObjects;
        for(typename EntrySeqType::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
        {
            typename IceInternal::MetricsMapT<ObserverMetricsType>::EntryTPtr e = (*p)->getMatching(mapName, helper);
            if(e)
            {
                metricsObjects.push_back(e);
            }
        }

        if(metricsObjects.empty())
        {
            return 0;
        }

        IceInternal::Handle<ObserverImpl> obsv = new ObserverImpl();
        obsv->init(helper, metricsObjects);
        return obsv;
    }

private:

    EntrySeqType _objects;
    IceUtilInternal::StopWatch _watch;
    IceUtil::Int64 _previousDelay;
};

template<typename ObserverImplType>
class ObserverFactoryT : public Updater, private IceUtil::Mutex
{
public:

    typedef IceUtil::Handle<ObserverImplType> ObserverImplPtrType;
    typedef typename ObserverImplType::MetricsType MetricsType;

    typedef std::vector<IceUtil::Handle<IceInternal::MetricsMapT<MetricsType> > > MetricsMapSeqType;

    ObserverFactoryT(const IceInternal::MetricsAdminIPtr& metrics, const std::string& name) :
        _metrics(metrics), _name(name), _enabled(0)
    {
        _metrics->registerMap<MetricsType>(name, this);
    }

    ~ObserverFactoryT()
    {
        if(_metrics)
        {
            _metrics->unregisterMap(_name);
        }
    }

    ObserverImplPtrType
    getObserver(const MetricsHelperT<MetricsType>& helper)
    {
        IceUtil::Mutex::Lock sync(*this);
        if(!_metrics)
        {
            return 0;
        }

        typename ObserverImplType::EntrySeqType metricsObjects;
        for(typename MetricsMapSeqType::const_iterator p = _maps.begin(); p != _maps.end(); ++p)
        {
            typename ObserverImplType::EntryPtrType entry = (*p)->getMatching(helper);
            if(entry)
            {
                metricsObjects.push_back(entry);
            }
        }

        if(metricsObjects.empty())
        {
            return 0;
        }

        ObserverImplPtrType obsv = new ObserverImplType();
        obsv->init(helper, metricsObjects);
        return obsv;
    }

    template<typename ObserverPtrType> ObserverImplPtrType
    getObserver(const MetricsHelperT<MetricsType>& helper, const ObserverPtrType& observer)
    {
        ObserverImplPtrType old = ObserverImplPtrType::dynamicCast(observer);
        if(!observer || !old)
        {
            return getObserver(helper);
        }

        IceUtil::Mutex::Lock sync(*this);
        if(!_metrics)
        {
            return 0;
        }

        typename ObserverImplType::EntrySeqType metricsObjects;
        for(typename MetricsMapSeqType::const_iterator p = _maps.begin(); p != _maps.end(); ++p)
        {
            typename ObserverImplType::EntryPtrType entry = (*p)->getMatching(helper, old->getEntry(p->get()));
            if(entry)
            {
                metricsObjects.push_back(entry);
            }
        }

        if(metricsObjects.empty())
        {
            old->detach();
            return 0;
        }

        ObserverImplPtrType obsv = new ObserverImplType();
        obsv->init(helper, metricsObjects, old.get());
        return obsv;
    }

    template<typename SubMapMetricsType> void
    registerSubMap(const std::string& subMap, MetricsMap MetricsType::* member)
    {
        assert(_metrics);
        _metrics->registerSubMap<SubMapMetricsType>(_name, subMap, member);
    }

    bool isEnabled() const
    {
        return _enabled != 0;
    }

    virtual void update()
    {
        UpdaterPtr updater;
        {
            IceUtil::Mutex::Lock sync(*this);
            if(!_metrics)
            {
                return;
            }

            std::vector<IceInternal::MetricsMapIPtr> maps = _metrics->getMaps(_name);
            _maps.clear();
            for(std::vector<IceInternal::MetricsMapIPtr>::const_iterator p = maps.begin(); p != maps.end(); ++p)
            {
                _maps.push_back(IceUtil::Handle<IceInternal::MetricsMapT<MetricsType> >::dynamicCast(*p));
                assert(_maps.back());
            }
            _enabled.exchange(_maps.empty() ? 0 : 1);
            updater = _updater;
        }

        if(updater)
        {
            updater->update();
        }
    }

    void setUpdater(const UpdaterPtr& updater)
    {
        IceUtil::Mutex::Lock sync(*this);
        _updater = updater;
    }

    void destroy()
    {
        IceUtil::Mutex::Lock sync(*this);
        _metrics = 0;
        _maps.clear();
    }

private:

    IceInternal::MetricsAdminIPtr _metrics;
    const std::string _name;
    MetricsMapSeqType _maps;
    //
    // TODO: Replace by std::atomic<bool> when it becomes widely
    // available.
    //
    IceUtilInternal::Atomic _enabled;
    UpdaterPtr _updater;
};

typedef ObserverT<Metrics> ObserverI;

}

#endif
