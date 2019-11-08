//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Options.h>
#include <Ice/Ice.h>
#include <Ice/Service.h>
#include <IceBox/Service.h>
#include <IceBox/ServiceManagerI.h>

using namespace std;
using namespace Ice;
using namespace IceBox;

using namespace std;

#ifdef  ICE_CPP11_MAPPING

IceBox::FailureException::~FailureException()
{
}

const ::std::string&
IceBox::FailureException::ice_staticId()
{
    static const ::std::string typeId = "::IceBox::FailureException";
    return typeId;
}

#else

IceBox::FailureException::FailureException(const char* file, int line) :
    ::Ice::LocalException(file, line)
{
}

IceBox::FailureException::FailureException(const char* file, int line, const ::std::string& iceP_reason) :
    ::Ice::LocalException(file, line),
    reason(iceP_reason)
{
}

IceBox::FailureException::~FailureException() throw()
{
}

::std::string
IceBox::FailureException::ice_id() const
{
    return "::IceBox::FailureException";
}

IceBox::FailureException*
IceBox::FailureException::ice_clone() const
{
    return new FailureException(*this);
}

void
IceBox::FailureException::ice_throw() const
{
    throw* this;
}

/// \cond INTERNAL
ICEBOX_API::Ice::LocalObject* IceBox::upCast(Service* p) { return p; }
/// \endcond

#endif //  ICE_CPP11_MAPPING

void
IceBox::FailureException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nservice failure exception: " << reason;
}

IceBox::Service::~Service()
{
}
