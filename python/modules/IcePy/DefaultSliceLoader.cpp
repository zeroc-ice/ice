// Copyright (c) ZeroC, Inc.

#include "DefaultSliceLoader.h"
#include "Thread.h"
#include "Types.h"

using namespace std;
using namespace IcePy;

Ice::ValuePtr
IcePy::DefaultSliceLoader::newClassInstance(string_view typeId) const
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    // Get the type information.
    ValueInfoPtr info =
        typeId == Ice::Value::ice_staticId() ? lookupValueInfo("::Ice::UnknownSlicedValue") : lookupValueInfo(typeId);

    if (!info)
    {
        return nullptr;
    }

    // Instantiate the object.
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
