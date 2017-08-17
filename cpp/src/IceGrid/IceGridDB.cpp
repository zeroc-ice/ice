// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/Options.h>
#include <IceUtil/FileUtil.h>
#include <Ice/Application.h>
#include <Freeze/Freeze.h>
#include <Freeze/CatalogIndexList.h>
#include <IceGrid/Admin.h>
#include <IceGrid/DBTypes.h>
#include <IceGrid/StringApplicationInfoDict.h>
#include <IceGrid/StringAdapterInfoDict.h>
#include <IceGrid/IdentityObjectInfoDict.h>
#include <IceGrid/SerialsDict.h>

using namespace std;
using namespace Ice;
using namespace IceGrid;

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
        "--dbhome DIR           The database directory.\n"
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

    if((!opts.isSet("import") && !opts.isSet("export")) || (opts.isSet("import") && opts.isSet("export")))
    {
        cerr << argv[0] << ": either --import or --export must be set" << endl;
        usage();
        return EXIT_FAILURE;
    }

    if(!opts.isSet("dbhome"))
    {
        cerr << argv[0] << ": database path must be specified" << endl;
        usage();
        return EXIT_FAILURE;
    }

    bool debug = opts.isSet("debug");
    bool import = opts.isSet("import");
    string dbFile = opts.optArg(import ? "import" : "export");
    string dbPath = opts.optArg("dbhome");

    try
    {
        IceGrid::AllData data;

        EncodingVersion encoding;
        encoding.major = 1;
        encoding.minor = 1;

        communicator()->getProperties()->setProperty("Freeze.DbEnv.Registry.DbHome", dbPath);

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

            ifstream fs(dbFile.c_str(), ios::binary);
            if(fs.fail())
            {
                cerr << argv[0] << ": could not open input file: " << strerror(errno) << endl;
                return EXIT_FAILURE;
            }
            fs.unsetf(ios::skipws);

            fs.seekg(0, ios::end);
            streampos fileSize = fs.tellg();
            fs.seekg(0, ios::beg);

            vector<Ice::Byte> buf;
            buf.reserve(static_cast<size_t>(fileSize));
            buf.insert(buf.begin(), istream_iterator<Ice::Byte>(fs), istream_iterator<Ice::Byte>());

            fs.close();

            string type;
            int version;

            Ice::InputStreamPtr stream = Ice::wrapInputStream(communicator(), buf, encoding);
            stream->read(type);
            if(type != "IceGrid")
            {
                cerr << argv[0] << ": incorrect input file type: " << type << endl;
                return EXIT_FAILURE;
            }
            stream->read(version);
            stream->read(data);

            {
                Freeze::ConnectionPtr connection = Freeze::createConnection(communicator(), "Registry");
                Freeze::TransactionHolder txn(connection);

                if(debug)
                {
                    cout << "Writing Applications Map:" << endl;
                }

                StringApplicationInfoDict applications(connection, "applications");
                for(ApplicationInfoSeq::const_iterator p = data.applications.begin();
                    p != data.applications.end();
                    ++p)
                {
                    if(debug)
                    {
                        cout << "  NAME = " << p->descriptor.name << endl;
                    }
                    applications.put(StringApplicationInfoDict::value_type(p->descriptor.name, *p));
                }

                if(debug)
                {
                    cout << "Writing Adapters Map:" << endl;
                }

                StringAdapterInfoDict adapters(connection, "adapters");
                for(AdapterInfoSeq::const_iterator p = data.adapters.begin(); p != data.adapters.end(); ++p)
                {
                    if(debug)
                    {
                        cout << "  NAME = " << p->id << endl;
                    }
                    adapters.put(StringAdapterInfoDict::value_type(p->id, *p));
                }

                if(debug)
                {
                    cout << "Writing Objects Map:" << endl;
                }

                IdentityObjectInfoDict objects(connection, "objects");
                for(ObjectInfoSeq::const_iterator p = data.objects.begin(); p != data.objects.end(); ++p)
                {
                    if(debug)
                    {
                        cout << "  NAME = " << communicator()->identityToString(p->proxy->ice_getIdentity()) << endl;
                    }
                    objects.put(IdentityObjectInfoDict::value_type(p->proxy->ice_getIdentity(), *p));
                }

                if(debug)
                {
                    cout << "Writing Internal Objects Map:" << endl;
                }

                IdentityObjectInfoDict internalObjects(connection, "internal-objects");
                for(ObjectInfoSeq::const_iterator p = data.internalObjects.begin();
                    p != data.internalObjects.end();
                    ++p)
                {
                    if(debug)
                    {
                        cout << "  NAME = " << communicator()->identityToString(p->proxy->ice_getIdentity()) << endl;
                    }
                    internalObjects.put(IdentityObjectInfoDict::value_type(p->proxy->ice_getIdentity(), *p));
                }

                if(debug)
                {
                    cout << "Writing Serials Map:" << endl;
                }

                SerialsDict serials(connection, "serials");
                for(StringLongDict::const_iterator p = data.serials.begin(); p != data.serials.end(); ++p)
                {
                    if(debug)
                    {
                        cout << "  NAME = " << p->first << endl;
                    }
                    serials.put(SerialsDict::value_type(p->first, p->second));
                }

                txn.commit();
            }
        }
        else
        {
            cout << "Exporting database from directory `" << dbPath << "' to file `" << dbFile << "'" << endl;

            {
                Freeze::ConnectionPtr connection = Freeze::createConnection(communicator(), "Registry");
                Freeze::TransactionHolder txn(connection);

                if(debug)
                {
                    cout << "Reading Application Map:" << endl;
                }

                IceGrid::StringApplicationInfoDict applications(connection, "applications", false);
                for(IceGrid::StringApplicationInfoDict::const_iterator p = applications.begin();
                    p != applications.end();
                    ++p)
                {
                    if(debug)
                    {
                        cout << "  APPLICATION = " << p->first << endl;
                    }
                    data.applications.push_back(p->second);
                }

                if(debug)
                {
                    cout << "Reading Adapter Map:" << endl;
                }

                StringAdapterInfoDict adapters(connection, "adapters", false);
                for(StringAdapterInfoDict::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
                {
                    if(debug)
                    {
                        cout << "  ADAPTER = " << p->first << endl;
                    }
                    data.adapters.push_back(p->second);
                }

                if(debug)
                {
                    cout << "Reading Object Map:" << endl;
                }

                IdentityObjectInfoDict objects(connection, "objects", false);
                for(IdentityObjectInfoDict::const_iterator p = objects.begin(); p != objects.end(); ++p)
                {
                    if(debug)
                    {
                        cout << "  IDENTITY = " << communicator()->identityToString(p->first) << endl;
                    }
                    data.objects.push_back(p->second);
                }

                if(debug)
                {
                    cout << "Reading Internal Object Map:" << endl;
                }

                IdentityObjectInfoDict internalObjects(connection, "internal-objects", false);
                for(IdentityObjectInfoDict::const_iterator p = internalObjects.begin();
                    p != internalObjects.end();
                    ++p)
                {
                    if(debug)
                    {
                        cout << "  IDENTITY = " << communicator()->identityToString(p->first) << endl;
                    }
                    data.internalObjects.push_back(p->second);
                }

                if(debug)
                {
                    cout << "Reading Serials Map:" << endl;
                }

                SerialsDict serials(connection, "serials", false);
                for(SerialsDict::const_iterator p = serials.begin(); p != serials.end(); ++p)
                {
                    if(debug)
                    {
                        cout << "  NAME = " << p->first << endl;
                    }
                    data.serials.insert(std::make_pair(p->first, p->second));
                }

                txn.rollback();
            }

            Ice::OutputStreamPtr stream = Ice::createOutputStream(communicator(), encoding);
            stream->write("IceGrid");
            stream->write(ICE_INT_VERSION);
            stream->write(data);
            pair<const Ice::Byte*, const Ice::Byte*> buf = stream->finished();

            ofstream fs(IceUtilInternal::streamFilename(dbFile).c_str(), ios::binary);
            if(fs.fail())
            {
                cerr << argv[0] << ": could not open output file: " << strerror(errno) << endl;
                return EXIT_FAILURE;
            }
            fs.write(reinterpret_cast<const char*>(buf.first), buf.second - buf.first);
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
