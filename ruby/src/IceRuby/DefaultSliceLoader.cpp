// Copyright (c) ZeroC, Inc.

#include "DefaultSliceLoader.h"
#include "Types.h"
#include "Util.h"

using namespace std;
using namespace IceRuby;

Ice::SliceLoaderPtr
IceRuby::DefaultSliceLoader::instance()
{
    static Ice::SliceLoaderPtr instance{new DefaultSliceLoader()};
    return instance;
}

Ice::ValuePtr
IceRuby::DefaultSliceLoader::newClassInstance(string_view typeId) const
{
    // When the type ID is ::Ice::Object, it indicates that the stream has not found a factory and is providing us an
    // opportunity to preserve the object.
    ClassInfoPtr info =
        typeId == Ice::Value::ice_staticId() ? lookupClassInfo("::Ice::UnknownSlicedValue") : lookupClassInfo(typeId);

    if (!info)
    {
        return nullptr;
    }

    volatile VALUE obj = callRuby(rb_class_new_instance, 0, reinterpret_cast<VALUE*>(0), info->rubyClass);
    assert(!NIL_P(obj));

    return make_shared<ValueReader>(obj, info);
}

std::exception_ptr
IceRuby::DefaultSliceLoader::newExceptionInstance(string_view typeId) const
{
    ExceptionInfoPtr info = lookupExceptionInfo(typeId);
    if (info)
    {
        return make_exception_ptr(ExceptionReader{info});
    }
    return nullptr;
}
