// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>

using namespace std;

namespace
{

class Client : public Ice::Application
{
public:
    virtual int
    run(int, char*[])
    {
        int count = communicator()->getProperties()->getPropertyAsInt("Client.Iterations");
        const string message = communicator()->getProperties()->getProperty("Client.Message");
        for(int i = 0; i < count; ++i)
        {
            communicator()->getLogger()->print(message);
        }
        return EXIT_SUCCESS;
    };
};

}

//
// We use messages with different sizes to compensate the different line end used in Windows
// and Unix. The Win32 message is 126 bytes plus 2 bytes for the Windows line end \r\n and
// that makes a total of 128 bytes. For all other platforms the message is 127 bytes plus 1
// byte for the line end \n and that makes a total of 128 bytes.  
//
#ifdef _WIN32
const string message = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                       "Vestibulum ornare, ex non bibendum hendrerit, felis tortor cras amet.";
#else
const string message = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                       "Fusce dictum turpis ante, sit amet finibus eros commodo vel. Sed amet.";
#endif

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif

    //
    // Run Client application 20 times, each times it generate 512 bytes of log messages,
    // the application logger is configured to archive log files larger than 512 bytes.
    //
    // This will generate 19 archived log files, all files including current log file
    // must have 512 bytes length.
    //
    for(int i = 0; i < 20; ++i)
    {
        Ice::InitializationData id;
        id.properties = Ice::createProperties();
        id.properties->load("config.client");
        id.properties->setProperty("Client.Iterations", "4");
        id.properties->setProperty("Client.Message", message);
        id.properties->setProperty("Ice.LogFile", "client5-0.log");
        id.properties->setProperty("Ice.LogFile.SizeMax", "512");
        Client c;
        if(c.main(argc, argv, id) != EXIT_SUCCESS)
        {
            return EXIT_FAILURE;
        }
    }
    
    //
    // Run Client application configured to generate 1024 bytes, the application is configured
    // to not archive log files, there must not be any archived log files, and log file must
    // have 1024 bytes size.
    //
    {
        Ice::InitializationData id;
        id.properties = Ice::createProperties();
        id.properties->load("config.client");
        id.properties->setProperty("Client.Iterations", "8");
        id.properties->setProperty("Client.Message", message);
        id.properties->setProperty("Ice.LogFile", "client5-1.log");
        id.properties->setProperty("Ice.LogFile.SizeMax", "0");
        Client c;
        if(c.main(argc, argv, id) != EXIT_SUCCESS)
        {
            return EXIT_FAILURE;
        }
    }

    //
    // Run Client application configured to generate 1024 bytes, the application is configured
    // to archive log files when size is greatert than 128 bytes, there should be 7 archived files
    // and current log file, all files must have 128 bytes size.
    //
    {
        Ice::InitializationData id;
        id.properties = Ice::createProperties();
        id.properties->load("config.client");
        id.properties->setProperty("Client.Iterations", "8");
        id.properties->setProperty("Client.Message", message);
        id.properties->setProperty("Ice.LogFile", "client5-2.log");
        id.properties->setProperty("Ice.LogFile.SizeMax", "128");
        Client c;
        if(c.main(argc, argv, id) != EXIT_SUCCESS)
        {
            return EXIT_FAILURE;
        }
    }
    
    //
    // Same as above but maximum size is lower than the message size, in this case we should
    // get the same result as messages are not trucated.
    //
    {
        Ice::InitializationData id;
        id.properties = Ice::createProperties();
        id.properties->load("config.client");
        id.properties->setProperty("Client.Iterations", "8");
        id.properties->setProperty("Client.Message", message);
        id.properties->setProperty("Ice.LogFile", "client5-3.log");
        id.properties->setProperty("Ice.LogFile.SizeMax", "64");
        Client c;
        if(c.main(argc, argv, id) != EXIT_SUCCESS)
        {
            return EXIT_FAILURE;
        }
    }
    
    //
    // Run Client application configured to generate 1024 bytes, the application is configured
    // to archive log files greater than 512 bytes, but the log directory is set to read only
    // after the log file is created, there must not be any archived log files and the log file
    // will contain an error indicating the failure to archive the log file
    //
    {
        Ice::InitializationData id;
        id.properties = Ice::createProperties();
        id.properties->load("config.client");
        id.properties->setProperty("Client.Iterations", "8");
        id.properties->setProperty("Client.Message", message);
        id.properties->setProperty("Ice.LogFile", "log/client5-4.log");
        id.properties->setProperty("Ice.LogFile.SizeMax", "512");
        Client c;
        if(c.main(argc, argv, id) != EXIT_SUCCESS)
        {
            return EXIT_FAILURE;
        }
    }
}
