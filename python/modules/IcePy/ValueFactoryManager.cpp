//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// PY_SSIZE_T_CLEAN is required for s#. Should we move it somewhere else, as it's always recommended to define it
// See https://docs.python.org/3/c-api/arg.html
#define PY_SSIZE_T_CLEAN
#include <ValueFactoryManager.h>
#include <Thread.h>
#include <Types.h>
#include <Ice/LocalException.h>

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

/* static */ ValueFactoryManagerPtr
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

IcePy::ValueFactoryManager::ValueFactoryManager() { _defaultFactory = make_shared<DefaultValueFactory>(); }

IcePy::ValueFactoryManager::~ValueFactoryManager()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.
    Py_XDECREF(_self);
}

void
IcePy::ValueFactoryManager::add(Ice::ValueFactoryFunc, string_view)
{
    throw Ice::FeatureNotSupportedException(__FILE__, __LINE__);
}

void
IcePy::ValueFactoryManager::add(Ice::ValueFactoryPtr f, string_view id)
{
    std::lock_guard lock(_mutex);

    if (id.empty())
    {
        if (_defaultFactory->getDelegate())
        {
            throw Ice::AlreadyRegisteredException(__FILE__, __LINE__, "value factory", string{id});
        }

        _defaultFactory->setDelegate(f);
    }
    else
    {
        FactoryMap::iterator p = _factories.find(id);
        if (p != _factories.end())
        {
            throw Ice::AlreadyRegisteredException(__FILE__, __LINE__, "value factory", string{id});
        }

        _factories.insert(FactoryMap::value_type(string{id}, std::move(f)));
    }
}

Ice::ValueFactoryFunc
IcePy::ValueFactoryManager::find(string_view id) const noexcept
{
    Ice::ValueFactoryPtr factory = findCore(id);

    if (factory)
    {
        return [factory](string_view type) -> shared_ptr<Ice::Value> { return factory->create(type); };
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
        add(make_shared<FactoryWrapper>(valueFactory), id);
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
    }
}

PyObject*
IcePy::ValueFactoryManager::findValueFactory(string_view id) const
{
    Ice::ValueFactoryPtr factory = findCore(id);
    if (factory)
    {
        auto w = dynamic_pointer_cast<FactoryWrapper>(factory);
        if (w)
        {
            return w->getValueFactory();
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject*
IcePy::ValueFactoryManager::getObject() const
{
    Py_INCREF(_self);
    return _self;
}

void
IcePy::ValueFactoryManager::destroy()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    FactoryMap factories;

    {
        std::lock_guard lock(_mutex);
        if (_self == 0)
        {
            //
            // Nothing to do if already destroyed (this can occur if communicator destroy is called multiple times)
            //
            return;
        }
        //
        // Break the cyclic reference.
        //
        Py_DECREF(_self);
        _self = 0;

        factories.swap(_factories);
    }

    for (FactoryMap::iterator p = factories.begin(); p != factories.end(); ++p)
    {
        auto w = dynamic_pointer_cast<FactoryWrapper>(p->second);
        if (w)
        {
            w->destroy();
        }
    }

    _defaultFactory->destroy();
}

Ice::ValueFactoryPtr
IcePy::ValueFactoryManager::findCore(string_view id) const noexcept
{
    std::lock_guard lock(_mutex);

    Ice::ValueFactoryPtr factory;

    if (id.empty())
    {
        return _defaultFactory;
    }
    else
    {
        FactoryMap::const_iterator p = _factories.find(id);
        if (p != _factories.end())
        {
            return p->second;
        }
    }
    return nullptr;
}

IcePy::FactoryWrapper::FactoryWrapper(PyObject* valueFactory) : _valueFactory(valueFactory)
{
    Py_INCREF(_valueFactory);
    assert(_valueFactory != Py_None); // This should always be present.
}

IcePy::FactoryWrapper::~FactoryWrapper() { Py_DECREF(_valueFactory); }

shared_ptr<Ice::Value>
IcePy::FactoryWrapper::create(string_view id)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    //
    // Get the type information.
    //
    ValueInfoPtr info = getValueInfo(id);

    if (!info)
    {
        return 0;
    }

    PyObjectHandle obj = PyObject_CallFunction(_valueFactory, "s#", id.data(), static_cast<Py_ssize_t>(id.size()));

    if (!obj.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    if (obj.get() == Py_None)
    {
        return 0;
    }

    return make_shared<ValueReader>(obj.get(), info);
    ;
}

PyObject*
IcePy::FactoryWrapper::getValueFactory() const
{
    Py_INCREF(_valueFactory);
    return _valueFactory;
}

void
IcePy::FactoryWrapper::destroy()
{
}

shared_ptr<Ice::Value>
IcePy::DefaultValueFactory::create(std::string_view id)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    shared_ptr<Ice::Value> v;

    //
    // Give the application-provided default factory a chance to create the object first.
    //
    if (_delegate)
    {
        v = _delegate->create(id);
        if (v)
        {
            return v;
        }
    }

    //
    // Get the type information.
    //
    ValueInfoPtr info = getValueInfo(id);

    if (!info)
    {
        return 0;
    }

    //
    // Instantiate the object.
    //
    PyTypeObject* type = reinterpret_cast<PyTypeObject*>(info->pythonType);
    PyObjectHandle args = PyTuple_New(0);
    PyObjectHandle obj = type->tp_new(type, args.get(), 0);
    if (!obj.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    return make_shared<ValueReader>(obj.get(), info);
}

void
IcePy::DefaultValueFactory::setDelegate(const Ice::ValueFactoryPtr& d)
{
    _delegate = d;
}

PyObject*
IcePy::DefaultValueFactory::getValueFactory() const
{
    if (_delegate)
    {
        auto w = dynamic_pointer_cast<FactoryWrapper>(_delegate);
        if (w)
        {
            return w->getValueFactory();
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}

void
IcePy::DefaultValueFactory::destroy()
{
    if (_delegate)
    {
        auto w = dynamic_pointer_cast<FactoryWrapper>(_delegate);
        if (w)
        {
            w->destroy();
        }
    }

    _delegate = 0;
}

#ifdef WIN32
extern "C"
#endif
    static ValueFactoryManagerObject*
    valueFactoryManagerNew(PyTypeObject* /*type*/, PyObject* /*args*/, PyObject* /*kwds*/)
{
    PyErr_Format(PyExc_RuntimeError, STRCAST("Do not instantiate this object directly"));
    return 0;
}

#ifdef WIN32
extern "C"
#endif
    static void
    valueFactoryManagerDealloc(ValueFactoryManagerObject* self)
{
    delete self->vfm;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    valueFactoryManagerAdd(ValueFactoryManagerObject* self, PyObject* args)
{
    assert(self->vfm);

    PyObject* factoryType = lookupType("types.FunctionType");
    assert(factoryType);

    PyObject* factory;
    PyObject* idObj;
    if (!PyArg_ParseTuple(args, STRCAST("O!O"), factoryType, &factory, &idObj))
    {
        return 0;
    }

    string id;
    if (!getStringArg(idObj, "id", id))
    {
        return 0;
    }

    (*self->vfm)->add(factory, id);
    if (PyErr_Occurred())
    {
        return 0;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    valueFactoryManagerFind(ValueFactoryManagerObject* self, PyObject* args)
{
    assert(self->vfm);

    PyObject* idObj;
    if (!PyArg_ParseTuple(args, STRCAST("O"), &idObj))
    {
        return 0;
    }

    string id;
    if (!getStringArg(idObj, "id", id))
    {
        return 0;
    }

    return (*self->vfm)->findValueFactory(id);
}

static PyMethodDef ValueFactoryManagerMethods[] = {
    {STRCAST("add"), reinterpret_cast<PyCFunction>(valueFactoryManagerAdd), METH_VARARGS,
     PyDoc_STR(STRCAST("add(factory, id) -> None"))},
    {STRCAST("find"), reinterpret_cast<PyCFunction>(valueFactoryManagerFind), METH_VARARGS,
     PyDoc_STR(STRCAST("find(id) -> function"))},
    {0, 0} /* sentinel */
};

namespace IcePy
{
    PyTypeObject ValueFactoryManagerType = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(0, 0) STRCAST("IcePy.ValueFactoryManager"), /* tp_name */
        sizeof(ValueFactoryManagerObject),                                /* tp_basicsize */
        0,                                                                /* tp_itemsize */
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
    if (PyModule_AddObject(module, STRCAST("ValueFactoryManager"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}
