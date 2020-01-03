//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef CS_UTIL_H
#define CS_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>
#include <functional>

namespace Slice
{

enum CSharpBaseType { ObjectType=1, ExceptionType=2 };

std::string marshaledResultStructName(const std::string&, const std::string&);
std::string returnValueName(const ParamDeclList&);
std::string resultType(const OperationPtr&, const std::string&, bool);
std::string resultTask(const OperationPtr&, const std::string&, bool);

bool isNullable(const TypePtr&);
bool isCollectionType(const TypePtr&);
bool isProxyType(const TypePtr&);
bool isClassType(const TypePtr&);
bool isValueType(const TypePtr&);
bool isImmutableType(const TypePtr&);
bool isReferenceType(const TypePtr&);

struct ParamInfo
{
    std::string name;
    TypePtr type;
    std::string typeStr;
    bool nullable;
    bool optional;
    int tag;
    ParamDeclPtr param; // 0 == return value

    ParamInfo(const std::string& name, const TypePtr& type, bool optional, int tag, const std::string& prefix = "");
    ParamInfo(const ParamDeclPtr& param, const std::string& prefix = "");
};

std::list<ParamInfo> getAllInParams(const OperationPtr&, const std::string& prefix = "");
void getInParams(const OperationPtr&, std::list<ParamInfo>&, std::list<ParamInfo>&, const std::string& prefix = "");

std::list<ParamInfo> getAllOutParams(const OperationPtr&, const std::string& prefix = "",
                                     bool returnTypeIsFirst = false);
void getOutParams(const OperationPtr&, std::list<ParamInfo>&, std::list<ParamInfo>&, const std::string& prefix = "");

std::vector<std::string> getNames(const std::list<ParamInfo>& params, std::string prefix = "");
std::vector<std::string> getNames(const std::list<ParamInfo>& params, std::function<std::string (const ParamInfo&)>);

template<typename T> inline std::vector<std::string>
mapfn(const std::list<T>& items, std::function<std::string (const T&)> fn)
{
    std::vector<std::string> result;
    for(const auto& item : items)
    {
        result.push_back(fn(item));
    }
    return result;
}

class CsGenerator : private ::IceUtil::noncopyable
{
public:

    virtual ~CsGenerator() {};

    //
    // Convert a dimension-less array declaration to one with a dimension.
    //
    static std::string toArrayAlloc(const std::string& decl, const std::string& sz);

    //
    // Validate all metadata in the unit with a "cs:" prefix.
    //
    static void validateMetaData(const UnitPtr&);

    //
    // Returns the namespace of a Contained entity.
    //
    static std::string getNamespace(const ContainedPtr&);

    static std::string getUnqualified(const std::string&, const std::string&, bool builtin = false);
    static std::string getUnqualified(const ContainedPtr&,
                                      const std::string& package = "",
                                      const std::string& prefix = "",
                                      const std::string& suffix = "");
    static std::string typeToString(const TypePtr&, const std::string&, bool = false);
    static std::string fixId(const std::string&, unsigned int = 0);

protected:

    //
    // Returns the namespace prefix of a Contained entity.
    //
    static std::string getNamespacePrefix(const ContainedPtr&);
    static std::string getCustomTypeIdNamespace(const UnitPtr&);

    static std::string getOptionalFormat(const TypePtr&, const std::string&);
    static std::string getStaticId(const TypePtr&);

    //
    // Generate code to marshal or unmarshal a type
    //
    void writeMarshalCode(::IceUtilInternal::Output&, const TypePtr&, const std::string&, const std::string&,
                          const std::string& = "ostr");
    void writeUnmarshalCode(::IceUtilInternal::Output&, const TypePtr&, const std::string&, const std::string&,
                            const std::string& = "istr");

    void writeOptionalMarshalCode(::IceUtilInternal::Output&, const TypePtr&, const std::string&, const std::string&,
                                  int, const std::string& = "ostr");
    void writeOptionalUnmarshalCode(::IceUtilInternal::Output&, const TypePtr&, const std::string&, const std::string&,
                                    int, const std::string& = "istr");
    void writeSequenceMarshalUnmarshalCode(::IceUtilInternal::Output&, const SequencePtr&, const std::string&,
                                           const std::string&, bool, bool, const std::string& = "");
    void writeOptionalSequenceMarshalUnmarshalCode(::IceUtilInternal::Output&, const SequencePtr&, const std::string&,
                                                   const std::string&, int, bool, const std::string& = "");
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
        virtual void visitParamDecl(const ParamDeclPtr&);
        virtual void visitDataMember(const DataMemberPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual void visitEnum(const EnumPtr&);
        virtual void visitConst(const ConstPtr&);

    private:

        void validate(const ContainedPtr&);

        std::string _fileName;
    };
};

}

#endif
