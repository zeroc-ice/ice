// Copyright (c) ZeroC, Inc.

#include "PropertiesAdmin.h"
#include "Ice/DisableWarnings.h"
#include "Thread.h"
#include "Types.h"
#include "Util.h"

using namespace std;
using namespace IcePy;

namespace IcePy
{
    struct NativePropertiesAdminObject
    {
        PyObject_HEAD Ice::NativePropertiesAdminPtr* admin;
        vector<pair<PyObject*, function<void()>>>* callbacks;
    };
}

extern "C" NativePropertiesAdminObject*
nativePropertiesAdminNew(PyTypeObject* /*type*/, PyObject* /*args*/, PyObject* /*kwds*/)
{
    PyErr_Format(PyExc_RuntimeError, "This object cannot be created directly");
    return nullptr;
}

extern "C" void
nativePropertiesAdminDealloc(NativePropertiesAdminObject* self)
{
    delete self->admin;
    delete self->callbacks;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

extern "C" PyObject*
nativePropertiesAdminAddUpdateCB(NativePropertiesAdminObject* self, PyObject* args)
{
    PyObject* callbackType = lookupType("Ice.PropertiesAdminUpdateCallback");
    PyObject* callback;
    if (!PyArg_ParseTuple(args, "O!", callbackType, &callback))
    {
        return nullptr;
    }

    std::function<void()> remover =
        (*self->admin)
            ->addUpdateCallback(
                [callback](const Ice::PropertyDict& dict)
                {
                    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

                    PyObjectHandle result{PyDict_New()};
                    if (result.get())
                    {
                        for (Ice::PropertyDict::const_iterator p = dict.begin(); p != dict.end(); ++p)
                        {
                            PyObjectHandle key{createString(p->first)};
                            PyObjectHandle val{createString(p->second)};
                            if (!val.get() || PyDict_SetItem(result.get(), key.get(), val.get()) < 0)
                            {
                                return;
                            }
                        }
                    }

                    PyObjectHandle obj{PyObject_CallMethod(callback, "updated", "O", result.get())};
                    if (!obj.get())
                    {
                        assert(PyErr_Occurred());
                        throw AbortMarshaling();
                    }
                });

    (*self->callbacks).push_back(make_pair(callback, remover));
    Py_INCREF(callback);

    return Py_None;
}

extern "C" PyObject*
nativePropertiesAdminRemoveUpdateCB(NativePropertiesAdminObject* self, PyObject* args)
{
    PyObject* callbackType = lookupType("Ice.PropertiesAdminUpdateCallback");
    PyObject* callback;
    if (!PyArg_ParseTuple(args, "O!", callbackType, &callback))
    {
        return nullptr;
    }

    auto& callbacks = *self->callbacks;

    auto p =
        std::find_if(callbacks.begin(), callbacks.end(), [callback](const auto& q) { return q.first == callback; });
    if (p != callbacks.end())
    {
        p->second();
        Py_DECREF(callback);
        callbacks.erase(p);
    }

    return Py_None;
}

static PyMethodDef NativePropertiesAdminMethods[] = {
    {"addUpdateCallback",
     reinterpret_cast<PyCFunction>(nativePropertiesAdminAddUpdateCB),
     METH_VARARGS,
     PyDoc_STR("addUpdateCallback(callback) -> None")},
    {"removeUpdateCallback",
     reinterpret_cast<PyCFunction>(nativePropertiesAdminRemoveUpdateCB),
     METH_VARARGS,
     PyDoc_STR("removeUpdateCallback(callback) -> None")},
    {0, 0} /* sentinel */
};

namespace IcePy
{
    PyTypeObject NativePropertiesAdminType = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(0, 0) "IcePy.NativePropertiesAdmin", /* tp_name */
        sizeof(NativePropertiesAdminObject),                       /* tp_basicsize */
        0,                                                         /* tp_itemsize */
        /* methods */
        reinterpret_cast<destructor>(nativePropertiesAdminDealloc), /* tp_dealloc */
        0,                                                          /* tp_print */
        0,                                                          /* tp_getattr */
        0,                                                          /* tp_setattr */
        0,                                                          /* tp_reserved */
        0,                                                          /* tp_repr */
        0,                                                          /* tp_as_number */
        0,                                                          /* tp_as_sequence */
        0,                                                          /* tp_as_mapping */
        0,                                                          /* tp_hash */
        0,                                                          /* tp_call */
        0,                                                          /* tp_str */
        0,                                                          /* tp_getattro */
        0,                                                          /* tp_setattro */
        0,                                                          /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                                         /* tp_flags */
        0,                                                          /* tp_doc */
        0,                                                          /* tp_traverse */
        0,                                                          /* tp_clear */
        0,                                                          /* tp_richcompare */
        0,                                                          /* tp_weaklistoffset */
        0,                                                          /* tp_iter */
        0,                                                          /* tp_iternext */
        NativePropertiesAdminMethods,                               /* tp_methods */
        0,                                                          /* tp_members */
        0,                                                          /* tp_getset */
        0,                                                          /* tp_base */
        0,                                                          /* tp_dict */
        0,                                                          /* tp_descr_get */
        0,                                                          /* tp_descr_set */
        0,                                                          /* tp_dictoffset */
        0,                                                          /* tp_init */
        0,                                                          /* tp_alloc */
        reinterpret_cast<newfunc>(nativePropertiesAdminNew),        /* tp_new */
        0,                                                          /* tp_free */
        0,                                                          /* tp_is_gc */
    };
}

bool
IcePy::initPropertiesAdmin(PyObject* module)
{
    if (PyType_Ready(&NativePropertiesAdminType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &NativePropertiesAdminType; // Necessary to prevent GCC's strict-alias warnings.
    if (PyModule_AddObject(module, "NativePropertiesAdmin", reinterpret_cast<PyObject*>(type)) < 0)
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
    if (!p)
    {
        return nullptr;
    }

    p->admin = new Ice::NativePropertiesAdminPtr(admin);
    p->callbacks = new vector<pair<PyObject*, function<void()>>>();
    return (PyObject*)p;
}
