//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef GEN_H
#define GEN_H

#include <SwiftUtil.h>

using namespace std;
using namespace Slice;

namespace Slice
{

class Gen
{
public:

    Gen(const std::string&,
        const std::vector<std::string>&,
        const std::string&);
    ~Gen();

    void generate(const UnitPtr&);
    void closeOutput();
    void printHeader();

private:

    IceUtilInternal::Output _out;

    std::vector<std::string> _includePaths;
    std::string _fileBase;

    class ImportVisitor : public SwiftGenerator, public ParserVisitor
    {
    public:

        ImportVisitor(IceUtilInternal::Output&);

        bool visitModuleStart(const ModulePtr&) override;
        bool visitClassDefStart(const ClassDefPtr&) override;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) override;
        bool visitStructStart(const StructPtr&) override;
        bool visitExceptionStart(const ExceptionPtr&) override;
        void visitSequence(const SequencePtr&) override;
        void visitDictionary(const DictionaryPtr&) override;

        void writeImports();

    private:

        void addImport(const TypePtr&, const ContainedPtr&);
        void addImport(const ContainedPtr&, const ContainedPtr&);
        void addImport(const std::string&);

        IceUtilInternal::Output& out;
        std::vector<std::string> _imports;
    };

    class TypesVisitor : public SwiftGenerator, public ParserVisitor
    {
    public:

        TypesVisitor(IceUtilInternal::Output&);

        bool visitClassDefStart(const ClassDefPtr&) override;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) override;
        bool visitExceptionStart(const ExceptionPtr&) override;
        bool visitStructStart(const StructPtr&) override;
        void visitSequence(const SequencePtr&) override;
        void visitDictionary(const DictionaryPtr&) override;
        void visitEnum(const EnumPtr&) override;
        void visitConst(const ConstPtr&) override;

    private:

        IceUtilInternal::Output& out;
    };

    class ProxyVisitor : public SwiftGenerator, public ParserVisitor
    {
    public:

        ProxyVisitor(::IceUtilInternal::Output&);

        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) override;
        void visitInterfaceDefEnd(const InterfaceDefPtr&) override;
        void visitOperation(const OperationPtr&) override;

    private:

        IceUtilInternal::Output& out;
    };

    // Value here means class instance.
    // TODO: merge into TypesVisitor
    class ValueVisitor : public SwiftGenerator, public ParserVisitor
    {
    public:

        ValueVisitor(::IceUtilInternal::Output&);

        bool visitClassDefStart(const ClassDefPtr&) override;
        void visitClassDefEnd(const ClassDefPtr&) override;

    private:

        IceUtilInternal::Output& out;
    };

    // Object here means servant
    class ObjectVisitor : public SwiftGenerator, public ParserVisitor
    {
    public:

        ObjectVisitor(::IceUtilInternal::Output&);

        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) override;
        void visitInterfaceDefEnd(const InterfaceDefPtr&) override;
        void visitOperation(const OperationPtr&) override;

    private:

        IceUtilInternal::Output& out;
    };

    class ObjectExtVisitor : public SwiftGenerator, public ParserVisitor
    {
    public:

        ObjectExtVisitor(::IceUtilInternal::Output&);

        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) override;
        void visitInterfaceDefEnd(const InterfaceDefPtr&) override;
        void visitOperation(const OperationPtr&) override;

    private:

        IceUtilInternal::Output& out;
    };
};

}

#endif
