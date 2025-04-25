// Copyright (c) ZeroC, Inc.

#include "RubySliceLoader.h"
#include "Types.h"

#include <cassert>

using namespace std;

IceRuby::RubySliceLoader::RubySliceLoader(VALUE sliceLoader) : _sliceLoader{sliceLoader}
{
    // Mark the object as in use for the lifetime of this wrapper.
    rb_gc_register_address(&sliceLoader);
}

IceRuby::RubySliceLoader::~RubySliceLoader() { rb_gc_unregister_address(&_sliceLoader); }

Ice::ValuePtr
IceRuby::RubySliceLoader::newClassInstance(string_view typeId) const
{
    volatile VALUE str = createString(typeId);
    volatile VALUE obj = callRuby(rb_funcall, _sliceLoader, rb_intern("newInstance"), 1, str);
    if (NIL_P(obj))
    {
        return nullptr;
    }

    // When the type ID is ::Ice::Object, it indicates that the stream has not found a factory and is providing us an
    // opportunity to preserve the object.
    ClassInfoPtr info =
        typeId == Ice::Value::ice_staticId() ? lookupClassInfo("::Ice::UnknownSlicedValue") : lookupClassInfo(typeId);

    if (!info)
    {
        return nullptr;
    }

    return make_shared<ValueReader>(obj, info);
}
