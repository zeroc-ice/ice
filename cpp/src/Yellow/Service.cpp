// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Yellow/AdminI.h>
#include <Yellow/QueryI.h>

#include <IceBox/IceBox.h>

#if defined(_WIN32)
#   define YELLOW_SERVICE_API __declspec(dllexport)
#else
#   define YELLOW_SERVICE_API /**/
#endif

using namespace std;
using namespace Ice;
using namespace Yellow;
using namespace Freeze;

namespace Yellow
{

class YELLOW_SERVICE_API ServiceI : public ::IceBox::FreezeService
{
public:

    ServiceI();
    virtual ~ServiceI();

    virtual void start(const string&,
		       const CommunicatorPtr&,
		       const StringSeq&,
		       const ::Freeze::DBEnvironmentPtr&);

    virtual void stop();

private:

    ObjectAdapterPtr _queryAdapter;
    ObjectAdapterPtr _adminAdapter;
};

} // End namespace Yellow

extern "C"
{

//
// Factory function
//
YELLOW_SERVICE_API ::IceBox::FreezeService*
create(Ice::CommunicatorPtr communicator)
{
    return new Yellow::ServiceI;
}

}

Yellow::ServiceI::ServiceI()
{
}

Yellow::ServiceI::~ServiceI()
{
}

void
Yellow::ServiceI::start(const string& name,
			const CommunicatorPtr& communicator,
			const StringSeq& args,
			const DBEnvironmentPtr& dbEnv)
{
    //
    // The Admin & Query interface implementations each have their own
    // map. This is safe because a) no iterators are used on the
    // database, and b) internally the database is synchronized.
    //
    DBPtr dbYellow = dbEnv->openDB("yellow", true);

    //
    // With respect to the adapters I want to support three use-cases:
    //
    // 1) admin is not permitted.
    // 2) both admin & query is permitted on the same endpoint
    // 3) query & admin are permitted, but on seperate endpoitns
    //
    // TODO: At present #2 isn't supported.
    //
    _queryAdapter = communicator->createObjectAdapter(name + ".Query");

    string adminEndpoints = communicator->getProperties()->getProperty(name + ".Admin.Endpoints");
    if(!adminEndpoints.empty())
    {
	_adminAdapter = communicator->createObjectAdapter(name + ".Admin");
	ObjectPtr admin = new AdminI(dbYellow);
	_adminAdapter->add(admin, stringToIdentity(name + "/Admin"));
    }

    ObjectPtr query = new QueryI(dbYellow);
    _queryAdapter->add(query, stringToIdentity(name + "/Query"));

    if(_adminAdapter)
    {
	_adminAdapter->activate();
    }
    _queryAdapter->activate();
}

void
Yellow::ServiceI::stop()
{
    if(_adminAdapter)
    {
	_adminAdapter->deactivate();
    }
    _queryAdapter->deactivate();
}
