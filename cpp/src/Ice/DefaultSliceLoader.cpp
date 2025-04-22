// Copyright (c) ZeroC, Inc.

#include "Ice/DefaultSliceLoader.h"
#include "Ice/ValueF.h"

using namespace std;

IceInternal::DefaultSliceLoaderPtr
IceInternal::DefaultSliceLoader::instance()
{
    static DefaultSliceLoaderPtr instance{new DefaultSliceLoader};
    return instance;
}

IceInternal::DefaultSliceLoader::~DefaultSliceLoader() = default; // avoids weak vtable

Ice::ValuePtr
IceInternal::DefaultSliceLoader::newClassInstance(std::string_view typeId) const
{
    lock_guard lock{_mutex};
    auto p = _classFactories.find(typeId);
    if (p != _classFactories.end())
    {
        return p->second.first();
    }
    return nullptr;
}

std::exception_ptr
IceInternal::DefaultSliceLoader::newExceptionInstance(std::string_view typeId) const
{
    lock_guard lock{_mutex};
    auto p = _exceptionFactories.find(typeId);
    if (p != _exceptionFactories.end())
    {
        return p->second.first();
    }
    return nullptr;
}
