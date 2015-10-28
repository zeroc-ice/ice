// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <Ice/Application.h>
#include <IceDB/IceDB.h>
#include <IceStorm/DBTypes.h>
#include <IcePatch2Lib/Util.h>
#include <IceUtil/DisableWarnings.h>

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
        "--dbhome DIR           The database directory.\n"
        "-d, --debug            Print debug messages."
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
        cerr << e.reason << endl;
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
        cerr << "Either --import or --export must be set" << endl;
        usage();
        return EXIT_FAILURE;
    }

    if(!opts.isSet("dbhome"))
    {
        cerr << "Database path must be specified" << endl;
        usage();
        return EXIT_FAILURE;
    }

    bool debug = opts.isSet("debug");
    bool import = opts.isSet("import");
    string dbFile = opts.optArg(import ? "import" : "export");
    string dbPath = opts.optArg("dbhome");

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
                cerr << "Output directory does not exist: " << dbPath << endl;
                return EXIT_FAILURE;
            }

            StringSeq files = IcePatch2Internal::readDirectory(dbPath);
            if(!files.empty())
            {
                cerr << "Output directory is not empty: " << dbPath << endl;
                return EXIT_FAILURE;
            }

            ifstream fs(dbFile.c_str(), ios::binary);
            if(fs.fail())
            {
                cerr << "Could not open input file: " << strerror(errno) << endl;
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

            Ice::InputStreamPtr stream = Ice::wrapInputStream(communicator(), buf, dbContext.encoding);
            stream->read(type);
            if(type != "IceStorm")
            {
                cerr << "Incorrect input file type: " << type << endl;
                return EXIT_FAILURE;
            }
            stream->read(version);
            stream->read(data);

            {
                IceDB::Env env(dbPath, 2);
                IceDB::ReadWriteTxn txn(env);

                if(debug)
                {
                    cout << "Writing LLU Map:" << endl;
                }

                IceDB::Dbi<string, LogUpdate, IceDB::IceContext, Ice::OutputStreamPtr>
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

                IceDB::Dbi<SubscriberRecordKey, SubscriberRecord, IceDB::IceContext, Ice::OutputStreamPtr>
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

                IceDB::Dbi<string, LogUpdate, IceDB::IceContext, Ice::OutputStreamPtr>
                    lluMap(txn, "llu", dbContext, 0);

                string s;
                LogUpdate llu;
                IceDB::ReadOnlyCursor<string, LogUpdate, IceDB::IceContext, Ice::OutputStreamPtr>
                    lluCursor(lluMap, txn);
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

                IceDB::Dbi<SubscriberRecordKey, SubscriberRecord, IceDB::IceContext, Ice::OutputStreamPtr>
                    subscriberMap(txn, "subscribers", dbContext, 0);

                SubscriberRecordKey key;
                SubscriberRecord record;
                IceDB::ReadOnlyCursor<SubscriberRecordKey, SubscriberRecord, IceDB::IceContext, Ice::OutputStreamPtr>
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

                txn.abort();
                env.close();
            }

            Ice::OutputStreamPtr stream = Ice::createOutputStream(communicator(), dbContext.encoding);
            stream->write("IceStorm");
            stream->write(ICE_INT_VERSION);
            stream->write(data);
            pair<const Ice::Byte*, const Ice::Byte*> buf = stream->finished();

            ofstream fs(dbFile.c_str(), ios::binary);
            if(fs.fail())
            {
                cerr << "Could not open output file: " << strerror(errno) << endl;
                return EXIT_FAILURE;
            }
            fs.write(reinterpret_cast<const char*>(buf.first), buf.second - buf.first);
            fs.close();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        cerr << (import ? "Import" : "Export") << " failed:\n" << ex << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
