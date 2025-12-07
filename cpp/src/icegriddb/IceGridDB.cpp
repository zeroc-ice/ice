// Copyright (c) ZeroC, Inc.

#include "../Ice/ConsoleUtil.h"
#include "../Ice/FileUtil.h"
#include "../Ice/Options.h"
#include "../IceDB/IceDB.h"
#include "DBTypes.h"
#include "Ice/Ice.h"
#include "Ice/StringUtil.h"
#include "IceGrid/Admin.h"

#include "../Ice/DisableWarnings.h"

#include <fstream>
#include <iterator>

using namespace std;
using namespace IceInternal;

namespace
{
    class ServerDescriptorI : public IceGrid::ServerDescriptor
    {
    public:
        ServerDescriptorI(string serverVersion) : _serverVersion(std::move(serverVersion)) {}

    protected:
        void ice_postUnmarshal() override { iceVersion = _serverVersion; }

    private:
        string _serverVersion;
    };

    class IceBoxDescriptorI : public IceGrid::IceBoxDescriptor
    {
    public:
        IceBoxDescriptorI(string serverVersion) : _serverVersion(std::move(serverVersion)) {}

    protected:
        void ice_postUnmarshal() override { iceVersion = _serverVersion; }

    private:
        string _serverVersion;
    };

    class CustomSliceLoader final : public Ice::SliceLoader
    {
    public:
        [[nodiscard]] Ice::ValuePtr newClassInstance(string_view typeId) const final
        {
            // When server version is set, we blindly overwrite the "iceVersion" field in all server descriptors and
            // IceBox descriptors using the "post unmarshal" hook.
            if (!_serverVersion.empty())
            {
                if (typeId == IceGrid::ServerDescriptor::ice_staticId())
                {
                    return std::make_shared<ServerDescriptorI>(_serverVersion);
                }
                else if (typeId == IceGrid::IceBoxDescriptor::ice_staticId())
                {
                    return std::make_shared<IceBoxDescriptorI>(_serverVersion);
                }
            }

            return nullptr;
        }

        void setServerVersion(string serverVersion) { _serverVersion = std::move(serverVersion); }

    private:
        string _serverVersion;
    };

    using CustomSliceLoaderPtr = shared_ptr<CustomSliceLoader>;
}

int run(const Ice::StringSeq&, const CustomSliceLoaderPtr&);

shared_ptr<Ice::Communicator> communicator;

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
        Ice::CtrlCHandler ctrlCHandler;
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        auto customSliceLoader = make_shared<CustomSliceLoader>();
        initData.sliceLoader = customSliceLoader;

        Ice::CommunicatorHolder ich{std::move(initData)};
        communicator = ich.communicator();

        ctrlCHandler.setCallback(&destroyCommunicator);

        status = run(Ice::argsToStringSeq(argc, argv), customSliceLoader);
    }
    catch (const std::exception& ex)
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
    consoleErr << "Options:\n"
                  "-h, --help             Show this message.\n"
                  "-v, --version          Display version.\n"
                  "--import FILE          Import database from FILE.\n"
                  "--export FILE          Export database to FILE.\n"
                  "--dbhome DIR           Source or target database environment.\n"
                  "--dbpath DIR           Source or target database environment.\n"
                  "--mapsize VALUE        Set LMDB map size in MB (optional, import only).\n"
                  "--server-version VER   Set Ice version for IceGrid servers (optional, import only).\n"
                  "-d, --debug            Print debug messages.\n";
}

int
run(const Ice::StringSeq& args, const CustomSliceLoaderPtr& customSliceLoader)
{
    IceInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("d", "debug");
    opts.addOpt("", "import", IceInternal::Options::NeedArg);
    opts.addOpt("", "export", IceInternal::Options::NeedArg);
    opts.addOpt("", "dbhome", IceInternal::Options::NeedArg);
    opts.addOpt("", "dbpath", IceInternal::Options::NeedArg);
    opts.addOpt("", "mapsize", IceInternal::Options::NeedArg);
    opts.addOpt("", "server-version", IceInternal::Options::NeedArg);

    try
    {
        if (!opts.parse(args).empty())
        {
            consoleErr << args[0] << ": too many arguments" << endl;
            usage(args[0]);
            return 1;
        }
    }
    catch (const IceInternal::BadOptException& e)
    {
        consoleErr << args[0] << ": " << e.what() << endl;
        usage(args[0]);
        return 1;
    }

    if (opts.isSet("help"))
    {
        usage(args[0]);
        return 0;
    }

    if (opts.isSet("version"))
    {
        consoleOut << ICE_STRING_VERSION << endl;
        return 0;
    }

    if (!(opts.isSet("import") ^ opts.isSet("export")))
    {
        consoleErr << args[0] << ": either --import or --export must be set" << endl;
        usage(args[0]);
        return 1;
    }

    if (!(opts.isSet("dbhome") ^ opts.isSet("dbpath")))
    {
        consoleErr << args[0] << ": set the database environment directory with either --dbhome or --dbpath" << endl;
        usage(args[0]);
        return 1;
    }

    bool debug = opts.isSet("debug");
    bool import = opts.isSet("import");
    string dbFile = opts.optArg(import ? "import" : "export");
    string dbPath;
    if (opts.isSet("dbhome"))
    {
        dbPath = opts.optArg("dbhome");
    }
    else
    {
        dbPath = opts.optArg("dbpath");
    }

    string mapSizeStr = opts.optArg("mapsize");
    size_t mapSize = IceDB::getMapSize(stoi(mapSizeStr));
    customSliceLoader->setServerVersion(opts.optArg("server-version"));

    try
    {
        IceGrid::AllData data;

        IceDB::IceContext dbContext{communicator};

        if (import)
        {
            consoleOut << "Importing database to directory '" << dbPath << "' from file '" << dbFile << "'" << endl;

            if (!IceInternal::directoryExists(dbPath))
            {
                consoleErr << args[0] << ": output directory does not exist: " << dbPath << endl;
                return 1;
            }

            if (!IceInternal::isEmptyDirectory(dbPath))
            {
                consoleErr << args[0] << ": output directory is not empty: " << dbPath << endl;
                return 1;
            }

            ifstream fs(IceInternal::streamFilename(dbFile).c_str(), ios::binary);
            if (fs.fail())
            {
                consoleErr << args[0] << ": could not open input file: " << IceInternal::errorToString(errno) << endl;
                return 1;
            }
            fs.unsetf(ios::skipws);

            fs.seekg(0, ios::end);
            streampos fileSize = fs.tellg();

            if (!fileSize)
            {
                fs.close();
                consoleErr << args[0] << ": empty input file" << endl;
                return 1;
            }

            fs.seekg(0, ios::beg);

            vector<byte> buf;
            buf.reserve(static_cast<size_t>(fileSize));
            fs.read(reinterpret_cast<char*>(buf.data()), fileSize);
            fs.close();

            Ice::InputStream stream(communicator, Ice::Encoding_1_1, buf);

            string type;
            int version;

            stream.read(type);
            if (type != "IceGrid")
            {
                consoleErr << args[0] << ": incorrect input file type: " << type << endl;
                return 1;
            }
            stream.read(version);
            if (version / 100 <= 305)
            {
                consoleErr << args[0] << ": cannot read file created by IceGrid 3.5 or earlier." << endl;
                return 1;
            }
            stream.read(data);

            {
                IceDB::Env env(dbPath, 5, mapSize);
                IceDB::ReadWriteTxn txn(env);

                if (debug)
                {
                    consoleOut << "Writing Applications Map:" << endl;
                }

                IceDB::Dbi<string, IceGrid::ApplicationInfo, IceDB::IceContext, Ice::OutputStream> apps(
                    txn,
                    "applications",
                    dbContext,
                    MDB_CREATE);

                for (const auto& application : data.applications)
                {
                    if (debug)
                    {
                        consoleOut << "  NAME = " << application.descriptor.name << endl;
                    }
                    apps.put(txn, application.descriptor.name, application);
                }

                if (debug)
                {
                    consoleOut << "Writing Adapters Map:" << endl;
                }

                IceDB::Dbi<string, IceGrid::AdapterInfo, IceDB::IceContext, Ice::OutputStream> adpts(
                    txn,
                    "adapters",
                    dbContext,
                    MDB_CREATE);

                for (const auto& adapter : data.adapters)
                {
                    if (debug)
                    {
                        consoleOut << "  NAME = " << adapter.id << endl;
                    }
                    adpts.put(txn, adapter.id, adapter);
                }

                if (debug)
                {
                    consoleOut << "Writing Objects Map:" << endl;
                }

                IceDB::Dbi<Ice::Identity, IceGrid::ObjectInfo, IceDB::IceContext, Ice::OutputStream> objs(
                    txn,
                    "objects",
                    dbContext,
                    MDB_CREATE);

                for (const auto& object : data.objects)
                {
                    if (debug)
                    {
                        consoleOut << "  NAME = " << communicator->identityToString(object.proxy->ice_getIdentity())
                                   << endl;
                    }
                    objs.put(txn, object.proxy->ice_getIdentity(), object);
                }

                if (debug)
                {
                    consoleOut << "Writing Internal Objects Map:" << endl;
                }

                IceDB::Dbi<Ice::Identity, IceGrid::ObjectInfo, IceDB::IceContext, Ice::OutputStream> internalObjs(
                    txn,
                    "internal-objects",
                    dbContext,
                    MDB_CREATE);

                for (const auto& internalObject : data.internalObjects)
                {
                    if (debug)
                    {
                        consoleOut << "  NAME = "
                                   << communicator->identityToString(internalObject.proxy->ice_getIdentity()) << endl;
                    }
                    internalObjs.put(txn, internalObject.proxy->ice_getIdentity(), internalObject);
                }

                if (debug)
                {
                    consoleOut << "Writing Serials Map:" << endl;
                }

                IceDB::Dbi<string, int64_t, IceDB::IceContext, Ice::OutputStream> srls(
                    txn,
                    "serials",
                    dbContext,
                    MDB_CREATE);

                for (const auto& serial : data.serials)
                {
                    if (debug)
                    {
                        consoleOut << "  NAME = " << serial.first << endl;
                    }
                    srls.put(txn, serial.first, serial.second);
                }

                txn.commit();
                env.close();
            }
        }
        else
        {
            consoleOut << "Exporting database from directory '" << dbPath << "' to file '" << dbFile << "'" << endl;

            {
                IceDB::Env env(dbPath, 5);
                IceDB::ReadOnlyTxn txn(env);

                if (debug)
                {
                    consoleOut << "Reading Application Map:" << endl;
                }

                IceDB::Dbi<string, IceGrid::ApplicationInfo, IceDB::IceContext, Ice::OutputStream> applications(
                    txn,
                    "applications",
                    dbContext,
                    0);

                string name;
                IceGrid::ApplicationInfo application;
                IceDB::ReadOnlyCursor<string, IceGrid::ApplicationInfo, IceDB::IceContext, Ice::OutputStream> appCursor(
                    applications,
                    txn);
                while (appCursor.get(name, application, MDB_NEXT))
                {
                    if (debug)
                    {
                        consoleOut << "  APPLICATION = " << name << endl;
                    }
                    data.applications.push_back(application);
                }
                appCursor.close();

                if (debug)
                {
                    consoleOut << "Reading Adapter Map:" << endl;
                }

                IceDB::Dbi<string, IceGrid::AdapterInfo, IceDB::IceContext, Ice::OutputStream> adapters(
                    txn,
                    "adapters",
                    dbContext,
                    0);

                IceGrid::AdapterInfo adapter;
                IceDB::ReadOnlyCursor<string, IceGrid::AdapterInfo, IceDB::IceContext, Ice::OutputStream> adapterCursor(
                    adapters,
                    txn);
                while (adapterCursor.get(name, adapter, MDB_NEXT))
                {
                    if (debug)
                    {
                        consoleOut << "  ADAPTER = " << name << endl;
                    }
                    data.adapters.push_back(adapter);
                }
                adapterCursor.close();

                if (debug)
                {
                    consoleOut << "Reading Object Map:" << endl;
                }

                IceDB::Dbi<Ice::Identity, IceGrid::ObjectInfo, IceDB::IceContext, Ice::OutputStream> objects(
                    txn,
                    "objects",
                    dbContext,
                    0);

                Ice::Identity id;
                IceGrid::ObjectInfo object;
                IceDB::ReadOnlyCursor<Ice::Identity, IceGrid::ObjectInfo, IceDB::IceContext, Ice::OutputStream>
                    objCursor(objects, txn);
                while (objCursor.get(id, object, MDB_NEXT))
                {
                    if (debug)
                    {
                        consoleOut << "  IDENTITY = " << communicator->identityToString(id) << endl;
                    }
                    data.objects.push_back(object);
                }
                objCursor.close();

                if (debug)
                {
                    consoleOut << "Reading Internal Object Map:" << endl;
                }

                IceDB::Dbi<Ice::Identity, IceGrid::ObjectInfo, IceDB::IceContext, Ice::OutputStream> internalObjects(
                    txn,
                    "internal-objects",
                    dbContext,
                    0);

                IceDB::ReadOnlyCursor<Ice::Identity, IceGrid::ObjectInfo, IceDB::IceContext, Ice::OutputStream>
                    iobjCursor(internalObjects, txn);
                while (iobjCursor.get(id, object, MDB_NEXT))
                {
                    if (debug)
                    {
                        consoleOut << "  IDENTITY = " << communicator->identityToString(id) << endl;
                    }
                    data.internalObjects.push_back(object);
                }
                iobjCursor.close();

                if (debug)
                {
                    consoleOut << "Reading Serials Map:" << endl;
                }

                IceDB::Dbi<string, int64_t, IceDB::IceContext, Ice::OutputStream> serials(txn, "serials", dbContext, 0);

                int64_t serial;
                IceDB::ReadOnlyCursor<string, int64_t, IceDB::IceContext, Ice::OutputStream> serialCursor(serials, txn);
                while (serialCursor.get(name, serial, MDB_NEXT))
                {
                    if (debug)
                    {
                        consoleOut << "  NAME = " << name << endl;
                    }
                    data.serials.insert(std::make_pair(name, serial));
                }
                serialCursor.close();

                txn.rollback();
                env.close();
            }

            Ice::OutputStream stream{Ice::Encoding_1_1};
            stream.write("IceGrid");
            stream.write(ICE_INT_VERSION);
            stream.write(data);

            ofstream fs(IceInternal::streamFilename(dbFile).c_str(), ios::binary);
            if (fs.fail())
            {
                consoleErr << args[0] << ": could not open output file: " << IceInternal::errorToString(errno) << endl;
                return 1;
            }
            fs.write(reinterpret_cast<const char*>(stream.b.begin()), static_cast<streamsize>(stream.b.size()));
            fs.close();
        }
    }
    catch (const Ice::Exception& ex)
    {
        consoleErr << args[0] << ": " << (import ? "import" : "export") << " failed:\n" << ex << endl;
        return 1;
    }

    return 0;
}
