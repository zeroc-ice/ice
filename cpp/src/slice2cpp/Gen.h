// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
    void generateChecksumMap(const UnitPtr&);
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
    bool _implCpp98;
    bool _implCpp11;
    bool _checksum;
    bool _ice;

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

        void emitUpcall(const ExceptionPtr&, const std::string&, const std::string&, bool = false);

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
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
        std::string _dllClassExport;
        std::string _dllMemberExport;
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

        ObjectVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

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
        void emitUpcall(const ClassDefPtr&, const std::string&, const std::string&);

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
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
        // Get the default value returned for a type
        //
        std::string defaultValue(const TypePtr&, const std::string&, const StringList&) const;
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
        ::IceUtilInternal::Output& C;

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
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual void visitEnum(const EnumPtr&);

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;
        std::string _dllExport;
    };

    //
    // C++11 Visitors
    //

    class Cpp11DeclVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        Cpp11DeclVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitUnitStart(const UnitPtr&);
        virtual void visitUnitEnd(const UnitPtr&);
        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual void visitClassDecl(const ClassDeclPtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual void visitOperation(const OperationPtr&);

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
    };

    class Cpp11TypesVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        Cpp11TypesVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

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

        void emitUpcall(const ExceptionPtr&, const std::string&, const std::string&, bool = false);

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
        std::string _dllClassExport;
        std::string _dllMemberExport;
        bool _doneStaticSymbol;
        int _useWstring;
        std::list<int> _useWstringHist;
    };

    class Cpp11ProxyVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        Cpp11ProxyVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

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

        std::string _dllClassExport;
        std::string _dllMemberExport;
        int _useWstring;
        std::list<int> _useWstringHist;
    };

    class Cpp11ObjectVisitor : public ParserVisitor
    {
    public:

        Cpp11ObjectVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

    protected:

        bool emitVirtualBaseInitializers(const ClassDefPtr&, const ClassDefPtr&);
        void emitOneShotConstructor(const ClassDefPtr&);
        void emitDataMember(const DataMemberPtr&);

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
        std::string _dllClassExport;
        std::string _dllMemberExport;
        bool _doneStaticSymbol;
        int _useWstring;
        std::list<int> _useWstringHist;
    };

    class Cpp11LocalObjectVisitor : private ::IceUtil::noncopyable, public Cpp11ObjectVisitor
    {
    public:

        Cpp11LocalObjectVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitOperation(const OperationPtr&);
    };

    class Cpp11InterfaceVisitor : private ::IceUtil::noncopyable, public Cpp11ObjectVisitor
    {
    public:

        Cpp11InterfaceVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitOperation(const OperationPtr&);
        void emitUpcall(const ClassDefPtr&, const std::string&, const std::string&);
    };

    class Cpp11ValueVisitor : private ::IceUtil::noncopyable, public Cpp11ObjectVisitor
    {
    public:

        Cpp11ValueVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitOperation(const OperationPtr&);
        void emitUpcall(const ClassDefPtr&, const std::string&, const std::string&);
    };

    class Cpp11StreamVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        Cpp11StreamVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitExceptionEnd(const ExceptionPtr&);
        virtual void visitEnum(const EnumPtr&);

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;
        std::string _dllExport;
    };

    class Cpp11CompatibilityVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        Cpp11CompatibilityVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual void visitClassDecl(const ClassDeclPtr&);

    private:

        ::IceUtilInternal::Output& H;
        std::string _dllExport;
    };

    class Cpp11ImplVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        Cpp11ImplVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

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
        // Get the default value returned for a type
        //
        std::string defaultValue(const TypePtr&, const std::string&, const StringList&) const;
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

        StringList validate(const SyntaxTreeBasePtr&, const StringList&, const std::string&, const std::string&,
                            bool = false);
    };

    class NormalizeMetaDataVisitor : public ParserVisitor
    {
    public:

        NormalizeMetaDataVisitor(bool);

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

        StringList normalize(const StringList&);

        bool _cpp11;
    };

    static void validateMetaData(const UnitPtr&);
    static void normalizeMetaData(const UnitPtr&, bool);
};

}

#endif
