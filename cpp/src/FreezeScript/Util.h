// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_SCRIPT_UTIL_H
#define FREEZE_SCRIPT_UTIL_H

#include <Slice/Parser.h>
#include <Ice/CommunicatorF.h>
#include <Freeze/CatalogData.h>

namespace FreezeScript
{

template<class T>
class Destroyer
{
public:

    Destroyer() {}
    Destroyer(T p) : _p(p) {}
    ~Destroyer()
    {
        if(_p)
        {
            _p->destroy();
        }
    }

    void set(T p)
    {
        if(_p)
        {
            _p->destroy();
        }
        _p = p;
    }

    void release()
    {
        _p = 0;
    }

private:

    T _p;
};

std::string typeName(const Slice::TypePtr&);
std::string typeToString(const Slice::TypePtr&);
bool ignoreType(const std::string&);
void createEvictorSliceTypes(const Slice::UnitPtr&);
bool parseSlice(const std::string&, const Slice::UnitPtr&, const std::vector<std::string>&, const std::string&, bool);

typedef std::map<std::string, Freeze::CatalogData> CatalogDataMap;

CatalogDataMap readCatalog(const Ice::CommunicatorPtr&, const std::string&);

} // End of namespace FreezeScript

#endif
