// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_HANDLE_H
#define ICEE_HANDLE_H

#include <IceE/ExceptionBase.h>
#include <algorithm> // This isn't needed, but the generated code depends on it.

//
// "Handle" or "smart pointer" class for classes derived from
// IceUtil::Shared or IceUtil::SimpleShared.
//
namespace IceUtil
{

template<typename T>
class HandleBase
{
public:

    typedef T element_type;
    
    T* get() const
    {
	return _ptr;
    }

    T* operator->() const
    {
	if(!_ptr)
	{
	    //
	    // We don't throw directly NullHandleException here to
	    // keep the code size of this method to a minimun (the
	    // assembly code for throwing an exception is much bigger
	    // than just a function call). This maximises the chances
	    // of inlining by compiler optimization.
	    //
	    throwNullHandleException(__FILE__, __LINE__);	    
	}

	return _ptr;
    }

    T& operator*() const
    {
	if(!_ptr)
	{
	    //
	    // We don't throw directly NullHandleException here to
	    // keep the code size of this method to a minimun (the
	    // assembly code for throwing an exception is much bigger
	    // than just a function call). This maximises the chances
	    // of inlining by compiler optimization.
	    //
	    throwNullHandleException(__FILE__, __LINE__);	    
	}

	return *_ptr;
    }

    operator bool() const
    {
	return _ptr ? true : false;
    }

    void swap(HandleBase& other)
    {
	std::swap(_ptr, other._ptr);
    }

    T* _ptr;

private:

    void throwNullHandleException(const char *, int) const;
};

template<typename T> void
HandleBase<T>::throwNullHandleException(const char* file, int line) const
{
    throw NullHandleException(file, line);
}

template<typename T, typename U>
inline bool operator==(const HandleBase<T>& lhs, const HandleBase<U>& rhs)
{
    T* l = lhs.get();
    U* r = rhs.get();
    if(l && r)
    {
	return *l == *r;
    }
    else
    {
	return !l && !r;
    }	
}

template<typename T, typename U>
inline bool operator!=(const HandleBase<T>& lhs, const HandleBase<U>& rhs)
{
    return !operator==(lhs, rhs);
}

template<typename T, typename U>
inline bool operator<(const HandleBase<T>& lhs, const HandleBase<U>& rhs)
{
    T* l = lhs.get();
    U* r = rhs.get();
    if(l && r)
    {
	return *l < *r;
    }
    else
    {
	return !l && r;
    }
}

template<typename T, typename U>
inline bool operator<=(const HandleBase<T>& lhs, const HandleBase<U>& rhs)
{
    return lhs < rhs || lhs == rhs;
}

template<typename T, typename U>
inline bool operator>(const HandleBase<T>& lhs, const HandleBase<U>& rhs)
{
    return !(lhs < rhs || lhs == rhs);
}

template<typename T, typename U>
inline bool operator>=(const HandleBase<T>& lhs, const HandleBase<U>& rhs)
{
    return !(lhs < rhs);
}

template<typename T>
class Handle : public HandleBase<T>
{
public:
    
    Handle(T* p = 0)
    {
	this->_ptr = p;

	if(this->_ptr)
	{
	    this->_ptr->__incRef();
	}
    }
    
    template<typename Y>
    Handle(const Handle<Y>& r)
    {
	this->_ptr = r._ptr;

	if(this->_ptr)
	{
	    this->_ptr->__incRef();
	}
    }

    Handle(const Handle& r)
    {
	this->_ptr = r._ptr;

	if(this->_ptr)
	{
	    this->_ptr->__incRef();
	}
    }
    
    ~Handle()
    {
	if(this->_ptr)
	{
	    this->_ptr->__decRef();
	}
    }
    
    Handle& operator=(T* p)
    {
	if(this->_ptr != p)
	{
	    if(p)
	    {
		p->__incRef();
	    }

	    T* ptr = this->_ptr;
	    this->_ptr = p;

	    if(ptr)
	    {
		ptr->__decRef();
	    }
	}
	return *this;
    }
        
    template<typename Y>
    Handle& operator=(const Handle<Y>& r)
    {
	if(this->_ptr != r._ptr)
	{
	    if(r._ptr)
	    {
		r._ptr->__incRef();
	    }

	    T* ptr = this->_ptr;
	    this->_ptr = r._ptr;

	    if(ptr)
	    {
		ptr->__decRef();
	    }
	}
	return *this;
    }

    Handle& operator=(const Handle& r)
    {
	if(this->_ptr != r._ptr)
	{
	    if(r._ptr)
	    {
		r._ptr->__incRef();
	    }

	    T* ptr = this->_ptr;
	    this->_ptr = r._ptr;

	    if(ptr)
	    {
		ptr->__decRef();
	    }
	}
	return *this;
    }
        
    template<class Y>
    static Handle dynamicCast(const HandleBase<Y>& r)
    {
	return Handle(dynamic_cast<T*>(r._ptr));
    }

    template<class Y>
    static Handle dynamicCast(Y* p)
    {
	return Handle(dynamic_cast<T*>(p));
    }
};

}

//
// "Handle" or "smart pointer" class for classes derived from
// IceUtil::Shared, or IceUtil::SimpleShared.
//
// In constrast to IceUtil::Handle, IceInternal::Handle requires the
// declaration of the one global operation IceInternal::upCast(T*).
// The use of global operations allows this template to be used for
// types which are declared but not defined, provided that the two 
// above mentioned operations are declared.
//

namespace IceInternal
{

template<typename T>
class Handle : public ::IceUtil::HandleBase<T>
{
public:
    
    Handle(T* p = 0)
    {
	this->_ptr = p;

	if(this->_ptr)
	{
	    upCast(this->_ptr)->__incRef();
	}
    }
    
    template<typename Y>
    Handle(const Handle<Y>& r)
    {
	this->_ptr = r._ptr;

	if(this->_ptr)
	{
	    upCast(this->_ptr)->__incRef();
	}
    }

    template<typename Y>
    Handle(const ::IceUtil::Handle<Y>& r)
    {
	this->_ptr = r._ptr;

	if(this->_ptr)
	{
	    upCast(this->_ptr)->__incRef();
	}
    }

    Handle(const Handle& r)
    {
	this->_ptr = r._ptr;

	if(this->_ptr)
	{
	    upCast(this->_ptr)->__incRef();
	}
    }
    
    ~Handle()
    {
	if(this->_ptr)
	{
	    upCast(this->_ptr)->__decRef();
	}
    }
    
    Handle& operator=(T* p)
    {
	if(this->_ptr != p)
	{
	    if(p)
	    {
		upCast(p)->__incRef();
	    }

	    T* ptr = this->_ptr;
	    this->_ptr = p;

	    if(ptr)
	    {
		upCast(ptr)->__decRef();
	    }
	}
	return *this;
    }
        
    template<typename Y>
    Handle& operator=(const Handle<Y>& r)
    {
	if(this->_ptr != r._ptr)
	{
	    if(r._ptr)
	    {
                upCast(r._ptr)->__incRef();
	    }

	    T* ptr = this->_ptr;
	    this->_ptr = r._ptr;

	    if(ptr)
	    {
		upCast(ptr)->__decRef();
	    }
	}
	return *this;
    }

    template<typename Y>
    Handle& operator=(const ::IceUtil::Handle<Y>& r)
    {
	if(this->_ptr != r._ptr)
	{
	    if(r._ptr)
	    {
		upCast(r._ptr)->__incRef();
	    }

	    T* ptr = this->_ptr;
	    this->_ptr = r._ptr;

	    if(ptr)
	    {
		upCast(ptr)->__decRef();
	    }
	}
	return *this;
    }

    Handle& operator=(const Handle& r)
    {
	if(this->_ptr != r._ptr)
	{
	    if(r._ptr)
	    {
		upCast(r._ptr)->__incRef();
	    }

	    T* ptr = this->_ptr;
	    this->_ptr = r._ptr;

	    if(ptr)
	    {
		upCast(ptr)->__decRef();
	    }
	}
	return *this;
    }
        
    template<class Y>
    static Handle dynamicCast(const ::IceUtil::HandleBase<Y>& r)
    {
	return Handle(dynamic_cast<T*>(r._ptr));
    }

    template<class Y>
    static Handle dynamicCast(Y* p)
    {
	return Handle(dynamic_cast<T*>(p));
    }
};

}

#endif
