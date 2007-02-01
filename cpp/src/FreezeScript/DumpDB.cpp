// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FreezeScript/DumpDescriptors.h>
#include <FreezeScript/Util.h>
#include <FreezeScript/Exception.h>
#include <IceUtil/OutputUtil.h>
#include <IceUtil/Options.h>
#include <db_cxx.h>
#include <sys/stat.h>
#include <fstream>
#include <algorithm>

using namespace std;
using namespace IceUtil;

#ifdef _WIN32
#   define FREEZE_SCRIPT_DB_MODE 0
#else
#   define FREEZE_SCRIPT_DB_MODE (S_IRUSR | S_IWUSR)
#endif

namespace FreezeScript
{

//
// SliceVisitor generates default descriptors.
//
class SliceVisitor : public Slice::ParserVisitor
{
public:

    SliceVisitor(IceUtil::XMLOutput&, const Slice::TypePtr&, const Slice::TypePtr&, const string&);

    virtual bool visitClassDefStart(const Slice::ClassDefPtr&);
    virtual bool visitStructStart(const Slice::StructPtr&);
    virtual void visitSequence(const Slice::SequencePtr&);
    virtual void visitDictionary(const Slice::DictionaryPtr&);
    virtual void visitEnum(const Slice::EnumPtr&);

private:

    IceUtil::XMLOutput& _out;
};

class DescriptorHandler : public IceXML::Handler
{
public:

    DescriptorHandler(const DataFactoryPtr&, const Slice::UnitPtr&, const ErrorReporterPtr&,
                      const FreezeScript::ObjectFactoryPtr&);

    virtual void startElement(const std::string&, const IceXML::Attributes&, int, int);
    virtual void endElement(const std::string&, int, int);
    virtual void characters(const std::string&, int, int);
    virtual void error(const std::string&, int, int);

    DumpDBDescriptorPtr descriptor() const;

private:

    DataFactoryPtr _factory;
    Slice::UnitPtr _unit;
    ErrorReporterPtr _errorReporter;
    DescriptorPtr _current;
    DumpDBDescriptorPtr _descriptor;
    FreezeScript::ObjectFactoryPtr _objectFactory;
};

}

static void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options] [dbenv [db]]\n";
    cerr <<
        "Options:\n"
        "-h, --help            Show this message.\n"
        "-v, --version         Display the Ice version.\n"
        "-DNAME                Define NAME as 1.\n"
        "-DNAME=DEF            Define NAME as DEF.\n"
        "-UNAME                Remove any definition for NAME.\n"
        "-IDIR                 Put DIR in the include file search path.\n"
        "-d, --debug           Print debug messages.\n"
        "--ice                 Permit `Ice' prefix (for building Ice source code only)\n"
        "-o FILE               Output sample descriptors into the file FILE.\n"
        "-f FILE               Execute the descriptors in the file FILE.\n"
        "--load SLICE          Load Slice definitions from the file SLICE.\n"
        "-e                    Indicates the database is an Evictor database.\n"
        "--key TYPE            Specifies the Slice type of the database key.\n"
        "--value TYPE          Specifies the Slice type of the database value.\n"
        "--select EXPR         Dump a record only if EXPR is true.\n"
        "-c, --catalog         Display information about the databases in an\n"
        "                      environment, or about a particular database.\n"
        ;
    // Note: --case-sensitive is intentionally not shown here!
}

static void
printCatalogData(const string& dbName, const Freeze::CatalogData& data)
{
    cout << dbName << ": ";
    if(data.evictor)
    {
        cout << "Evictor database" << endl;
    }
    else
    {
        cout << "Map database" << endl;
        cout << "  key type   = " << data.key << endl;
        cout << "  value type = " << data.value << endl;
    }
}

static int
run(int argc, char** argv, const Ice::CommunicatorPtr& communicator)
{
    string cppArgs;
    bool debug;
    bool ice = true; // Needs to be true in order to create default definitions.
    string outputFile;
    string inputFile;
    vector<string> slice;
    bool evictor;
    string keyTypeName;
    string valueTypeName;
    string selectExpr;
    bool caseSensitive;
    string dbEnvName, dbName;

    IceUtil::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("D", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("U", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("I", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("d", "debug");
    opts.addOpt("", "ice");
    opts.addOpt("o", "", IceUtil::Options::NeedArg);
    opts.addOpt("f", "", IceUtil::Options::NeedArg);
    opts.addOpt("", "load", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("e");
    opts.addOpt("", "key", IceUtil::Options::NeedArg);
    opts.addOpt("", "value", IceUtil::Options::NeedArg);
    opts.addOpt("", "select", IceUtil::Options::NeedArg);
    opts.addOpt("c", "catalog");
    opts.addOpt("", "case-sensitive");

    vector<string> args;
    try
    {
        args = opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtil::BadOptException& e)
    {
        cerr << argv[0] << ": " << e.reason << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if(opts.isSet("h"))
    {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }
    if(opts.isSet("version"))
    {
        cout << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }
    if(opts.isSet("c"))
    {
        if(args.empty())
        {
            cerr << argv[0] << ": no database environment specified." << endl;
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        else if(args.size() > 2)
        {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        try
        {
            FreezeScript::CatalogDataMap catalog = FreezeScript::readCatalog(communicator, args[0]);
            if(args.size() == 1)
            {
                if(catalog.empty())
                {
                    cout << "Catalog is empty." << endl;
                }
                else
                {
                    cout << "Catalog contents:" << endl;
                    for(FreezeScript::CatalogDataMap::const_iterator p = catalog.begin(); p != catalog.end(); ++p)
                    {
                        cout << endl;
                        printCatalogData(p->first, p->second);
                    }
                }
            }
            else
            {
                FreezeScript::CatalogDataMap::const_iterator p = catalog.find(args[1]);
                if(p == catalog.end())
                {
                    cerr << argv[0] << ": database `" << args[1] << "' not found in environment `" << args[0] << "'."
                         << endl;
                    return EXIT_FAILURE;
                }
                else
                {
                    printCatalogData(p->first, p->second);
                }
            }
            return EXIT_SUCCESS;
        }
        catch(const FreezeScript::FailureException& ex)
        {
            cerr << argv[0] << ": " << ex.reason() << endl;
            return EXIT_FAILURE;
        }
    }
    if(opts.isSet("D"))
    {
        vector<string> optargs = opts.argVec("D");
        for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
        {
            cppArgs += " -D" + *i;
        }
    }
    if(opts.isSet("U"))
    {
        vector<string> optargs = opts.argVec("U");
        for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
        {
            cppArgs += " -U" + *i;
        }
    }
    if(opts.isSet("I"))
    {
        vector<string> optargs = opts.argVec("I");
        for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
        {
            cppArgs += " -I" + *i;
        }
    }
    debug = opts.isSet("debug");

    // No need to set --ice option here -- it is always true.

    if(opts.isSet("o"))
    {
        outputFile = opts.optArg("o");
    }
    if(opts.isSet("f"))
    {
        inputFile = opts.optArg("f");
    }
    if(opts.isSet("load"))
    {
        vector<string> optArgs = opts.argVec("load");
        for(vector<string>::const_iterator i = optArgs.begin(); i != optArgs.end(); ++i)
        {
            slice.push_back(*i);
        }
    }
    evictor = opts.isSet("e");
    if(opts.isSet("key"))
    {
        keyTypeName = opts.optArg("key");
    }
    if(opts.isSet("value"))
    {
        valueTypeName = opts.optArg("value");
    }
    if(opts.isSet("select"))
    {
        selectExpr = opts.optArg("select");
    }
    caseSensitive = opts.isSet("case-sensitive");

    if(outputFile.empty() && args.size() != 2)
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if(!args.empty())
    {
        dbEnvName = args[0];
    }
    if(args.size() == 2)
    {
        dbName = args[1];
    }
    else
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if(!inputFile.empty() && !selectExpr.empty())
    {
        cerr << argv[0] << ": an input file cannot be specified with --select" << endl;
        return EXIT_FAILURE;
    }

    Slice::UnitPtr unit = Slice::Unit::createUnit(true, true, ice, caseSensitive);
    FreezeScript::Destroyer<Slice::UnitPtr> unitD(unit);
    if(!FreezeScript::parseSlice(argv[0], unit, slice, cppArgs, debug))
    {
        return EXIT_FAILURE;
    }

    FreezeScript::createEvictorSliceTypes(unit);

    //
    // If no input file was provided, then we need to generate default descriptors.
    //
    string descriptors;
    if(inputFile.empty())
    {
        const string evictorKeyTypeName = "::Ice::Identity";
        const string evictorValueTypeName = "::Freeze::ObjectRecord";

        if((!keyTypeName.empty() && valueTypeName.empty()) || (keyTypeName.empty() && !valueTypeName.empty()))
        {
            cerr << argv[0] << ": a key type and a value type must be specified" << endl;
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        else if(!evictor && keyTypeName.empty() && valueTypeName.empty())
        {
            try
            {
                FreezeScript::CatalogDataMap catalog = FreezeScript::readCatalog(communicator, dbEnvName);
                FreezeScript::CatalogDataMap::iterator p = catalog.find(dbName);
                if(p == catalog.end())
                {
                    cerr << argv[0] << ": database `" << dbName << "' not found in catalog." << endl;
                    cerr << "Current catalog databases:" << endl;
                    for(p = catalog.begin(); p != catalog.end(); ++p)
                    {
                        cerr << "  " << p->first << endl;
                    }
                    return EXIT_FAILURE;
                }
                else
                {
                    if(p->second.evictor)
                    {
                        evictor = true;
                    }
                    else
                    {
                        keyTypeName = p->second.key;
                        valueTypeName = p->second.value;
                    }
                }
            }
            catch(const FreezeScript::FailureException& ex)
            {
                cerr << argv[0] << ": " << ex.reason() << endl;
                return EXIT_FAILURE;
            }
        }

        if(evictor)
        {
            keyTypeName = evictorKeyTypeName;
            valueTypeName = evictorValueTypeName;
        }

        Slice::TypePtr keyType, valueType;
        Slice::TypeList l;

        l = unit->lookupType(keyTypeName, false);
        if(l.empty())
        {
            cerr << argv[0] << ": unknown key type `" << keyTypeName << "'" << endl;
            return EXIT_FAILURE;
        }
        keyType = l.front();

        l = unit->lookupType(valueTypeName, false);
        if(l.empty())
        {
            cerr << argv[0] << ": unknown value type `" << valueTypeName << "'" << endl;
            return EXIT_FAILURE;
        }
        valueType = l.front();

        ostringstream os;
        IceUtil::XMLOutput out(os);

        out << se("dumpdb");

        FreezeScript::SliceVisitor visitor(out, keyType, valueType, selectExpr);
        unit->visit(&visitor, false);

        out << ee;

        descriptors = os.str();

        if(!outputFile.empty())
        {
            ofstream of(outputFile.c_str());
            if(!of.good())
            {
                cerr << argv[0] << ": unable to open file `" << outputFile << "'" << endl;
                return EXIT_FAILURE;
            }
            of << descriptors << endl;
            of.close();
            return EXIT_SUCCESS;
        }
    }
    else
    {
        ifstream in(inputFile.c_str());
        char buff[1024];
        while(true)
        {
            in.read(buff, 1024);
            descriptors.append(buff, in.gcount());
            if(in.gcount() < 1024)
            {
                break;
            }
        }
        in.close();
    }
    FreezeScript::ObjectFactoryPtr objectFactory = new FreezeScript::ObjectFactory;
    communicator->addObjectFactory(objectFactory, "");

    DbEnv dbEnv(0);
    DbTxn* txn = 0;
    int status = EXIT_SUCCESS;
    try
    {
#ifdef _WIN32
        //
        // Berkeley DB may use a different C++ runtime.
        //
        dbEnv.set_alloc(::malloc, ::realloc, ::free);
#endif

        //
        // Open the database environment and start a transaction.
        //
        {
            u_int32_t flags = DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN | DB_RECOVER | DB_CREATE;
            dbEnv.open(dbEnvName.c_str(), flags, FREEZE_SCRIPT_DB_MODE);
        }
        dbEnv.txn_begin(0, &txn, 0);

        FreezeScript::ErrorReporterPtr errorReporter = new FreezeScript::ErrorReporter(cerr, false);
        try
        {
            FreezeScript::DataFactoryPtr factory = new FreezeScript::DataFactory(communicator, unit, errorReporter);
            FreezeScript::DescriptorHandler dh(factory, unit, errorReporter, objectFactory);

            istringstream istr(descriptors);
            IceXML::Parser::parse(istr, dh);

            FreezeScript::DumpDBDescriptorPtr descriptor = dh.descriptor();
            descriptor->validate();

            if(evictor)
            {
                //
                // The evictor database file contains multiple databases. We must first
                // determine the names of those databases, ignoring any whose names
                // begin with "$index:". Each database represents a separate facet, with
                // the facet name used as the database name. The database named "$default"
                // represents the main object.
                //
                vector<string> dbNames;
                {
                    Db db(&dbEnv, 0);
                    db.open(txn, dbName.c_str(), 0, DB_UNKNOWN, DB_RDONLY, 0);
                    Dbt dbKey, dbValue;
                    dbKey.set_flags(DB_DBT_MALLOC);
                    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

                    Dbc* dbc = 0;
                    db.cursor(0, &dbc, 0);

                    while(dbc->get(&dbKey, &dbValue, DB_NEXT) == 0)
                    {
                        string s(static_cast<char*>(dbKey.get_data()), dbKey.get_size());
                        if(s.find("$index:") != 0)
                        {
                            dbNames.push_back(s);
                        }
                        free(dbKey.get_data());
                    }

                    dbc->close();
                    db.close(0);
                }

                //
                // Dump each database.
                //
                for(vector<string>::iterator p = dbNames.begin(); p != dbNames.end(); ++p)
                {
                    string name = *p;
                    string facet = (name == "$default" ? string("") : name);
                    Db db(&dbEnv, 0);
                    db.open(txn, dbName.c_str(), name.c_str(), DB_BTREE, DB_RDONLY, FREEZE_SCRIPT_DB_MODE);
                    descriptor->dump(communicator, &db, txn, facet);
                    db.close(0);
                }
            }
            else
            {
                //
                // Dump a map database.
                //
                Db db(&dbEnv, 0);
                db.open(txn, dbName.c_str(), 0, DB_BTREE, DB_RDONLY, FREEZE_SCRIPT_DB_MODE);
                descriptor->dump(communicator, &db, txn, "");
                db.close(0);
            }
        }
        catch(const IceXML::ParserException& ex)
        {
            errorReporter->error(ex.reason());
        }
    }
    catch(const DbException& ex)
    {
        cerr << argv[0] << ": database error: " << ex.what() << endl;
        status = EXIT_FAILURE;
    }
    catch(...)
    {
        try
        {
            if(txn)
            {
                txn->abort();
            }
            dbEnv.close(0);
        }
        catch(const DbException& ex)
        {
            cerr << argv[0] << ": database error: " << ex.what() << endl;
        }
        throw;
    }

    try
    {
        if(txn)
        {
            txn->abort();
        }
        dbEnv.close(0);
    }
    catch(const DbException& ex)
    {
        cerr << argv[0] << ": database error: " << ex.what() << endl;
        status = EXIT_FAILURE;
    }

    return status;
}

int
main(int argc, char* argv[])
{
    Ice::CommunicatorPtr communicator;
    int status = EXIT_SUCCESS;
    try
    {
        communicator = Ice::initialize(argc, argv);
        status = run(argc, argv, communicator);
    }
    catch(const FreezeScript::FailureException& ex)
    {
        string reason = ex.reason();
        cerr << argv[0] << ": " << reason;
        if(reason[reason.size() - 1] != '\n')
        {
            cerr << endl;
        }
        return EXIT_FAILURE;
    }
    catch(const IceUtil::Exception& ex)
    {
        cerr << argv[0] << ": " << ex << endl;
        return EXIT_FAILURE;
    }

    if(communicator)
    {
        communicator->destroy();
    }

    return status;
}

//
// SliceVisitor
//
FreezeScript::SliceVisitor::SliceVisitor(IceUtil::XMLOutput& out, const Slice::TypePtr& keyType,
                                         const Slice::TypePtr& valueType, const string& selectExpr) :
    _out(out)
{
    out << se("database") << attr("key", typeToString(keyType)) << attr("value", typeToString(valueType));
    out << se("record");
    if(!selectExpr.empty())
    {
        out << se("if") << attr("test", selectExpr);
        out << se("echo") << attr("message", "Key: ") << attr("value", "key") << ee;
        out << se("echo") << attr("message", "Value: ") << attr("value", "value") << ee;
        out << ee;
    }
    else
    {
        out << se("echo") << attr("message", "Key: ") << attr("value", "key") << ee;
        out << se("echo") << attr("message", "Value: ") << attr("value", "value") << ee;
    }
    out << ee;
    out << ee;
}

bool
FreezeScript::SliceVisitor::visitClassDefStart(const Slice::ClassDefPtr& v)
{
    if(v->isInterface() || v->isLocal())
    {
        return false;
    }

    string scoped = v->scoped();
    if(ignoreType(scoped))
    {
        return false;
    }

    _out.newline();
    _out.newline();
    _out << "<!-- class " << scoped << " -->";
    _out << se("dump") << attr("type", scoped) << ee;

    return false;
}

bool
FreezeScript::SliceVisitor::visitStructStart(const Slice::StructPtr& v)
{
    if(v->isLocal())
    {
        return false;
    }

    string scoped = v->scoped();
    if(ignoreType(scoped))
    {
        return false;
    }

    _out.newline();
    _out.newline();
    _out << "<!-- struct " << scoped << " -->";
    _out << se("dump") << attr("type", scoped) << ee;

    return false;
}

void
FreezeScript::SliceVisitor::visitSequence(const Slice::SequencePtr& v)
{
    if(v->isLocal())
    {
        return;
    }

    string scoped = v->scoped();
    if(ignoreType(scoped))
    {
        return;
    }

    _out.newline();
    _out.newline();
    _out << "<!-- sequence " << scoped << " -->";
    _out << se("dump") << attr("type", scoped) << ee;
}

void
FreezeScript::SliceVisitor::visitDictionary(const Slice::DictionaryPtr& v)
{
    if(v->isLocal())
    {
        return;
    }

    string scoped = v->scoped();
    if(ignoreType(scoped))
    {
        return;
    }

    _out.newline();
    _out.newline();
    _out << "<!-- dictionary " << scoped << " -->";
    _out << se("dump") << attr("type", scoped) << ee;
}

void
FreezeScript::SliceVisitor::visitEnum(const Slice::EnumPtr& v)
{
    if(v->isLocal())
    {
        return;
    }

    string scoped = v->scoped();
    if(ignoreType(scoped))
    {
        return;
    }

    _out.newline();
    _out.newline();
    _out << "<!-- enum " << scoped << " -->";
    _out << se("dump") << attr("type", scoped) << ee;
}

//
// DescriptorHandler
//
FreezeScript::DescriptorHandler::DescriptorHandler(const DataFactoryPtr& factory, const Slice::UnitPtr& unit,
                                                   const ErrorReporterPtr& errorReporter,
                                                   const FreezeScript::ObjectFactoryPtr& objectFactory) :
    _factory(factory), _unit(unit), _errorReporter(errorReporter), _objectFactory(objectFactory)
{
}

void
FreezeScript::DescriptorHandler::startElement(const string& name, const IceXML::Attributes& attributes, int line,
                                              int column)
{
    DescriptorPtr d;

    if(name == "dumpdb")
    {
        if(_current)
        {
            _errorReporter->descriptorError("<dumpdb> must be the top-level element", line);
        }

        _descriptor = new DumpDBDescriptor(line, _factory, _errorReporter, attributes, _unit);
        d = _descriptor;
    }
    else if(name == "database")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<database> must be a child of <dumpdb>", line);
        }

        d = new DatabaseDescriptor(_current, line, _factory, _errorReporter, attributes, _unit);
    }
    else if(name == "record")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<record> must be a child of <database>", line);
        }

        d = new RecordDescriptor(_current, line, _factory, _errorReporter, attributes, _unit, _objectFactory);
    }
    else if(name == "dump")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<dump> must be a child of <dumpdb>", line);
        }

        d = new DumpDescriptor(_current, line, _factory, _errorReporter, attributes, _unit);
    }
    else if(name == "set")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<set> cannot be a top-level element", line);
        }

        d = new SetDescriptor(_current, line, _factory, _errorReporter, attributes);
    }
    else if(name == "define")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<define> cannot be a top-level element", line);
        }

        d = new DefineDescriptor(_current, line, _factory, _errorReporter, attributes, _unit);
    }
    else if(name == "add")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<add> cannot be a top-level element", line);
        }

        d = new AddDescriptor(_current, line, _factory, _errorReporter, attributes);
    }
    else if(name == "remove")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<remove> cannot be a top-level element", line);
        }

        d = new RemoveDescriptor(_current, line, _factory, _errorReporter, attributes);
    }
    else if(name == "fail")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<fail> cannot be a top-level element", line);
        }

        d = new FailDescriptor(_current, line, _factory, _errorReporter, attributes);
    }
    else if(name == "echo")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<echo> cannot be a top-level element", line);
        }

        d = new EchoDescriptor(_current, line, _factory, _errorReporter, attributes);
    }
    else if(name == "if")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<if> cannot be a top-level element", line);
        }

        d = new IfDescriptor(_current, line, _factory, _errorReporter, attributes);
    }
    else if(name == "iterate")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<iterate> cannot be a top-level element", line);
        }

        d = new IterateDescriptor(_current, line, _factory, _errorReporter, attributes);
    }
    else
    {
        _errorReporter->descriptorError("unknown descriptor `" + name + "'", line);
    }

    if(_current)
    {
        _current->addChild(d);
    }
    _current = d;
}

void
FreezeScript::DescriptorHandler::endElement(const std::string& name, int, int)
{
    assert(_current);
    _current = _current->parent();
}

void
FreezeScript::DescriptorHandler::characters(const std::string&, int, int)
{
}

void
FreezeScript::DescriptorHandler::error(const std::string& msg, int line, int col)
{
    _errorReporter->descriptorError(msg, line);
}

FreezeScript::DumpDBDescriptorPtr
FreezeScript::DescriptorHandler::descriptor() const
{
    return _descriptor;
}
