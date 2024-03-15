//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Exception.h>
#include <Ice/LocalException.h>
#include <Ice/Network.h>
#include <Ice/Plugin.h>
#include <Ice/SlicedData.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>
#include <Ice/Initialize.h>
#include <IceUtil/StringUtil.h>
#include <iomanip>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace IceInternal
{
    namespace Ex
    {
        void throwUOE(const string& expectedType, const shared_ptr<Ice::Value>& v)
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

        void throwMemoryLimitException(const char* file, int line, size_t requested, size_t maximum)
        {
            ostringstream s;
            s << "requested " << requested << " bytes, maximum allowed is " << maximum
              << " bytes (see Ice.MessageSizeMax)";
            throw Ice::MemoryLimitException(file, line, s.str());
        }

        void throwMarshalException(const char* file, int line, const string& reason)
        {
            throw Ice::MarshalException(file, line, reason);
        }

    }
}

namespace
{
    const string userException_ids[] = {"::Ice::UserException"};
}

std::string_view
Ice::UserException::ice_staticId() noexcept
{
    return userException_ids[0];
}

void
Ice::UserException::_write(::Ice::OutputStream* os) const
{
    os->startException();
    _writeImpl(os);
    os->endException();
}

void
Ice::UserException::_read(::Ice::InputStream* is)
{
    is->startException();
    _readImpl(is);
    is->endException();
}

bool
Ice::UserException::_usesClasses() const
{
    return false;
}
