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

#ifndef JAVA_UTIL_H
#define JAVA_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

class SLICE_API JavaGenerator : public ::IceUtil::noncopyable
{
public:

    virtual ~JavaGenerator();

protected:

    JavaGenerator(const std::string&, const std::string&);

    //
    // Given the fully-scoped Java class name, create any intermediate
    // package directories and open the class file
    //
    bool open(const std::string&);
    void close();

    ::IceUtil::Output& output() const;

    //
    // Check a symbol against any of the Java keywords. If a
    // match is found, return the symbol with a leading underscore.
    //
    std::string fixKwd(const std::string&) const;

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

private:

    void printHeader();

    std::string _dir;
    std::string _package;

    ::IceUtil::Output* _out;
};

}

#endif
