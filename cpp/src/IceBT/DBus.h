// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BT_DBUS_H
#define ICE_BT_DBUS_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

using namespace std;

namespace IceBT
{
namespace DBus
{

class Exception
{
public:

    std::string reason;

protected:

    Exception() {}
    Exception(const std::string& s) : reason(s) {}
};

//
// Type is the base class for a hierarchy representing DBus data types.
//
class Type;
typedef IceUtil::Handle<Type> TypePtr;

class Type : public IceUtil::SimpleShared
{
public:

    enum Kind
    {
        KindInvalid,
        KindBoolean,
        KindByte,
        KindUint16,
        KindInt16,
        KindUint32,
        KindInt32,
        KindUint64,
        KindInt64,
        KindDouble,
        KindString,
        KindObjectPath,
        KindSignature,
        KindUnixFD,
        KindArray,
        KindVariant,
        KindStruct,
        KindDictEntry
    };

    static TypePtr getPrimitive(Kind);

    virtual Kind getKind() const = 0;
    virtual std::string getSignature() const = 0;

protected:

    Type() {}
};

class ArrayType : public Type
{
public:

    ArrayType(const TypePtr& t) :
        elementType(t)
    {
    }

    virtual Kind getKind() const
    {
        return KindArray;
    }

    virtual std::string getSignature() const;

    TypePtr elementType;
};
typedef IceUtil::Handle<ArrayType> ArrayTypePtr;

class VariantType : public Type
{
public:

    VariantType() {}

    virtual Kind getKind() const
    {
        return KindVariant;
    }

    virtual std::string getSignature() const;
};
typedef IceUtil::Handle<VariantType> VariantTypePtr;

class StructType : public Type
{
public:

    StructType(const std::vector<TypePtr>& types) :
        memberTypes(types)
    {
    }

    virtual Kind getKind() const
    {
        return KindStruct;
    }

    virtual std::string getSignature() const;

    std::vector<TypePtr> memberTypes;
};
typedef IceUtil::Handle<StructType> StructTypePtr;

class DictEntryType : public Type
{
public:

    DictEntryType(const TypePtr& k, const TypePtr& v) :
        keyType(k),
        valueType(v)
    {
    }

    virtual Kind getKind() const
    {
        return KindDictEntry;
    }

    virtual std::string getSignature() const;

    TypePtr keyType;
    TypePtr valueType;
};
typedef IceUtil::Handle<DictEntryType> DictEntryTypePtr;

//
// Value is the base class of a hierarchy representing DBus data values.
//
class Value;
typedef IceUtil::Handle<Value> ValuePtr;

class Value : public IceUtil::SimpleShared
{
public:

    virtual TypePtr getType() const = 0;

    virtual ValuePtr clone() const = 0;

    virtual std::string toString() const = 0;

protected:

    virtual void print(std::ostream&) = 0;

    friend std::ostream& operator<<(std::ostream&, const ValuePtr&);
};

inline std::ostream&
operator<<(std::ostream& ostr, const ValuePtr& v)
{
    if(v)
    {
        v->print(ostr);
    }
    else
    {
        ostr << "nil";
    }
    return ostr;
}

template<typename E, Type::Kind K>
class PrimitiveValue : public Value
{
public:

    PrimitiveValue() : v(E()), kind(K) {}
    PrimitiveValue(const E& val) : v(val), kind(K) {}

    virtual TypePtr getType() const
    {
        return Type::getPrimitive(kind);
    }

    virtual ValuePtr clone() const
    {
        return new PrimitiveValue(v);
    }

    virtual std::string toString() const
    {
        std::ostringstream out;
        out << v;
        return out.str();
    }

    E v;
    Type::Kind kind;

protected:

    virtual void print(std::ostream& ostr)
    {
        ostr << v;
    }
};

typedef PrimitiveValue<bool, Type::KindBoolean> BooleanValue;
typedef IceUtil::Handle<BooleanValue> BooleanValuePtr;
typedef PrimitiveValue<unsigned char, Type::KindByte> ByteValue;
typedef IceUtil::Handle<ByteValue> ByteValuePtr;
typedef PrimitiveValue<unsigned short, Type::KindUint16> Uint16Value;
typedef IceUtil::Handle<Uint16Value> Uint16ValuePtr;
typedef PrimitiveValue<short, Type::KindInt16> Int16Value;
typedef IceUtil::Handle<Int16Value> Int16ValuePtr;
typedef PrimitiveValue<unsigned int, Type::KindUint32> Uint32Value;
typedef IceUtil::Handle<Uint32Value> Uint32ValuePtr;
typedef PrimitiveValue<int, Type::KindInt32> Int32Value;
typedef IceUtil::Handle<Int32Value> Int32ValuePtr;
typedef PrimitiveValue<unsigned long long, Type::KindUint64> Uint64Value;
typedef IceUtil::Handle<Uint64Value> Uint64ValuePtr;
typedef PrimitiveValue<long long, Type::KindInt64> Int64Value;
typedef IceUtil::Handle<Int64Value> Int64ValuePtr;
typedef PrimitiveValue<double, Type::KindDouble> DoubleValue;
typedef IceUtil::Handle<DoubleValue> DoubleValuePtr;
typedef PrimitiveValue<string, Type::KindString> StringValue;
typedef IceUtil::Handle<StringValue> StringValuePtr;
typedef PrimitiveValue<string, Type::KindObjectPath> ObjectPathValue;
typedef IceUtil::Handle<ObjectPathValue> ObjectPathValuePtr;
typedef PrimitiveValue<string, Type::KindSignature> SignatureValue;
typedef IceUtil::Handle<SignatureValue> SignatureValuePtr;
typedef PrimitiveValue<unsigned int, Type::KindUnixFD> UnixFDValue;
typedef IceUtil::Handle<UnixFDValue> UnixFDValuePtr;

class VariantValue;
typedef IceUtil::Handle<VariantValue> VariantValuePtr;

class VariantValue : public Value
{
public:

    VariantValue() : _type(new VariantType) {}

    VariantValue(const ValuePtr& val) :
        v(val),
        _type(new VariantType)
    {
    }

    virtual TypePtr getType() const
    {
        return _type;
    }

    virtual ValuePtr clone() const
    {
        return const_cast<VariantValue*>(this);
    }

    virtual std::string toString() const
    {
        return v ? v->toString() : "nil";
    }

    ValuePtr v;

protected:

    virtual void print(std::ostream& ostr)
    {
        ostr << v;
    }

private:

    TypePtr _type;
};

class DictEntryValue;
typedef IceUtil::Handle<DictEntryValue> DictEntryValuePtr;

class DictEntryValue : public Value
{
public:

    DictEntryValue(const DictEntryTypePtr& t) : _type(t) {}

    DictEntryValue(const DictEntryTypePtr& t, const ValuePtr& k, const ValuePtr& v) :
        key(k),
        value(v),
        _type(t)
    {
    }

    virtual TypePtr getType() const
    {
        return _type;
    }

    virtual ValuePtr clone() const
    {
        DictEntryValuePtr r = new DictEntryValue(_type);
        r->key = key->clone();
        r->value = value->clone();
        return r;
    }

    virtual std::string toString() const
    {
        std::ostringstream out;
        out << key->toString() << "=" << value->toString();
        return out.str();
    }

    ValuePtr key;
    ValuePtr value;

protected:

    virtual void print(std::ostream& ostr)
    {
        ostr << '{' << key << ": " << value << '}' << endl;
    }

private:

    DictEntryTypePtr _type;
};

class ArrayValue;
typedef IceUtil::Handle<ArrayValue> ArrayValuePtr;

class ArrayValue : public Value
{
public:

    ArrayValue(const TypePtr& t) : _type(t) {}

    virtual TypePtr getType() const
    {
        return _type;
    }

    virtual ValuePtr clone() const
    {
        ArrayValuePtr r = new ArrayValue(_type);
        for(std::vector<ValuePtr>::const_iterator p = elements.begin(); p != elements.end(); ++p)
        {
            r->elements.push_back((*p)->clone());
        }
        return r;
    }

    virtual std::string toString() const
    {
        std::ostringstream out;
        for(std::vector<ValuePtr>::const_iterator p = elements.begin(); p != elements.end(); ++p)
        {
            if(p != elements.begin())
            {
                out << ',';
            }
            out << (*p)->toString();
        }
        return out.str();
    }

    void toStringMap(std::map<std::string, ValuePtr>& m)
    {
        for(std::vector<ValuePtr>::const_iterator p = elements.begin(); p != elements.end(); ++p)
        {
            DictEntryValuePtr de = DictEntryValuePtr::dynamicCast(*p);
            assert(de);
            StringValuePtr s = StringValuePtr::dynamicCast(de->key);
            assert(s);
            m[s->v] = de->value;
        }
    }

    std::vector<ValuePtr> elements;

protected:

    virtual void print(std::ostream& ostr)
    {
        for(std::vector<ValuePtr>::const_iterator p = elements.begin(); p != elements.end(); ++p)
        {
            ostr << *p << endl;
        }
    }

private:

    TypePtr _type;
};

class StructValue;
typedef IceUtil::Handle<StructValue> StructValuePtr;

class StructValue : public Value
{
public:

    StructValue(const StructTypePtr& t) : _type(t) {}

    virtual TypePtr getType() const
    {
        return _type;
    }

    virtual ValuePtr clone() const
    {
        StructValuePtr r = new StructValue(_type);
        for(std::vector<ValuePtr>::const_iterator p = members.begin(); p != members.end(); ++p)
        {
            r->members.push_back((*p)->clone());
        }
        return r;
    }

    virtual std::string toString() const
    {
        std::ostringstream out;
        for(std::vector<ValuePtr>::const_iterator p = members.begin(); p != members.end(); ++p)
        {
            if(p != members.begin())
            {
                out << ',';
            }
            out << (*p)->toString();
        }
        return out.str();
    }

    std::vector<ValuePtr> members;

protected:

    virtual void print(std::ostream& ostr)
    {
        for(std::vector<ValuePtr>::const_iterator p = members.begin(); p != members.end(); ++p)
        {
            ostr << *p << endl;
        }
    }

private:

    StructTypePtr _type;
};

//
// Message encapsulates a DBus message. It only provides the functionality required by the IceBT transport.
//
class Message;
typedef IceUtil::Handle<Message> MessagePtr;

class Message : public IceUtil::Shared
{
public:

    virtual bool isError() const = 0;
    virtual std::string getErrorName() const = 0;
    virtual void throwException() = 0;

    virtual bool isSignal() const = 0;
    virtual bool isMethodCall() const = 0;
    virtual bool isMethodReturn() const = 0;

    virtual std::string getPath() const = 0;
    virtual std::string getInterface() const = 0;
    virtual std::string getMember() const = 0;
    virtual std::string getDestination() const = 0;

    //
    // Writing arguments.
    //
    virtual void write(const ValuePtr&) = 0;
    virtual void write(const std::vector<ValuePtr>&) = 0;

    //
    // Reading arguments.
    //
    virtual bool checkTypes(const std::vector<TypePtr>&) const = 0;
    virtual ValuePtr read() = 0;
    virtual std::vector<ValuePtr> readAll() = 0;

    static MessagePtr createCall(const string& dest, const string& path, const string& iface, const string& method);
    static MessagePtr createReturn(const MessagePtr& call);
};

class AsyncResult;
typedef IceUtil::Handle<AsyncResult> AsyncResultPtr;

class AsyncCallback
#ifndef ICE_CPP11_MAPPING
    : public virtual IceUtil::Shared
#endif
{
public:

    virtual void completed(const AsyncResultPtr&) = 0;
};
ICE_DEFINE_PTR(AsyncCallbackPtr, AsyncCallback);

//
// The result of an asynchronous DBus operation.
//
class AsyncResult : public IceUtil::Shared
{
public:

    virtual bool isPending() const = 0;
    virtual bool isComplete() const = 0;

    virtual MessagePtr waitUntilFinished() const = 0;

    virtual MessagePtr getReply() const = 0;

    virtual void setCallback(const AsyncCallbackPtr&) = 0;
};

class Connection;
typedef IceUtil::Handle<Connection> ConnectionPtr;

//
// Allows a subclass to intercept DBus messages.
//
class Filter
#ifndef ICE_CPP11_MAPPING
    : public virtual IceUtil::Shared
#endif
{
public:

    //
    // Return true if message is handled or false otherwise.
    //
    virtual bool handleMessage(const ConnectionPtr&, const MessagePtr&) = 0;
};
ICE_DEFINE_PTR(FilterPtr, Filter);

//
// Allows a subclass to receive DBus method invocations.
//
class Service
#ifndef ICE_CPP11_MAPPING
    : public virtual IceUtil::Shared
#endif
{
public:

    virtual void handleMethodCall(const ConnectionPtr&, const MessagePtr&) = 0;
};
ICE_DEFINE_PTR(ServicePtr, Service);

//
// Encapsulates a DBus connection.
//
class Connection : public IceUtil::Shared
{
public:

    static ConnectionPtr getSystemBus();
    static ConnectionPtr getSessionBus();

    virtual void addFilter(const FilterPtr&) = 0;
    virtual void removeFilter(const FilterPtr&) = 0;

    virtual void addService(const std::string&, const ServicePtr&) = 0;
    virtual void removeService(const std::string&) = 0;

    //
    // Asynchronously invokes a method call. The returned AsyncResult can be used
    // to determine completion status and obtain the reply, or supply a callback
    // to be notified when the call completes.
    //
    virtual AsyncResultPtr callAsync(const MessagePtr&, const AsyncCallbackPtr& = 0) = 0;

    //
    // Sends a message without blocking. Use this to send signals and replies.
    //
    virtual void sendAsync(const MessagePtr&) = 0;

    virtual void close() = 0;

protected:

    Connection() {}
};

void initThreads();

}
}

#endif
