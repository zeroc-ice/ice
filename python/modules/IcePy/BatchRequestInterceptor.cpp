// Copyright (c) ZeroC, Inc.

#include "BatchRequestInterceptor.h"
#include "Proxy.h"
#include "Thread.h"

using namespace std;
using namespace IcePy;

namespace IcePy
{
    struct BatchRequestObject
    {
        PyObject_HEAD const Ice::BatchRequest* request;
        PyObject* size;
        PyObject* operation;
        PyObject* proxy;
    };
}

extern "C" BatchRequestObject*
batchRequestNew(PyTypeObject* /*type*/, PyObject* /*args*/, PyObject* /*kwds*/)
{
    PyErr_Format(PyExc_RuntimeError, "Batch requests can only be created by the Ice runtime");
    return nullptr;
}

extern "C" void
batchRequestDealloc(BatchRequestObject* self)
{
    Py_XDECREF(self->size);
    Py_XDECREF(self->operation);
    Py_XDECREF(self->proxy);
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

extern "C" PyObject*
batchRequestGetSize(BatchRequestObject* self, PyObject* /*args*/)
{
    assert(self->request);
    if (!self->size)
    {
        int32_t size;
        try
        {
            size = self->request->getSize();
        }
        catch (...)
        {
            setPythonException(current_exception());
            return nullptr;
        }

        self->size = PyLong_FromLong(size);
    }
    return Py_NewRef(self->size);
}

extern "C" PyObject*
batchRequestGetOperation(BatchRequestObject* self, PyObject* /*args*/)
{
    assert(self->request);
    if (!self->operation)
    {
        string operation;
        try
        {
            operation = self->request->getOperation();
        }
        catch (...)
        {
            setPythonException(current_exception());
            return nullptr;
        }

        self->operation = createString(operation);
    }
    return Py_NewRef(self->operation);
}

extern "C" PyObject*
batchRequestGetProxy(BatchRequestObject* self, PyObject* /*args*/)
{
    assert(self->request);
    if (!self->proxy)
    {
        optional<Ice::ObjectPrx> proxy;
        try
        {
            proxy = self->request->getProxy();
            assert(proxy);
        }
        catch (...)
        {
            setPythonException(current_exception());
            return nullptr;
        }

        self->proxy = createProxy(proxy.value(), proxy->ice_getCommunicator());
    }
    return Py_NewRef(self->proxy);
}

extern "C" PyObject*
batchRequestEnqueue(BatchRequestObject* self, PyObject* /*args*/)
{
    assert(self->request);

    try
    {
        self->request->enqueue();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

static PyMethodDef BatchRequestMethods[] = {
    {"getSize", reinterpret_cast<PyCFunction>(batchRequestGetSize), METH_NOARGS, PyDoc_STR("getSize() -> int")},
    {"getOperation",
     reinterpret_cast<PyCFunction>(batchRequestGetOperation),
     METH_NOARGS,
     PyDoc_STR("getOperation() -> string")},
    {"getProxy",
     reinterpret_cast<PyCFunction>(batchRequestGetProxy),
     METH_NOARGS,
     PyDoc_STR("getProxy() -> Ice.ObjectPrx")},
    {"enqueue", reinterpret_cast<PyCFunction>(batchRequestEnqueue), METH_NOARGS, PyDoc_STR("enqueue() -> None")},
    {} /* sentinel */
};

namespace IcePy
{
    // clang-format off
    PyTypeObject BatchRequestType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.BatchRequest",
        .tp_basicsize = sizeof(BatchRequestObject),
        .tp_dealloc = (destructor)batchRequestDealloc,
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_methods = BatchRequestMethods,
        .tp_new = (newfunc)batchRequestNew,
    };
    // clang-format on
}

bool
IcePy::initBatchRequest(PyObject* module)
{
    if (PyType_Ready(&BatchRequestType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "BatchRequest", reinterpret_cast<PyObject*>(&BatchRequestType)) < 0)
    {
        return false;
    }

    return true;
}

IcePy::BatchRequestInterceptorWrapper::BatchRequestInterceptorWrapper(PyObject* interceptor)
    : _interceptor(Py_NewRef(interceptor))
{
    if (!PyCallable_Check(interceptor))
    {
        throw Ice::InitializationException{__FILE__, __LINE__, "the batch request interceptor must be a callable object"};
    }
}

void
IcePy::BatchRequestInterceptorWrapper::enqueue(const Ice::BatchRequest& request, int queueCount, int queueSize)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    auto* obj = reinterpret_cast<BatchRequestObject*>(BatchRequestType.tp_alloc(&BatchRequestType, 0));
    if (!obj)
    {
        return;
    }

    obj->request = &request;
    obj->size = nullptr;
    obj->operation = nullptr;
    obj->proxy = nullptr;
    PyObjectHandle tmp{PyObject_CallFunction(_interceptor.get(), "Oii", obj, queueCount, queueSize)};
    Py_DECREF(reinterpret_cast<PyObject*>(obj));
    if (!tmp.get())
    {
        throwPythonException();
    }
}
