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
    Py_INCREF(self->size);
    return self->size;
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
    Py_INCREF(self->operation);
    return self->operation;
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
    Py_INCREF(self->proxy);
    return self->proxy;
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
    {0, 0} /* sentinel */
};

namespace IcePy
{
    PyTypeObject BatchRequestType = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(0, 0) "IcePy.BatchRequest", /* tp_name */
        sizeof(BatchRequestObject),                       /* tp_basicsize */
        0,                                                /* tp_itemsize */
        /* methods */
        reinterpret_cast<destructor>(batchRequestDealloc), /* tp_dealloc */
        0,                                                 /* tp_print */
        0,                                                 /* tp_getattr */
        0,                                                 /* tp_setattr */
        0,                                                 /* tp_reserved */
        0,                                                 /* tp_repr */
        0,                                                 /* tp_as_number */
        0,                                                 /* tp_as_sequence */
        0,                                                 /* tp_as_mapping */
        0,                                                 /* tp_hash */
        0,                                                 /* tp_call */
        0,                                                 /* tp_str */
        0,                                                 /* tp_getattro */
        0,                                                 /* tp_setattro */
        0,                                                 /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                                /* tp_flags */
        0,                                                 /* tp_doc */
        0,                                                 /* tp_traverse */
        0,                                                 /* tp_clear */
        0,                                                 /* tp_richcompare */
        0,                                                 /* tp_weaklistoffset */
        0,                                                 /* tp_iter */
        0,                                                 /* tp_iternext */
        BatchRequestMethods,                               /* tp_methods */
        0,                                                 /* tp_members */
        0,                                                 /* tp_getset */
        0,                                                 /* tp_base */
        0,                                                 /* tp_dict */
        0,                                                 /* tp_descr_get */
        0,                                                 /* tp_descr_set */
        0,                                                 /* tp_dictoffset */
        0,                                                 /* tp_init */
        0,                                                 /* tp_alloc */
        reinterpret_cast<newfunc>(batchRequestNew),        /* tp_new */
        0,                                                 /* tp_free */
        0,                                                 /* tp_is_gc */
    };
}

bool
IcePy::initBatchRequest(PyObject* module)
{
    if (PyType_Ready(&BatchRequestType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &BatchRequestType; // Necessary to prevent GCC's strict-alias warnings.
    if (PyModule_AddObject(module, "BatchRequest", reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

IcePy::BatchRequestInterceptorWrapper::BatchRequestInterceptorWrapper(PyObject* interceptor) : _interceptor(interceptor)
{
    if (!PyCallable_Check(interceptor) && !PyObject_HasAttrString(interceptor, "enqueue"))
    {
        throw Ice::InitializationException(
            __FILE__,
            __LINE__,
            "batch request interceptor must either be a callable or an object with an 'enqueue' method");
    }

    Py_INCREF(interceptor);
}

void
IcePy::BatchRequestInterceptorWrapper::enqueue(const Ice::BatchRequest& request, int queueCount, int queueSize)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    BatchRequestObject* obj = reinterpret_cast<BatchRequestObject*>(BatchRequestType.tp_alloc(&BatchRequestType, 0));
    if (!obj)
    {
        return;
    }

    obj->request = &request;
    obj->size = 0;
    obj->operation = 0;
    obj->proxy = 0;
    PyObjectHandle tmp;
    if (PyCallable_Check(_interceptor.get()))
    {
        tmp = PyObject_CallFunction(_interceptor.get(), "Oii", obj, queueCount, queueSize);
    }
    else
    {
        tmp = PyObject_CallMethod(_interceptor.get(), "enqueue", "Oii", obj, queueCount, queueSize);
    }
    Py_DECREF(reinterpret_cast<PyObject*>(obj));
    if (!tmp.get())
    {
        throwPythonException();
    }
}
