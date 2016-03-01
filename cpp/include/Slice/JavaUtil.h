// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef JAVA_UTIL_H
#define JAVA_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

//
// Compute Java serialVersionUID for a Slice class
//
SLICE_API long
computeSerialVersionUUID(const ClassDefPtr&);

//
// Compute Java serialVersionUID for a Slice class
//
SLICE_API long
computeSerialVersionUUID(const ExceptionPtr&);

//
// Compute Java serialVersionUID for a Slice struct
//
SLICE_API long
computeSerialVersionUUID(const StructPtr&);

class SLICE_API JavaOutput : public ::IceUtilInternal::Output
{
public:

    JavaOutput();
    JavaOutput(std::ostream&);
    JavaOutput(const char*);

    //
    // Open a file to hold the source for a Java class. The first
    // argument is the class name (including an optional leading
    // package). Intermediate directories will be created as
    // necessary to open the file in the package. The second
    // argument specifies a directory prefix in which to locate
    // the class.
    //
    // After successfully opening the file, the function invokes
    // printHeader() and then emits a "package" statement if
    // necessary.
    //
    void openClass(const std::string&, const std::string&, const std::string& = std::string());

    virtual void printHeader();
};

class SLICE_API JavaGenerator : private ::IceUtil::noncopyable
{
public:

    virtual ~JavaGenerator();

    //
    // Validate all metadata in the unit with a "java:" prefix.
    //
    static void validateMetaData(const UnitPtr&);

    void close();

protected:

    JavaGenerator(const std::string&);

    //
    // Given the fully-scoped Java class name, create any intermediate
    // package directories and open the class file,
    //
    void open(const std::string&, const std::string&);

    ::IceUtilInternal::Output& output() const;

    //
    // Check a symbol against any of the Java keywords. If a
    // match is found, return the symbol with a leading underscore.
    //
    std::string fixKwd(const std::string&) const;

    //
    // Convert a Slice scoped name into a Java name.
    //
    std::string convertScopedName(const std::string&,
                                  const std::string& = std::string(),
                                  const std::string& = std::string()) const;


    //
    // Returns the package prefix for a give Slice file.
    //
    std::string getPackagePrefix(const ContainedPtr&) const;

    //
    // Returns the Java package of a Contained entity.
    //
    std::string getPackage(const ContainedPtr&) const;

    //
    // Returns the Java name for a Contained entity. If the optional
    // package argument matches the entity's package name, then the
    // package is removed from the result.
    //
    std::string getAbsolute(const ContainedPtr&,
                            const std::string& = std::string(),
                            const std::string& = std::string(),
                            const std::string& = std::string()) const;

    //
    // Return the method call necessary to obtain the static type ID for an object type.
    //
    std::string getStaticId(const TypePtr&, const std::string&) const;

    //
    // Determines whether an operation should use the optional mapping.
    //
    bool useOptionalMapping(const OperationPtr&);

    //
    // Returns the optional type corresponding to the given Slice type.
    //
    std::string getOptionalFormat(const TypePtr&);

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
    std::string typeToString(const TypePtr&, TypeMode, const std::string& = std::string(),
                             const StringList& = StringList(), bool = true, bool = false) const;

    //
    // Get the Java object name for a type. For primitive types, this returns the
    // Java class type (e.g., Integer). For all other types, this function delegates
    // to typeToString.
    //
    std::string typeToObjectString(const TypePtr&, TypeMode, const std::string& = std::string(),
                                   const StringList& = StringList(), bool = true) const;

    //
    // Generate code to marshal or unmarshal a type.
    //
    enum OptionalMode
    {
        OptionalNone,
        OptionalInParam,
        OptionalOutParam,
        OptionalReturnParam,
        OptionalMember
    };

    void writeMarshalUnmarshalCode(::IceUtilInternal::Output&, const std::string&, const TypePtr&, OptionalMode,
                                   bool, int, const std::string&, bool, int&, bool = false,
                                   const StringList& = StringList(), const std::string& patchParams = "");

    //
    // Generate code to marshal or unmarshal a dictionary type.
    //
    void writeDictionaryMarshalUnmarshalCode(::IceUtilInternal::Output&, const std::string&, const DictionaryPtr&,
                                           const std::string&, bool, int&, bool,
                                           const StringList& = StringList());

    //
    // Generate code to marshal or unmarshal a sequence type.
    //
    void writeSequenceMarshalUnmarshalCode(::IceUtilInternal::Output&, const std::string&, const SequencePtr&,
                                           const std::string&, bool, int&, bool, const StringList& = StringList());

    //
    // Generate code to marshal or unmarshal a type using the public stream API.
    //
    void writeStreamMarshalUnmarshalCode(::IceUtilInternal::Output&, const std::string&, const TypePtr&, bool, int,
                                          const std::string&, bool, int&, bool = false,
                                         const StringList& = StringList(), const std::string& patchParams = "");

    //
    // Generate code to marshal or unmarshal a dictionary type using the public stream API.
    //
    void writeStreamDictionaryMarshalUnmarshalCode(::IceUtilInternal::Output&, const std::string&, const DictionaryPtr&,
                                                   const std::string&, bool, int&, bool,
                                                   const StringList& = StringList());

    //
    // Generate code to marshal or unmarshal a sequence type using the public stream API.
    //
    void writeStreamSequenceMarshalUnmarshalCode(::IceUtilInternal::Output&, const std::string&, const SequencePtr&,
                                                 const std::string&, bool, int&, bool,
                                                 const StringList& = StringList());

    //
    // Search metadata for an entry with the given prefix and return the entire string.
    //
    static bool findMetaData(const std::string&, const StringList&, std::string&);

    //
    // Get custom type metadata. If metadata is found, the abstract and
    // concrete types are extracted and the function returns true. If an
    // abstract type is not specified, it is set to an empty string.
    //
    static bool getTypeMetaData(const StringList&, std::string&, std::string&);

    //
    // Determine whether a custom type is defined. The function checks the
    // metadata of the type's original definition, as well as any optional
    // metadata that typically represents a data member or parameter.
    //
    static bool hasTypeMetaData(const TypePtr&, const StringList& = StringList());

    //
    // Obtain the concrete and abstract types for a dictionary or sequence type.
    // The functions return true if a custom type was defined and false to indicate
    // the default mapping was used.
    //
    bool getDictionaryTypes(const DictionaryPtr&, const std::string&, const StringList&,
                            std::string&, std::string&) const;
    bool getSequenceTypes(const SequencePtr&, const std::string&, const StringList&, std::string&, std::string&) const;

    bool sequenceHasHolder(const SequencePtr&) const;

    virtual JavaOutput* createOutput();

    static const std::string _getSetMetaData;

private:

    class MetaDataVisitor : public ParserVisitor
    {
    public:

        virtual bool visitUnitStart(const UnitPtr&);
        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitClassDecl(const ClassDeclPtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitOperation(const OperationPtr&);
        virtual void visitDataMember(const DataMemberPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual void visitEnum(const EnumPtr&);
        virtual void visitConst(const ConstPtr&);

    private:

        StringList getMetaData(const ContainedPtr&);
        void validateType(const SyntaxTreeBasePtr&, const StringList&, const std::string&, const std::string&);
        void validateGetSet(const SyntaxTreeBasePtr&, const StringList&, const std::string&, const std::string&);

        StringSet _history;
    };

    friend class JavaGenerator::MetaDataVisitor;

    std::string _dir;
    ::IceUtilInternal::Output* _out;
    mutable std::map<std::string, std::string> _filePackagePrefix;
};

}

#endif
