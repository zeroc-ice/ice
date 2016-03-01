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
    assert(_factoryMap.empty());
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

        FactoryMap::iterator p = _factoryMap.find(id);
        if(p != _factoryMap.end())
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
    FactoryMap factories;

    {
        Lock sync(*this);
        factories = _factoryMap;
        _factoryMap.clear();
    }

    //
    // We release the GIL before calling communicator->destroy(), so we must
    // reacquire it before calling back into Python.
    //
    AdoptThread adoptThread;

    for(FactoryMap::iterator p = factories.begin(); p != factories.end(); ++p)
    {
        //
        // Invoke the destroy method on each registered Python factory.
        //
        PyObjectHandle obj = PyObject_CallMethod(p->second, STRCAST("destroy"), 0);
        PyErr_Clear(); // Ignore errors.
        Py_DECREF(p->second);
    }
}

bool
IcePy::ObjectFactory::add(PyObject* factory, const string& id)
{
    Lock sync(*this);

    FactoryMap::iterator p = _factoryMap.find(id);
    if(p != _factoryMap.end())
    {
        Ice::AlreadyRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "object factory";
        ex.id = id;
        setPythonException(ex);
        return false;
    }

    _factoryMap.insert(FactoryMap::value_type(id, factory));
    Py_INCREF(factory);

    return true;
}

bool
IcePy::ObjectFactory::remove(const string& id)
{
    Lock sync(*this);

    FactoryMap::iterator p = _factoryMap.find(id);
    if(p == _factoryMap.end())
    {
        Ice::NotRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "object factory";
        ex.id = id;
        setPythonException(ex);
        return false;
    }

    Py_DECREF(p->second);
    _factoryMap.erase(p);

    return true;
}

PyObject*
IcePy::ObjectFactory::find(const string& id)
{
    Lock sync(*this);

    FactoryMap::iterator p = _factoryMap.find(id);
    if(p == _factoryMap.end())
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    Py_INCREF(p->second);
    return p->second;
}
