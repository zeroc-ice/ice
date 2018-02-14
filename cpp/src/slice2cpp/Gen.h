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

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

class Gen : private ::IceUtil::noncopyable
{
public:

    Gen(const std::string&,
        const std::string&,
        const std::string&,
        const std::vector<std::string>&,
        const std::string&,
        const std::vector<std::string>&,
        const std::string&,
        const std::string&,
        bool,
        bool,
        bool,
        bool);
    ~Gen();

    void generate(const UnitPtr&);
    void closeOutput();
    
    static int setUseWstring(ContainedPtr, std::list<int>&, int);
    static int resetUseWstring(std::list<int>&);

private:

    void writeExtraHeaders(::IceUtilInternal::Output&);


    //
    // Returns the header extension defined in the global metadata for a given file,
    // or an empty string if no global metadata was found.
    //
    std::string getHeaderExt(const std::string& file, const UnitPtr& unit);

    ::IceUtilInternal::Output H;
    ::IceUtilInternal::Output C;

    ::IceUtilInternal::Output implH;
    ::IceUtilInternal::Output implC;

    std::string _base;
    std::string _headerExtension;
    std::string _implHeaderExtension;
    std::string _sourceExtension;
    std::vector<std::string> _extraHeaders;
    std::string _include;
    std::vector<std::string> _includePaths;
    std::string _dllExport;
    std::string _dir;
    bool _impl;
    bool _checksum;
    bool _stream;
    bool _ice;

    class TypesVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        TypesVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&, bool);

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
        bool _stream;
        bool _doneStaticSymbol;
        int _useWstring;
        std::list<int> _useWstringHist;
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
        int _useWstring;
        std::list<int> _useWstringHist;
    };

    class ObjectDeclVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        ObjectDeclVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual void visitClassDecl(const ClassDeclPtr&);
        virtual void visitOperation(const OperationPtr&);

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
    };

    class ObjectVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        ObjectVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&, bool);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitOperation(const OperationPtr&);

    private:

        void emitDataMember(const DataMemberPtr&);
        void emitGCFunctions(const ClassDefPtr&);
        void emitGCVisitCode(const TypePtr&, const std::string&, const std::string&, int);
        void emitGCClearCode(const TypePtr&, const std::string&, const std::string&, int);
        bool emitVirtualBaseInitializers(const ClassDefPtr&, bool virtualInheritance, bool direct);
        void emitOneShotConstructor(const ClassDefPtr&);
        void emitUpcall(const ClassDefPtr&, const std::string&);

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
        bool _stream;
        bool _doneStaticSymbol;
        int _useWstring;
        std::list<int> _useWstringHist;
    };

    class AsyncCallbackVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        AsyncCallbackVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);

    private:

        ::IceUtilInternal::Output& H;

        std::string _dllExport;
        int _useWstring;
        std::list<int> _useWstringHist;
    };

    class AsyncCallbackTemplateVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        AsyncCallbackTemplateVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitUnitStart(const UnitPtr&);
        virtual void visitUnitEnd(const UnitPtr&);
        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);

    private:

        void generateOperation(const OperationPtr&, bool);

        ::IceUtilInternal::Output& H;

        std::string _dllExport;
        int _useWstring;
        std::list<int> _useWstringHist;
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
        int _useWstring;
        std::list<int> _useWstringHist;

        //
        // Generate code to emit a local variable declaration and initialize it
        // if necessary.
        //
        void writeDecl(::IceUtilInternal::Output&, const std::string&, const TypePtr&, const StringList&);

        //
        // Generate code to return a dummy value
        //
        void writeReturn(::IceUtilInternal::Output&, const TypePtr&, const StringList&);
    };

    class AsyncVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        AsyncVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);

    private:

        ::IceUtilInternal::Output& H;

        std::string _dllExport;
        int _useWstring;
        std::list<int> _useWstringHist;
    };

    class AsyncImplVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        AsyncImplVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

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
        int _useWstring;
        std::list<int> _useWstringHist;
    };
    
    class StreamVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        StreamVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual void visitEnum(const EnumPtr&);

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;
        std::string _dllExport;
    };

private:

    class MetaDataVisitor : public ParserVisitor
    {
    public:

        virtual bool visitUnitStart(const UnitPtr&);
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
};

}

#endif
