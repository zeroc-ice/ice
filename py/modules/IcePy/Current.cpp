// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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
};

//
// Member identifiers.
//
const int CURRENT_ADAPTER    = 0;
const int CURRENT_CONNECTION = 1;
const int CURRENT_ID         = 2;
const int CURRENT_FACET      = 3;
const int CURRENT_OPERATION  = 4;
const int CURRENT_MODE       = 5;
const int CURRENT_CTX        = 6;

}

#ifdef WIN32
extern "C"
#endif
static CurrentObject*
currentNew(PyObject* /*arg*/)
{
    CurrentObject* self = PyObject_New(CurrentObject, &CurrentType);
    if(self == NULL)
    {
        return NULL;
    }

    self->current = new Ice::Current;
    self->adapter = NULL;
    self->con = NULL;
    self->id = NULL;
    self->facet = NULL;
    self->operation = NULL;
    self->mode = NULL;
    self->ctx = NULL;

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
    delete self->current;
    PyObject_Del(self);
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
    PyObject* result = NULL;

    assert(self->current);

    switch((int)closure)
    {
    case CURRENT_ADAPTER:
    {
        if(self->adapter == NULL)
        {
	    self->adapter = wrapObjectAdapter(self->current->adapter);
            if(self->adapter == NULL)
            {
                return NULL;
            }
        }
        Py_INCREF(self->adapter);
        result = self->adapter;
        break;
    }
    case CURRENT_CONNECTION:
    {
	if(self->con == NULL)
	{
	    self->con = createConnection(self->current->con, self->current->adapter->getCommunicator());
	    if(self->con == NULL)
	    {
		return NULL;
	    }
	}
        Py_INCREF(self->con);
        result = self->con;
        break;
    }
    case CURRENT_ID:
    {
        if(self->id == NULL)
        {
            self->id = createIdentity(self->current->id);
        }
        Py_INCREF(self->id);
        result = self->id;
        break;
    }
    case CURRENT_FACET:
    {
        if(self->facet == NULL)
        {
            self->facet = PyString_FromString(const_cast<char*>(self->current->facet.c_str()));
        }
        Py_INCREF(self->facet);
        result = self->facet;
        break;
    }
    case CURRENT_OPERATION:
    {
        if(self->operation == NULL)
        {
            self->operation = PyString_FromString(const_cast<char*>(self->current->operation.c_str()));
        }
        Py_INCREF(self->operation);
        result = self->operation;
        break;
    }
    case CURRENT_MODE:
    {
        if(self->mode == NULL)
        {
            PyObject* type = lookupType("Ice.OperationMode");
            assert(type != NULL);
            char* enumerator = 0;
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
            self->mode = PyObject_GetAttrString(type, enumerator);
            assert(self->mode != NULL);
        }
        Py_INCREF(self->mode);
        result = self->mode;
        break;
    }
    case CURRENT_CTX:
    {
        if(self->ctx == NULL)
        {
            self->ctx = PyDict_New();
            if(self->ctx != NULL && !contextToDictionary(self->current->ctx, self->ctx))
            {
                Py_DECREF(self->ctx);
                self->ctx = NULL;
                break;
            }
        }
        Py_INCREF(self->ctx);
        result = self->ctx;
        break;
    }
    }

    return result;
}

static PyGetSetDef CurrentGetSetters[] =
{
    {"adapter", (getter)currentGetter, NULL, "object adapter", (void*)CURRENT_ADAPTER},
    {"con", (getter)currentGetter, NULL, "connection info", (void*)CURRENT_CONNECTION},
    {"id", (getter)currentGetter, NULL, "identity", (void*)CURRENT_ID},
    {"facet", (getter)currentGetter, NULL, "facet name", (void*)CURRENT_FACET},
    {"operation", (getter)currentGetter, NULL, "operation name", (void*)CURRENT_OPERATION},
    {"mode", (getter)currentGetter, NULL, "operation mode", (void*)CURRENT_MODE},
    {"ctx", (getter)currentGetter, NULL, "context", (void*)CURRENT_CTX},
    {NULL}  /* Sentinel */
};

namespace IcePy
{

PyTypeObject CurrentType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(NULL)
    0,                               /* ob_size */
    "IcePy.Current",                 /* tp_name */
    sizeof(CurrentObject),           /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)currentDealloc,      /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_compare */
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
    (newfunc)currentNew,             /* tp_new */
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
    if(PyModule_AddObject(module, "Current", (PyObject*)&CurrentType) < 0)
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
    CurrentObject* obj = currentNew(NULL);
    if(obj != NULL)
    {
        *obj->current = current;
    }
    return (PyObject*)obj;
}
