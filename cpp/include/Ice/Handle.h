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

namespace IceInternal
{

//
// Generic handle class, using intrusive reference counting. Two
// global operations IceInternal::incRef(T*) and IceInternal::decRef(T*) must be
// declared for each T before using this template. The use of global
// operations allows this template to be used for types which are
// declared but not defined, provided that the two above mentioned
// operations are declared as well.
//
template<typename T>
class Handle
{
public:
    
    typedef T element_type;
    
//    Handle() : Ptr(0) { }
    
    Handle(T* p = 0)
	: Ptr(p)
    {
	if (Ptr)
	    incRef(Ptr);
    }
    
    template<typename Y>
    Handle(const Handle<Y>& r)
	: Ptr(r.Ptr)
    {
	if (Ptr)
	    incRef(Ptr);
    }

#ifdef WIN32 // COMPILERBUG: Is VC++ or GNU C++ right here???
    template<>
    Handle(const Handle<T>& r)
#else
    Handle(const Handle& r)
#endif
	: Ptr(r.Ptr)
    {
	if (Ptr)
	    incRef(Ptr);
    }
    
    ~Handle()
    {
	if (Ptr)
	    decRef(Ptr);
    }
    
    Handle& operator=(T* p)
    {
	if (Ptr != p)
	{
	    if (p)
		incRef(p);

	    if (Ptr)
		decRef(Ptr);
	    
	    Ptr = p;
	}
	return *this;
    }
        
    template<typename Y>
    Handle& operator=(const Handle<Y>& r)
    {
	if (Ptr != r.Ptr)
	{
	    if (r.Ptr)
		incRef(r.Ptr);

	    if (Ptr)
		decRef(Ptr);
	    
	    Ptr = r.Ptr;
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
	if (Ptr != r.Ptr)
	{
	    if (r.Ptr)
		incRef(r.Ptr);

	    if (Ptr)
		decRef(Ptr);
	    
	    Ptr = r.Ptr;
	}
	return *this;
    }
        
    template<class Y>
    static Handle dynamicCast(const Handle<Y>& r)
    {
	return Handle(dynamic_cast<T*>(r.Ptr));
    }

    template<class Y>
    static Handle dynamicCast(Y* p)
    {
	return Handle(dynamic_cast<T*>(p));
    }

    T* get() const { return Ptr; }
    T* operator->() const { return Ptr; }
    operator bool() const { return Ptr ? true : false; }

    void swap(Handle& other) { std::swap(Ptr, other.Ptr); }

#ifndef WIN32 // COMPILERBUG: VC++ 6.0 doesn't understand this
 
    template<typename Y> friend class Handle;

protected:

#endif

    T* Ptr;
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

}

#endif
