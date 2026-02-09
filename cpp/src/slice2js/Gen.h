// Copyright (c) ZeroC, Inc.

#ifndef GEN_H
#define GEN_H

#include "JsUtil.h"

namespace Slice
{
    struct DocSummaryOptions
    {
        /// If false, `@@deprecated` tags in the Slice doc-comment will be ignored.
        bool generateDeprecated{true};

        /// If false, `@@remarks` tags in the Slice doc-comment will be ignored. This should always be 'true' for
        /// typescript generated code, and 'false' for javascript generated code.
        bool includeRemarks{true};

        std::optional<std::string> generatedType{std::nullopt};
    };

    class JsVisitor : public ParserVisitor
    {
    public:
        JsVisitor(::IceInternal::Output& output);
        ~JsVisitor() override;

    protected:
        void writeMarshalDataMembers(const DataMemberList&, const DataMemberList&);
        void writeUnmarshalDataMembers(const DataMemberList&, const DataMemberList&);
        void writeOneShotConstructorArguments(const DataMemberList& members);

        std::string getValue(const TypePtr&);

        std::string writeConstantValue(const TypePtr&, const SyntaxTreeBasePtr&, const std::string&);

        /// Generates and outputs a doc-summary for Slice definition @p p.
        /// @param p The Slice definition to be documented.
        /// @param options Options that control how the doc-summary is generated.
        void writeDocSummary(const ContainedPtr& p, const DocSummaryOptions& options = {});

        ::IceInternal::Output& _out;
    };

    class IncludeAggregationVisitor : public ParserVisitor
    {
    public:
        bool visitUnitStart(const UnitPtr& unit) final;
        void visitModuleEnd(const ModulePtr& module) final;

        bool visitClassDefStart(const ClassDefPtr&) final;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
        bool visitStructStart(const StructPtr&) final;
        bool visitExceptionStart(const ExceptionPtr&) final;
        void visitSequence(const SequencePtr&) final;
        void visitDictionary(const DictionaryPtr&) final;
        void visitEnum(const EnumPtr&) final;

        [[nodiscard]] bool shouldVisitIncludedDefinitions() const final;
        [[nodiscard]] const std::map<std::string, std::map<std::string, std::set<std::string>>>&
        nestedModulesByTopLevel() const;
        [[nodiscard]] std::map<std::string, std::set<std::string>>
        importedTypesByTopLevel(const std::string& topLevelFile) const;

    private:
        void addImportedType(const ContainedPtr& definition);

        // Resolves a transitive include to its owning direct include.
        // Returns the normalized include key, or nullopt if not found.
        [[nodiscard]] std::optional<std::string> resolveDirectInclude(const DefinitionContextPtr& dc) const;

        // Map of top-level Slice files to nested modules per direct include file.
        // For each top-level visited Slice file we keep a map:
        // - Key is a normalized direct include filename (using same logic as import generation)
        // - Value is the set of nested (non-top-level) modules defined in that include, directly or transitively.
        // Used by JavaScript ImportVisitor::writeImports to aggregate nested modules imported from multiple files.
        std::map<std::string, std::map<std::string, std::set<std::string>>> _nestedModulesByTopLevel;

        // Map of Slice files to scoped types per direct include file.
        // For each top-level visited Slice file we keep a map:
        // - Key is a normalized direct include filename (using same logic as import generation)
        // - Value is a set of types defined in the included file, either directly or transitively.
        // This map answers the question: what types are imported from a given included file.
        std::map<std::string, std::map<std::string, std::set<std::string>>> _importedTypesByTopLevel;

        // The current top-level file being processed.
        std::string _topLevelFile;
        // The js:module of the top-level file (empty if not set).
        std::string _topLevelModule;
        // The unit being visited (needed for include file normalization).
        UnitPtr _unit;
        // Map from transitive include file (resolved path) to owning direct include (normalized key).
        // Only populated for aggregatable includes (not external js:module imports).
        std::map<std::string, std::string> _transitiveToDirectInclude;
    };

    class Gen final
    {
    public:
        Gen(const std::string&, const std::string&, bool);

        Gen(const std::string&, const std::string&, bool, std::ostream&);

        ~Gen();

        void generate(const UnitPtr&);

    private:
        IceInternal::Output _javaScriptOutput;
        IceInternal::Output _typeScriptOutput;

        std::string _fileBase;
        bool _useStdout;
        bool _typeScript;

        class ImportVisitor final : public JsVisitor
        {
        public:
            ImportVisitor(::IceInternal::Output&);

            bool visitClassDefStart(const ClassDefPtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            bool visitStructStart(const StructPtr&) final;
            void visitOperation(const OperationPtr&) final;
            bool visitExceptionStart(const ExceptionPtr&) final;
            void visitSequence(const SequencePtr&) final;
            void visitDictionary(const DictionaryPtr&) final;
            void visitEnum(const EnumPtr&) final;

            // Emit the import statements for the given unit and return a list of the imported modules.
            std::set<std::string>
            writeImports(const UnitPtr&, const std::map<std::string, std::map<std::string, std::set<std::string>>>&);

        private:
            bool _seenClass{false};
            bool _seenInterface{false};
            bool _seenOperation{false};
            bool _seenStruct{false};
            bool _seenUserException{false};
            bool _seenEnum{false};
            bool _seenSeq{false};
            bool _seenDict{false};
            bool _seenObjectSeq{false};
            bool _seenObjectProxySeq{false};
            bool _seenObjectDict{false};
            bool _seenObjectProxyDict{false};
        };

        class ExportsVisitor final : public JsVisitor
        {
        public:
            ExportsVisitor(::IceInternal::Output&, std::set<std::string>);

            bool visitModuleStart(const ModulePtr&) final;

            [[nodiscard]] std::set<std::string> exportedModules() const;

        private:
            std::string encodeTypeForOperation(const TypePtr&);

            std::set<std::string> _importedModules;
            std::set<std::string> _exportedModules;
        };

        class TypesVisitor final : public JsVisitor
        {
        public:
            TypesVisitor(::IceInternal::Output&);

            bool visitClassDefStart(const ClassDefPtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            bool visitExceptionStart(const ExceptionPtr&) final;
            bool visitStructStart(const StructPtr&) final;
            void visitSequence(const SequencePtr&) final;
            void visitDictionary(const DictionaryPtr&) final;
            void visitEnum(const EnumPtr&) final;
            void visitConst(const ConstPtr&) final;

        private:
            std::string encodeTypeForOperation(const TypePtr&);
        };

        class TypeScriptImportVisitor final : public JsVisitor
        {
        public:
            TypeScriptImportVisitor(::IceInternal::Output&, std::map<std::string, std::set<std::string>>);

            bool visitUnitStart(const UnitPtr&) final;
            bool visitClassDefStart(const ClassDefPtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            bool visitStructStart(const StructPtr&) final;
            bool visitExceptionStart(const ExceptionPtr&) final;
            void visitSequence(const SequencePtr&) final;
            void visitDictionary(const DictionaryPtr&) final;

            // Emit the import statements for the given unit and return a map of the imported types per module.
            std::map<std::string, std::string> writeImports();

        private:
            void addImport(const ContainedPtr&);

            // All modules imported by the current unit.
            std::set<std::string> _importedModules;
            // A map of imported types to their module name.
            std::map<std::string, std::string> _importedTypes;
            // The module name of the current unit.
            std::string _module;
            // The filename of the current unit.
            std::string _filename;
            // A map of include files to types for re-export (used to determine which files to import).
            std::map<std::string, std::set<std::string>> _importedTypesByInclude;
        };

        class TypeScriptVisitor final : public JsVisitor
        {
        public:
            TypeScriptVisitor(
                ::IceInternal::Output&,
                std::map<std::string, std::string>,
                std::map<std::string, std::set<std::string>>);

            bool visitUnitStart(const UnitPtr&) final;
            void visitUnitEnd(const UnitPtr&) final;
            bool visitModuleStart(const ModulePtr&) final;
            void visitModuleEnd(const ModulePtr&) final;
            bool visitClassDefStart(const ClassDefPtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            bool visitExceptionStart(const ExceptionPtr&) final;
            bool visitStructStart(const StructPtr&) final;
            void visitSequence(const SequencePtr&) final;
            void visitDictionary(const DictionaryPtr&) final;
            void visitEnum(const EnumPtr&) final;
            void visitConst(const ConstPtr&) final;

        private:
            [[nodiscard]] std::string importPrefix(const std::string&) const;
            [[nodiscard]] std::string
            typeToTsString(const TypePtr&, bool nullable = false, bool forParameter = false, bool optional = false)
                const;
            void writeOpDocSummary(::IceInternal::Output& out, const OperationPtr& op, bool forDispatch);
            void writeNestedModuleExports(const std::string& currentModuleScope);

            // The module name of the current unit.
            std::string _module;
            // The import prefix for the "ice" module either empty string when building Ice or "__module__zeroc_ice."
            std::string _iceImportPrefix;
            // A map of imported types to their module name.
            std::map<std::string, std::string> _importedTypes;
            // A map of include files to the types they define (for generating nested module exports).
            std::map<std::string, std::set<std::string>> _importedTypesByInclude;
            // The set of modules visited in the current file (used to detect missing nested modules).
            std::set<std::string> _visitedModules;
            // Cache of types already exported (to prevent duplicates across nested module exports).
            std::set<std::string> _exportedTypes;
        };
    };
}

#endif
