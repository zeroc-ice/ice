// Copyright (c) ZeroC, Inc.

#include "ValueFactoryManager.h"
#include "Ice/LocalExceptions.h"
#include "Thread.h"
#include "Types.h"

using namespace std;
using namespace IcePy;

namespace IcePy
{
    struct ValueFactoryManagerObject
    {
        PyObject_HEAD ValueFactoryManagerPtr* vfm;
    };
}

namespace
{
    ValueInfoPtr getValueInfo(string_view id)
    {
        return id == Ice::Value::ice_staticId() ? lookupValueInfo("::Ice::UnknownSlicedValue") : lookupValueInfo(id);
    }
}

ValueFactoryManagerPtr
IcePy::ValueFactoryManager::create()
{
    // can't use make_shared because constructor is private
    auto vfm = shared_ptr<ValueFactoryManager>(new ValueFactoryManager);

    //
    // Create a Python wrapper around this object. Note that this is cyclic - we clear the
    // reference in destroy().
    //
    ValueFactoryManagerObject* obj =
        reinterpret_cast<ValueFactoryManagerObject*>(ValueFactoryManagerType.tp_alloc(&ValueFactoryManagerType, 0));
    assert(obj);

    // We're creating a shared_ptr on the heap
    obj->vfm = new ValueFactoryManagerPtr(vfm);
    vfm->_self = reinterpret_cast<PyObject*>(obj);

    return vfm;
}

IcePy::ValueFactoryManager::ValueFactoryManager() : _defaultFactory{make_shared<DefaultValueFactory>()} {}

void
IcePy::ValueFactoryManager::add(Ice::ValueFactory, string_view)
{
    throw Ice::FeatureNotSupportedException{
        __FILE__,
        __LINE__,
        "the Ice Python value factory manager does not accept C++ value factories"};
}

Ice::ValueFactory
IcePy::ValueFactoryManager::find(string_view typeId) const noexcept
{
    ValueFactoryPtr factory;
    {
        CustomFactoryMap::const_iterator p = _customFactories.find(typeId);
        if (p != _customFactories.end())
        {
            factory = p->second;
        }
        else if (typeId.empty())
        {
            factory = _defaultFactory;
        }
    }

    if (factory)
    {
        return [factory = std::move(factory)](string_view id) { return factory->create(id); };
    }
    else
    {
        return nullptr;
    }
}

void
IcePy::ValueFactoryManager::add(PyObject* valueFactory, string_view id)
{
    try
    {
        auto [_, inserted] = _customFactories.try_emplace(string{id}, make_shared<CustomValueFactory>(valueFactory));
        if (!inserted)
        {
            throw Ice::AlreadyRegisteredException(__FILE__, __LINE__, "value factory", string{id});
        }
    }
    catch (...)
    {
        setPythonException(current_exception());
    }
}

PyObject*
IcePy::ValueFactoryManager::findValueFactory(string_view id) const
{
    // Called from the Python thread, no need to lock.

    CustomFactoryMap::const_iterator p = _customFactories.find(id);
    if (p != _customFactories.end())
    {
        return p->second->getValueFactory();
    }

    return Py_None;
}

PyObject*
IcePy::ValueFactoryManager::getObject() const
{
    PyObject* obj = _self.get();
    Py_INCREF(obj);
    return obj;
}

void
IcePy::ValueFactoryManager::destroy()
{
    // Called by the Python thread from communicatorDestroy.
    if (_self)
    {
        // Break the cyclic reference.
        _self = nullptr;
        _customFactories.clear();
    }
}

IcePy::CustomValueFactory::CustomValueFactory(PyObject* valueFactory) : _valueFactory(valueFactory)
{
    Py_INCREF(_valueFactory);
    assert(_valueFactory != Py_None); // This should always be present.
}

IcePy::CustomValueFactory::~CustomValueFactory() { Py_DECREF(_valueFactory); }

shared_ptr<Ice::Value>
IcePy::CustomValueFactory::create(string_view id)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    //
    // Get the type information.
    //
    ValueInfoPtr info = getValueInfo(id);

    if (!info)
    {
        return nullptr;
    }

    PyObjectHandle obj{PyObject_CallFunction(_valueFactory, "s#", id.data(), static_cast<Py_ssize_t>(id.size()))};

    if (!obj.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    if (obj.get() == Py_None)
    {
        return nullptr;
    }

    return make_shared<ValueReader>(obj.get(), info);
    ;
}

PyObject*
IcePy::CustomValueFactory::getValueFactory() const
{
    Py_INCREF(_valueFactory);
    return _valueFactory;
}

shared_ptr<Ice::Value>
IcePy::DefaultValueFactory::create(std::string_view id)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    //
    // Get the type information.
    //
    ValueInfoPtr info = getValueInfo(id);

    if (!info)
    {
        return nullptr;
    }

    //
    // Instantiate the object.
    //
    PyTypeObject* type = reinterpret_cast<PyTypeObject*>(info->pythonType);
    PyObjectHandle args{PyTuple_New(0)};
    PyObjectHandle obj{type->tp_new(type, args.get(), 0)};
    if (!obj.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    return make_shared<ValueReader>(obj.get(), info);
}

extern "C" ValueFactoryManagerObject*
valueFactoryManagerNew(PyTypeObject* /*type*/, PyObject* /*args*/, PyObject* /*kwds*/)
{
    PyErr_Format(PyExc_RuntimeError, "Do not instantiate this object directly");
    return nullptr;
}

extern "C" void
valueFactoryManagerDealloc(ValueFactoryManagerObject* self)
{
    delete self->vfm;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

extern "C" PyObject*
valueFactoryManagerAdd(ValueFactoryManagerObject* self, PyObject* args)
{
    assert(self->vfm);

    PyObject* factoryType = lookupType("types.FunctionType");
    assert(factoryType);

    PyObject* factory;
    PyObject* idObj;
    if (!PyArg_ParseTuple(args, "O!O", factoryType, &factory, &idObj))
    {
        return nullptr;
    }

    string id;
    if (!getStringArg(idObj, "id", id))
    {
        return nullptr;
    }

    (*self->vfm)->add(factory, id);
    if (PyErr_Occurred())
    {
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
valueFactoryManagerFind(ValueFactoryManagerObject* self, PyObject* args)
{
    assert(self->vfm);

    PyObject* idObj;
    if (!PyArg_ParseTuple(args, "O", &idObj))
    {
        return nullptr;
    }

    string id;
    if (!getStringArg(idObj, "id", id))
    {
        return nullptr;
    }

    return (*self->vfm)->findValueFactory(id);
}

static PyMethodDef ValueFactoryManagerMethods[] = {
    {"add", reinterpret_cast<PyCFunction>(valueFactoryManagerAdd), METH_VARARGS, PyDoc_STR("add(factory, id) -> None")},
    {"find", reinterpret_cast<PyCFunction>(valueFactoryManagerFind), METH_VARARGS, PyDoc_STR("find(id) -> function")},
    {0, 0} /* sentinel */
};

namespace IcePy
{
    PyTypeObject ValueFactoryManagerType = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(0, 0) "IcePy.ValueFactoryManager", /* tp_name */
        sizeof(ValueFactoryManagerObject),                       /* tp_basicsize */
        0,                                                       /* tp_itemsize */
        /* methods */
        reinterpret_cast<destructor>(valueFactoryManagerDealloc), /* tp_dealloc */
        0,                                                        /* tp_print */
        0,                                                        /* tp_getattr */
        0,                                                        /* tp_setattr */
        0,                                                        /* tp_reserved */
        0,                                                        /* tp_repr */
        0,                                                        /* tp_as_number */
        0,                                                        /* tp_as_sequence */
        0,                                                        /* tp_as_mapping */
        0,                                                        /* tp_hash */
        0,                                                        /* tp_call */
        0,                                                        /* tp_str */
        0,                                                        /* tp_getattro */
        0,                                                        /* tp_setattro */
        0,                                                        /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                                       /* tp_flags */
        0,                                                        /* tp_doc */
        0,                                                        /* tp_traverse */
        0,                                                        /* tp_clear */
        0,                                                        /* tp_richcompare */
        0,                                                        /* tp_weaklistoffset */
        0,                                                        /* tp_iter */
        0,                                                        /* tp_iternext */
        ValueFactoryManagerMethods,                               /* tp_methods */
        0,                                                        /* tp_members */
        0,                                                        /* tp_getset */
        0,                                                        /* tp_base */
        0,                                                        /* tp_dict */
        0,                                                        /* tp_descr_get */
        0,                                                        /* tp_descr_set */
        0,                                                        /* tp_dictoffset */
        0,                                                        /* tp_init */
        0,                                                        /* tp_alloc */
        reinterpret_cast<newfunc>(valueFactoryManagerNew),        /* tp_new */
        0,                                                        /* tp_free */
        0,                                                        /* tp_is_gc */
    };
}

bool
IcePy::initValueFactoryManager(PyObject* module)
{
    if (PyType_Ready(&ValueFactoryManagerType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &ValueFactoryManagerType; // Necessary to prevent GCC's strict-alias warnings.
    if (PyModule_AddObject(module, "ValueFactoryManager", reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}
