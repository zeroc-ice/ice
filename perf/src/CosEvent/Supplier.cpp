// Supplier.cpp,v 1.4 2003/11/02 23:27:21 dhinton Exp

#include "Supplier.h"
#include "orbsvcs/CosEventChannelAdminS.h"
#include "ace/OS_NS_unistd.h"
#include "ace/Date_Time.h"
#include "PerfC.h"
#include "PerfS.h"

//
// A reference counted implementation
//
class Intf_impl : public POA_Perf::Intf, public PortableServer::RefCountServantBase
{
public:

    Intf_impl(PortableServer::POA_ptr poa) : 
	_poa(PortableServer::POA::_duplicate(poa)) 
    {
    }

    virtual PortableServer::POA_ptr _default_POA() { return PortableServer::POA::_duplicate(_poa); }

private:

    PortableServer::POA_var _poa;

};

int
main (int argc, char* argv[])
{
    Supplier supplier;
    return supplier.run (argc, argv);
}

Supplier::Supplier (void)
{
}

int
Supplier::run (int argc, char* argv[])
{
    int period = 0;
    int repetitions = 10000;
    bool twoway = false;
    char* ior = 0;
    for(int i = 1; i < argc; i++)
    {
	if(strcmp(argv[i], "-p") == 0)
	{
	    period = atoi(argv[++i]);
	}
	else if(strcmp(argv[i], "-r") == 0)
	{
	    repetitions = atoi(argv[++i]);
	}
	else if(strcmp(argv[i], "-t") == 0)
	{
	    twoway = true;
	}
	else if(strlen(argv[i]) > 3 && argv[i][0] == 'I' && argv[i][1] == 'O' && argv[i][2] == 'R')
	{
	    ior = strdup(argv[i]);
	}
    }

    ACE_DECLARE_NEW_CORBA_ENV;
    ACE_TRY
    {
	CORBA::ORB_var orb = CORBA::ORB_init (argc, argv, "" ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	if (argc <= 1)
	{
	    ACE_ERROR ((LM_ERROR, "Usage: Supplier <event_channel_ior>\n"));
	    return 1;
	}
	
	CORBA::Object_var object = orb->resolve_initial_references ("RootPOA" ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	PortableServer::POA_var poa = PortableServer::POA::_narrow (object.in () ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	PortableServer::POAManager_var poa_manager = poa->the_POAManager (ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;
	poa_manager->activate (ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;

	object = orb->string_to_object (ior ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	CosEventChannelAdmin::EventChannel_var event_channel = 
	    CosEventChannelAdmin::EventChannel::_narrow (object.in () ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	CosEventChannelAdmin::SupplierAdmin_var supplier_admin =
	    event_channel->for_suppliers (ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;

	CosEventChannelAdmin::ProxyPushConsumer_var consumer =
	    supplier_admin->obtain_push_consumer (ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	CosEventComm::PushSupplier_var supplier = this->_this (ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	consumer->connect_push_supplier (supplier.in () ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	//
	// Create a servant to obtain a persistent reference.
	//
	CORBA::PolicyList pl(2);
	pl.length(2);
	pl[0] = poa -> create_lifespan_policy(PortableServer::PERSISTENT);
	pl[1] = poa -> create_id_assignment_policy(PortableServer::USER_ID);
	PortableServer::POA_var intfPOA = poa -> create_POA("intf", poa_manager, pl);

	Intf_impl* intfImpl = new Intf_impl(intfPOA);
	PortableServer::ServantBase_var servant = intfImpl;

	PortableServer::ObjectId_var oid = PortableServer::string_to_ObjectId("intf");
	intfPOA -> activate_object_with_id(oid, intfImpl);
	CORBA::Object_var obj = intfImpl -> _this();
	Perf::Intf_var intf = Perf::Intf::_narrow(obj);
	
	ACE_Time_Value sleep_time(0, period * 1000); // 10 milliseconds

	timeval tv;
	
	for (int i = 0; i < repetitions; ++i)
	{
	    CORBA::Any event;
	    gettimeofday(&tv, 0);
	    event <<= CORBA::LongLong(tv.tv_sec * static_cast<long long>(1000000) + tv.tv_usec);
	
	    consumer->push (event ACE_ENV_ARG_PARAMETER);
	    ACE_TRY_CHECK;
	    ACE_OS::sleep (sleep_time);
	}

	for (int i = 0; i < repetitions; ++i)
	{
	    Perf::Event e;
	    e.e = Perf::A;
	    e.i = 10;
	    e.s.s = "TEST";
	    e.ref = intf;
	    gettimeofday(&tv, 0);
	    e.time = tv.tv_sec * static_cast<long long>(1000000) + tv.tv_usec;

	    CORBA::Any event;
	    event <<= e;	
	    consumer->push (event ACE_ENV_ARG_PARAMETER);
	    ACE_TRY_CHECK;
	    ACE_OS::sleep (sleep_time);
	}
	
	// Disconnect from the EC
	consumer->disconnect_push_consumer (ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;

	ACE_Time_Value wait_time(0, 1000 * 1000 * 5); // 10 seconds
	ACE_OS::sleep (wait_time);
	
	// Destroy the EC....
	event_channel->destroy (ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	// Deactivate this object...
	PortableServer::ObjectId_var id = poa->servant_to_id (this ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	poa->deactivate_object (id.in () ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	// Destroy the POA
	poa->destroy (1, 0 ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
    }
    ACE_CATCHANY
    {
	ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION, "Supplier::run");
	return 1;
    }
    ACE_ENDTRY;
    return 0;
}

void
Supplier::disconnect_push_supplier (ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
    ACE_THROW_SPEC ((CORBA::SystemException))
{
}

// ****************************************************************

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
#elif defined(ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */
