// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_OPERATION_H
#define ICEPY_OPERATION_H

#include <Config.h>
#include <Ice/Current.h>
#include <Ice/Object.h>
#include <Ice/AsyncResultF.h>
#include <Ice/CommunicatorF.h>

namespace IcePy
{

bool initOperation(PyObject*);

//
// Builtin operations.
//
PyObject* invokeBuiltin(PyObject*, const std::string&, PyObject*);
PyObject* beginBuiltin(PyObject*, const std::string&, PyObject*);
PyObject* endBuiltin(PyObject*, const std::string&, PyObject*);

//
// Blobject invocations.
//
PyObject* iceInvoke(PyObject*, PyObject*);
PyObject* iceInvokeAsync(PyObject*, PyObject*);
PyObject* beginIceInvoke(PyObject*, PyObject*, PyObject*);
PyObject* endIceInvoke(PyObject*, PyObject*);

extern PyTypeObject AsyncResultType;
PyObject* createAsyncResult(const Ice::AsyncResultPtr&, PyObject*, PyObject*, PyObject*);
Ice::AsyncResultPtr getAsyncResult(PyObject*);

//
// Used as the callback for getConnection operation.
//
class GetConnectionCallback : public IceUtil::Shared
{
public:

    GetConnectionCallback(const Ice::CommunicatorPtr&, PyObject*, PyObject*, const std::string&);
    ~GetConnectionCallback();

    void response(const Ice::ConnectionPtr&);
    void exception(const Ice::Exception&);

protected:

    Ice::CommunicatorPtr _communicator;
    PyObject* _response;
    PyObject* _ex;
    std::string _op;
};
typedef IceUtil::Handle<GetConnectionCallback> GetConnectionCallbackPtr;

//
// Used as the callback for the various flushBatchRequest operations.
//
class FlushCallback : public IceUtil::Shared
{
public:

    FlushCallback(PyObject*, PyObject*, const std::string&);
    ~FlushCallback();

    void exception(const Ice::Exception&);
    void sent(bool);

protected:

    PyObject* _ex;
    PyObject* _sent;
    std::string _op;
};
typedef IceUtil::Handle<FlushCallback> FlushCallbackPtr;

//
// ServantWrapper handles dispatching to a Python servant.
//
class ServantWrapper : public Ice::BlobjectArrayAsync
{
public:

    ServantWrapper(PyObject*);
    ~ServantWrapper();

    PyObject* getObject();

protected:

    PyObject* _servant;
};
typedef IceUtil::Handle<ServantWrapper> ServantWrapperPtr;

ServantWrapperPtr createServantWrapper(PyObject*);

}

#endif
