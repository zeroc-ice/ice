//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef GEN_H
#define GEN_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

class Gen
{
public:

    Gen(const std::string&,
        const std::string&,
        const std::string&,
        const std::vector<std::string>&,
        const std::string&,
        const std::vector<std::string>&,
        const std::string&,
        const std::string&);
    ~Gen();

    Gen(const Gen&) = delete;

    void generate(const UnitPtr&);
    void closeOutput();

    static int setUseWstring(ContainedPtr, std::list<int>&, int);
    static int resetUseWstring(std::list<int>&);

private:

    void writeExtraHeaders(::IceUtilInternal::Output&);

    //
    // Returns the header extension defined in the file metadata for a given file,
    // or an empty string if no file metadata was found.
    //
    std::string getHeaderExt(const std::string& file, const UnitPtr& unit);

    //
    // Returns the source extension defined in the file metadata for a given file,
    // or an empty string if no file metadata was found.
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

    // Visitors

    // Generates forward declarations for classes, proxies and structs. Also generates using for sequences and
    // dictionaries, enum definitions and constants.
    class ForwardDeclVisitor final : public ParserVisitor
    {
    public:

        ForwardDeclVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&);
        ForwardDeclVisitor(const ForwardDeclVisitor&) = delete;

        bool visitUnitStart(const UnitPtr&) final;
        void visitUnitEnd(const UnitPtr&) final;
        bool visitModuleStart(const ModulePtr&) final;
        void visitModuleEnd(const ModulePtr&) final;
        void visitClassDecl(const ClassDeclPtr&) final;
        bool visitClassDefStart(const ClassDefPtr&) final; // TODO: move
        bool visitStructStart(const StructPtr&) final;
        void visitInterfaceDecl(const InterfaceDeclPtr&) final;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) final; // TODO: move
        bool visitExceptionStart(const ExceptionPtr&) final;
        void visitOperation(const OperationPtr&) final;
        void visitSequence(const SequencePtr&);
        void visitDictionary(const DictionaryPtr&);
        void visitEnum(const EnumPtr&);
        void visitConst(const ConstPtr&);

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C; // TODO: probably not needed

        int _useWstring;
        std::list<int> _useWstringHist;
    };

    // The second code-generation visitor. We need to generate the proxies early because fields (in structs, classes
    // and exceptions) can use fields and these fields must see complete proxy types.
    class ProxyVisitor : public ParserVisitor
    {
    public:

        ProxyVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);
        ProxyVisitor(const ProxyVisitor&) = delete;

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
        virtual void visitInterfaceDefEnd(const InterfaceDefPtr&);
        virtual void visitOperation(const OperationPtr&);

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
        int _useWstring;
        std::list<int> _useWstringHist;
    };

    class StructVisitor final : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        StructVisitor(::IceUtilInternal::Output&);

        bool visitModuleStart(const ModulePtr&) final;
        void visitModuleEnd(const ModulePtr&) final;
        bool visitStructStart(const StructPtr&) final;
        void visitStructEnd(const StructPtr&) final;
        void visitDataMember(const DataMemberPtr&) final;

        // Otherwise we visit their data members.
        bool visitClassDefStart(const ClassDefPtr&) final { return false; }
        bool visitExceptionStart(const ExceptionPtr&) final { return false; }

    private:

        ::IceUtilInternal::Output& H;
        int _useWstring;
        std::list<int> _useWstringHist;
    };

    class ExceptionVisitor final : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:
        ExceptionVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        bool visitModuleStart(const ModulePtr&) final;
        void visitModuleEnd(const ModulePtr&) final;
        bool visitExceptionStart(const ExceptionPtr&) final;
        void visitExceptionEnd(const ExceptionPtr&) final;
        void visitDataMember(const DataMemberPtr&) final;

        // Otherwise we visit their data members.
        bool visitClassDefStart(const ClassDefPtr&) final { return false; }
        bool visitStructStart(const StructPtr&) final { return false; }

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
        std::string _dllClassExport;
        std::string _dllMemberExport;
        bool _doneStaticSymbol;
        int _useWstring;
        std::list<int> _useWstringHist;
    };

    class InterfaceVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        InterfaceVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
        virtual void visitInterfaceDefEnd(const InterfaceDefPtr&);
        virtual void visitOperation(const OperationPtr&);

    private:

        ::IceUtilInternal::Output& H;
        ::IceUtilInternal::Output& C;

        std::string _dllExport;
        int _useWstring;
        std::list<int> _useWstringHist;
    };

    class ValueVisitor final : public ParserVisitor
    {
    public:

        ValueVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);
        ValueVisitor(const ValueVisitor&) = delete;

        bool visitModuleStart(const ModulePtr&) final;
        void visitModuleEnd(const ModulePtr&) final;
        bool visitClassDefStart(const ClassDefPtr&) final;
        void visitClassDefEnd(const ClassDefPtr&) final;

    private:

        bool emitBaseInitializers(const ClassDefPtr&);
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

    // Generates StreamHelper template specializations for enums, structs, classes and exceptions.
    class StreamVisitor final : public ParserVisitor
    {
    public:

        StreamVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);
        StreamVisitor(const StreamVisitor&) = delete;

        bool visitModuleStart(const ModulePtr&) final;
        void visitModuleEnd(const ModulePtr&) final;
        bool visitStructStart(const StructPtr&) final;
        bool visitClassDefStart(const ClassDefPtr&) final;
        bool visitExceptionStart(const ExceptionPtr&) final;
        void visitExceptionEnd(const ExceptionPtr&) final;
        void visitEnum(const EnumPtr&);

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
