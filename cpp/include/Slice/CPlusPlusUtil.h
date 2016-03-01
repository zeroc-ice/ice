// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
SLICE_API extern std::string paramPrefix;

struct ToIfdef
{
    SLICE_API char operator()(char);
};

SLICE_API void printHeader(::IceUtilInternal::Output&);
SLICE_API void printVersionCheck(::IceUtilInternal::Output&);
SLICE_API void printDllExportStuff(::IceUtilInternal::Output&, const std::string&);

const int TypeContextInParam = 1;
const int TypeContextAMIEnd = 2;
const int TypeContextAMIPrivateEnd = 4;
const int TypeContextAMICallPrivateEnd = 8;

const int TypeContextUseWstring = 16;

SLICE_API std::string typeToString(const TypePtr&, const StringList& = StringList(), int = 0);
SLICE_API std::string typeToString(const TypePtr&, bool, const StringList& = StringList(), int = 0);
SLICE_API std::string returnTypeToString(const TypePtr&, bool, const StringList& = StringList(), int = 0);
SLICE_API std::string inputTypeToString(const TypePtr&, bool, const StringList& = StringList(), int = 0);
SLICE_API std::string outputTypeToString(const TypePtr&, bool, const StringList& = StringList(), int = 0);
SLICE_API std::string operationModeToString(Operation::Mode);
SLICE_API std::string opFormatTypeToString(const OperationPtr&);

SLICE_API std::string fixKwd(const std::string&);

SLICE_API void writeMarshalUnmarshalCode(::IceUtilInternal::Output&, const TypePtr&, bool, int, const std::string&,
                                         bool, const StringList& = StringList(), int = 0, const std::string& = "",
                                         bool = true);

SLICE_API void writeMarshalCode(::IceUtilInternal::Output&, const ParamDeclList&, const OperationPtr&, bool, int = 0);
SLICE_API void writeUnmarshalCode(::IceUtilInternal::Output&, const ParamDeclList&, const OperationPtr&, bool, int = 0);
SLICE_API void writeAllocateCode(::IceUtilInternal::Output&, const ParamDeclList&, const OperationPtr&, bool, int = 0);

SLICE_API std::string getEndArg(const TypePtr&, const StringList&, const std::string&);
SLICE_API void writeEndCode(::IceUtilInternal::Output&, const ParamDeclList&, const OperationPtr&, bool = false);

SLICE_API std::string findMetaData(const StringList&, int = 0);
SLICE_API bool inWstringModule(const SequencePtr&);

SLICE_API std::string getDataMemberRef(const DataMemberPtr&);
}

#endif
