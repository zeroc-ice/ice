// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_FUNCTIONAL_H
#define ICE_FUNCTIONAL_H

#include <IceUtil/Functional.h>
#include <Ice/Handle.h>

// ----------------------------------------------------------------------
// Inline functions that return function objects that work with
// IceInternal::Handle
// ----------------------------------------------------------------------

namespace IceInternal
{

template<class R, class T>
inline ::IceUtilInternal::MemFun<R, T, Handle<T> >
memFun(R (T::*p)(void))
{
    return ::IceUtilInternal::MemFun<R, T, Handle<T> >(p);
}

template<class R, class T, class A>
inline ::IceUtilInternal::MemFun1<R, T, Handle<T>, A>
memFun1(R (T::*p)(A))
{
    return ::IceUtilInternal::MemFun1<R, T, Handle<T>, A>(p);
}

template<class T>
inline ::IceUtilInternal::VoidMemFun<T, Handle<T> >
voidMemFun(void (T::*p)(void))
{
    return ::IceUtilInternal::VoidMemFun<T, Handle<T> >(p);
}

template<class T, class A>
inline ::IceUtilInternal::VoidMemFun1<T, Handle<T>, A>
voidMemFun1(void (T::*p)(A))
{
    return ::IceUtilInternal::VoidMemFun1<T, Handle<T>, A>(p);
}

template<class R, class K, class T>
inline ::IceUtilInternal::SecondMemFun<R, K, T, Handle<T> >
secondMemFun(R (T::*p)(void))
{
    return ::IceUtilInternal::SecondMemFun<R, K, T, Handle<T> >(p);
}

template<class R, class K, class T, class A>
inline ::IceUtilInternal::SecondMemFun1<R, K, T, Handle<T>, A>
secondMemFun1(R (T::*p)(A))
{
    return ::IceUtilInternal::SecondMemFun1<R, K, T, Handle<T>, A>(p);
}

template<class K, class T>
inline ::IceUtilInternal::SecondVoidMemFun<K, T, Handle<T> >
secondVoidMemFun(void (T::*p)(void))
{
    return ::IceUtilInternal::SecondVoidMemFun<K, T, Handle<T> >(p);
}

template<class K, class T, class A>
inline ::IceUtilInternal::SecondVoidMemFun1<K, T, Handle<T>, A>
secondVoidMemFun1(void (T::*p)(A))
{
    return ::IceUtilInternal::SecondVoidMemFun1<K, T, Handle<T>, A>(p);
}

template<class R, class T>
inline ::IceUtilInternal::ConstMemFun<R, T, Handle<T> >
constMemFun(R (T::*p)(void) const)
{
    return ::IceUtilInternal::ConstMemFun<R, T, Handle<T> >(p);
}

template<class R, class T, class A>
inline ::IceUtilInternal::ConstMemFun1<R, T, Handle<T>, A>
constMemFun1(R (T::*p)(A))
{
    return ::IceUtilInternal::ConstMemFun1<R, T, Handle<T>, A>(p);
}

template<class T>
inline ::IceUtilInternal::ConstVoidMemFun<T, Handle<T> >
constVoidMemFun(void (T::*p)(void) const)
{
    return ::IceUtilInternal::ConstVoidMemFun<T, Handle<T> >(p);
}

template<class T, class A>
inline ::IceUtilInternal::ConstVoidMemFun1<T, Handle<T>, A>
constVoidMemFun1(void (T::*p)(A))
{
    return ::IceUtilInternal::ConstVoidMemFun1<T, Handle<T>, A>(p);
}

template<class R, class K, class T>
inline ::IceUtilInternal::SecondConstMemFun<R, K, T, Handle<T> >
secondConstMemFun(R (T::*p)(void) const)
{
    return ::IceUtilInternal::SecondConstMemFun<R, K, T, Handle<T> >(p);
}

template<class R, class K, class T, class A>
inline ::IceUtilInternal::SecondConstMemFun1<R, K, T, Handle<T>, A>
secondConstMemFun1(R (T::*p)(A))
{
    return ::IceUtilInternal::SecondConstMemFun1<R, K, T, Handle<T>, A>(p);
}

template<class K, class T>
inline ::IceUtilInternal::SecondConstVoidMemFun<K, T, Handle<T> >
secondConstVoidMemFun(void (T::*p)(void) const)
{
    return ::IceUtilInternal::SecondConstVoidMemFun<K, T, Handle<T> >(p);
}

template<class K, class T, class A>
inline ::IceUtilInternal::SecondConstVoidMemFun1<K, T, Handle<T>, A>
secondConstVoidMemFun1(void (T::*p)(A))
{
    return ::IceUtilInternal::SecondConstVoidMemFun1<K, T, Handle<T>, A>(p);
}

}

#endif
