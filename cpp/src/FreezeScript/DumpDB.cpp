// **********************************************************************
//
// Copyright (c) 2004
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <FreezeScript/DumpDescriptors.h>
#include <FreezeScript/Util.h>
#include <FreezeScript/Exception.h>
#include <IceUtil/OutputUtil.h>
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

    DescriptorHandler(const DataFactoryPtr&, const Slice::UnitPtr&, const ErrorReporterPtr&);

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
};

}

static void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options] [dbenv db]\n";
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
        ;
    // Note: --case-sensitive is intentionally not shown here!
}

static int
run(int argc, char** argv, const Ice::CommunicatorPtr& communicator)
{
    string cppArgs;
    bool debug = false;
    bool ice = true; // Needs to be true in order to create default definitions.
    bool caseSensitive = false;
    string outputFile;
    string inputFile;
    vector<string> slice;
    bool evictor = false;
    string keyTypeName;
    string valueTypeName;
    string selectExpr;
    string dbEnvName, dbName;

    int idx = 1;
    while(idx < argc)
    {
        if(strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0)
        {
            usage(argv[0]);
            return EXIT_SUCCESS;
        }
        else if(strcmp(argv[idx], "-v") == 0 || strcmp(argv[idx], "--version") == 0)
        {
            cout << ICE_STRING_VERSION << endl;
            return EXIT_SUCCESS;
        }
        else if(strncmp(argv[idx], "-D", 2) == 0 || strncmp(argv[idx], "-U", 2) == 0)
        {
            cppArgs += ' ';
            cppArgs += argv[idx];

            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strncmp(argv[idx], "-I", 2) == 0)
        {
            cppArgs += ' ';
            cppArgs += argv[idx];

            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "-d") == 0 || strcmp(argv[idx], "--debug") == 0)
        {
            debug = true;
            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "--ice") == 0)
        {
            ice = true;
            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "--case-sensitive") == 0)
        {
            caseSensitive = true;
            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "-o") == 0)
        {
            if(idx + 1 >= argc || argv[idx + 1][0] == '-')
            {
                cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            outputFile = argv[idx + 1];

            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else if(strcmp(argv[idx], "-f") == 0)
        {
            if(idx + 1 >= argc || argv[idx + 1][0] == '-')
            {
                cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            inputFile = argv[idx + 1];

            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else if(strcmp(argv[idx], "--load") == 0)
        {
            if(idx + 1 >= argc || argv[idx + 1][0] == '-')
            {
                cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            slice.push_back(argv[idx + 1]);

            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else if(strcmp(argv[idx], "-e") == 0)
        {
            evictor = true;
            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "--key") == 0)
        {
            if(idx + 1 >= argc || argv[idx + 1][0] == '-')
            {
                cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            keyTypeName = argv[idx + 1];

            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else if(strcmp(argv[idx], "--value") == 0)
        {
            if(idx + 1 >= argc || argv[idx + 1][0] == '-')
            {
                cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            valueTypeName = argv[idx + 1];

            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else if(strcmp(argv[idx], "--select") == 0)
        {
            if(idx + 1 >= argc || argv[idx + 1][0] == '-')
            {
                cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            selectExpr = argv[idx + 1];

            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else if(argv[idx][0] == '-')
        {
            cerr << argv[0] << ": unknown option `" << argv[idx] << "'" << endl;
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        else
        {
            ++idx;
        }
    }

    if(outputFile.empty() && argc < 3)
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if(argc > 1)
    {
        dbEnvName = argv[1];
    }
    if(argc > 2)
    {
        dbName = argv[2];
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

    FreezeScript::createCoreSliceTypes(unit);
    FreezeScript::createEvictorSliceTypes(unit);

    //
    // If no input file was provided, then we need to generate default descriptors.
    //
    string descriptors;
    if(inputFile.empty())
    {
        if(evictor)
        {
            keyTypeName = "::Freeze::EvictorStorageKey";
            valueTypeName = "::Freeze::ObjectRecord";
        }
        else if(keyTypeName.empty() || valueTypeName.empty())
        {
            usage(argv[0]);
            return EXIT_FAILURE;
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
        unit->visit(&visitor);

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

    DbEnv dbEnv(0);
    DbTxn* txn = 0;
    Db* db = 0;
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
        // Open the database environment.
        //
        {
            u_int32_t flags = DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN | DB_RECOVER | DB_CREATE;
            dbEnv.open(dbEnvName.c_str(), flags, FREEZE_SCRIPT_DB_MODE);
        }

        //
        // Open the database in a transaction.
        //
        db = new Db(&dbEnv, 0);
        dbEnv.txn_begin(0, &txn, 0);
        db->open(txn, dbName.c_str(), 0, DB_BTREE, DB_RDONLY, FREEZE_SCRIPT_DB_MODE);

        istringstream istr(descriptors);

        FreezeScript::ErrorReporterPtr errorReporter = new FreezeScript::ErrorReporter(cerr, false);

        try
        {
            FreezeScript::DataFactoryPtr factory = new FreezeScript::DataFactory(communicator, unit, errorReporter);
            FreezeScript::DescriptorHandler dh(factory, unit, errorReporter);
            IceXML::Parser::parse(istr, dh);

            FreezeScript::DumpDBDescriptorPtr descriptor = dh.descriptor();
            descriptor->validate();
            descriptor->dump(communicator, db, txn);
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
        if(txn)
        {
            txn->abort();
        }
        if(db)
        {
            db->close(0);
            delete db;
        }
        dbEnv.close(0);
        throw;
    }

    if(txn)
    {
        txn->abort();
    }
    if(db)
    {
        db->close(0);
        delete db;
    }
    dbEnv.close(0);

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
    catch(const FreezeScript::Exception& ex)
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

    _out.nl();
    _out.nl();
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

    _out.nl();
    _out.nl();
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

    _out.nl();
    _out.nl();
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

    _out.nl();
    _out.nl();
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

    _out.nl();
    _out.nl();
    _out << "<!-- enum " << scoped << " -->";
    _out << se("dump") << attr("type", scoped) << ee;
}

//
// DescriptorHandler
//
FreezeScript::DescriptorHandler::DescriptorHandler(const DataFactoryPtr& factory, const Slice::UnitPtr& unit,
                                                   const ErrorReporterPtr& errorReporter) :
    _factory(factory), _unit(unit), _errorReporter(errorReporter)
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

        d = new RecordDescriptor(_current, line, _factory, _errorReporter, attributes, _unit);
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
