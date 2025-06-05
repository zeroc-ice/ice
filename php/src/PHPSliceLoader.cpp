// Copyright (c) ZeroC, Inc.

#include "PHPSliceLoader.h"
#include "Types.h"
#include "Util.h"

#include <cassert>

using namespace std;

IcePHP::PHPSliceLoader::PHPSliceLoader(zval* sliceLoader, const CommunicatorInfoPtr& communicatorInfo)
    : _communicatorInfo{communicatorInfo}
{
    ZVAL_COPY(&_sliceLoader, sliceLoader);
}

IcePHP::PHPSliceLoader::~PHPSliceLoader() { zval_ptr_dtor(&_sliceLoader); }

Ice::ValuePtr
IcePHP::PHPSliceLoader::newClassInstance(string_view typeId) const
{
    zval arg;
    AutoDestroy destroyArg(&arg);
    ZVAL_STRINGL(&arg, typeId.data(), static_cast<int>(typeId.length()));

    zval obj;
    ZVAL_UNDEF(&obj);

    zend_try
    {
        assert(Z_TYPE(_sliceLoader) == IS_OBJECT);
        zend_call_method(
            Z_OBJ_P(&_sliceLoader),
            0,
            0,
            const_cast<char*>("newInstance"),
            sizeof("newInstance") - 1,
            &obj,
            1,
            &arg,
            0);
    }
    zend_catch
    {
        // We throw AbortMarshaling below in case of an exception in the PHP call.
    }
    zend_end_try();

    // Bail out if an exception has already been thrown.
    if (Z_ISUNDEF(obj) || EG(exception))
    {
        throw AbortMarshaling();
    }

    AutoDestroy destroyObj(&obj);

    if (Z_TYPE(obj) == IS_NULL)
    {
        return nullptr;
    }

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

    return make_shared<ValueReader>(&obj, cls, communicatorInfo);
}
