//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Options.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/FileUtil.h>
#include <Ice/Ice.h>
#include <Ice/ConsoleUtil.h>
#include <IceDB/IceDB.h>
#include <IceGrid/Admin.h>
#include <IceGrid/DBTypes.h>
#include <IceUtil/DisableWarnings.h>

#include <fstream>

using namespace std;
using namespace IceInternal;

namespace
{

bool skipReplicaGroupFilter = false;

class ServerDescriptorI : public IceGrid::ServerDescriptor
{
public:

    ServerDescriptorI(const string& serverVersion) :
        _serverVersion(serverVersion)
    {
    }

protected:

    virtual void ice_postUnmarshal()
    {
        iceVersion = _serverVersion;
    }

private:

    string _serverVersion;
};

class IceBoxDescriptorI : public IceGrid::IceBoxDescriptor
{
public:

    IceBoxDescriptorI(const string& serverVersion) :
        _serverVersion(serverVersion)
    {
    }

protected:

    virtual void ice_postUnmarshal()
    {
        iceVersion = _serverVersion;
    }

private:

    string _serverVersion;
};

class ValueFactoryI : public Ice::ValueFactory
{
public:

    ValueFactoryI(const string& serverVersion) :
        _serverVersion(serverVersion)
    {
    }

    virtual Ice::ObjectPtr create(const string& type)
    {
        if(type == "::IceGrid::ServerDescriptor")
        {
            return new ServerDescriptorI(_serverVersion);
        }
        else if(type == "::IceGrid::IceBoxDescriptor")
        {
            return new IceBoxDescriptorI(_serverVersion);
        }
        return 0;
    }

private:

    string _serverVersion;
};

}

//
// This custom version of the StreamReader allows us to customize the
// reading of ReplicaGroupDescriptor
//
namespace Ice
{

template<>
struct StreamReader<IceGrid::ReplicaGroupDescriptor, Ice::InputStream>
{
    static void read(Ice::InputStream* is, IceGrid::ReplicaGroupDescriptor& v)
    {
        is->read(v.id);
        is->read(v.loadBalancing);
        is->read(v.proxyOptions);
        is->read(v.objects);
        is->read(v.description);
        if(!skipReplicaGroupFilter)
        {
            is->read(v.filter);
        }
    }
};
}

int run(const Ice::StringSeq&);

Ice::CommunicatorPtr communicator;

void
destroyCommunicator(int)
{
    communicator->destroy();
}

int
#ifdef _WIN32
wmain(int argc, wchar_t* argv[])
#else
main(int argc, char* argv[])
#endif
{
    int status = 0;

    try
    {
        IceUtil::CtrlCHandler ctrlCHandler;
        Ice::CommunicatorHolder ich(argc, argv);
        communicator = ich.communicator();

        ctrlCHandler.setCallback(&destroyCommunicator);

        status = run(Ice::argsToStringSeq(argc, argv));
    }
    catch(const std::exception& ex)
    {
        consoleErr << ex.what() << endl;
        status = 1;
    }

    return status;
}

void
usage(const string& name)
{
    consoleErr << "Usage: " << name << " <options>\n";
    consoleErr <<
        "Options:\n"
        "-h, --help             Show this message.\n"
        "-v, --version          Display version.\n"
        "--import FILE          Import database from FILE.\n"
        "--export FILE          Export database to FILE.\n"
        "--dbhome DIR           Source or target database environment.\n"
        "--dbpath DIR           Source or target database environment.\n"
        "--mapsize VALUE        Set LMDB map size in MB (optional, import only).\n"
        "--server-version VER   Set Ice version for IceGrid servers (optional, import only).\n"
        "-d, --debug            Print debug messages.\n"
        ;
}

int
run(const Ice::StringSeq& args)
{
    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("d", "debug");
    opts.addOpt("", "import", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "export", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "dbhome", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "dbpath", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "mapsize", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "server-version", IceUtilInternal::Options::NeedArg);

    try
    {
        if(!opts.parse(args).empty())
        {
            consoleErr << args[0] << ": too many arguments" << endl;
            usage(args[0]);
            return 1;
        }
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        consoleErr << args[0] << ": " << e.reason << endl;
        usage(args[0]);
        return 1;
    }

    if(opts.isSet("help"))
    {
        usage(args[0]);
        return 0;
    }

    if(opts.isSet("version"))
    {
        consoleOut << ICE_STRING_VERSION << endl;
        return 0;
    }

    if(!(opts.isSet("import") ^ opts.isSet("export")))
    {
        consoleErr << args[0] << ": either --import or --export must be set" << endl;
        usage(args[0]);
        return 1;
    }

    if(!(opts.isSet("dbhome") ^ opts.isSet("dbpath")))
    {
        consoleErr << args[0] << ": set the database environment directory with either --dbhome or --dbpath" << endl;
        usage(args[0]);
        return 1;
    }

    bool debug = opts.isSet("debug");
    bool import = opts.isSet("import");
    string dbFile = opts.optArg(import ? "import" : "export");
    string dbPath;
    if(opts.isSet("dbhome"))
    {
        dbPath = opts.optArg("dbhome");
    }
    else
    {
        dbPath = opts.optArg("dbpath");
    }

    string mapSizeStr = opts.optArg("mapsize");
    size_t mapSize = IceDB::getMapSize(atoi(mapSizeStr.c_str()));
    string serverVersion = opts.optArg("server-version");

    try
    {
        IceGrid::AllData data;

        IceDB::IceContext dbContext;
        dbContext.communicator = communicator;
        dbContext.encoding.major = 1;
        dbContext.encoding.minor = 1;

        if(import)
        {
            consoleOut << "Importing database to directory `" << dbPath << "' from file `" << dbFile << "'"
                       << endl;

            if(!IceUtilInternal::directoryExists(dbPath))
            {
                consoleErr << args[0] << ": output directory does not exist: " << dbPath << endl;
                return 1;
            }

            if(!IceUtilInternal::isEmptyDirectory(dbPath))
            {
                consoleErr << args[0] << ": output directory is not empty: " << dbPath << endl;
                return 1;
            }

            ifstream fs(IceUtilInternal::streamFilename(dbFile).c_str(), ios::binary);
            if(fs.fail())
            {
                consoleErr << args[0] << ": could not open input file: " << IceUtilInternal::errorToString(errno) << endl;
                return 1;
            }
            fs.unsetf(ios::skipws);

            fs.seekg(0, ios::end);
            streampos fileSize = fs.tellg();

            if(!fileSize)
            {
                fs.close();
                consoleErr << args[0] << ": empty input file" << endl;
                return 1;
            }

            fs.seekg(0, ios::beg);

            vector<Ice::Byte> buf;
            buf.reserve(static_cast<size_t>(fileSize));
            buf.insert(buf.begin(), istream_iterator<Ice::Byte>(fs), istream_iterator<Ice::Byte>());

            fs.close();

            if(!serverVersion.empty())
            {
                Ice::ValueFactoryPtr factory = new ValueFactoryI(serverVersion);
                communicator->getValueFactoryManager()->add(factory, "::IceGrid::ServerDescriptor");
                communicator->getValueFactoryManager()->add(factory, "::IceGrid::IceBoxDescriptor");
            }

            Ice::InputStream stream(communicator, dbContext.encoding, buf);

            string type;
            int version;

            stream.read(type);
            if(type != "IceGrid")
            {
                consoleErr << args[0] << ": incorrect input file type: " << type << endl;
                return 1;
            }
            stream.read(version);
            if(version / 100 == 305)
            {
                if(debug)
                {
                    consoleOut << "Reading Ice 3.5.x data" << endl;
                }
                skipReplicaGroupFilter = true;
            }
            stream.read(data);

            {
                IceDB::Env env(dbPath, 5, mapSize);
                IceDB::ReadWriteTxn txn(env);

                if(debug)
                {
                    consoleOut << "Writing Applications Map:" << endl;
                }

                IceDB::Dbi<string, IceGrid::ApplicationInfo, IceDB::IceContext, Ice::OutputStream>
                    apps(txn, "applications", dbContext, MDB_CREATE);

                for(IceGrid::ApplicationInfoSeq::const_iterator p = data.applications.begin(); p != data.applications.end(); ++p)
                {
                    if(debug)
                    {
                        consoleOut << "  NAME = " << p->descriptor.name << endl;
                    }
                    apps.put(txn, p->descriptor.name, *p);
                }

                if(debug)
                {
                    consoleOut << "Writing Adapters Map:" << endl;
                }

                IceDB::Dbi<string, IceGrid::AdapterInfo, IceDB::IceContext, Ice::OutputStream>
                    adpts(txn, "adapters", dbContext, MDB_CREATE);

                for(IceGrid::AdapterInfoSeq::const_iterator p = data.adapters.begin(); p != data.adapters.end(); ++p)
                {
                    if(debug)
                    {
                        consoleOut << "  NAME = " << p->id << endl;
                    }
                    adpts.put(txn, p->id, *p);
                }

                if(debug)
                {
                    consoleOut << "Writing Objects Map:" << endl;
                }

                IceDB::Dbi<Ice::Identity, IceGrid::ObjectInfo, IceDB::IceContext, Ice::OutputStream>
                    objs(txn, "objects", dbContext, MDB_CREATE);

                for(IceGrid::ObjectInfoSeq::const_iterator p = data.objects.begin(); p != data.objects.end(); ++p)
                {
                    if(debug)
                    {
                        consoleOut << "  NAME = " << communicator->identityToString(p->proxy->ice_getIdentity())
                                   << endl;
                    }
                    objs.put(txn, p->proxy->ice_getIdentity(), *p);
                }

                if(debug)
                {
                    consoleOut << "Writing Internal Objects Map:" << endl;
                }

                IceDB::Dbi<Ice::Identity, IceGrid::ObjectInfo, IceDB::IceContext, Ice::OutputStream>
                    internalObjs(txn, "internal-objects", dbContext, MDB_CREATE);

                for(IceGrid::ObjectInfoSeq::const_iterator p = data.internalObjects.begin(); p != data.internalObjects.end();
                    ++p)
                {
                    if(debug)
                    {
                        consoleOut << "  NAME = " << communicator->identityToString(p->proxy->ice_getIdentity())
                                   << endl;
                    }
                    internalObjs.put(txn, p->proxy->ice_getIdentity(), *p);
                }

                if(debug)
                {
                    consoleOut << "Writing Serials Map:" << endl;
                }

                IceDB::Dbi<string, Ice::Long, IceDB::IceContext, Ice::OutputStream>
                    srls(txn, "serials", dbContext, MDB_CREATE);

                for(IceGrid::StringLongDict::const_iterator p = data.serials.begin(); p != data.serials.end(); ++p)
                {
                    if(debug)
                    {
                        consoleOut << "  NAME = " << p->first << endl;
                    }
                    srls.put(txn, p->first, p->second);
                }

                txn.commit();
                env.close();
            }
        }
        else
        {
            consoleOut << "Exporting database from directory `" << dbPath << "' to file `" << dbFile << "'"
                       << endl;

            {
                IceDB::Env env(dbPath, 5);
                IceDB::ReadOnlyTxn txn(env);

                if(debug)
                {
                    consoleOut << "Reading Application Map:" << endl;
                }

                IceDB::Dbi<string, IceGrid::ApplicationInfo, IceDB::IceContext, Ice::OutputStream>
                    applications(txn, "applications", dbContext, 0);

                string name;
                IceGrid::ApplicationInfo application;
                IceDB::ReadOnlyCursor<string, IceGrid::ApplicationInfo, IceDB::IceContext, Ice::OutputStream>
                    appCursor(applications, txn);
                while(appCursor.get(name, application, MDB_NEXT))
                {
                    if(debug)
                    {
                        consoleOut << "  APPLICATION = " << name << endl;
                    }
                    data.applications.push_back(application);
                }
                appCursor.close();

                if(debug)
                {
                    consoleOut << "Reading Adapter Map:" << endl;
                }

                IceDB::Dbi<string, IceGrid::AdapterInfo, IceDB::IceContext, Ice::OutputStream>
                    adapters(txn, "adapters", dbContext, 0);

                IceGrid::AdapterInfo adapter;
                IceDB::ReadOnlyCursor<string, IceGrid::AdapterInfo, IceDB::IceContext, Ice::OutputStream>
                    adapterCursor(adapters, txn);
                while(adapterCursor.get(name, adapter, MDB_NEXT))
                {
                    if(debug)
                    {
                        consoleOut << "  ADAPTER = " << name << endl;
                    }
                    data.adapters.push_back(adapter);
                }
                adapterCursor.close();

                if(debug)
                {
                    consoleOut << "Reading Object Map:" << endl;
                }

                IceDB::Dbi<Ice::Identity, IceGrid::ObjectInfo, IceDB::IceContext, Ice::OutputStream>
                    objects(txn, "objects", dbContext, 0);

                Ice::Identity id;
                IceGrid::ObjectInfo object;
                IceDB::ReadOnlyCursor<Ice::Identity, IceGrid::ObjectInfo, IceDB::IceContext, Ice::OutputStream>
                    objCursor(objects, txn);
                while(objCursor.get(id, object, MDB_NEXT))
                {
                    if(debug)
                    {
                        consoleOut << "  IDENTITY = " << communicator->identityToString(id) << endl;
                    }
                    data.objects.push_back(object);
                }
                objCursor.close();

                if(debug)
                {
                    consoleOut << "Reading Internal Object Map:" << endl;
                }

                IceDB::Dbi<Ice::Identity, IceGrid::ObjectInfo, IceDB::IceContext, Ice::OutputStream>
                    internalObjects(txn, "internal-objects", dbContext, 0);

                IceDB::ReadOnlyCursor<Ice::Identity, IceGrid::ObjectInfo, IceDB::IceContext, Ice::OutputStream>
                    iobjCursor(internalObjects, txn);
                while(iobjCursor.get(id, object, MDB_NEXT))
                {
                    if(debug)
                    {
                        consoleOut << "  IDENTITY = " << communicator->identityToString(id) << endl;
                    }
                    data.internalObjects.push_back(object);
                }
                iobjCursor.close();

                if(debug)
                {
                    consoleOut << "Reading Serials Map:" << endl;
                }

                IceDB::Dbi<string, Ice::Long, IceDB::IceContext, Ice::OutputStream>
                    serials(txn, "serials", dbContext, 0);

                Ice::Long serial;
                IceDB::ReadOnlyCursor<string, Ice::Long, IceDB::IceContext, Ice::OutputStream>
                    serialCursor(serials, txn);
                while(serialCursor.get(name, serial, MDB_NEXT))
                {
                    if(debug)
                    {
                        consoleOut << "  NAME = " << name << endl;
                    }
                    data.serials.insert(std::make_pair(name, serial));
                }
                serialCursor.close();

                txn.rollback();
                env.close();
            }

            Ice::OutputStream stream(communicator, dbContext.encoding);
            stream.write("IceGrid");
            stream.write(ICE_INT_VERSION);
            stream.write(data);

            ofstream fs(IceUtilInternal::streamFilename(dbFile).c_str(), ios::binary);
            if(fs.fail())
            {
                consoleErr << args[0] << ": could not open output file: " << IceUtilInternal::errorToString(errno)
                           << endl;
                return 1;
            }
            fs.write(reinterpret_cast<const char*>(stream.b.begin()), static_cast<streamsize>(stream.b.size()));
            fs.close();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        consoleErr << args[0] << ": " << (import ? "import" : "export") << " failed:\n" << ex << endl;
        return 1;
    }

    return 0;
}
