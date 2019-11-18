//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef JAVA_UTIL_H
#define JAVA_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

//
// Compute Java serialVersionUID for a Slice class
//
long
computeSerialVersionUUID(const ClassDefPtr&);

//
// Compute Java serialVersionUID for a Slice class
//
long
computeSerialVersionUUID(const ExceptionPtr&);

//
// Compute Java serialVersionUID for a Slice struct
//
long
computeSerialVersionUUID(const StructPtr&);

class JavaOutput : public ::IceUtilInternal::Output
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

class JavaGenerator : private ::IceUtil::noncopyable
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
    // Returns the package prefix of a Contained entity.
    //
    std::string getPackagePrefix(const ContainedPtr&) const;

    //
    // Returns the Java package of a Contained entity.
    //
    std::string getPackage(const ContainedPtr&) const;

    //
    // Returns the Java type without a package if the package
    // matches the current package
    //
    std::string getUnqualified(const std::string&, const std::string&) const;

    //
    // Returns the Java name for a Contained entity. If the optional
    // package argument matches the entity's package name, then the
    // package is removed from the result.
    //
    std::string getUnqualified(const ContainedPtr&,
                            const std::string& = std::string(),
                            const std::string& = std::string(),
                            const std::string& = std::string()) const;

    //
    // Return the method call necessary to obtain the static type ID for an object type.
    //
    std::string getStaticId(const TypePtr&, const std::string&) const;

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

    std::string getWriteFunction(const std::string&, const TypePtr&);
    std::string getReadFunction(const std::string&, const TypePtr&);

    void writeMarshalUnmarshalCode(::IceUtilInternal::Output&, const std::string&, const TypePtr&, OptionalMode,
                                   bool, int, const std::string&, bool, int&, const std::string& = "",
                                   const StringList& = StringList(), const std::string& = "");

    //
    // Generate code to marshal or unmarshal a dictionary type.
    //
    void writeDictionaryMarshalUnmarshalCode(::IceUtilInternal::Output&, const std::string&, const DictionaryPtr&,
                                             const std::string&, bool, int&, bool,
                                             const std::string& = "", const StringList& = StringList());

    //
    // Generate code to marshal or unmarshal a sequence type.
    //
    void writeSequenceMarshalUnmarshalCode(::IceUtilInternal::Output&, const std::string&, const SequencePtr&,
                                           const std::string&, bool, int&, bool,
                                           const std::string& = "", const StringList& = StringList());

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

    virtual JavaOutput* createOutput();

    static const std::string _getSetMetaData;

private:

    std::string _dir;
    ::IceUtilInternal::Output* _out;
};

}

#endif
