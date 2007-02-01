// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

class SLICE_API JavaOutput : public ::IceUtil::Output
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
    bool openClass(const std::string&, const std::string& = std::string());

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

protected:

    JavaGenerator(const std::string&);

    JavaGenerator(const std::string&, Slice::FeatureProfile profile);

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
    // Convert a Slice scoped name into a Java name.
    //
    std::string convertScopedName(const std::string&,
                                  const std::string& = std::string(),
                                  const std::string& = std::string()) const;

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
                             const StringList& = StringList(), bool = true) const;

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
    void writeMarshalUnmarshalCode(::IceUtil::Output&, const std::string&, const TypePtr&, const std::string&,
                                   bool, int&, bool = false, const StringList& = StringList(),
                                   const std::string& patchParams = "");

    //
    // Generate code to marshal or unmarshal a dictionary type.
    //
    void writeDictionaryMarshalUnmarshalCode(::IceUtil::Output&, const std::string&, const DictionaryPtr&,
                                           const std::string&, bool, int&, bool,
                                           const StringList& = StringList());

    //
    // Generate code to marshal or unmarshal a sequence type.
    //
    void writeSequenceMarshalUnmarshalCode(::IceUtil::Output&, const std::string&, const SequencePtr&,
                                           const std::string&, bool, int&, bool,
                                           const StringList& = StringList());

    //
    // Generate code to marshal or unmarshal a type using the public stream API.
    //
    void writeStreamMarshalUnmarshalCode(::IceUtil::Output&, const std::string&, const TypePtr&, const std::string&,
                                         bool, int&, bool = false, const StringList& = StringList(),
                                         const std::string& patchParams = "");

    //
    // Generate code to marshal or unmarshal a dictionary type using the public stream API.
    //
    void writeStreamDictionaryMarshalUnmarshalCode(::IceUtil::Output&, const std::string&, const DictionaryPtr&,
                                                   const std::string&, bool, int&, bool,
                                                   const StringList& = StringList());

    //
    // Generate code to marshal or unmarshal a sequence type using the public stream API.
    //
    void writeStreamSequenceMarshalUnmarshalCode(::IceUtil::Output&, const std::string&, const SequencePtr&,
                                                 const std::string&, bool, int&, bool,
                                                 const StringList& = StringList());

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
    bool getSequenceTypes(const SequencePtr&, const std::string&, const StringList&,
                          std::string&, std::string&) const;

    virtual JavaOutput* createOutput();

    Slice::FeatureProfile _featureProfile;

    static const std::string _getSetMetaData;
    static const std::string _java2MetaData;
    static const std::string _java5MetaData;

private:

    class MetaDataVisitor : public ParserVisitor
    {
    public:

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
    ::IceUtil::Output* _out;
};

}

#endif
