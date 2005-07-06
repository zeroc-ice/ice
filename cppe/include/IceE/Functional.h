// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_FUNCTIONAL_H
#define ICEE_FUNCTIONAL_H

#include <IceE/Handle.h>
#include <functional>

namespace IceInternal
{

// ----------------------------------------------------------------------
// Various function objects that work with handles instead of plain
// pointers.
// ----------------------------------------------------------------------

template<class R, class T, class H>
class MemFun : public std::unary_function<H, R>
{
    typedef R (T::*MemberFN)(void);
    MemberFN _mfn;

public:

    explicit MemFun(MemberFN p) : _mfn(p) { }
    R operator()(H handle) const
    {
	return (handle.get() ->* _mfn)();
    }
};

template<class R, class T, class H, class A>
class MemFun1 : public std::binary_function<H, A, R>
{
    typedef R (T::*MemberFN)(A);
    MemberFN _mfn;

public:

    explicit MemFun1(MemberFN p) : _mfn(p) { }
    R operator()(H handle, A arg) const
    {
	return (handle.get() ->* _mfn)(arg);
    }
};

template<class T, class H>
class VoidMemFun : public std::unary_function<H, void>
{
    typedef void (T::*MemberFN)(void);
    MemberFN _mfn;

public:

    explicit VoidMemFun(MemberFN p) : _mfn(p) { }
    void operator()(H handle) const
    {
	(handle.get() ->* _mfn)();
    }
};

template<class T, class H, class A>
class VoidMemFun1 : public std::binary_function<H, A, void>
{
    typedef void (T::*MemberFN)(A);
    MemberFN _mfn;

public:

    explicit VoidMemFun1(MemberFN p) : _mfn(p) { }
    void operator()(H handle, A arg) const
    {
	(handle.get() ->* _mfn)(arg);
    }
};

template<class R, class K, class T, class H>
class SecondMemFun : public std::unary_function<std::pair<K, H>, R>
{
    typedef R (T::*MemberFN)(void);
    MemberFN _mfn;

public:

    explicit SecondMemFun(MemberFN p) : _mfn(p) { }
    R operator()(std::pair<K, H> pair) const
    {
	return (pair.second.get() ->* _mfn)();
    }
};

template<class R, class K, class T, class H, class A>
class SecondMemFun1 : public std::binary_function<std::pair<K, H>, A, R>
{
    typedef R (T::*MemberFN)(A);
    MemberFN _mfn;

public:

    explicit SecondMemFun1(MemberFN p) : _mfn(p) { }
    R operator()(std::pair<K, H> pair, A arg) const
    {
	return (pair.second.get() ->* _mfn)(arg);
    }
};

template<class K, class T, class H>
class SecondVoidMemFun : public std::unary_function<std::pair<K, H>, void>
{
    typedef void (T::*MemberFN)(void);
    MemberFN _mfn;

public:

    explicit SecondVoidMemFun(MemberFN p) : _mfn(p) { }
    void operator()(std::pair<K, H> pair) const
    {
	(pair.second.get() ->* _mfn)();
    }
};

template<class K, class T, class H, class A>
class SecondVoidMemFun1 : public std::binary_function<std::pair<K, H>, A, void>
{
    typedef void (T::*MemberFN)(A);
    MemberFN _mfn;

public:

    explicit SecondVoidMemFun1(MemberFN p) : _mfn(p) { }
    void operator()(std::pair<K, H> pair, A arg) const
    {
	(pair.second.get() ->* _mfn)(arg);
    }
};

template<class R, class T, class H>
class ConstMemFun : public std::unary_function<H, R>
{
    typedef R (T::*MemberFN)(void) const;
    MemberFN _mfn;

public:

    explicit ConstMemFun(MemberFN p) : _mfn(p) { }
    R operator()(H handle) const
    {
	return (handle.get() ->* _mfn)();
    }
};

template<class R, class T, class H, class A>
class ConstMemFun1 : public std::binary_function<H, A, R>
{
    typedef R (T::*MemberFN)(A) const;
    MemberFN _mfn;

public:

    explicit ConstMemFun1(MemberFN p) : _mfn(p) { }
    R operator()(H handle, A arg) const
    {
	return (handle.get() ->* _mfn)(arg);
    }
};

template<class T, class H>
class ConstVoidMemFun : public std::unary_function<H, void>
{
    typedef void (T::*MemberFN)(void) const;
    MemberFN _mfn;

public:

    explicit ConstVoidMemFun(MemberFN p) : _mfn(p) { }
    void operator()(H handle) const
    {
	(handle.get() ->* _mfn)();
    }
};

template<class T, class H, class A>
class ConstVoidMemFun1 : public std::binary_function<H, A, void>
{
    typedef void (T::*MemberFN)(A) const;
    MemberFN _mfn;

public:

    explicit ConstVoidMemFun1(MemberFN p) : _mfn(p) { }
    void operator()(H handle, A arg) const
    {
	(handle.get() ->* _mfn)(arg);
    }
};

template<class R, class K, class T, class H>
class SecondConstMemFun : public std::unary_function<std::pair<K, H>, R>
{
    typedef R (T::*MemberFN)(void) const;
    MemberFN _mfn;

public:

    explicit SecondConstMemFun(MemberFN p) : _mfn(p) { }
    R operator()(std::pair<K, H> pair) const
    {
	return (pair.second.get() ->* _mfn)();
    }
};

template<class R, class K, class T, class H, class A>
class SecondConstMemFun1 : public std::binary_function<std::pair<K, H>, A, R>
{
    typedef R (T::*MemberFN)(A) const;
    MemberFN _mfn;

public:

    explicit SecondConstMemFun1(MemberFN p) : _mfn(p) { }
    R operator()(std::pair<K, H> pair, A arg) const
    {
	return (pair.second.get() ->* _mfn)(arg);
    }
};

template<class K, class T, class H>
class SecondConstVoidMemFun : public std::unary_function<std::pair<K, H>, void>
{
    typedef void (T::*MemberFN)(void) const;
    MemberFN _mfn;

public:

    explicit SecondConstVoidMemFun(MemberFN p) : _mfn(p) { }
    void operator()(std::pair<K, H> pair) const
    {
	(pair.second.get() ->* _mfn)();
    }
};

template<class K, class T, class H, class A>
class SecondConstVoidMemFun1 : public std::binary_function<std::pair<K, H>, A, void>
{
    typedef void (T::*MemberFN)(A) const;
    MemberFN _mfn;

public:

    explicit SecondConstVoidMemFun1(MemberFN p) : _mfn(p) { }
    void operator()(std::pair<K, H> pair, A arg) const
    {
	(pair.second.get() ->* _mfn)(arg);
    }
};

}

// ----------------------------------------------------------------------
// Inline functions that return function objects that work with
// IceUtilE::Handle
// ----------------------------------------------------------------------

namespace IceUtilE
{

template<class R, class T>
inline ::IceInternal::MemFun<R, T, Ice::Handle<T> >
memFun(R (T::*p)(void))
{
    return ::IceInternal::MemFun<R, T, Ice::Handle<T> >(p);
}

template<class R, class T, class A>
inline ::IceInternal::MemFun1<R, T, Ice::Handle<T>, A>
memFun1(R (T::*p)(A))
{
    return ::IceInternal::MemFun1<R, T, Ice::Handle<T>, A>(p);
}

template<class T>
inline ::IceInternal::VoidMemFun<T, Ice::Handle<T> >
voidMemFun(void (T::*p)(void))
{
    return ::IceInternal::VoidMemFun<T, Ice::Handle<T> >(p);
}

template<class T, class A>
inline ::IceInternal::VoidMemFun1<T, Ice::Handle<T>, A>
voidMemFun1(void (T::*p)(A))
{
    return ::IceInternal::VoidMemFun1<T, Ice::Handle<T>, A>(p);
}

template<class R, class K, class T>
inline ::IceInternal::SecondMemFun<R, K, T, Ice::Handle<T> >
secondMemFun(R (T::*p)(void))
{
    return ::IceInternal::SecondMemFun<R, K, T, Ice::Handle<T> >(p);
}

template<class R, class K, class T, class A>
inline ::IceInternal::SecondMemFun1<R, K, T, Ice::Handle<T>, A>
secondMemFun1(R (T::*p)(A))
{
    return ::IceInternal::SecondMemFun1<R, K, T, Ice::Handle<T>, A>(p);
}

template<class K, class T>
inline ::IceInternal::SecondVoidMemFun<K, T, Ice::Handle<T> >
secondVoidMemFun(void (T::*p)(void))
{
    return ::IceInternal::SecondVoidMemFun<K, T, Ice::Handle<T> >(p);
}

template<class K, class T, class A>
inline ::IceInternal::SecondVoidMemFun1<K, T, Ice::Handle<T>, A>
secondVoidMemFun1(void (T::*p)(A))
{
    return ::IceInternal::SecondVoidMemFun1<K, T, Ice::Handle<T>, A>(p);
}

template<class R, class T>
inline ::IceInternal::ConstMemFun<R, T, Ice::Handle<T> >
constMemFun(R (T::*p)(void) const)
{
    return ::IceInternal::ConstMemFun<R, T, Ice::Handle<T> >(p);
}

template<class R, class T, class A>
inline ::IceInternal::ConstMemFun1<R, T, Ice::Handle<T>, A>
constMemFun1(R (T::*p)(A) const)
{
    return ::IceInternal::ConstMemFun1<R, T, Ice::Handle<T>, A>(p);
}

template<class T>
inline ::IceInternal::ConstVoidMemFun<T, Ice::Handle<T> >
constVoidMemFun(void (T::*p)(void) const)
{
    return ::IceInternal::ConstVoidMemFun<T, Ice::Handle<T> >(p);
}

template<class T, class A>
inline ::IceInternal::ConstVoidMemFun1<T, Ice::Handle<T>, A>
constVoidMemFun1(void (T::*p)(A) const)
{
    return ::IceInternal::ConstVoidMemFun1<T, Ice::Handle<T>, A>(p);
}

template<class R, class K, class T>
inline ::IceInternal::SecondConstMemFun<R, K, T, Ice::Handle<T> >
secondConstMemFun(R (T::*p)(void) const)
{
    return ::IceInternal::SecondConstMemFun<R, K, T, Ice::Handle<T> >(p);
}

template<class R, class K, class T, class A>
inline ::IceInternal::SecondConstMemFun1<R, K, T, Ice::Handle<T>, A>
secondConstMemFun1(R (T::*p)(A) const)
{
    return ::IceInternal::SecondConstMemFun1<R, K, T, Ice::Handle<T>, A>(p);
}

template<class K, class T>
inline ::IceInternal::SecondConstVoidMemFun<K, T, Ice::Handle<T> >
secondConstVoidMemFun(void (T::*p)(void) const)
{
    return ::IceInternal::SecondConstVoidMemFun<K, T, Ice::Handle<T> >(p);
}

template<class K, class T, class A>
inline ::IceInternal::SecondConstVoidMemFun1<K, T, Ice::Handle<T>, A>
secondConstVoidMemFun1(void (T::*p)(A) const)
{
    return ::IceInternal::SecondConstVoidMemFun1<K, T, Ice::Handle<T>, A>(p);
}

}

// ----------------------------------------------------------------------
// Extension for STLport: Special versions for bind1st and bind2nd for
// operations that do not return anything (i.e., return void). Needed
// for broken compilers, such as Visual C++ 6.0.
// ----------------------------------------------------------------------

#ifdef _STLP_BEGIN_NAMESPACE

_STLP_BEGIN_NAMESPACE

template <class _Operation> 
class voidbinder1st : 
    public unary_function<typename __BINARY_ARG(_Operation,second_argument_type),
                          typename __BINARY_ARG(_Operation,result_type) > {
protected:
  _Operation _M_op;
  typename _Operation::first_argument_type _M_value;
public:
  voidbinder1st(const _Operation& __x,
            const typename _Operation::first_argument_type& __y)
      : _M_op(__x), _M_value(__y) {}
  typename _Operation::result_type
  operator()(const typename _Operation::second_argument_type& __x) const {
    _M_op(_M_value, __x); 
  }
};

template <class _Operation, class _Tp>
inline voidbinder1st<_Operation> 
voidbind1st(const _Operation& __fn, const _Tp& __x) 
{
  typedef typename _Operation::first_argument_type _Arg1_type;
  return voidbinder1st<_Operation>(__fn, _Arg1_type(__x));
}

template <class _Operation> 
class voidbinder2nd
  : public unary_function<typename __BINARY_ARG(_Operation,first_argument_type),
                          typename __BINARY_ARG(_Operation,result_type)> {
protected:
  _Operation _M_op;
  typename _Operation::second_argument_type value;
public:
  voidbinder2nd(const _Operation& __x,
            const typename _Operation::second_argument_type& __y) 
      : _M_op(__x), value(__y) {}
  typename _Operation::result_type
  operator()(const typename _Operation::first_argument_type& __x) const {
    _M_op(__x, value); 
  }
};

template <class _Operation, class _Tp>
inline voidbinder2nd<_Operation> 
voidbind2nd(const _Operation& __fn, const _Tp& __x) 
{
  typedef typename _Operation::second_argument_type _Arg2_type;
  return voidbinder2nd<_Operation>(__fn, _Arg2_type(__x));
}

_STLP_END_NAMESPACE

#endif

// ----------------------------------------------------------------------
// Inline functions that return function objects that work with
// IceInternal::Handle
// ----------------------------------------------------------------------

namespace Ice
{

template<class R, class T>
inline ::IceInternal::MemFun<R, T, ::IceInternal::Handle<T> >
memFun(R (T::*p)(void))
{
    return ::IceInternal::MemFun<R, T, ::IceInternal::Handle<T> >(p);
}

template<class R, class T, class A>
inline ::IceInternal::MemFun1<R, T, ::IceInternal::Handle<T>, A>
memFun1(R (T::*p)(A))
{
    return ::IceInternal::MemFun1<R, T, ::IceInternal::Handle<T>, A>(p);
}

template<class T>
inline ::IceInternal::VoidMemFun<T, ::IceInternal::Handle<T> >
voidMemFun(void (T::*p)(void))
{
    return ::IceInternal::VoidMemFun<T, ::IceInternal::Handle<T> >(p);
}

template<class T, class A>
inline ::IceInternal::VoidMemFun1<T, ::IceInternal::Handle<T>, A>
voidMemFun1(void (T::*p)(A))
{
    return ::IceInternal::VoidMemFun1<T, ::IceInternal::Handle<T>, A>(p);
}

template<class R, class K, class T>
inline ::IceInternal::SecondMemFun<R, K, T, ::IceInternal::Handle<T> >
secondMemFun(R (T::*p)(void))
{
    return ::IceInternal::SecondMemFun<R, K, T, ::IceInternal::Handle<T> >(p);
}

template<class R, class K, class T, class A>
inline ::IceInternal::SecondMemFun1<R, K, T, ::IceInternal::Handle<T>, A>
secondMemFun1(R (T::*p)(A))
{
    return ::IceInternal::SecondMemFun1<R, K, T, ::IceInternal::Handle<T>, A>(p);
}

template<class K, class T>
inline ::IceInternal::SecondVoidMemFun<K, T, ::IceInternal::Handle<T> >
secondVoidMemFun(void (T::*p)(void))
{
    return ::IceInternal::SecondVoidMemFun<K, T, ::IceInternal::Handle<T> >(p);
}

template<class K, class T, class A>
inline ::IceInternal::SecondVoidMemFun1<K, T, ::IceInternal::Handle<T>, A>
secondVoidMemFun1(void (T::*p)(A))
{
    return ::IceInternal::SecondVoidMemFun1<K, T, ::IceInternal::Handle<T>, A>(p);
}

template<class R, class T>
inline ::IceInternal::ConstMemFun<R, T, ::IceInternal::Handle<T> >
constMemFun(R (T::*p)(void) const)
{
    return ::IceInternal::ConstMemFun<R, T, ::IceInternal::Handle<T> >(p);
}

template<class R, class T, class A>
inline ::IceInternal::ConstMemFun1<R, T, ::IceInternal::Handle<T>, A>
constMemFun1(R (T::*p)(A) const)
{
    return ::IceInternal::ConstMemFun1<R, T, ::IceInternal::Handle<T>, A>(p);
}

template<class T>
inline ::IceInternal::ConstVoidMemFun<T, ::IceInternal::Handle<T> >
constVoidMemFun(void (T::*p)(void) const)
{
    return ::IceInternal::ConstVoidMemFun<T, ::IceInternal::Handle<T> >(p);
}

template<class T, class A>
inline ::IceInternal::ConstVoidMemFun1<T, ::IceInternal::Handle<T>, A>
constVoidMemFun1(void (T::*p)(A) const)
{
    return ::IceInternal::ConstVoidMemFun1<T, ::IceInternal::Handle<T>, A>(p);
}

template<class R, class K, class T>
inline ::IceInternal::SecondConstMemFun<R, K, T, ::IceInternal::Handle<T> >
secondConstMemFun(R (T::*p)(void) const)
{
    return ::IceInternal::SecondConstMemFun<R, K, T, ::IceInternal::Handle<T> >(p);
}

template<class R, class K, class T, class A>
inline ::IceInternal::SecondConstMemFun1<R, K, T, ::IceInternal::Handle<T>, A>
secondConstMemFun1(R (T::*p)(A) const)
{
    return ::IceInternal::SecondConstMemFun1<R, K, T, ::IceInternal::Handle<T>, A>(p);
}

template<class K, class T>
inline ::IceInternal::SecondConstVoidMemFun<K, T, ::IceInternal::Handle<T> >
secondConstVoidMemFun(void (T::*p)(void) const)
{
    return ::IceInternal::SecondConstVoidMemFun<K, T, ::IceInternal::Handle<T> >(p);
}

template<class K, class T, class A>
inline ::IceInternal::SecondConstVoidMemFun1<K, T, ::IceInternal::Handle<T>, A>
secondConstVoidMemFun1(void (T::*p)(A) const)
{
    return ::IceInternal::SecondConstVoidMemFun1<K, T, ::IceInternal::Handle<T>, A>(p);
}

}

#endif
