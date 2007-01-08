// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/Checksum.h>
#include <IceUtil/MD5.h>

using namespace std;
using namespace Slice;

namespace Slice
{

class ChecksumVisitor : public ParserVisitor
{
public:

    ChecksumVisitor(ChecksumMap&);

    virtual bool visitClassDefStart(const ClassDefPtr&);
    virtual bool visitExceptionStart(const ExceptionPtr&);
    virtual bool visitStructStart(const StructPtr&);
    virtual void visitSequence(const SequencePtr&);
    virtual void visitDictionary(const DictionaryPtr&);
    virtual void visitEnum(const EnumPtr&);
    virtual void visitConst(const ConstPtr&);

private:

    string typeToString(const TypePtr&);
    void updateMap(const string&, const string&);

    ChecksumMap& _map;
};

}

Slice::ChecksumVisitor::ChecksumVisitor(ChecksumMap& m) :
    _map(m)
{
}

bool
Slice::ChecksumVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    ClassList bases = p->bases();

    ostringstream ostr;

    if(p->isInterface())
    {
        ostr << "interface ";
    }
    else
    {
        ostr << "class ";
    }
    ostr << p->name();

    if(!bases.empty())
    {
        if(!bases.front()->isInterface())
        {
            ostr << " extends " << bases.front()->scoped();
            bases.erase(bases.begin());
        }
        if(!bases.empty())
        {
            if(p->isInterface())
            {
                ostr << " extends ";
            }
            else
            {
                ostr << " implements ";
            }
            for(ClassList::iterator q = bases.begin(); q != bases.end(); ++q)
            {
                if(q != bases.begin())
                {
                    ostr << ", ";
                }
                ostr << (*q)->scoped();
            }
        }
    }
    ostr << endl;

    if(p->hasDataMembers())
    {
        DataMemberList members = p->dataMembers();
        for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
        {
            ostr << typeToString((*q)->type()) << ' ' << (*q)->name() << endl;
        }
    }

    if(p->hasOperations())
    {
        OperationList ops = p->operations();
        for(OperationList::iterator q = ops.begin(); q != ops.end(); ++q)
        {
            ostr << typeToString((*q)->returnType()) << ' ' << (*q)->name() << '(';
            ParamDeclList params = (*q)->parameters();
            for(ParamDeclList::iterator r = params.begin(); r != params.end(); ++r)
            {
                if(r != params.begin())
                {
                    ostr << ", ";
                }
                if((*r)->isOutParam())
                {
                    ostr << "out ";
                }
                ostr << typeToString((*r)->type()) << ' ' << (*r)->name();
            }
            ostr << ')';
            ExceptionList ex = (*q)->throws();
            if(!ex.empty())
            {
                ostr << " throws ";
                for(ExceptionList::iterator s = ex.begin(); s != ex.end(); ++s)
                {
                    if(s != ex.begin())
                    {
                        ostr << ", ";
                    }
                    ostr << (*s)->scoped();
                }
            }
            ostr << endl;
        }
    }

    updateMap(p->scoped(), ostr.str());

    return false;
}

bool
Slice::ChecksumVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    ExceptionPtr base = p->base();

    ostringstream ostr;

    ostr << "exception " << p->name();
    if(base)
    {
        ostr << " extends " << base->scoped();
    }
    ostr << endl;

    DataMemberList members = p->dataMembers();
    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        ostr << typeToString((*q)->type()) << ' ' << (*q)->name() << endl;
    }

    updateMap(p->scoped(), ostr.str());

    return false;
}

bool
Slice::ChecksumVisitor::visitStructStart(const StructPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    ostringstream ostr;

    ostr << "struct " << p->name() << endl;

    DataMemberList members = p->dataMembers();
    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        ostr << typeToString((*q)->type()) << ' ' << (*q)->name() << endl;
    }

    updateMap(p->scoped(), ostr.str());

    return false;
}

void
Slice::ChecksumVisitor::visitSequence(const SequencePtr& p)
{
    if(p->isLocal())
    {
        return;
    }

    ostringstream ostr;
    ostr << "sequence<" << typeToString(p->type()) << "> " << p->name() << endl;
    updateMap(p->scoped(), ostr.str());
}

void
Slice::ChecksumVisitor::visitDictionary(const DictionaryPtr& p)
{
    if(p->isLocal())
    {
        return;
    }

    ostringstream ostr;
    ostr << "dictionary<" << typeToString(p->keyType()) << ", " << typeToString(p->valueType()) << "> " << p->name()
         << endl;
    updateMap(p->scoped(), ostr.str());
}

void
Slice::ChecksumVisitor::visitEnum(const EnumPtr& p)
{
    if(p->isLocal())
    {
        return;
    }

    ostringstream ostr;

    ostr << "enum " << p->name() << endl;

    EnumeratorList enums = p->getEnumerators();
    for(EnumeratorList::iterator q = enums.begin(); q != enums.end(); ++q)
    {
        ostr << (*q)->name() << endl;
    }

    updateMap(p->scoped(), ostr.str());
}

void
Slice::ChecksumVisitor::visitConst(const ConstPtr& p)
{
    ostringstream ostr;
    ostr << "const " << typeToString(p->type()) << ' ' << p->name() << " = " << p->value() << endl;
    updateMap(p->scoped(), ostr.str());
}

string
Slice::ChecksumVisitor::typeToString(const TypePtr& type)
{
    static const char* builtinTable[] =
    {
        "byte",
        "boolean",
        "short",
        "int",
        "long",
        "float",
        "double",
        "string",
        "Object",
        "Object*",
        "LocalObject"
    };

    if(!type)
    {
        return "void";
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        return builtinTable[builtin->kind()];
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return proxy->_class()->scoped() + "*";
    }

    ContainedPtr cont = ContainedPtr::dynamicCast(type);
    assert(cont);
    return cont->scoped();
}

void
Slice::ChecksumVisitor::updateMap(const string& scoped, const string& data)
{
    IceUtil::MD5 md5(reinterpret_cast<const unsigned char*>(data.c_str()), static_cast<int>(data.size()));
    vector<unsigned char> bytes;
    bytes.resize(16);
    md5.getDigest(reinterpret_cast<unsigned char*>(&bytes[0]));
    _map.insert(ChecksumMap::value_type(scoped, bytes));
}

Slice::ChecksumMap
Slice::createChecksums(const UnitPtr& u)
{
    ChecksumMap result;

    ChecksumVisitor visitor(result);
    u->visit(&visitor, false);

    return result;
}
