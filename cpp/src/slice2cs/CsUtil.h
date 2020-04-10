//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef CS_UTIL_H
#define CS_UTIL_H

#include <Slice/Parser.h>
#include <Slice/Util.h>
#include <IceUtil/OutputUtil.h>
#include <functional>

namespace Slice
{

enum CSharpBaseType { ObjectType=1, ExceptionType=2 };

std::string fixId(const std::string&, unsigned int = 0);
//
// Returns the namespace of a Contained entity.
//
std::string getNamespace(const ContainedPtr&);
//
// Returns the namespace prefix of a Contained entity.
//
std::string getNamespacePrefix(const ContainedPtr&);

std::string getUnqualified(const std::string&, const std::string&, bool builtin = false);
std::string getUnqualified(const ContainedPtr&,
                           const std::string& package = "",
                           const std::string& prefix = "",
                           const std::string& suffix = "");

struct ParamInfo
{
    std::string name;
    TypePtr type;
    std::string typeStr;
    bool nullable;
    bool tagged;
    int tag;
    ParamDeclPtr param; // 0 == return value
    OperationPtr operation;

    ParamInfo(const OperationPtr& operation, const std::string& name, const TypePtr& type, bool tagged, int tag,
              const std::string& prefix = "");
    ParamInfo(const ParamDeclPtr& param, const std::string& prefix = "");
};

bool normalizeCase(const ContainedPtr&);
std::string operationName(const OperationPtr&);
std::string paramName(const ParamInfo&);
std::string interfaceName(const ProxyPtr&);
std::string interfaceName(const ClassDefPtr&);
std::string dataMemberName(const ParamInfo&);
std::string dataMemberName(const DataMemberPtr&);

std::string helperName(const TypePtr&, const std::string&);

std::string returnValueName(const ParamDeclList&);
std::string resultType(const OperationPtr&, const std::string&, bool);
std::string resultTask(const OperationPtr&, const std::string&, bool);

bool isNullable(const TypePtr&);
bool isCollectionType(const TypePtr&);
bool isProxyType(const TypePtr&);
bool isClassType(const TypePtr&);
bool isValueType(const TypePtr&);
bool isReferenceType(const TypePtr&);

std::list<ParamInfo> getAllInParams(const OperationPtr&, const std::string& prefix = "");
void getInParams(const OperationPtr&, std::list<ParamInfo>&, std::list<ParamInfo>&, const std::string& prefix = "");

std::list<ParamInfo> getAllOutParams(const OperationPtr&, const std::string& prefix = "",
                                     bool returnTypeIsFirst = false);
void getOutParams(const OperationPtr&, std::list<ParamInfo>&, std::list<ParamInfo>&, const std::string& prefix = "");

std::vector<std::string> getNames(const std::list<ParamInfo>& params, std::string prefix = "");
std::vector<std::string> getNames(const std::list<ParamInfo>& params, std::function<std::string (const ParamInfo&)>);

std::string toTuple(const std::list<ParamInfo>& params, const std::string& = "");
std::string toTupleType(const std::list<ParamInfo>& params, const std::string& = "");

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
    // Validate all metadata in the unit with a "cs:" prefix.
    //
    static void validateMetaData(const UnitPtr&);

    static std::string typeToString(const TypePtr& type, const std::string& package, bool optional = false,
                                    bool readOnly = false);

protected:

    static std::string getCustomTypeIdNamespace(const UnitPtr&);

    static std::string getTagFormat(const TypePtr&, const std::string&);

    //
    // Generate code to marshal or unmarshal a type
    //
    std::string outputStreamWriter(const TypePtr&, const std::string&);
    void writeMarshalCode(::IceUtilInternal::Output&, const TypePtr&, const std::string&, const std::string&,
                          const std::string& = "ostr");

    std::string inputStreamReader(const TypePtr&, const std::string&);
    void writeUnmarshalCode(::IceUtilInternal::Output&, const TypePtr&, const std::string&, const std::string&,
                            const std::string& = "istr");

    void writeTaggedMarshalCode(::IceUtilInternal::Output&, const TypePtr&, const std::string&, const std::string&,
                                  int, const std::string& = "ostr");
    void writeTaggedUnmarshalCode(::IceUtilInternal::Output&, const TypePtr&, const std::string&, const std::string&,
                                    int, const std::string& = "istr");

    std::string sequenceMarshalCode(const SequencePtr&, const std::string&, const std::string&, const std::string&);
    std::string sequenceUnmarshalCode(const SequencePtr&, const std::string&, const std::string&);

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
