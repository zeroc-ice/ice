// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_OPERATION_H
#define ICEPY_OPERATION_H

#include <Config.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Current.h>
#include <Ice/Object.h>

namespace IcePy
{

class Operation : public IceUtil::Shared
{
public:

    virtual ~Operation();

    virtual PyObject* invoke(const Ice::ObjectPrx&, PyObject*, PyObject*) = 0;
    virtual PyObject* invokeAsync(const Ice::ObjectPrx&, PyObject*, PyObject*, PyObject*) = 0;
    virtual void deprecate(const std::string&) = 0;
    virtual Ice::OperationMode mode() const = 0;

    virtual void dispatch(PyObject*, const Ice::AMD_Object_ice_invokePtr&, const std::vector<Ice::Byte>&,
                          const Ice::Current&) = 0;
};
typedef IceUtil::Handle<Operation> OperationPtr;

bool initOperation(PyObject*);

OperationPtr getOperation(PyObject*);

}

#endif
