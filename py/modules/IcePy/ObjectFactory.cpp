// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _WIN32
#   include <IceUtil/Config.h>
#endif
#include <ObjectFactory.h>
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
    Lock sync(*this);

    //
    // Get the type information.
    //
    ClassInfoPtr info = lookupClassInfo(id);
    if(!info)
    {
        return 0;
    }

    //
    // Check if the application has registered a factory for this id.
    //
    FactoryMap::iterator p = _factoryMap.find(id);
    if(p != _factoryMap.end())
    {
        //
        // Invoke the create method on the Python factory object.
        //
        PyObjectHandle obj = PyObject_CallMethod(p->second, STRCAST("create"), STRCAST("s"), id.c_str());
        if(!obj.get())
        {
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
        throw AbortMarshaling();
    }

    return new ObjectReader(obj.get(), info);
}

void
IcePy::ObjectFactory::destroy()
{
    Lock sync(*this);

    //
    // We release the GIL before calling communicator->destroy(), so we must
    // reacquire it before calling back into Python.
    //
    AdoptThread adoptThread;

    for(FactoryMap::iterator p = _factoryMap.begin(); p != _factoryMap.end(); ++p)
    {
        //
        // Invoke the destroy method on each registered Python factory.
        //
        PyObjectHandle obj = PyObject_CallMethod(p->second, STRCAST("destroy"), 0);
        PyErr_Clear();
        Py_DECREF(p->second);
    }
    _factoryMap.clear();
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
