// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_SCRIPT_TRANSFORM_ANALYZER_H
#define FREEZE_SCRIPT_TRANSFORM_ANALYZER_H

#include <Slice/Parser.h>
#include <ostream>

namespace FreezeScript
{

class TransformAnalyzer
{
public:

    TransformAnalyzer(const Slice::UnitPtr&, const Slice::UnitPtr&, bool);
    void analyze(const Slice::TypePtr&, const Slice::TypePtr&, const Slice::TypePtr&, const Slice::TypePtr&,
                 std::ostream&, std::vector<std::string>&, std::vector<std::string>&);

private:

    Slice::UnitPtr _old;
    Slice::UnitPtr _new;
    bool _ignoreTypeChanges;
};

}

#endif
