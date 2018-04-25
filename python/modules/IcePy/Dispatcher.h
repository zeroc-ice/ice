// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_DISPATCHER_H
#define ICEPY_DISPATCHER_H

#include <Config.h>
#include <Util.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Dispatcher.h>

namespace IcePy
{

bool initDispatcher(PyObject*);

class Dispatcher : public Ice::Dispatcher
{
public:

    Dispatcher(PyObject*);

    void setCommunicator(const Ice::CommunicatorPtr&);

    virtual void dispatch(const Ice::DispatcherCallPtr&, const Ice::ConnectionPtr&);

private:

    PyObjectHandle _dispatcher;
    Ice::CommunicatorPtr _communicator;
};
typedef IceUtil::Handle<Dispatcher> DispatcherPtr;

}

#endif
