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

extern "C"
{
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
}

#include <Ice/Ice.h>
#include <Slice/Parser.h>

class Marshaler;
typedef IceUtil::Handle<Marshaler> MarshalerPtr;

class Marshaler : public IceUtil::SimpleShared
{
public:
    virtual ~Marshaler();

    static MarshalerPtr createMarshaler(const Slice::TypePtr&);
    static MarshalerPtr createMemberMarshaler(const std::string&, const Slice::TypePtr&);

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC) = 0;
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC) = 0;

    static std::string zendTypeToString(int);

protected:
    Marshaler();
};

#endif
