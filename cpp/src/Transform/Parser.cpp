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

#include <Transform/Parser.h>
#include <Transform/GrammarUtil.h>
#include <IceUtil/Mutex.h>

using namespace std;

Transform::DataFactoryPtr Transform::parseDataFactory;
Transform::ErrorReporterPtr Transform::parseErrorReporter;
Transform::NodePtr Transform::parseResult;
int Transform::parseLine;

static string _input;
static string::size_type _pos;
static IceUtil::Mutex _parserMutex;

//
// Parser
//
Transform::NodePtr
Transform::Parser::parse(const string& expr, const DataFactoryPtr& factory, const ErrorReporterPtr& errorReporter)
{
    //
    // The bison grammar is not thread-safe.
    //
    IceUtil::Mutex::Lock sync(_parserMutex);

    parseDataFactory = factory;
    parseErrorReporter = errorReporter;
    parseLine = 1;

    parseErrorReporter->setExpression(expr);

    _input = expr;
    _pos = 0;

    int status = transform_parse();
    if(status != 0)
    {
        parseResult = 0;
    }

    parseErrorReporter->clearExpression();
    parseErrorReporter = 0;

    return parseResult;
}

int
Transform::getInput(char* buf, int maxSize)
{
    if(_pos < _input.length())
    {
        buf[0] = _input[_pos];
        _pos++;
        return 1;
    }
    else
    {
        return 0;
    }
}
