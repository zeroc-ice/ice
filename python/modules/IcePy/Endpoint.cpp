// Copyright (c) ZeroC, Inc.

#include "Endpoint.h"
#include "EndpointInfo.h"
#include "Ice/TargetCompare.h"
#include "Util.h"

using namespace std;
using namespace IcePy;

namespace
{
    constexpr const char* endpointToString_doc = R"(toString() -> str

Returns a string representation of this endpoint.

Returns
-------
str
    The string representation of this endpoint.)";

    constexpr const char* endpointGetInfo_doc = R"(getInfo() -> Ice.EndpointInfo

Returns this endpoint's information.

Returns
-------
Ice.EndpointInfo
    This endpoint's information class.)";

    constexpr const char* EndpointType_doc =
        R"(An endpoint specifies the address of the server-end of an Ice connection.
An object adapter listens on one or more endpoints and a client establishes a connection to an endpoint.)";
}

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
    bool result{false};

    if (PyObject_TypeCheck(other, &EndpointType))
    {
        auto* p2 = reinterpret_cast<EndpointObject*>(other);

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
        return createString((*self->endpoint)->toString());
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
    return endpointToString(self, nullptr);
}

extern "C" PyObject*
endpointGetInfo(EndpointObject* self, PyObject* /*args*/)
{
    assert(self->endpoint);
    try
    {
        return createEndpointInfo((*self->endpoint)->getInfo());
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

static PyMethodDef EndpointMethods[] = {
    {"toString", reinterpret_cast<PyCFunction>(endpointToString), METH_NOARGS, PyDoc_STR(endpointToString_doc)},
    {"getInfo", reinterpret_cast<PyCFunction>(endpointGetInfo), METH_NOARGS, PyDoc_STR(endpointGetInfo_doc)},
    {} /* sentinel */
};

namespace IcePy
{
    // clang-format off
    PyTypeObject EndpointType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.Endpoint",
        .tp_basicsize = sizeof(EndpointObject),
        .tp_dealloc = reinterpret_cast<destructor>(endpointDealloc),
        .tp_repr = reinterpret_cast<reprfunc>(endpointRepr),
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = PyDoc_STR(EndpointType_doc),
        .tp_richcompare = reinterpret_cast<richcmpfunc>(endpointCompare),
        .tp_methods = EndpointMethods,
        .tp_new = reinterpret_cast<newfunc>(endpointNew),
    };
    // clang-format on
}

bool
IcePy::initEndpoint(PyObject* module)
{
    if (PyType_Ready(&EndpointType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "Endpoint", reinterpret_cast<PyObject*>(&EndpointType)) < 0)
    {
        return false;
    }

    return true;
}

Ice::EndpointPtr
IcePy::getEndpoint(PyObject* obj)
{
    assert(PyObject_IsInstance(obj, reinterpret_cast<PyObject*>(&EndpointType)));
    return *(reinterpret_cast<EndpointObject*>(obj)->endpoint);
}

PyObject*
IcePy::createEndpoint(const Ice::EndpointPtr& endpoint)
{
    auto obj = reinterpret_cast<EndpointObject*>(EndpointType.tp_alloc(&EndpointType, 0));
    if (!obj)
    {
        return nullptr;
    }
    obj->endpoint = new Ice::EndpointPtr(endpoint);
    return reinterpret_cast<PyObject*>(obj);
}

bool
IcePy::toEndpointSeq(PyObject* endpoints, Ice::EndpointSeq& seq)
{
    Py_ssize_t sz{PySequence_Fast_GET_SIZE(endpoints)};
    for (Py_ssize_t i = 0; i < sz; ++i)
    {
        PyObject* p{PySequence_Fast_GET_ITEM(endpoints, i)};
        if (!PyObject_IsInstance(p, reinterpret_cast<PyObject*>(&EndpointType)))
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
