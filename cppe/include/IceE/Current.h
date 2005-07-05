// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_CURRENT_H
#define ICEE_CURRENT_H

#include <IceE/ObjectAdapterF.h>
#include <IceE/ConnectionF.h>
#include <IceE/Identity.h>
#include <IceE/OperationMode.h>

namespace IceE
{

struct Current
{
    ::IceE::ObjectAdapterPtr adapter;
    ::IceE::ConnectionPtr con;
    ::IceE::Identity id;
    ::std::string facet;
    ::std::string operation;
    ::IceE::OperationMode mode;
    ::IceE::Context ctx;

    ICEE_API bool operator==(const Current&) const;
    ICEE_API bool operator!=(const Current&) const;
    ICEE_API bool operator<(const Current&) const;
};

}

#endif
