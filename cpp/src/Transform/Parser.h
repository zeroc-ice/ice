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

#ifndef TRANSFORM_PARSER_H
#define TRANSFORM_PARSER_H

#include <Transform/Node.h>
#include <string>

namespace Transform
{

class Parser
{
public:

    static NodePtr parse(const std::string&, const DataFactoryPtr&, const ErrorReporterPtr&);
};

} // End of namespace Transform

#endif
