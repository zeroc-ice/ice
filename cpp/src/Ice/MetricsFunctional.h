// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_METRICSFUNCTIONAL_H
#define ICE_METRICSFUNCTIONAL_H

namespace IceMX
{

template<typename R> struct ReferenceWrapper
{
    static R* get(const R& v)
    {
        return const_cast<R*>(&v);
    }
};
    
template<typename R> struct ReferenceWrapper<IceUtil::Handle<R> >
{
    static R* get(const IceUtil::Handle<R>& v)
    {
        return v.get();
    }
};

template<typename R> struct ReferenceWrapper<const IceUtil::Handle<R>& >
{
    static R* get(const IceUtil::Handle<R>& v)
    {
        return v.get();
    }
};

template<typename R> struct ReferenceWrapper<IceInternal::ProxyHandle<R> >
{
    static R* get(const IceInternal::ProxyHandle<R>& v)
    {
        return v.get();
    }
};

template<typename R> struct ReferenceWrapper<const IceInternal::ProxyHandle<R>& >
{
    static R* get(const IceInternal::ProxyHandle<R>& v)
    {
        return v.get();
    }
};
    
template<typename R> struct ReferenceWrapper<IceInternal::Handle<R> >
{
    static R* get(const IceInternal::Handle<R>& v)
    {
        return v.get();
    }
};

template<typename R> struct ReferenceWrapper<const IceInternal::Handle<R>& >
{
    static R* get(const IceInternal::Handle<R>& v)
    {
        return v.get();
    }
};
    
template<typename R> struct ReferenceWrapper<R*>
{
    static R* get(R* v)
    {
        return v;
    }
};

template<typename R> struct ReferenceWrapper<const R&>
{
    static R* get(const R& v)
    {
        return const_cast<R*>(&v);
    }
};

template<class T, typename Y, typename Func> struct ApplyOnMember
{
    ApplyOnMember(Y T::*member, Func func) : func(func), member(member)
    {
    }

    template<typename R>
    void operator()(const R& v)
    {
        func(ReferenceWrapper<R>::get(v)->*member);
    }

    Func func;
    Y T::*member;
};

template<typename T> struct Decrement
{
    void operator()(T& v)
    {
        --v;
    }
};

template<typename T> struct Increment
{
    void operator()(T& v)
    {
        ++v;
    }
};

template<typename T> struct Add
{
    Add(T value) : value(value) { }

    template<typename Y>
    void operator()(Y& v)
    {
        v += value;
    }

    T value;
};

template<typename Func1, typename Func2> struct Chain
{
    Chain(Func1 f1, Func2 f2) : func1(f1), func2(f2)
    {
    }

    template<typename T>
    void operator()(const T& v)
    {
        func1(v);
        func2(v);
    }

    Func1 func1;
    Func2 func2;
};

template<typename Func1, typename Func2> Chain<Func1, Func2> chain(Func1 f1, Func2 f2)
{
    return Chain<Func1, Func2>(f1, f2);
}  

template<class T, typename Y, typename F> ApplyOnMember<T, Y, F> applyOnMember(Y T::*member, F func)
{
    return ApplyOnMember<T, Y, F>(member, func);
}

template<class T, typename Y, typename V> ApplyOnMember<T, Y, Add<V> > add(Y T::*member, V value)
{
    return applyOnMember(member, Add<V>(value));
}

template<class T, typename Y> ApplyOnMember<T, Y, Increment<Y> > inc(Y T::*member)
{
    return applyOnMember(member, Increment<Y>());
}

template<class T, typename Y> ApplyOnMember<T, Y, Decrement<Y> > dec(Y T::*member)
{
    return applyOnMember(member, Decrement<Y>());
}

}

#endif
