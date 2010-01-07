// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _WIN32
#   include <IceUtil/Config.h>
#endif
#include <Endpoint.h>
#include <EndpointInfo.h>
#include <Util.h>

using namespace std;
using namespace IcePy;

namespace IcePy
{

struct EndpointObject
{
    PyObject_HEAD
    Ice::EndpointPtr* endpoint;
};

}

#ifdef WIN32
extern "C"
#endif
static EndpointObject*
endpointNew(PyObject* /*arg*/)
{
    PyErr_Format(PyExc_RuntimeError, STRCAST("An endpoint cannot be created directly"));
    return 0;
}

#ifdef WIN32
extern "C"
#endif
static void
endpointDealloc(EndpointObject* self)
{
    delete self->endpoint;
    PyObject_Del(self);
}

#ifdef WIN32
extern "C"
#endif
static int
endpointCompare(EndpointObject* p1, EndpointObject* p2)
{
    if(*p1->endpoint < *p2->endpoint)
    {
        return -1;
    }
    else if(*p1->endpoint == *p2->endpoint)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
endpointToString(EndpointObject* self)
{
    assert(self->endpoint);
    try
    {
        string str = (*self->endpoint)->toString();
        return createString(str);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
endpointRepr(EndpointObject* self)
{
    return endpointToString(self);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
endpointGetInfo(EndpointObject* self)
{
    assert(self->endpoint);
    try
    {
        Ice::EndpointInfoPtr info = (*self->endpoint)->getInfo();
        return createEndpointInfo(info);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

static PyMethodDef EndpointMethods[] =
{
    { STRCAST("toString"), reinterpret_cast<PyCFunction>(endpointToString), METH_NOARGS,
        PyDoc_STR(STRCAST("toString() -> string")) },
    { STRCAST("getInfo"), reinterpret_cast<PyCFunction>(endpointGetInfo), METH_NOARGS,
        PyDoc_STR(STRCAST("getInfo() -> Ice.EndpointInfo")) },
    { 0, 0 } /* sentinel */
};

namespace IcePy
{

PyTypeObject EndpointType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                               /* ob_size */
    STRCAST("IcePy.Endpoint"),       /* tp_name */
    sizeof(EndpointObject),          /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)endpointDealloc,     /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    (cmpfunc)endpointCompare,        /* tp_compare */
    (reprfunc)endpointRepr,          /* tp_repr */
    0,                               /* tp_as_number */
    0,                               /* tp_as_sequence */
    0,                               /* tp_as_mapping */
    0,                               /* tp_hash */
    0,                               /* tp_call */
    0,                               /* tp_str */
    0,                               /* tp_getattro */
    0,                               /* tp_setattro */
    0,                               /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    0,                               /* tp_doc */
    0,                               /* tp_traverse */
    0,                               /* tp_clear */
    0,                               /* tp_richcompare */
    0,                               /* tp_weaklistoffset */
    0,                               /* tp_iter */
    0,                               /* tp_iternext */
    EndpointMethods,                 /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    (newfunc)endpointNew,            /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

};

bool
IcePy::initEndpoint(PyObject* module)
{
    if(PyType_Ready(&EndpointType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &EndpointType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("Endpoint"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

Ice::EndpointPtr
IcePy::getEndpoint(PyObject* obj)
{
    assert(PyObject_IsInstance(obj, reinterpret_cast<PyObject*>(&EndpointType)));
    EndpointObject* eobj = reinterpret_cast<EndpointObject*>(obj);
    return *eobj->endpoint;
}

PyObject*
IcePy::createEndpoint(const Ice::EndpointPtr& endpoint)
{
    EndpointObject* obj = PyObject_New(EndpointObject, &EndpointType);
    if(!obj)
    {
        return 0;
    }
    obj->endpoint = new Ice::EndpointPtr(endpoint);
    return (PyObject*)obj;
}
