// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_USER_EXCEPTION_FACTORY_MANAGER_F_H
#define ICE_USER_EXCEPTION_FACTORY_MANAGER_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class UserExceptionFactoryManager;
void incRef(UserExceptionFactoryManager*);
void decRef(UserExceptionFactoryManager*);
typedef IceInternal::Handle<UserExceptionFactoryManager> UserExceptionFactoryManagerPtr;

}

#endif
