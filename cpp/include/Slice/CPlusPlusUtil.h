// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
SLICE_API void printDefInt64Macro(::IceUtil::Output&);
SLICE_API void printUndefInt64Macro(::IceUtil::Output&);
SLICE_API void printDllExportStuff(::IceUtil::Output&, const std::string&);

SLICE_API std::string typeToString(const TypePtr&);
SLICE_API std::string returnTypeToString(const TypePtr&);
SLICE_API std::string inputTypeToString(const TypePtr&);
SLICE_API std::string outputTypeToString(const TypePtr&);

SLICE_API void writeMarshalUnmarshalCode(::IceUtil::Output&, const TypePtr&, const std::string&, bool,
					 const std::string& = "", bool = true);
SLICE_API void writeMarshalCode(::IceUtil::Output&, const std::list<std::pair<TypePtr, std::string> >&,
				const TypePtr&);
SLICE_API void writeUnmarshalCode(::IceUtil::Output&, const std::list<std::pair<TypePtr, std::string> >&,
				  const TypePtr&);
SLICE_API void writeAllocateCode(::IceUtil::Output&, const std::list<std::pair<TypePtr, std::string> >&,
				 const TypePtr&);

SLICE_API void writeGenericMarshalUnmarshalCode(::IceUtil::Output&, const TypePtr&, const std::string&, bool,
						const std::string& = "", const std::string& = "", bool = true);

SLICE_API void writeGenericMarshalCode(::IceUtil::Output&, const std::list<std::pair<TypePtr, std::string> >&,
				       const TypePtr&);
SLICE_API void writeGenericUnmarshalCode(::IceUtil::Output&, const std::list<std::pair<TypePtr, std::string> >&,
					 const TypePtr&);

}

#endif
