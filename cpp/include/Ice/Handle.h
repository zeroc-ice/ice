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
    
    Handle(T* p = 0)
	: ptr_(p)
    {
	if(ptr_)
	    incRef(ptr_);
    }
    
    Handle(const Handle& r)
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
	    if(ptr_)
		decRef(ptr_);
	    
	    ptr_ = p;
	    
	    if(ptr_)
		incRef(ptr_);
	}
	return *this;
    }
        
    Handle& operator=(const Handle& r)
    {
	if(ptr_ != r.ptr_)
	{
	    if(ptr_)
		decRef(ptr_);
	    
	    ptr_ = r.ptr_;
	    
	    if(ptr_)
		incRef(ptr_);
	}
	return *this;
    }
        
//
// Some compilers (like Visual C++ 6.0) do not support member
// templates :-( I therefore don't use them, otherwise Ice code could
// be non-portable.
//
/*
    template<typename Y>
    Handle(const Handle<Y>& r)
	: ptr_(r.ptr_)
    {
	if(ptr_)
	    incRef(ptr_);
    }

    template<typename Y>
    Handle& operator=(const Handle<Y>& r)
    {
	if(ptr_ != r.ptr_)
	{
	    if(ptr_)
		decRef(ptr_);
	    
	    ptr_ = r.ptr_;
	    
	    if(ptr_)
		incRef(ptr_);
	}
	return *this;
    }

    template<typename Y> friend class Handle;
*/

    T* get() const { return ptr_; }

    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
    operator T*() const { return ptr_; }

    void swap(Handle<T>& other) { std::swap(ptr_, other.ptr_); }

protected:

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
