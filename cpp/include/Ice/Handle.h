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

namespace __Ice
{

//
// Generic handle class, using intrusive reference counting. Two
// global operations __Ice::incRef(T*) and __Ice::decRef(T*) must be
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
    
//    Handle() : ptr_(0) { }
    
    Handle(T* p = 0)
	: ptr_(p)
    {
	if(ptr_)
	    incRef(ptr_);
    }
    
    template<typename Y>
    Handle(const Handle<Y>& r)
	: ptr_(r.ptr_)
    {
	if(ptr_)
	    incRef(ptr_);
    }

#ifdef WIN32 // COMPILERBUG: Is VC++ or GNU C++ right here???
    template<>
    Handle(const Handle<T>& r)
#else
    Handle(const Handle& r)
#endif
	: ptr_(r.ptr_)
    {
	if(ptr_)
	    incRef(ptr_);
    }
    
    ~Handle()
    {
	if(ptr_)
	    decRef(ptr_);
    }
    
    Handle& operator=(T* p)
    {
	if(ptr_ != p)
	{
	    if(p)
		incRef(p);

	    if(ptr_)
		decRef(ptr_);
	    
	    ptr_ = p;
	}
	return *this;
    }
        
    template<typename Y>
    Handle& operator=(const Handle<Y>& r)
    {
	if(ptr_ != r.ptr_)
	{
	    if(r.ptr_)
		incRef(r.ptr_);

	    if(ptr_)
		decRef(ptr_);
	    
	    ptr_ = r.ptr_;
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
	if(ptr_ != r.ptr_)
	{
	    if(r.ptr_)
		incRef(r.ptr_);

	    if(ptr_)
		decRef(ptr_);
	    
	    ptr_ = r.ptr_;
	}
	return *this;
    }
        
    template<class Y>
    static Handle dynamicCast(const Handle<Y>& r)
    {
	return Handle(dynamic_cast<T*>(r.ptr_));
    }

    template<class Y>
    static Handle dynamicCast(Y* p)
    {
	return Handle(dynamic_cast<T*>(p));
    }

    T* get() const { return ptr_; }

    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
    operator bool() const { return ptr_ ? true : false; }

    void swap(Handle& other) { std::swap(ptr_, other.ptr_); }

#ifndef WIN32 // COMPILERBUG: VC++ 6.0 doesn't understand this
 
    template<typename Y> friend class Handle;

protected:

#endif

    T* ptr_;
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
