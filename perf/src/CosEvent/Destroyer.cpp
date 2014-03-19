// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <orbsvcs/CosEventChannelAdminC.h>

class Destroyer
{
public:
    int run(int argc, char* argv[]);
};

int
main(int argc, char* argv[])
{
    Destroyer destroyer;
    return destroyer.run(argc, argv);
}

int 
Destroyer::run(int argc, char* argv[])
{
    CORBA::String_var ior;
    for(int i = 1; i < argc; i++)
    {
        if(strlen(argv[i]) > 3 && strncmp(argv[i], "IOR", strlen("IOR")))
        {
            ior = CORBA::string_dup(argv[i]);
        }
    }

    ACE_DECLARE_NEW_CORBA_ENV;
    ACE_TRY
    {
        CORBA::ORB_var orb = CORBA::ORB_init(argc, argv, "" ACE_ENV_ARG_PARAMETER);
        ACE_TRY_CHECK;

        if(argc <= 1)
        {
            ACE_ERROR ((LM_ERROR, "Usage: Destroyer <event_channel_ior>\n"));
            return 1;
        }

        CORBA::Object_var object = orb->string_to_object(ior.in() ACE_ENV_ARG_PARAMETER);
        ACE_TRY_CHECK;

        CosEventChannelAdmin::EventChannel_var event_channel = 
            CosEventChannelAdmin::EventChannel::_narrow(object.in() ACE_ENV_ARG_PARAMETER);
        ACE_TRY_CHECK;

        // Destroy the EC....
        event_channel->destroy(ACE_ENV_SINGLE_ARG_PARAMETER);
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
