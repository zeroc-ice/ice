//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        Gen(const std::string&);

        void generate(const UnitPtr&);

    private:
        std::string _fileBase;

        class OutputVisitor final : public ParserVisitor
        {
        public:
            bool visitClassDefStart(const ClassDefPtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            bool visitExceptionStart(const ExceptionPtr&) final;
            bool visitStructStart(const StructPtr&) final;
            void visitSequence(const SequencePtr&)final;
            void visitDictionary(const DictionaryPtr&) final;
            void visitEnum(const EnumPtr&) final;

            std::set<std::string> modules() const;

        private:
            std::set<std::string> _modules;
        };

        class TypesVisitor final : public ParserVisitor
        {
        public:
            TypesVisitor(const std::string&, const std::set<std::string>&);

            bool visitClassDefStart(const ClassDefPtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            bool visitExceptionStart(const ExceptionPtr&) final;
            bool visitStructStart(const StructPtr&) final;
            void visitSequence(const SequencePtr&) final;
            void visitDictionary(const DictionaryPtr&) final;
            void visitEnum(const EnumPtr&) final;
            void visitConst(const ConstPtr&) final;

            void newLine(); // Ensure all files end with a newline

        private:
            IceInternal::Output& getOutput(const ContainedPtr&);

            std::string _fileBase;
            std::set<std::string> _modules;
            std::map<std::string, std::unique_ptr<IceInternal::Output>> _outputs;
        };
    };
}

#endif
