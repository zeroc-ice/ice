// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <FreezeScript/Data.h>
#include <FreezeScript/Util.h>
#include <FreezeScript/Exception.h>
#include <IceUtil/InputUtil.h>
#include <climits>

using namespace std;
using namespace IceUtil;

namespace FreezeScript
{

//
// ObjectWriter
//
class ObjectWriter : public Ice::ObjectWriter
{
public:

    ObjectWriter(const ObjectDataPtr&);

    virtual void write(const Ice::OutputStreamPtr&) const;

private:

    ObjectDataPtr _value;
};

//
// ReadObjectCallback
//
class ReadObjectCallback : public Ice::ReadObjectCallback
{
public:

    ReadObjectCallback(const ObjectRefPtr&);

    virtual void invoke(const Ice::ObjectPtr&);

private:

    ObjectRefPtr _ref;
};

//
// ObjectReader
//
class ObjectReader : public Ice::ObjectReader
{
public:

    ObjectReader(const DataFactoryPtr&, const Slice::TypePtr&);

    virtual void read(const Ice::InputStreamPtr&);

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
FreezeScript::ObjectWriter::ObjectWriter(const ObjectDataPtr& value) :
    _value(value)
{
}

void
FreezeScript::ObjectWriter::write(const Ice::OutputStreamPtr& out) const
{
    out->startObject(0);

    Slice::ClassDeclPtr decl = Slice::ClassDeclPtr::dynamicCast(_value->_type);
    Slice::ClassDefPtr type;
    if(decl)
    {
        type = decl->definition();
    }
    while(type)
    {
        Slice::ClassDefPtr base;
        Slice::ClassList bases = type->bases();
        if(!bases.empty() && !bases.front()->isInterface())
        {
            base = bases.front();
        }

        out->startSlice(type->scoped(), type->compactId(), !base);
        Slice::DataMemberList members = type->dataMembers();
        for(Slice::DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
        {
            DataMemberMap::const_iterator q = _value->_members.find((*p)->name());
            assert(q != _value->_members.end());
            q->second->marshal(out);
        }
        out->endSlice();

        type = base;
    }

    out->endObject();
}

//
// ObjectReader
//
FreezeScript::ObjectReader::ObjectReader(const DataFactoryPtr& factory, const Slice::TypePtr& type) :
    _factory(factory), _type(type)
{
}

void
FreezeScript::ObjectReader::read(const Ice::InputStreamPtr& in)
{
    const_cast<ObjectDataPtr&>(_value) = new ObjectData(_factory, _type, true);
    Slice::ClassDeclPtr decl = Slice::ClassDeclPtr::dynamicCast(_type);
    Slice::ClassDefPtr type;
    if(decl)
    {
        type = decl->definition();
    }

    in->startObject();

    while(type)
    {
        Slice::ClassDefPtr base;
        Slice::ClassList bases = type->bases();
        if(!bases.empty() && !bases.front()->isInterface())
        {
            base = bases.front();
        }

        in->startSlice();

        Slice::DataMemberList members = type->dataMembers();
        for(Slice::DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
        {
            DataMemberMap::iterator q = _value->_members.find((*p)->name());
            assert(q != _value->_members.end());
            q->second->unmarshal(in);
        }

        in->endSlice();

        type = base;
    }

    in->endObject(false);
}

FreezeScript::ObjectDataPtr
FreezeScript::ObjectReader::getValue() const
{
    return _value;
}

//
// DataFactory
//
FreezeScript::DataFactory::DataFactory(const Ice::CommunicatorPtr& communicator, const Slice::UnitPtr& unit,
                                       const ErrorReporterPtr& errorReporter) :
    _communicator(communicator), _unit(unit), _errorReporter(errorReporter)
{
}

FreezeScript::DataPtr
FreezeScript::DataFactory::create(const Slice::TypePtr& type, bool readOnly)
{
    DataPtr data = createImpl(type, readOnly);
    if(!readOnly)
    {
        initialize(data);
    }
    return data;
}

FreezeScript::DataPtr
FreezeScript::DataFactory::createBoolean(bool b, bool readOnly)
{
    DataPtr data = new BooleanData(getBuiltin(Slice::Builtin::KindBool), _errorReporter, readOnly, b);
    if(!readOnly)
    {
        initialize(data);
    }
    return data;
}

FreezeScript::DataPtr
FreezeScript::DataFactory::createInteger(Ice::Long i, bool readOnly)
{
    DataPtr data = new IntegerData(getBuiltin(Slice::Builtin::KindLong), _errorReporter, readOnly, i);
    if(!readOnly)
    {
        initialize(data);
    }
    return data;
}

FreezeScript::DataPtr
FreezeScript::DataFactory::createDouble(double d, bool readOnly)
{
    DataPtr data = new DoubleData(getBuiltin(Slice::Builtin::KindDouble), _errorReporter, readOnly, d);
    if(!readOnly)
    {
        initialize(data);
    }
    return data;
}

FreezeScript::DataPtr
FreezeScript::DataFactory::createString(const string& s, bool readOnly)
{
    DataPtr data = new StringData(this, getBuiltin(Slice::Builtin::KindString), _errorReporter, readOnly, s);
    if(!readOnly)
    {
        initialize(data);
    }
    return data;
}

FreezeScript::DataPtr
FreezeScript::DataFactory::createNil(bool readOnly)
{
    DataPtr data = new ObjectRef(this, getBuiltin(Slice::Builtin::KindObject), readOnly);
    if(!readOnly)
    {
        initialize(data);
    }
    return data;
}

FreezeScript::DataPtr
FreezeScript::DataFactory::createObject(const Slice::TypePtr& type, bool readOnly)
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
        _errorReporter->error("type `" + typeToString(type) + "' is not a class");
    }

    obj->instantiate();

    if(!readOnly)
    {
        initialize(obj);
    }

    return obj;
}

Slice::BuiltinPtr
FreezeScript::DataFactory::getBuiltin(Slice::Builtin::Kind kind) const
{
    return _unit->builtin(kind);
}

Ice::CommunicatorPtr
FreezeScript::DataFactory::getCommunicator() const
{
    return _communicator;
}

FreezeScript::ErrorReporterPtr
FreezeScript::DataFactory::getErrorReporter() const
{
    return _errorReporter;
}

FreezeScript::DataPtr
FreezeScript::DataFactory::createImpl(const Slice::TypePtr& type, bool readOnly)
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
FreezeScript::DataFactory::initialize(const DataPtr&)
{
}

//
// Data
//
FreezeScript::Data::Data(const ErrorReporterPtr& errorReporter, bool readOnly) :
    _errorReporter(errorReporter), _readOnly(readOnly)
{
}

FreezeScript::Data::~Data()
{
}

bool
FreezeScript::Data::readOnly() const
{
    return _readOnly;
}

//
// PrimitiveData
//
FreezeScript::PrimitiveData::PrimitiveData(const ErrorReporterPtr& errorReporter, bool readOnly) :
    Data(errorReporter, readOnly)
{
}

FreezeScript::DataPtr
FreezeScript::PrimitiveData::getMember(const string&) const
{
    return 0;
}

FreezeScript::DataPtr
FreezeScript::PrimitiveData::getElement(const DataPtr&) const
{
    _errorReporter->error("element requested of primitive value");
    return 0;
}

void
FreezeScript::PrimitiveData::destroy()
{
}

//
// BooleanData
//
FreezeScript::BooleanData::BooleanData(const Slice::BuiltinPtr& type, const ErrorReporterPtr& errorReporter,
                                       bool readOnly) :
    PrimitiveData(errorReporter, readOnly), _type(type), _value(false)
{
}

FreezeScript::BooleanData::BooleanData(const Slice::BuiltinPtr& type, const ErrorReporterPtr& errorReporter,
                                       bool readOnly, bool value) :
    PrimitiveData(errorReporter, readOnly), _type(type), _value(value)
{
}

bool
FreezeScript::BooleanData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
FreezeScript::BooleanData::operator!=(const Data& rhs) const
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
FreezeScript::BooleanData::operator<(const Data& rhs) const
{
    const BooleanData* b = dynamic_cast<const BooleanData*>(&rhs);
    if(!b)
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _value < b->_value;
}

FreezeScript::DataPtr
FreezeScript::BooleanData::clone() const
{
    return new BooleanData(_type, _errorReporter, _readOnly, _value);
}

Slice::TypePtr
FreezeScript::BooleanData::getType() const
{
    return _type;
}

void
FreezeScript::BooleanData::marshal(const Ice::OutputStreamPtr& out) const
{
    out->write(_value);
}

void
FreezeScript::BooleanData::unmarshal(const Ice::InputStreamPtr& in)
{
    in->read(_value);
}

bool
FreezeScript::BooleanData::booleanValue(bool) const
{
    return _value;
}

Ice::Long
FreezeScript::BooleanData::integerValue(bool) const
{
    _errorReporter->error("boolean cannot be converted to integer");
    return 0;
}

double
FreezeScript::BooleanData::doubleValue(bool) const
{
    _errorReporter->error("boolean cannot be converted to double");
    return 0;
}

string
FreezeScript::BooleanData::stringValue(bool convert) const
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
FreezeScript::BooleanData::toString() const
{
    return (_value ? "true" : "false");
}

void
FreezeScript::BooleanData::visit(DataVisitor& visitor)
{
    visitor.visitBoolean(this);
}

bool
FreezeScript::BooleanData::getValue() const
{
    return _value;
}

void
FreezeScript::BooleanData::setValue(bool v)
{
    _value = v;
}

//
// IntegerData
//
FreezeScript::IntegerData::IntegerData(const Slice::BuiltinPtr& type, const ErrorReporterPtr& errorReporter,
                                       bool readOnly) :
    PrimitiveData(errorReporter, readOnly), _type(type), _value(0)
{
}

FreezeScript::IntegerData::IntegerData(const Slice::BuiltinPtr& type, const ErrorReporterPtr& errorReporter,
                                       bool readOnly, Ice::Long value) :
    PrimitiveData(errorReporter, readOnly), _type(type), _value(value)
{
}

bool
FreezeScript::IntegerData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
FreezeScript::IntegerData::operator!=(const Data& rhs) const
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
FreezeScript::IntegerData::operator<(const Data& rhs) const
{
    const IntegerData* i = dynamic_cast<const IntegerData*>(&rhs);
    if(!i)
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _value < i->_value;
}

FreezeScript::DataPtr
FreezeScript::IntegerData::clone() const
{
    return new IntegerData(_type, _errorReporter, _readOnly, _value);
}

Slice::TypePtr
FreezeScript::IntegerData::getType() const
{
    return _type;
}

void
FreezeScript::IntegerData::marshal(const Ice::OutputStreamPtr& out) const
{
    rangeCheck(_value, true);

    switch(_type->kind())
    {
    case Slice::Builtin::KindByte:
    {
        out->write(static_cast<Ice::Byte>(_value));
        break;
    }
    case Slice::Builtin::KindShort:
    {
        out->write(static_cast<Ice::Short>(_value));
        break;
    }
    case Slice::Builtin::KindInt:
    {
        out->write(static_cast<Ice::Int>(_value));
        break;
    }
    case Slice::Builtin::KindLong:
    {
        out->write(_value);
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
FreezeScript::IntegerData::unmarshal(const Ice::InputStreamPtr& in)
{
    switch(_type->kind())
    {
    case Slice::Builtin::KindByte:
    {
        Ice::Byte val;
        in->read(val);
        _value = val & 0xff;
        break;
    }
    case Slice::Builtin::KindShort:
    {
        Ice::Short val;
        in->read(val);
        _value = val;
        break;
    }
    case Slice::Builtin::KindInt:
    {
        Ice::Int val;
        in->read(val);
        _value = val;
        break;
    }
    case Slice::Builtin::KindLong:
    {
        Ice::Long val;
        in->read(val);
        _value = val;
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
FreezeScript::IntegerData::booleanValue(bool) const
{
    _errorReporter->error("integer cannot be converted to boolean");
    return 0;
}

Ice::Long
FreezeScript::IntegerData::integerValue(bool) const
{
    return _value;
}

double
FreezeScript::IntegerData::doubleValue(bool convert) const
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
FreezeScript::IntegerData::stringValue(bool convert) const
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
FreezeScript::IntegerData::toString() const
{
    return toString(_value);
}

void
FreezeScript::IntegerData::visit(DataVisitor& visitor)
{
    visitor.visitInteger(this);
}

Ice::Long
FreezeScript::IntegerData::getValue() const
{
    return _value;
}

void
FreezeScript::IntegerData::setValue(Ice::Long v, bool fatal)
{
    if(rangeCheck(v, fatal))
    {
        _value = v;
    }
}

bool
FreezeScript::IntegerData::rangeCheck(Ice::Long value, bool fatal) const
{
    switch(_type->kind())
    {
    case Slice::Builtin::KindByte:
    {
        if(value < 0 || value > 255)
        {
            _errorReporter->rangeError(toString(value), _type, fatal);
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
            _errorReporter->rangeError(toString(value), _type, fatal);
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
            _errorReporter->rangeError(toString(value), _type, fatal);
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
FreezeScript::IntegerData::toString(Ice::Long value)
{
    char buf[64];
#ifdef _WIN32
    sprintf_s(buf, "%I64d", value);
#elif defined(ICE_64)
    sprintf(buf, "%ld", value);
#else
    sprintf(buf, "%lld", value);
#endif
    return string(buf);
}

//
// DoubleData
//
FreezeScript::DoubleData::DoubleData(const Slice::BuiltinPtr& type, const ErrorReporterPtr& errorReporter,
                                     bool readOnly) :
    PrimitiveData(errorReporter, readOnly), _type(type), _value(0)
{
}

FreezeScript::DoubleData::DoubleData(const Slice::BuiltinPtr& type, const ErrorReporterPtr& errorReporter,
                                     bool readOnly, double value) :
    PrimitiveData(errorReporter, readOnly), _type(type), _value(value)
{
}

bool
FreezeScript::DoubleData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
FreezeScript::DoubleData::operator!=(const Data& rhs) const
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
FreezeScript::DoubleData::operator<(const Data& rhs) const
{
    const DoubleData* d = dynamic_cast<const DoubleData*>(&rhs);
    if(!d)
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _value < d->_value;
}

FreezeScript::DataPtr
FreezeScript::DoubleData::clone() const
{
    return new DoubleData(_type, _errorReporter, _readOnly, _value);
}

Slice::TypePtr
FreezeScript::DoubleData::getType() const
{
    return _type;
}

void
FreezeScript::DoubleData::marshal(const Ice::OutputStreamPtr& out) const
{
    switch(_type->kind())
    {
    case Slice::Builtin::KindFloat:
    {
        out->write(static_cast<Ice::Float>(_value));
        break;
    }
    case Slice::Builtin::KindDouble:
    {
        out->write(_value);
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
FreezeScript::DoubleData::unmarshal(const Ice::InputStreamPtr& in)
{
    switch(_type->kind())
    {
    case Slice::Builtin::KindFloat:
    {
        Ice::Float val;
        in->read(val);
        _value = val;
        break;
    }
    case Slice::Builtin::KindDouble:
    {
        in->read(_value);
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
FreezeScript::DoubleData::booleanValue(bool) const
{
    _errorReporter->error("double cannot be converted to boolean");
    return 0;
}

Ice::Long
FreezeScript::DoubleData::integerValue(bool convert) const
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
FreezeScript::DoubleData::doubleValue(bool) const
{
    return _value;
}

string
FreezeScript::DoubleData::stringValue(bool convert) const
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
FreezeScript::DoubleData::toString() const
{
    char buff[64];
    sprintf(buff, "%g", _value);
    return string(buff);
}

void
FreezeScript::DoubleData::visit(DataVisitor& visitor)
{
    visitor.visitDouble(this);
}

double
FreezeScript::DoubleData::getValue() const
{
    return _value;
}

void
FreezeScript::DoubleData::setValue(double v)
{
    _value = v;
}

//
// StringData
//
FreezeScript::StringData::StringData(const DataFactoryPtr& factory, const Slice::BuiltinPtr& type,
                                     const ErrorReporterPtr& errorReporter, bool readOnly) :
    PrimitiveData(errorReporter, readOnly), _factory(factory), _type(type)
{
    setValue("");
}

FreezeScript::StringData::StringData(const DataFactoryPtr& factory, const Slice::BuiltinPtr& type,
                                     const ErrorReporterPtr& errorReporter, bool readOnly, const string& value) :
    PrimitiveData(errorReporter, readOnly), _factory(factory), _type(type)
{
    setValue(value);
}

FreezeScript::DataPtr
FreezeScript::StringData::getMember(const string& member) const
{
    if(member == "length")
    {
        _length->setValue(static_cast<Ice::Long>(_value.size()), false);
        return _length;
    }

    return 0;
}

bool
FreezeScript::StringData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
FreezeScript::StringData::operator!=(const Data& rhs) const
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
FreezeScript::StringData::operator<(const Data& rhs) const
{
    const StringData* s = dynamic_cast<const StringData*>(&rhs);
    if(!s)
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _value < s->_value;
}

FreezeScript::DataPtr
FreezeScript::StringData::clone() const
{
    return new StringData(_factory, _type, _errorReporter, _readOnly, _value);
}

Slice::TypePtr
FreezeScript::StringData::getType() const
{
    return _type;
}

void
FreezeScript::StringData::marshal(const Ice::OutputStreamPtr& out) const
{
    out->write(_value);
}

void
FreezeScript::StringData::unmarshal(const Ice::InputStreamPtr& in)
{
    string val;
    in->read(val);
    setValue(val);
}

bool
FreezeScript::StringData::booleanValue(bool) const
{
    _errorReporter->error("string cannot be converted to boolean");
    return false;
}

Ice::Long
FreezeScript::StringData::integerValue(bool) const
{
    _errorReporter->error("string cannot be converted to integer");
    return 0;
}

double
FreezeScript::StringData::doubleValue(bool) const
{
    _errorReporter->error("string cannot be converted to double");
    return 0;
}

string
FreezeScript::StringData::stringValue(bool) const
{
    return _value;
}

string
FreezeScript::StringData::toString() const
{
    return _value;
}

void
FreezeScript::StringData::visit(DataVisitor& visitor)
{
    visitor.visitString(this);
}

string
FreezeScript::StringData::getValue() const
{
    return _value;
}

void
FreezeScript::StringData::setValue(const string& v)
{
    _value = v;
    _length = IntegerDataPtr::dynamicCast(_factory->createInteger(static_cast<Ice::Long>(_value.size()), true));
}

//
// ProxyData
//
FreezeScript::ProxyData::ProxyData(const Slice::TypePtr& type, const Ice::CommunicatorPtr& communicator,
                                   const ErrorReporterPtr& errorReporter, bool readOnly) :
    Data(errorReporter, readOnly), _type(type), _communicator(communicator)
{
}

FreezeScript::ProxyData::ProxyData(const Slice::TypePtr& type, const Ice::CommunicatorPtr& communicator,
                                   const ErrorReporterPtr& errorReporter, bool readOnly, const Ice::ObjectPrx& value) :
    Data(errorReporter, readOnly), _type(type), _communicator(communicator), _value(value)
{
}

FreezeScript::DataPtr
FreezeScript::ProxyData::getMember(const string& /*member*/) const
{
    // TODO: Support members (id, facet, etc.)?

    return 0;
}

FreezeScript::DataPtr
FreezeScript::ProxyData::getElement(const DataPtr&) const
{
    _errorReporter->error("element requested of proxy value");
    return 0;
}

bool
FreezeScript::ProxyData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
FreezeScript::ProxyData::operator!=(const Data& rhs) const
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
FreezeScript::ProxyData::operator<(const Data& rhs) const
{
    const ProxyData* p = dynamic_cast<const ProxyData*>(&rhs);
    if(!p)
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _value < p->_value;
}

FreezeScript::DataPtr
FreezeScript::ProxyData::clone() const
{
    return new ProxyData(_type, _communicator, _errorReporter, _readOnly, _value);
}

Slice::TypePtr
FreezeScript::ProxyData::getType() const
{
    return _type;
}

void
FreezeScript::ProxyData::destroy()
{
}

void
FreezeScript::ProxyData::marshal(const Ice::OutputStreamPtr& out) const
{
    out->write(_value);
}

void
FreezeScript::ProxyData::unmarshal(const Ice::InputStreamPtr& in)
{
    in->read(_value);
}

bool
FreezeScript::ProxyData::booleanValue(bool) const
{
    _errorReporter->error("proxy cannot be converted to boolean");
    return 0;
}

Ice::Long
FreezeScript::ProxyData::integerValue(bool) const
{
    _errorReporter->error("proxy cannot be converted to integer");
    return 0;
}

double
FreezeScript::ProxyData::doubleValue(bool) const
{
    _errorReporter->error("proxy cannot be converted to double");
    return 0;
}

string
FreezeScript::ProxyData::stringValue(bool) const
{
    _errorReporter->error("proxy cannot be converted to string");
    return string();
}

string
FreezeScript::ProxyData::toString() const
{
    if(_value && _str.empty())
    {
        const_cast<string&>(_str) = _communicator->proxyToString(_value);
    }
    return _str;
}

void
FreezeScript::ProxyData::visit(DataVisitor& visitor)
{
    visitor.visitProxy(this);
}

Ice::ObjectPrx
FreezeScript::ProxyData::getValue() const
{
    return _value;
}

void
FreezeScript::ProxyData::setValue(const string& str, bool fatal)
{
    try
    {
        _value = _communicator->stringToProxy(str);
        _str = str;
    }
    catch(const Ice::LocalException&)
    {
        _errorReporter->conversionError(str, _type, fatal);
    }
}

void
FreezeScript::ProxyData::setValue(const Ice::ObjectPrx& v)
{
    _value = v;
    _str.clear();
}


//
// StructData
//
FreezeScript::StructData::StructData(const DataFactoryPtr& factory, const Slice::StructPtr& type,
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

FreezeScript::StructData::StructData(const Slice::StructPtr& type, const ErrorReporterPtr& errorReporter,
                                     bool readOnly, const DataMemberMap& members) :
    Data(errorReporter, readOnly), _type(type)
{
    for(DataMemberMap::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        _members[p->first] = p->second->clone();
    }
}

FreezeScript::DataPtr
FreezeScript::StructData::getMember(const string& member) const
{
    DataMemberMap::const_iterator p = _members.find(member);
    if(p != _members.end())
    {
        return p->second;
    }

    return 0;
}

FreezeScript::DataPtr
FreezeScript::StructData::getElement(const DataPtr&) const
{
    _errorReporter->error("element requested of struct value");
    return 0;
}

bool
FreezeScript::StructData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
FreezeScript::StructData::operator!=(const Data& rhs) const
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
FreezeScript::StructData::operator<(const Data& rhs) const
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

FreezeScript::DataPtr
FreezeScript::StructData::clone() const
{
    return new StructData(_type, _errorReporter, _readOnly, _members);
}

Slice::TypePtr
FreezeScript::StructData::getType() const
{
    return _type;
}

void
FreezeScript::StructData::destroy()
{
    for(DataMemberMap::const_iterator p = _members.begin(); p != _members.end(); ++p)
    {
        p->second->destroy();
    }
}

void
FreezeScript::StructData::marshal(const Ice::OutputStreamPtr& out) const
{
    Slice::DataMemberList members = _type->dataMembers();
    for(Slice::DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
    {
        DataMemberMap::const_iterator q = _members.find((*p)->name());
        assert(q != _members.end());
        q->second->marshal(out);
    }
}

void
FreezeScript::StructData::unmarshal(const Ice::InputStreamPtr& in)
{
    Slice::DataMemberList members = _type->dataMembers();
    for(Slice::DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
    {
        DataMemberMap::const_iterator q = _members.find((*p)->name());
        assert(q != _members.end());
        q->second->unmarshal(in);
    }
}

bool
FreezeScript::StructData::booleanValue(bool) const
{
    _errorReporter->error("struct " + typeToString(_type) + " cannot be converted to boolean");
    return 0;
}

Ice::Long
FreezeScript::StructData::integerValue(bool) const
{
    _errorReporter->error("struct " + typeToString(_type) + " cannot be converted to integer");
    return 0;
}

double
FreezeScript::StructData::doubleValue(bool) const
{
    _errorReporter->error("struct " + typeToString(_type) + " cannot be converted to double");
    return 0;
}

string
FreezeScript::StructData::stringValue(bool) const
{
    _errorReporter->error("struct " + typeToString(_type) + " cannot be converted to string");
    return string();
}

string
FreezeScript::StructData::toString() const
{
    return stringValue();
}

void
FreezeScript::StructData::visit(DataVisitor& visitor)
{
    visitor.visitStruct(this);
}

FreezeScript::DataMemberMap&
FreezeScript::StructData::getMembers()
{
    return _members;
}

//
// SequenceData
//
FreezeScript::SequenceData::SequenceData(const DataFactoryPtr& factory, const Slice::SequencePtr& type,
                                         const ErrorReporterPtr& errorReporter, bool readOnly) :
    Data(errorReporter, readOnly), _factory(factory), _type(type)
{
    _length = IntegerDataPtr::dynamicCast(_factory->createInteger(0, true));
}

FreezeScript::SequenceData::SequenceData(const DataFactoryPtr& factory, const Slice::SequencePtr& type,
                                         const ErrorReporterPtr& errorReporter, bool readOnly,
                                         const DataList& elements) :
    Data(errorReporter, readOnly), _factory(factory), _type(type)
{
    for(DataList::const_iterator p = elements.begin(); p != elements.end(); ++p)
    {
        _elements.push_back((*p)->clone());
    }
    _length = IntegerDataPtr::dynamicCast(_factory->createInteger(static_cast<Ice::Long>(_elements.size()), true));
}

FreezeScript::DataPtr
FreezeScript::SequenceData::getMember(const string& member) const
{
    if(member == "length")
    {
        _length->setValue(static_cast<Ice::Long>(_elements.size()), false);
        return _length;
    }

    return 0;
}

FreezeScript::DataPtr
FreezeScript::SequenceData::getElement(const DataPtr& element) const
{
    IntegerDataPtr i = IntegerDataPtr::dynamicCast(element);
    if(!i)
    {
        _errorReporter->error("invalid sequence index type " + typeToString(element->getType()));
    }

    Ice::Long iv = i->integerValue();
    if(iv < 0 || iv >= static_cast<Ice::Long>(_elements.size()))
    {
        _errorReporter->error("sequence index " + i->toString() + " out of range");
    }

    return _elements[static_cast<DataList::size_type>(iv)];
}

bool
FreezeScript::SequenceData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
FreezeScript::SequenceData::operator!=(const Data& rhs) const
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
FreezeScript::SequenceData::operator<(const Data& rhs) const
{
    const SequenceData* s = dynamic_cast<const SequenceData*>(&rhs);
    if(!s || _type->scoped() != s->_type->scoped())
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _elements < s->_elements;
}

FreezeScript::DataPtr
FreezeScript::SequenceData::clone() const
{
    return new SequenceData(_factory, _type, _errorReporter, _readOnly, _elements);
}

Slice::TypePtr
FreezeScript::SequenceData::getType() const
{
    return _type;
}

void
FreezeScript::SequenceData::destroy()
{
    for(DataList::const_iterator p = _elements.begin(); p != _elements.end(); ++p)
    {
        (*p)->destroy();
    }
}

void
FreezeScript::SequenceData::marshal(const Ice::OutputStreamPtr& out) const
{
    out->writeSize(static_cast<int>(_elements.size()));
    for(DataList::const_iterator p = _elements.begin(); p != _elements.end(); ++p)
    {
        (*p)->marshal(out);
    }
}

void
FreezeScript::SequenceData::unmarshal(const Ice::InputStreamPtr& in)
{
    Slice::TypePtr type = _type->type();
    Ice::Int sz = in->readSize();
    _elements.reserve(sz);
    for(Ice::Int i = 0; i < sz; ++i)
    {
        DataPtr v = _factory->create(type, _readOnly);
        v->unmarshal(in);
        _elements.push_back(v);
    }
    _length->setValue(sz, false);
}

bool
FreezeScript::SequenceData::booleanValue(bool) const
{
    _errorReporter->error("sequence " + typeToString(_type) + " cannot be converted to boolean");
    return 0;
}

Ice::Long
FreezeScript::SequenceData::integerValue(bool) const
{
    _errorReporter->error("sequence " + typeToString(_type) + " cannot be converted to integer");
    return 0;
}

double
FreezeScript::SequenceData::doubleValue(bool) const
{
    _errorReporter->error("sequence " + typeToString(_type) + " cannot be converted to double");
    return 0;
}

string
FreezeScript::SequenceData::stringValue(bool) const
{
    _errorReporter->error("sequence " + typeToString(_type) + " cannot be converted to string");
    return string();
}

string
FreezeScript::SequenceData::toString() const
{
    return stringValue();
}

void
FreezeScript::SequenceData::visit(DataVisitor& visitor)
{
    visitor.visitSequence(this);
}

FreezeScript::DataList&
FreezeScript::SequenceData::getElements()
{
    return _elements;
}

//
// EnumData
//
FreezeScript::EnumData::EnumData(const Slice::EnumPtr& type, const ErrorReporterPtr& errorReporter, bool readOnly) :
    Data(errorReporter, readOnly), _type(type), _value(0)
{
    _count = static_cast<int>(type->getEnumerators().size());
}

FreezeScript::EnumData::EnumData(const Slice::EnumPtr& type, const ErrorReporterPtr& errorReporter, bool readOnly,
                                 Ice::Int value) :
    Data(errorReporter, readOnly), _type(type), _value(value)
{
    _count = static_cast<int>(type->getEnumerators().size());
}

FreezeScript::DataPtr
FreezeScript::EnumData::getMember(const string&) const
{
    return 0;
}

FreezeScript::DataPtr
FreezeScript::EnumData::getElement(const DataPtr&) const
{
    _errorReporter->error("element requested of enum value");
    return 0;
}

bool
FreezeScript::EnumData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
FreezeScript::EnumData::operator!=(const Data& rhs) const
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
FreezeScript::EnumData::operator<(const Data& rhs) const
{
    const EnumData* e = dynamic_cast<const EnumData*>(&rhs);
    if(!e || _type->scoped() != e->_type->scoped())
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _value < e->_value;
}

FreezeScript::DataPtr
FreezeScript::EnumData::clone() const
{
    return new EnumData(_type, _errorReporter, _value);
}

Slice::TypePtr
FreezeScript::EnumData::getType() const
{
    return _type;
}

void
FreezeScript::EnumData::destroy()
{
}

void
FreezeScript::EnumData::marshal(const Ice::OutputStreamPtr& out) const
{
    if(_count <= 127)
    {
        out->write(static_cast<Ice::Byte>(_value));
    }
    else if(_count <= 32767)
    {
        out->write(static_cast<Ice::Short>(_value));
    }
    else
    {
        out->write(_value);
    }
}

void
FreezeScript::EnumData::unmarshal(const Ice::InputStreamPtr& in)
{
    if(_count <= 127)
    {
        Ice::Byte val;
        in ->read(val);
        _value = val & 0xff;
    }
    else if(_count <= 32767)
    {
        Ice::Short val;
        in->read(val);
        _value = val;
    }
    else
    {
        in->read(_value);
    }
}

bool
FreezeScript::EnumData::booleanValue(bool) const
{
    _errorReporter->error("enum " + typeToString(_type) + " cannot be converted to boolean");
    return 0;
}

Ice::Long
FreezeScript::EnumData::integerValue(bool convert) const
{
    if(convert)
    {
        return _value;
    }
    else
    {
        _errorReporter->error("enum " + typeToString(_type) + " cannot be converted to integer");
        return 0;
    }
}

double
FreezeScript::EnumData::doubleValue(bool) const
{
    _errorReporter->error("enum " + typeToString(_type) + " cannot be converted to double");
    return 0;
}

string
FreezeScript::EnumData::stringValue(bool) const
{
    _errorReporter->error("enum " + typeToString(_type) + " cannot be converted to string");
    return string();
}

string
FreezeScript::EnumData::toString() const
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

void
FreezeScript::EnumData::visit(DataVisitor& visitor)
{
    visitor.visitEnum(this);
}

Ice::Int
FreezeScript::EnumData::getValue() const
{
    return _value;
}

bool
FreezeScript::EnumData::setValue(Ice::Int v)
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
FreezeScript::EnumData::setValueAsString(const string& v)
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

//
// DictionaryData
//
FreezeScript::DictionaryData::DictionaryData(const DataFactoryPtr& factory, const Slice::DictionaryPtr& type,
                                             const ErrorReporterPtr& errorReporter, bool readOnly) :
    Data(errorReporter, readOnly), _factory(factory), _type(type)
{
    _length = IntegerDataPtr::dynamicCast(_factory->createInteger(0, true));
}

FreezeScript::DictionaryData::DictionaryData(const DataFactoryPtr& factory, const Slice::DictionaryPtr& type,
                                             const ErrorReporterPtr& errorReporter, bool readOnly, const DataMap& m) :
    Data(errorReporter, readOnly), _factory(factory), _type(type)
{
    for(DataMap::const_iterator p = m.begin(); p != m.end(); ++p)
    {
        _map.insert(DataMap::value_type(p->first->clone(), p->second->clone()));
    }
    _length = IntegerDataPtr::dynamicCast(_factory->createInteger(static_cast<Ice::Long>(_map.size()), true));
}

FreezeScript::DataPtr
FreezeScript::DictionaryData::getMember(const string& member) const
{
    if(member == "length")
    {
        _length->setValue(static_cast<Ice::Long>(_map.size()), false);
        return _length;
    }

    return 0;
}

FreezeScript::DataPtr
FreezeScript::DictionaryData::getElement(const DataPtr& element) const
{
    DataMap::const_iterator p = _map.find(element);
    if(p != _map.end())
    {
        return p->second;
    }

    return 0;
}

bool
FreezeScript::DictionaryData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
FreezeScript::DictionaryData::operator!=(const Data& rhs) const
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
FreezeScript::DictionaryData::operator<(const Data& rhs) const
{
    const DictionaryData* d = dynamic_cast<const DictionaryData*>(&rhs);
    if(!d || _type->scoped() != d->_type->scoped())
    {
        _errorReporter->typeMismatchError(_type, rhs.getType(), true);
        assert(false);
    }

    return _map < d->_map;
}

FreezeScript::DataPtr
FreezeScript::DictionaryData::clone() const
{
    return new DictionaryData(_factory, _type, _errorReporter, _readOnly, _map);
}

Slice::TypePtr
FreezeScript::DictionaryData::getType() const
{
    return _type;
}

void
FreezeScript::DictionaryData::destroy()
{
    for(DataMap::const_iterator p = _map.begin(); p != _map.end(); ++p)
    {
        p->first->destroy();
        p->second->destroy();
    }
}

void
FreezeScript::DictionaryData::marshal(const Ice::OutputStreamPtr& out) const
{
    out->writeSize(static_cast<int>(_map.size()));
    for(DataMap::const_iterator p = _map.begin(); p != _map.end(); ++p)
    {
        p->first->marshal(out);
        p->second->marshal(out);
    }
}

void
FreezeScript::DictionaryData::unmarshal(const Ice::InputStreamPtr& in)
{
    Slice::TypePtr keyType = _type->keyType();
    Slice::TypePtr valueType = _type->valueType();

    Ice::Int sz = in->readSize();

    for(Ice::Int i = 0; i < sz; ++i)
    {
        DataPtr key = _factory->create(keyType, _readOnly);
        key->unmarshal(in);

        DataPtr value = _factory->create(valueType, _readOnly);
        value->unmarshal(in);

        _map.insert(DataMap::value_type(key, value));
    }

    _length->setValue(static_cast<Ice::Long>(_map.size()), false);
}

bool
FreezeScript::DictionaryData::booleanValue(bool) const
{
    _errorReporter->error("dictionary " + typeToString(_type) + " cannot be converted to boolean");
    return 0;
}

Ice::Long
FreezeScript::DictionaryData::integerValue(bool) const
{
    _errorReporter->error("dictionary " + typeToString(_type) + " cannot be converted to integer");
    return 0;
}

double
FreezeScript::DictionaryData::doubleValue(bool) const
{
    _errorReporter->error("dictionary " + typeToString(_type) + " cannot be converted to double");
    return 0;
}

string
FreezeScript::DictionaryData::stringValue(bool) const
{
    _errorReporter->error("dictionary " + typeToString(_type) + " cannot be converted to string");
    return string();
}

string
FreezeScript::DictionaryData::toString() const
{
    return stringValue();
}

void
FreezeScript::DictionaryData::visit(DataVisitor& visitor)
{
    visitor.visitDictionary(this);
}

FreezeScript::DataMap&
FreezeScript::DictionaryData::getElements()
{
    return _map;
}

//
// ObjectData
//
FreezeScript::ObjectData::ObjectData(const DataFactoryPtr& factory, const Slice::TypePtr& type, bool readOnly) :
    Data(factory->getErrorReporter(), readOnly), _type(type), _refCount(0)
{
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

FreezeScript::DataPtr
FreezeScript::ObjectData::getMember(const string& member) const
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

FreezeScript::DataPtr
FreezeScript::ObjectData::getElement(const DataPtr& /*element*/) const
{
    _errorReporter->error("element requested of object value");
    return 0;
}

bool
FreezeScript::ObjectData::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
FreezeScript::ObjectData::operator!=(const Data& /*rhs*/) const
{
    _errorReporter->error("object comparison not supported");
    return false;
}

bool
FreezeScript::ObjectData::operator<(const Data& /*rhs*/) const
{
    _errorReporter->error("object comparison not supported");
    return false;
}

FreezeScript::DataPtr
FreezeScript::ObjectData::clone() const
{
    assert(false);
    return 0;
}

Slice::TypePtr
FreezeScript::ObjectData::getType() const
{
    return _type;
}

void
FreezeScript::ObjectData::destroy()
{
    assert(false);
}

void
FreezeScript::ObjectData::marshal(const Ice::OutputStreamPtr& out) const
{
    if(!_marshaler)
    {
        const_cast<Ice::ObjectPtr&>(_marshaler) = new ObjectWriter(const_cast<ObjectData*>(this));
    }
    out->writeObject(_marshaler);
}

void
FreezeScript::ObjectData::unmarshal(const Ice::InputStreamPtr& /*in*/)
{
    //
    // Unmarshaling is done by ObjectReader.
    //
    assert(false);
}

bool
FreezeScript::ObjectData::booleanValue(bool) const
{
    assert(false);
    return 0;
}

Ice::Long
FreezeScript::ObjectData::integerValue(bool) const
{
    assert(false);
    return 0;
}

double
FreezeScript::ObjectData::doubleValue(bool) const
{
    assert(false);
    return 0;
}

string
FreezeScript::ObjectData::stringValue(bool) const
{
    assert(false);
    return string();
}

string
FreezeScript::ObjectData::toString() const
{
    assert(false);
    return string();
}

void
FreezeScript::ObjectData::visit(DataVisitor& /*visitor*/)
{
    assert(false);
}

FreezeScript::DataMemberMap&
FreezeScript::ObjectData::getMembers()
{
    return _members;
}

void
FreezeScript::ObjectData::incRef()
{
    _refCount++;
}

void
FreezeScript::ObjectData::decRef()
{
    assert(_refCount > 0);
    _refCount--;
    if(_refCount == 0)
    {
        destroyI();
    }
}

void
FreezeScript::ObjectData::destroyI()
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
FreezeScript::ObjectRef::ObjectRef(const DataFactoryPtr& factory, const Slice::TypePtr& type, bool readOnly) :
    Data(factory->getErrorReporter(), readOnly), _factory(factory), _type(type)
{
}

FreezeScript::ObjectRef::ObjectRef(const DataFactoryPtr& factory, const Slice::TypePtr& type, bool readOnly,
                                   const ObjectDataPtr& value) :
    Data(factory->getErrorReporter(), readOnly), _factory(factory), _type(type), _value(value)
{
    _value->incRef();
}

FreezeScript::DataPtr
FreezeScript::ObjectRef::getMember(const string& member) const
{
    if(!_value)
    {
        _errorReporter->error("member `" + member + "' requested of nil object");
    }

    return _value->getMember(member);
}

FreezeScript::DataPtr
FreezeScript::ObjectRef::getElement(const DataPtr&) const
{
    _errorReporter->error("element requested of object value");
    return 0;
}

bool
FreezeScript::ObjectRef::operator==(const Data& rhs) const
{
    return !operator!=(rhs);
}

bool
FreezeScript::ObjectRef::operator!=(const Data& rhs) const
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
FreezeScript::ObjectRef::operator<(const Data& /*rhs*/) const
{
    _errorReporter->error("object comparison not supported");
    return false;
}

FreezeScript::DataPtr
FreezeScript::ObjectRef::clone() const
{
    return new ObjectRef(_factory, _type, _value);
}

Slice::TypePtr
FreezeScript::ObjectRef::getType() const
{
    return _type;
}

void
FreezeScript::ObjectRef::destroy()
{
    if(_value)
    {
        _value->decRef();
        _value = 0;
    }
}

void
FreezeScript::ObjectRef::marshal(const Ice::OutputStreamPtr& out) const
{
    if(!_value)
    {
        out->writeObject(0);
        return;
    }

    _value->marshal(out);
}

FreezeScript::ReadObjectCallback::ReadObjectCallback(const ObjectRefPtr& ref) :
    _ref(ref)
{
}

void
FreezeScript::ReadObjectCallback::invoke(const Ice::ObjectPtr& p)
{
    if(p)
    {
        ObjectReaderPtr reader = ObjectReaderPtr::dynamicCast(p);
        assert(reader);

        _ref->setValue(reader->getValue());
    }
}

void
FreezeScript::ObjectRef::unmarshal(const Ice::InputStreamPtr& in)
{
    in->readObject(new ReadObjectCallback(this));
}

bool
FreezeScript::ObjectRef::booleanValue(bool) const
{
    _errorReporter->error(typeToString(_type) + " cannot be converted to boolean");
    return 0;
}

Ice::Long
FreezeScript::ObjectRef::integerValue(bool) const
{
    _errorReporter->error(typeToString(_type) + " cannot be converted to integer");
    return 0;
}

double
FreezeScript::ObjectRef::doubleValue(bool) const
{
    _errorReporter->error(typeToString(_type) + " cannot be converted to double");
    return 0;
}

string
FreezeScript::ObjectRef::stringValue(bool) const
{
    _errorReporter->error(typeToString(_type) + " cannot be converted to string");
    return string();
}

string
FreezeScript::ObjectRef::toString() const
{
    return stringValue();
}

void
FreezeScript::ObjectRef::visit(DataVisitor& visitor)
{
    visitor.visitObject(this);
}

void
FreezeScript::ObjectRef::instantiate()
{
    ObjectDataPtr data = new ObjectData(_factory, _type, _readOnly);
    setValue(data);
}

FreezeScript::ObjectDataPtr
FreezeScript::ObjectRef::getValue() const
{
    return _value;
}

void
FreezeScript::ObjectRef::setValue(const ObjectDataPtr& value)
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

//
// ObjectFactory
//
Ice::ObjectPtr
FreezeScript::ObjectFactory::create(const string& id)
{
    Ice::ObjectPtr result;

    if(_factory)
    {
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
    }

    return result;
}

void
FreezeScript::ObjectFactory::destroy()
{
}

void
FreezeScript::ObjectFactory::activate(const DataFactoryPtr& factory, const Slice::UnitPtr& unit)
{
    assert(!_factory && !_unit);
    _factory = factory;
    _unit = unit;
}
void
FreezeScript::ObjectFactory::deactivate()
{
    _factory = 0;
    _unit = 0;
}

//
// DataVisitor
//
FreezeScript::DataVisitor::~DataVisitor()
{
}

void
FreezeScript::DataVisitor::visitBoolean(const BooleanDataPtr&)
{
}

void
FreezeScript::DataVisitor::visitInteger(const IntegerDataPtr&)
{
}

void
FreezeScript::DataVisitor::visitDouble(const DoubleDataPtr&)
{
}

void
FreezeScript::DataVisitor::visitString(const StringDataPtr&)
{
}

void
FreezeScript::DataVisitor::visitProxy(const ProxyDataPtr&)
{
}

void
FreezeScript::DataVisitor::visitStruct(const StructDataPtr&)
{
}

void
FreezeScript::DataVisitor::visitSequence(const SequenceDataPtr&)
{
}

void
FreezeScript::DataVisitor::visitEnum(const EnumDataPtr&)
{
}

void
FreezeScript::DataVisitor::visitDictionary(const DictionaryDataPtr&)
{
}

void
FreezeScript::DataVisitor::visitObject(const ObjectRefPtr&)
{
}
