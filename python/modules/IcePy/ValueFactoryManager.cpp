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
    auto* obj =
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
        auto p = _customFactories.find(typeId);
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

    auto p = _customFactories.find(id);
    if (p != _customFactories.end())
    {
        return p->second->getValueFactory();
    }

    return Py_None;
}

PyObject*
IcePy::ValueFactoryManager::getObject() const
{
    return Py_NewRef(_self.get());
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

IcePy::CustomValueFactory::CustomValueFactory(PyObject* valueFactory) : _valueFactory(Py_NewRef(valueFactory))
{
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
    return Py_NewRef(_valueFactory);
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
    auto* type = reinterpret_cast<PyTypeObject*>(info->pythonType);
    PyObjectHandle emptyArgs{PyTuple_New(0)};
    PyObjectHandle obj{type->tp_new(type, emptyArgs.get(), nullptr)};
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
    {} /* sentinel */
};

namespace IcePy
{
    // clang-format off
    PyTypeObject ValueFactoryManagerType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.ValueFactoryManager",
        .tp_basicsize = sizeof(ValueFactoryManagerObject),
        .tp_dealloc = reinterpret_cast<destructor>(valueFactoryManagerDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_methods = ValueFactoryManagerMethods,
        .tp_new = reinterpret_cast<newfunc>(valueFactoryManagerNew),
    };
    // clang-format on
}

bool
IcePy::initValueFactoryManager(PyObject* module)
{
    if (PyType_Ready(&ValueFactoryManagerType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "ValueFactoryManager", reinterpret_cast<PyObject*>(&ValueFactoryManagerType)) < 0)
    {
        return false;
    }

    return true;
}
