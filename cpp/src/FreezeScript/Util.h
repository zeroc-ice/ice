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

#ifndef FREEZE_SCRIPT_UTIL_H
#define FREEZE_SCRIPT_UTIL_H

#include <Slice/Parser.h>

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

} // End of namespace FreezeScript

#endif
