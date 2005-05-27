// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_UTIL_H
#define ICE_GRID_UTIL_H

#include <IceUtil/StringUtil.h>
#include <IceGrid/Internal.h>

namespace IceGrid
{

template<typename T> std::insert_iterator<T>
inline set_inserter(T& container) 
{ 
    return std::insert_iterator<T>(container, container.begin()); 
}

template<class Function>
struct ForEachComponent : std::unary_function<ComponentDescriptorPtr&, void>
{
    ForEachComponent(Function f) : _function(f)
    {
    }

    void
    operator()(const InstanceDescriptor& instance)
    {
	operator()(instance.descriptor);
    }

    void
    operator()(const ComponentDescriptorPtr& descriptor)
    {
	_function(descriptor);
	IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(descriptor);
	if(iceBox)
	{
	    for_each(iceBox->services.begin(), iceBox->services.end(), forEachComponent(_function));
	}
    }

    Function _function;
};

template<typename Function> ForEachComponent<Function>
inline forEachComponent(Function function) 
{ 
    return ForEachComponent<Function>(function);
}

template<class T, class A>
struct ObjFunc : std::unary_function<A, void>
{
    T& _obj;
    typedef void (T::*MemberFN)(A);
    MemberFN _mfn;

public:

    explicit ObjFunc(T& obj, void (T::*f)(A)) : _obj(obj), _mfn(f) { }
    void operator()(A arg) const
    {
	(_obj.*_mfn)(arg);
    }
};

template<class T, class A> ObjFunc<T, A>
inline objFunc(T& obj, void (T::*p)(A))
{ 
    return ObjFunc<T, A>(obj, p);
}

template <class T> std::vector<std::string>
inline getMatchingKeys(const T& m, const std::string& expression)
{
    std::vector<std::string> keys;
    for(typename T::const_iterator p = m.begin(); p != m.end(); ++p)
    {
	if(expression.empty() || IceUtil::match(p->first, expression, true))
	{
	    keys.push_back(p->first);
	}
    }
    return keys;
}

};

#endif
