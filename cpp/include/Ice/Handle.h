// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_HANDLE_H
#define ICE_HANDLE_H

#include <Ice/Config.h>
#include <algorithm>

//
// "Handle" or "smart pointer" classes for classes derived from
// Ice::Shared or Ice::SimpleShared.
//
// IceInternal::Handle
// ===================
//
// Generic handle class, using intrusive reference counting. Two
// global operations IceInternal::incRef(T*) and
// IceInternal::decRef(T*) must be declared for each T before using
// this template. The use of global operations allows this template to
// be used for types which are declared but not defined, provided that
// the two above mentioned operations are declared as well.
//
// Ice::Handle
// ===========
//
// A simplified handle class similar to IceInternal::Handle, but
// intended to be used by application code, and therfore in the Ice
// namespace instead of in IceInternal. No global operations
// incRef(T*) and decRef(T*) must be declared for this handle
// class. Instead, this handle class call __incRef() and __decRef() on
// T directly.
//

namespace IceInternal
{

template<typename T>
class Handle
{
public:
    
    Handle(T* p = 0) :
	_ptr(p)
    {
	if (_ptr)
	{
	    incRef(_ptr);
	}
    }
    
    template<typename Y>
    Handle(const Handle<Y>& r) :
	_ptr(r._ptr)
    {
	if (_ptr)
	{
	    incRef(_ptr);
	}
    }

#ifdef WIN32 // COMPILERBUG: Is VC++ or GNU C++ right here???
    template<>
    Handle(const Handle<T>& r) :
#else
    Handle(const Handle& r) :
#endif
	_ptr(r._ptr)
    {
	if (_ptr)
	{
	    incRef(_ptr);
	}
    }
    
    ~Handle()
    {
	if (_ptr)
	{
	    decRef(_ptr);
	}
    }
    
    Handle& operator=(T* p)
    {
	if (_ptr != p)
	{
	    if (p)
	    {
		incRef(p);
	    }

	    if (_ptr)
	    {
		decRef(_ptr);
	    }
	    
	    _ptr = p;
	}
	return *this;
    }
        
    template<typename Y>
    Handle& operator=(const Handle<Y>& r)
    {
	if (_ptr != r._ptr)
	{
	    if (r._ptr)
	    {
		incRef(r._ptr);
	    }

	    if (_ptr)
	    {
		decRef(_ptr);
	    }
	    
	    _ptr = r._ptr;
	}
	return *this;
    }

#ifdef WIN32 // COMPILERBUG: Is VC++ or GNU C++ right here???
    template<>
    Handle& operator=(const Handle<T>& r)
#else
    Handle& operator=(const Handle& r)
#endif
    {
	if (_ptr != r._ptr)
	{
	    if (r._ptr)
	    {
		incRef(r._ptr);
	    }

	    if (_ptr)
	    {
		decRef(_ptr);
	    }
	    
	    _ptr = r._ptr;
	}
	return *this;
    }
        
    template<class Y>
    static Handle dynamicCast(const Handle<Y>& r)
    {
	return Handle(dynamic_cast<T*>(r._ptr));
    }

    template<class Y>
    static Handle dynamicCast(Y* p)
    {
	return Handle(dynamic_cast<T*>(p));
    }

    typedef T element_type;
    
    T* get() const { return _ptr; }
    T* operator->() const { return _ptr; }
    operator bool() const { return _ptr ? true : false; }

    void swap(Handle& other) { std::swap(_ptr, other._ptr); }

#ifndef WIN32 // COMPILERBUG: VC++ 6.0 doesn't understand this
 
    template<typename Y> friend class Handle;

protected:

#endif

    T* _ptr;
};

template<typename T, typename U>
inline bool operator==(const Handle<T>& a, const Handle<U>& b)
{
    return *a.get() == *b.get();
}

template<typename T, typename U>
inline bool operator!=(const Handle<T>& a, const Handle<U>& b)
{
    return *a.get() != *b.get();
}

template<typename T, typename U>
inline bool operator<(const Handle<T>& a, const Handle<U>& b)
{
    return *a.get() < *b.get();
}

template<typename T, typename U>
inline bool operator<=(const Handle<T>& a, const Handle<U>& b)
{
    return *a.get() <= *b.get();
}

template<typename T, typename U>
inline bool operator>(const Handle<T>& a, const Handle<U>& b)
{
    return *a.get() > *b.get();
}

template<typename T, typename U>
inline bool operator>=(const Handle<T>& a, const Handle<U>& b)
{
    return *a.get() >= *b.get();
}

}

namespace Ice
{

template<typename T>
class Handle
{
public:
    
    Handle(T* p = 0) :
	_ptr(p)
    {
	if (_ptr)
	{
	    _ptr->__incRef();
	}
    }
    
    template<typename Y>
    Handle(const Handle<Y>& r) :
	_ptr(r._ptr)
    {
	if (_ptr)
	{
	    _ptr->__incRef();
	}
    }

#ifdef WIN32 // COMPILERBUG: Is VC++ or GNU C++ right here???
    template<>
    Handle(const Handle<T>& r) :
#else
    Handle(const Handle& r) :
#endif
	_ptr(r._ptr)
    {
	if (_ptr)
	{
	    _ptr->__incRef();
	}
    }
    
    ~Handle()
    {
	if (_ptr)
	{
	    _ptr->__decRef();
	}
    }
    
    Handle& operator=(T* p)
    {
	if (_ptr != p)
	{
	    if (p)
	    {
		p->__incRef();
	    }

	    if (_ptr)
	    {
		_ptr->__decRef();
	    }
	    
	    _ptr = p;
	}
	return *this;
    }
        
    template<typename Y>
    Handle& operator=(const Handle<Y>& r)
    {
	if (_ptr != r._ptr)
	{
	    if (r._ptr)
	    {
		r._ptr->__incRef();
	    }

	    if (_ptr)
	    {
		_ptr->__decRef();
	    }
	    
	    _ptr = r._ptr;
	}
	return *this;
    }

#ifdef WIN32 // COMPILERBUG: Is VC++ or GNU C++ right here???
    template<>
    Handle& operator=(const Handle<T>& r)
#else
    Handle& operator=(const Handle& r)
#endif
    {
	if (_ptr != r._ptr)
	{
	    if (r._ptr)
	    {
		r._ptr->__incRef();
	    }

	    if (_ptr)
	    {
		_ptr->__decRef();
	    }
	    
	    _ptr = r._ptr;
	}
	return *this;
    }
        
    template<class Y>
    static Handle dynamicCast(const Handle<Y>& r)
    {
	return Handle(dynamic_cast<T*>(r._ptr));
    }

    template<class Y>
    static Handle dynamicCast(Y* p)
    {
	return Handle(dynamic_cast<T*>(p));
    }

    typedef T element_type;
    
    T* get() const { return _ptr; }
    T* operator->() const { return _ptr; }
    operator bool() const { return _ptr ? true : false; }

    void swap(Handle& other) { std::swap(_ptr, other._ptr); }

#ifndef WIN32 // COMPILERBUG: VC++ 6.0 doesn't understand this
 
    template<typename Y> friend class Handle;

protected:

#endif

    T* _ptr;
};

template<typename T, typename U>
inline bool operator==(const Handle<T>& a, const Handle<U>& b)
{
    return *a.get() == *b.get();
}

template<typename T, typename U>
inline bool operator!=(const Handle<T>& a, const Handle<U>& b)
{
    return *a.get() != *b.get();
}

template<typename T, typename U>
inline bool operator<(const Handle<T>& a, const Handle<U>& b)
{
    return *a.get() < *b.get();
}

template<typename T, typename U>
inline bool operator<=(const Handle<T>& a, const Handle<U>& b)
{
    return *a.get() <= *b.get();
}

template<typename T, typename U>
inline bool operator>(const Handle<T>& a, const Handle<U>& b)
{
    return *a.get() > *b.get();
}

template<typename T, typename U>
inline bool operator>=(const Handle<T>& a, const Handle<U>& b)
{
    return *a.get() >= *b.get();
}

}

#endif
