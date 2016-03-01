// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _WIN32
#   include <IceUtil/Config.h>
#endif
#include <BatchRequestInterceptor.h>
#include <Proxy.h>
#include <Thread.h>
#include <Ice/Initialize.h>

using namespace std;
using namespace IcePy;

namespace IcePy
{

struct BatchRequestObject
{
    PyObject_HEAD
    const Ice::BatchRequest* request;
    PyObject* size;
    PyObject* operation;
    PyObject* proxy;
};

}

#ifdef WIN32
extern "C"
#endif
static BatchRequestObject*
batchRequestNew(PyTypeObject* /*type*/, PyObject* /*args*/, PyObject* /*kwds*/)
{
    PyErr_Format(PyExc_RuntimeError, STRCAST("Batch requests can only be created by the Ice runtime"));
    return 0;
}

#ifdef WIN32
extern "C"
#endif
static void
batchRequestDealloc(BatchRequestObject* self)
{
    Py_XDECREF(self->size);
    Py_XDECREF(self->operation);
    Py_XDECREF(self->proxy);
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
batchRequestGetSize(BatchRequestObject* self)
{
    assert(self->request);
    if(!self->size)
    {
        Ice::Int size;
        try
        {
            size = self->request->getSize();
        }
        catch(const Ice::Exception& ex)
        {
            setPythonException(ex);
            return 0;
        }

        self->size = PyLong_FromLong(size);
    }
    Py_INCREF(self->size);
    return self->size;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
batchRequestGetOperation(BatchRequestObject* self)
{
    assert(self->request);
    if(!self->operation)
    {
        string operation;
        try
        {
            operation = self->request->getOperation();
        }
        catch(const Ice::Exception& ex)
        {
            setPythonException(ex);
            return 0;
        }

        self->operation = createString(operation);
    }
    Py_INCREF(self->operation);
    return self->operation;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
batchRequestGetProxy(BatchRequestObject* self)
{
    assert(self->request);
    if(!self->proxy)
    {
        Ice::ObjectPrx proxy;
        try
        {
            proxy = self->request->getProxy();
        }
        catch(const Ice::Exception& ex)
        {
            setPythonException(ex);
            return 0;
        }

        self->proxy = createProxy(proxy, proxy->ice_getCommunicator());
    }
    Py_INCREF(self->proxy);
    return self->proxy;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
batchRequestEnqueue(BatchRequestObject* self)
{
    assert(self->request);

    try
    {
        self->request->enqueue();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef BatchRequestMethods[] =
{
    { STRCAST("getSize"), reinterpret_cast<PyCFunction>(batchRequestGetSize), METH_NOARGS,
        PyDoc_STR(STRCAST("getSize() -> int")) },
    { STRCAST("getOperation"), reinterpret_cast<PyCFunction>(batchRequestGetOperation), METH_NOARGS,
        PyDoc_STR(STRCAST("getOperation() -> string")) },
    { STRCAST("getProxy"), reinterpret_cast<PyCFunction>(batchRequestGetProxy), METH_NOARGS,
        PyDoc_STR(STRCAST("getProxy() -> Ice.ObjectPrx")) },
    { STRCAST("enqueue"), reinterpret_cast<PyCFunction>(batchRequestEnqueue), METH_NOARGS,
        PyDoc_STR(STRCAST("enqueue() -> None")) },
    { 0, 0 } /* sentinel */
};

namespace IcePy
{

PyTypeObject BatchRequestType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.BatchRequest"),        /* tp_name */
    sizeof(BatchRequestObject),           /* tp_basicsize */
    0,                                    /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(batchRequestDealloc), /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_reserved */
    0,                               /* tp_repr */
    0,                               /* tp_as_number */
    0,                               /* tp_as_sequence */
    0,                               /* tp_as_mapping */
    0,                               /* tp_hash */
    0,                               /* tp_call */
    0,                               /* tp_str */
    0,                               /* tp_getattro */
    0,                               /* tp_setattro */
    0,                               /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,              /* tp_flags */
    0,                               /* tp_doc */
    0,                               /* tp_traverse */
    0,                               /* tp_clear */
    0,                               /* tp_richcompare */
    0,                               /* tp_weaklistoffset */
    0,                               /* tp_iter */
    0,                               /* tp_iternext */
    BatchRequestMethods,             /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    reinterpret_cast<newfunc>(batchRequestNew), /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

}

bool
IcePy::initBatchRequest(PyObject* module)
{
    if(PyType_Ready(&BatchRequestType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &BatchRequestType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("BatchRequest"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

IcePy::BatchRequestInterceptor::BatchRequestInterceptor(PyObject* interceptor) : _interceptor(interceptor)
{
    Py_INCREF(interceptor);
}

void
IcePy::BatchRequestInterceptor::enqueue(const Ice::BatchRequest& request, int queueCount, int queueSize)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    BatchRequestObject* obj = reinterpret_cast<BatchRequestObject*>(BatchRequestType.tp_alloc(&BatchRequestType, 0));
    if(!obj)
    {
        return;
    }

    obj->request = &request;
    obj->size = 0;
    obj->operation = 0;
    obj->proxy = 0;
    PyObjectHandle tmp = PyObject_CallMethod(_interceptor.get(), STRCAST("enqueue"), STRCAST("Oii"), obj, queueCount,
                                             queueSize);
    if(!tmp.get())
    {
        throwPythonException();
    }
}
