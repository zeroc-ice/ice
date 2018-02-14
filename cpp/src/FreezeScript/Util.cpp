// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FreezeScript/Util.h>
#include <FreezeScript/Exception.h>
#include <Freeze/Catalog.h>
#include <Freeze/Connection.h>
#include <Freeze/Initialize.h>
#include <Slice/Preprocessor.h>
#include <db_cxx.h>
#include <sys/stat.h>

using namespace std;
using namespace Slice;

string
FreezeScript::typeToString(const TypePtr& type)
{
    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    ContainedPtr c = ContainedPtr::dynamicCast(type);
    if(b)
    {
        return b->kindAsString();
    }
    else if(c)
    {
        return c->scoped();
    }
    else
    {
        ProxyPtr p = ProxyPtr::dynamicCast(type);
        assert(p);
        return p->_class()->scoped() + "*";
    }
}

bool
FreezeScript::ignoreType(const string& type)
{
    //
    // Suppress descriptors for the following pre-defined types. Must be kept in ascending alphabetical order.
    //
    static const string ignoreTypeList[] =
    {
        "::Freeze::ObjectRecord",
        "::Freeze::Statistics",
        "::Ice::Identity"
    };

    return binary_search(&ignoreTypeList[0], &ignoreTypeList[sizeof(ignoreTypeList) / sizeof(*ignoreTypeList)], type);
}

void
FreezeScript::createEvictorSliceTypes(const Slice::UnitPtr& u)
{
    Slice::TypeList l;
    Slice::ContainedList c;

    //
    // Create the Ice module if necessary.
    //
    c = u->lookupContained("Ice", false);
    Slice::ModulePtr ice;
    if(c.empty())
    {
        ice = u->createModule("Ice");
    }
    else
    {
        ice = Slice::ModulePtr::dynamicCast(c.front());
        if(!ice)
        {
            throw FailureException(__FILE__, __LINE__, "the symbol `::Ice' is defined in Slice but is not a module");
        }
    }

    //
    // Create the Slice definition for Ice::Identity if it doesn't exist.
    //
    string scoped = "::Ice::Identity";
    l = u->lookupTypeNoBuiltin(scoped, false);
    Slice::StructPtr identity;
    if(l.empty())
    {
        identity = ice->createStruct("Identity", false);
        Slice::TypePtr str = u->builtin(Slice::Builtin::KindString);
        identity->createDataMember("name", str, false, 0, 0, "", "");
        identity->createDataMember("category", str, false, 0, 0, "", "");
    }
    else
    {
        identity = Slice::StructPtr::dynamicCast(l.front());
        if(!identity)
        {
            throw FailureException(__FILE__, __LINE__,
                                   "the symbol `::Ice::Identity' is defined in Slice but is not a struct");
        }
    }

    //
    // Create the Freeze module if necessary.
    //
    c = u->lookupContained("Freeze", false);
    Slice::ModulePtr freeze;
    if(c.empty())
    {
        freeze = u->createModule("Freeze");
    }
    else
    {
        freeze = Slice::ModulePtr::dynamicCast(c.front());
        if(!freeze)
        {
            throw FailureException(__FILE__, __LINE__,
                                   "the symbol `::Freeze' is defined in Slice but is not a module");
        }
    }

    //
    // Create the Slice definition for Freeze::Statistics if it doesn't exist.
    //
    scoped = "::Freeze::Statistics";
    l = u->lookupTypeNoBuiltin(scoped, false);
    Slice::StructPtr stats;
    if(l.empty())
    {
        stats = freeze->createStruct("Statistics", false);
        Slice::TypePtr tl = u->builtin(Slice::Builtin::KindLong);
        stats->createDataMember("creationTime", tl, false, 0, 0, "", "");
        stats->createDataMember("lastSaveTime", tl, false, 0, 0, "", "");
        stats->createDataMember("avgSaveTime", tl, false, 0, 0, "", "");
    }
    else
    {
        stats = Slice::StructPtr::dynamicCast(l.front());
        if(!stats)
        {
            throw FailureException(__FILE__, __LINE__, "the symbol `::Freeze::Statistics' is defined in "
                                   "Slice but is not a struct");
        }
    }

    //
    // Create the Slice definition for Freeze::ObjectRecord if it doesn't exist.
    //
    scoped = "::Freeze::ObjectRecord";
    l = u->lookupTypeNoBuiltin(scoped, false);
    if(l.empty())
    {
        Slice::StructPtr rec = freeze->createStruct("ObjectRecord", false);
        Slice::TypePtr obj = u->builtin(Slice::Builtin::KindObject);
        rec->createDataMember("servant", obj, false, 0, 0, "", "");
        rec->createDataMember("stats", stats, false, 0, 0, "", "");
    }
    else
    {
        if(!Slice::StructPtr::dynamicCast(l.front()))
        {
            throw FailureException(__FILE__, __LINE__, "the symbol `::Freeze::ObjectRecord' is defined in "
                                   "Slice but is not a struct");
        }
    }
}

bool
FreezeScript::parseSlice(const string& n, const Slice::UnitPtr& u, const vector<string>& files,
                         const vector<string>& cppArgs, bool debug, const std::string& extraArgs)
{
    //
    // Parse the Slice files.
    //
    for(vector<string>::const_iterator p = files.begin(); p != files.end(); ++p)
    {
        PreprocessorPtr icecpp = Preprocessor::create(n, *p, cppArgs);

        FILE* cppHandle = icecpp->preprocess(false, extraArgs);

        if(cppHandle == 0)
        {
            return false;
        }

        int status = u->parse(*p, cppHandle, debug);

        if(!icecpp->close())
        {
            return false;            
        }

        if(status != EXIT_SUCCESS)
        {
            return false;
        }
    }

    u->mergeModules();

    return true;
}

FreezeScript::CatalogDataMap
FreezeScript::readCatalog(const Ice::CommunicatorPtr& communicator, const string& dbEnvName)
{
    CatalogDataMap result;

    DbEnv dbEnv(0);
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
            u_int32_t flags = DB_THREAD | DB_CREATE | DB_INIT_TXN | DB_INIT_MPOOL;
            dbEnv.open(dbEnvName.c_str(), flags, 0);
        }

        Freeze::ConnectionPtr connection = Freeze::createConnection(communicator, dbEnvName, dbEnv);
        Freeze::Catalog catalog(connection, Freeze::catalogName());
        for(Freeze::Catalog::const_iterator p = catalog.begin(); p != catalog.end(); ++p)
        {
            result.insert(make_pair(p->first, p->second));
        }
    }
    catch(const DbException& ex)
    {
        dbEnv.close(0);
        throw FailureException(__FILE__, __LINE__, string("database error: ") + ex.what());
    }
    catch(const IceUtil::FileLockException&)
    {
        dbEnv.close(0);
        throw FailureException(__FILE__, __LINE__, "environment `" + dbEnvName + "' is locked");
    }
    catch(...)
    {
        dbEnv.close(0);
        throw;
    }

    dbEnv.close(0);

    return result;
}

string
FreezeScript::CompactIdResolverI::resolve(Ice::Int id) const
{
    string type;
    map<Ice::Int, string>::const_iterator p = _ids.find(id);
    if(p != _ids.end())
    {
        type = p->second;
    }
    return type;
}

void
FreezeScript::CompactIdResolverI::add(Ice::Int id, const string& type)
{
#ifndef NDEBUG
    map<Ice::Int, string>::const_iterator p = _ids.find(id);
#else
    _ids.find(id);
#endif
    assert(p == _ids.end());

    _ids[id] = type;
}

void
FreezeScript::collectCompactIds(const UnitPtr& unit, const FreezeScript::CompactIdResolverIPtr& r)
{
    class Visitor : public ParserVisitor
    {
    public:

        Visitor(const FreezeScript::CompactIdResolverIPtr& resolver) : _r(resolver)
        {
        }

        virtual bool visitClassDefStart(const ClassDefPtr& p)
        {
            if(p->compactId() != -1)
            {
                _r->add(p->compactId(), p->scoped());
            }
            return true;
        }

        FreezeScript::CompactIdResolverIPtr _r;
    };

    Visitor v(r);
    unit->visit(&v, false);
}
