// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_HANDLE_FUNCTIONAL_H
#define ICE_HANDLE_FUNCTIONAL_H

#include <Ice/Handle.h>
#include <functional>

namespace Ice
{

template<class R, class T>
class memFunT
    : public std::unary_function<__Ice::Handle<T>, R>
{
    typedef R (T::*funT)(void);
    funT mfn;

public:

    explicit memFunT(funT p) : mfn(p) { }
    R operator()(const __Ice::Handle<T>& p) const
    {
	return ((p.get() ->* mfn)());
    }
};

template<class R, class T>
inline memFunT<R, T>
memFun(R (T::*p)(void))
{
    return (memFunT<R, T>(p));
}

template<class R, class T, class A>
class memFun1T
    : public std::binary_function<__Ice::Handle<T>, A, R>
{
    typedef R (T::*funT)(A);
    funT mfn;

public:

    explicit memFun1T(funT p) : mfn(p) { }
    R operator()(const __Ice::Handle<T>& h, A arg) const
    {
	return ((p.get() ->* mfn)(arg));
    }
};

template<class R, class T, class A>
inline memFun1T<R, T, A>
memFun1(R (T::*p)(A))
{
    return (memFun1T<R, T, A>(p));
}

template<class T>
class voidMemFunT
    : public std::unary_function<__Ice::Handle<T>, void>
{
    typedef void (T::*funT)(void);
    funT mfn;

public:

    explicit voidMemFunT(funT p) : mfn(p) { }
    void operator()(const __Ice::Handle<T>& p) const
    {
	(p.get() ->* mfn)();
    }
};

template<class T>
inline voidMemFunT<T>
voidMemFun(void (T::*p)(void))
{
    return voidMemFunT<T>(p);
}

template<class T, class A>
class voidMemFun1T
    : public std::binary_function<__Ice::Handle<T>, A, void>
{
    typedef void (T::*funT)(A);
    funT mfn;

public:

    explicit voidMemFun1T(funT p) : mfn(p) { }
    void operator()(const __Ice::Handle<T>& h, A arg) const
    {
	(p.get() ->* mfn)(arg);
    }
};

template<class T, class A>
inline voidMemFun1T<T, A>
voidMemFun1(void (T::*p)(A))
{
    return voidMemFun1T<T, A>(p);
}

template<class R, class K, class T>
class secondMemFunT
    : public std::unary_function<std::pair<K, __Ice::Handle<T> >, R>
{
    typedef R (T::*funT)(void);
    funT mfn;

public:

    explicit secondMemFunT(funT p) : mfn(p) { }
    R operator()(std::pair<K, const __Ice::Handle<T>&> p) const
    {
	return ((p.second.get() ->* mfn)());
    }
};

template<class R, class K, class T>
inline secondMemFunT<R, K, T>
secondMemFun(R (T::*p)(void))
{
    return (secondMemFunT<R, K, T>(p));
}

template<class R, class K, class T, class A>
class secondMemFun1T
    : public std::binary_function<std::pair<K, const __Ice::Handle<T>&>, A, R>
{
    typedef R (T::*funT)(A);
    funT mfn;

public:

    explicit secondMemFun1T(funT p) : mfn(p) { }
    R operator()(std::pair<K, const __Ice::Handle<T>&> h, A arg) const
    {
	return ((p.second.get() ->* mfn)(arg));
    }
};

template<class R, class K, class T, class A>
inline secondMemFun1T<R, K, T, A>
secondMemFun1(R (T::*p)(A))
{
    return (secondMemFun1T<R, K, T, A>(p));
}

template<class K, class T>
class secondVoidMemFunT
    : public std::unary_function<std::pair<K, __Ice::Handle<T> >, void>
{
    typedef void (T::*funT)(void);
    funT mfn;

public:

    explicit secondVoidMemFunT(funT p) : mfn(p) { }
    void operator()(std::pair<K, __Ice::Handle<T> > p) const
    {
	(p.second.get() ->* mfn)();
    }
};

template<class K, class T>
inline secondVoidMemFunT<K, T>
secondVoidMemFun(void (T::*p)(void))
{
    return secondVoidMemFunT<K, T>(p);
}

template<class K, class T, class A>
class secondVoidMemFun1T
    : public std::binary_function<std::pair<K, __Ice::Handle<T> >, A, void>
{
    typedef void (T::*funT)(A);
    funT mfn;

public:

    explicit secondVoidMemFun1T(funT p) : mfn(p) { }
    void operator()(std::pair<K, __Ice::Handle<T> > h, A arg) const
    {
	(p.second.get() ->* mfn)(arg);
    }
};

template<class K, class T, class A>
inline secondVoidMemFun1T<K, T, A>
secondVoidMemFun1(void (T::*p)(A))
{
    return secondVoidMemFun1T<K, T, A>(p);
}

template<class R, class T>
class constMemFunT
    : public std::unary_function<__Ice::Handle<T>, R>
{
    typedef R (T::*funT)(void) const;
    funT mfn;

public:

    explicit constMemFunT(funT p) : mfn(p) { }
    R operator()(const __Ice::Handle<T>& p) const
    {
	return ((p.get() ->* mfn)());
    }
};

template<class R, class T>
inline constMemFunT<R, T>
constMemFun(R (T::*p)(void) const)
{
    return (constMemFunT<R, T>(p));
}

template<class R, class T, class A>
class constMemFun1T
    : public std::binary_function<__Ice::Handle<T>, A, R>
{
    typedef R (T::*funT)(A);
    funT mfn;

public:

    explicit constMemFun1T(funT p) : mfn(p) { }
    R operator()(const __Ice::Handle<T>& h, A arg) const
    {
	return ((p.get() ->* mfn)(arg));
    }
};

template<class R, class T, class A>
inline constMemFun1T<R, T, A>
constMemFun1(R (T::*p)(A))
{
    return (constMemFun1T<R, T, A>(p));
}

template<class T>
class constVoidMemFunT
    : public std::unary_function<__Ice::Handle<T>, void>
{
    typedef void (T::*funT)(void) const;
    funT mfn;

public:

    explicit constVoidMemFunT(funT p) : mfn(p) { }
    void operator()(const __Ice::Handle<T>& p) const
    {
	(p.get() ->* mfn)();
    }
};

template<class T>
inline constVoidMemFunT<T>
constVoidMemFun(void (T::*p)(void) const)
{
    return constVoidMemFunT<T>(p);
}

template<class T, class A>
class constVoidMemFun1T
    : public std::binary_function<__Ice::Handle<T>, A, void>
{
    typedef void (T::*funT)(A);
    funT mfn;

public:

    explicit constVoidMemFun1T(funT p) : mfn(p) { }
    void operator()(const __Ice::Handle<T>& h, A arg) const
    {
	(p.get() ->* mfn)(arg);
    }
};

template<class T, class A>
inline constVoidMemFun1T<T, A>
constVoidMemFun1(void (T::*p)(A))
{
    return constVoidMemFun1T<T, A>(p);
}

template<class R, class K, class T>
class secondConstMemFunT
    : public std::unary_function<std::pair<K, __Ice::Handle<T> >, R>
{
    typedef R (T::*funT)(void) const;
    funT mfn;

public:

    explicit secondConstMemFunT(funT p) : mfn(p) { }
    R operator()(std::pair<K, const __Ice::Handle<T>&> p) const
    {
	return ((p.second.get() ->* mfn)());
    }
};

template<class R, class K, class T>
inline secondConstMemFunT<R, K, T>
secondConstMemFun(R (T::*p)(void) const)
{
    return (secondConstMemFunT<R, K, T>(p));
}

template<class R, class K, class T, class A>
class secondConstMemFun1T
    : public std::binary_function<std::pair<K, const __Ice::Handle<T>&>, A, R>
{
    typedef R (T::*funT)(A);
    funT mfn;

public:

    explicit secondConstMemFun1T(funT p) : mfn(p) { }
    R operator()(std::pair<K, const __Ice::Handle<T>&> h, A arg) const
    {
	return ((p.second.get() ->* mfn)(arg));
    }
};

template<class R, class K, class T, class A>
inline secondConstMemFun1T<R, K, T, A>
secondConstMemFun1(R (T::*p)(A))
{
    return (secondConstMemFun1T<R, K, T, A>(p));
}

template<class K, class T>
class secondConstVoidMemFunT
    : public std::unary_function<std::pair<K, __Ice::Handle<T> >, void>
{
    typedef void (T::*funT)(void) const;
    funT mfn;

public:

    explicit secondConstVoidMemFunT(funT p) : mfn(p) { }
    void operator()(std::pair<K, __Ice::Handle<T> > p) const
    {
	(p.second.get() ->* mfn)();
    }
};

template<class K, class T>
inline secondConstVoidMemFunT<K, T>
secondConstVoidMemFun(void (T::*p)(void) const)
{
    return secondConstVoidMemFunT<K, T>(p);
}

template<class K, class T, class A>
class secondConstVoidMemFun1T
    : public std::binary_function<std::pair<K, __Ice::Handle<T> >, A, void>
{
    typedef void (T::*funT)(A);
    funT mfn;

public:

    explicit secondConstVoidMemFun1T(funT p) : mfn(p) { }
    void operator()(std::pair<K, __Ice::Handle<T> > h, A arg) const
    {
	(p.second.get() ->* mfn)(arg);
    }
};

template<class K, class T, class A>
inline secondConstVoidMemFun1T<K, T, A>
secondConstVoidMemFun1(void (T::*p)(A))
{
    return secondConstVoidMemFun1T<K, T, A>(p);
}

}

#endif
