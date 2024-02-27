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

class Dispatcher final
{
public:

    Dispatcher(PyObject*);

    void setCommunicator(const Ice::CommunicatorPtr&);

    void dispatch(std::function<void()> call, const Ice::ConnectionPtr&);

private:

    PyObjectHandle _dispatcher;
    Ice::CommunicatorPtr _communicator;
};
using DispatcherPtr = std::shared_ptr<Dispatcher>;

}

#endif
