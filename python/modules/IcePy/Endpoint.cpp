// Copyright (c) ZeroC, Inc.

#include "Endpoint.h"
#include "EndpointInfo.h"
#include "Util.h"

using namespace std;
using namespace IcePy;

namespace IcePy
{
    struct EndpointObject
    {
        PyObject_HEAD Ice::EndpointPtr* endpoint;
    };
}

extern "C" EndpointObject*
endpointNew(PyTypeObject* /*type*/, PyObject* /*args*/, PyObject* /*kwds*/)
{
    PyErr_Format(PyExc_RuntimeError, "An endpoint cannot be created directly");
    return nullptr;
}

extern "C" void
endpointDealloc(EndpointObject* self)
{
    delete self->endpoint;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

extern "C" PyObject*
endpointCompare(EndpointObject* p1, PyObject* other, int op)
{
    bool result = false;

    if (PyObject_TypeCheck(other, &EndpointType))
    {
        EndpointObject* p2 = reinterpret_cast<EndpointObject*>(other);

        switch (op)
        {
            case Py_EQ:
                result = Ice::targetEqualTo(*p1->endpoint, *p2->endpoint);
                break;
            case Py_NE:
                result = !Ice::targetEqualTo(*p1->endpoint, *p2->endpoint);
                break;
            case Py_LE:
                result = Ice::targetLessEqual(*p1->endpoint, *p2->endpoint);
                break;
            case Py_GE:
                result = Ice::targetGreaterEqual(*p1->endpoint, *p2->endpoint);
                break;
            case Py_LT:
                result = Ice::targetLess(*p1->endpoint, *p2->endpoint);
                break;
            case Py_GT:
                result = Ice::targetGreater(*p1->endpoint, *p2->endpoint);
                break;
        }
    }
    else
    {
        if (op == Py_EQ)
        {
            result = false;
        }
        else if (op == Py_NE)
        {
            result = true;
        }
        else
        {
            PyErr_Format(PyExc_TypeError, "can't compare %s to %s", Py_TYPE(p1)->tp_name, Py_TYPE(other)->tp_name);
            return nullptr;
        }
    }

    return result ? Py_True : Py_False;
}

extern "C" PyObject*
endpointToString(EndpointObject* self, PyObject* /*args*/)
{
    assert(self->endpoint);
    try
    {
        string str = (*self->endpoint)->toString();
        return createString(str);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

extern "C" PyObject*
endpointRepr(EndpointObject* self)
{
    return endpointToString(self, 0);
}

extern "C" PyObject*
endpointGetInfo(EndpointObject* self, PyObject* /*args*/)
{
    assert(self->endpoint);
    try
    {
        Ice::EndpointInfoPtr info = (*self->endpoint)->getInfo();
        return createEndpointInfo(info);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

static PyMethodDef EndpointMethods[] = {
    {"toString", reinterpret_cast<PyCFunction>(endpointToString), METH_NOARGS, PyDoc_STR("toString() -> string")},
    {"getInfo",
     reinterpret_cast<PyCFunction>(endpointGetInfo),
     METH_NOARGS,
     PyDoc_STR("getInfo() -> Ice.EndpointInfo")},
    {0, 0} /* sentinel */
};

namespace IcePy
{
    PyTypeObject EndpointType = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(0, 0) "IcePy.Endpoint", /* tp_name */
        sizeof(EndpointObject),                       /* tp_basicsize */
        0,                                            /* tp_itemsize */
        /* methods */
        reinterpret_cast<destructor>(endpointDealloc),  /* tp_dealloc */
        0,                                              /* tp_print */
        0,                                              /* tp_getattr */
        0,                                              /* tp_setattr */
        0,                                              /* tp_reserved */
        reinterpret_cast<reprfunc>(endpointRepr),       /* tp_repr */
        0,                                              /* tp_as_number */
        0,                                              /* tp_as_sequence */
        0,                                              /* tp_as_mapping */
        0,                                              /* tp_hash */
        0,                                              /* tp_call */
        0,                                              /* tp_str */
        0,                                              /* tp_getattro */
        0,                                              /* tp_setattro */
        0,                                              /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,       /* tp_flags */
        0,                                              /* tp_doc */
        0,                                              /* tp_traverse */
        0,                                              /* tp_clear */
        reinterpret_cast<richcmpfunc>(endpointCompare), /* tp_richcompare */
        0,                                              /* tp_weaklistoffset */
        0,                                              /* tp_iter */
        0,                                              /* tp_iternext */
        EndpointMethods,                                /* tp_methods */
        0,                                              /* tp_members */
        0,                                              /* tp_getset */
        0,                                              /* tp_base */
        0,                                              /* tp_dict */
        0,                                              /* tp_descr_get */
        0,                                              /* tp_descr_set */
        0,                                              /* tp_dictoffset */
        0,                                              /* tp_init */
        0,                                              /* tp_alloc */
        reinterpret_cast<newfunc>(endpointNew),         /* tp_new */
        0,                                              /* tp_free */
        0,                                              /* tp_is_gc */
    };

};

bool
IcePy::initEndpoint(PyObject* module)
{
    if (PyType_Ready(&EndpointType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &EndpointType; // Necessary to prevent GCC's strict-alias warnings.
    if (PyModule_AddObject(module, "Endpoint", reinterpret_cast<PyObject*>(type)) < 0)
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
    if (!obj)
    {
        return nullptr;
    }
    obj->endpoint = new Ice::EndpointPtr(endpoint);
    return (PyObject*)obj;
}

bool
IcePy::toEndpointSeq(PyObject* endpoints, Ice::EndpointSeq& seq)
{
    Py_ssize_t sz = PySequence_Fast_GET_SIZE(endpoints);
    for (Py_ssize_t i = 0; i < sz; ++i)
    {
        PyObject* p = PySequence_Fast_GET_ITEM(endpoints, i);
        PyTypeObject* type = &EndpointType; // Necessary to prevent GCC's strict-alias warnings.
        if (!PyObject_IsInstance(p, reinterpret_cast<PyObject*>(type)))
        {
            PyErr_Format(PyExc_ValueError, "expected element of type Ice.Endpoint");
            return false;
        }
        Ice::EndpointPtr endp = getEndpoint(p);
        if (!endp)
        {
            return false;
        }
        seq.push_back(endp);
    }

    return true;
}
