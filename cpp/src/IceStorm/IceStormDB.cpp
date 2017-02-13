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
#include <DBTypes.h>
#include <LLUMap.h>
#include <SubscriberMap.h>

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
        IceStorm::AllData data;

        EncodingVersion encoding;
        encoding.major = 1;
        encoding.minor = 1;

        communicator()->getProperties()->setProperty("Freeze.DbEnv.IceStorm.DbHome", dbPath);

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
            if(type != "IceStorm")
            {
                cerr << argv[0] << ": incorrect input file type: " << type << endl;
                return EXIT_FAILURE;
            }
            stream->read(version);
            stream->read(data);

            {
                Freeze::ConnectionPtr connection = Freeze::createConnection(communicator(), "IceStorm");
                Freeze::TransactionHolder txn(connection);

                if(debug)
                {
                    cout << "Writing LLU Map:" << endl;
                }

                IceStorm::LLUMap llumap(connection, "llu");
                for(StringLogUpdateDict::const_iterator p = data.llus.begin(); p != data.llus.end(); ++p)
                {
                    if(debug)
                    {
                        cout << "  KEY = " << p->first << endl;
                    }
                    llumap.put(*p);
                }

                if(debug)
                {
                    cout << "Writing Subscriber Map:" << endl;
                }

                IceStorm::SubscriberMap subscribers(connection, "subscribers");
                for(SubscriberRecordDict::const_iterator q = data.subscribers.begin(); q != data.subscribers.end(); ++q)
                {
                    if(debug)
                    {
                        cout << "  KEY = TOPIC(" << communicator()->identityToString(q->first.topic)
                             << ") ID(" << communicator()->identityToString(q->first.id) << ")" <<endl;
                    }
                    subscribers.put(*q);
                }

                txn.commit();
            }
        }
        else
        {
            cout << "Exporting database from directory " << dbPath << " to file " << dbFile << endl;

            {
                Freeze::ConnectionPtr connection = Freeze::createConnection(communicator(), "IceStorm");

                if(debug)
                {
                    cout << "Reading LLU Map:" << endl;
                }

                IceStorm::LLUMap llumap(connection, "llu", false);
                for(IceStorm::LLUMap::const_iterator p = llumap.begin(); p != llumap.end(); ++p)
                {
                    if(debug)
                    {
                        cout << "  KEY = " << p->first << endl;
                    }
                    data.llus.insert(*p);
                }

                if(debug)
                {
                    cout << "Reading Subscriber Map:" << endl;
                }

                IceStorm::SubscriberMap subscribers(connection, "subscribers", false);
                for(IceStorm::SubscriberMap::const_iterator q = subscribers.begin(); q != subscribers.end(); ++q)
                {
                    if(debug)
                    {
                        cout << "  KEY = TOPIC(" << communicator()->identityToString(q->first.topic)
                             << ") ID(" << communicator()->identityToString(q->first.id) << ")" <<endl;
                    }
                    data.subscribers.insert(*q);
                }
            }

            Ice::OutputStreamPtr stream = Ice::createOutputStream(communicator(), encoding);
            stream->write("IceStorm");
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
