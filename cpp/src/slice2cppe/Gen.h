// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GEN_H
#define GEN_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

//
// The Ice-E version.
//
#define ICEE_STRING_VERSION "1.2.0" // "A.B.C", with A=major, B=minor, C=patch
#define ICEE_INT_VERSION 10200      // AABBCC, with AA=major, BB=minor, CC=patch

namespace Slice
{

class Gen : private ::IceUtil::noncopyable
{
public:

    Gen(const std::string&,
        const std::string&,
        const std::string&,
        const std::string&,
        const std::vector<std::string>&,
        const std::string&,
        const std::vector<std::string>&,
        const std::string&,
        const std::string&,
        bool,
        bool);
    ~Gen();

    bool operator!() const; // Returns true if there was a constructor error

    void generate(const UnitPtr&);
    void closeOutput();
    static bool setUseWstring(ContainedPtr, std::list<bool>&, bool);
    static bool resetUseWstring(std::list<bool>&);

private:

    void writeExtraHeaders(::IceUtilInternal::Output&);

    ::IceUtilInternal::Output H;
    ::IceUtilInternal::Output C;

    ::IceUtilInternal::Output implH;
    ::IceUtilInternal::Output implC;

    std::string _base;
    std::string _headerExtension;
    std::string _sourceExtension;
    std::vector<std::string> _extraHeaders;
    std::string _include;
    std::vector<std::string> _includePaths;
    std::string _dllExport;
    bool _impl;
    bool _ice;

    class GlobalIncludeVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        GlobalIncludeVisitor(::IceUtilInternal::Output&);

        virtual bool visitModuleStart(const ModulePtr&);

    private:

        ::IceUtilInternal::Output& H;

        bool _finished;
    };

    class TypesVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        TypesVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual void visitExceptionEnd(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitStructEnd(const StructPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual void visitEnum(const EnumPtr&);
        virtual void visitConst(const ConstPtr&);
        virtual void visitDataMember(const DataMemberPtr&);

    private:

        void emitUpcall(const ExceptionPtr&, const std::string&, bool = false);

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
        bool _doneStaticSymbol;
        bool _useWstring;
        std::list<bool> _useWstringHist;
    };

    class ProxyDeclVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        ProxyDeclVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitUnitStart(const UnitPtr&);
        virtual void visitUnitEnd(const UnitPtr&);
        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual void visitClassDecl(const ClassDeclPtr&);

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
    };

    class ProxyVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        ProxyVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitUnitStart(const UnitPtr&);
        virtual void visitUnitEnd(const UnitPtr&);
        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
        bool _useWstring;
        std::list<bool> _useWstringHist;
    };

    class ObjectDeclVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        ObjectDeclVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual void visitClassDecl(const ClassDeclPtr&);

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
    };

    class ObjectVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        ObjectVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitOperation(const OperationPtr&);
        virtual void visitDataMember(const DataMemberPtr&);

    private:

        bool emitVirtualBaseInitializers(const ClassDefPtr&);
        void emitOneShotConstructor(const ClassDefPtr&);
        void emitUpcall(const ClassDefPtr&, const std::string&);

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
        bool _useWstring;
        std::list<bool> _useWstringHist;
    };

    class IceInternalVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        IceInternalVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitUnitStart(const UnitPtr&);
        virtual void visitUnitEnd(const UnitPtr&);
        virtual void visitClassDecl(const ClassDeclPtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
    };

    class HandleVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        HandleVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual void visitClassDecl(const ClassDeclPtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
    };

    class ImplVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        ImplVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
        bool _useWstring;
        std::list<bool> _useWstringHist;

        //
        // Generate code to return a dummy value
        //
        void writeReturn(::IceUtilInternal::Output&, const TypePtr&, const StringList&);
    };

    class MetaDataVisitor : public ParserVisitor
    {
    public:

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual void visitClassDecl(const ClassDeclPtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual void visitExceptionEnd(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitStructEnd(const StructPtr&);
        virtual void visitOperation(const OperationPtr&);
        virtual void visitParamDecl(const ParamDeclPtr&);
        virtual void visitDataMember(const DataMemberPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual void visitEnum(const EnumPtr&);
        virtual void visitConst(const ConstPtr&);

    private:

        void validate(const SyntaxTreeBasePtr&, const StringList&, const std::string&, const std::string&,
                      bool = false);

        StringSet _history;
    };

    static void validateMetaData(const UnitPtr&);
    static void printHeader(IceUtilInternal::Output&);
    static void printVersionCheck(IceUtilInternal::Output&);
    static void printDllExportStuff(IceUtilInternal::Output&, const std::string&);
};

}

#endif
