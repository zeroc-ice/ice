// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FreezeScript/Print.h>
#include <FreezeScript/Util.h>
#include <IceUtil/OutputUtil.h>

using namespace std;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace FreezeScript
{

class PrintVisitor : public DataVisitor
{
public:

    PrintVisitor(ostream&);
    ~PrintVisitor();

    virtual void visitBoolean(const BooleanDataPtr&);
    virtual void visitInteger(const IntegerDataPtr&);
    virtual void visitDouble(const DoubleDataPtr&);
    virtual void visitString(const StringDataPtr&);
    virtual void visitProxy(const ProxyDataPtr&);
    virtual void visitStruct(const StructDataPtr&);
    virtual void visitSequence(const SequenceDataPtr&);
    virtual void visitEnum(const EnumDataPtr&);
    virtual void visitDictionary(const DictionaryDataPtr&);
    virtual void visitObject(const ObjectRefPtr&);

private:

    void printObjectMembers(const Slice::ClassDefPtr&, const DataMemberMap&);

    IceUtilInternal::Output _out;
    typedef map<const ObjectData*, int> ObjectHistoryMap;
    ObjectHistoryMap _objectHistory;
    int _objectCount;
};

} // End of namespace FreezeScript

//
// printData
//
void
FreezeScript::printData(const DataPtr& data, ostream& os)
{
    PrintVisitor visitor(os);
    data->visit(visitor);
}

//
// PrintVisitor
//
FreezeScript::PrintVisitor::PrintVisitor(ostream& os) :
    _out(os), _objectCount(0)
{
}

FreezeScript::PrintVisitor::~PrintVisitor()
{
}

void
FreezeScript::PrintVisitor::visitBoolean(const BooleanDataPtr& data)
{
    _out << "bool(" << data->toString() << ")";
}

void
FreezeScript::PrintVisitor::visitInteger(const IntegerDataPtr& data)
{
    _out << typeToString(data->getType()) << "(" << data->toString() << ")";
}

void
FreezeScript::PrintVisitor::visitDouble(const DoubleDataPtr& data)
{
    _out << typeToString(data->getType()) << "(" << data->toString() << ")";
}

void
FreezeScript::PrintVisitor::visitString(const StringDataPtr& data)
{
    _out << "'" << data->toString() << "'";
}

void
FreezeScript::PrintVisitor::visitProxy(const ProxyDataPtr& data)
{
    string str = data->toString();
    _out << typeToString(data->getType()) << '(' << (str.empty() ? string("nil") : str) << ')';
}

void
FreezeScript::PrintVisitor::visitStruct(const StructDataPtr& data)
{
    Slice::StructPtr type = Slice::StructPtr::dynamicCast(data->getType());
    assert(type);
    _out << "struct " << typeToString(type);
    _out << sb;
    //
    // Print members in order of declaration.
    //
    Slice::DataMemberList l = type->dataMembers();
    DataMemberMap members = data->getMembers();
    for(Slice::DataMemberList::const_iterator p = l.begin(); p != l.end(); ++p)
    {
        DataMemberMap::const_iterator q = members.find((*p)->name());
        assert(q != members.end());
        _out << nl;
        _out << q->first << " = ";
        q->second->visit(*this);
    }
    _out << eb;
}

void
FreezeScript::PrintVisitor::visitSequence(const SequenceDataPtr& data)
{
    DataList& elements = data->getElements();
    _out << "sequence " << typeToString(data->getType()) << " (size = " << elements.size() << ")";
    _out << sb;
    Ice::Int i = 0;
    for(DataList::const_iterator p = elements.begin(); p != elements.end(); ++p, ++i)
    {
        _out << nl << "[" << i << "] = ";
        (*p)->visit(*this);
    }
    _out << eb;
}

void
FreezeScript::PrintVisitor::visitEnum(const EnumDataPtr& data)
{
    _out << typeToString(data->getType()) << "(" << data->toString() << ")";
}

void
FreezeScript::PrintVisitor::visitDictionary(const DictionaryDataPtr& data)
{
    DataMap& map = data->getElements();
    _out << "dictionary " << typeToString(data->getType()) << " (size = " << map.size() << ")";
    _out << sb;
    for(DataMap::const_iterator p = map.begin(); p != map.end(); ++p)
    {
        _out << nl;
        p->first->visit(*this);
        _out << " => ";
        p->second->visit(*this);
    }
    _out << eb;
}

void
FreezeScript::PrintVisitor::visitObject(const ObjectRefPtr& data)
{
    ObjectDataPtr value = data->getValue();
    if(!value)
    {
        _out << typeToString(data->getType()) << "(nil)";
    }
    else
    {
        Slice::TypePtr type = value->getType();
        Slice::ClassDeclPtr decl = Slice::ClassDeclPtr::dynamicCast(type); // May be nil for Ice::Object

        _out << "class " << typeToString(type);
        ObjectHistoryMap::iterator p = _objectHistory.find(value.get());
        if(p != _objectHistory.end())
        {
            ostringstream ostr;
            ostr << " (object #" << p->second << ")";
            _out << ostr.str();
        }
        else
        {
            _objectHistory.insert(ObjectHistoryMap::value_type(value.get(), _objectCount));
            ostringstream ostr;
            ostr << " (object #" << _objectCount << ")";
            _out << ostr.str();
            _objectCount++;
            _out << sb;

            DataMemberMap members = value->getMembers();

            //
            // If Ice::Object had any members, this is where we would print them.
            //

            if(decl)
            {
                Slice::ClassDefPtr def = decl->definition();
                assert(def);
                printObjectMembers(def, members);
            }

            _out << eb;
        }
    }
}

void
FreezeScript::PrintVisitor::printObjectMembers(const Slice::ClassDefPtr& type, const DataMemberMap& members)
{
    Slice::ClassList bases = type->bases();
    if(!bases.empty() && !bases.front()->isAbstract())
    {
        printObjectMembers(bases.front(), members);
    }

    //
    // Print members in order of declaration.
    //
    Slice::DataMemberList l = type->dataMembers();
    for(Slice::DataMemberList::const_iterator p = l.begin(); p != l.end(); ++p)
    {
        DataMemberMap::const_iterator q = members.find((*p)->name());
        assert(q != members.end());
        _out << nl;
        _out << q->first << " = ";
        q->second->visit(*this);
    }
}
