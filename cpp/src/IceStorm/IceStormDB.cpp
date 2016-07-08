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
#include <IceStorm/DBTypes.h>
#include <IceUtil/DisableWarnings.h>
#include <fstream>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace IceStormElection;

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

    try
    {
        IceStorm::AllData data;

        IceDB::IceContext dbContext;
        dbContext.communicator = communicator();
        dbContext.encoding.major = 1;
        dbContext.encoding.minor = 1;

        if(import)
        {
            cout << "Importing database to directory " << dbPath << " from file " << dbFile << endl;

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

            string type;
            int version;

            Ice::InputStream stream(communicator(), dbContext.encoding, buf);
            stream.read(type);
            if(type != "IceStorm")
            {
                cerr << argv[0] << ": incorrect input file type: " << type << endl;
                return EXIT_FAILURE;
            }
            stream.read(version);
            stream.read(data);

            {
                IceDB::Env env(dbPath, 2, mapSize);
                IceDB::ReadWriteTxn txn(env);

                if(debug)
                {
                    cout << "Writing LLU Map:" << endl;
                }

                IceDB::Dbi<string, LogUpdate, IceDB::IceContext, Ice::OutputStream>
                    lluMap(txn, "llu", dbContext, MDB_CREATE);

                for(StringLogUpdateDict::const_iterator p = data.llus.begin(); p != data.llus.end(); ++p)
                {
                    if(debug)
                    {
                        cout << "  KEY = " << p->first << endl;
                    }
                    lluMap.put(txn, p->first, p->second);
                }

                if(debug)
                {
                    cout << "Writing Subscriber Map:" << endl;
                }

                IceDB::Dbi<SubscriberRecordKey, SubscriberRecord, IceDB::IceContext, Ice::OutputStream>
                    subscriberMap(txn, "subscribers", dbContext, MDB_CREATE);

                for(SubscriberRecordDict::const_iterator q = data.subscribers.begin(); q != data.subscribers.end(); ++q)
                {
                    if(debug)
                    {
                        cout << "  KEY = TOPIC(" << communicator()->identityToString(q->first.topic)
                             << ") ID(" << communicator()->identityToString(q->first.id) << ")" <<endl;
                    }
                    subscriberMap.put(txn, q->first, q->second);
                }

                txn.commit();
                env.close();
            }
        }
        else
        {
            cout << "Exporting database from directory " << dbPath << " to file " << dbFile << endl;

            {
                IceDB::Env env(dbPath, 2);
                IceDB::ReadOnlyTxn txn(env);

                if(debug)
                {
                    cout << "Reading LLU Map:" << endl;
                }

                IceDB::Dbi<string, LogUpdate, IceDB::IceContext, Ice::OutputStream>
                    lluMap(txn, "llu", dbContext, 0);

                string s;
                LogUpdate llu;
                IceDB::ReadOnlyCursor<string, LogUpdate, IceDB::IceContext, Ice::OutputStream> lluCursor(lluMap, txn);
                while(lluCursor.get(s, llu, MDB_NEXT))
                {
                    if(debug)
                    {
                        cout << "  KEY = " << s << endl;
                    }
                    data.llus.insert(std::make_pair(s, llu));
                }
                lluCursor.close();

                if(debug)
                {
                    cout << "Reading Subscriber Map:" << endl;
                }

                IceDB::Dbi<SubscriberRecordKey, SubscriberRecord, IceDB::IceContext, Ice::OutputStream>
                    subscriberMap(txn, "subscribers", dbContext, 0);

                SubscriberRecordKey key;
                SubscriberRecord record;
                IceDB::ReadOnlyCursor<SubscriberRecordKey, SubscriberRecord, IceDB::IceContext, Ice::OutputStream>
                    subCursor(subscriberMap, txn);
                while(subCursor.get(key, record, MDB_NEXT))
                {
                    if(debug)
                    {
                        cout << "  KEY = TOPIC(" << communicator()->identityToString(key.topic)
                             << ") ID(" << communicator()->identityToString(key.id) << ")" <<endl;
                    }
                    data.subscribers.insert(std::make_pair(key, record));
                }
                subCursor.close();

                txn.rollback();
                env.close();
            }

            Ice::OutputStream stream(communicator(), dbContext.encoding);
            stream.write("IceStorm");
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
