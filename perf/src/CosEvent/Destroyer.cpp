// Destroyer.cpp,v 1.4 2003/11/02 23:27:21 dhinton Exp

#include "orbsvcs/CosEventChannelAdminS.h"
#include "ace/OS_NS_unistd.h"
#include "ace/Date_Time.h"
#include "PerfC.h"
#include "PerfS.h"

class Destroyer
{
public:

    Destroyer(void);
    int run (int argc, char* argv[]);
};

int
main (int argc, char* argv[])
{
    Destroyer destroyer;
    return destroyer.run (argc, argv);
}

Destroyer::Destroyer (void)
{
}

int
Destroyer::run (int argc, char* argv[])
{
    char* ior = 0;
    for(int i = 1; i < argc; i++)
    {
	if(strlen(argv[i]) > 3 && argv[i][0] == 'I' && argv[i][1] == 'O' && argv[i][2] == 'R')
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
	    ACE_ERROR ((LM_ERROR, "Usage: Destroyer <event_channel_ior>\n"));
	    return 1;
	}
	
	CORBA::Object_var object = orb->string_to_object (ior ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	CosEventChannelAdmin::EventChannel_var event_channel = 
	    CosEventChannelAdmin::EventChannel::_narrow (object.in () ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	// Destroy the EC....
	event_channel->destroy (ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;	
    }
    ACE_CATCHANY
    {
	ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION, "Destroyer::run");
	return 1;
    }
    ACE_ENDTRY;
    return 0;
}

// ****************************************************************

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
#elif defined(ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */
