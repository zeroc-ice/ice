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
        "::Freeze::EvictorStorageKey",
        "::Freeze::ObjectRecord",
        "::Freeze::Statistics",
        "::Ice::FacetPath",
        "::Ice::Identity",
        "::_FacetMap"
    };

    return binary_search(&ignoreTypeList[0], &ignoreTypeList[sizeof(ignoreTypeList) / sizeof(*ignoreTypeList)], type);
}

void
FreezeScript::createCoreSliceTypes(const Slice::UnitPtr& unit)
{
    string scoped;
    Slice::TypeList l;
    Slice::ContainedList c;

    //
    // Create the Slice definition for _FacetMap if it doesn't exist. This type is
    // necessary for marshaling an object's facet map.
    //
    l = unit->lookupTypeNoBuiltin("::_FacetMap", false);
    if(l.empty())
    {
        Slice::TypePtr str = unit->builtin(Slice::Builtin::KindString);
        Slice::TypePtr obj = unit->builtin(Slice::Builtin::KindObject);
        unit->createDictionary("_FacetMap", str, obj, false);
    }
    else
    {
        assert(Slice::DictionaryPtr::dynamicCast(l.front()));
    }
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
            throw Exception(__FILE__, __LINE__, "the symbol `::Ice' is defined in Slice but is not a module");
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
        identity->createDataMember("category", str);
        identity->createDataMember("name", str);
    }
    else
    {
        identity = Slice::StructPtr::dynamicCast(l.front());
        if(!identity)
        {
            throw Exception(__FILE__, __LINE__, "the symbol `::Ice::Identity' is defined in Slice but is not a struct");
        }
    }

    //
    // Create the Slice definition for Ice::FacetPath if it doesn't exist.
    //
    scoped = "::Ice::FacetPath";
    l = unit->lookupTypeNoBuiltin(scoped, false);
    Slice::SequencePtr facetPath;
    if(l.empty())
    {
        Slice::TypePtr str = unit->builtin(Slice::Builtin::KindString);
        facetPath = ice->createSequence("FacetPath", str, false);
    }
    else
    {
        facetPath = Slice::SequencePtr::dynamicCast(l.front());
        if(!facetPath)
        {
            throw Exception(__FILE__, __LINE__,
                            "the symbol `::Ice::FacetPath' is defined in Slice but is not a sequence");
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
            throw Exception(__FILE__, __LINE__, "the symbol `::Freeze' is defined in Slice but is not a module");
        }
    }

    //
    // Create the Slice definition for Freeze::EvictorStorageKey if it doesn't exist.
    //
    scoped = "::Freeze::EvictorStorageKey";
    l = unit->lookupTypeNoBuiltin(scoped, false);
    if(l.empty())
    {
        Slice::StructPtr esk = freeze->createStruct("EvictorStorageKey", false);
        esk->createDataMember("identity", identity);
        esk->createDataMember("facet", facetPath);
    }
    else
    {
        if(!Slice::StructPtr::dynamicCast(l.front()))
        {
            throw Exception(__FILE__, __LINE__, "the symbol `::Freeze::EvictorStorageKey' is defined in "
                            "Slice but is not a struct");
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
            throw Exception(__FILE__, __LINE__, "the symbol `::Freeze::Statistics' is defined in "
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
            throw Exception(__FILE__, __LINE__, "the symbol `::Freeze::ObjectRecord' is defined in "
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
