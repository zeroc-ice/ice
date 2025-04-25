// Copyright (c) ZeroC, Inc.

#include "DefaultSliceLoader.h"
#include "Types.h"
#include "Util.h"

using namespace std;
using namespace IcePHP;

IcePHP::DefaultSliceLoader::DefaultSliceLoader(const CommunicatorInfoPtr& communicatorInfo)
    : _communicatorInfo(communicatorInfo)
{
}

Ice::ValuePtr
IcePHP::DefaultSliceLoader::newClassInstance(string_view typeId) const
{
    ClassInfoPtr cls =
        typeId == Ice::Value::ice_staticId() ? getClassInfoById("::Ice::UnknownSlicedValue") : getClassInfoById(typeId);

    if (!cls)
    {
        return nullptr;
    }

    CommunicatorInfoPtr communicatorInfo = _communicatorInfo.lock();
    if (!communicatorInfo)
    {
        return nullptr; // Communicator has been destroyed.
    }

    // Instantiate the object.
    zval obj;

    if (object_init_ex(&obj, const_cast<zend_class_entry*>(cls->zce)) != SUCCESS)
    {
        throw AbortMarshaling();
    }

#ifdef NDEBUG
    // BUGFIX: releasing this object triggers an assert in PHP objects_store
    // https://github.com/php/php-src/issues/10593
    AutoDestroy release(&obj);
#endif
    if (!invokeMethod(&obj, ZEND_CONSTRUCTOR_FUNC_NAME))
    {
        throw AbortMarshaling();
    }

    return make_shared<ValueReader>(&obj, cls, communicatorInfo);
}

std::exception_ptr
IcePHP::DefaultSliceLoader::newExceptionInstance(string_view typeId) const
{
    ExceptionInfoPtr info = getExceptionInfo(typeId);

    if (!info)
    {
        return nullptr;
    }

    CommunicatorInfoPtr communicatorInfo = _communicatorInfo.lock();
    if (!communicatorInfo)
    {
        return nullptr; // Communicator has been destroyed.
    }

    return make_exception_ptr(ExceptionReader{communicatorInfo, info});
}
