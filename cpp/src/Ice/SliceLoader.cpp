// Copyright (c) ZeroC, Inc.

#include "Ice/SliceLoader.h"

using namespace std;

Ice::SliceLoader::~SliceLoader() = default; // avoids weak vtable

Ice::ValuePtr
Ice::SliceLoader::newClassInstance(string_view) const
{
    return nullptr;
}

std::exception_ptr
Ice::SliceLoader::newExceptionInstance(string_view) const
{
    return nullptr;
}

Ice::CompositeSliceLoader::~CompositeSliceLoader() = default; // avoids weak vtable

void
Ice::CompositeSliceLoader::add(SliceLoaderPtr loader) noexcept
{
    _loaders.emplace_back(std::move(loader));
}

Ice::ValuePtr
Ice::CompositeSliceLoader::newClassInstance(string_view typeId) const
{
    for (const auto& loader : _loaders)
    {
        if (auto instance = loader->newClassInstance(typeId))
        {
            return instance;
        }
    }
    return nullptr;
}

std::exception_ptr
Ice::CompositeSliceLoader::newExceptionInstance(string_view typeId) const
{
    for (const auto& loader : _loaders)
    {
        if (auto instance = loader->newExceptionInstance(typeId))
        {
            return instance;
        }
    }
    return nullptr;
}
