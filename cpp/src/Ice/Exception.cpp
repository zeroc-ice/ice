//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Exception.h"
#include "Ice/LocalException.h"
#include "Ice/SlicedData.h"

#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
IceInternal::Ex::throwUOE(const string& expectedType, const shared_ptr<Ice::Value>& v)
{
    //
    // If the object is an unknown sliced object, we didn't find an
    // value factory, in this case raise a NoValueFactoryException
    // instead.
    //
    UnknownSlicedValue* usv = dynamic_cast<UnknownSlicedValue*>(v.get());
    if (usv)
    {
        throw NoValueFactoryException(__FILE__, __LINE__, "", usv->ice_id());
    }

    string type = v->ice_id();
    throw Ice::UnexpectedObjectException(
        __FILE__,
        __LINE__,
        "expected element of type `" + expectedType + "' but received `" + type + "'",
        type,
        expectedType);
}

void
IceInternal::Ex::throwMemoryLimitException(const char* file, int line, size_t requested, size_t maximum)
{
    ostringstream s;
    s << "requested " << requested << " bytes, maximum allowed is " << maximum << " bytes (see Ice.MessageSizeMax)";
    throw Ice::MemoryLimitException(file, line, s.str());
}

void
IceInternal::Ex::throwMarshalException(const char* file, int line, string reason)
{
    throw Ice::MarshalException{file, line, std::move(reason)};
}
