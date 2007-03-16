// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/OperationMode.h>
#include <IceE/BasicStream.h>

void
Ice::__write(::IceInternal::BasicStream* __os, ::Ice::OperationMode v)
{
    __os->write(static_cast< ::Ice::Byte>(v));
}

void
Ice::__read(::IceInternal::BasicStream* __is, ::Ice::OperationMode& v)
{
    ::Ice::Byte val;
    __is->read(val);
    v = static_cast< ::Ice::OperationMode>(val);
}
