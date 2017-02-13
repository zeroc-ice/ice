// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_SCRIPT_TRANSFORM_ANALYZER_H
#define FREEZE_SCRIPT_TRANSFORM_ANALYZER_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>
#include <ostream>

namespace FreezeScript
{

class AnalyzeTransformVisitor;

class TransformAnalyzer
{
public:

    TransformAnalyzer(const Slice::UnitPtr&, const Slice::UnitPtr&, bool, std::ostream&, std::vector<std::string>&,
                      std::vector<std::string>&);
    ~TransformAnalyzer();

    void addDatabase(const std::string&, const Slice::TypePtr&, const Slice::TypePtr&, const Slice::TypePtr&,
                     const Slice::TypePtr&);
    void finish();

private:

    Slice::UnitPtr _old;
    Slice::UnitPtr _new;
    IceUtilInternal::XMLOutput _out;
    AnalyzeTransformVisitor* _visitor;
};

}

#endif
