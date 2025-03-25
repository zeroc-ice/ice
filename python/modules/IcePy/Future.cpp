// Copyright (c) ZeroC, Inc.

#include "Future.h"
#include "Communicator.h"
#include "Util.h"

PyObject*
IcePy::wrapFuture(const Ice::CommunicatorPtr& communicator, PyObject* future)
{
    PyObjectHandle communicatorHandle{IcePy::getCommunicatorWrapper(communicator)};
    assert(communicatorHandle.get());
    return wrapFuture(communicatorHandle.get(), future);
}

PyObject*
IcePy::wrapFuture(PyObject* communicator, PyObject* future)
{
    PyObjectHandle futureHandle{Py_NewRef(future)};
    PyObjectHandle eventLoopAdapter{getAttr(communicator, "eventLoopAdapter", false)};
    if (eventLoopAdapter.get())
    {
        PyObjectHandle wrappedFuture{callMethod(eventLoopAdapter.get(), "wrapFuture", futureHandle.get())};
        if (PyErr_Occurred())
        {
            return nullptr;
        }
        return wrappedFuture.release();
    }
    return futureHandle.release();
}
