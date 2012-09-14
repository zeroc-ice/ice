// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_METRICSOBSERVER_I_H
#define ICE_METRICSOBSERVER_I_H

#include <IceUtil/StopWatch.h>

#include <Ice/Instrumentation.h>
#include <Ice/Metrics.h>
#include <Ice/Endpoint.h>
#include <Ice/Connection.h>
#include <Ice/MetricsAdminI.h>
#include <Ice/MetricsFunctional.h>

#include <stdexcept>

namespace IceMX
{

class MetricsHelper
{
public:

    virtual std::string operator()(const std::string&) const = 0;
};

class Updater : public IceUtil::Shared
{
public:

    virtual void update() = 0;
};
typedef IceUtil::Handle<Updater> UpdaterPtr;

template<typename T> class MetricsHelperT : public MetricsHelper
{
public:

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
            _attributes.insert(make_pair(name, new HelperMemberResolver<Y>(name, member)));
        }

        template<typename Y> void
        add(const std::string& name, Y (Helper::*memberFn)() const)
        {
            _attributes.insert(make_pair(name, new HelperMemberFunctionResolver<Y>(name, memberFn)));
        }

        template<typename I, typename O, typename Y> void
        add(const std::string& name, O (Helper::*getFn)() const, Y I::*member)
        {
            _attributes.insert(make_pair(name, new MemberResolver<I, O, Y>(name, getFn, member)));
        }

        template<typename I, typename O, typename Y> void
        add(const std::string& name, O (Helper::*getFn)() const, Y (I::*memberFn)() const)
        {
            _attributes.insert(make_pair(name, new MemberFunctionResolver<I, O, Y>(name, getFn, memberFn)));
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
                I* v = dynamic_cast<I*>(ReferenceWrapper<O>::get(o));
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
                I* v = dynamic_cast<I*>(ReferenceWrapper<O>::get(o));
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
        toString(const Ice::EndpointPtr& e)
        {
            return e->toString();
        }

        static std::string
        toString(const Ice::ConnectionPtr& e)
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

class ObserverI;

template<typename T> class ObserverT : virtual public Ice::Instrumentation::Observer
{
public:

    typedef T MetricsType;
    typedef typename MetricsMapT<MetricsType>::EntryTPtr EntryPtrType;
    typedef std::vector<EntryPtrType> EntrySeqType;

    ObserverT()
    {
    }
    
    virtual void 
    attach()
    {
        _watch.start();
    }

    virtual void 
    detach()
    {
        Ice::Long lifetime = _watch.stop();
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
    init(const MetricsHelperT<MetricsType>& helper, EntrySeqType& objects)
    {
        assert(_objects.empty());
        _objects.swap(objects);
        std::sort(_objects.begin(), _objects.end());
        for(typename EntrySeqType::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
        {
            (*p)->attach(helper);
        }
    }

    void
    update(const MetricsHelperT<MetricsType>& helper, EntrySeqType& objects)
    {
        std::sort(objects.begin(), objects.end());
        typename EntrySeqType::const_iterator p = objects.begin();
        typename EntrySeqType::iterator q = _objects.begin();
        while(p != objects.end())
        {
            if(q == _objects.end() || *p < *q) // New metrics object
            {
                q = _objects.insert(q, *p);
                (*p)->attach(helper);
                ++p;
                ++q;
            }
            else if(*p == *q) // Same metrics object
            {
                ++p;
                ++q;
            }
            else // Removed metrics object
            {
                (*q)->detach(_watch.delay());
                q = _objects.erase(q);
            }
        }
        if(q != _objects.end())
        {
            for(p = q; p != _objects.end(); ++p)
            {
                (*q)->detach(_watch.delay());
            }
            _objects.erase(q, _objects.end());
        }
    }

    template<typename ObserverImpl, typename ObserverMetricsType> IceInternal::Handle<ObserverImpl>
    getObserver(const std::string& mapName, const MetricsHelperT<ObserverMetricsType>& helper)
    {
        std::vector<typename MetricsMapT<ObserverMetricsType>::EntryTPtr> metricsObjects;
        for(typename EntrySeqType::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
        {
            typename MetricsMapT<ObserverMetricsType>::EntryTPtr e = (*p)->getMatching(mapName, helper);
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
};

class ObserverI : virtual public Ice::Instrumentation::Observer, public ObserverT<Metrics>
{
};

template<typename T> Updater*
newUpdater(const IceUtil::Handle<T>& updater, void (T::*fn)())
{
    return new UpdaterT<T>(updater.get(), fn);
}

template<typename T> Updater*
newUpdater(const IceInternal::Handle<T>& updater, void (T::*fn)())
{
    return new UpdaterT<T>(updater.get(), fn);
}

template<typename ObserverImplType> 
class ObserverFactoryT : public Updater, private IceUtil::Mutex
{
public:

    typedef IceUtil::Handle<ObserverImplType> ObserverImplPtrType;
    typedef typename ObserverImplType::MetricsType MetricsType;

    typedef std::vector<IceUtil::Handle<MetricsMapT<MetricsType> > > MetricsMapSeqType;

    ObserverFactoryT(const MetricsAdminIPtr& metrics, const std::string& name) : _metrics(metrics), _name(name)
    {
        _metrics->registerMap<MetricsType>(name, this);
    }

    ObserverFactoryT(const std::string& name) : _name(name)
    {
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

        std::sort(metricsObjects.begin(), metricsObjects.end());

        ObserverImplPtrType obsv = new ObserverImplType();
        obsv->init(helper, metricsObjects);
        return obsv;
    }

    template<typename ObserverPtrType> ObserverImplPtrType
    getObserver(const MetricsHelperT<MetricsType>& helper, const ObserverPtrType& observer)
    {
        IceUtil::Mutex::Lock sync(*this);

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

        std::sort(metricsObjects.begin(), metricsObjects.end());

        ObserverImplPtrType obsv = ObserverImplPtrType::dynamicCast(observer);
        if(!obsv)
        {
            obsv = new ObserverImplType();
            obsv->init(helper, metricsObjects);
        }
        else
        {
            obsv->update(helper, metricsObjects);
        }
        return obsv;
    }

    template<typename SubMapMetricsType> void 
    registerSubMap(const std::string& subMap, MetricsMap MetricsType::* member)
    {
        _metrics->registerSubMap<SubMapMetricsType>(_name, subMap, member);
    }

    bool isEnabled() const
    {
        return _enabled;
    }

    virtual void update()
    {
        UpdaterPtr updater;
        {
            IceUtil::Mutex::Lock sync(*this);
            std::vector<MetricsMapIPtr> maps = _metrics->getMaps(_name);
            _maps.clear();
            for(std::vector<MetricsMapIPtr>::const_iterator p = maps.begin(); p != maps.end(); ++p)
            {
                _maps.push_back(IceUtil::Handle<MetricsMapT<MetricsType> >::dynamicCast(*p));
                assert(_maps.back());
            }
            _enabled = !_maps.empty();
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

private:

    const MetricsAdminIPtr _metrics;
    const std::string _name;
    MetricsMapSeqType _maps;
    volatile bool _enabled;
    UpdaterPtr _updater;
};

}

#endif
