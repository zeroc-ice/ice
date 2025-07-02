// Copyright (c) ZeroC, Inc.

#ifndef JAVA_UTIL_H
#define JAVA_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

namespace Slice
{
    namespace Java
    {
        enum TypeMode
        {
            TypeModeIn,
            TypeModeOut,
            TypeModeMember,
            TypeModeReturn
        };

        [[nodiscard]] std::string
        getResultType(const OperationPtr& op, const std::string& package, bool object, bool dispatch);

        /// Returns a vector of this operation's parameters with each of them formatted as 'paramType paramName'.
        /// If 'internal' is true, the names will be prefixed with "iceP_".
        [[nodiscard]] std::vector<std::string>
        getParamsProxy(const OperationPtr& op, const std::string& package, bool optionalMapping, bool internal = false);

        /// Returns a vector of this operation's parameter's names in order.
        /// If 'internal' is true, the names will be prefixed with "iceP_".
        [[nodiscard]] std::vector<std::string> getInArgs(const OperationPtr& op, bool internal = false);

        //
        // These functions should only be called for classes, exceptions, and structs.
        // Enums automatically implement Serializable (Java just serializes the enumerator's identifier),
        // and proxies get their implementation from `_ObjectPrxI`.
        //
        [[nodiscard]] std::string getSerialVersionUID(const ContainedPtr& p);
        [[nodiscard]] std::int64_t computeDefaultSerialVersionUID(const ContainedPtr& p);

        /// Returns true if we can generate a method from the given data member list. A Java method
        /// can have a maximum of 255 parameters (including the implicit 'this') where each parameter
        /// is counted as 1 unit, except for long and double which are counted as 2 units.
        /// See https://docs.oracle.com/javase/specs/jvms/se20/html/jvms-4.html#jvms-4.3.3
        [[nodiscard]] bool isValidMethodParameterList(const DataMemberList& members, int additionalUnits = 0);

        /// Returns true if and only if 'p' maps to one of the builtin Java types (ie. a primitive type or a string).
        [[nodiscard]] bool mapsToJavaBuiltinType(const TypePtr& p);

        /// Returns a package prefix specified by the 'java:package' metadata if present, or the empty string if not.
        [[nodiscard]] std::string getPackagePrefix(const ContainedPtr& contained);

        /// Returns the Java package that 'contained' will be mapped into.
        [[nodiscard]] std::string getPackage(const ContainedPtr& contained);

        /// Returns the Java type without a package if the package matches the current package
        [[nodiscard]] std::string getUnqualified(const std::string& type, const std::string& package);

        /// Returns the qualified Java name that 'contained' will be mapped to (ie. package + '.' + name).
        ///
        /// This name is qualified relative to the provided 'package',
        /// so if 'contained' lives within this package, the returned name will have no qualification.
        [[nodiscard]] std::string getUnqualified(const ContainedPtr& cont, const std::string& package = std::string());

        /// Returns the method call necessary to obtain the static type ID for an object type.
        [[nodiscard]] std::string getStaticId(const TypePtr& type, const std::string& package);

        /// Returns the optional type corresponding to the given Slice type.
        [[nodiscard]] std::string getOptionalFormat(const TypePtr& type);

        [[nodiscard]] std::string typeToString(
            const TypePtr& type,
            TypeMode mode,
            const std::string& package = std::string(),
            const MetadataList& metadata = MetadataList(),
            bool formal = true,
            bool optional = false);

        /// Get the Java object name for a type. For primitive types, this returns the Java class type (e.g., Integer).
        /// For all other types, this function delegates to typeToString.
        [[nodiscard]] std::string typeToObjectString(
            const TypePtr& type,
            TypeMode mode,
            const std::string& package = std::string(),
            const MetadataList& metadata = MetadataList(),
            bool formal = true);

        /// Returns `true` if the metadata has an entry with the given directive, and `false` otherwise.
        [[nodiscard]] bool hasMetadata(const std::string& directive, const MetadataList& metadata);

        /// Gets any custom type metadata.
        /// If metadata is found, the abstract and concrete types are extracted and the function returns true.
        /// If an abstract type is not specified, it is set to an empty string.
        [[nodiscard]] bool
        getTypeMetadata(const MetadataList& metadata, std::string& instanceType, std::string& formalType);

        /// Determine whether a custom type is defined. The function checks the metadata of the type's original
        /// definition, as well as any optional metadata that typically represents a data member or parameter.
        [[nodiscard]] bool hasTypeMetadata(const SequencePtr& seq, const MetadataList& localMetadata = MetadataList());

        //
        // Obtain the concrete and abstract types for a dictionary or sequence type.
        // The functions return true if a custom type was defined and false to indicate
        // the default mapping was used.
        //
        bool getDictionaryTypes(
            const DictionaryPtr& dict,
            const std::string& package,
            const MetadataList& metadata,
            std::string& instanceType,
            std::string& formalType);
        bool getSequenceTypes(
            const SequencePtr& seq,
            const std::string& package,
            const MetadataList& metadata,
            std::string& instanceType,
            std::string& formalType);

        void validateMetadata(const UnitPtr&);
    }

    class JavaOutput final : public ::IceInternal::Output
    {
    public:
        JavaOutput();

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

        void close();

        JavaGenerator(std::string);

        /// Creates a new '.java' file (and any necessary intermediate directories) for generating 'qualifiedEntity's
        /// source code into. After calling this function, `_out` will be set to write into this new file.
        /// @param qualifiedEntity The fully qualified Java type name (ie. package + '.' + name).
        /// @param sliceFile The path of the Slice file that we're generating code for.
        void open(const std::string& qualifiedEntity, const std::string& sliceFile);

        [[nodiscard]] IceInternal::Output& output() const;

    private:
        std::string _dir;
        ::IceInternal::Output* _out{nullptr};
    };
}

#endif
