// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <Ice/Ice.h>
#include <Ice/Service.h>
#include <IceBox/ServiceManagerI.h>

using namespace std;
using namespace Ice;
using namespace IceBox;

namespace IceBox
{

class IceBoxService : public Ice::Service
{
public:

    IceBoxService();

protected:

    virtual bool start(int, char*[], int&);
    virtual bool stop();

private:

    void usage(const std::string&);
    ServiceManagerIPtr _serviceManager;
};

}

IceBox::IceBoxService::IceBoxService()
{
}

bool
IceBox::IceBoxService::start(int argc, char* argv[], int& status)
{
    // Run through the command line arguments removing all the service
    // properties.
    vector<string> args = Ice::argsToStringSeq(argc, argv);
    PropertiesPtr properties = communicator()->getProperties();
    const string prefix = "IceBox.Service.";
    PropertyDict services = properties->getPropertiesForPrefix(prefix);
    for(PropertyDict::const_iterator p = services.begin(); p != services.end(); ++p)
    {
        string name = p->first.substr(prefix.size());
        StringSeq::iterator q = args.begin();
        while(q != args.end())
        {
            if(q->find("--" + name + ".") == 0)
            {
                q = args.erase(q);
                continue;
            }
            ++q;
        }
    }

    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");

    try
    {
        args = opts.parse(args);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        error(e.reason);
        usage(argv[0]);
        return false;
    }

    if(opts.isSet("help"))
    {
        usage(argv[0]);
        status = EXIT_SUCCESS;
        return false;
    }
    if(opts.isSet("version"))
    {
        print(ICE_STRING_VERSION);
        status = EXIT_SUCCESS;
        return false;
    }

    if(!args.empty())
    {
        usage(argv[0]);
        return false;
    }

    _serviceManager = new ServiceManagerI(communicator(), argc, argv);

    return _serviceManager->start();
}

bool
IceBox::IceBoxService::stop()
{
    if(_serviceManager)
    {
        _serviceManager->stop();
        _serviceManager = 0;
    }
    return true;
}

void
IceBox::IceBoxService::usage(const string& appName)
{
    string options =
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.";
#ifndef _WIN32
    options.append(
        "\n"
        "\n"
        "--daemon             Run as a daemon.\n"
        "--pidfile FILE       Write process ID into FILE.\n"
        "--noclose            Do not close open file descriptors.\n"
        "--nochdir            Do not change the current working directory."
    );
#endif
    print("Usage: " + appName + " [options]\n" + options);
}

#ifdef _WIN32

int
wmain(int argc, wchar_t* argv[])

#else

int
main(int argc, char* argv[])

#endif
{
    IceBox::IceBoxService svc;

    InitializationData initData;
    initData.properties = createProperties();
    initData.properties->setProperty("Ice.Admin.DelayCreation", "1");
    return svc.main(argc, argv, initData);
}

