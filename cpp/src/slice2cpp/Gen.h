//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

    //
    // Returns the source extension defined in the global metadata for a given file,
    // or an empty string if no global metadata was found.
    //
    std::string getSourceExt(const std::string& file, const UnitPtr& unit);

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

    // Visitors

    class DeclVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        DeclVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        bool visitUnitStart(const UnitPtr&) override;
        void visitUnitEnd(const UnitPtr&) override;
        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        void visitClassDecl(const ClassDeclPtr&) override;
        bool visitClassDefStart(const ClassDefPtr&) override;
        void visitInterfaceDecl(const InterfaceDeclPtr&) override;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) override;
        bool visitExceptionStart(const ExceptionPtr&) override;
        void visitOperation(const OperationPtr&) override;

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
    };

    class TypesVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        TypesVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        bool visitClassDefStart(const ClassDefPtr&) override { return false; }
        bool visitExceptionStart(const ExceptionPtr&) override;
        void visitExceptionEnd(const ExceptionPtr&) override;
        bool visitStructStart(const StructPtr&) override;
        void visitStructEnd(const StructPtr&) override;
        void visitSequence(const SequencePtr&) override;
        void visitDictionary(const DictionaryPtr&) override;
        void visitEnum(const EnumPtr&) override;
        void visitConst(const ConstPtr&) override;
        void visitDataMember(const MemberPtr&) override;

    private:

        void emitUpcall(const ExceptionPtr&, const std::string&, const std::string&);

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
        std::string _dllClassExport;
        std::string _dllMemberExport;
        bool _doneStaticSymbol;
        int _useWstring;
        std::list<int> _useWstringHist;
    };

    class ProxyVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        ProxyVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        bool visitUnitStart(const UnitPtr&) override;
        void visitUnitEnd(const UnitPtr&) override;
        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) override;
        void visitInterfaceDefEnd(const InterfaceDefPtr&) override;
        void visitOperation(const OperationPtr&) override;

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllClassExport;
        std::string _dllMemberExport;
        int _useWstring;
        std::list<int> _useWstringHist;
    };

    // TODO: merge with ValueVisitor
    class ObjectVisitor : public ParserVisitor
    {
    public:

        ObjectVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

    protected:

        bool emitVirtualBaseInitializers(const ClassDefPtr&, const ClassDefPtr&);
        void emitOneShotConstructor(const ClassDefPtr&);
        void emitDataMember(const MemberPtr&);

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
        std::string _dllClassExport;
        std::string _dllMemberExport;
        bool _doneStaticSymbol;
        int _useWstring;
        std::list<int> _useWstringHist;
    };

    class InterfaceVisitor : private ::IceUtil::noncopyable, public ObjectVisitor
    {
    public:

        InterfaceVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) override;
        void visitInterfaceDefEnd(const InterfaceDefPtr&) override;
        void visitOperation(const OperationPtr&) override;
        void emitUpcall(const InterfaceDefPtr&, const std::string&, const std::string&);
    };

    class ValueVisitor : private ::IceUtil::noncopyable, public ObjectVisitor
    {
    public:

        ValueVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        bool visitClassDefStart(const ClassDefPtr&) override;
        void visitClassDefEnd(const ClassDefPtr&) override;
        void emitUpcall(const ClassDefPtr&, const std::string&, const std::string&);
    };

    class StreamVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        StreamVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        bool visitStructStart(const StructPtr&) override;
        bool visitClassDefStart(const ClassDefPtr&) override;
        void visitExceptionEnd(const ExceptionPtr&) override;
        void visitEnum(const EnumPtr&) override;

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;
        std::string _dllExport;
    };

    class CompatibilityVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        CompatibilityVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        void visitClassDecl(const ClassDeclPtr&) override;
        void visitInterfaceDecl(const InterfaceDeclPtr&) override;

    private:

        ::IceUtilInternal::Output& H;
        std::string _dllExport;
    };

    class ImplVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        ImplVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) override;

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
        int _useWstring;
        std::list<int> _useWstringHist;

        //
        // Get the default value returned for a type
        //
        std::string defaultValue(const TypePtr&, const std::string&, const StringList&) const;
    };

    class MetadataVisitor : public ParserVisitor
    {
    public:

        bool visitUnitStart(const UnitPtr&) override;
        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        void visitClassDecl(const ClassDeclPtr&) override;
        bool visitClassDefStart(const ClassDefPtr&) override;
        void visitClassDefEnd(const ClassDefPtr&) override;
        void visitInterfaceDecl(const InterfaceDeclPtr&) override;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) override;
        void visitInterfaceDefEnd(const InterfaceDefPtr&) override;
        bool visitExceptionStart(const ExceptionPtr&) override;
        void visitExceptionEnd(const ExceptionPtr&) override;
        bool visitStructStart(const StructPtr&) override;
        void visitStructEnd(const StructPtr&) override;
        void visitOperation(const OperationPtr&) override;
        void visitDataMember(const MemberPtr&) override;
        void visitSequence(const SequencePtr&) override;
        void visitDictionary(const DictionaryPtr&) override;
        void visitEnum(const EnumPtr&) override;
        void visitConst(const ConstPtr&) override;

    private:

        StringList validate(const SyntaxTreeBasePtr&, const StringList&, const std::string&, int, bool = false);
    };

    static void validateMetadata(const UnitPtr&);
};

}

#endif
