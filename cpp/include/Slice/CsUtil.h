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

#ifndef CS_UTIL_H
#define CS_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

class SLICE_API CsGenerator : public ::IceUtil::noncopyable
{
public:

    static std::string fixId(const std::string&);
    static std::string typeToString(const TypePtr&);
    static bool isValueType(const TypePtr&);
    //
    // Generate code to marshal or unmarshal a type
    //
    void writeMarshalUnmarshalCode(::IceUtil::Output&, const TypePtr&, const std::string&, bool, bool);
    void writeSequenceMarshalUnmarshalCode(::IceUtil::Output&, const SequencePtr&, const std::string&, bool, bool);

#if 0
protected:

    //
    // Check a symbol against any of the Java keywords. If a
    // match is found, return the symbol with a leading underscore.
    //

    //
    // Convert a scoped name into a Java class name. If an optional
    // scope is provided, the scope will be removed from the result.
    //
    std::string getAbsolute(const std::string&,
                            const std::string& = std::string(),
                            const std::string& = std::string(),
                            const std::string& = std::string()) const;

    //
    // Get the Java name for a type. If an optional scope is provided,
    // the scope will be removed from the result if possible.
    //
    enum TypeMode
    {
        TypeModeIn,
        TypeModeOut,
        TypeModeMember,
        TypeModeReturn
    };
    std::string typeToString(const TypePtr&, TypeMode mode,
                             const std::string& = std::string(),
                             const std::list<std::string>& = std::list<std::string>()) const;

    //
    // Generate code to marshal or unmarshal a type
    //
    void writeMarshalUnmarshalCode(::IceUtil::Output&, const std::string&, const TypePtr&, const std::string&,
                                   bool, int&, bool = false, const std::list<std::string>& = std::list<std::string>(),
				   const std::string& patchParams = "");

    //
    // Generate code to marshal or unmarshal a sequence type
    //
    void writeSequenceMarshalUnmarshalCode(::IceUtil::Output&, const std::string&, const SequencePtr&,
                                           const std::string&, bool, int&, bool,
                                           const std::list<std::string>& = std::list<std::string>());

protected:

    static std::string findMetaData(const std::list<std::string>&);
    IceUtil::Output& _out;

private:
#endif

};

}

#endif
