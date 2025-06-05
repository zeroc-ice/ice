// Copyright (c) ZeroC, Inc.

#ifndef JAVA_UTIL_H
#define JAVA_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

namespace Slice
{
    // Get the Java name for a type.
    // If an optional scope is provided, the scope will be removed from the result if possible.
    enum TypeMode
    {
        TypeModeIn,
        TypeModeOut,
        TypeModeMember,
        TypeModeReturn
    };

    //
    // These functions should only be called for classes, exceptions, and structs.
    // Enums automatically implement Serializable (Java just serializes the enumerator's identifier),
    // and proxies get their implementation from `_ObjectPrxI`.
    //
    std::string getSerialVersionUID(const ContainedPtr&);
    std::int64_t computeDefaultSerialVersionUID(const ContainedPtr&);

    //
    // Returns true if we can generate a method from the given data member list. A Java method
    // can have a maximum of 255 parameters (including the implicit 'this') where each parameter
    // is counted as 1 unit, except for long and double which are counted as 2 units.
    // See https://docs.oracle.com/javase/specs/jvms/se20/html/jvms-4.html#jvms-4.3.3
    //
    bool isValidMethodParameterList(const DataMemberList&, int additionalUnits = 0);

    /// Returns true if and only if 'p' maps to one of the builtin Java types (ie. a primitive type or a string).
    bool mapsToJavaBuiltinType(const TypePtr& p);

    class JavaOutput final : public ::IceInternal::Output
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

        void printHeader();
    };

    class JavaGenerator
    {
    public:
        JavaGenerator(const JavaGenerator&) = delete;
        virtual ~JavaGenerator();

        JavaGenerator& operator=(const JavaGenerator&) = delete;

        static void validateMetadata(const UnitPtr&);

        void close();

        JavaGenerator(std::string);

        /// Creates a new '.java' file (and any necessary intermediate directories) for generating 'qualifiedEntity's
        /// source code into. After calling this function, `_out` will be set to write into this new file.
        /// @param qualifiedEntity The fully qualified Java type name (ie. package + '.' + name).
        /// @param sliceFile The path of the Slice file that we're generating code for.
        void open(const std::string& qualifiedEntity, const std::string& sliceFile);

        [[nodiscard]] IceInternal::Output& output() const;

        /// Returns a package prefix specified by the 'java:package' metadata if present, or the empty string if not.
        [[nodiscard]] static std::string getPackagePrefix(const ContainedPtr& contained);

        /// Returns the Java package that 'contained' will be mapped into.
        [[nodiscard]] static std::string getPackage(const ContainedPtr& contained);

        // Returns the Java type without a package if the package matches the current package
        [[nodiscard]] static std::string getUnqualified(const std::string& type, const std::string& package);

        /// Returns the qualified Java name that 'contained' will be mapped to (ie. package + '.' + name).
        ///
        /// This name is qualified relative to the provided 'package',
        /// so if 'contained' lives within this package, the returned name will have no qualification.
        [[nodiscard]] static std::string
        getUnqualified(const ContainedPtr& cont, const std::string& package = std::string());

        //
        // Return the method call necessary to obtain the static type ID for an object type.
        //
        [[nodiscard]] static std::string getStaticId(const TypePtr&, const std::string&);

        //
        // Returns the optional type corresponding to the given Slice type.
        //
        [[nodiscard]] static std::string getOptionalFormat(const TypePtr&);

        [[nodiscard]] static std::string typeToString(
            const TypePtr&,
            TypeMode,
            const std::string& = std::string(),
            const MetadataList& = MetadataList(),
            bool = true,
            bool = false);

        //
        // Get the Java object name for a type. For primitive types, this returns the
        // Java class type (e.g., Integer). For all other types, this function delegates
        // to typeToString.
        //
        [[nodiscard]] static std::string typeToObjectString(
            const TypePtr&,
            TypeMode,
            const std::string& = std::string(),
            const MetadataList& = MetadataList(),
            bool = true);

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

        void writeMarshalUnmarshalCode(
            ::IceInternal::Output&,
            const std::string&,
            const TypePtr&,
            OptionalMode,
            bool,
            std::int32_t,
            const std::string&,
            bool,
            int&,
            const std::string& = "",
            const MetadataList& = MetadataList(),
            const std::string& = "");

        //
        // Generate code to marshal or unmarshal a dictionary type.
        //
        void writeDictionaryMarshalUnmarshalCode(
            ::IceInternal::Output&,
            const std::string&,
            const DictionaryPtr&,
            const std::string&,
            bool,
            int&,
            bool,
            const std::string& = "",
            const MetadataList& = MetadataList());

        //
        // Generate code to marshal or unmarshal a sequence type.
        //
        void writeSequenceMarshalUnmarshalCode(
            ::IceInternal::Output&,
            const std::string&,
            const SequencePtr&,
            const std::string&,
            bool,
            int&,
            bool,
            const std::string& = "",
            const MetadataList& = MetadataList());

        //
        // Returns` true` if the metadata has an entry with the given directive, and `false` otherwise.
        //
        static bool hasMetadata(const std::string&, const MetadataList&);

        //
        // Get custom type metadata. If metadata is found, the abstract and
        // concrete types are extracted and the function returns true. If an
        // abstract type is not specified, it is set to an empty string.
        //
        static bool getTypeMetadata(const MetadataList&, std::string&, std::string&);

        //
        // Determine whether a custom type is defined. The function checks the
        // metadata of the type's original definition, as well as any optional
        // metadata that typically represents a data member or parameter.
        //
        static bool hasTypeMetadata(const SequencePtr&, const MetadataList& = MetadataList());

        //
        // Obtain the concrete and abstract types for a dictionary or sequence type.
        // The functions return true if a custom type was defined and false to indicate
        // the default mapping was used.
        //
        static bool
        getDictionaryTypes(const DictionaryPtr&, const std::string&, const MetadataList&, std::string&, std::string&);
        static bool
        getSequenceTypes(const SequencePtr&, const std::string&, const MetadataList&, std::string&, std::string&);

        JavaOutput* createOutput();

    private:
        std::string _dir;
        ::IceInternal::Output* _out{nullptr};
    };
}

#endif
