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
endpointNew(PyTypeObject* /*type*/, PyObject* /*args*/, PyObject* /*kwds*/)
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
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
endpointCompare(EndpointObject* p1, PyObject* other, int op)
{
    bool result = false;

    if(PyObject_TypeCheck(other, &EndpointType))
    {
        EndpointObject* p2 = reinterpret_cast<EndpointObject*>(other);

        switch(op)
        {
        case Py_EQ:
            result = *p1->endpoint == *p2->endpoint;
            break;
        case Py_NE:
            result = *p1->endpoint != *p2->endpoint;
            break;
        case Py_LE:
            result = *p1->endpoint <= *p2->endpoint;
            break;
        case Py_GE:
            result = *p1->endpoint >= *p2->endpoint;
            break;
        case Py_LT:
            result = *p1->endpoint < *p2->endpoint;
            break;
        case Py_GT:
            result = *p1->endpoint > *p2->endpoint;
            break;
        }
    }
    else
    {
        if(op == Py_EQ)
        {
            result = false;
        }
        else if(op == Py_NE)
        {
            result = true;
        }
        else
        {
            PyErr_Format(PyExc_TypeError, "can't compare %s to %s", Py_TYPE(p1)->tp_name, Py_TYPE(other)->tp_name);
            return 0;
        }
    }

    PyObject* r = result ? getTrue() : getFalse();
    Py_INCREF(r);
    return r;
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
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.Endpoint"),       /* tp_name */
    sizeof(EndpointObject),          /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(endpointDealloc), /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_reserved */
    reinterpret_cast<reprfunc>(endpointRepr), /* tp_repr */
    0,                               /* tp_as_number */
    0,                               /* tp_as_sequence */
    0,                               /* tp_as_mapping */
    0,                               /* tp_hash */
    0,                               /* tp_call */
    0,                               /* tp_str */
    0,                               /* tp_getattro */
    0,                               /* tp_setattro */
    0,                               /* tp_as_buffer */
#if PY_VERSION_HEX >= 0x03000000
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,             /* tp_flags */
#else
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE |
    Py_TPFLAGS_HAVE_RICHCOMPARE,     /* tp_flags */
#endif
    0,                               /* tp_doc */
    0,                               /* tp_traverse */
    0,                               /* tp_clear */
    reinterpret_cast<richcmpfunc>(endpointCompare), /* tp_richcompare */
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
    reinterpret_cast<newfunc>(endpointNew), /* tp_new */
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
    EndpointObject* obj = reinterpret_cast<EndpointObject*>(EndpointType.tp_alloc(&EndpointType, 0));
    if(!obj)
    {
        return 0;
    }
    obj->endpoint = new Ice::EndpointPtr(endpoint);
    return (PyObject*)obj;
}
