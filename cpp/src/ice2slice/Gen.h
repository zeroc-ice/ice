// Copyright (c) ZeroC, Inc.

#ifndef GEN_H
#define GEN_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

#include <map>

namespace Slice
{
    class Gen
    {
    public:
        Gen(std::string);

        void generate(const UnitPtr&);

    private:
        std::string _fileBase;

        /// The OutputModulesVisitor class gathers a list of modules requiring the generation of output `.slice` files.
        /// Each `.ice` file may correspond to multiple `.slice` files, with one for each module that contains Slice
        /// definitions.
        class OutputModulesVisitor final : public ParserVisitor
        {
        public:
            bool visitClassDefStart(const ClassDefPtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            bool visitExceptionStart(const ExceptionPtr&) final;
            bool visitStructStart(const StructPtr&) final;
            void visitSequence(const SequencePtr&) final;
            void visitDictionary(const DictionaryPtr&) final;
            void visitEnum(const EnumPtr&) final;

            /// After visiting all definitions, return the list of scopes corresponding to modules for which we need
            /// to generate an output `.slice`.
            /// @return The list of modules for which we need to generate an output `.slice`.
            [[nodiscard]] std::set<std::string> modules() const;

        private:
            std::set<std::string> _modules;
        };

        /// The TypesVisitor class converts Slice definitions in `.ice` files to corresponding Slice definitions in
        /// `.slice` files.
        ///
        /// If the `.ice` file contains a single module, the output is a single `.slice` file with the same base name,
        /// replacing the `.ice` extension with `.slice`.
        ///
        /// If the `.ice` file contains multiple modules, it generates a `.slice` file for each module. Each file uses
        /// the base name of the `.ice` file, appending the module scope with "::" replaced by "_" and the `.slice`
        /// extension.
        class TypesVisitor final : public ParserVisitor
        {
        public:
            TypesVisitor(std::string fileBase, const std::set<std::string>& modules);

            void visitUnitEnd(const UnitPtr&) final;
            bool visitClassDefStart(const ClassDefPtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            bool visitExceptionStart(const ExceptionPtr&) final;
            bool visitStructStart(const StructPtr&) final;
            void visitSequence(const SequencePtr&) final;
            void visitDictionary(const DictionaryPtr&) final;
            void visitEnum(const EnumPtr&) final;
            void visitConst(const ConstPtr&) final;

        private:
            IceInternal::Output& getOutput(const ContainedPtr&);

            std::string _fileBase;
            std::set<std::string> _modules;
            std::map<std::string, std::unique_ptr<IceInternal::Output>> _outputs;
        };
    };
}

#endif
