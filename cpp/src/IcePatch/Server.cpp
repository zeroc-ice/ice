// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Service.h>
#include <IcePatch/FileLocator.h>
#include <IcePatch/IcePatchI.h>
#include <IcePatch/Util.h>
#ifdef _WIN32
#   include <direct.h>
#endif

using namespace std;
using namespace Ice;
using namespace IcePatch;

namespace IcePatch
{

class IcePatchService : public Service
{
public:

    IcePatchService();

    void usage(const string&);

protected:

    virtual bool start(int, char*[]);
    virtual bool stop();

private:

    void runUpdater();
    void cleanup();
    void cleanupRec(const FileDescSeq&);

    IceUtil::Monitor<IceUtil::Mutex> _monitor;
    IceUtil::ThreadPtr _thread;
    bool _shutdown;
    ObjectAdapterPtr _adapter;
    IceUtil::Time _updatePeriod;

    class UpdaterThread : public IceUtil::Thread
    {
    public:

        UpdaterThread(IcePatchService*);

        virtual void run();

    protected:

        IcePatchService* _service;
    };
    friend class UpdaterThread;
};

}

IcePatch::IcePatchService::IcePatchService() :
    _shutdown(false)
{
}

void
IcePatch::IcePatchService::usage(const string& name)
{
    string options =
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.";
#ifdef _WIN32
    if(checkSystem())
    {
        options.append(
        "\n"
        "\n"
        "--service NAME       Run as the Windows service NAME.\n"
        "\n"
        "--install NAME [--display DISP] [--executable EXEC] [args]\n"
        "                     Install as Windows service NAME. If DISP is\n"
        "                     provided, use it as the display name,\n"
        "                     otherwise NAME is used. If EXEC is provided,\n"
        "                     use it as the service executable, otherwise\n"
        "                     this executable is used. Any additional\n"
        "                     arguments are passed unchanged to the\n"
        "                     service at startup.\n"
        "--uninstall NAME     Uninstall Windows service NAME.\n"
        "--start NAME [args]  Start Windows service NAME. Any additional\n"
        "                     arguments are passed unchanged to the\n"
        "                     service.\n"
        "--stop NAME          Stop Windows service NAME."
        );
    }
#else
    options.append(
        "\n"
        "\n"
        "--daemon             Run as a daemon.\n"
        "--noclose            Do not close open file descriptors.\n"
        "--nochdir            Do not change the current working directory."
    );
#endif
    cerr << "Usage: " << name << " [options]" << endl;
    cerr << options << endl;
}

bool
IcePatch::IcePatchService::start(int argc, char* argv[])
{
    IceUtil::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    
    vector<string> args;
    try
    {
    	args = opts.parse(argc, argv);
    }
    catch(const IceUtil::Options::BadOpt& e)
    {
        cerr << e.reason << endl;
	usage(argv[0]);
	return false;
    }

    if(opts.isSet("h") || opts.isSet("help"))
    {
	usage(argv[0]);
	return false;
    }
    if(opts.isSet("v") || opts.isSet("version"))
    {
	cout << ICE_STRING_VERSION << endl;
	return false;
    }

    if(!args.empty())
    {
	usage(argv[0]);
	return false;
    }

    PropertiesPtr properties = communicator()->getProperties();

    //
    // Get the IcePatch endpoints.
    //
    const char* endpointsProperty = "IcePatch.Endpoints";
    if(properties->getProperty(endpointsProperty).empty())
    {
	error(string(argv[0]) + ": property `" + endpointsProperty + "' is not set");
	return false;
    }

    //
    // Create and initialize the object adapter and the file locator.
    //
    string dataDir = properties->getProperty("IcePatch.Directory");
    if(dataDir.empty())
    {
#ifdef _WIN32
        char cwd[_MAX_PATH];
        _getcwd(cwd, _MAX_PATH);
#else
        char cwd[PATH_MAX];
        getcwd(cwd, PATH_MAX);
#endif
        dataDir = cwd;
    }
    _adapter = communicator()->createObjectAdapter("IcePatch");
    ServantLocatorPtr fileLocator = new FileLocator(_adapter, dataDir);
    _adapter->addServantLocator(fileLocator, "IcePatch");

    //
    // Start the updater thread if an update period is set.
    //
    _updatePeriod = IceUtil::Time::seconds(properties->getPropertyAsIntWithDefault("IcePatch.UpdatePeriod", 60));
    if(_updatePeriod != IceUtil::Time())
    {
	if(_updatePeriod < IceUtil::Time::seconds(1))
	{
	    _updatePeriod = IceUtil::Time::seconds(1);
	}
	_thread = new UpdaterThread(this);
	_thread->start();
    }

    //
    // Everything ok, let's go.
    //
    _adapter->activate();

    return true;
}

bool
IcePatch::IcePatchService::stop()
{
    if(_thread)
    {
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
            _shutdown = true;
            _monitor.notify();
        }
        _thread->getThreadControl().join();
        _thread = 0;
    }
    return true;
}

void
IcePatch::IcePatchService::runUpdater()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);

    while(!_shutdown)
    {
	try
	{
	    cleanup();
	}
	catch(const FileAccessException& ex)
	{
            ostringstream ostr;
	    ostr << "exception during update:\n" << ex << ":\n" << ex.reason;
            error(ostr.str());
	}
	catch(const BusyException&)
	{
	    //
	    // Just loop if we're busy.
	    //
	}
	catch(const Exception& ex)
	{
	    //
	    // Log other exceptions only if we are not destroyed.
	    //
	    if(!_shutdown)
	    {
                ostringstream ostr;
		ostr << "exception during update:\n" << ex;
                error(ostr.str());
	    }
	}

	if(_shutdown)
	{
	    break;
	}

	_monitor.timedWait(_updatePeriod);
    }
}

void
IcePatch::IcePatchService::cleanup()
{
    Identity ident;
    ident.category = "IcePatch";
    ident.name = ".";
    ObjectPrx topObj = _adapter->createProxy(ident);
    FilePrx top = FilePrx::checkedCast(topObj);
    assert(top);
    DirectoryDescPtr topDesc = DirectoryDescPtr::dynamicCast(top->describe());
    assert(topDesc);
    cleanupRec(topDesc->dir->getContents());
    topDesc->dir->describe(); // Refresh the top-level MD5 file.
}

void
IcePatch::IcePatchService::cleanupRec(const FileDescSeq& fileDescSeq)
{
    for(FileDescSeq::const_iterator p = fileDescSeq.begin(); p != fileDescSeq.end(); ++p)
    {
	if(_shutdown)
	{
	    return;
	}

	DirectoryDescPtr directoryDesc = DirectoryDescPtr::dynamicCast(*p);
	if(directoryDesc)
	{
	    //
	    // Force MD5 files to be created and orphaned files to be
	    // removed. Then recurse into subdirectories.
	    //
	    cleanupRec(directoryDesc->dir->getContents());

	    //
	    // Call describe(), because BZ2 and MD5 files in the
	    // directory might have changed, resulting in a different
	    // summary MD5 for this directory.
	    //
	    directoryDesc->dir->describe();
	}
	else
	{
	    RegularDescPtr regularDesc = RegularDescPtr::dynamicCast(*p);
	    assert(regularDesc);

	    //
	    // Force BZ2 files to be created for all regular files.
	    //
	    regularDesc->reg->getBZ2Size();
	}
    }
}

IcePatch::IcePatchService::UpdaterThread::UpdaterThread(IcePatchService* service) :
    _service(service)
{
}

void
IcePatch::IcePatchService::UpdaterThread::run()
{
    _service->runUpdater();
}

int
main(int argc, char* argv[])
{
    IcePatchService svc;
    return svc.main(argc, argv);
}
