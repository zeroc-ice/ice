// Copyright (c) ZeroC, Inc.

#ifndef ICEPY_FUTURE_H
#define ICEPY_FUTURE_H

#include "Config.h"
#include <Ice/Communicator.h>

namespace IcePy
{
    /// Wraps an Ice.Future object in a custom future object, using the communicator's event loop adapter. If the event
    /// loop adapter is Py_None the future is returned as is.
    ///
    /// @param communicator The communicator.
    /// @param future The future to wrap.
    /// @return The wrapped future. This method always returns a new reference.
    PyObject* wrapFuture(const Ice::CommunicatorPtr& communicator, PyObject* future);

    /// Wraps an Ice.Future object in a custom future object, using the communicator's event loop adapter. If the event
    /// loop adapter is Py_None the future is returned as is.
    ///
    /// @param communicator The communicator.
    /// @param future The future to wrap.
    /// @return The wrapped future. This method always returns a new reference.
    PyObject* wrapFuture(PyObject* communicator, PyObject* future);
}

#endif
