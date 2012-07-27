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

#include <Ice/Observer.h>
#include <Ice/Metrics.h>

#include <Ice/MetricsAdminI.h>
#include <Ice/MetricsFunctional.h>

namespace IceMX
{

typedef std::vector<std::pair<IceMX::MetricsObjectPtr, IceUtil::Mutex*> > MetricsObjectAndLockSeq;

class ObjectHelper
{
public:

    virtual std::string operator()(const std::string&) const = 0;

    virtual MetricsObjectPtr newMetricsObject() const = 0;
};

class ObjectObserverUpdater : public IceUtil::Shared
{
public:

    virtual void update() = 0;
};
typedef IceUtil::Handle<ObjectObserverUpdater> ObjectObserverUpdaterPtr;

template<class ObjectMetricsType> class ObjectObserverT : virtual public ::Ice::ObjectObserver
{
public:

    typedef ObjectMetricsType Type;
    typedef IceInternal::Handle<ObjectMetricsType> PtrType;
    typedef std::vector<std::pair<PtrType, IceUtil::Mutex*> > SeqType;

    ObjectObserverT()
    {
    }

    virtual void 
    attach()
    {
    }

    virtual void 
    detach()
    {
        forEach(dec(&MetricsObject::current));
    }

    template<typename Function> void
    forEach(Function func)
    {
        for(typename SeqType::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
        {
            IceUtil::Mutex::Lock sync(*p->second);
            func(p->first);
        }
    }
    
    template<typename ObjectHelper> void
    update(const ObjectHelper& helper, const MetricsObjectAndLockSeq& objects)
    {
        typename MetricsObjectAndLockSeq::const_iterator p = objects.begin();
        typename SeqType::iterator q = _objects.begin();
        while(p != objects.end())
        {
            if(q == _objects.end() || p->first.get() < static_cast<MetricsObject*>(q->first.get())) // New metrics object
            {
                IceUtil::Mutex::Lock sync(*p->second);
                ++p->first->total;
                ++p->first->current;
                PtrType v = PtrType::dynamicCast(p->first);
                helper.initMetricsObject(v);
                q = _objects.insert(q, std::make_pair(v, p->second));
                ++p;
                ++q;
            }
            else if(p->first.get() == static_cast<MetricsObject*>(q->first.get())) // Same metrics object
            {
                ++p;
                ++q;
            }
            else // Removed metrics object
            {
                q = _objects.erase(q);
            }
        }
    }
    
private:

    SeqType _objects;
};

template<typename T> class ObjectHelperT : public ObjectHelper
{
public:

    virtual MetricsObjectPtr newMetricsObject() const
    {
        return new T();
    }

    virtual void initMetricsObject(const IceInternal::Handle<T>&) const
    {
        // To be overriden in specialization to initialize state attributes
    }

protected:

    template<typename Helper> class AttributeResolverT
    {
        class Resolver
        {
        public:
            virtual ~Resolver() { }
            virtual std::string operator()(const Helper* h) const = 0;
        };

    public:

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
                return "unknown";
            }
            return (*p->second)(helper);
        }

        template<typename Y> void
        add(const std::string& name, Y Helper::*member)
        {
            _attributes.insert(make_pair(name,  new HelperMemberResolver<Y>(member)));
        }

        template<typename Y> void
        add(const std::string& name, Y (Helper::*memberFn)() const)
        {
            _attributes.insert(make_pair(name,  new HelperMemberFunctionResolver<Y>(memberFn)));
        }

        template<typename I, typename O, typename Y> void
        add(const std::string& name, O (Helper::*getFn)() const, Y I::*member)
        {
            _attributes.insert(make_pair(name,  new MemberResolver<I, O, Y>(getFn, member)));
        }

        template<typename I, typename O, typename Y> void
        add(const std::string& name, O (Helper::*getFn)() const, Y (I::*memberFn)() const)
        {
            _attributes.insert(make_pair(name,  new MemberFunctionResolver<I, O, Y>(getFn, memberFn)));
        }

    private:
            
        template<typename Y> class HelperMemberResolver : public Resolver
        {
        public:

            HelperMemberResolver(Y Helper::*member) : _member(member)
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

            HelperMemberFunctionResolver(Y (Helper::*memberFn)() const) : _memberFn(memberFn)
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

            MemberResolver(O (Helper::*getFn)() const, Y I::*member) : _getFn(getFn), _member(member)
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
                    return "unknown";
                }
            }

        private:

            O (Helper::*_getFn)() const;
            Y I::*_member;
        };

        template<typename I, typename O, typename Y> class MemberFunctionResolver : public Resolver
        {
        public:

            MemberFunctionResolver(O (Helper::*getFn)() const, Y (I::*memberFn)() const) :
                _getFn(getFn), _memberFn(memberFn)
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
                    return "unknown";
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

        static std::string
        toString(const std::string& s)
        {
            return s;
        }

        static std::string
        toString(bool v)
        {
            return v ? "true" : "false";
        }

        std::map<std::string, Resolver*> _attributes;
    };

};

template<typename T> class ObjectObserverUpdaterT : public ObjectObserverUpdater
{
public:
    
    ObjectObserverUpdaterT(T* updater, void (T::*fn)()) : 
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

template<typename T> ObjectObserverUpdater*
newUpdater(const IceUtil::Handle<T>& updater, void (T::*fn)())
{
    return new ObjectObserverUpdaterT<T>(updater.get(), fn);
}

template<typename T> ObjectObserverUpdater*
newUpdater(const IceInternal::Handle<T>& updater, void (T::*fn)())
{
    return new ObjectObserverUpdaterT<T>(updater.get(), fn);
}

template<typename ObserverImplType> 
class ObjectObserverResolverT
{
public:

    typedef IceUtil::Handle<ObserverImplType> ObserverImplPtrType;

    ObjectObserverResolverT(const std::string& name, const MetricsAdminIPtr& metrics) :
        _name(name), _metrics(metrics)
    {
    }

    template<typename ObjectHelper> ObserverImplPtrType
    getObserver(const ObjectHelper& helper)
    {
        MetricsObjectAndLockSeq metricsObjects = _metrics->getMatching(_name, helper);
        if(metricsObjects.empty())
        {
            return 0;
        }

        ObserverImplPtrType obsv = new ObserverImplType();
        obsv->update(helper, metricsObjects);
        return obsv;
    }

    template<typename ObjectHelper, typename ObserverPtrType> ObserverImplPtrType
    getObserver(const ObjectHelper& helper, const ObserverPtrType& observer)
    {
        MetricsObjectAndLockSeq metricsObjects = _metrics->getMatching(_name, helper);
        if(metricsObjects.empty())
        {
            return 0;
        }

        ObserverImplPtrType obsv = ObserverImplPtrType::dynamicCast(observer);
        if(!obsv)
        {
            obsv = new ObserverImplType();
        }
        obsv->update(helper, metricsObjects);
        return obsv;
    }

private:

    const std::string _name;
    const MetricsAdminIPtr _metrics;
};

}

#endif
