// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_OPERATION_H
#define ICEPY_OPERATION_H

#include <Python.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Current.h>

namespace IcePy
{

class Operation : public IceUtil::Shared
{
public:

    virtual ~Operation();

    virtual PyObject* invoke(const Ice::ObjectPrx&, const Ice::CommunicatorPtr&, PyObject*) = 0;
    virtual bool dispatch(PyObject*, const std::vector<Ice::Byte>&, std::vector<Ice::Byte>&, const Ice::Current&) = 0;
};
typedef IceUtil::Handle<Operation> OperationPtr;

OperationPtr getOperation(const std::string&, const std::string&);

}

#endif
