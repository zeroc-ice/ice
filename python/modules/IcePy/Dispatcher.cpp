// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Dispatcher.h>
#include <Connection.h>
#include <Thread.h>
#include <Ice/Initialize.h>

using namespace std;
using namespace IcePy;

namespace IcePy
{

struct DispatcherCallObject
{
    PyObject_HEAD
    Ice::DispatcherCallPtr* call;
};

}

#ifdef WIN32
extern "C"
#endif
static void
dispatcherCallDealloc(DispatcherCallObject* self)
{
    delete self->call;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
dispatcherCallInvoke(DispatcherCallObject* self, PyObject* /*args*/, PyObject* /*kwds*/)
{
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        (*self->call)->run();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return incRef(Py_None);
}

namespace IcePy
{

PyTypeObject DispatcherCallType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.DispatcherCall"),      /* tp_name */
    sizeof(DispatcherCallObject),         /* tp_basicsize */
    0,                                    /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(dispatcherCallDealloc), /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_reserved */
    0,                               /* tp_repr */
    0,                               /* tp_as_number */
    0,                               /* tp_as_sequence */
    0,                               /* tp_as_mapping */
    0,                               /* tp_hash */
    reinterpret_cast<ternaryfunc>(dispatcherCallInvoke), /* tp_call */
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
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    0,                               /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

}

bool
IcePy::initDispatcher(PyObject* module)
{
    if(PyType_Ready(&DispatcherCallType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &DispatcherCallType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("DispatcherCall"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

IcePy::Dispatcher::Dispatcher(PyObject* dispatcher) :
    _dispatcher(dispatcher)
{
    if(!PyCallable_Check(dispatcher))
    {
        throw Ice::InitializationException(__FILE__, __LINE__, "dispatcher must be a callable");
    }

    Py_INCREF(dispatcher);
}

void
IcePy::Dispatcher::setCommunicator(const Ice::CommunicatorPtr& communicator)
{
    _communicator = communicator;
}

void
IcePy::Dispatcher::dispatch(const Ice::DispatcherCallPtr& call, const Ice::ConnectionPtr& con)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    DispatcherCallObject* obj =
        reinterpret_cast<DispatcherCallObject*>(DispatcherCallType.tp_alloc(&DispatcherCallType, 0));
    if(!obj)
    {
        return;
    }

    obj->call = new Ice::DispatcherCallPtr(call);
    PyObjectHandle c = createConnection(con, _communicator);
    PyObjectHandle tmp = PyObject_CallFunction(_dispatcher.get(), STRCAST("OO"), obj, c.get());
    Py_DECREF(reinterpret_cast<PyObject*>(obj));
    if(!tmp.get())
    {
        throwPythonException();
    }
}
