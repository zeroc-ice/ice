// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_PHP_MARSHAL_H
#define ICE_PHP_MARSHAL_H

#include "common.h"
#include <Slice/Parser.h>

void Marshal_preOperation(TSRMLS_D);
void Marshal_postOperation(TSRMLS_D);
void Marshal_registerFactories(const Ice::CommunicatorPtr& TSRMLS_DC);

class Marshaler;
typedef IceUtil::Handle<Marshaler> MarshalerPtr;

class Marshaler : public IceUtil::SimpleShared
{
public:
    virtual ~Marshaler();

    static MarshalerPtr createMarshaler(const Slice::TypePtr& TSRMLS_DC);
    static MarshalerPtr createMemberMarshaler(const std::string&, const Slice::TypePtr& TSRMLS_DC);
    static MarshalerPtr createExceptionMarshaler(const Slice::ExceptionPtr& TSRMLS_DC);

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC) = 0;
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC) = 0;

    virtual void destroy() = 0;

protected:
    Marshaler();
};

#endif
