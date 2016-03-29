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
#include <Current.h>
#include <structmember.h>
#include <Connection.h>
#include <ObjectAdapter.h>
#include <Util.h>
#include <Ice/ObjectAdapter.h>

using namespace std;
using namespace IcePy;

namespace IcePy
{

struct CurrentObject
{
    PyObject_HEAD
    Ice::Current* current;
    PyObject* adapter;
    PyObject* con;
    PyObject* id;
    PyObject* facet;
    PyObject* operation;
    PyObject* mode;
    PyObject* ctx;
    PyObject* requestId;
    PyObject* encoding;
};

//
// Member identifiers.
//
const Py_ssize_t CURRENT_ADAPTER    = 0;
const Py_ssize_t CURRENT_CONNECTION = 1;
const Py_ssize_t CURRENT_ID         = 2;
const Py_ssize_t CURRENT_FACET      = 3;
const Py_ssize_t CURRENT_OPERATION  = 4;
const Py_ssize_t CURRENT_MODE       = 5;
const Py_ssize_t CURRENT_CTX        = 6;
const Py_ssize_t CURRENT_REQUEST_ID = 7;
const Py_ssize_t CURRENT_ENCODING   = 8;

}

#ifdef WIN32
extern "C"
#endif
static CurrentObject*
currentNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    CurrentObject* self = reinterpret_cast<CurrentObject*>(type->tp_alloc(type, 0));
    if(!self)
    {
        return 0;
    }

    self->current = new Ice::Current;
    self->adapter = 0;
    self->con = 0;
    self->id = 0;
    self->facet = 0;
    self->operation = 0;
    self->mode = 0;
    self->ctx = 0;
    self->requestId = 0;
    self->encoding = 0;

    return self;
}

#ifdef WIN32
extern "C"
#endif
static void
currentDealloc(CurrentObject* self)
{
    Py_XDECREF(self->adapter);
    Py_XDECREF(self->con);
    Py_XDECREF(self->id);
    Py_XDECREF(self->facet);
    Py_XDECREF(self->operation);
    Py_XDECREF(self->mode);
    Py_XDECREF(self->ctx);
    Py_XDECREF(self->requestId);
    Py_XDECREF(self->encoding);
    delete self->current;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
currentGetter(CurrentObject* self, void* closure)
{
    //
    // This function intercepts requests for attributes of a Current object. We use this
    // lazy initialization in order to minimize the cost of translating Ice::Current into a
    // Python object for every upcall.
    //
    PyObject* result = 0;

    assert(self->current);

    Py_ssize_t field = reinterpret_cast<Py_ssize_t>(closure);
    switch(field)
    {
    case CURRENT_ADAPTER:
    {
        if(!self->adapter)
        {
            self->adapter = wrapObjectAdapter(self->current->adapter);
            if(!self->adapter)
            {
                return 0;
            }
        }
        Py_INCREF(self->adapter);
        result = self->adapter;
        break;
    }
    case CURRENT_CONNECTION:
    {
        if(!self->con)
        {
            self->con = createConnection(self->current->con, self->current->adapter->getCommunicator());
            if(!self->con)
            {
                return 0;
            }
        }
        Py_INCREF(self->con);
        result = self->con;
        break;
    }
    case CURRENT_ID:
    {
        if(!self->id)
        {
            self->id = createIdentity(self->current->id);
        }
        Py_INCREF(self->id);
        result = self->id;
        break;
    }
    case CURRENT_FACET:
    {
        if(!self->facet)
        {
            self->facet = createString(self->current->facet);
        }
        Py_INCREF(self->facet);
        result = self->facet;
        break;
    }
    case CURRENT_OPERATION:
    {
        if(!self->operation)
        {
            self->operation = createString(self->current->operation);
        }
        Py_INCREF(self->operation);
        result = self->operation;
        break;
    }
    case CURRENT_MODE:
    {
        if(!self->mode)
        {
            PyObject* type = lookupType("Ice.OperationMode");
            assert(type);
            const char* enumerator = 0;
            switch(self->current->mode)
            {
            case Ice::Normal:
                enumerator = "Normal";
                break;
            case Ice::Nonmutating:
                enumerator = "Nonmutating";
                break;
            case Ice::Idempotent:
                enumerator = "Idempotent";
                break;
            }
            self->mode = PyObject_GetAttrString(type, STRCAST(enumerator));
            assert(self->mode);
        }
        Py_INCREF(self->mode);
        result = self->mode;
        break;
    }
    case CURRENT_CTX:
    {
        if(!self->ctx)
        {
            self->ctx = PyDict_New();
            if(self->ctx && !contextToDictionary(self->current->ctx, self->ctx))
            {
                Py_DECREF(self->ctx);
                self->ctx = 0;
                break;
            }
        }
        Py_INCREF(self->ctx);
        result = self->ctx;
        break;
    }
    case CURRENT_REQUEST_ID:
    {
        if(!self->requestId)
        {
            self->requestId = PyLong_FromLong(self->current->requestId);
            assert(self->requestId);
        }
        Py_INCREF(self->requestId);
        result = self->requestId;
        break;
    }
    case CURRENT_ENCODING:
    {
        if(!self->encoding)
        {
            self->encoding = IcePy::createEncodingVersion(self->current->encoding);
            assert(self->encoding);
        }
        Py_INCREF(self->encoding);
        result = self->encoding;
        break;
    }
    }

    return result;
}

static PyGetSetDef CurrentGetSetters[] =
{
    { STRCAST("adapter"), reinterpret_cast<getter>(currentGetter), 0, STRCAST("object adapter"),
      reinterpret_cast<void*>(CURRENT_ADAPTER) },
    { STRCAST("con"), reinterpret_cast<getter>(currentGetter), 0, STRCAST("connection info"),
      reinterpret_cast<void*>(CURRENT_CONNECTION) },
    { STRCAST("id"), reinterpret_cast<getter>(currentGetter), 0, STRCAST("identity"),
      reinterpret_cast<void*>(CURRENT_ID) },
    { STRCAST("facet"), reinterpret_cast<getter>(currentGetter), 0, STRCAST("facet name"),
      reinterpret_cast<void*>(CURRENT_FACET) },
    { STRCAST("operation"), reinterpret_cast<getter>(currentGetter), 0, STRCAST("operation name"),
      reinterpret_cast<void*>(CURRENT_OPERATION) },
    { STRCAST("mode"), reinterpret_cast<getter>(currentGetter), 0, STRCAST("operation mode"),
      reinterpret_cast<void*>(CURRENT_MODE) },
    { STRCAST("ctx"), reinterpret_cast<getter>(currentGetter), 0, STRCAST("context"),
      reinterpret_cast<void*>(CURRENT_CTX) },
    { STRCAST("requestId"), reinterpret_cast<getter>(currentGetter), 0, STRCAST("requestId"),
      reinterpret_cast<void*>(CURRENT_REQUEST_ID) },
    { STRCAST("encoding"), reinterpret_cast<getter>(currentGetter), 0, STRCAST("encoding"),
      reinterpret_cast<void*>(CURRENT_ENCODING) },
    { 0 }  /* Sentinel */
};

namespace IcePy
{

PyTypeObject CurrentType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.Current"),        /* tp_name */
    sizeof(CurrentObject),           /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(currentDealloc), /* tp_dealloc */
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
    0,                               /* tp_methods */
    0,                               /* tp_members */
    CurrentGetSetters,               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    reinterpret_cast<newfunc>(currentNew), /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

}

bool
IcePy::initCurrent(PyObject* module)
{
    if(PyType_Ready(&CurrentType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &CurrentType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("Current"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

PyObject*
IcePy::createCurrent(const Ice::Current& current)
{
    //
    // Return an instance of IcePy.Current to hold the current information.
    //
    CurrentObject* obj = currentNew(&CurrentType, 0, 0);
    if(obj)
    {
        *obj->current = current;
    }
    return reinterpret_cast<PyObject*>(obj);
}
