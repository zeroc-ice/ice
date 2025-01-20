// Copyright (c) ZeroC, Inc.

#ifndef ICEPY_OPERATION_H
#define ICEPY_OPERATION_H

#include "Config.h"
#include "Ice/CommunicatorF.h"
#include "Ice/Current.h"
#include "Ice/Object.h"
#include "Util.h"

#include <functional>

namespace IcePy
{
    bool initOperation(PyObject*);

    extern PyTypeObject AsyncInvocationContextType;

    // Builtin operations.
    PyObject* invokeBuiltin(PyObject*, const std::string&, PyObject*);
    PyObject* invokeBuiltinAsync(PyObject*, const std::string&, PyObject*);

    // Blobject invocations.
    PyObject* iceInvoke(PyObject*, PyObject*);
    PyObject* iceInvokeAsync(PyObject*, PyObject*);

    // Used as the callback for getConnectionAsync operation.
    class GetConnectionAsyncCallback
    {
    public:
        GetConnectionAsyncCallback(const Ice::CommunicatorPtr&, const std::string&);
        ~GetConnectionAsyncCallback();

        void setFuture(PyObject*);

        void response(const Ice::ConnectionPtr&);
        void exception(std::exception_ptr);

    protected:
        Ice::CommunicatorPtr _communicator;
        std::string _op;
        PyObject* _future;
        Ice::ConnectionPtr _connection;
        PyObject* _exception;
    };
    using GetConnectionAsyncCallbackPtr = std::shared_ptr<GetConnectionAsyncCallback>;

    // Used as the callback for the various flushBatchRequestAsync operations.
    class FlushAsyncCallback
    {
    public:
        FlushAsyncCallback(const std::string&);
        ~FlushAsyncCallback();

        void setFuture(PyObject*);

        void exception(std::exception_ptr);
        void sent(bool);

    protected:
        std::string _op;
        PyObject* _future;
        bool _sent;
        bool _sentSynchronously;
        PyObject* _exception;
    };
    using FlushAsyncCallbackPtr = std::shared_ptr<FlushAsyncCallback>;

    // ServantWrapper handles dispatching to a Python servant.
    class ServantWrapper : public Ice::BlobjectArrayAsync
    {
    public:
        ServantWrapper(PyObject*);
        ~ServantWrapper();

        PyObject* getObject();

    protected:
        PyObject* _servant;
    };
    using ServantWrapperPtr = std::shared_ptr<ServantWrapper>;

    ServantWrapperPtr createServantWrapper(PyObject*);

    PyObject* createAsyncInvocationContext(std::function<void()>, Ice::CommunicatorPtr);
    PyObject* createFuture(const std::string&, PyObject*);
}

#endif
