//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
