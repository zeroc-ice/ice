// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/OperationMode.h>
#include <IceE/BasicStream.h>

void
IceE::__write(::IceEInternal::BasicStream* __os, ::IceE::OperationMode v)
{
    __os->write(static_cast< ::IceE::Byte>(v));
}

void
IceE::__read(::IceEInternal::BasicStream* __is, ::IceE::OperationMode& v)
{
    ::IceE::Byte val;
    __is->read(val);
    v = static_cast< ::IceE::OperationMode>(val);
}
