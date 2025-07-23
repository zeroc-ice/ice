// Copyright (c) ZeroC, Inc.

#include "DocCommentParser.h"

using namespace std;
using namespace Slice;

namespace Slice
{
    class DocCommentParser final : public ParserVisitor
    {
    public:
        DocCommentParser(DocLinkFormatter linkFormatter) : _linkFormatter(linkFormatter) {}

        bool visitModuleStart(const ModulePtr& p) final;
        void visitClassDecl(const ClassDeclPtr& p) final;
        bool visitClassDefStart(const ClassDefPtr& p) final;
        void visitInterfaceDecl(const InterfaceDeclPtr& p) final;
        bool visitInterfaceDefStart(const InterfaceDefPtr& p) final;
        bool visitExceptionStart(const ExceptionPtr& p) final;
        bool visitStructStart(const StructPtr& p) final;
        void visitOperation(const OperationPtr& p) final;
        void visitDataMember(const DataMemberPtr& p) final;
        void visitSequence(const SequencePtr& p) final;
        void visitDictionary(const DictionaryPtr& p) final;
        void visitEnum(const EnumPtr& p) final;
        void visitConst(const ConstPtr& p) final;

        [[nodiscard]] bool shouldVisitIncludedDefinitions() const final { return true; }

    private:
        void parseDocCommentFor(const ContainedPtr& p);

        DocLinkFormatter _linkFormatter;
    };
}

void
Slice::parseAllDocComments(const UnitPtr& unit, DocLinkFormatter linkFormatter)
{
    DocCommentParser visitor{linkFormatter};
    unit->visit(&visitor);
}

bool
DocCommentParser::visitModuleStart(const ModulePtr& p)
{
    parseDocCommentFor(p);
    return true;
}

void
DocCommentParser::visitClassDecl(const ClassDeclPtr& p)
{
    parseDocCommentFor(p);
}

bool
DocCommentParser::visitClassDefStart(const ClassDefPtr& p)
{
    parseDocCommentFor(p);
    return true;
}

void
DocCommentParser::visitInterfaceDecl(const InterfaceDeclPtr& p)
{
    parseDocCommentFor(p);
}

bool
DocCommentParser::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    parseDocCommentFor(p);
    return true;
}

bool
DocCommentParser::visitExceptionStart(const ExceptionPtr& p)
{
    parseDocCommentFor(p);
    return true;
}

bool
DocCommentParser::visitStructStart(const StructPtr& p)
{
    parseDocCommentFor(p);
    return true;
}

void
DocCommentParser::visitOperation(const OperationPtr& p)
{
    parseDocCommentFor(p);
    for (const auto& param : p->parameters())
    {
        parseDocCommentFor(param);
    }
}

void
DocCommentParser::visitDataMember(const DataMemberPtr& p)
{
    parseDocCommentFor(p);
}

void
DocCommentParser::visitSequence(const SequencePtr& p)
{
    parseDocCommentFor(p);
}

void
DocCommentParser::visitDictionary(const DictionaryPtr& p)
{
    parseDocCommentFor(p);
}

void
DocCommentParser::visitEnum(const EnumPtr& p)
{
    parseDocCommentFor(p);
    for (const auto& enumerator : p->enumerators())
    {
        parseDocCommentFor(enumerator);
    }
}

void
DocCommentParser::visitConst(const ConstPtr& p)
{
    parseDocCommentFor(p);
}

void
DocCommentParser::parseDocCommentFor(const ContainedPtr& p)
{
    if (auto& docComment = p->_docComment)
    {
        docComment->parse(p, _linkFormatter);
    }
}
