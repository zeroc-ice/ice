// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FreezeScript/Util.h>
#include <FreezeScript/Exception.h>
#include <Slice/Preprocessor.h>

using namespace std;
using namespace Slice;

string
FreezeScript::typeToString(const TypePtr& type)
{
    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    ContainedPtr c = ContainedPtr::dynamicCast(type);
    ProxyPtr p = ProxyPtr::dynamicCast(type);
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
FreezeScript::createEvictorSliceTypes(const Slice::UnitPtr& unit)
{
    string scoped;
    Slice::TypeList l;
    Slice::ContainedList c;

    //
    // Create the Ice module if necessary.
    //
    c = unit->lookupContained("Ice", false);
    Slice::ModulePtr ice;
    if(c.empty())
    {
        ice = unit->createModule("Ice");
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
    scoped = "::Ice::Identity";
    l = unit->lookupTypeNoBuiltin(scoped, false);
    Slice::StructPtr identity;
    if(l.empty())
    {
        identity = ice->createStruct("Identity", false);
        Slice::TypePtr str = unit->builtin(Slice::Builtin::KindString);
        identity->createDataMember("name", str);
        identity->createDataMember("category", str);
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
    c = unit->lookupContained("Freeze", false);
    Slice::ModulePtr freeze;
    if(c.empty())
    {
        freeze = unit->createModule("Freeze");
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
    l = unit->lookupTypeNoBuiltin(scoped, false);
    Slice::StructPtr stats;
    if(l.empty())
    {
        stats = freeze->createStruct("Statistics", false);
        Slice::TypePtr tl = unit->builtin(Slice::Builtin::KindLong);
        stats->createDataMember("creationTime", tl);
        stats->createDataMember("lastSaveTime", tl);
        stats->createDataMember("avgSaveTime", tl);
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
    l = unit->lookupTypeNoBuiltin(scoped, false);
    if(l.empty())
    {
        Slice::StructPtr rec = freeze->createStruct("ObjectRecord", false);
        Slice::TypePtr obj = unit->builtin(Slice::Builtin::KindObject);
        rec->createDataMember("servant", obj);
        rec->createDataMember("stats", stats);
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
FreezeScript::parseSlice(const string& n, const Slice::UnitPtr& u, const vector<string>& files, const string& cppArgs,
                         bool debug)
{
    //
    // Parse the Slice files.
    //
    for(vector<string>::const_iterator p = files.begin(); p != files.end(); ++p)
    {
        Preprocessor icecpp(n, *p, cppArgs);

        FILE* cppHandle = icecpp.preprocess(false);

        if(cppHandle == 0)
        {
            return false;
        }

        int status = u->parse(cppHandle, debug);

        if(!icecpp.close())
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
