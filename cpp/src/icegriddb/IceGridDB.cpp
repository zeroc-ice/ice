// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <IceUtil/FileUtil.h>
#include <Ice/Ice.h>
#include <IceDB/IceDB.h>
#include <IceGrid/Admin.h>
#include <IceGrid/DBTypes.h>
#include <IceUtil/DisableWarnings.h>

#include <fstream>

using namespace std;
using namespace Ice;
using namespace IceGrid;

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

    virtual ObjectPtr create(const string& type)
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
        // cerr << "Custom read" << endl;
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


class Client : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);
};

#ifdef _WIN32

int
wmain(int argc, wchar_t* argv[])

#else

int
main(int argc, char* argv[])

#endif
{
    Client app;
    return app.main(argc, argv);
}

void
Client::usage()
{
    cerr << "Usage: " << appName() << " <options>\n";
    cerr <<
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
Client::run(int argc, char* argv[])
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

    vector<string> args;
    try
    {
        args = opts.parse(argc, const_cast<const char**>(argv));
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        cerr << argv[0] << ": " << e.reason << endl;
        usage();
        return EXIT_FAILURE;
    }
    if(!args.empty())
    {
        cerr << argv[0] << ": too many arguments" << endl;
        usage();
        return EXIT_FAILURE;
    }

    if(opts.isSet("help"))
    {
        usage();
        return EXIT_SUCCESS;
    }

    if(opts.isSet("version"))
    {
        cout << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    if(!(opts.isSet("import") ^ opts.isSet("export")))
    {
        cerr << argv[0] << ": either --import or --export must be set" << endl;
        usage();
        return EXIT_FAILURE;
    }

    if(!(opts.isSet("dbhome") ^ opts.isSet("dbpath")))
    {
        cerr << argv[0] << ": set the database environment directory with either --dbhome or --dbpath" << endl;
        usage();
        return EXIT_FAILURE;
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
        dbContext.communicator = communicator();
        dbContext.encoding.major = 1;
        dbContext.encoding.minor = 1;

        if(import)
        {
            cout << "Importing database to directory `" << dbPath << "' from file `" << dbFile << "'" << endl;

            if(!IceUtilInternal::directoryExists(dbPath))
            {
                cerr << argv[0] << ": output directory does not exist: " << dbPath << endl;
                return EXIT_FAILURE;
            }

            if(!IceUtilInternal::isEmptyDirectory(dbPath))
            {
                cerr << argv[0] << ": output directory is not empty: " << dbPath << endl;
                return EXIT_FAILURE;
            }

            ifstream fs(IceUtilInternal::streamFilename(dbFile), ios::binary);
            if(fs.fail())
            {
                cerr << argv[0] << ": could not open input file: " << strerror(errno) << endl;
                return EXIT_FAILURE;
            }
            fs.unsetf(ios::skipws);

            fs.seekg(0, ios::end);
            streampos fileSize = fs.tellg();

            if(!fileSize)
            {
                fs.close();
                cerr << argv[0] << ": empty input file" << endl;
                return EXIT_FAILURE;
            }

            fs.seekg(0, ios::beg);

            vector<Ice::Byte> buf;
            buf.reserve(static_cast<size_t>(fileSize));
            buf.insert(buf.begin(), istream_iterator<Ice::Byte>(fs), istream_iterator<Ice::Byte>());

            fs.close();

            if(!serverVersion.empty())
            {
                ValueFactoryPtr factory = new ValueFactoryI(serverVersion);
                communicator()->getValueFactoryManager()->add(factory, "::IceGrid::ServerDescriptor");
                communicator()->getValueFactoryManager()->add(factory, "::IceGrid::IceBoxDescriptor");
            }

            Ice::InputStream stream(communicator(), dbContext.encoding, buf);

            string type;
            int version;

            stream.read(type);
            if(type != "IceGrid")
            {
                cerr << argv[0] << ": incorrect input file type: " << type << endl;
                return EXIT_FAILURE;
            }
            stream.read(version);
            if(version / 100 == 305)
            {
                if(debug)
                {
                    cout << "Reading Ice 3.5.x data" << endl;
                }
                skipReplicaGroupFilter = true;
            }
            stream.read(data);

            {
                IceDB::Env env(dbPath, 5, mapSize);
                IceDB::ReadWriteTxn txn(env);

                if(debug)
                {
                    cout << "Writing Applications Map:" << endl;
                }

                IceDB::Dbi<string, ApplicationInfo, IceDB::IceContext, Ice::OutputStream>
                    apps(txn, "applications", dbContext, MDB_CREATE);

                for(ApplicationInfoSeq::const_iterator p = data.applications.begin(); p != data.applications.end(); ++p)
                {
                    if(debug)
                    {
                        cout << "  NAME = " << p->descriptor.name << endl;
                    }
                    apps.put(txn, p->descriptor.name, *p);
                }

                if(debug)
                {
                    cout << "Writing Adapters Map:" << endl;
                }

                IceDB::Dbi<string, AdapterInfo, IceDB::IceContext, Ice::OutputStream>
                    adpts(txn, "adapters", dbContext, MDB_CREATE);

                for(AdapterInfoSeq::const_iterator p = data.adapters.begin(); p != data.adapters.end(); ++p)
                {
                    if(debug)
                    {
                        cout << "  NAME = " << p->id << endl;
                    }
                    adpts.put(txn, p->id, *p);
                }

                if(debug)
                {
                    cout << "Writing Objects Map:" << endl;
                }

                IceDB::Dbi<Identity, ObjectInfo, IceDB::IceContext, Ice::OutputStream>
                    objs(txn, "objects", dbContext, MDB_CREATE);

                for(ObjectInfoSeq::const_iterator p = data.objects.begin(); p != data.objects.end(); ++p)
                {
                    if(debug)
                    {
                        cout << "  NAME = " << communicator()->identityToString(p->proxy->ice_getIdentity()) << endl;
                    }
                    objs.put(txn, p->proxy->ice_getIdentity(), *p);
                }

                if(debug)
                {
                    cout << "Writing Internal Objects Map:" << endl;
                }

                IceDB::Dbi<Identity, ObjectInfo, IceDB::IceContext, Ice::OutputStream>
                    internalObjs(txn, "internal-objects", dbContext, MDB_CREATE);

                for(ObjectInfoSeq::const_iterator p = data.internalObjects.begin(); p != data.internalObjects.end();
                    ++p)
                {
                    if(debug)
                    {
                        cout << "  NAME = " << communicator()->identityToString(p->proxy->ice_getIdentity()) << endl;
                    }
                    internalObjs.put(txn, p->proxy->ice_getIdentity(), *p);
                }

                if(debug)
                {
                    cout << "Writing Serials Map:" << endl;
                }

                IceDB::Dbi<string, Long, IceDB::IceContext, Ice::OutputStream>
                    srls(txn, "serials", dbContext, MDB_CREATE);

                for(StringLongDict::const_iterator p = data.serials.begin(); p != data.serials.end(); ++p)
                {
                    if(debug)
                    {
                        cout << "  NAME = " << p->first << endl;
                    }
                    srls.put(txn, p->first, p->second);
                }

                txn.commit();
                env.close();
            }
        }
        else
        {
            cout << "Exporting database from directory `" << dbPath << "' to file `" << dbFile << "'" << endl;

            {
                IceDB::Env env(dbPath, 5);
                IceDB::ReadOnlyTxn txn(env);

                if(debug)
                {
                    cout << "Reading Application Map:" << endl;
                }

                IceDB::Dbi<string, IceGrid::ApplicationInfo, IceDB::IceContext, Ice::OutputStream>
                    applications(txn, "applications", dbContext, 0);

                string name;
                ApplicationInfo application;
                IceDB::ReadOnlyCursor<string, IceGrid::ApplicationInfo, IceDB::IceContext, Ice::OutputStream>
                    appCursor(applications, txn);
                while(appCursor.get(name, application, MDB_NEXT))
                {
                    if(debug)
                    {
                        cout << "  APPLICATION = " << name << endl;
                    }
                    data.applications.push_back(application);
                }
                appCursor.close();

                if(debug)
                {
                    cout << "Reading Adapter Map:" << endl;
                }

                IceDB::Dbi<string, IceGrid::AdapterInfo, IceDB::IceContext, Ice::OutputStream>
                    adapters(txn, "adapters", dbContext, 0);

                AdapterInfo adapter;
                IceDB::ReadOnlyCursor<string, IceGrid::AdapterInfo, IceDB::IceContext, Ice::OutputStream>
                    adapterCursor(adapters, txn);
                while(adapterCursor.get(name, adapter, MDB_NEXT))
                {
                    if(debug)
                    {
                        cout << "  ADAPTER = " << name << endl;
                    }
                    data.adapters.push_back(adapter);
                }
                adapterCursor.close();

                if(debug)
                {
                    cout << "Reading Object Map:" << endl;
                }

                IceDB::Dbi<Identity, IceGrid::ObjectInfo, IceDB::IceContext, Ice::OutputStream>
                    objects(txn, "objects", dbContext, 0);

                Identity id;
                ObjectInfo object;
                IceDB::ReadOnlyCursor<Identity, IceGrid::ObjectInfo, IceDB::IceContext, Ice::OutputStream>
                    objCursor(objects, txn);
                while(objCursor.get(id, object, MDB_NEXT))
                {
                    if(debug)
                    {
                        cout << "  IDENTITY = " << communicator()->identityToString(id) << endl;
                    }
                    data.objects.push_back(object);
                }
                objCursor.close();

                if(debug)
                {
                    cout << "Reading Internal Object Map:" << endl;
                }

                IceDB::Dbi<Identity, IceGrid::ObjectInfo, IceDB::IceContext, Ice::OutputStream>
                    internalObjects(txn, "internal-objects", dbContext, 0);

                IceDB::ReadOnlyCursor<Identity, IceGrid::ObjectInfo, IceDB::IceContext, Ice::OutputStream>
                    iobjCursor(internalObjects, txn);
                while(iobjCursor.get(id, object, MDB_NEXT))
                {
                    if(debug)
                    {
                        cout << "  IDENTITY = " << communicator()->identityToString(id) << endl;
                    }
                    data.internalObjects.push_back(object);
                }
                iobjCursor.close();

                if(debug)
                {
                    cout << "Reading Serials Map:" << endl;
                }

                IceDB::Dbi<string, Long, IceDB::IceContext, Ice::OutputStream>
                    serials(txn, "serials", dbContext, 0);

                Long serial;
                IceDB::ReadOnlyCursor<string, Long, IceDB::IceContext, Ice::OutputStream>
                    serialCursor(serials, txn);
                while(serialCursor.get(name, serial, MDB_NEXT))
                {
                    if(debug)
                    {
                        cout << "  NAME = " << name << endl;
                    }
                    data.serials.insert(std::make_pair(name, serial));
                }
                serialCursor.close();

                txn.rollback();
                env.close();
            }

            Ice::OutputStream stream(communicator(), dbContext.encoding);
            stream.write("IceGrid");
            stream.write(ICE_INT_VERSION);
            stream.write(data);

            ofstream fs(IceUtilInternal::streamFilename(dbFile), ios::binary);
            if(fs.fail())
            {
                cerr << argv[0] << ": could not open output file: " << strerror(errno) << endl;
                return EXIT_FAILURE;
            }
            fs.write(reinterpret_cast<const char*>(stream.b.begin()), stream.b.size());
            fs.close();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        cerr << argv[0] << ": " << (import ? "import" : "export") << " failed:\n" << ex << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
