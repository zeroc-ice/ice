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
#include <IceUtil/DisableWarnings.h>
#include <PropertiesAdmin.h>
#include <Util.h>
#include <Thread.h>
#include <Types.h>

using namespace std;
using namespace IcePy;

namespace IcePy
{

class UpdateCallbackWrapper : public Ice::PropertiesAdminUpdateCallback
{
public:

    UpdateCallbackWrapper(PyObject*);
    ~UpdateCallbackWrapper();

    PyObject* getObject() const;

    void updated(const Ice::PropertyDict&);

private:

    PyObject* _callback;
};
typedef IceUtil::Handle<UpdateCallbackWrapper> UpdateCallbackWrapperPtr;

struct NativePropertiesAdminObject
{
    PyObject_HEAD
    Ice::NativePropertiesAdminPtr* admin;
    vector<UpdateCallbackWrapperPtr>* callbacks;
};

}

IcePy::UpdateCallbackWrapper::UpdateCallbackWrapper(PyObject* callback) : _callback(callback)
{
    Py_INCREF(_callback);
}

IcePy::UpdateCallbackWrapper::~UpdateCallbackWrapper()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    Py_DECREF(_callback);
}

PyObject*
IcePy::UpdateCallbackWrapper::getObject() const
{
    return _callback;
}

void
IcePy::UpdateCallbackWrapper::updated(const Ice::PropertyDict& dict)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle result = PyDict_New();
    if(result.get())
    {
        for(Ice::PropertyDict::const_iterator p = dict.begin(); p != dict.end(); ++p)
        {
            PyObjectHandle key = createString(p->first);
            PyObjectHandle val = createString(p->second);
            if(!val.get() || PyDict_SetItem(result.get(), key.get(), val.get()) < 0)
            {
                return;
            }
        }
    }

    PyObjectHandle obj = PyObject_CallMethod(_callback, STRCAST("updated"), STRCAST("O"), result.get());
    if(!obj.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }
}

#ifdef WIN32
extern "C"
#endif
static NativePropertiesAdminObject*
nativePropertiesAdminNew(PyTypeObject* /*type*/, PyObject* /*args*/, PyObject* /*kwds*/)
{
    PyErr_Format(PyExc_RuntimeError, STRCAST("This object cannot be created directly"));
    return 0;
}

#ifdef WIN32
extern "C"
#endif
static void
nativePropertiesAdminDealloc(NativePropertiesAdminObject* self)
{
    delete self->admin;
    delete self->callbacks;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
nativePropertiesAdminAddUpdateCB(NativePropertiesAdminObject* self, PyObject* args)
{
    PyObject* callbackType = lookupType("Ice.PropertiesAdminUpdateCallback");
    PyObject* callback;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), callbackType, &callback))
    {
        return 0;
    }

    (*self->callbacks).push_back(new UpdateCallbackWrapper(callback));
    (*self->admin)->addUpdateCallback((*self->callbacks).back());
    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
nativePropertiesAdminRemoveUpdateCB(NativePropertiesAdminObject* self, PyObject* args)
{
    PyObject* callbackType = lookupType("Ice.PropertiesAdminUpdateCallback");
    PyObject* callback;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), callbackType, &callback))
    {
        return 0;
    }

    for(vector<UpdateCallbackWrapperPtr>::const_iterator p = (*self->callbacks).begin(); p != (*self->callbacks).end();
        ++p)
    {
        if((*p)->getObject() == callback)
        {
            (*self->admin)->removeUpdateCallback(*p);
            break;
        }
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef NativePropertiesAdminMethods[] =
{
    { STRCAST("addUpdateCallback"), reinterpret_cast<PyCFunction>(nativePropertiesAdminAddUpdateCB), METH_VARARGS,
        PyDoc_STR(STRCAST("addUpdateCallback(callback) -> None")) },
    { STRCAST("removeUpdateCallback"), reinterpret_cast<PyCFunction>(nativePropertiesAdminRemoveUpdateCB), METH_VARARGS,
        PyDoc_STR(STRCAST("removeUpdateCallback(callback) -> None")) },
    { 0, 0 } /* sentinel */
};

namespace IcePy
{

PyTypeObject NativePropertiesAdminType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.NativePropertiesAdmin"), /* tp_name */
    sizeof(NativePropertiesAdminObject),    /* tp_basicsize */
    0,                                      /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(nativePropertiesAdminDealloc), /* tp_dealloc */
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
    NativePropertiesAdminMethods,    /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    reinterpret_cast<newfunc>(nativePropertiesAdminNew), /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

}

bool
IcePy::initPropertiesAdmin(PyObject* module)
{
    if(PyType_Ready(&NativePropertiesAdminType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &NativePropertiesAdminType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("NativePropertiesAdmin"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }
    return true;
}

PyObject*
IcePy::createNativePropertiesAdmin(const Ice::NativePropertiesAdminPtr& admin)
{
    PyTypeObject* type = &NativePropertiesAdminType;

    NativePropertiesAdminObject* p = reinterpret_cast<NativePropertiesAdminObject*>(type->tp_alloc(type, 0));
    if(!p)
    {
        return 0;
    }

    p->admin = new Ice::NativePropertiesAdminPtr(admin);
    p->callbacks = new vector<UpdateCallbackWrapperPtr>();
    return (PyObject*)p;
}
