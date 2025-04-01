// Copyright (c) ZeroC, Inc.

#ifndef ICE_METRICS_FUNCTIONAL_H
#define ICE_METRICS_FUNCTIONAL_H

namespace IceInternal
{
    template<typename R> struct ReferenceWrapper
    {
        static R* get(const R& v) { return const_cast<R*>(&v); }
    };

    template<typename R> struct ReferenceWrapper<std::shared_ptr<R>>
    {
        static R* get(const std::shared_ptr<R>& v) { return v.get(); }
    };

    template<typename R> struct ReferenceWrapper<const std::shared_ptr<R>&>
    {
        static R* get(const std::shared_ptr<R>& v) { return v.get(); }
    };

    template<typename R> struct ReferenceWrapper<const std::optional<R>&>
    {
        static R* get(const std::optional<R>& v) { return v ? const_cast<R*>(&v.value()) : nullptr; }
    };

    template<typename R> struct ReferenceWrapper<R*>
    {
        static R* get(R* v) { return v; }
    };

    template<typename R> struct ReferenceWrapper<const R&>
    {
        static R* get(const R& v) { return const_cast<R*>(&v); }
    };

};

namespace IceMX
{
    template<class T, typename Y, typename Func> struct ApplyOnMember
    {
        ApplyOnMember(Y T::* m, Func f) : func(f), member(m) {}

        template<typename R> void operator()(const R& v) { func(IceInternal::ReferenceWrapper<R>::get(v)->*member); }

        Func func;
        Y T::* member;
    };

    template<typename T> struct Decrement
    {
        void operator()(T& v) { --v; }
    };

    template<typename T> struct Increment
    {
        void operator()(T& v) { ++v; }
    };

    template<typename T> struct Add
    {
        Add(T v) : value(v) {}

        template<typename Y> void operator()(Y& v) { v += value; }

        T value;
    };

    template<class T, typename Y, typename F> ApplyOnMember<T, Y, F> applyOnMember(Y T::* member, F func)
    {
        return ApplyOnMember<T, Y, F>(member, func);
    }

    template<class T, typename Y, typename V> ApplyOnMember<T, Y, Add<V>> add(Y T::* member, V value)
    {
        return applyOnMember(member, Add<V>(value));
    }

    template<class T, typename Y> ApplyOnMember<T, Y, Increment<Y>> inc(Y T::* member)
    {
        return applyOnMember(member, Increment<Y>());
    }

    template<class T, typename Y> ApplyOnMember<T, Y, Decrement<Y>> dec(Y T::* member)
    {
        return applyOnMember(member, Decrement<Y>());
    }
}

#endif
