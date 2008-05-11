// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_CURRENT_H
#define ICEE_CURRENT_H

#include <IceE/ObjectAdapterF.h>
#include <IceE/ConnectionF.h>
#include <IceE/Identity.h>
#include <IceE/OperationMode.h>

namespace Ice
{

struct Current
{
    ::Ice::ObjectAdapter* adapter;
    ::Ice::Connection* con;
    ::Ice::Identity id;
    ::std::string facet;
    ::std::string operation;
    ::Ice::OperationMode mode;
    ::Ice::Context ctx;
    ::Ice::Int requestId;

    ICE_API bool operator==(const Current&) const;
    ICE_API bool operator!=(const Current&) const;
    ICE_API bool operator<(const Current&) const;
};

}

#endif
