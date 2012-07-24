// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_METRICSSTATS_I_H
#define ICE_METRICSSTATS_I_H

#include <Ice/Observer.h>
#include <Ice/Metrics.h>

#include <Ice/MetricsFunctional.h>

namespace IceMX
{

class MetricsAdminI;
typedef IceUtil::Handle<MetricsAdminI> MetricsAdminIPtr;

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

template<class T> class ObjectObserverT : virtual public ::Ice::ObjectObserver
{
public:

    typedef T Type;
    typedef IceUtil::Handle<T> PtrType;
    typedef std::vector<std::pair<PtrType, IceUtil::Mutex*> > SeqType;


    ObjectObserverT(const MetricsObjectAndLockSeq& objects)
    {
        for(MetricsObjectAndLockSeq::const_iterator p = objects.begin(); p != objects.end(); ++p)
        {
            _objects.push_back(std::make_pair(PtrType::dynamicCast(p->first), p->second));
        }
    }

    virtual void 
    attach()
    {
        for(typename SeqType::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
        {
            IceUtil::Mutex::Lock sync(*p->second);
            ++p->first->total;
            ++p->first->current;
        }
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

    void
    update(ObjectObserverT* old)
    {
        typename SeqType::const_iterator p = _objects.begin();
        typename SeqType::const_iterator q = old->_objects.begin();
        while(p != _objects.end())
        {
            if(q == old->_objects.end() || p->first < q->first) // New metrics object
            {
                IceUtil::Mutex::Lock sync(*p->second);
                ++p->first->total;
                ++p->first->current;
                ++p;
            }
            else if(p->first == q->first) // Same metrics object
            {
                ++p;
                ++q;
            }
            else // Removed metrics object
            {
                ++q;
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
};

template<typename Helper> class ObjectAttributeResolverT
{
    class Resolver
    {
    public:
        virtual ~Resolver() { }
        virtual std::string operator()(const Helper* h) const = 0;
    };

public:

    ~ObjectAttributeResolverT()
    {
        for(typename std::map<std::string, Resolver*>::iterator p = _attributes.begin(); p != _attributes.end(); ++p)
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

protected:

    template<class T, typename O, typename Y> void
    add(const std::string& name, O* (Helper::*getFn)() const, Y T::*member)
    {
        _attributes.insert(make_pair(name,  new MemberResolver<T, O, Y>(getFn, member)));
    }

    template<class T, typename O, typename Y> void
    add(const std::string& name, O* (Helper::*getFn)() const, Y (T::*memberFn)() const)
    {
        _attributes.insert(make_pair(name,  new MemberFunctionResolver<T, O, Y>(getFn, memberFn)));
    }

private:

    template<class T, typename O, typename Y> class MemberResolver : public Resolver
    {
    public:

        MemberResolver(O* (Helper::*getFn)() const, Y T::*member) : _getFn(getFn), _member(member)
        {
        }

        virtual std::string operator()(const Helper* r) const
        {
            O* o = (r->*_getFn)();
            T* v = dynamic_cast<T*>(o);
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
        
        O* (Helper::*_getFn)() const;
        Y T::*_member;
    };

    template<class T, typename O, typename Y> class MemberFunctionResolver : public Resolver
    {
    public:

        MemberFunctionResolver(O* (Helper::*getFn)() const, Y (T::*memberFn)() const) :
            _getFn(getFn), _memberFn(memberFn)
        {
        }

        virtual std::string operator()(const Helper* r) const
        {
            O* o = (r->*_getFn)();
            T* v = dynamic_cast<T*>(o);
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
        
        O* (Helper::*_getFn)() const;
        Y (T::*_memberFn)() const;
    };

    template<typename T> static std::string
    toString(const T& v)
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

    std::map<std::string, Resolver*> _attributes;
};

class ObjectObserverResolver
{
public:

    virtual void clear() = 0;
};

template<typename T> class ObjectObserverUpdaterT : public ObjectObserverUpdater
{
public:
    
    ObjectObserverUpdaterT(T* updater, void (T::*fn)(), ObjectObserverResolver& resolver) : 
        _updater(updater), _fn(fn), _resolver(resolver)
    {
    }
        
    virtual void update()
    {
        _resolver.clear();
        (_updater.get()->*_fn)();
    }
    
private: 
    
    const IceUtil::Handle<T> _updater;
    void (T::*_fn)();
    ObjectObserverResolver& _resolver;
};

template<typename T> ObjectObserverUpdater*
newUpdater(const IceUtil::Handle<T>& updater, void (T::*fn)(), ObjectObserverResolver& resolver)
{
    return new ObjectObserverUpdaterT<T>(updater.get(), fn, resolver);
}

template<typename T> ObjectObserverUpdater*
newUpdater(const IceInternal::Handle<T>& updater, void (T::*fn)(), ObjectObserverResolver& resolver)
{
    return new ObjectObserverUpdaterT<T>(updater.get(), fn, resolver);
}

template<typename T> 
class ObjectObserverResolverT : public ObjectObserverResolver, public IceUtil::Mutex
{
    struct Compare
    {
        //
        // Only sort on the metrics object pointer value.
        //
        bool operator()(const MetricsObjectAndLockSeq& lhs, const MetricsObjectAndLockSeq& rhs)
        { 
            if(lhs.size() < rhs.size())
            {
                return true;
            } 
            else if(lhs.size() > rhs.size())
            {
                return false;
            }

            typename MetricsObjectAndLockSeq::const_iterator p = lhs.begin();
            typename MetricsObjectAndLockSeq::const_iterator q = rhs.begin();
            while(p != lhs.end())
            {
                if(p->first.get() < q->first.get())
                {
                    return true;
                }
                else if(p->first.get() > q->first.get())
                {
                    return false;
                }
            }
            return false;
        }
    };

public:

    typedef IceUtil::Handle<T> TPtr;
    typedef std::map<MetricsObjectAndLockSeq, TPtr, Compare> ObserverMap;

    template<typename S> T*
    getObserver(const MetricsObjectAndLockSeq& objects, S* oldObserver)
    {
        if(objects.empty())
        {
            return 0;
        }

        IceUtil::Mutex::Lock sync(*this);
        typename ObserverMap::const_iterator p = _observers.find(objects);
        if(p == _observers.end())
        {
            p = _observers.insert(make_pair(objects, new T(objects))).first;
        }
        
        T* newObserver = p->second.get();
        if(oldObserver)
        {
            newObserver->update(dynamic_cast<T*>(oldObserver));
        }
        return newObserver;
    }


    virtual void
    clear()
    {
        IceUtil::Mutex::Lock sync(*this);
        _observers.clear();
    }

private:

    ObserverMap _observers;
};

class ConnectionObserverI : public ::Ice::ConnectionObserver, public ObjectObserverT<Ice::ConnectionMetricsObject>
{
public:

    ConnectionObserverI(const MetricsObjectAndLockSeq& objects) : ObjectObserverT(objects)
    {
    }

    virtual void attach();
    virtual void detach();

    virtual void stateChanged(::Ice::ObserverConnectionState, ::Ice::ObserverConnectionState);
    virtual void sentBytes(::Ice::Int, ::Ice::Long);
    virtual void receivedBytes(::Ice::Int, ::Ice::Long);
};

class ObserverResolverI : public ::Ice::ObserverResolver
{
public:

    ObserverResolverI(const MetricsAdminIPtr&);

    virtual void setObserverUpdater(const ::Ice::ObserverUpdaterPtr&);

    virtual ::Ice::ConnectionObserverPtr 
    getConnectionObserver(const ::Ice::ConnectionPtr&, const ::Ice::ConnectionObserverPtr&);

    virtual ::Ice::ObjectObserverPtr 
    getThreadObserver(const std::string&, const std::string&, const ::Ice::ObjectObserverPtr&);

    virtual ::Ice::ThreadPoolThreadObserverPtr 
    getThreadPoolThreadObserver(const std::string&, const std::string&, const ::Ice::ThreadPoolThreadObserverPtr&);

    virtual ::Ice::RequestObserverPtr getInvocationObserver(const ::Ice::ObjectPrx&, const std::string&);

    virtual ::Ice::RequestObserverPtr getDispatchObserver(const ::Ice::ObjectPtr&, const ::Ice::Current&);

private:

    const MetricsAdminIPtr _metrics;

    ObjectObserverResolverT<ConnectionObserverI> _connections;
};

}

#endif
