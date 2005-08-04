// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
    ::Ice::ObjectAdapterPtr adapter;
    ::Ice::ConnectionPtr con;
    ::Ice::Identity id;
    ::std::string facet;
    ::std::string operation;
    ::Ice::OperationMode mode;
    ::Ice::Context ctx;

    ICEE_API bool operator==(const Current&) const;
    ICEE_API bool operator!=(const Current&) const;
    ICEE_API bool operator<(const Current&) const;
};

}

#endif
