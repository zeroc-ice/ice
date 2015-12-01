// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_VALUE_F_H
#define ICE_VALUE_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>
#include <Ice/ObjectF.h>

#ifdef ICE_CPP11_MAPPING // C++11 mapping
namespace Ice
{

class Value;
typedef ::std::shared_ptr< ::Ice::Value> ValuePtr;
ICE_API void __patch(ValuePtr&, const ValuePtr&);

}
#else // C++98 mapping
//
// Define value as an alias to Object
//
//typedef ::Ice::Object Value;
//typedef ::Ice::ObjectPtr ValuePtr;
#endif

#endif
