// Copyright (c) ZeroC, Inc.

#include "PySliceLoader.h"
#include "Types.h"

#include <cassert>

using namespace std;

IcePy::PySliceLoader::PySliceLoader(PyObject* sliceLoader) : _sliceLoader{Py_NewRef(sliceLoader)}
{
    assert(_sliceLoader);
    assert(_sliceLoader != Py_None);

    if (!PyCallable_Check(_sliceLoader))
    {
        throw Ice::InitializationException{__FILE__, __LINE__, "sliceLoader must be a callable"};
    }
}

IcePy::PySliceLoader::~PySliceLoader()
{
    // Called by destroy/destroyAsync with the GIL locked.
    assert(PyGILState_Check());
    Py_DECREF(_sliceLoader);
}

Ice::ValuePtr
IcePy::PySliceLoader::newClassInstance(string_view typeId) const
{
    // The unmarshaling always runs with the GIL locked.
    assert(PyGILState_Check());

    PyObjectHandle obj{
        PyObject_CallFunction(_sliceLoader, "s#", typeId.data(), static_cast<Py_ssize_t>(typeId.size()))};

    if (!obj)
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    if (obj.get() == Py_None)
    {
        return nullptr;
    }

    // Get the type information.
    ValueInfoPtr info =
        typeId == Ice::Value::ice_staticId() ? lookupValueInfo("::Ice::UnknownSlicedValue") : lookupValueInfo(typeId);

    if (!info)
    {
        return nullptr;
    }

    return make_shared<ValueReader>(obj.get(), info);
}
