// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef TRANSFORM_TRANSFORM_UTIL_H
#define TRANSFORM_TRANSFORM_UTIL_H

#include <Slice/Parser.h>

namespace Transform
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
            _p->destroy();
    }

    void set(T p)
    {
        if(_p)
            _p->destroy();
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

} // End of namespace Transform

#endif
