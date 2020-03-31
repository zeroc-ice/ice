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

IceBox::FailureException::~FailureException()
{
}

const ::std::string&
IceBox::FailureException::ice_staticId()
{
    static const ::std::string typeId = "::IceBox::FailureException";
    return typeId;
}

void
IceBox::FailureException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nservice failure exception: " << reason;
}

IceBox::Service::~Service()
{
}
