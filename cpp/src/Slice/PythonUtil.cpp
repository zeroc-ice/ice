// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/PythonUtil.h>
#include <Slice/Checksum.h>
#include <Slice/Util.h>
#include <IceUtil/IceUtil.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/InputUtil.h>
#include <IceUtil/Unicode.h>
#include <climits>
#include <iterator>

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace Slice
{
namespace Python
{

class MetaDataVisitor : public ParserVisitor
{
public:

    virtual bool visitUnitStart(const UnitPtr&);
    virtual bool visitModuleStart(const ModulePtr&);
    virtual void visitClassDecl(const ClassDeclPtr&);
    virtual bool visitClassDefStart(const ClassDefPtr&);
    virtual bool visitExceptionStart(const ExceptionPtr&);
    virtual bool visitStructStart(const StructPtr&);
    virtual void visitOperation(const OperationPtr&);
    virtual void visitDataMember(const DataMemberPtr&);
    virtual void visitSequence(const SequencePtr&);
    virtual void visitDictionary(const DictionaryPtr&);
    virtual void visitEnum(const EnumPtr&);
    virtual void visitConst(const ConstPtr&);

private:

    //
    // Validates sequence metadata.
    //
    void validateSequence(const string&, const string&, const TypePtr&, const StringList&);

    //
    // Checks a definition that doesn't currently support Python metadata.
    //
    void reject(const ContainedPtr&);

    StringSet _history;
};

//
// ModuleVisitor finds all of the Slice modules whose include level is greater
// than 0 and emits a statement of the following form:
//
// _M_Foo = Ice.openModule('Foo')
//
// This statement allows the code generated for this translation unit to refer
// to types residing in those included modules.
//
class ModuleVisitor : public ParserVisitor
{
public:

    ModuleVisitor(Output&, set<string>&);

    virtual bool visitModuleStart(const ModulePtr&);

private:

    Output& _out;
    set<string>& _history;
};

//
// CodeVisitor generates the Python mapping for a translation unit.
//
class CodeVisitor : public ParserVisitor
{
public:

    CodeVisitor(IceUtilInternal::Output&, set<string>&);

    virtual bool visitModuleStart(const ModulePtr&);
    virtual void visitModuleEnd(const ModulePtr&);
    virtual void visitClassDecl(const ClassDeclPtr&);
    virtual bool visitClassDefStart(const ClassDefPtr&);
    virtual bool visitExceptionStart(const ExceptionPtr&);
    virtual bool visitStructStart(const StructPtr&);
    virtual void visitSequence(const SequencePtr&);
    virtual void visitDictionary(const DictionaryPtr&);
    virtual void visitEnum(const EnumPtr&);
    virtual void visitConst(const ConstPtr&);

private:

    //
    // Return a Python symbol for the given parser element.
    //
    string getSymbol(const ContainedPtr&, const string& = string());

    //
    // Emit Python code to assign the given symbol in the current module.
    //
    void registerName(const string&);

    //
    // Emit the tuple for a Slice type.
    //
    void writeType(const TypePtr&);

    //
    // Write an initializer value for a given type.
    //
    void writeInitializer(const DataMemberPtr&);

    //
    // Add a value to a hash code.
    //
    void writeHash(const string&, const TypePtr&, int&);

    //
    // Write Python metadata as a tuple.
    //
    void writeMetaData(const StringList&);

    //
    // Convert an operation mode into a string.
    //
    string getOperationMode(Slice::Operation::Mode);

    struct MemberInfo
    {
        string fixedName;
        bool inherited;
        DataMemberPtr dataMember;
    };
    typedef list<MemberInfo> MemberInfoList;

    //
    // Write a member assignment statement for a constructor.
    //
    void writeAssign(const MemberInfo&);

    //
    // Write a constant value.
    //
    void writeConstantValue(const TypePtr&, const SyntaxTreeBasePtr&, const string&);

    //
    // Write constructor parameters with default values.
    //
    void writeConstructorParams(const MemberInfoList&);

    void collectClassMembers(const ClassDefPtr&, MemberInfoList&, bool);
    void collectExceptionMembers(const ExceptionPtr&, MemberInfoList&, bool);

    typedef vector<string> StringVec;

    StringVec stripMarkup(const string&);

    void writeDocstring(const string&, const string& = "");
    void writeDocstring(const string&, const DataMemberList&);
    void writeDocstring(const string&, const EnumeratorList&);

    typedef map<string, string> StringMap;
    struct OpComment
    {
        StringVec description;
        StringMap params;
        string returns;
        StringMap exceptions;
    };
    bool parseOpComment(const string&, OpComment&);

    enum DocstringMode { DocSync, DocAsyncBegin, DocAsyncEnd, DocDispatch, DocAsyncDispatch };

    void writeDocstring(const OperationPtr&, DocstringMode, bool);

    Output& _out;
    set<string>& _moduleHistory;
    list<string> _moduleStack;
    set<string> _classHistory;
};

}
}

string
u32CodePoint(unsigned int value)
{
    ostringstream s;
    s << "\\U";
    s << hex;
    s.width(8);
    s.fill('0');
    s << value;
    return s.str();
}

void
writeU8Buffer(const vector<unsigned char>& u8buffer, ostringstream& out)
{
    vector<unsigned int> u32buffer;
    IceUtilInternal::ConversionResult result = convertUTF8ToUTF32(u8buffer, u32buffer, IceUtil::lenientConversion);
    switch(result)
    {
        case conversionOK:
            break;
        case sourceExhausted:
            throw IceUtil::IllegalConversionException(__FILE__, __LINE__, "string source exhausted");
        case sourceIllegal:
            throw IceUtil::IllegalConversionException(__FILE__, __LINE__, "string source illegal");
        default:
        {
            assert(0);
            throw IceUtil::IllegalConversionException(__FILE__, __LINE__);
        }
    }

    for(vector<unsigned int>::const_iterator c = u32buffer.begin(); c != u32buffer.end(); ++c)
    {
        out << u32CodePoint(*c);
    }
}

static string
lookupKwd(const string& name)
{
    //
    // Keyword list. *Must* be kept in alphabetical order.
    //
    static const string keywordList[] =
    {
        "None", "and", "assert", "break", "class", "continue", "def", "del", "elif", "else", "except", "exec",
        "finally", "for", "from", "global", "if", "import", "in", "is", "lambda", "not", "or", "pass",
        "print", "raise", "return", "self", "try", "while", "yield"
    };
    bool found =  binary_search(&keywordList[0],
                                &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                                name);
    return found ? "_" + name : name;
}

//
// Split a scoped name into its components and return the components as a list of (unscoped) identifiers.
//
static vector<string>
splitScopedName(const string& scoped)
{
    assert(scoped[0] == ':');
    vector<string> ids;
    string::size_type next = 0;
    string::size_type pos;
    while((pos = scoped.find("::", next)) != string::npos)
    {
        pos += 2;
        if(pos != scoped.size())
        {
            string::size_type endpos = scoped.find("::", pos);
            if(endpos != string::npos)
            {
                ids.push_back(scoped.substr(pos, endpos - pos));
            }
        }
        next = pos;
    }
    if(next != scoped.size())
    {
        ids.push_back(scoped.substr(next));
    }
    else
    {
        ids.push_back("");
    }

    return ids;
}

static string
getDictLookup(const ContainedPtr& cont, const string& suffix = string())
{
    string scope = Slice::Python::scopedToName(cont->scope());
    assert(!scope.empty());

    string package = Slice::Python::getPackageMetadata(cont);
    if(!package.empty())
    {
        scope = package + "." + scope;
    }

    return "'" + suffix + Slice::Python::fixIdent(cont->name()) + "' not in _M_" + scope + "__dict__";
}

//
// ModuleVisitor implementation.
//
Slice::Python::ModuleVisitor::ModuleVisitor(Output& out, set<string>& history) :
    _out(out), _history(history)
{
}

bool
Slice::Python::ModuleVisitor::visitModuleStart(const ModulePtr& p)
{
    if(p->includeLevel() > 0)
    {
        string abs = getAbsolute(p);
        if(_history.count(abs) == 0)
        {
            //
            // If this is a top-level module, then we check if it has package metadata.
            // If so, we need to emit statements to open each of the modules in the
            // package before we can open this module.
            //
            if(UnitPtr::dynamicCast(p->container()))
            {
                string pkg = getPackageMetadata(p);
                if(!pkg.empty())
                {
                    vector<string> v;
                    splitString(pkg, ".", v);
                    string mod;
                    for(vector<string>::iterator q = v.begin(); q != v.end(); ++q)
                    {
                        mod = mod.empty() ? *q : mod + "." + *q;
                        if(_history.count(mod) == 0)
                        {
                            _out << nl << "_M_" << mod << " = Ice.openModule('" << mod << "')";
                            _history.insert(mod);
                        }
                    }
                }
            }

            _out << sp << nl << "# Included module " << abs;
            _out << nl << "_M_" << abs << " = Ice.openModule('" << abs << "')";
            _history.insert(abs);
        }
    }

    return true;
}

//
// CodeVisitor implementation.
//
Slice::Python::CodeVisitor::CodeVisitor(Output& out, set<string>& moduleHistory) :
    _out(out), _moduleHistory(moduleHistory)
{
}

bool
Slice::Python::CodeVisitor::visitModuleStart(const ModulePtr& p)
{
    //
    // As each module is opened, we emit the statement
    //
    // __name__ = 'Foo'
    //
    // This renames the current module to 'Foo' so that subsequent
    // type definitions have the proper fully-qualified name.
    //
    // We also emit the statement
    //
    // _M_Foo = Ice.openModule('Foo')
    //
    // This allows us to create types in the module Foo.
    //
    string abs = getAbsolute(p);
    _out << sp << nl << "# Start of module " << abs;
    if(_moduleHistory.count(abs) == 0) // Don't emit this more than once for each module.
    {
        //
        // If this is a top-level module, then we check if it has package metadata.
        // If so, we need to emit statements to open each of the modules in the
        // package before we can open this module.
        //
        if(UnitPtr::dynamicCast(p->container()))
        {
            string pkg = getPackageMetadata(p);
            if(!pkg.empty())
            {
                vector<string> v;
                splitString(pkg, ".", v);
                string mod;
                for(vector<string>::iterator q = v.begin(); q != v.end(); ++q)
                {
                    mod = mod.empty() ? *q : mod + "." + *q;
                    if(_moduleHistory.count(mod) == 0) // Don't emit this more than once for each module.
                    {
                        _out << nl << "_M_" << mod << " = Ice.openModule('" << mod << "')";
                        _moduleHistory.insert(mod);
                    }
                }
            }
        }
        _out << nl << "_M_" << abs << " = Ice.openModule('" << abs << "')";
        _moduleHistory.insert(abs);
    }
    _out << nl << "__name__ = '" << abs << "'";

    writeDocstring(p->comment(), "_M_" + abs + ".__doc__ = ");

    _moduleStack.push_front(abs);
    return true;
}

void
Slice::Python::CodeVisitor::visitModuleEnd(const ModulePtr&)
{
    assert(!_moduleStack.empty());
    _out << sp << nl << "# End of module " << _moduleStack.front();
    _moduleStack.pop_front();

    if(!_moduleStack.empty())
    {
        _out << sp << nl << "__name__ = '" << _moduleStack.front() << "'";
    }
}

void
Slice::Python::CodeVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    //
    // Emit forward declarations.
    //
    string scoped = p->scoped();
    if(_classHistory.count(scoped) == 0)
    {
        _out << sp << nl << "if " << getDictLookup(p) << ':';
        _out.inc();
        string type = getAbsolute(p, "_t_");
        _out << nl << "_M_" << type << " = IcePy.declareClass('" << scoped << "')";
        if(!p->isLocal())
        {
            _out << nl << "_M_" << getAbsolute(p, "_t_", "Prx") << " = IcePy.declareProxy('" << scoped << "')";
        }
        _out.dec();
        _classHistory.insert(scoped); // Avoid redundant declarations.
    }
}

bool
Slice::Python::CodeVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string scoped = p->scoped();
    string type = getAbsolute(p, "_t_");
    string abs = getAbsolute(p);
    string name = fixIdent(p->name());
    string prxAbs = getAbsolute(p, "", "Prx");
    string prxName = fixIdent(p->name() + "Prx");
    string prxType = getAbsolute(p, "_t_", "Prx");
    ClassList bases = p->bases();
    ClassDefPtr base;
    OperationList ops = p->operations();
    bool isAbstract = p->isInterface() || p->allOperations().size() > 0; // Don't use isAbstract() - see bug 3739

    //
    // Define the class.
    //
    _out << sp << nl << "if " << getDictLookup(p) << ':';
    _out.inc();
    _out << nl << "_M_" << abs << " = Ice.createTempClass()";
    _out << nl << "class " << name << '(';
    if(bases.empty())
    {
        if(p->isLocal())
        {
            _out << "object";
        }
        else
        {
            _out << "Ice.Object";
        }
    }
    else
    {
        for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
        {
            if(q != bases.begin())
            {
                _out << ", ";
            }
            _out << getSymbol(*q);
        }
        if(!bases.front()->isInterface())
        {
            base = bases.front();
        }
    }
    _out << "):";

    _out.inc();

    writeDocstring(p->comment(), p->dataMembers());

    //
    // __init__
    //
    _out << nl << "def __init__(self";
    MemberInfoList allMembers;
    collectClassMembers(p, allMembers, false);
    writeConstructorParams(allMembers);
    _out << "):";
    _out.inc();
    if(!base && !p->hasDataMembers() && !isAbstract)
    {
        _out << nl << "pass";
    }
    else
    {
        if(isAbstract)
        {
            _out << nl << "if Ice.getType(self) == _M_" << abs << ':';
            _out.inc();
            _out << nl << "raise RuntimeError('" << abs << " is an abstract class')";
            _out.dec();
        }
        if(base)
        {
            _out << nl << getSymbol(base) << ".__init__(self";
            for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
            {
                if(q->inherited)
                {
                    _out << ", " << q->fixedName;
                }
            }
            _out << ')';
        }
        for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
        {
            if(!q->inherited)
            {
                writeAssign(*q);
            }
        }
    }
    _out.dec();

    if(!p->isLocal())
    {
        //
        // ice_ids
        //
        ClassList allBases = p->allBases();
        StringList ids;
        transform(allBases.begin(), allBases.end(), back_inserter(ids), IceUtil::constMemFun(&Contained::scoped));
        StringList other;
        other.push_back(scoped);
        other.push_back("::Ice::Object");
        other.sort();
        ids.merge(other);
        ids.unique();
        _out << sp << nl << "def ice_ids(self, current=None):";
        _out.inc();
        _out << nl << "return (";
        for(StringList::iterator q = ids.begin(); q != ids.end(); ++q)
        {
            if(q != ids.begin())
            {
                _out << ", ";
            }
            _out << "'" << *q << "'";
        }
        _out << ')';
        _out.dec();

        //
        // ice_id
        //
        _out << sp << nl << "def ice_id(self, current=None):";
        _out.inc();
        _out << nl << "return '" << scoped << "'";
        _out.dec();

        //
        // ice_staticId
        //
        _out << sp << nl << "def ice_staticId():";
        _out.inc();
        _out << nl << "return '" << scoped << "'";
        _out.dec();
        _out << nl << "ice_staticId = staticmethod(ice_staticId)";
    }

    if(!ops.empty())
    {
        //
        // Emit a placeholder for each operation.
        //
        for(OperationList::iterator oli = ops.begin(); oli != ops.end(); ++oli)
        {
            string fixedOpName = fixIdent((*oli)->name());
            if(!p->isLocal() && (p->hasMetaData("amd") || (*oli)->hasMetaData("amd")))
            {
                _out << sp << nl << "def " << (*oli)->name() << "_async(self, _cb";

                ParamDeclList params = (*oli)->parameters();

                for(ParamDeclList::iterator pli = params.begin(); pli != params.end(); ++pli)
                {
                    if(!(*pli)->isOutParam())
                    {
                        _out << ", " << fixIdent((*pli)->name());
                    }
                }
                if(!p->isLocal())
                {
                    _out << ", current=None";
                }
                _out << "):";
                _out.inc();

                writeDocstring(*oli, DocAsyncDispatch, false);

                _out << nl << "pass";
                _out.dec();
            }
            else
            {
                _out << sp << nl << "def " << fixedOpName << "(self";

                ParamDeclList params = (*oli)->parameters();

                for(ParamDeclList::iterator pli = params.begin(); pli != params.end(); ++pli)
                {
                    if(!(*pli)->isOutParam())
                    {
                        _out << ", " << fixIdent((*pli)->name());
                    }
                }
                if(!p->isLocal())
                {
                    _out << ", current=None";
                }
                _out << "):";
                _out.inc();
                writeDocstring(*oli, DocDispatch, p->isLocal());
                _out << nl << "pass";
                _out.dec();
            }
        }
    }

    //
    // __str__
    //
    _out << sp << nl << "def __str__(self):";
    _out.inc();
    _out << nl << "return IcePy.stringify(self, _M_" << getAbsolute(p, "_t_") << ")";
    _out.dec();
    _out << sp << nl << "__repr__ = __str__";

    _out.dec();

    //
    // Define the proxy class.
    //
    if(!p->isLocal())
    {
        _out << sp << nl << "_M_" << prxAbs << " = Ice.createTempClass()";
        _out << nl << "class " << prxName << "(";
        if(bases.empty())
        {
            _out << "Ice.ObjectPrx";
        }
        else
        {
            ClassList::const_iterator q = bases.begin();
            while(q != bases.end())
            {
                _out << getSymbol(*q, "Prx");
                if(++q != bases.end())
                {
                    _out << ", ";
                }
            }
        }
        _out << "):";
        _out.inc();

        for(OperationList::iterator oli = ops.begin(); oli != ops.end(); ++oli)
        {
            string fixedOpName = fixIdent((*oli)->name());
            if(fixedOpName == "checkedCast" || fixedOpName == "uncheckedCast")
            {
                fixedOpName.insert(0, "_");
            }
            TypePtr ret = (*oli)->returnType();
            ParamDeclList paramList = (*oli)->parameters();
            string inParams;

            for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
            {
                if(!(*q)->isOutParam())
                {
                    if(!inParams.empty())
                    {
                        inParams.append(", ");
                    }
                    inParams.append(fixIdent((*q)->name()));
                }
            }

            _out << sp;
            writeDocstring(*oli, DocSync, false);
            _out << nl << "def " << fixedOpName << "(self";
            if(!inParams.empty())
            {
                _out << ", " << inParams;
            }
            _out << ", _ctx=None):";
            _out.inc();
            _out << nl << "return _M_" << abs << "._op_" << (*oli)->name() << ".invoke(self, ((" << inParams;
            if(!inParams.empty() && inParams.find(',') == string::npos)
            {
                _out << ", ";
            }
            _out << "), _ctx))";
            _out.dec();

            //
            // Async operations.
            //
            _out << sp;
            writeDocstring(*oli, DocAsyncBegin, false);
            _out << nl << "def begin_" << (*oli)->name() << "(self";
            if(!inParams.empty())
            {
                _out << ", " << inParams;
            }
            _out << ", _response=None, _ex=None, _sent=None, _ctx=None):";
            _out.inc();
            _out << nl << "return _M_" << abs << "._op_" << (*oli)->name() << ".begin(self, ((" << inParams;
            if(!inParams.empty() && inParams.find(',') == string::npos)
            {
                _out << ", ";
            }
            _out << "), _response, _ex, _sent, _ctx))";
            _out.dec();

            _out << sp;
            writeDocstring(*oli, DocAsyncEnd, false);
            _out << nl << "def end_" << (*oli)->name() << "(self, _r):";
            _out.inc();
            _out << nl << "return _M_" << abs << "._op_" << (*oli)->name() << ".end(self, _r)";
            _out.dec();
        }

        _out << sp << nl << "def checkedCast(proxy, facetOrCtx=None, _ctx=None):";
        _out.inc();
        _out << nl << "return _M_" << prxAbs << ".ice_checkedCast(proxy, '" << scoped << "', facetOrCtx, _ctx)";
        _out.dec();
        _out << nl << "checkedCast = staticmethod(checkedCast)";

        _out << sp << nl << "def uncheckedCast(proxy, facet=None):";
        _out.inc();
        _out << nl << "return _M_" << prxAbs << ".ice_uncheckedCast(proxy, facet)";
        _out.dec();
        _out << nl << "uncheckedCast = staticmethod(uncheckedCast)";


	//
        // ice_staticId
        //
        _out << sp << nl << "def ice_staticId():";
        _out.inc();
        _out << nl << "return '" << scoped << "'";
        _out.dec();
        _out << nl << "ice_staticId = staticmethod(ice_staticId)";

	_out.dec();

        _out << sp << nl << "_M_" << prxType << " = IcePy.defineProxy('" << scoped << "', " << prxName << ")";
    }

    if(_classHistory.count(scoped) == 0 && p->canBeCyclic())
    {
        //
        // Emit a forward declaration for the class in case a data member refers to this type.
        //
        _out << sp << nl << "_M_" << type << " = IcePy.declareClass('" << scoped << "')";
    }

    DataMemberList members = p->dataMembers();
    _out << sp << nl << "_M_" << type << " = IcePy.defineClass('" << scoped << "', " << name << ", " << p->compactId()
         << ", ";
    writeMetaData(p->getMetaData());
    const bool preserved = p->hasMetaData("preserve-slice") || p->inheritsMetaData("preserve-slice");
    _out << ", " << (isAbstract ? "True" : "False") << ", " << (preserved ? "True" : "False") << ", ";
    if(!base)
    {
        _out << "None";
    }
    else
    {
        _out << "_M_" << getAbsolute(base, "_t_");
    }
    _out << ", (";
    //
    // Interfaces
    //
    int interfaceCount = 0;
    for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
    {
        if((*q)->isInterface())
        {
            if(interfaceCount > 0)
            {
                _out << ", ";
            }
            _out << "_M_" << getAbsolute(*q, "_t_");
            ++interfaceCount;
        }
    }
    if(interfaceCount == 1)
    {
        _out << ',';
    }
    //
    // Members
    //
    // Data members are represented as a tuple:
    //
    //   ('MemberName', MemberMetaData, MemberType, Optional, Tag)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
    //
    _out << "), (";
    if(members.size() > 1)
    {
        _out.inc();
        _out << nl;
    }
    bool isProtected = p->hasMetaData("protected");
    for(DataMemberList::iterator r = members.begin(); r != members.end(); ++r)
    {
        if(r != members.begin())
        {
            _out << ',' << nl;
        }
        _out << "('";
        if(isProtected || (*r)->hasMetaData("protected"))
        {
            _out << '_';
        }
        _out << fixIdent((*r)->name()) << "', ";
        writeMetaData((*r)->getMetaData());
        _out << ", ";
        writeType((*r)->type());
        _out << ", " << ((*r)->optional() ? "True" : "False") << ", "
             << ((*r)->optional() ? (*r)->tag() : 0) << ')';
    }
    if(members.size() == 1)
    {
        _out << ',';
    }
    else if(members.size() > 1)
    {
        _out.dec();
        _out << nl;
    }
    _out << "))";
    _out << nl << name << "._ice_type = _M_" << type;

    //
    // Define each operation. The arguments to the IcePy.Operation constructor are:
    //
    // 'opName', Mode, SendMode, AMD, Format, MetaData, (InParams), (OutParams), ReturnParam, (Exceptions)
    //
    // where InParams and OutParams are tuples of type descriptions, and Exceptions
    // is a tuple of exception type ids.
    //
    if(!p->isLocal())
    {
        if(!ops.empty())
        {
            _out << sp;
        }
        for(OperationList::iterator s = ops.begin(); s != ops.end(); ++s)
        {
            ParamDeclList params = (*s)->parameters();
            ParamDeclList::iterator t;
            int count;
            string format;
            switch((*s)->format())
            {
            case DefaultFormat:
                format = "None";
                break;
            case CompactFormat:
                format = "Ice.FormatType.CompactFormat";
                break;
            case SlicedFormat:
                format = "Ice.FormatType.SlicedFormat";
                break;
            }

            _out << nl << name << "._op_" << (*s)->name() << " = IcePy.Operation('" << (*s)->name() << "', "
                 << getOperationMode((*s)->mode()) << ", " << getOperationMode((*s)->sendMode()) << ", "
                 << ((p->hasMetaData("amd") || (*s)->hasMetaData("amd")) ? "True" : "False") << ", "
                 << format << ", ";
            writeMetaData((*s)->getMetaData());
            _out << ", (";
            for(t = params.begin(), count = 0; t != params.end(); ++t)
            {
                if(!(*t)->isOutParam())
                {
                    if(count > 0)
                    {
                        _out << ", ";
                    }
                    _out << '(';
                    writeMetaData((*t)->getMetaData());
                    _out << ", ";
                    writeType((*t)->type());
                    _out << ", " << ((*t)->optional() ? "True" : "False") << ", "
                         << ((*t)->optional() ? (*t)->tag() : 0) << ')';
                    ++count;
                }
            }
            if(count == 1)
            {
                _out << ',';
            }
            _out << "), (";
            for(t = params.begin(), count = 0; t != params.end(); ++t)
            {
                if((*t)->isOutParam())
                {
                    if(count > 0)
                    {
                        _out << ", ";
                    }
                    _out << '(';
                    writeMetaData((*t)->getMetaData());
                    _out << ", ";
                    writeType((*t)->type());
                    _out << ", " << ((*t)->optional() ? "True" : "False") << ", "
                         << ((*t)->optional() ? (*t)->tag() : 0) << ')';
                    ++count;
                }
            }
            if(count == 1)
            {
                _out << ',';
            }
            _out << "), ";
            TypePtr returnType = (*s)->returnType();
            if(returnType)
            {
                //
                // The return type has the same format as an in/out parameter:
                //
                // MetaData, Type, Optional?, OptionalTag
                //
                _out << "((), ";
                writeType(returnType);
                _out << ", " << ((*s)->returnIsOptional() ? "True" : "False") << ", "
                     << ((*s)->returnIsOptional() ? (*s)->returnTag() : 0) << ')';
            }
            else
            {
                _out << "None";
            }
            _out << ", (";
            ExceptionList exceptions = (*s)->throws();
            for(ExceptionList::iterator u = exceptions.begin(); u != exceptions.end(); ++u)
            {
                if(u != exceptions.begin())
                {
                    _out << ", ";
                }
                _out << "_M_" << getAbsolute(*u, "_t_");
            }
            if(exceptions.size() == 1)
            {
                _out << ',';
            }
            _out << "))";

            string deprecateMetadata;
            if((*s)->findMetaData("deprecate", deprecateMetadata) || p->findMetaData("deprecate", deprecateMetadata))
            {
                string msg;
                string::size_type pos = deprecateMetadata.find(':');
                if(pos != string::npos && pos < deprecateMetadata.size() - 1)
                {
                    msg = deprecateMetadata.substr(pos + 1);
                }
                _out << nl << name << "._op_" << (*s)->name() << ".deprecate(\"" << msg << "\")";
            }
        }
    }

    registerName(name);

    if(!p->isLocal())
    {
        registerName(prxName);
    }

    _out.dec();

    if(_classHistory.count(scoped) == 0)
    {
        _classHistory.insert(scoped); // Avoid redundant declarations.
    }

    return false;
}

bool
Slice::Python::CodeVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string scoped = p->scoped();
    string abs = getAbsolute(p);
    string name = fixIdent(p->name());

    _out << sp << nl << "if " << getDictLookup(p) << ':';
    _out.inc();
    _out << nl << "_M_" << abs << " = Ice.createTempClass()";
    _out << nl << "class " << name << '(';
    ExceptionPtr base = p->base();
    string baseName;
    if(base)
    {
        baseName = getSymbol(base);
        _out << baseName;
    }
    else if(p->isLocal())
    {
        _out << "Ice.LocalException";
    }
    else
    {
        _out << "Ice.UserException";
    }
    _out << "):";
    _out.inc();

    DataMemberList members = p->dataMembers();

    writeDocstring(p->comment(), members);

    //
    // __init__
    //
    _out << nl << "def __init__(self";
    MemberInfoList allMembers;
    collectExceptionMembers(p, allMembers, false);
    writeConstructorParams(allMembers);
    _out << "):";
    _out.inc();
    if(!base && members.empty())
    {
        _out << nl << "pass";
    }
    else
    {
        if(base)
        {
            _out << nl << baseName << ".__init__(self";
            for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
            {
                if(q->inherited)
                {
                    _out << ", " << q->fixedName;
                }
            }
            _out << ')';
        }
        for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
        {
            if(!q->inherited)
            {
                writeAssign(*q);
            }
        }
    }
    _out.dec();

    //
    // __str__
    //
    _out << sp << nl << "def __str__(self):";
    _out.inc();
    _out << nl << "return IcePy.stringifyException(self)";
    _out.dec();
    _out << sp << nl << "__repr__ = __str__";

    //
    // _ice_name
    //
    _out << sp << nl << "_ice_name = '" << scoped.substr(2) << "'";

    _out.dec();

    //
    // Emit the type information.
    //
    string type = getAbsolute(p, "_t_");
    _out << sp << nl << "_M_" << type << " = IcePy.defineException('" << scoped << "', " << name << ", ";
    writeMetaData(p->getMetaData());
    const bool preserved = p->hasMetaData("preserve-slice") || p->inheritsMetaData("preserve-slice");
    _out << ", " << (preserved ? "True" : "False") << ", ";
    if(!base)
    {
        _out << "None";
    }
    else
    {
         _out << "_M_" << getAbsolute(base, "_t_");
    }
    _out << ", (";
    if(members.size() > 1)
    {
        _out.inc();
        _out << nl;
    }
    //
    // Data members are represented as a tuple:
    //
    //   ('MemberName', MemberMetaData, MemberType, Optional, Tag)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
    //
    for(DataMemberList::iterator dmli = members.begin(); dmli != members.end(); ++dmli)
    {
        if(dmli != members.begin())
        {
            _out << ',' << nl;
        }
        _out << "('" << fixIdent((*dmli)->name()) << "', ";
        writeMetaData((*dmli)->getMetaData());
        _out << ", ";
        writeType((*dmli)->type());
        _out << ", " << ((*dmli)->optional() ? "True" : "False") << ", "
             << ((*dmli)->optional() ? (*dmli)->tag() : 0) << ')';
    }
    if(members.size() == 1)
    {
        _out << ',';
    }
    else if(members.size() > 1)
    {
        _out.dec();
        _out << nl;
    }
    _out << "))";
    _out << nl << name << "._ice_type = _M_" << type;

    registerName(name);

    _out.dec();

    return false;
}

bool
Slice::Python::CodeVisitor::visitStructStart(const StructPtr& p)
{
    string scoped = p->scoped();
    string abs = getAbsolute(p);
    string name = fixIdent(p->name());
    DataMemberList members = p->dataMembers();
    MemberInfoList memberList;

    {
        for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
        {
            memberList.push_back(MemberInfo());
            memberList.back().fixedName = fixIdent((*q)->name());
            memberList.back().inherited = false;
            memberList.back().dataMember = *q;
        }
    }

    _out << sp << nl << "if " << getDictLookup(p) << ':';
    _out.inc();
    _out << nl << "_M_" << abs << " = Ice.createTempClass()";
    _out << nl << "class " << name << "(object):";
    _out.inc();

    writeDocstring(p->comment(), members);

    _out << nl << "def __init__(self";
    writeConstructorParams(memberList);
    _out << "):";
    _out.inc();
    for(MemberInfoList::iterator r = memberList.begin(); r != memberList.end(); ++r)
    {
        writeAssign(*r);
    }
    _out.dec();

    //
    // Only generate __hash__ and the comparison operators if this structure type
    // is a legal dictionary key type.
    //
    bool containsSequence = false;
    if(Dictionary::legalKeyType(p, containsSequence))
    {
        _out << sp << nl << "def __hash__(self):";
        _out.inc();
        _out << nl << "_h = 0";
        int iter = 0;
        for(MemberInfoList::iterator r = memberList.begin(); r != memberList.end(); ++r)
        {
            string s = "self." + r->fixedName;
            writeHash(s, r->dataMember->type(), iter);
        }
        _out << nl << "return _h % 0x7fffffff";
        _out.dec();

        //
        // Rich operators.  __lt__, __le__, __eq__, __ne__, __gt__, __ge__
        //

        _out << sp << nl << "def __compare(self, other):";
        _out.inc();
        _out << nl << "if other is None:";
        _out.inc();
        _out << nl << "return 1";
        _out.dec();
        _out << nl << "elif not isinstance(other, _M_" << abs << "):";
        _out.inc();
        _out << nl << "return NotImplemented";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        for(MemberInfoList::iterator r = memberList.begin(); r != memberList.end(); ++r)
        {
            //
            // The None value is not orderable in Python 3.
            //
            _out << nl << "if self." << r->fixedName << " is None or other." << r->fixedName << " is None:";
            _out.inc();
            _out << nl << "if self." << r->fixedName << " != other." << r->fixedName << ':';
            _out.inc();
            _out << nl << "return (-1 if self." << r->fixedName << " is None else 1)";
            _out.dec();
            _out.dec();
            _out << nl << "else:";
            _out.inc();
            _out << nl << "if self." << r->fixedName << " < other." << r->fixedName << ':';
            _out.inc();
            _out << nl << "return -1";
            _out.dec();
            _out << nl << "elif self." << r->fixedName << " > other." << r->fixedName << ':';
            _out.inc();
            _out << nl << "return 1";
            _out.dec();
            _out.dec();
        }
        _out << nl << "return 0";
        _out.dec();
        _out.dec();

        _out << sp << nl << "def __lt__(self, other):";
        _out.inc();
        _out << nl << "r = self.__compare(other)";
        _out << nl << "if r is NotImplemented:";
        _out.inc();
        _out << nl << "return r";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        _out << nl << "return r < 0";
        _out.dec();
        _out.dec();

        _out << sp << nl << "def __le__(self, other):";
        _out.inc();
        _out << nl << "r = self.__compare(other)";
        _out << nl << "if r is NotImplemented:";
        _out.inc();
        _out << nl << "return r";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        _out << nl << "return r <= 0";
        _out.dec();
        _out.dec();

        _out << sp << nl << "def __gt__(self, other):";
        _out.inc();
        _out << nl << "r = self.__compare(other)";
        _out << nl << "if r is NotImplemented:";
        _out.inc();
        _out << nl << "return r";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        _out << nl << "return r > 0";
        _out.dec();
        _out.dec();

        _out << sp << nl << "def __ge__(self, other):";
        _out.inc();
        _out << nl << "r = self.__compare(other)";
        _out << nl << "if r is NotImplemented:";
        _out.inc();
        _out << nl << "return r";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        _out << nl << "return r >= 0";
        _out.dec();
        _out.dec();

        _out << sp << nl << "def __eq__(self, other):";
        _out.inc();
        _out << nl << "r = self.__compare(other)";
        _out << nl << "if r is NotImplemented:";
        _out.inc();
        _out << nl << "return r";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        _out << nl << "return r == 0";
        _out.dec();
        _out.dec();

        _out << sp << nl << "def __ne__(self, other):";
        _out.inc();
        _out << nl << "r = self.__compare(other)";
        _out << nl << "if r is NotImplemented:";
        _out.inc();
        _out << nl << "return r";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        _out << nl << "return r != 0";
        _out.dec();
        _out.dec();
    }
    else
    {
        //
        // Only generate the rich equality operators __eq__ and __ne__.
        //

        _out << sp << nl << "def __eq__(self, other):";
        _out.inc();
        _out << nl << "if other is None:";
        _out.inc();
        _out << nl << "return False";
        _out.dec();
        _out << nl << "elif not isinstance(other, _M_" << abs << "):";
        _out.inc();
        _out << nl << "return NotImplemented";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        for(MemberInfoList::iterator r = memberList.begin(); r != memberList.end(); ++r)
        {
            //
            // The None value is not orderable in Python 3.
            //
            _out << nl << "if self." << r->fixedName << " != other." << r->fixedName << ':';
            _out.inc();
            _out << nl << "return False";
            _out.dec();
        }
        _out << nl << "return True";
        _out.dec();
        _out.dec();

        _out << sp << nl << "def __ne__(self, other):";
        _out.inc();
        _out << nl << "return not self.__eq__(other)";
        _out.dec();
    }

    //
    // __str__
    //
    _out << sp << nl << "def __str__(self):";
    _out.inc();
    _out << nl << "return IcePy.stringify(self, _M_" << getAbsolute(p, "_t_") << ")";
    _out.dec();
    _out << sp << nl << "__repr__ = __str__";

    _out.dec();

    //
    // Emit the type information.
    //
    _out << sp << nl << "_M_" << getAbsolute(p, "_t_") << " = IcePy.defineStruct('" << scoped << "', " << name << ", ";
    writeMetaData(p->getMetaData());
    _out << ", (";
    //
    // Data members are represented as a tuple:
    //
    //   ('MemberName', MemberMetaData, MemberType)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
    //
    if(memberList.size() > 1)
    {
        _out.inc();
        _out << nl;
    }
    for(MemberInfoList::iterator r = memberList.begin(); r != memberList.end(); ++r)
    {
        if(r != memberList.begin())
        {
            _out << ',' << nl;
        }
        _out << "('" << r->fixedName << "', ";
        writeMetaData(r->dataMember->getMetaData());
        _out << ", ";
        writeType(r->dataMember->type());
        _out << ')';
    }
    if(memberList.size() == 1)
    {
        _out << ',';
    }
    else if(memberList.size() > 1)
    {
        _out.dec();
        _out << nl;
    }
    _out << "))";

    registerName(name);

    _out.dec();

    return false;
}

void
Slice::Python::CodeVisitor::visitSequence(const SequencePtr& p)
{
    static const string protobuf = "python:protobuf:";
    StringList metaData = p->getMetaData();
    bool isCustom = false;
    string customType;
    for(StringList::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
    {
        if(q->find(protobuf) == 0)
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(p->type());
            if(!builtin || builtin->kind() != Builtin::KindByte)
            {
                continue;
            }
            isCustom = true;
            customType = q->substr(protobuf.size());
            break;
        }
    }

    //
    // Emit the type information.
    //
    string scoped = p->scoped();
    _out << sp << nl << "if " << getDictLookup(p, "_t_") << ':';
    _out.inc();
    if(isCustom)
    {
        string package = customType.substr(0, customType.find('.'));
        _out << nl << "import " << package;
        _out << nl << "_M_" << getAbsolute(p, "_t_")
             << " = IcePy.defineCustom('" << scoped << "', " << customType << ")";
    }
    else
    {
        _out << nl << "_M_" << getAbsolute(p, "_t_") << " = IcePy.defineSequence('" << scoped << "', ";
        writeMetaData(metaData);
        _out << ", ";
        writeType(p->type());
        _out << ")";
    }
    _out.dec();
}

void
Slice::Python::CodeVisitor::visitDictionary(const DictionaryPtr& p)
{
    //
    // Emit the type information.
    //
    string scoped = p->scoped();
    _out << sp << nl << "if " << getDictLookup(p, "_t_") << ':';
    _out.inc();
    _out << nl << "_M_" << getAbsolute(p, "_t_") << " = IcePy.defineDictionary('" << scoped << "', ";
    writeMetaData(p->getMetaData());
    _out << ", ";
    writeType(p->keyType());
    _out << ", ";
    writeType(p->valueType());
    _out << ")";
    _out.dec();
}

void
Slice::Python::CodeVisitor::visitEnum(const EnumPtr& p)
{
    string scoped = p->scoped();
    string abs = getAbsolute(p);
    string name = fixIdent(p->name());
    EnumeratorList enums = p->getEnumerators();
    EnumeratorList::iterator q;

    _out << sp << nl << "if " << getDictLookup(p) << ':';
    _out.inc();
    _out << nl << "_M_" << abs << " = Ice.createTempClass()";
    _out << nl << "class " << name << "(Ice.EnumBase):";
    _out.inc();

    writeDocstring(p->comment(), enums);

    _out << sp << nl << "def __init__(self, _n, _v):";
    _out.inc();
    _out << nl << "Ice.EnumBase.__init__(self, _n, _v)";
    _out.dec();

    _out << sp << nl << "def valueOf(self, _n):";
    _out.inc();
    _out << nl << "if _n in self._enumerators:";
    _out.inc();
    _out << nl << "return self._enumerators[_n]";
    _out.dec();
    _out << nl << "return None";
    _out.dec();
    _out << nl << "valueOf = classmethod(valueOf)";

    _out.dec();

    _out << sp;
    for(q = enums.begin(); q != enums.end(); ++q)
    {
        string fixedEnum = fixIdent((*q)->name());
        _out << nl << name << '.' << fixedEnum << " = " << name << "(\"" << (*q)->name() << "\", " << (*q)->value()
             << ')';
    }
    _out << nl << name << "._enumerators = { ";
    for(q = enums.begin(); q != enums.end(); ++q)
    {
        if(q != enums.begin())
        {
            _out << ", ";
        }
        string fixedEnum = fixIdent((*q)->name());
        _out << (*q)->value() << ':' << name << '.' << fixedEnum;
    }
    _out << " }";

    //
    // Emit the type information.
    //
    _out << sp << nl << "_M_" << getAbsolute(p, "_t_") << " = IcePy.defineEnum('" << scoped << "', " << name
         << ", ";
    writeMetaData(p->getMetaData());
    _out << ", " << name << "._enumerators)";

    registerName(name);

    _out.dec();
}

void
Slice::Python::CodeVisitor::visitConst(const ConstPtr& p)
{
    Slice::TypePtr type = p->type();
    string name = fixIdent(p->name());

    _out << sp << nl << "_M_" << getAbsolute(p) << " = ";
    writeConstantValue(type, p->valueType(), p->value());
}

string
Slice::Python::CodeVisitor::getSymbol(const ContainedPtr& p, const string& nameSuffix)
{
    //
    // An explicit reference to another type must always be prefixed with "_M_".
    //
    return "_M_" + getAbsolute(p, "", nameSuffix);
}

void
Slice::Python::CodeVisitor::registerName(const string& name)
{
    assert(!_moduleStack.empty());
    _out << sp << nl << "_M_" << _moduleStack.front() << '.' << name << " = " << name;
    _out << nl << "del " << name;
}

void
Slice::Python::CodeVisitor::writeType(const TypePtr& p)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindBool:
            {
                _out << "IcePy._t_bool";
                break;
            }
            case Builtin::KindByte:
            {
                _out << "IcePy._t_byte";
                break;
            }
            case Builtin::KindShort:
            {
                _out << "IcePy._t_short";
                break;
            }
            case Builtin::KindInt:
            {
                _out << "IcePy._t_int";
                break;
            }
            case Builtin::KindLong:
            {
                _out << "IcePy._t_long";
                break;
            }
            case Builtin::KindFloat:
            {
                _out << "IcePy._t_float";
                break;
            }
            case Builtin::KindDouble:
            {
                _out << "IcePy._t_double";
                break;
            }
            case Builtin::KindString:
            {
                _out << "IcePy._t_string";
                break;
            }
            case Builtin::KindObject:
            {
                _out << "IcePy._t_Object";
                break;
            }
            case Builtin::KindObjectProxy:
            {
                _out << "IcePy._t_ObjectPrx";
                break;
            }
            case Builtin::KindLocalObject:
            {
                _out << "IcePy._t_LocalObject";
                break;
            }
        }
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(p);
    if(prx)
    {
        _out << "_M_" << getAbsolute(prx->_class(), "_t_", "Prx");
        return;
    }

    ContainedPtr cont = ContainedPtr::dynamicCast(p);
    assert(cont);
    _out << "_M_" << getAbsolute(cont, "_t_");
}

void
Slice::Python::CodeVisitor::writeInitializer(const DataMemberPtr& m)
{
    TypePtr p = m->type();
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindBool:
            {
                _out << "False";
                break;
            }
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            {
                _out << "0";
                break;
            }
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            {
                _out << "0.0";
                break;
            }
            case Builtin::KindString:
            {
                _out << "''";
                break;
            }
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
            {
                _out << "None";
                break;
            }
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(p);
    if(en)
    {
        EnumeratorList enums = en->getEnumerators();
        _out << getSymbol(en) << "." << fixIdent(enums.front()->name());
        return;
    }

    StructPtr st = StructPtr::dynamicCast(p);
    if(st)
    {
        //
        // We cannot emit a call to the struct's constructor here because Python
        // only evaluates this expression once (see bug 3676). Instead, we emit
        // a marker that allows us to determine whether the application has
        // supplied a value.
        //
        _out << "Ice._struct_marker";
        return;
    }

    _out << "None";
}

void
Slice::Python::CodeVisitor::writeHash(const string& name, const TypePtr& p, int& iter)
{
    SequencePtr seq = SequencePtr::dynamicCast(p);
    if(seq)
    {
        _out << nl << "if " << name << ':';
        _out.inc();
        _out << nl << "for _i" << iter << " in " << name << ':';
        _out.inc();
        ostringstream elem;
        elem << "_i" << iter;
        iter++;
        writeHash(elem.str(), seq->type(), iter);
        _out.dec();
        _out.dec();
        return;
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(p);
    if(dict)
    {
        _out << nl << "if " << name << ':';
        _out.inc();
        _out << nl << "for _i" << iter << " in " << name << ':';
        _out.inc();
        ostringstream key;
        key << "_i" << iter;
        ostringstream value;
        value << name << '[' << key.str() << ']';
        iter++;
        writeHash(key.str(), dict->keyType(), iter);
        writeHash(value.str(), dict->valueType(), iter);
        _out.dec();
        _out.dec();
        return;
    }

    _out << nl << "_h = 5 * _h + Ice.getHash(" << name << ")";
}

void
Slice::Python::CodeVisitor::writeMetaData(const StringList& meta)
{
    int i = 0;
    _out << '(';
    for(StringList::const_iterator p = meta.begin(); p != meta.end(); ++p)
    {
        if(p->find("python:") == 0)
        {
            if(i > 0)
            {
                _out << ", ";
            }
            _out << "'" << *p << "'";
            ++i;
        }
    }
    if(i == 1)
    {
        _out << ',';
    }
    _out << ')';
}

void
Slice::Python::CodeVisitor::writeAssign(const MemberInfo& info)
{
    string paramName = info.fixedName;
    string memberName = info.fixedName;

    //
    // Structures are treated differently (see bug 3676).
    //
    StructPtr st = StructPtr::dynamicCast(info.dataMember->type());
    if(st && !info.dataMember->optional())
    {
        _out << nl << "if " << paramName << " is Ice._struct_marker:";
        _out.inc();
        _out << nl << "self." << memberName << " = " << getSymbol(st) << "()";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        _out << nl << "self." << memberName << " = " << paramName;
        _out.dec();
    }
    else
    {
        _out << nl << "self." << memberName << " = " << paramName;
    }
}

void
Slice::Python::CodeVisitor::writeConstantValue(const TypePtr& type, const SyntaxTreeBasePtr& valueType,
                                               const string& value)
{
    ConstPtr constant = ConstPtr::dynamicCast(valueType);
    if(constant)
    {
        _out << "_M_" << getAbsolute(constant);
    }
    else
    {
        Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(type);
        Slice::EnumPtr en = Slice::EnumPtr::dynamicCast(type);
        if(b)
        {
            switch(b->kind())
            {
            case Slice::Builtin::KindBool:
            {
                _out << (value == "true" ? "True" : "False");
                break;
            }
            case Slice::Builtin::KindByte:
            case Slice::Builtin::KindShort:
            case Slice::Builtin::KindInt:
            case Slice::Builtin::KindFloat:
            case Slice::Builtin::KindDouble:
            case Slice::Builtin::KindLong:
            {
                _out << value;
                break;
            }
            case Slice::Builtin::KindString:
            {
                ostringstream sv2;
                ostringstream sv3;
                
                //
                // Expand strings into the basic source character set. We can't use isalpha() and the like
                // here because they are sensitive to the current locale.
                //
                static const string basicSourceChars = "abcdefghijklmnopqrstuvwxyz"
                                                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                       "0123456789"
                                                       "_{}[]#()<>%:;.?*+-/^&|~!=, '";
                static const set<char> charSet(basicSourceChars.begin(), basicSourceChars.end());

                for(size_t i = 0; i < value.size();)
                {
                    char c = value[i];
                    switch(c)
                    {
                        case '"':
                        {
                            sv2 << "\\\"";
                            break;
                        }
                        case '\\':
                        {
                            string s = "\\";
                            size_t j = i + 1;
                            for(; j < value.size(); ++j)
                            {
                                if(value[j] != '\\')
                                {
                                    break;
                                }
                                s += "\\";
                            }

                            //
                            // An even number of slash \ will escape the backslash and
                            // the codepoint will be interpreted as its charaters
                            //
                            // \\u00000041  - ['\\', 'u', '0', '0', '0', '0', '0', '0', '4', '1']
                            // \\\u00000041 - ['\\', 'A'] (41 is the codepoint for 'A')
                            //
                            if(s.size() % 2 != 0 && (value[j] == 'U' || value[j] == 'u'))
                            {
                                //
                                // Convert codepoint to UTF8 bytes and write the escaped bytes
                                //
                                sv2 << s.substr(0, s.size() - 1);

                                size_t sz = value[j] == 'U' ? 8 : 4;
                                string codepoint = value.substr(j + 1, sz);
                                assert(codepoint.size() ==  sz);

                                IceUtil::Int64 v = IceUtilInternal::strToInt64(codepoint.c_str(), 0, 16);

                                vector<unsigned int> u32buffer;
                                u32buffer.push_back(static_cast<unsigned int>(v));

                                vector<unsigned char> u8buffer;
                                IceUtilInternal::ConversionResult result = convertUTF32ToUTF8(u32buffer, u8buffer, IceUtil::lenientConversion);
                                switch(result)
                                {
                                    case conversionOK:
                                        break;
                                    case sourceExhausted:
                                        throw IceUtil::IllegalConversionException(__FILE__, __LINE__, "string source exhausted");
                                    case sourceIllegal:
                                        throw IceUtil::IllegalConversionException(__FILE__, __LINE__, "string source illegal");
                                    default:
                                    {
                                        assert(0);
                                        throw IceUtil::IllegalConversionException(__FILE__, __LINE__);
                                    }
                                }

                                ostringstream s;
                                for(vector<unsigned char>::const_iterator q = u8buffer.begin(); q != u8buffer.end(); ++q)
                                {
                                    s << "\\";
                                    s.fill('0');
                                    s.width(3);
                                    s << oct;
                                    s << static_cast<unsigned int>(*q);
                                }
                                sv2 << s.str();

                                i = j + 1 + sz;
                            }
                            else
                            {
                                sv2 << s;
                                i = j;
                            }
                            continue;
                        }
                        case '\r':
                        {
                            sv2 << "\\r";
                            break;
                        }
                        case '\n':
                        {
                            sv2 << "\\n";
                            break;
                        }
                        case '\t':
                        {
                            sv2 << "\\t";
                            break;
                        }
                        case '\b':
                        {
                            sv2 << "\\b";
                            break;
                        }
                        case '\f':
                        {
                            sv2 << "\\f";
                            break;
                        }
                        default:
                        {
                            if(charSet.find(c) == charSet.end())
                            {
                                unsigned char uc = c;               // Char may be signed, so make it positive.
                                stringstream s;
                                s << "\\";                          // Print as octal if not in basic source character set.
                                s.flags(ios_base::oct);
                                s.width(3);
                                s.fill('0');
                                s << static_cast<unsigned>(uc);
                                sv2 << s.str();
                            }
                            else
                            {
                                sv2 << c;                          // Print normally if in basic source character set.
                            }
                            break;
                        }
                    }
                    ++i;
                }
                
                vector<unsigned char> u8buffer;                     // Buffer to convert multibyte characters

                for(size_t i = 0; i < value.size();)
                {
                    if(charSet.find(value[i]) == charSet.end())
                    {
                        char c = value[i];
                        if(static_cast<unsigned char>(c) < 128) // Single byte character
                        {
                            //
                            // Print as unicode if not in basic source character set
                            //
                            switch(c)
                            {
                                //
                                // Don't encode this special characters as universal characters
                                //
                                case '\r':
                                {
                                    sv3 << "\\r";
                                    break;
                                }
                                case '\n':
                                {
                                    sv3 << "\\n";
                                    break;
                                }
                                case  '\\':
                                {
                                    sv3 << "\\";
                                    break;
                                }
                                default:
                                {
                                    sv3 << u32CodePoint(c);
                                    break;
                                }
                            }
                        }
                        else
                        {
                            u8buffer.push_back(value[i]);
                        }
                    }
                    else
                    {
                        //
                        // Write any pedding characters in the utf8 buffer
                        //
                        if(!u8buffer.empty())
                        {
                            writeU8Buffer(u8buffer, sv3);
                            u8buffer.clear();
                        }
                        switch(value[i])
                        {
                            case '\\':
                            {
                                string s = "\\";
                                size_t j = i + 1;
                                for(; j < value.size(); ++j)
                                {
                                    if(value[j] != '\\')
                                    {
                                        break;
                                    }
                                    s += "\\";
                                }

                                //
                                // An even number of slash \ will escape the backslash and
                                // the codepoint will be interpreted as its charaters
                                //
                                // \\U00000041  - ['\\', 'U', '0', '0', '0', '0', '0', '0', '4', '1']
                                // \\\U00000041 - ['\\', 'A'] (41 is the codepoint for 'A')
                                //
                                if(s.size() % 2 != 0 && (value[j] == 'U' || value[j] == 'u'))
                                {
                                    size_t sz = value[j] == 'U' ? 8 : 4;
                                    sv3 << s.substr(0, s.size() - 1);
                                    i = j + 1;

                                    string codepoint = value.substr(j + 1, sz);
                                    assert(codepoint.size() ==  sz);

                                    IceUtil::Int64 v = IceUtilInternal::strToInt64(codepoint.c_str(), 0, 16);
                                    sv3 << u32CodePoint(static_cast<unsigned int>(v));
                                    i = j + 1 + sz;
                                }
                                else
                                {
                                    sv3 << s;
                                    i = j;
                                }
                                continue;
                            }
                            case '"':
                            {
                                sv3 << "\\";
                                break;
                            }
                        }
                        sv3 << value[i];                        // Print normally if in basic source character set
                    }
                    i++;
                }

                //
                // Write any pedding characters in the utf8 buffer
                //
                if(!u8buffer.empty())
                {
                    writeU8Buffer(u8buffer, sv3);
                    u8buffer.clear();
                }
                

                _out << "\"" << sv2.str() << "\"";
                if(sv2.str() != sv3.str())
                {
                    _out << " if _version_info_[0] < 3 else \"" << sv3.str() << "\"";
                }
                break;
            }
            case Slice::Builtin::KindObject:
            case Slice::Builtin::KindObjectProxy:
            case Slice::Builtin::KindLocalObject:
                assert(false);
            }
        }
        else if(en)
        {
            string enumName = getSymbol(en);
            string::size_type colon = value.rfind(':');
            string enumerator;
            if(colon != string::npos)
            {
                enumerator = fixIdent(value.substr(colon + 1));
            }
            else
            {
                enumerator = fixIdent(value);
            }
            _out << enumName << '.' << enumerator;
        }
        else
        {
            assert(false); // Unknown const type.
        }
    }
}

void
Slice::Python::CodeVisitor::writeConstructorParams(const MemberInfoList& members)
{
    for(MemberInfoList::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        _out << ", " << p->fixedName << "=";

        const DataMemberPtr member = p->dataMember;
        if(member->defaultValueType())
        {
            writeConstantValue(member->type(), member->defaultValueType(), member->defaultValue());
        }
        else if(member->optional())
        {
            _out << "Ice.Unset";
        }
        else
        {
            writeInitializer(member);
        }
    }
}

string
Slice::Python::CodeVisitor::getOperationMode(Slice::Operation::Mode mode)
{
    string result;
    switch(mode)
    {
    case Operation::Normal:
        result = "Ice.OperationMode.Normal";
        break;
    case Operation::Nonmutating:
        result = "Ice.OperationMode.Nonmutating";
        break;
    case Operation::Idempotent:
        result = "Ice.OperationMode.Idempotent";
        break;
    }
    return result;
}

void
Slice::Python::CodeVisitor::collectClassMembers(const ClassDefPtr& p, MemberInfoList& allMembers, bool inherited)
{
    ClassList bases = p->bases();
    if(!bases.empty() && !bases.front()->isInterface())
    {
        collectClassMembers(bases.front(), allMembers, true);
    }

    DataMemberList members = p->dataMembers();

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        MemberInfo m;
        if(p->hasMetaData("protected") || (*q)->hasMetaData("protected"))
        {
            m.fixedName = "_" + fixIdent((*q)->name());
        }
        else
        {
            m.fixedName = fixIdent((*q)->name());
        }
        m.inherited = inherited;
        m.dataMember = *q;
        allMembers.push_back(m);
    }
}

void
Slice::Python::CodeVisitor::collectExceptionMembers(const ExceptionPtr& p, MemberInfoList& allMembers, bool inherited)
{
    ExceptionPtr base = p->base();
    if(base)
    {
        collectExceptionMembers(base, allMembers, true);
    }

    DataMemberList members = p->dataMembers();

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        MemberInfo m;
        m.fixedName = fixIdent((*q)->name());
        m.inherited = inherited;
        m.dataMember = *q;
        allMembers.push_back(m);
    }
}

Slice::Python::CodeVisitor::StringVec
Slice::Python::CodeVisitor::stripMarkup(const string& comment)
{
    //
    // Strip HTML markup and javadoc links.
    //
    string text = comment;
    string::size_type pos = 0;
    do
    {
        pos = text.find('<', pos);
        if(pos != string::npos)
        {
            string::size_type endpos = text.find('>', pos);
            if(endpos == string::npos)
            {
                break;
            }
            text.erase(pos, endpos - pos + 1);
        }
    } while(pos != string::npos);

    const string link = "{@link";
    pos = 0;
    do
    {
        pos = text.find(link, pos);
        if(pos != string::npos)
        {
            text.erase(pos, link.size());
            string::size_type endpos = text.find('}', pos);
            if(endpos != string::npos)
            {
                string::size_type identpos = text.find_first_not_of(" \t#", pos);
                if(identpos != string::npos && identpos < endpos)
                {
                    string ident = text.substr(identpos, endpos - identpos);
                    text.replace(pos, endpos - pos + 1, fixIdent(ident));
                }
            }
        }
    } while(pos != string::npos);

    //
    // Strip @see sections.
    //
    static const string seeTag = "@see";
    pos = 0;
    do
    {
        //
        // Look for the next @ and delete up to that, or
        // to the end of the string, if not found.
        //
        pos = text.find(seeTag, pos);
        if(pos != string::npos)
        {
            string::size_type next = text.find('@', pos + seeTag.size());
            if(next != string::npos)
            {
                text.erase(pos, next - pos);
            }
            else
            {
                text.erase(pos, string::npos);
            }
        }
    } while(pos != string::npos);

    //
    // Escape triple quotes.
    //
    static const string singleQuotes = "'''";
    pos = 0;
    while((pos = text.find(singleQuotes, pos)) != string::npos)
    {
        text.insert(pos, "\\");
        pos += singleQuotes.size() + 1;
    }
    static const string doubleQuotes = "\"\"\"";
    pos = 0;
    while((pos = text.find(doubleQuotes, pos)) != string::npos)
    {
        text.insert(pos, "\\");
        pos += doubleQuotes.size() + 1;
    }

    //
    // Fold multiple empty lines.
    //
    pos = 0;
    while(true)
    {
        pos = text.find('\n', pos);
        if(pos == string::npos)
        {
            break;
        }

        //
        // Skip the next LF or CR/LF, if present.
        //
        if(pos < text.size() - 1 && text[pos + 1] == '\n')
        {
            pos += 2;
        }
        else if(pos < text.size() - 2 && text[pos + 1] == '\r' && text[pos + 2] == '\n')
        {
            pos += 3;
        }
        else
        {
            ++pos;
            continue;
        }

        //
        // Erase any more CR/LF characters.
        //
        string::size_type next = text.find_first_not_of("\r\n", pos);
        if(next != string::npos)
        {
            text.erase(pos, next - pos);
        }
    }

    //
    // Remove trailing whitespace.
    //
    pos = text.find_last_not_of(" \t\r\n");
    if(pos != string::npos)
    {
        text.erase(pos + 1, text.size() - pos - 1);
    }

    //
    // Split text into lines.
    //
    StringVec lines;
    if(!text.empty())
    {
        string::size_type start = 0;
        while(start != string::npos)
        {
            string::size_type nl = text.find_first_of("\r\n", start);
            string line;
            if(nl != string::npos)
            {
                line = text.substr(start, nl - start);
                start = nl;
            }
            else
            {
                line = text.substr(start);
                start = text.size();
            }

            lines.push_back(line);

            start = text.find_first_not_of("\r\n", start);
        }
    }

    return lines;
}

void
Slice::Python::CodeVisitor::writeDocstring(const string& comment, const string& prefix)
{
    StringVec lines = stripMarkup(comment);
    if(lines.empty())
    {
        return;
    }

    _out << nl << prefix << "\"\"\"";

    for(StringVec::const_iterator q = lines.begin(); q != lines.end(); ++q)
    {
        _out << nl << *q;
    }

    _out << nl << "\"\"\"";
}

void
Slice::Python::CodeVisitor::writeDocstring(const string& comment, const DataMemberList& members)
{
    StringVec lines = stripMarkup(comment);
    if(lines.empty())
    {
        return;
    }

    _out << nl << "\"\"\"";

    for(StringVec::const_iterator q = lines.begin(); q != lines.end(); ++q)
    {
        _out << nl << *q;
    }

    if(!members.empty())
    {
        //
        // Collect docstrings (if any) for the members.
        //
        map<string, StringVec> docs;
        for(DataMemberList::const_iterator m = members.begin(); m != members.end(); ++m)
        {
            StringVec doc = stripMarkup((*m)->comment());
            if(!doc.empty())
            {
                docs[(*m)->name()] = doc;
            }
        }
        //
        // Only emit members if there's a docstring for at least one member.
        //
        if(!docs.empty())
        {
            _out << nl << "Members:";
            for(DataMemberList::const_iterator m = members.begin(); m != members.end(); ++m)
            {
                _out << nl << fixIdent((*m)->name()) << " -- ";
                map<string, StringVec>::iterator p = docs.find((*m)->name());
                if(p != docs.end())
                {
                    for(StringVec::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
                    {
                        if(q != p->second.begin())
                        {
                            _out << nl;
                        }
                        _out << *q;
                    }
                }
            }
        }
    }

    _out << nl << "\"\"\"";
}

void
Slice::Python::CodeVisitor::writeDocstring(const string& comment, const EnumeratorList& enums)
{
    StringVec lines = stripMarkup(comment);
    if(lines.empty())
    {
        return;
    }

    _out << nl << "\"\"\"";

    for(StringVec::const_iterator q = lines.begin(); q != lines.end(); ++q)
    {
        _out << nl << *q;
    }

    if(!enums.empty())
    {
        //
        // Collect docstrings (if any) for the enumerators.
        //
        map<string, StringVec> docs;
        for(EnumeratorList::const_iterator e = enums.begin(); e != enums.end(); ++e)
        {
            StringVec doc = stripMarkup((*e)->comment());
            if(!doc.empty())
            {
                docs[(*e)->name()] = doc;
            }
        }
        //
        // Only emit enumerators if there's a docstring for at least one enumerator.
        //
        if(!docs.empty())
        {
            _out << nl << "Enumerators:";
            for(EnumeratorList::const_iterator e = enums.begin(); e != enums.end(); ++e)
            {
                _out << nl << fixIdent((*e)->name()) << " -- ";
                map<string, StringVec>::iterator p = docs.find((*e)->name());
                if(p != docs.end())
                {
                    for(StringVec::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
                    {
                        if(q != p->second.begin())
                        {
                            _out << nl;
                        }
                        _out << *q;
                    }
                }
            }
        }
    }

    _out << nl << "\"\"\"";
}

bool
Slice::Python::CodeVisitor::parseOpComment(const string& comment, OpComment& c)
{
    //
    // Remove most javadoc & HTML markup.
    //
    StringVec lines = stripMarkup(comment);
    if(lines.empty())
    {
        return false;
    }

    //
    // Extract the descriptions of parameters, exceptions and return values.
    //
    string name;
    bool inParam = false, inException = false, inReturn = false;
    vector<string>::size_type i = 0;
    while(i < lines.size())
    {
        string l = lines[i];
        string::size_type paramTag = l.find("@param");
        string::size_type throwTag = l.find("@throw");
        string::size_type returnTag = l.find("@return");

        if(paramTag != string::npos)
        {
            string::size_type pos = l.find_first_of(" \t", paramTag);
            if(pos != string::npos)
            {
                pos = l.find_first_not_of(" \t", pos);
            }
            if(pos != string::npos)
            {
                string::size_type namePos = pos;
                pos = l.find_first_of(" \t", pos);
                inParam = true;
                inException = false;
                inReturn = false;
                if(pos == string::npos)
                {
                    //
                    // Doc assumed to have the format
                    //
                    // @param foo
                    // Description of foo...
                    //
                    name = l.substr(namePos);
                    c.params[name] = "";
                }
                else
                {
                    name = l.substr(namePos, pos - namePos);
                    pos = l.find_first_not_of(" \t", pos);
                    if(pos != string::npos)
                    {
                        c.params[name] = l.substr(pos);
                    }
                    else
                    {
                        c.params[name] = "";
                    }
                }
            }
            lines.erase(lines.begin() + i);
            continue;
        }
        else if(throwTag != string::npos)
        {
            string::size_type pos = l.find_first_of(" \t", throwTag);
            if(pos != string::npos)
            {
                pos = l.find_first_not_of(" \t", pos);
            }
            if(pos != string::npos)
            {
                string::size_type namePos = pos;
                pos = l.find_first_of(" \t", pos);
                inException = true;
                inParam = false;
                inReturn = false;
                if(pos == string::npos)
                {
                    //
                    // Doc assumed to have the format
                    //
                    // @throw foo
                    // Description of foo...
                    //
                    name = l.substr(namePos);
                    c.exceptions[name] = "";
                }
                else
                {
                    name = l.substr(namePos, pos - namePos);
                    pos = l.find_first_not_of(" \t", pos);
                    if(pos != string::npos)
                    {
                        c.exceptions[name] = l.substr(pos);
                    }
                    else
                    {
                        c.exceptions[name] = "";
                    }
                }
            }
            lines.erase(lines.begin() + i);
            continue;
        }
        else if(returnTag != string::npos)
        {
            string::size_type pos = l.find_first_of(" \t", returnTag);
            if(pos != string::npos)
            {
                pos = l.find_first_not_of(" \t", pos);
            }
            if(pos != string::npos)
            {
                inReturn = true;
                inException = false;
                inParam = false;
                c.returns = l.substr(pos);
            }
            lines.erase(lines.begin() + i);
            continue;
        }
        else
        {
            //
            // We didn't find a tag so we assume this line is a continuation of a
            // previous description.
            //
            string::size_type pos = l.find_first_not_of(" \t");
            if(pos != string::npos && l[pos] != '@')
            {
                if(inParam)
                {
                    assert(!name.empty());
                    if(!c.params[name].empty())
                    {
                        c.params[name] += " ";
                    }
                    c.params[name] += l.substr(pos);
                    lines.erase(lines.begin() + i);
                    continue;
                }
                else if(inException)
                {
                    assert(!name.empty());
                    if(!c.exceptions[name].empty())
                    {
                        c.exceptions[name] += " ";
                    }
                    c.exceptions[name] += l.substr(pos);
                    lines.erase(lines.begin() + i);
                    continue;
                }
                else if(inReturn)
                {
                    if(!c.returns.empty())
                    {
                        c.returns += " ";
                    }
                    c.returns += l.substr(pos);
                    lines.erase(lines.begin() + i);
                    continue;
                }
            }
        }

        i++;
    }

    //
    // All remaining lines become the general description.
    //
    for(vector<string>::iterator p = lines.begin(); p != lines.end(); ++p)
    {
        if(p->find_first_not_of(" \t\n\r") != string::npos)
        {
            c.description.push_back(*p);
        }
    }

    return true;
}

void
Slice::Python::CodeVisitor::writeDocstring(const OperationPtr& op, DocstringMode mode, bool local)
{
    OpComment comment;
    if(!parseOpComment(op->comment(), comment))
    {
        return;
    }

    TypePtr ret = op->returnType();
    ParamDeclList params = op->parameters();
    vector<string> inParams, outParams;
    for(ParamDeclList::iterator p = params.begin(); p != params.end(); ++p)
    {
        if((*p)->isOutParam())
        {
            outParams.push_back((*p)->name());
        }
        else
        {
            inParams.push_back((*p)->name());
        }
    }

    if(comment.description.empty())
    {
        if((mode == DocSync || mode == DocDispatch) && comment.params.empty() && comment.exceptions.empty() &&
           comment.returns.empty())
        {
            return;
        }
        else if(mode == DocAsyncBegin && inParams.empty())
        {
            return;
        }
        else if(mode == DocAsyncEnd && outParams.empty() && comment.returns.empty())
        {
            return;
        }
        else if(mode == DocAsyncDispatch && inParams.empty() && comment.exceptions.empty())
        {
            return;
        }
    }

    //
    // Emit the general description.
    //
    _out << nl << "\"\"\"";
    if(!comment.description.empty())
    {
        for(StringVec::const_iterator q = comment.description.begin(); q != comment.description.end(); ++q)
        {
            _out << nl << *q;
        }
    }

    //
    // Emit arguments.
    //
    bool needArgs = false;
    switch(mode)
    {
    case DocSync:
    case DocAsyncBegin:
    case DocDispatch:
        needArgs = !local || !inParams.empty();
        break;
    case DocAsyncEnd:
    case DocAsyncDispatch:
        needArgs = true;
        break;
    }

    if(needArgs)
    {
        _out << nl << "Arguments:";
        if(mode == DocAsyncDispatch)
        {
            _out << nl << "_cb -- The asynchronous callback object.";
        }
        for(vector<string>::iterator q = inParams.begin(); q != inParams.end(); ++q)
        {
            string fixed = fixIdent(*q);
            _out << nl << fixed << " -- ";
            StringMap::const_iterator r = comment.params.find(*q);
            if(r == comment.params.end())
            {
                r = comment.params.find(fixed); // Just in case.
            }
            if(r != comment.params.end())
            {
                _out << r->second;
            }
        }
        if(mode == DocAsyncBegin)
        {
            _out << nl << "_response -- The asynchronous response callback."
                 << nl << "_ex -- The asynchronous exception callback."
                 << nl << "_sent -- The asynchronous sent callback.";
        }
        if(!local && (mode == DocSync || mode == DocAsyncBegin))
        {
            _out << nl << "_ctx -- The request context for the invocation.";
        }
        if(!local && (mode == DocDispatch || mode == DocAsyncDispatch))
        {
            _out << nl << "current -- The Current object for the invocation.";
        }
    }
    else if(mode == DocAsyncEnd)
    {
        _out << nl << "Arguments:"
             << nl << "_r - The asynchronous result object for the invocation.";
    }

    //
    // Emit return value(s).
    //
    if(mode == DocAsyncBegin)
    {
        _out << nl << "Returns: An asynchronous result object for the invocation.";
    }

    if((mode == DocSync || mode == DocAsyncEnd || mode == DocDispatch) && (ret || !outParams.empty()))
    {
        if((outParams.size() + (ret ? 1 : 0)) > 1)
        {
            _out << nl << "Returns a tuple containing the following:";
            if(ret)
            {
                _out << nl << "_retval -- " << comment.returns;
            }
            for(vector<string>::iterator q = outParams.begin(); q != outParams.end(); ++q)
            {
                string fixed = fixIdent(*q);
                _out << nl << fixed << " -- ";
                StringMap::const_iterator r = comment.params.find(*q);
                if(r == comment.params.end())
                {
                    r = comment.params.find(fixed); // Just in case.
                }
                if(r != comment.params.end())
                {
                    _out << r->second;
                }
            }
        }
        else if(ret && !comment.returns.empty())
        {
            _out << nl << "Returns: " << comment.returns;
        }
        else if(!outParams.empty())
        {
            assert(outParams.size() == 1);
            _out << nl << "Returns:";
            string fixed = fixIdent(outParams[0]);
            _out << nl << fixed << " -- ";
            StringMap::const_iterator r = comment.params.find(outParams[0]);
            if(r == comment.params.end())
            {
                r = comment.params.find(fixed); // Just in case.
            }
            if(r != comment.params.end())
            {
                _out << r->second;
            }
        }
    }

    //
    // Emit exceptions.
    //
    if((mode == DocSync || mode == DocAsyncEnd || mode == DocDispatch || mode == DocAsyncDispatch) &&
       !comment.exceptions.empty())
    {
        _out << nl << "Throws:";
        for(StringMap::const_iterator r = comment.exceptions.begin(); r != comment.exceptions.end(); ++r)
        {
            _out << nl << r->first << " -- " << r->second;
        }
    }
    _out << nl << "\"\"\"";
}

void
Slice::Python::generate(const UnitPtr& un, bool all, bool checksum, const vector<string>& includePaths, Output& out)
{
    Slice::Python::MetaDataVisitor visitor;
    un->visit(&visitor, false);

    out << nl << "from sys import version_info as _version_info_";
    out << nl << "import Ice, IcePy";

    if(!all)
    {
        vector<string> paths = includePaths;
        for(vector<string>::iterator p = paths.begin(); p != paths.end(); ++p)
        {
            *p = fullPath(*p);
        }

        StringList includes = un->includeFiles();
        for(StringList::const_iterator q = includes.begin(); q != includes.end(); ++q)
        {
            string file = changeInclude(*q, paths);
            replace(file.begin(), file.end(), '/', '_');
            out << nl << "import " << file << "_ice";
        }
    }

    set<string> moduleHistory;

    ModuleVisitor moduleVisitor(out, moduleHistory);
    un->visit(&moduleVisitor, true);

    CodeVisitor codeVisitor(out, moduleHistory);
    un->visit(&codeVisitor, false);

    if(checksum)
    {
        ChecksumMap checksums = createChecksums(un);
        if(!checksums.empty())
        {
            out << sp;
            for(ChecksumMap::const_iterator p = checksums.begin(); p != checksums.end(); ++p)
            {
                out << nl << "Ice.sliceChecksums[\"" << p->first << "\"] = \"";
                ostringstream str;
                str.flags(ios_base::hex);
                str.fill('0');
                for(vector<unsigned char>::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
                {
                    str << static_cast<int>(*q);
                }
                out << str.str() << "\"";
            }
        }
    }

    out << nl; // Trailing newline.
}

string
Slice::Python::scopedToName(const string& scoped)
{
    string result = fixIdent(scoped);
    if(result.find("::") == 0)
    {
        result.erase(0, 2);
    }

    string::size_type pos;
    while((pos = result.find("::")) != string::npos)
    {
        result.replace(pos, 2, ".");
    }

    return result;
}

string
Slice::Python::fixIdent(const string& ident)
{
    if(ident[0] != ':')
    {
        return lookupKwd(ident);
    }
    vector<string> ids = splitScopedName(ident);
    transform(ids.begin(), ids.end(), ids.begin(), ptr_fun(lookupKwd));
    stringstream result;
    for(vector<string>::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        result << "::" + *i;
    }
    return result.str();
}

string
Slice::Python::getPackageMetadata(const ContainedPtr& cont)
{
    UnitPtr unit = cont->container()->unit();
    string file = cont->file();
    assert(!file.empty());

    static const string prefix = "python:package:";
    DefinitionContextPtr dc = unit->findDefinitionContext(file);
    assert(dc);
    string q = dc->findMetaData(prefix);
    if(!q.empty())
    {
        q = q.substr(prefix.size());
    }
    return q;
}

string
Slice::Python::getAbsolute(const ContainedPtr& cont, const string& suffix, const string& nameSuffix)
{
    string scope = scopedToName(cont->scope());

    string package = getPackageMetadata(cont);
    if(!package.empty())
    {
        if(!scope.empty())
        {
            scope = package + "." + scope;
        }
        else
        {
            scope = package + ".";
        }
    }

    if(suffix.empty())
    {
        return scope + fixIdent(cont->name() + nameSuffix);
    }
    else
    {
        return scope + suffix + fixIdent(cont->name() + nameSuffix);
    }
}

void
Slice::Python::printHeader(IceUtilInternal::Output& out)
{
    static const char* header =
"# **********************************************************************\n"
"#\n"
"# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.\n"
"#\n"
"# This copy of Ice is licensed to you under the terms described in the\n"
"# ICE_LICENSE file included in this distribution.\n"
"#\n"
"# **********************************************************************\n"
        ;

    out << header;
    out << "#\n";
    out << "# Ice version " << ICE_STRING_VERSION << "\n";
    out << "#\n";
}

bool
Slice::Python::MetaDataVisitor::visitUnitStart(const UnitPtr& p)
{
    static const string prefix = "python:";

    //
    // Validate global metadata in the top-level file and all included files.
    //
    StringList files = p->allFiles();

    for(StringList::iterator q = files.begin(); q != files.end(); ++q)
    {
        string file = *q;
        DefinitionContextPtr dc = p->findDefinitionContext(file);
        assert(dc);
        StringList globalMetaData = dc->getMetaData();
        for(StringList::const_iterator r = globalMetaData.begin(); r != globalMetaData.end(); ++r)
        {
            string s = *r;
            if(_history.count(s) == 0)
            {
                _history.insert(s);
                if(s.find(prefix) == 0)
                {
                    static const string packagePrefix = "python:package:";
                    if(s.find(packagePrefix) == 0 && s.size() > packagePrefix.size())
                    {
                        continue;
                    }
                    emitWarning(file, "", "ignoring invalid global metadata `" + s + "'");
                }
            }
        }
    }
    return true;
}

bool
Slice::Python::MetaDataVisitor::visitModuleStart(const ModulePtr& p)
{
    reject(p);
    return true;
}

void
Slice::Python::MetaDataVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    reject(p);
}

bool
Slice::Python::MetaDataVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    reject(p);
    return true;
}

bool
Slice::Python::MetaDataVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    reject(p);
    return true;
}

bool
Slice::Python::MetaDataVisitor::visitStructStart(const StructPtr& p)
{
    reject(p);
    return true;
}

void
Slice::Python::MetaDataVisitor::visitOperation(const OperationPtr& p)
{
    TypePtr ret = p->returnType();
    if(ret)
    {
        validateSequence(p->file(), p->line(), ret, p->getMetaData());
    }

    ParamDeclList params = p->parameters();
    for(ParamDeclList::iterator q = params.begin(); q != params.end(); ++q)
    {
        validateSequence(p->file(), (*q)->line(), (*q)->type(), (*q)->getMetaData());
    }
}

void
Slice::Python::MetaDataVisitor::visitDataMember(const DataMemberPtr& p)
{
    validateSequence(p->file(), p->line(), p->type(), p->getMetaData());
}

void
Slice::Python::MetaDataVisitor::visitSequence(const SequencePtr& p)
{
    static const string protobuf = "python:protobuf:";
    StringList metaData = p->getMetaData();
    const string file = p->file();
    const string line = p->line();
    for(StringList::const_iterator q = metaData.begin(); q != metaData.end(); )
    {
        string s = *q++;
        if(s.find(protobuf) == 0)
        {
            //
            // Remove from list so validateSequence does not try to handle as well.
            //
            metaData.remove(s);

            BuiltinPtr builtin = BuiltinPtr::dynamicCast(p->type());
            if(!builtin || builtin->kind() != Builtin::KindByte)
            {
                emitWarning(file, line, "ignoring invalid metadata `" + s + ": " +
                            "`protobuf' encoding must be a byte sequence");
            }
        }
    }

    validateSequence(file, line, p, metaData);
}

void
Slice::Python::MetaDataVisitor::visitDictionary(const DictionaryPtr& p)
{
    reject(p);
}

void
Slice::Python::MetaDataVisitor::visitEnum(const EnumPtr& p)
{
    reject(p);
}

void
Slice::Python::MetaDataVisitor::visitConst(const ConstPtr& p)
{
    reject(p);
}

void
Slice::Python::MetaDataVisitor::validateSequence(const string& file, const string& line,
                                                 const TypePtr& type, const StringList& meta)
{
    static const string prefix = "python:";

    for(StringList::const_iterator p = meta.begin(); p != meta.end(); ++p)
    {
        string s = *p;
        if(s.find(prefix) == 0)
        {
            string::size_type pos = s.find(':', prefix.size());
            if(pos != string::npos && s.substr(prefix.size(), pos - prefix.size()) == "seq")
            {
                static const string seqPrefix = "python:seq:";
                string arg = s.substr(seqPrefix.size(), pos - seqPrefix.size());
                if(SequencePtr::dynamicCast(type))
                {
                    if(arg == "tuple" || arg == "list" || arg == "default")
                    {
                        continue;
                    }
                }
            }
            emitWarning(file, line, "ignoring invalid metadata `" + s + "'");
        }
    }
}

void
Slice::Python::MetaDataVisitor::reject(const ContainedPtr& cont)
{
    StringList localMetaData = cont->getMetaData();
    static const string prefix = "python:";
    for(StringList::const_iterator p = localMetaData.begin(); p != localMetaData.end(); ++p)
    {
        if(p->find(prefix) == 0)
        {
            emitWarning(cont->file(), cont->line(), "ignoring invalid metadata `" + *p + "'");
        }
    }
}
