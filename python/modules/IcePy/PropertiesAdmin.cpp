// Copyright (c) ZeroC, Inc.

#include "PropertiesAdmin.h"
#include "Ice/DisableWarnings.h"
#include "Thread.h"
#include "Types.h"
#include "Util.h"

#include <algorithm>

using namespace std;
using namespace IcePy;

namespace
{
    constexpr const char* nativePropertiesAdminAddUpdateCB_doc =
        R"(addUpdateCallback(callback: Callable[[dict[str, str]], None]) -> None

Registers an update callback that will be invoked when a property update occurs.

Parameters
----------
callback : Callable[[dict[str, str]], None]
    The callback.)";

    constexpr const char* nativePropertiesAdminRemoveUpdateCB_doc =
        R"(removeUpdateCallback(callback: Callable[[dict[str, str]], None]) -> None

Removes a previously registered update callback.

Parameters
----------
callback : Callable[[dict[str, str]], None]
    The callback to remove.)";
}

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
    PyObject* callback{nullptr};
    if (!PyArg_ParseTuple(args, "O", &callback))
    {
        return nullptr;
    }

    if (!PyCallable_Check(callback))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a callable object");
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
                        for (const auto& [key, value] : dict)
                        {
                            PyObjectHandle pyKey{createString(key)};
                            PyObjectHandle pyValue{createString(value)};
                            if (!pyValue.get() || PyDict_SetItem(result.get(), pyKey.get(), pyValue.get()) < 0)
                            {
                                return;
                            }
                        }
                    }

                    PyObjectHandle callbackArgs{PyTuple_New(1)};
                    PyTuple_SetItem(callbackArgs.get(), 0, result.release());

                    PyObjectHandle obj{PyObject_Call(callback, callbackArgs.get(), nullptr)};
                    if (!obj.get())
                    {
                        assert(PyErr_Occurred());
                        throw AbortMarshaling();
                    }
                });

    (*self->callbacks).emplace_back(Py_NewRef(callback), remover);
    return Py_None;
}

extern "C" PyObject*
nativePropertiesAdminRemoveUpdateCB(NativePropertiesAdminObject* self, PyObject* args)
{
    PyObject* callback{nullptr};
    if (!PyArg_ParseTuple(args, "O", &callback))
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
     PyDoc_STR(nativePropertiesAdminAddUpdateCB_doc)},
    {"removeUpdateCallback",
     reinterpret_cast<PyCFunction>(nativePropertiesAdminRemoveUpdateCB),
     METH_VARARGS,
     PyDoc_STR(nativePropertiesAdminRemoveUpdateCB_doc)},
    {} /* sentinel */
};

namespace IcePy
{
    // clang-format off
    PyTypeObject NativePropertiesAdminType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.NativePropertiesAdmin",
        .tp_basicsize = sizeof(NativePropertiesAdminObject),
        .tp_dealloc = reinterpret_cast<destructor>(nativePropertiesAdminDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_doc = PyDoc_STR("The default implementation of the 'Properties' admin facet."),
        .tp_methods = NativePropertiesAdminMethods,
        .tp_new = reinterpret_cast<newfunc>(nativePropertiesAdminNew),
    };
    // clang-format on
}

bool
IcePy::initPropertiesAdmin(PyObject* module)
{
    if (PyType_Ready(&NativePropertiesAdminType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "NativePropertiesAdmin", reinterpret_cast<PyObject*>(&NativePropertiesAdminType)) <
        0)
    {
        return false;
    }
    return true;
}

PyObject*
IcePy::createNativePropertiesAdmin(const Ice::NativePropertiesAdminPtr& admin)
{
    PyTypeObject* type = &NativePropertiesAdminType;

    auto* p = reinterpret_cast<NativePropertiesAdminObject*>(type->tp_alloc(type, 0));
    if (!p)
    {
        return nullptr;
    }

    p->admin = new Ice::NativePropertiesAdminPtr(admin);
    p->callbacks = new vector<pair<PyObject*, function<void()>>>();
    return (PyObject*)p;
}
