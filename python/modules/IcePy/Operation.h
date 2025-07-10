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

    // Blobject invocations.
    PyObject* iceInvoke(PyObject*, PyObject*);
    PyObject* iceInvokeAsync(PyObject*, PyObject*);

    // Used as the callback for getConnectionAsync operation.
    class GetConnectionAsyncCallback
    {
    public:
        GetConnectionAsyncCallback(Ice::CommunicatorPtr, std::string);
        ~GetConnectionAsyncCallback();

        void setFuture(PyObject*);

        void response(const Ice::ConnectionPtr&);
        void exception(std::exception_ptr);

    protected:
        Ice::CommunicatorPtr _communicator;
        std::string _op;
        PyObject* _future{nullptr};
        Ice::ConnectionPtr _connection;
        PyObject* _exception{nullptr};
    };
    using GetConnectionAsyncCallbackPtr = std::shared_ptr<GetConnectionAsyncCallback>;

    // Used as the callback for the various flushBatchRequestAsync operations.
    class FlushAsyncCallback
    {
    public:
        FlushAsyncCallback(std::string);
        ~FlushAsyncCallback();

        void setFuture(PyObject*);

        void exception(std::exception_ptr);
        void sent(bool);

    protected:
        std::string _op;
        PyObject* _future{nullptr};
        bool _sent{false};
        bool _sentSynchronously{false};
        PyObject* _exception{nullptr};
    };
    using FlushAsyncCallbackPtr = std::shared_ptr<FlushAsyncCallback>;

    // ServantWrapper handles dispatching to a Python servant.
    class ServantWrapper : public Ice::BlobjectArrayAsync
    {
    public:
        ServantWrapper(PyObject*);
        ~ServantWrapper() override;

        PyObject* getObject();

    protected:
        PyObject* _servant;
    };
    using ServantWrapperPtr = std::shared_ptr<ServantWrapper>;

    ServantWrapperPtr createServantWrapper(PyObject*);

    PyObject* createAsyncInvocationContext(std::function<void()>, Ice::CommunicatorPtr);
    PyObject* createFuture(PyObject*);
}

#endif
