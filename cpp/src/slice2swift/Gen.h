// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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

        virtual bool visitModuleStart(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);

        void writeImports();

    private:

        void addImport(const TypePtr&, const ContainedPtr&);
        void addImport(const ContainedPtr&, const ContainedPtr&);

        IceUtilInternal::Output& out;
        std::vector< std::string> _imports;
    };

    class TypesVisitor : public SwiftGenerator, public ParserVisitor
    {

    public:

        TypesVisitor(IceUtilInternal::Output&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual void visitEnum(const EnumPtr&);
        virtual void visitConst(const ConstPtr&);

    private:

        IceUtilInternal::Output& out;
    };
};

}

#endif
