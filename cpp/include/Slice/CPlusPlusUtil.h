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

#ifndef C_PLUS_PLUS_UTIL_H
#define C_PLUS_PLUS_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

struct ToIfdef
{
    SLICE_API char operator()(char);
};

SLICE_API std::string changeInclude(const std::string&, const std::vector<std::string>&);
SLICE_API void printHeader(::IceUtil::Output&);
SLICE_API void printVersionCheck(::IceUtil::Output&);
SLICE_API void printDllExportStuff(::IceUtil::Output&, const std::string&);

SLICE_API std::string typeToString(const TypePtr&);
SLICE_API std::string returnTypeToString(const TypePtr&);
SLICE_API std::string inputTypeToString(const TypePtr&);
SLICE_API std::string outputTypeToString(const TypePtr&);

SLICE_API std::string fixKwd(const std::string&);

SLICE_API void writeMarshalUnmarshalCode(::IceUtil::Output&, const TypePtr&, const std::string&, bool,
					 const std::string& = "", bool = true);
SLICE_API void writeMarshalCode(::IceUtil::Output&, const std::list<std::pair<TypePtr, std::string> >&,
				const TypePtr&);
SLICE_API void writeUnmarshalCode(::IceUtil::Output&, const std::list<std::pair<TypePtr, std::string> >&,
				  const TypePtr&);
SLICE_API void writeAllocateCode(::IceUtil::Output&, const std::list<std::pair<TypePtr, std::string> >&,
				 const TypePtr&);

}

#endif
