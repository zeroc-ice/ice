// Copyright (c) ZeroC, Inc.

#ifndef SLICE_PYTHON_UTIL_H
#define SLICE_PYTHON_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

#include <map>
#include <set>
#include <string>

namespace Slice::Python
{
    /// Creates the package directory structure for a given Python module name.
    ///
    /// For example, if the module name is "Foo.Bar.Baz", this function creates
    /// the directories "Foo/Bar" under the specified output path.
    ///
    /// @param moduleName The name of the Python module (e.g., "Foo.Bar.Baz").
    /// @param outputPath The base directory in which to create the package directories. Must already exist.
    void createPackagePath(const std::string& moduleName, const std::string& outputPath);

    /// Writes the standard header comment to a generated Python source file.
    ///
    /// @param out The output stream to write the header to.
    void writeHeader(IceInternal::Output& out);

    /// Generate Python code for a translation unit.
    /// @param unit is the Slice unit to generate code for.
    /// @param outputDir The base-directory to write the generated Python files to.
    void generate(const Slice::UnitPtr& unit, const std::string& outputDir);

    int compile(const std::vector<std::string>&);

    /// Returns a DocString formatted link to the provided Slice identifier.
    std::string
    pyLinkFormatter(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target);

    /// Validates the Slice Python metadata for the given unit.
    void validatePythonMetadata(const UnitPtr&);

    // Collects Python definitions for each generated Python package.
    // Each package corresponds to a Slice module and includes an `__init__.py` file
    // that re-exports selected definitions from the package's internal modules.
    class PackageVisitor final : public ParserVisitor
    {
    public:
        bool visitModuleStart(const ModulePtr&) final;
        bool visitClassDefStart(const ClassDefPtr&) final;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
        bool visitStructStart(const StructPtr&) final;
        bool visitExceptionStart(const ExceptionPtr&) final;
        void visitSequence(const SequencePtr&) final;
        void visitDictionary(const DictionaryPtr&) final;
        void visitEnum(const EnumPtr&) final;
        void visitConst(const ConstPtr&) final;

        [[nodiscard]] const std::map<std::string, std::map<std::string, std::set<std::string>>>& imports()
        {
            return _imports;
        }
        [[nodiscard]] const std::set<std::string>& generatedModules() const { return _generatedModules; }
        [[nodiscard]] const std::set<std::string>& packageIndexFiles() const { return _packageIndexFiles; }

    private:
        void addRuntimeImport(const ContainedPtr& definition, const std::string& prefix = "");
        void addRuntimeImportForMetaType(const ContainedPtr& definition);

        // A map where:
        // - The outer key is the generated Python package name (e.g., "Foo" for the Slice module ::Foo).
        // - The inner key is the generated Python module name (e.g., "Bar" for the Slice class ::Foo::Bar).
        // - The value is the set of symbol names that the generated module contributes to the package.
        //   For example, the generated module for the Slice class "Bar" contributes the symbols "Bar" and
        //   "__Foo_Bar_t", corresponding to the class itself and its associated meta-type.
        std::map<std::string, std::map<std::string, std::set<std::string>>> _imports;

        // The set of generated Python modules.
        std::set<std::string> _generatedModules;

        // The se of generated package index files.
        std::set<std::string> _packageIndexFiles;
    };
}

#endif
