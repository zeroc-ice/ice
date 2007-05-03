// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef C_PLUS_PLUS_UTIL_H
#define C_PLUS_PLUS_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

SLICE_API extern FeatureProfile featureProfile;

struct ToIfdef
{
    SLICE_API char operator()(char);
};

SLICE_API std::string normalizePath(const std::string&);
SLICE_API std::string changeInclude(const std::string&, const std::vector<std::string>&);
SLICE_API void printHeader(::IceUtil::Output&);
SLICE_API void printVersionCheck(::IceUtil::Output&);
SLICE_API void printDllExportStuff(::IceUtil::Output&, const std::string&);

SLICE_API std::string typeToString(const TypePtr&, bool, const StringList& = StringList(), bool = true);
SLICE_API std::string returnTypeToString(const TypePtr&, bool, const StringList& = StringList());
SLICE_API std::string inputTypeToString(const TypePtr&, bool, const StringList& = StringList());
SLICE_API std::string outputTypeToString(const TypePtr&, bool, const StringList& = StringList());
SLICE_API std::string operationModeToString(Operation::Mode);

SLICE_API std::string fixKwd(const std::string&);

SLICE_API void writeMarshalUnmarshalCode(::IceUtil::Output&, const TypePtr&, const std::string&, bool,
                                         const std::string& = "", bool = true, const StringList& = StringList(),
                                         bool = false);
                                             
SLICE_API void writeMarshalCode(::IceUtil::Output&, const ParamDeclList&, const TypePtr&, 
                                const StringList&, bool = false);
SLICE_API void writeUnmarshalCode(::IceUtil::Output&, const ParamDeclList&, const TypePtr&,
                                  const StringList&, bool = false);

SLICE_API void writeAllocateCode(::IceUtil::Output&, const ParamDeclList&, const TypePtr&,
                                 const StringList&, bool = false, bool = false);
SLICE_API void writeStreamMarshalUnmarshalCode(::IceUtil::Output&, const TypePtr&, const std::string&, bool,
                                               const std::string& = "", bool = false, const StringList& = StringList());
SLICE_API std::string findMetaData(const StringList&, bool);
SLICE_API bool inWstringModule(const SequencePtr&);

}

#endif
