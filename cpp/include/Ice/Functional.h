// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

namespace Ice
{

/// \cond INTERNAL
template<class R, class T>
inline ::IceUtilInternal::MemFun<R, T, ICE_INTERNAL_HANDLE<T> >
memFun(R (T::*p)(void))
{
    return ::IceUtilInternal::MemFun<R, T, ICE_INTERNAL_HANDLE<T> >(p);
}

template<class R, class T, class A>
inline ::IceUtilInternal::MemFun1<R, T, ICE_INTERNAL_HANDLE<T>, A>
memFun1(R (T::*p)(A))
{
    return ::IceUtilInternal::MemFun1<R, T, ICE_INTERNAL_HANDLE<T>, A>(p);
}

template<class T>
inline ::IceUtilInternal::VoidMemFun<T, ICE_INTERNAL_HANDLE<T> >
voidMemFun(void (T::*p)(void))
{
    return ::IceUtilInternal::VoidMemFun<T, ICE_INTERNAL_HANDLE<T> >(p);
}

template<class T, class A>
inline ::IceUtilInternal::VoidMemFun1<T, ICE_INTERNAL_HANDLE<T>, A>
voidMemFun1(void (T::*p)(A))
{
    return ::IceUtilInternal::VoidMemFun1<T, ICE_INTERNAL_HANDLE<T>, A>(p);
}

template<class R, class K, class T>
inline ::IceUtilInternal::SecondMemFun<R, K, T, ICE_INTERNAL_HANDLE<T> >
secondMemFun(R (T::*p)(void))
{
    return ::IceUtilInternal::SecondMemFun<R, K, T, ICE_INTERNAL_HANDLE<T> >(p);
}

template<class R, class K, class T, class A>
inline ::IceUtilInternal::SecondMemFun1<R, K, T, ICE_INTERNAL_HANDLE<T>, A>
secondMemFun1(R (T::*p)(A))
{
    return ::IceUtilInternal::SecondMemFun1<R, K, T, ICE_INTERNAL_HANDLE<T>, A>(p);
}

template<class K, class T>
inline ::IceUtilInternal::SecondVoidMemFun<K, T, ICE_INTERNAL_HANDLE<T> >
secondVoidMemFun(void (T::*p)(void))
{
    return ::IceUtilInternal::SecondVoidMemFun<K, T, ICE_INTERNAL_HANDLE<T> >(p);
}

template<class K, class T, class A>
inline ::IceUtilInternal::SecondVoidMemFun1<K, T, ICE_INTERNAL_HANDLE<T>, A>
secondVoidMemFun1(void (T::*p)(A))
{
    return ::IceUtilInternal::SecondVoidMemFun1<K, T, ICE_INTERNAL_HANDLE<T>, A>(p);
}

template<class R, class T>
inline ::IceUtilInternal::ConstMemFun<R, T, ICE_INTERNAL_HANDLE<T> >
constMemFun(R (T::*p)(void) const)
{
    return ::IceUtilInternal::ConstMemFun<R, T, ICE_INTERNAL_HANDLE<T> >(p);
}

template<class R, class T, class A>
inline ::IceUtilInternal::ConstMemFun1<R, T, ICE_INTERNAL_HANDLE<T>, A>
constMemFun1(R (T::*p)(A) const)
{
    return ::IceUtilInternal::ConstMemFun1<R, T, ICE_INTERNAL_HANDLE<T>, A>(p);
}

template<class T>
inline ::IceUtilInternal::ConstVoidMemFun<T, ICE_INTERNAL_HANDLE<T> >
constVoidMemFun(void (T::*p)(void) const)
{
    return ::IceUtilInternal::ConstVoidMemFun<T, ICE_INTERNAL_HANDLE<T> >(p);
}

template<class T, class A>
inline ::IceUtilInternal::ConstVoidMemFun1<T, ICE_INTERNAL_HANDLE<T>, A>
constVoidMemFun1(void (T::*p)(A) const)
{
    return ::IceUtilInternal::ConstVoidMemFun1<T, ICE_INTERNAL_HANDLE<T>, A>(p);
}

template<class R, class K, class T>
inline ::IceUtilInternal::SecondConstMemFun<R, K, T, ICE_INTERNAL_HANDLE<T> >
secondConstMemFun(R (T::*p)(void) const)
{
    return ::IceUtilInternal::SecondConstMemFun<R, K, T, ICE_INTERNAL_HANDLE<T> >(p);
}

template<class R, class K, class T, class A>
inline ::IceUtilInternal::SecondConstMemFun1<R, K, T, ICE_INTERNAL_HANDLE<T>, A>
secondConstMemFun1(R (T::*p)(A) const)
{
    return ::IceUtilInternal::SecondConstMemFun1<R, K, T, ICE_INTERNAL_HANDLE<T>, A>(p);
}

template<class K, class T>
inline ::IceUtilInternal::SecondConstVoidMemFun<K, T, ICE_INTERNAL_HANDLE<T> >
secondConstVoidMemFun(void (T::*p)(void) const)
{
    return ::IceUtilInternal::SecondConstVoidMemFun<K, T, ICE_INTERNAL_HANDLE<T> >(p);
}

template<class K, class T, class A>
inline ::IceUtilInternal::SecondConstVoidMemFun1<K, T, ICE_INTERNAL_HANDLE<T>, A>
secondConstVoidMemFun1(void (T::*p)(A) const)
{
    return ::IceUtilInternal::SecondConstVoidMemFun1<K, T, ICE_INTERNAL_HANDLE<T>, A>(p);
}
/// \endcond

}

#endif
