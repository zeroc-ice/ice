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

#ifndef TRANSFORM_ANALYZER_H
#define TRANSFORM_ANALYZER_H

#include <Slice/Parser.h>
#include <ostream>

namespace Transform
{

class Analyzer
{
public:

    Analyzer(const Slice::UnitPtr&, const Slice::UnitPtr&, bool);
    void analyze(const Slice::TypePtr&, const Slice::TypePtr&, const Slice::TypePtr&, const Slice::TypePtr&,
                 std::ostream&, std::vector<std::string>&, std::vector<std::string>&);

private:

    Slice::UnitPtr _old;
    Slice::UnitPtr _new;
    bool _ignoreTypeChanges;
};

}

#endif
