// Copyright (c) ZeroC, Inc.

#include "DefaultSliceLoader.h"
#include "Thread.h"
#include "Types.h"

#include <cassert>

using namespace std;
using namespace IcePy;

Ice::SliceLoaderPtr
IcePy::DefaultSliceLoader::instance()
{
    static Ice::SliceLoaderPtr instance{new DefaultSliceLoader()};
    return instance;
}

Ice::ValuePtr
IcePy::DefaultSliceLoader::newClassInstance(string_view typeId) const
{
    // The unmarshaling always runs with the GIL locked.
    assert(PyGILState_Check());

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
    if (!obj)
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    return make_shared<ValueReader>(obj.get(), info);
}

std::exception_ptr
IcePy::DefaultSliceLoader::newExceptionInstance(string_view typeId) const
{
    ExceptionInfoPtr info = lookupExceptionInfo(typeId);
    if (info)
    {
        return make_exception_ptr(ExceptionReader{info});
    }
    return nullptr;
}
