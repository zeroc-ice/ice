// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Transform/Data.h>
#include <Transform/TransformUtil.h>
#include <IceUtil/InputUtil.h>

using namespace std;
using namespace IceUtil;

namespace Transform
{

//
// ObjectWriter
//
class ObjectWriter : public Ice::Object
{
public:

    ObjectWriter(const ObjectDataPtr&);

    virtual void __write(IceInternal::BasicStream*, bool) const;
    virtual void __read(IceInternal::BasicStream*, bool = true);

private:

    ObjectDataPtr _value;
};

//
// ObjectReader
//
class ObjectReader : public Ice::Object
{
public:

    ObjectReader(const DataFactoryPtr&, const Slice::TypePtr&);

    virtual void __write(IceInternal::BasicStream*, bool) const;
    virtual void __read(IceInternal::BasicStream*, bool = true);

    ObjectDataPtr getValue() const;

private:

    DataFactoryPtr _factory;
    Slice::TypePtr _type;
    ObjectDataPtr _value;
};
typedef IceUtil::Handle<ObjectReader> ObjectReaderPtr;

} // End of namespace Transform

//
// ObjectWriter
//
Transform::ObjectWriter::ObjectWriter(const ObjectDataPtr& value) :
    _value(value)
{
}

void
Transform::ObjectWriter::__write(IceInternal::BasicStream* os, bool) const
{
    Slice::ClassDeclPtr decl = Slice::ClassDeclPtr::dynamicCast(_value->_type);
    Slice::ClassDefPtr type;
    if(decl)
    {
        type = decl->definition();
    }
    while(type)
    {
        os->writeTypeId(type->scoped());
        os->startWriteSlice();
        Slice::DataMemberList members = type->dataMembers();
        for(Slice::DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
        {
            DataMemberMap::const_iterator q = _value->_members.find((*p)->name());
            assert(q != _value->_members.end());
            q->second->marshal(*os);
        }
        os->endWriteSlice();

        Slice::ClassList bases = type->bases();
        if(!bases.empty() && !bases.front()->isInterface())
        {
            type = bases.front();
        }
        else
        {
            type = 0;
        }
    }

    //
    // Ice::Object slice
    //
    os->writeTypeId(Ice::Object::ice_staticId());
    os->startWriteSlice();
    _value->_facetMap->marshal(*os);
    os->endWriteSlice();
}

void
Transform::ObjectWriter::__read(IceInternal::BasicStream*, bool)
{
    assert(false);
}

//
// ObjectReader
//
Transform::ObjectReader::ObjectReader(const DataFactoryPtr& factory, const Slice::TypePtr& type) :
    _factory(factory), _type(type)
{
}

void
Transform::ObjectReader::__write(IceInternal::BasicStream*, bool) const
{
    assert(false);
}

void
Transform::ObjectReader::__read(IceInternal::BasicStream* is, bool rid)
{
    const_cast<ObjectDataPtr&>(_value) = new ObjectData(_factory, _type, true);
    Slice::ClassDeclPtr decl = Slice::ClassDeclPtr::dynamicCast(_type);
    Slice::ClassDefPtr type;
    if(decl)
    {
        type = decl->definition();
    }
    while(type)
    {
        if(rid)
        {
            string id;
            is->readTypeId(id);
            assert(id == type->scoped());
        }

        is->startReadSlice();
        Slice::DataMemberList members = type->dataMembers();
        for(Slice::DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
        {
            DataMemberMap::iterator q = _value->_members.find((*p)->name());
            assert(q != _value->_members.end());
            q->second->unmarshal(*is);
        }
        is->endReadSlice();

        Slice::ClassList bases = type->bases();
        if(!bases.empty() && !bases.front()->isInterface())
        {
            type = bases.front();
        }
        else
        {
            type = 0;
        }

        rid = true;
    }

    //
    // Ice::Object slice
    //
    if(rid)
    {
        string id;
        is->readTypeId(id);
        if(id != Ice::Object::ice_staticId())
        {
            throw Ice::MarshalException(__FILE__, __LINE__);
        }
    }
    is->startReadSlice();
    _value->_facetMap->unmarshal(*is);
    is->endReadSlice();
}

Transform::ObjectDataPtr
Transform::ObjectReader::getValue() const
{
    return _value;
}

//
// DataInterceptor
//
Transform::DataInterceptor::~DataInterceptor()
{
}

//
// DataInitializer
//
Transform::DataInitializer::~DataInitializer()
{
}

//
// DataFactory
//
Transform::DataFactory::DataFactory(const Ice::CommunicatorPtr& communicator, const Slice::UnitPtr& unit,
                                    const ErrorReporterPtr& errorReporter) :
    _communicator(communicator), _unit(unit), _errorReporter(errorReporter), _initializersEnabled(true)
{
}

Transform::DataPtr
Transform::DataFactory::create(const Slice::TypePtr& type, bool readOnly)
{
    DataPtr data = createImpl(type, readOnly);
    if(!readOnly)
    {
        initialize(data);
    }
    return data;
}

Transform::DataPtr
Transform::DataFactory::createBoolean(bool b, bool readOnly)
{
    DataPtr data = new BooleanData(getBuiltin(Slice::Builtin::KindBool), _errorReporter, readOnly, b);
    if(!readOnly)
    {
        initialize(data);
    }
    return data;
}

Transform::DataPtr
Transform::DataFactory::createInteger(Ice::Long i, bool readOnly)
{
    DataPtr data = new IntegerData(getBuiltin(Slice::Builtin::KindLong), _errorReporter, readOnly, i);
    if(!readOnly)
    {
        initialize(data);
    }
    return data;
}

Transform::DataPtr
Transform::DataFactory::createDouble(double d, bool readOnly)
{
    DataPtr data = new DoubleData(getBuiltin(Slice::Builtin::KindDouble), _errorReporter, readOnly, d);
    if(!readOnly)
    {
        initialize(data);
    }
    return data;
}

Transform::DataPtr
Transform::DataFactory::createString(const string& s, bool readOnly)
{
    DataPtr data = new StringData(this, getBuiltin(Slice::Builtin::KindString), _errorReporter, readOnly, s);
    if(!readOnly)
    {
        initialize(data);
    }
    return data;
}

Transform::DataPtr
Transform::DataFactory::createNil(bool readOnly)
{
    DataPtr data = new ObjectRef(this, getBuiltin(Slice::Builtin::KindObject), readOnly);
    if(!readOnly)
    {
        initialize(data);
    }
    return data;
}

Transform::DataPtr
Transform::DataFactory::createObject(const Slice::TypePtr& type, bool readOnly)
{
    ObjectRefPtr obj;
    Slice::ClassDeclPtr cl = Slice::ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        Slice::ClassDefPtr def = cl->definition();
        if(!def)
        {
            _errorReporter->error("class " + cl->scoped() + " declared but not defined");
        }
        obj = new ObjectRef(this, cl, readOnly);
    }
    else
    {
        Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(type);
        if(b && b->kind() == Slice::Builtin::KindObject)
        {
            obj = new ObjectRef(this, b, readOnly);
        }
    }

    if(!obj)
    {
        _errorReporter->error("type `" + typeName(type) + "' is not a class");
    }

    obj->instantiate();

    if(!readOnly)
    {
        initialize(obj);
    }

    return obj;
}

Slice::BuiltinPtr
Transform::DataFactory::getBuiltin(Slice::Builtin::Kind kind) const
{
    return _unit->builtin(kind);
}

void
Transform::DataFactory::addInitializer(const string& type, const DataInitializerPtr& init)
{
    _initializers.insert(InitMap::value_type(type, init));
}

void
Transform::DataFactory::enableInitializers()
{
    _initializersEnabled = true;
}

void
Transform::DataFactory::disableInitializers()
{
    _initializersEnabled = false;
}

Transform::ErrorReporterPtr
Transform::DataFactory::getErrorReporter() const
{
    return _errorReporter;
}

Transform::DataPtr
Transform::DataFactory::createImpl(const Slice::TypePtr& type, bool readOnly)
{
    Slice::BuiltinPtr builtin = Slice::BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
        case Slice::Builtin::KindBool:
            return new BooleanData(builtin, _errorReporter, readOnly);

        case Slice::Builtin::KindByte:
        case Slice::Builtin::KindShort:
        case Slice::Builtin::KindInt:
        case Slice::Builtin::KindLong:
            return new IntegerData(builtin, _errorReporter, readOnly);

        case Slice::Builtin::KindFloat:
        case Slice::Builtin::KindDouble:
            return new DoubleData(builtin, _errorReporter, readOnly);

        case Slice::Builtin::KindString:
            return new StringData(this, builtin, _errorReporter, readOnly);

        case Slice::Builtin::KindObject:
            return new ObjectRef(this, builtin, readOnly);

        case Slice::Builtin::KindObjectProxy:
            return new ProxyData(builtin, _communicator, _errorReporter, readOnly);

        case Slice::Builtin::KindLocalObject:
            assert(false);
            return 0;
        }
    }

    Slice::SequencePtr seq = Slice::SequencePtr::dynamicCast(type);
    if(seq)
    {
        return new SequenceData(this, seq, _errorReporter, readOnly);
    }

    Slice::ProxyPtr proxy = Slice::ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return new ProxyData(proxy, _communicator, _errorReporter, readOnly);
    }

    Slice::StructPtr st = Slice::StructPtr::dynamicCast(type);
    if(st)
    {
        return new StructData(this, st, _errorReporter, readOnly);
    }

    Slice::EnumPtr en = Slice::EnumPtr::dynamicCast(type);
    if(en)
    {
        return new EnumData(en, _errorReporter, readOnly);
    }

    Slice::DictionaryPtr dict = Slice::DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        return new DictionaryData(this, dict, _errorReporter, readOnly);
    }

    Slice::ClassDeclPtr cl = Slice::ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        Slice::ClassDefPtr def = cl->definition();
        if(!def)
        {
            _errorReporter->error("class " + cl->scoped() + " declared but not defined");
        }
        return new ObjectRef(this, cl, readOnly);
    }

    return 0;
}

void
Transform::DataFactory::initialize(const DataPtr& data)
{
    if(_initializersEnabled)
    {
        string name = typeName(data->getType());
        InitMap::iterator p = _initializers.find(name);
        if(p != _initializers.end())
        {
            p->second->initialize(this, data, _communicator);
        }
    }
}

//
// Data
//
Transform::Data::Data(const ErrorReporterPtr& errorReporter, bool readOnly) :
    _errorReporter(errorReporter), _readOnly(readOnly)
{
}

Transform::Data::~Data()
{
}

bool
Transform::Data::readOnly() const
{
    return _readOnly;
}

void
Transform::Data::print(ostream& os) const
{
    ObjectDataHistory history;
    IceUtil::Output out(os);
    printI(out, history);
}

bool
Transform::Data::isCompatible(const Slice::TypePtr& dest, const Slice::TypePtr& src, DataInterceptor& interceptor)
{
    Slice::BuiltinPtr b1 = Slice::BuiltinPtr::dynamicCast(dest);
    if(b1)
    {
        Slice::BuiltinPtr b2 = Slice::BuiltinPtr::dynamicCast(src);
        switch(b1->kind())
        {
        case Slice::Builtin::KindByte:
        case Slice::Builtin::KindShort:
        case Slice::Builtin::KindInt:
        case Slice::Builtin::KindLong:
        {
            if(b2)
            {
                switch(b2->kind())
                {
                case Slice::Builtin::KindByte:
                case Slice::Builtin::KindShort:
                case Slice::Builtin::KindInt:
                case Slice::Builtin::KindLong:
                case Slice::Builtin::KindString:
                {
                    return true;
                }
                case Slice::Builtin::KindBool:
                case Slice::Builtin::KindFloat:
                case Slice::Builtin::KindDouble:
                case Slice::Builtin::KindObject:
                case Slice::Builtin::KindObjectProxy:
                case Slice::Builtin::KindLocalObject:
                {
                    return false;
                }
                }
            }

            return false;
        }
        case Slice::Builtin::KindBool:
        {
            if(b2 && (b2->kind() == Slice::Builtin::KindBool || b2->kind() == Slice::Builtin::KindString))
            {
                return true;
            }

            return false;
        }
        case Slice::Builtin::KindFloat:
        case Slice::Builtin::KindDouble:
        {
            if(b2)
            {
                switch(b2->kind())
                {
                case Slice::Builtin::KindFloat:
                case Slice::Builtin::KindDouble:
                case Slice::Builtin::KindString:
                {
                    return true;
                }
                case Slice::Builtin::KindByte:
                case Slice::Builtin::KindShort:
                case Slice::Builtin::KindInt:
                case Slice::Builtin::KindLong:
                case Slice::Builtin::KindBool:
                case Slice::Builtin::KindObject:
                case Slice::Builtin::KindObjectProxy:
                case Slice::Builtin::KindLocalObject:
                {
                    return false;
                }
                }
            }

            return false;
        }
        case Slice::Builtin::KindString:
        {
            if(b2)
            {
                switch(b2->kind())
                {
                case Slice::Builtin::KindByte:
                case Slice::Builtin::KindBool:
                case Slice::Builtin::KindShort:
                case Slice::Builtin::KindInt:
                case Slice::Builtin::KindLong:
                case Slice::Builtin::KindFloat:
                case Slice::Builtin::KindDouble:
                case Slice::Builtin::KindString:
                case Slice::Builtin::KindObjectProxy:
                {
                    return true;
                }
                case Slice::Builtin::KindObject:
                case Slice::Builtin::KindLocalObject:
                {
                    return false;
                }
                }

                return false;
            }
            else if(Slice::EnumPtr::dynamicCast(src))
            {
                return true;
            }
            else if(Slice::ProxyPtr::dynamicCast(src))
            {
                return true;
            }

            return false;
        }
        case Slice::Builtin::KindObject:
        {
            //
            // Allow transformation from Object to class. Validation has to
            // be done during transformation, when the actual type of
            // an instance can be compared for compatibility with the
            // new type.
            //
            Slice::ClassDeclPtr cl = Slice::ClassDeclPtr::dynamicCast(src);
            if(cl || (b2 && b2->kind() == Slice::Builtin::KindObject))
            {
                return true;
            }

            return false;
        }
        case Slice::Builtin::KindObjectProxy:
        {
            Slice::ProxyPtr p = Slice::ProxyPtr::dynamicCast(src);
            if(p || (b2 && b2->kind() == Slice::Builtin::KindObjectProxy) ||
               (b2 && b2->kind() == Slice::Builtin::KindString))
            {
                return true;
            }

            return false;
        }
        case Slice::Builtin::KindLocalObject:
        {
            assert(false);
            return false;
        }
        }

        assert(false);
    }

    Slice::ClassDeclPtr cl1 = Slice::ClassDeclPtr::dynamicCast(dest);
    if(cl1)
    {
        Slice::ClassDeclPtr cl2 = Slice::ClassDeclPtr::dynamicCast(src);
        if(cl2 && checkClasses(cl1, cl2, interceptor))
        {
            return true;
        }

        return false;
    }

    Slice::StructPtr s1 = Slice::StructPtr::dynamicCast(dest);
    if(s1)
    {
        Slice::TypePtr s = interceptor.getRename(src);
        if(!s)
        {
            s = src;
        }
        Slice::StructPtr s2 = Slice::StructPtr::dynamicCast(s);
        if(s2 && s1->scoped() == s2->scoped())
        {
            return true;
        }

        return false;
    }

    Slice::ProxyPtr p1 = Slice::ProxyPtr::dynamicCast(dest);
    if(p1)
    {
        Slice::BuiltinPtr b2 = Slice::BuiltinPtr::dynamicCast(src);
        if(b2 && (b2->kind() == Slice::Builtin::KindObjectProxy || b2->kind() == Slice::Builtin::KindString))
        {
            return true;
        }

        Slice::ProxyPtr p2 = Slice::ProxyPtr::dynamicCast(src);
        if(p2 && checkClasses(p1->_class(), p2->_class(), interceptor))
        {
            return true;
        }

        return false;
    }

    Slice::DictionaryPtr d1 = Slice::DictionaryPtr::dynamicCast(dest);
    if(d1)
    {
        Slice::DictionaryPtr d2 = Slice::DictionaryPtr::dynamicCast(src);
        if(d2)
        {
            return isCompatible(d1->keyType(), d2->keyType(), interceptor) &&
                   isCompatible(d1->valueType(), d2->valueType(), interceptor);
        }

        return false;
    }

    Slice::SequencePtr seq1 = Slice::SequencePtr::dynamicCast(dest);
    if(seq1)
    {
        Slice::SequencePtr seq2 = Slice::SequencePtr::dynamicCast(src);
        if(seq2)
        {
            return isCompatible(seq1->type(), seq2->type(), interceptor);
        }

        return false;
    }

    Slice::EnumPtr e1 = Slice::EnumPtr::dynamicCast(dest);
    if(e1)
    {
        Slice::BuiltinPtr b2 = Slice::BuiltinPtr::dynamicCast(src);
        if(b2 && b2->kind() == Slice::Builtin::KindString)
        {
            return true;
        }

        Slice::TypePtr s = interceptor.getRename(src);
        if(!s)
        {
            s = src;
        }
        Slice::EnumPtr e2 = Slice::EnumPtr::dynamicCast(s);
        if(e2 && e1->scoped() == e2->scoped())
        {
            return true;
        }

        return false;
    }

    assert(false);
    return false;
}

bool
Transform::Data::checkClasses(const Slice::ClassDeclPtr& dest, const Slice::ClassDeclPtr& src,
                              DataInterceptor& interceptor)
{
    //
    // Here are the rules for verifying class compatibility:
    //
    // 1. If the type ids are the same, assume they are compatible.
    // 2. If the source type has been renamed, then check its equivalent new definition for compatibility.
    // 3. Otherwise, the types are only compatible if they are defined in the same Slice unit, and if the
    //    destination type is a base type of the source type.
    //
    string s1 = dest->scoped();
    string s2 = src->scoped();
    if(s1 == s2)
    {
        return true;
    }
    else
    {
        Slice::TypePtr t = interceptor.getRename(src);
        Slice::ClassDeclPtr s = Slice::ClassDeclPtr::dynamicCast(t);
        if(s)
        {
            return checkClasses(dest, s, interceptor);
        }

        if(dest->unit().get() != src->unit().get())
        {
            Slice::TypeList l = dest->unit()->lookupTypeNoBuiltin(s2, false);
            if(l.empty())
            {
                _errorReporter->error("class " + s2 + " not found in new Slice definitions");
            }
            s = Slice::ClassDeclPtr::dynamicCast(l.front());
        }
        else
        {
            s = src;
        }

        if(s)
        {
            Slice::ClassDefPtr def = s->definition();
            if(!def)
            {
                _errorReporter->error("class " + s2 + " declared but not defined");
            }
            Slice::ClassList bases = def->allBases();
            for(Slice::ClassList::iterator p = bases.begin(); p != bases.end(); ++p)
            {
                if((*p)->scoped() == s1)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

//
// PrimitiveData
//
Transform::PrimitiveData::PrimitiveData(const ErrorReporterPtr& errorReporter, bool readOnly) :
    Data(errorReporter, readOnly)
{
}

Transform::DataPtr
Transform::PrimitiveData::getMember(const string&) const
{
    return 0;
}

Transform::DataPtr
Transform::PrimitiveData::getElement(const DataPtr&) const
{
    _errorReporter->error("element requested of primitive value");
    return 0;
}

void
Transform::PrimitiveData::registerObjects(ObjectDataMap&) const
{
}

void
Transform::PrimitiveData::destroy()
{
}

//
// BooleanData
//
Transform::BooleanData::BooleanData(const Slice::BuiltinPtr& type, const ErrorReporterPtr& errorReporter,
                                    bool readOnly) :
    PrimitiveData(errorReporter, readOnly), _type(type), _value(false)
{
}

Transform::BooleanData::BooleanData(const Slice::BuiltinPtr& type, const ErrorReporterPtr& errorReporter,
                                    bool readOnly, bool value) :
    PrimitiveData(errorReporter, readOnly), _type(type), _value(value)
{
}

bool
Transform::BooleanData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
Transform::BooleanData::operator!=(const Data& rhs) const
{
    const BooleanData* b = dynamic_cast<const BooleanData*>(&rhs);
    if(!b)
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _value != b->_value;
}

bool
Transform::BooleanData::operator<(const Data& rhs) const
{
    const BooleanData* b = dynamic_cast<const BooleanData*>(&rhs);
    if(!b)
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _value < b->_value;
}

void
Transform::BooleanData::transform(const DataPtr& data, DataInterceptor& interceptor)
{
    if(interceptor.preTransform(this, data))
    {
        BooleanDataPtr b = BooleanDataPtr::dynamicCast(data);
        StringDataPtr s = StringDataPtr::dynamicCast(data);
        if(b)
        {
            _value = b->booleanValue();
        }
        else if(s)
        {
            string v = s->stringValue();
            if(v == "true")
            {
                _value = true;
            }
            else if(v == "false")
            {
                _value = false;
            }
            else
            {
                _errorReporter->conversionError(v, _type);
            }
        }
        else
        {
            _errorReporter->typeMismatchError(_type, data->getType(), false);
        }
    }
    interceptor.postTransform(this, data);
}

void
Transform::BooleanData::set(const DataPtr& value, DataInterceptor&, bool convert)
{
    StringDataPtr s = StringDataPtr::dynamicCast(value);
    if(s)
    {
        string v = s->stringValue();
        if(v == "true")
        {
            _value = true;
        }
        else if(v == "false")
        {
            _value = false;
        }
        else
        {
            _errorReporter->conversionError(v, _type);
        }
    }
    else
    {
        _value = value->booleanValue(convert);
    }
}

Transform::DataPtr
Transform::BooleanData::clone() const
{
    return new BooleanData(_type, _errorReporter, _readOnly, _value);
}

Slice::TypePtr
Transform::BooleanData::getType() const
{
    return _type;
}

void
Transform::BooleanData::marshal(IceInternal::BasicStream& os) const
{
    os.write(_value);
}

void
Transform::BooleanData::unmarshal(IceInternal::BasicStream& is)
{
    is.read(_value);
}

bool
Transform::BooleanData::booleanValue(bool) const
{
    return _value;
}

Ice::Long
Transform::BooleanData::integerValue(bool) const
{
    _errorReporter->error("boolean cannot be converted to integer");
    return 0;
}

double
Transform::BooleanData::doubleValue(bool) const
{
    _errorReporter->error("boolean cannot be converted to double");
    return 0;
}

string
Transform::BooleanData::stringValue(bool convert) const
{
    if(convert)
    {
        return toString();
    }
    else
    {
        return string();
    }
}

string
Transform::BooleanData::toString() const
{
    return (_value ? "true" : "false");
}

void
Transform::BooleanData::printI(IceUtil::Output& out, ObjectDataHistory&) const
{
    out << "bool(" << toString() << ")";
}

//
// IntegerData
//
Transform::IntegerData::IntegerData(const Slice::BuiltinPtr& type, const ErrorReporterPtr& errorReporter,
                                    bool readOnly) :
    PrimitiveData(errorReporter, readOnly), _type(type), _value(0)
{
}

Transform::IntegerData::IntegerData(const Slice::BuiltinPtr& type, const ErrorReporterPtr& errorReporter,
                                    bool readOnly, Ice::Long value) :
    PrimitiveData(errorReporter, readOnly), _type(type), _value(value)
{
}

bool
Transform::IntegerData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
Transform::IntegerData::operator!=(const Data& rhs) const
{
    const IntegerData* i = dynamic_cast<const IntegerData*>(&rhs);
    if(!i)
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _value != i->_value;
}

bool
Transform::IntegerData::operator<(const Data& rhs) const
{
    const IntegerData* i = dynamic_cast<const IntegerData*>(&rhs);
    if(!i)
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _value < i->_value;
}

void
Transform::IntegerData::transform(const DataPtr& data, DataInterceptor& interceptor)
{
    if(interceptor.preTransform(this, data))
    {
        Ice::Long value;
        IntegerDataPtr i = IntegerDataPtr::dynamicCast(data);
        StringDataPtr s = StringDataPtr::dynamicCast(data);
        if(i)
        {
            value = i->_value;
        }
        else if(s)
        {
            string str = s->stringValue();
            string::size_type pos;
            if(!IceUtil::stringToInt64(str, value, pos))
            {
                _errorReporter->conversionError(str, _type);
            }
        }
        else
        {
            _errorReporter->typeMismatchError(_type, data->getType(), false);
        }

        if(rangeCheck(value))
        {
            _value = value;
        }
    }
    interceptor.postTransform(this, data);
}

void
Transform::IntegerData::set(const DataPtr& value, DataInterceptor&, bool convert)
{
    _value = value->integerValue(convert);
}

Transform::DataPtr
Transform::IntegerData::clone() const
{
    return new IntegerData(_type, _errorReporter, _readOnly, _value);
}

Slice::TypePtr
Transform::IntegerData::getType() const
{
    return _type;
}

void
Transform::IntegerData::marshal(IceInternal::BasicStream& os) const
{
    rangeCheck(_value);

    switch(_type->kind())
    {
    case Slice::Builtin::KindByte:
    {
        os.write(static_cast<Ice::Byte>(_value));
        break;
    }
    case Slice::Builtin::KindShort:
    {
        os.write(static_cast<Ice::Short>(_value));
        break;
    }
    case Slice::Builtin::KindInt:
    {
        os.write(static_cast<Ice::Int>(_value));
        break;
    }
    case Slice::Builtin::KindLong:
    {
        os.write(_value);
        break;
    }

    case Slice::Builtin::KindBool:
    case Slice::Builtin::KindFloat:
    case Slice::Builtin::KindDouble:
    case Slice::Builtin::KindString:
    case Slice::Builtin::KindObject:
    case Slice::Builtin::KindObjectProxy:
    case Slice::Builtin::KindLocalObject:
        assert(false);
    }
}

void
Transform::IntegerData::unmarshal(IceInternal::BasicStream& is)
{
    switch(_type->kind())
    {
    case Slice::Builtin::KindByte:
    {
        Ice::Byte val;
        is.read(val);
        _value = val & 0xff;
        break;
    }
    case Slice::Builtin::KindShort:
    {
        Ice::Short val;
        is.read(val);
        _value = val;
        break;
    }
    case Slice::Builtin::KindInt:
    {
        Ice::Int val;
        is.read(val);
        _value = val;
        break;
    }
    case Slice::Builtin::KindLong:
    {
        is.read(_value);
        break;
    }

    case Slice::Builtin::KindBool:
    case Slice::Builtin::KindFloat:
    case Slice::Builtin::KindDouble:
    case Slice::Builtin::KindString:
    case Slice::Builtin::KindObject:
    case Slice::Builtin::KindObjectProxy:
    case Slice::Builtin::KindLocalObject:
        assert(false);
    }
}

bool
Transform::IntegerData::booleanValue(bool) const
{
    _errorReporter->error("integer cannot be converted to boolean");
    return 0;
}

Ice::Long
Transform::IntegerData::integerValue(bool) const
{
    return _value;
}

double
Transform::IntegerData::doubleValue(bool convert) const
{
    if(convert)
    {
        return static_cast<double>(_value);
    }
    else
    {
        _errorReporter->error("integer cannot be converted to double");
        return 0;
    }
}

string
Transform::IntegerData::stringValue(bool convert) const
{
    if(convert)
    {
        return toString();
    }
    else
    {
        _errorReporter->error("integer cannot be converted to string");
        return string();
    }
}

string
Transform::IntegerData::toString() const
{
    return toString(_value);
}

bool
Transform::IntegerData::rangeCheck(Ice::Long value) const
{
    switch(_type->kind())
    {
    case Slice::Builtin::KindByte:
    {
        if(value < 0 || value > 255)
        {
            _errorReporter->rangeError(toString(value), _type);
        }
        else
        {
            return true;
        }
        break;
    }

    case Slice::Builtin::KindShort:
    {
        if(value < SHRT_MIN || value > SHRT_MAX)
        {
            _errorReporter->rangeError(toString(value), _type);
        }
        else
        {
            return true;
        }
        break;
    }

    case Slice::Builtin::KindInt:
    {
        if(value < INT_MIN || value > INT_MAX)
        {
            _errorReporter->rangeError(toString(value), _type);
        }
        else
        {
            return true;
        }
        break;
    }

    case Slice::Builtin::KindLong:
    {
        return true;
    }

    case Slice::Builtin::KindBool:
    case Slice::Builtin::KindFloat:
    case Slice::Builtin::KindDouble:
    case Slice::Builtin::KindString:
    case Slice::Builtin::KindObject:
    case Slice::Builtin::KindObjectProxy:
    case Slice::Builtin::KindLocalObject:
        assert(false);
    }

    return false;
}

string
Transform::IntegerData::toString(Ice::Long value)
{
    char buf[64];
#ifdef WIN32
    sprintf(buf, "%I64d", value);
#else
    sprintf(buf, "%lld", value);
#endif
    return string(buf);
}

void
Transform::IntegerData::printI(IceUtil::Output& out, ObjectDataHistory&) const
{
    out << "integer(" << toString() << ")";
}

//
// DoubleData
//
Transform::DoubleData::DoubleData(const Slice::BuiltinPtr& type, const ErrorReporterPtr& errorReporter,
                                  bool readOnly) :
    PrimitiveData(errorReporter, readOnly), _type(type), _value(0)
{
}

Transform::DoubleData::DoubleData(const Slice::BuiltinPtr& type, const ErrorReporterPtr& errorReporter,
                                  bool readOnly, double value) :
    PrimitiveData(errorReporter, readOnly), _type(type), _value(value)
{
}

bool
Transform::DoubleData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
Transform::DoubleData::operator!=(const Data& rhs) const
{
    const DoubleData* d = dynamic_cast<const DoubleData*>(&rhs);
    if(!d)
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _value != d->_value;
}

bool
Transform::DoubleData::operator<(const Data& rhs) const
{
    const DoubleData* d = dynamic_cast<const DoubleData*>(&rhs);
    if(!d)
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _value < d->_value;
}

void
Transform::DoubleData::transform(const DataPtr& data, DataInterceptor& interceptor)
{
    if(interceptor.preTransform(this, data))
    {
        DoubleDataPtr d = DoubleDataPtr::dynamicCast(data);
        StringDataPtr s = StringDataPtr::dynamicCast(data);
        if(d)
        {
            _value = d->doubleValue();
        }
        else if(s)
        {
            string str = s->stringValue();
            const char* start = str.c_str();
            char* end;
            double v = strtod(start, &end);
            if(errno == ERANGE)
            {
                _errorReporter->rangeError(str, _type);
            }
            else
            {
                while(*end)
                {
                    if(!isspace(*end))
                    {
                        _errorReporter->conversionError(str, _type);
                        break;
                    }
                    end++;
                }
                if(!*end)
                {
                    _value = v;
                }
            }
        }
        else
        {
            _errorReporter->typeMismatchError(_type, data->getType(), false);
        }
    }
    interceptor.postTransform(this, data);
}

void
Transform::DoubleData::set(const DataPtr& value, DataInterceptor&, bool convert)
{
    _value = value->doubleValue(convert);
}

Transform::DataPtr
Transform::DoubleData::clone() const
{
    return new DoubleData(_type, _errorReporter, _readOnly, _value);
}

Slice::TypePtr
Transform::DoubleData::getType() const
{
    return _type;
}

void
Transform::DoubleData::marshal(IceInternal::BasicStream& os) const
{
    switch(_type->kind())
    {
    case Slice::Builtin::KindFloat:
    {
        os.write(static_cast<Ice::Float>(_value));
        break;
    }
    case Slice::Builtin::KindDouble:
    {
        os.write(_value);
        break;
    }

    case Slice::Builtin::KindByte:
    case Slice::Builtin::KindShort:
    case Slice::Builtin::KindInt:
    case Slice::Builtin::KindLong:
    case Slice::Builtin::KindBool:
    case Slice::Builtin::KindString:
    case Slice::Builtin::KindObject:
    case Slice::Builtin::KindObjectProxy:
    case Slice::Builtin::KindLocalObject:
        assert(false);
    }
}

void
Transform::DoubleData::unmarshal(IceInternal::BasicStream& is)
{
    switch(_type->kind())
    {
    case Slice::Builtin::KindFloat:
    {
        Ice::Float val;
        is.read(val);
        _value = val;
        break;
    }
    case Slice::Builtin::KindDouble:
    {
        is.read(_value);
        break;
    }

    case Slice::Builtin::KindByte:
    case Slice::Builtin::KindShort:
    case Slice::Builtin::KindInt:
    case Slice::Builtin::KindLong:
    case Slice::Builtin::KindBool:
    case Slice::Builtin::KindString:
    case Slice::Builtin::KindObject:
    case Slice::Builtin::KindObjectProxy:
    case Slice::Builtin::KindLocalObject:
        assert(false);
    }
}

bool
Transform::DoubleData::booleanValue(bool) const
{
    _errorReporter->error("double cannot be converted to boolean");
    return 0;
}

Ice::Long
Transform::DoubleData::integerValue(bool convert) const
{
    if(convert)
    {
        return static_cast<Ice::Long>(_value);
    }
    else
    {
        _errorReporter->error("double cannot be converted to integer");
        return 0;
    }
}

double
Transform::DoubleData::doubleValue(bool) const
{
    return _value;
}

string
Transform::DoubleData::stringValue(bool convert) const
{
    if(convert)
    {
        return toString();
    }
    else
    {
        _errorReporter->error("double cannot be converted to string");
        return string();
    }
}

string
Transform::DoubleData::toString() const
{
    char buff[64];
    sprintf(buff, "%g", _value);
    return string(buff);
}

void
Transform::DoubleData::printI(IceUtil::Output& out, ObjectDataHistory&) const
{
    out << "double(" << toString() << ")";
}

//
// StringData
//
Transform::StringData::StringData(const DataFactoryPtr& factory, const Slice::BuiltinPtr& type,
                                  const ErrorReporterPtr& errorReporter, bool readOnly) :
    PrimitiveData(errorReporter, readOnly), _factory(factory), _type(type)
{
    setValue("");
}

Transform::StringData::StringData(const DataFactoryPtr& factory, const Slice::BuiltinPtr& type,
                                  const ErrorReporterPtr& errorReporter, bool readOnly, const string& value) :
    PrimitiveData(errorReporter, readOnly), _factory(factory), _type(type)
{
    setValue(value);
}

Transform::DataPtr
Transform::StringData::getMember(const string& member) const
{
    if(member == "length")
    {
        return _length;
    }

    return 0;
}

bool
Transform::StringData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
Transform::StringData::operator!=(const Data& rhs) const
{
    const StringData* s = dynamic_cast<const StringData*>(&rhs);
    if(!s)
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _value != s->_value;
}

bool
Transform::StringData::operator<(const Data& rhs) const
{
    const StringData* s = dynamic_cast<const StringData*>(&rhs);
    if(!s)
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _value < s->_value;
}

void
Transform::StringData::transform(const DataPtr& data, DataInterceptor& interceptor)
{
    if(interceptor.preTransform(this, data))
    {
        StringDataPtr s = StringDataPtr::dynamicCast(data);
        BooleanDataPtr b = BooleanDataPtr::dynamicCast(data);
        IntegerDataPtr i = IntegerDataPtr::dynamicCast(data);
        DoubleDataPtr d = DoubleDataPtr::dynamicCast(data);
        EnumDataPtr e = EnumDataPtr::dynamicCast(data);
        ProxyDataPtr p = ProxyDataPtr::dynamicCast(data);
        if(s || b || i || d || e || p)
        {
            setValue(data->toString());
        }
        else
        {
            _errorReporter->typeMismatchError(_type, data->getType(), false);
        }
    }
    interceptor.postTransform(this, data);
}

void
Transform::StringData::set(const DataPtr& value, DataInterceptor&, bool convert)
{
    setValue(value->stringValue(convert));
}

Transform::DataPtr
Transform::StringData::clone() const
{
    return new StringData(_factory, _type, _errorReporter, _readOnly, _value);
}

Slice::TypePtr
Transform::StringData::getType() const
{
    return _type;
}

void
Transform::StringData::marshal(IceInternal::BasicStream& os) const
{
    os.write(_value);
}

void
Transform::StringData::unmarshal(IceInternal::BasicStream& is)
{
    string s;
    is.read(s);
    setValue(s);
}

bool
Transform::StringData::booleanValue(bool) const
{
    _errorReporter->error("string cannot be converted to boolean");
    return false;
}

Ice::Long
Transform::StringData::integerValue(bool) const
{
    _errorReporter->error("string cannot be converted to integer");
    return 0;
}

double
Transform::StringData::doubleValue(bool) const
{
    _errorReporter->error("string cannot be converted to double");
    return 0;
}

string
Transform::StringData::stringValue(bool) const
{
    return _value;
}

string
Transform::StringData::toString() const
{
    return _value;
}

void
Transform::StringData::printI(IceUtil::Output& out, ObjectDataHistory&) const
{
    out << "'" << toString() << "'";
}

void
Transform::StringData::setValue(const string& value)
{
    _value = value;
    _length = _factory->createInteger(static_cast<Ice::Long>(_value.size()), true);
}

//
// ProxyData
//
Transform::ProxyData::ProxyData(const Slice::TypePtr& type, const Ice::CommunicatorPtr& communicator,
                                const ErrorReporterPtr& errorReporter, bool readOnly) :
    Data(errorReporter, readOnly), _type(type), _communicator(communicator)
{
}

Transform::ProxyData::ProxyData(const Slice::TypePtr& type, const Ice::CommunicatorPtr& communicator,
                                const ErrorReporterPtr& errorReporter, bool readOnly, const Ice::ObjectPrx& value) :
    Data(errorReporter, readOnly), _type(type), _communicator(communicator), _value(value)
{
}

Transform::DataPtr
Transform::ProxyData::getMember(const string& member) const
{
    // TODO: Support members (id, facet, etc.)?

    return 0;
}

Transform::DataPtr
Transform::ProxyData::getElement(const DataPtr&) const
{
    _errorReporter->error("element requested of proxy value");
    return 0;
}

bool
Transform::ProxyData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
Transform::ProxyData::operator!=(const Data& rhs) const
{
    const ProxyData* p = dynamic_cast<const ProxyData*>(&rhs);
    if(!p)
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _value != p->_value;
}

bool
Transform::ProxyData::operator<(const Data& rhs) const
{
    const ProxyData* p = dynamic_cast<const ProxyData*>(&rhs);
    if(!p)
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _value < p->_value;
}

void
Transform::ProxyData::registerObjects(ObjectDataMap&) const
{
}

void
Transform::ProxyData::transform(const DataPtr& data, DataInterceptor& interceptor)
{
    if(interceptor.preTransform(this, data))
    {
        transformI(data);
    }
    interceptor.postTransform(this, data);
}

void
Transform::ProxyData::set(const DataPtr& value, DataInterceptor&, bool)
{
    transformI(value);
}

Transform::DataPtr
Transform::ProxyData::clone() const
{
    return new ProxyData(_type, _communicator, _errorReporter, _readOnly, _value);
}

Slice::TypePtr
Transform::ProxyData::getType() const
{
    return _type;
}

void
Transform::ProxyData::destroy()
{
}

void
Transform::ProxyData::marshal(IceInternal::BasicStream& os) const
{
    os.write(_value);
}

void
Transform::ProxyData::unmarshal(IceInternal::BasicStream& is)
{
    is.read(_value);
}

bool
Transform::ProxyData::booleanValue(bool) const
{
    _errorReporter->error("proxy cannot be converted to boolean");
    return 0;
}

Ice::Long
Transform::ProxyData::integerValue(bool) const
{
    _errorReporter->error("proxy cannot be converted to integer");
    return 0;
}

double
Transform::ProxyData::doubleValue(bool) const
{
    _errorReporter->error("proxy cannot be converted to double");
    return 0;
}

string
Transform::ProxyData::stringValue(bool) const
{
    _errorReporter->error("proxy cannot be converted to string");
    return string();
}

string
Transform::ProxyData::toString() const
{
    if(_value && _str.empty())
    {
        const_cast<string&>(_str) = _communicator->proxyToString(_value);
    }
    return _str;
}

Ice::ObjectPrx
Transform::ProxyData::getValue() const
{
    return _value;
}

void
Transform::ProxyData::printI(IceUtil::Output& out, ObjectDataHistory&) const
{
    out << typeName(_type) << "*(" << toString() << ")";
}

void
Transform::ProxyData::transformI(const DataPtr& data)
{
    ProxyDataPtr p = ProxyDataPtr::dynamicCast(data);
    StringDataPtr s = StringDataPtr::dynamicCast(data);
    if(p)
    {
        _value = p->getValue();
        _str.clear();
    }
    else if(s)
    {
        setValue(s->stringValue());
    }
    else
    {
        _errorReporter->typeMismatchError(_type, data->getType(), false);
    }
}

void
Transform::ProxyData::setValue(const string& str)
{
    try
    {
        _value = _communicator->stringToProxy(str);
        _str = str;
    }
    catch(const Ice::LocalException&)
    {
        _errorReporter->conversionError(str, _type);
    }
}

//
// StructData
//
Transform::StructData::StructData(const DataFactoryPtr& factory, const Slice::StructPtr& type,
                                  const ErrorReporterPtr& errorReporter, bool readOnly) :
    Data(errorReporter, readOnly), _type(type)
{
    Slice::DataMemberList members = type->dataMembers();
    for(Slice::DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        DataPtr value = factory->create((*q)->type(), _readOnly);
        assert(value);
        _members[(*q)->name()] = value;
    }
}

Transform::StructData::StructData(const Slice::StructPtr& type, const ErrorReporterPtr& errorReporter, bool readOnly,
                                  const DataMemberMap& members) :
    Data(errorReporter, readOnly), _type(type)
{
    for(DataMemberMap::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        _members[p->first] = p->second->clone();
    }
}

Transform::DataPtr
Transform::StructData::getMember(const string& member) const
{
    DataMemberMap::const_iterator p = _members.find(member);
    if(p != _members.end())
    {
        return p->second;
    }

    return 0;
}

Transform::DataPtr
Transform::StructData::getElement(const DataPtr&) const
{
    _errorReporter->error("element requested of struct value");
    return 0;
}

bool
Transform::StructData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
Transform::StructData::operator!=(const Data& rhs) const
{
    const StructData* s = dynamic_cast<const StructData*>(&rhs);
    if(!s || _type->scoped() != s->_type->scoped())
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    assert(_members.size() == s->_members.size());

    for(DataMemberMap::const_iterator p = _members.begin(); p != _members.end(); ++p)
    {
        DataMemberMap::const_iterator q = s->_members.find(p->first);
        assert(q != s->_members.end());

        if(p->second != q->second)
        {
            return true;
        }
    }

    return false;
}

bool
Transform::StructData::operator<(const Data& rhs) const
{
    const StructData* s = dynamic_cast<const StructData*>(&rhs);
    if(!s || _type->scoped() != s->_type->scoped())
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    assert(_members.size() == s->_members.size());

    for(DataMemberMap::const_iterator p = _members.begin(); p != _members.end(); ++p)
    {
        DataMemberMap::const_iterator q = s->_members.find(p->first);
        assert(q != s->_members.end());

        if(p->second < q->second)
        {
            return true;
        }
        else if(q->second < p->second)
        {
            return false;
        }
    }

    return false;
}

void
Transform::StructData::registerObjects(ObjectDataMap& m) const
{
    for(DataMemberMap::const_iterator p = _members.begin(); p != _members.end(); ++p)
    {
        p->second->registerObjects(m);
    }
}

void
Transform::StructData::transform(const DataPtr& data, DataInterceptor& interceptor)
{
    if(interceptor.preTransform(this, data))
    {
        transformI(data, interceptor);
    }
    interceptor.postTransform(this, data);
}

void
Transform::StructData::set(const DataPtr& value, DataInterceptor& interceptor, bool)
{
    if(value->getType()->unit().get() != _type->unit().get())
    {
        transform(value, interceptor);
    }
    else
    {
        transformI(value, interceptor);
    }
}

Transform::DataPtr
Transform::StructData::clone() const
{
    return new StructData(_type, _errorReporter, _readOnly, _members);
}

Slice::TypePtr
Transform::StructData::getType() const
{
    return _type;
}

void
Transform::StructData::destroy()
{
    for(DataMemberMap::const_iterator p = _members.begin(); p != _members.end(); ++p)
    {
        p->second->destroy();
    }
}

void
Transform::StructData::marshal(IceInternal::BasicStream& os) const
{
    Slice::DataMemberList members = _type->dataMembers();
    for(Slice::DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
    {
        DataMemberMap::const_iterator q = _members.find((*p)->name());
        assert(q != _members.end());
        q->second->marshal(os);
    }
}

void
Transform::StructData::unmarshal(IceInternal::BasicStream& is)
{
    Slice::DataMemberList members = _type->dataMembers();
    for(Slice::DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
    {
        DataMemberMap::const_iterator q = _members.find((*p)->name());
        assert(q != _members.end());
        q->second->unmarshal(is);
    }
}

bool
Transform::StructData::booleanValue(bool) const
{
    _errorReporter->error("struct " + typeName(_type) + " cannot be converted to boolean");
    return 0;
}

Ice::Long
Transform::StructData::integerValue(bool) const
{
    _errorReporter->error("struct " + typeName(_type) + " cannot be converted to integer");
    return 0;
}

double
Transform::StructData::doubleValue(bool) const
{
    _errorReporter->error("struct " + typeName(_type) + " cannot be converted to double");
    return 0;
}

string
Transform::StructData::stringValue(bool) const
{
    _errorReporter->error("struct " + typeName(_type) + " cannot be converted to string");
    return string();
}

string
Transform::StructData::toString() const
{
    return stringValue();
}

Transform::DataMemberMap
Transform::StructData::getMembers() const
{
    return _members;
}

void
Transform::StructData::printI(IceUtil::Output& out, ObjectDataHistory& history) const
{
    out << "struct " << typeName(_type);
    out << sb;
    for(DataMemberMap::const_iterator p = _members.begin(); p != _members.end(); ++p)
    {
        out << nl;
        out << p->first << " = ";
        p->second->printI(out, history);
    }
    out << eb;
}

void
Transform::StructData::transformI(const DataPtr& data, DataInterceptor& interceptor)
{
    StructDataPtr s = StructDataPtr::dynamicCast(data);
    if(s && isCompatible(_type, s->_type, interceptor))
    {
        //
        // Invoke transform() on members with the same name.
        //
        for(DataMemberMap::iterator p = _members.begin(); p != _members.end(); ++p)
        {
            DataMemberMap::iterator q = s->_members.find(p->first);
            if(q != s->_members.end())
            {
                p->second->transform(q->second, interceptor);
            }
        }
    }
    else
    {
        _errorReporter->typeMismatchError(_type, data->getType(), false);
    }
}

//
// SequenceData
//
Transform::SequenceData::SequenceData(const DataFactoryPtr& factory, const Slice::SequencePtr& type,
                                      const ErrorReporterPtr& errorReporter, bool readOnly) :
    Data(errorReporter, readOnly), _factory(factory), _type(type)
{
    _length = _factory->createInteger(0, true);
}

Transform::SequenceData::SequenceData(const DataFactoryPtr& factory, const Slice::SequencePtr& type,
                                      const ErrorReporterPtr& errorReporter, bool readOnly, const DataList& elements) :
    Data(errorReporter, readOnly), _factory(factory), _type(type)
{
    for(DataList::const_iterator p = elements.begin(); p != elements.end(); ++p)
    {
        _elements.push_back((*p)->clone());
    }
    _length = _factory->createInteger(static_cast<Ice::Long>(_elements.size()), true);
}

Transform::DataPtr
Transform::SequenceData::getMember(const string& member) const
{
    if(member == "length")
    {
        return _length;
    }

    return 0;
}

Transform::DataPtr
Transform::SequenceData::getElement(const DataPtr& element) const
{
    IntegerDataPtr i = IntegerDataPtr::dynamicCast(element);
    if(!i)
    {
        _errorReporter->error("invalid sequence index type " + typeName(element->getType()));
    }

    Ice::Long iv = i->integerValue();
    if(iv < 0 || iv >= _elements.size())
    {
        _errorReporter->error("sequence index " + i->toString() + " out of range");
    }

    return _elements[static_cast<DataList::size_type>(iv)];
}

bool
Transform::SequenceData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
Transform::SequenceData::operator!=(const Data& rhs) const
{
    const SequenceData* s = dynamic_cast<const SequenceData*>(&rhs);
    if(!s || _type->scoped() != s->_type->scoped())
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _elements != s->_elements;
}

bool
Transform::SequenceData::operator<(const Data& rhs) const
{
    const SequenceData* s = dynamic_cast<const SequenceData*>(&rhs);
    if(!s || _type->scoped() != s->_type->scoped())
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _elements < s->_elements;
}

void
Transform::SequenceData::registerObjects(ObjectDataMap& m) const
{
    if(_type->type()->usesClasses())
    {
        for(DataList::const_iterator p = _elements.begin(); p != _elements.end(); ++p)
        {
            (*p)->registerObjects(m);
        }
    }
}

void
Transform::SequenceData::transform(const DataPtr& data, DataInterceptor& interceptor)
{
    if(interceptor.preTransform(this, data))
    {
        transformI(data, interceptor);
    }
    interceptor.postTransform(this, data);
}

void
Transform::SequenceData::set(const DataPtr& value, DataInterceptor& interceptor, bool)
{
    if(value->getType()->unit().get() != _type->unit().get())
    {
        transform(value, interceptor);
    }
    else
    {
        transformI(value, interceptor);
    }
}

Transform::DataPtr
Transform::SequenceData::clone() const
{
    return new SequenceData(_factory, _type, _errorReporter, _readOnly, _elements);
}

Slice::TypePtr
Transform::SequenceData::getType() const
{
    return _type;
}

void
Transform::SequenceData::destroy()
{
    for(DataList::const_iterator p = _elements.begin(); p != _elements.end(); ++p)
    {
        (*p)->destroy();
    }
}

void
Transform::SequenceData::marshal(IceInternal::BasicStream& os) const
{
    os.writeSize(_elements.size());
    for(DataList::const_iterator p = _elements.begin(); p != _elements.end(); ++p)
    {
        (*p)->marshal(os);
    }
}

void
Transform::SequenceData::unmarshal(IceInternal::BasicStream& is)
{
    Slice::TypePtr type = _type->type();
    Ice::Int sz;
    is.readSize(sz);
    _elements.reserve(sz);
    for(Ice::Int i = 0; i < sz; ++i)
    {
        DataPtr v = _factory->create(type, _readOnly);
        v->unmarshal(is);
        _elements.push_back(v);
    }
    _length = _factory->createInteger(sz, true);
}

bool
Transform::SequenceData::booleanValue(bool) const
{
    _errorReporter->error("sequence " + typeName(_type) + " cannot be converted to boolean");
    return 0;
}

Ice::Long
Transform::SequenceData::integerValue(bool) const
{
    _errorReporter->error("sequence " + typeName(_type) + " cannot be converted to integer");
    return 0;
}

double
Transform::SequenceData::doubleValue(bool) const
{
    _errorReporter->error("sequence " + typeName(_type) + " cannot be converted to double");
    return 0;
}

string
Transform::SequenceData::stringValue(bool) const
{
    _errorReporter->error("sequence " + typeName(_type) + " cannot be converted to string");
    return string();
}

string
Transform::SequenceData::toString() const
{
    return stringValue();
}

Transform::DataList
Transform::SequenceData::getElements() const
{
    return _elements;
}

void
Transform::SequenceData::resize(const DataPtr& length, const DataPtr& value, DataInterceptor& interceptor, bool convert)
{
    Ice::Long l = length->integerValue();
    if(l < 0 || l > INT_MAX)
    {
        _errorReporter->error("sequence length " + length->toString() + " is out of range");
        return;
    }

    DataList::size_type len = static_cast<DataList::size_type>(l);
    if(len < _elements.size())
    {
        for(DataList::size_type i = len; i < _elements.size(); ++i)
        {
            _elements[i]->destroy();
        }
        _elements.resize(len);
    }
    else if(len > _elements.size())
    {
        Slice::TypePtr type = _type->type();
        for(DataList::size_type i = _elements.size(); i < len; ++i)
        {
            DataPtr v = _factory->create(type, _readOnly);
            if(value)
            {
                v->set(value, interceptor, convert);
            }
            _elements.push_back(v);
        }
    }

    _length = _factory->createInteger(l, true);
}

void
Transform::SequenceData::printI(IceUtil::Output& out, ObjectDataHistory& history) const
{
    out << "sequence " << typeName(_type) << " (size = " << _elements.size() << ")";
    out << sb;
    Ice::Int i = 0;
    for(DataList::const_iterator p = _elements.begin(); p != _elements.end(); ++p, ++i)
    {
        out << nl << "[" << i << "] = ";
        (*p)->printI(out, history);
    }
    out << eb;
}

void
Transform::SequenceData::transformI(const DataPtr& data, DataInterceptor& interceptor)
{
    SequenceDataPtr s = SequenceDataPtr::dynamicCast(data);
    if(s && isCompatible(_type, s->_type, interceptor))
    {
        DataList elements;
        for(DataList::const_iterator p = s->_elements.begin(); p != s->_elements.end(); ++p)
        {
            DataPtr element = _factory->create(_type->type(), _readOnly);
            Destroyer<DataPtr> elementDestroyer(element);
            try
            {
                element->transform((*p), interceptor);
                elements.push_back(element);
                elementDestroyer.release();
            }
            catch(const ClassNotFoundException& ex)
            {
                //
                // If transformation of the sequence element fails because a class
                // could not be found, then we invoke purgeObjects() to determine
                // whether we should ignore the situation (and remove the element
                // from the sequence) or raise the exception again.
                //
                if(!interceptor.purgeObjects())
                {
                    throw;
                }
                _errorReporter->warning("purging element of sequence " + typeName(_type) +
                                        " due to missing class type " + ex.id);
            }
        }
        _elements = elements;
        _length = _factory->createInteger(static_cast<Ice::Long>(_elements.size()), true);
    }
    else
    {
        _errorReporter->typeMismatchError(_type, data->getType(), false);
    }
}

//
// EnumData
//
Transform::EnumData::EnumData(const Slice::EnumPtr& type, const ErrorReporterPtr& errorReporter, bool readOnly) :
    Data(errorReporter, readOnly), _type(type), _value(0)
{
    _count = static_cast<long>(type->getEnumerators().size());
}

Transform::EnumData::EnumData(const Slice::EnumPtr& type, const ErrorReporterPtr& errorReporter, bool readOnly,
                              Ice::Int value) :
    Data(errorReporter, readOnly), _type(type), _value(value)
{
    _count = static_cast<long>(type->getEnumerators().size());
}

Transform::DataPtr
Transform::EnumData::getMember(const string&) const
{
    return 0;
}

Transform::DataPtr
Transform::EnumData::getElement(const DataPtr&) const
{
    _errorReporter->error("element requested of enum value");
    return 0;
}

bool
Transform::EnumData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
Transform::EnumData::operator!=(const Data& rhs) const
{
    const EnumData* e = dynamic_cast<const EnumData*>(&rhs);
    if(!e || _type->scoped() != e->_type->scoped())
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _value != e->_value;
}

bool
Transform::EnumData::operator<(const Data& rhs) const
{
    const EnumData* e = dynamic_cast<const EnumData*>(&rhs);
    if(!e || _type->scoped() != e->_type->scoped())
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _value < e->_value;
}

void
Transform::EnumData::registerObjects(ObjectDataMap&) const
{
}

void
Transform::EnumData::transform(const DataPtr& data, DataInterceptor& interceptor)
{
    if(interceptor.preTransform(this, data))
    {
        transformI(data, interceptor);
    }
    interceptor.postTransform(this, data);
}

void
Transform::EnumData::set(const DataPtr& value, DataInterceptor& interceptor, bool convert)
{
    IntegerDataPtr i = IntegerDataPtr::dynamicCast(value);
    if(i && convert)
    {
        Ice::Long v = i->integerValue();
        if(v < 0 || v > _count)
        {
            _errorReporter->rangeError(i->toString(), _type);
        }
        else
        {
            _value = static_cast<Ice::Int>(v);
            _name.clear();
        }
    }
    else if(value->getType()->unit().get() != _type->unit().get())
    {
        transform(value, interceptor);
    }
    else
    {
        transformI(value, interceptor);
    }
}

Transform::DataPtr
Transform::EnumData::clone() const
{
    return new EnumData(_type, _errorReporter, _value);
}

Slice::TypePtr
Transform::EnumData::getType() const
{
    return _type;
}

void
Transform::EnumData::destroy()
{
}

void
Transform::EnumData::marshal(IceInternal::BasicStream& os) const
{
    if(_count <= 127)
    {
        os.write(static_cast<Ice::Byte>(_value));
    }
    else if(_count <= 32767)
    {
        os.write(static_cast<Ice::Short>(_value));
    }
    else
    {
        os.write(_value);
    }
}

void
Transform::EnumData::unmarshal(IceInternal::BasicStream& is)
{
    if(_count <= 127)
    {
        Ice::Byte val;
        is.read(val);
        _value = val & 0xff;
    }
    else if(_count <= 32767)
    {
        Ice::Short val;
        is.read(val);
        _value = val;
    }
    else
    {
        is.read(_value);
    }
}

bool
Transform::EnumData::booleanValue(bool) const
{
    _errorReporter->error("enum " + typeName(_type) + " cannot be converted to boolean");
    return 0;
}

Ice::Long
Transform::EnumData::integerValue(bool convert) const
{
    if(convert)
    {
        return _value;
    }
    else
    {
        _errorReporter->error("enum " + typeName(_type) + " cannot be converted to integer");
        return 0;
    }
}

double
Transform::EnumData::doubleValue(bool) const
{
    _errorReporter->error("enum " + typeName(_type) + " cannot be converted to double");
    return 0;
}

string
Transform::EnumData::stringValue(bool) const
{
    _errorReporter->error("enum " + typeName(_type) + " cannot be converted to string");
    return string();
}

string
Transform::EnumData::toString() const
{
    if(_name.empty())
    {
        Slice::EnumeratorList l = _type->getEnumerators();
        Slice::EnumeratorList::const_iterator p = l.begin();
        Ice::Int i = _value;
        while(i > 0)
        {
            ++p;
            assert(p != l.end());
            --i;
        }
        const_cast<string&>(_name) = (*p)->name();
    }
    return _name;
}

Ice::Int
Transform::EnumData::getValue() const
{
    return _value;
}

bool
Transform::EnumData::setValue(Ice::Int v)
{
    if(v >= 0 && v < _count)
    {
        _value = v;
        _name.clear();
        return true;
    }
    return false;
}

bool
Transform::EnumData::setValueAsString(const string& v)
{
    Slice::EnumeratorList l = _type->getEnumerators();
    Slice::EnumeratorList::const_iterator p;
    Ice::Int i = 0;
    for(p = l.begin(); p != l.end(); ++p, ++i)
    {
        if((*p)->name() == v)
        {
            _value = i;
            _name = v;
            break;
        }
    }
    return p != l.end();
}

void
Transform::EnumData::printI(IceUtil::Output& out, ObjectDataHistory&) const
{
    out << typeName(_type) << "(" << toString() << ")";
}

void
Transform::EnumData::transformI(const DataPtr& data, DataInterceptor& interceptor)
{
    EnumDataPtr e = EnumDataPtr::dynamicCast(data);
    StringDataPtr s = StringDataPtr::dynamicCast(data);
    if(e && isCompatible(_type, e->_type, interceptor))
    {
        //
        // Get the enumerator's name and attempt to find it in our type.
        //
        string name = e->toString();
        if(!setValueAsString(name))
        {
            _errorReporter->conversionError(name, _type);
        }
    }
    else if(s)
    {
        string v = s->stringValue();
        if(!setValueAsString(v))
        {
            _errorReporter->conversionError(v, _type);
        }
    }
    else
    {
        _errorReporter->typeMismatchError(_type, data->getType(), false);
    }
}

//
// DictionaryData
//
Transform::DictionaryData::DictionaryData(const DataFactoryPtr& factory, const Slice::DictionaryPtr& type,
                                          const ErrorReporterPtr& errorReporter, bool readOnly) :
    Data(errorReporter, readOnly), _factory(factory), _type(type)
{
    _length = _factory->createInteger(0, true);
}

Transform::DictionaryData::DictionaryData(const DataFactoryPtr& factory, const Slice::DictionaryPtr& type,
                                          const ErrorReporterPtr& errorReporter, bool readOnly, const DataMap& m) :
    Data(errorReporter, readOnly), _factory(factory), _type(type)
{
    for(DataMap::const_iterator p = m.begin(); p != m.end(); ++p)
    {
        _map.insert(DataMap::value_type(p->first->clone(), p->second->clone()));
    }
    _length = _factory->createInteger(static_cast<Ice::Long>(_map.size()), true);
}

Transform::DataPtr
Transform::DictionaryData::getMember(const string& member) const
{
    if(member == "length")
    {
        return _length;
    }

    return 0;
}

Transform::DataPtr
Transform::DictionaryData::getElement(const DataPtr& element) const
{
    DataMap::const_iterator p = _map.find(element);
    if(p != _map.end())
    {
        return p->second;
    }

    return 0;
}

bool
Transform::DictionaryData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
Transform::DictionaryData::operator!=(const Data& rhs) const
{
    const DictionaryData* d = dynamic_cast<const DictionaryData*>(&rhs);
    if(!d || _type->scoped() != d->_type->scoped())
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _map != d->_map;
}

bool
Transform::DictionaryData::operator<(const Data& rhs) const
{
    const DictionaryData* d = dynamic_cast<const DictionaryData*>(&rhs);
    if(!d || _type->scoped() != d->_type->scoped())
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _map < d->_map;
}

void
Transform::DictionaryData::registerObjects(ObjectDataMap& m) const
{
    if(_type->valueType()->usesClasses())
    {
        for(DataMap::const_iterator p = _map.begin(); p != _map.end(); ++p)
        {
            p->second->registerObjects(m);
        }
    }
}

void
Transform::DictionaryData::transform(const DataPtr& data, DataInterceptor& interceptor)
{
    if(interceptor.preTransform(this, data))
    {
        transformI(data, interceptor);
    }
    interceptor.postTransform(this, data);
}

void
Transform::DictionaryData::set(const DataPtr& value, DataInterceptor& interceptor, bool)
{
    if(value->getType()->unit().get() != _type->unit().get())
    {
        transform(value, interceptor);
    }
    else
    {
        transformI(value, interceptor);
    }
}

Transform::DataPtr
Transform::DictionaryData::clone() const
{
    return new DictionaryData(_factory, _type, _errorReporter, _readOnly, _map);
}

Slice::TypePtr
Transform::DictionaryData::getType() const
{
    return _type;
}

void
Transform::DictionaryData::destroy()
{
    for(DataMap::const_iterator p = _map.begin(); p != _map.end(); ++p)
    {
        p->first->destroy();
        p->second->destroy();
    }
}

void
Transform::DictionaryData::marshal(IceInternal::BasicStream& os) const
{
    os.writeSize(_map.size());
    for(DataMap::const_iterator p = _map.begin(); p != _map.end(); ++p)
    {
        p->first->marshal(os);
        p->second->marshal(os);
    }
}

void
Transform::DictionaryData::unmarshal(IceInternal::BasicStream& is)
{
    Slice::TypePtr keyType = _type->keyType();
    Slice::TypePtr valueType = _type->valueType();

    Ice::Int sz;
    is.readSize(sz);

    for(Ice::Int i = 0; i < sz; ++i)
    {
        DataPtr key = _factory->create(keyType, _readOnly);
        key->unmarshal(is);

        DataPtr value = _factory->create(valueType, _readOnly);
        value->unmarshal(is);

        _map.insert(DataMap::value_type(key, value));
    }

    _length = _factory->createInteger(static_cast<Ice::Long>(_map.size()), true);
}

bool
Transform::DictionaryData::booleanValue(bool) const
{
    _errorReporter->error("dictionary " + typeName(_type) + " cannot be converted to boolean");
    return 0;
}

Ice::Long
Transform::DictionaryData::integerValue(bool) const
{
    _errorReporter->error("dictionary " + typeName(_type) + " cannot be converted to integer");
    return 0;
}

double
Transform::DictionaryData::doubleValue(bool) const
{
    _errorReporter->error("dictionary " + typeName(_type) + " cannot be converted to double");
    return 0;
}

string
Transform::DictionaryData::stringValue(bool) const
{
    _errorReporter->error("dictionary " + typeName(_type) + " cannot be converted to string");
    return string();
}

string
Transform::DictionaryData::toString() const
{
    return stringValue();
}

void
Transform::DictionaryData::printI(IceUtil::Output& out, ObjectDataHistory& history) const
{
    out << "dictionary " << typeName(_type) << " (size = " << _map.size() << ")";
    out << sb;
    for(DataMap::const_iterator p = _map.begin(); p != _map.end(); ++p)
    {
        out << nl;
        p->first->printI(out, history);
        out << " => ";
        p->second->printI(out, history);
    }
    out << eb;
}

void
Transform::DictionaryData::transformI(const DataPtr& data, DataInterceptor& interceptor)
{
    DictionaryDataPtr d = DictionaryDataPtr::dynamicCast(data);
    if(d && isCompatible(_type, d->_type, interceptor))
    {
        DataMap m;
        for(DataMap::const_iterator p = d->_map.begin(); p != d->_map.end(); ++p)
        {
            DataPtr key = _factory->create(_type->keyType(), _readOnly);
            Destroyer<DataPtr> keyDestroyer(key);
            DataPtr value = _factory->create(_type->valueType(), _readOnly);
            Destroyer<DataPtr> valueDestroyer(value);

            key->transform(p->first, interceptor);

            try
            {
                value->transform(p->second, interceptor);
            }
            catch(const ClassNotFoundException& ex)
            {
                //
                // If transformation of the dictionary value fails because a class
                // could not be found, then we invoke purgeObjects() to determine
                // whether we should ignore the situation (and remove the element
                // from the dictionary) or raise the exception again.
                //
                if(!interceptor.purgeObjects())
                {
                    throw;
                }
                _errorReporter->warning("purging element of dictionary " + typeName(_type) +
                                        " due to missing class type " + ex.id);
                continue;
            }

            DataMap::const_iterator q = m.find(key);
            if(q != m.end())
            {
                ostringstream ostr;
                ostr << "duplicate dictionary key (";
                key->print(ostr);
                ostr << ")";
                _errorReporter->warning(ostr.str());
            }
            else
            {
                m.insert(DataMap::value_type(key, value));
                keyDestroyer.release();
                valueDestroyer.release();
            }
        }
        _map = m;
        _length = _factory->createInteger(static_cast<Ice::Long>(_map.size()), true);
    }
    else
    {
        _errorReporter->typeMismatchError(_type, data->getType(), false);
    }
}

Transform::DataMap
Transform::DictionaryData::getElements() const
{
    return _map;
}

void
Transform::DictionaryData::add(const DataPtr& key, const DataPtr& value)
{
    assert(!readOnly());

    DataMap::iterator p = _map.find(key);
    assert(p == _map.end());

    _map.insert(DataMap::value_type(key, value));
    _length = _factory->createInteger(static_cast<Ice::Long>(_map.size()), true);
}

void
Transform::DictionaryData::remove(const DataPtr& key)
{
    DataMap::iterator p = _map.find(key);
    if(p != _map.end())
    {
        p->first->destroy();
        p->second->destroy();
        _map.erase(p);
        _length = _factory->createInteger(static_cast<Ice::Long>(_map.size()), true);
    }
}

//
// ObjectData
//
Transform::ObjectData::ObjectData(const DataFactoryPtr& factory, const Slice::TypePtr& type, bool readOnly) :
    Data(factory->getErrorReporter(), readOnly), _type(type), _refCount(0)
{
    //
    // Create a Data object for the facet map. We add this to the _members map so that
    // the `ice_facets' member is handled like all other members.
    //
    Slice::TypeList l = _type->unit()->lookupType("::_FacetMap", false);
    assert(!l.empty());
    _facetMap = factory->create(l.front(), readOnly);
    _members["ice_facets"] = _facetMap;

    Slice::ClassDeclPtr decl = Slice::ClassDeclPtr::dynamicCast(type);
    if(decl)
    {
        Slice::ClassDefPtr def = decl->definition();
        assert(def);
        Slice::DataMemberList members = def->allDataMembers();
        for(Slice::DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
        {
            _members[(*p)->name()] = factory->create((*p)->type(), _readOnly);
        }
        _id = factory->createString(decl->scoped(), true);
    }
    else
    {
        _id = factory->createString("::Ice::Object", true);
    }
}

Transform::DataPtr
Transform::ObjectData::getMember(const string& member) const
{
    if(member == "ice_id")
    {
        return _id;
    }

    DataMemberMap::const_iterator p = _members.find(member);
    if(p != _members.end())
    {
        return p->second;
    }

    return 0;
}

Transform::DataPtr
Transform::ObjectData::getElement(const DataPtr& element) const
{
    _errorReporter->error("element requested of object value");
    return 0;
}

bool
Transform::ObjectData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
Transform::ObjectData::operator!=(const Data& rhs) const
{
    _errorReporter->error("object comparison not supported");
    return false;
}

bool
Transform::ObjectData::operator<(const Data& rhs) const
{
    _errorReporter->error("object comparison not supported");
    return false;
}

void
Transform::ObjectData::registerObjects(ObjectDataMap& m) const
{
    ObjectDataMap::iterator p = m.find(this);
    if(p == m.end())
    {
        m.insert(ObjectDataMap::value_type(this, 0));
        for(DataMemberMap::const_iterator q = _members.begin(); q != _members.end(); ++q)
        {
            q->second->registerObjects(m);
        }
    }
}

void
Transform::ObjectData::transform(const DataPtr& data, DataInterceptor& interceptor)
{
    ObjectDataPtr o = ObjectDataPtr::dynamicCast(data);
    if(o)
    {
        //
        // The source object must be present in the object map (we currently don't support
        // transforming two ObjectData instances from the same Slice unit - this transform
        // would be handled by-reference at the ObjectRef level). We must update the object
        // map before transforming members in order to handle cycles.
        //
        ObjectDataMap& objectMap = interceptor.getObjectMap();
        ObjectDataMap::iterator p = objectMap.find(o.get());
        assert(p != objectMap.end());
        assert(p->second == 0);
        objectMap.erase(p);
        objectMap.insert(ObjectDataMap::value_type(o.get(), this));

        try
        {
            //
            // Invoke transform() on members with the same name.
            //
            for(DataMemberMap::iterator q = _members.begin(); q != _members.end(); ++q)
            {
                DataMemberMap::iterator r = o->_members.find(q->first);
                if(r != o->_members.end())
                {
                    q->second->transform(r->second, interceptor);
                }
            }
        }
        catch(...)
        {
            objectMap.erase(p);
            objectMap.insert(ObjectDataMap::value_type(o.get(), 0));
            throw;
        }
    }
    else
    {
        _errorReporter->typeMismatchError(_type, data->getType(), false);
    }
}

void
Transform::ObjectData::set(const DataPtr&, DataInterceptor&, bool)
{
    assert(false);
}

Transform::DataPtr
Transform::ObjectData::clone() const
{
    assert(false);
    return 0;
}

Slice::TypePtr
Transform::ObjectData::getType() const
{
    return _type;
}

void
Transform::ObjectData::destroy()
{
    assert(false);
}

void
Transform::ObjectData::marshal(IceInternal::BasicStream& os) const
{
    if(!_marshaler)
    {
        const_cast<Ice::ObjectPtr&>(_marshaler) = new ObjectWriter(const_cast<ObjectData*>(this));
    }
    os.write(_marshaler);
}

void
Transform::ObjectData::unmarshal(IceInternal::BasicStream& is)
{
    //
    // Unmarshaling is done by ObjectReader.
    //
    assert(false);
}

bool
Transform::ObjectData::booleanValue(bool) const
{
    assert(false);
    return 0;
}

Ice::Long
Transform::ObjectData::integerValue(bool) const
{
    assert(false);
    return 0;
}

double
Transform::ObjectData::doubleValue(bool) const
{
    assert(false);
    return 0;
}

string
Transform::ObjectData::stringValue(bool) const
{
    assert(false);
    return string();
}

string
Transform::ObjectData::toString() const
{
    assert(false);
    return string();
}

void
Transform::ObjectData::incRef()
{
    _refCount++;
}

void
Transform::ObjectData::decRef()
{
    assert(_refCount > 0);
    _refCount--;
    if(_refCount == 0)
    {
        destroyI();
    }
}

void
Transform::ObjectData::printI(IceUtil::Output& out, ObjectDataHistory& history) const
{
    out << "class " << typeName(_type);
    ObjectDataHistory::iterator p = history.find(this);
    if(p != history.end())
    {
        out << " (cycle)";
    }
    else
    {
        history.insert(ObjectDataHistory::value_type(this, true));
        out << sb;
        for(DataMemberMap::const_iterator q = _members.begin(); q != _members.end(); ++q)
        {
            out << nl << q->first << " = ";
            q->second->printI(out, history);
        }
        out << eb;
    }
}

void
Transform::ObjectData::destroyI()
{
    DataMemberMap members = _members;

    _members.clear();
    _marshaler = 0;

    for(DataMemberMap::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        p->second->destroy();
    }
}

//
// ObjectRef
//
Transform::ObjectRef::ObjectRef(const DataFactoryPtr& factory, const Slice::TypePtr& type, bool readOnly) :
    Data(factory->getErrorReporter(), readOnly), _factory(factory), _type(type)
{
}

Transform::ObjectRef::ObjectRef(const DataFactoryPtr& factory, const Slice::TypePtr& type, bool readOnly,
                                const ObjectDataPtr& value) :
    Data(factory->getErrorReporter(), readOnly), _factory(factory), _type(type), _value(value)
{
    _value->incRef();
}

Transform::DataPtr
Transform::ObjectRef::getMember(const string& member) const
{
    if(!_value)
    {
        _errorReporter->error("member `" + member + "' requested of nil object");
    }

    return _value->getMember(member);
}

Transform::DataPtr
Transform::ObjectRef::getElement(const DataPtr&) const
{
    _errorReporter->error("element requested of object value");
    return 0;
}

bool
Transform::ObjectRef::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
Transform::ObjectRef::operator!=(const Data& rhs) const
{
    const ObjectRef* r = dynamic_cast<const ObjectRef*>(&rhs);
    if(!r)
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
    }

    //
    // Reference comparison.
    //
    return _value.get() != r->_value.get();
}

bool
Transform::ObjectRef::operator<(const Data& rhs) const
{
    _errorReporter->error("object comparison not supported");
    return false;
}

void
Transform::ObjectRef::registerObjects(ObjectDataMap& m) const
{
    if(_value)
    {
        _value->registerObjects(m);
    }
}

void
Transform::ObjectRef::transform(const DataPtr& data, DataInterceptor& interceptor)
{
    if(interceptor.preTransform(this, data))
    {
        transformI(data, interceptor);
    }
    interceptor.postTransform(this, data);
}

void
Transform::ObjectRef::set(const DataPtr& value, DataInterceptor& interceptor, bool)
{
    transformI(value, interceptor);
}

Transform::DataPtr
Transform::ObjectRef::clone() const
{
    return new ObjectRef(_factory, _type, _value);
}

Slice::TypePtr
Transform::ObjectRef::getType() const
{
    return _type;
}

void
Transform::ObjectRef::destroy()
{
    if(_value)
    {
        _value->decRef();
        _value = 0;
    }
}

void
Transform::ObjectRef::marshal(IceInternal::BasicStream& os) const
{
    if(!_value)
    {
        os.write(Ice::ObjectPtr());
        return;
    }

    _value->marshal(os);
}

static void
patchObject(void* addr, Ice::ObjectPtr& v)
{
    if(v)
    {
        Transform::ObjectRef* ref = static_cast<Transform::ObjectRef*>(addr);
        assert(ref);

        Transform::ObjectReaderPtr reader = Transform::ObjectReaderPtr::dynamicCast(v);
        assert(reader);

        ref->setValue(reader->getValue());
    }
}

void
Transform::ObjectRef::unmarshal(IceInternal::BasicStream& is)
{
    is.read(patchObject, this);
}

bool
Transform::ObjectRef::booleanValue(bool) const
{
    _errorReporter->error(typeName(_type) + " cannot be converted to boolean");
    return 0;
}

Ice::Long
Transform::ObjectRef::integerValue(bool) const
{
    _errorReporter->error(typeName(_type) + " cannot be converted to integer");
    return 0;
}

double
Transform::ObjectRef::doubleValue(bool) const
{
    _errorReporter->error(typeName(_type) + " cannot be converted to double");
    return 0;
}

string
Transform::ObjectRef::stringValue(bool) const
{
    _errorReporter->error(typeName(_type) + " cannot be converted to string");
    return string();
}

string
Transform::ObjectRef::toString() const
{
    return stringValue();
}

void
Transform::ObjectRef::instantiate()
{
    ObjectDataPtr data = new ObjectData(_factory, _type, _readOnly);
    setValue(data);
}

void
Transform::ObjectRef::setValue(const ObjectDataPtr& value)
{
    // TODO: Type check?
    ObjectDataPtr oldValue = _value;
    _value = value;
    if(_value)
    {
        _value->incRef();
    }
    if(oldValue)
    {
        oldValue->decRef();
    }
}

void
Transform::ObjectRef::printI(IceUtil::Output& out, ObjectDataHistory& history) const
{
    if(!_value)
    {
        out << typeName(_type) << "(nil)";
    }
    else
    {
        ostringstream ostr;
        _value->printI(out, history);
    }
}

void
Transform::ObjectRef::transformI(const DataPtr& data, DataInterceptor& interceptor)
{
    ObjectRefPtr o = ObjectRefPtr::dynamicCast(data);
    if(!o)
    {
        _errorReporter->typeMismatchError(_type, data->getType(), false);
    }

    if(!o->_value)
    {
        //
        // Allow a nil value from type Object.
        //
        if(Slice::BuiltinPtr::dynamicCast(o->_type) || isCompatible(_type, o->_type, interceptor))
        {
            setValue(0);
        }
        else
        {
            _errorReporter->typeMismatchError(_type, o->_type, false);
        }
    }
    else
    {
        Slice::TypePtr otype = o->_value->getType();
        if(isCompatible(_type, otype, interceptor))
        {
            //
            // If the types are in the same Slice unit, then we can simply
            // copy the reference. Otherwise, we check the object map to
            // see if an equivalent object has already been created, and
            // if not, then we have to create one.
            //
            if(_type->unit().get() == otype->unit().get())
            {
                setValue(o->_value);
            }
            else
            {
                ObjectDataMap& objectMap = interceptor.getObjectMap();
                ObjectDataMap::iterator p = objectMap.find(o->_value.get());
                if(p != objectMap.end() && p->second)
                {
                    setValue(p->second);
                }
                else
                {
                    //
                    // If the type has been renamed, we need to get its equivalent
                    // in the new Slice definitions.
                    //
                    Slice::TypePtr newType = interceptor.getRename(otype);
                    if(!newType)
                    {
                        string name = typeName(otype);
                        Slice::TypeList l = _type->unit()->lookupType(name, false);
                        if(l.empty())
                        {
                            throw ClassNotFoundException(name);
                        }
                        newType = l.front();
                    }

                    //
                    // Use createObject() so that an initializer is invoked if necessary.
                    //
                    DataPtr newObj = _factory->createObject(newType, _readOnly);
                    ObjectRefPtr newRef = ObjectRefPtr::dynamicCast(newObj);
                    assert(newRef);

                    try
                    {
                        newRef->_value->transform(o->_value, interceptor);
                    }
                    catch(...)
                    {
                        newObj->destroy();
                        throw;
                    }

                    setValue(newRef->_value);
                    newObj->destroy();
                }
            }
        }
        else
        {
            _errorReporter->typeMismatchError(_type, otype, false);
        }
    }
}

//
// ObjectFactory
//
Transform::ObjectFactory::ObjectFactory(const DataFactoryPtr& factory, const Slice::UnitPtr& unit) :
    _factory(factory), _unit(unit)
{
}

Ice::ObjectPtr
Transform::ObjectFactory::create(const string& id)
{
    Ice::ObjectPtr result;

    if(id == Ice::Object::ice_staticId())
    {
        result = new ObjectReader(_factory, _unit->builtin(Slice::Builtin::KindObject));
    }
    else
    {
        Slice::TypeList l = _unit->lookupTypeNoBuiltin(id);
        if(!l.empty())
        {
            Slice::ClassDeclPtr decl = Slice::ClassDeclPtr::dynamicCast(l.front());
            if(!decl)
            {
                _factory->getErrorReporter()->error("Slice definition for `" + id + "' is not a class");
            }
            Slice::ClassDefPtr def = decl->definition();
            if(!def)
            {
                _factory->getErrorReporter()->error("no class definition for `" + id + "'");
            }
            result = new ObjectReader(_factory, decl);
        }
    }

    return result;
}

void
Transform::ObjectFactory::destroy()
{
}
