// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _WIN32
#   include <IceUtil/Config.h>
#endif
#include <ObjectFactory.h>
#include <Thread.h>
#include <Types.h>
#include <Util.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace IcePy;

IcePy::ObjectFactory::ObjectFactory()
{
}

IcePy::ObjectFactory::~ObjectFactory()
{
    assert(_valueFactoryMap.empty());
    assert(_objectFactoryMap.empty());
}

Ice::ObjectPtr
IcePy::ObjectFactory::create(const string& id)
{
    PyObject* factory = 0;

    //
    // Check if the application has registered a factory for this id.
    //
    {
        Lock sync(*this);

        FactoryMap::iterator p = _valueFactoryMap.find(id);
        if(p != _valueFactoryMap.end())
        {
            factory = p->second;
        }
    }

    //
    // Get the type information.
    //
    ClassInfoPtr info;
    if(id == Ice::Object::ice_staticId())
    {
        //
        // When the ID is that of Ice::Object, it indicates that the stream has not
        // found a factory and is providing us an opportunity to preserve the object.
        //
        info = lookupClassInfo("::Ice::UnknownSlicedObject");
    }
    else
    {
        info = lookupClassInfo(id);
    }

    if(!info)
    {
        return 0;
    }

    if(factory)
    {
        //
        // Invoke the create method on the Python factory object.
        //
        PyObjectHandle obj = PyObject_CallMethod(factory, STRCAST("create"), STRCAST("s"), id.c_str());
        if(!obj.get())
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }
        if(obj.get() == Py_None)
        {
            return 0;
        }
        return new ObjectReader(obj.get(), info);
    }

    //
    // If the requested type is an abstract class, then we give up.
    //
    if(info->isAbstract)
    {
        return 0;
    }

    //
    // Instantiate the object.
    //
    PyTypeObject* type = reinterpret_cast<PyTypeObject*>(info->pythonType.get());
    PyObjectHandle args = PyTuple_New(0);
    PyObjectHandle obj = type->tp_new(type, args.get(), 0);
    if(!obj.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    return new ObjectReader(obj.get(), info);
}

void
IcePy::ObjectFactory::destroy()
{
    FactoryMap valueFactories;
    FactoryMap objectFactories;

    {
        Lock sync(*this);
        objectFactories = _objectFactoryMap;
        valueFactories = _valueFactoryMap;
        _valueFactoryMap.clear();
        _objectFactoryMap.clear();
    }

    //
    // We release the GIL before calling communicator->destroy(), so we must
    // reacquire it before calling back into Python.
    //
    AdoptThread adoptThread;

    for(FactoryMap::iterator p = _objectFactoryMap.begin(); p != _objectFactoryMap.end(); ++p)
    {
        //
        // Invoke the destroy method on each registered Python "object" factory.
        //
        PyObjectHandle obj = PyObject_CallMethod(p->second, STRCAST("destroy"), 0);
        PyErr_Clear(); // Ignore errors.
        Py_DECREF(p->second);
    }
    for(FactoryMap::iterator p = _valueFactoryMap.begin(); p != _valueFactoryMap.end(); ++p)
    {
        PyErr_Clear(); // Ignore errors.
        Py_DECREF(p->second);
    }
}

bool
IcePy::ObjectFactory::addValueFactory(PyObject* factory, const string& id)
{
    Lock sync(*this);

    FactoryMap::iterator p = _valueFactoryMap.find(id);
    if(p != _valueFactoryMap.end())
    {
        Ice::AlreadyRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "value factory";
        ex.id = id;
        setPythonException(ex);
        return false;
    }

    _valueFactoryMap.insert(FactoryMap::value_type(id, factory));
    Py_INCREF(factory);

    return true;
}

bool
IcePy::ObjectFactory::addObjectFactory(PyObject* factory, const string& id)
{
    Lock sync(*this);

    FactoryMap::iterator p = _valueFactoryMap.find(id);
    if(p != _valueFactoryMap.end())
    {
        Ice::AlreadyRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "value factory";
        ex.id = id;
        setPythonException(ex);
        return false;
    }

    _valueFactoryMap.insert(FactoryMap::value_type(id, factory));
    Py_INCREF(factory);
    _objectFactoryMap.insert(FactoryMap::value_type(id, factory));
    Py_INCREF(factory);

    return true;
}

PyObject*
IcePy::ObjectFactory::findValueFactory(const string& id)
{
    Lock sync(*this);

    FactoryMap::iterator p = _valueFactoryMap.find(id);
    if(p == _valueFactoryMap.end())
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    Py_INCREF(p->second);
    return p->second;
}

PyObject*
IcePy::ObjectFactory::findObjectFactory(const string& id)
{
    Lock sync(*this);

    FactoryMap::iterator p = _objectFactoryMap.find(id);
    if(p == _objectFactoryMap.end())
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    Py_INCREF(p->second);
    return p->second;
}
