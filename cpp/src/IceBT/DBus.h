// Copyright (c) ZeroC, Inc.

#ifndef ICE_BT_DBUS_H
#define ICE_BT_DBUS_H

#include <cassert>
#include <cstdint>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

namespace IceBT::DBus
{
    class Exception
    {
    public:
        std::string reason;

    protected:
        Exception() = default;
        Exception(std::string s) : reason(std::move(s)) {}
    };

    //
    // Type is the base class for a hierarchy representing DBus data types.
    //
    class Type;
    using TypePtr = std::shared_ptr<Type>;

    class Type
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

        [[nodiscard]] virtual Kind getKind() const = 0;
        [[nodiscard]] virtual std::string getSignature() const = 0;

    protected:
        Type() = default;
    };

    class ArrayType : public Type
    {
    public:
        ArrayType(TypePtr t) : elementType(std::move(t)) {}

        [[nodiscard]] Kind getKind() const override { return KindArray; }

        [[nodiscard]] std::string getSignature() const override;

        TypePtr elementType;
    };
    using ArrayTypePtr = std::shared_ptr<ArrayType>;

    class VariantType : public Type
    {
    public:
        VariantType() = default;

        [[nodiscard]] Kind getKind() const override { return KindVariant; }

        [[nodiscard]] std::string getSignature() const override;
    };
    using VariantTypePtr = std::shared_ptr<VariantType>;

    class StructType : public Type
    {
    public:
        StructType(std::vector<TypePtr> types) : memberTypes(std::move(types)) {}

        [[nodiscard]] Kind getKind() const override { return KindStruct; }

        [[nodiscard]] std::string getSignature() const override;

        std::vector<TypePtr> memberTypes;
    };
    using StructTypePtr = std::shared_ptr<StructType>;

    class DictEntryType : public Type
    {
    public:
        DictEntryType(TypePtr k, TypePtr v) : keyType(std::move(k)), valueType(std::move(v)) {}

        [[nodiscard]] Kind getKind() const override { return KindDictEntry; }

        [[nodiscard]] std::string getSignature() const override;

        TypePtr keyType;
        TypePtr valueType;
    };
    using DictEntryTypePtr = std::shared_ptr<DictEntryType>;

    //
    // Value is the base class of a hierarchy representing DBus data values.
    //
    class Value;
    using ValuePtr = std::shared_ptr<Value>;

    class Value
    {
    public:
        [[nodiscard]] virtual TypePtr getType() const = 0;

        [[nodiscard]] virtual ValuePtr clone() const = 0;

        [[nodiscard]] virtual std::string toString() const = 0;

    protected:
        virtual void print(std::ostream&) = 0;

        friend std::ostream& operator<<(std::ostream&, const ValuePtr&);
    };

    inline std::ostream& operator<<(std::ostream& ostr, const ValuePtr& v)
    {
        if (v)
        {
            v->print(ostr);
        }
        else
        {
            ostr << "nil";
        }
        return ostr;
    }

    template<typename E, Type::Kind K> class PrimitiveValue final : public Value
    {
    public:
        PrimitiveValue() : v(E{}), kind(K) {}
        PrimitiveValue(E val) : v(std::move(val)), kind(K) {}

        [[nodiscard]] TypePtr getType() const final { return Type::getPrimitive(kind); }

        [[nodiscard]] ValuePtr clone() const final { return make_shared<PrimitiveValue>(v); }

        [[nodiscard]] std::string toString() const final
        {
            std::ostringstream out;
            out << v;
            return out.str();
        }

        E v;
        Type::Kind kind;

    protected:
        void print(std::ostream& ostr) final { ostr << v; }
    };

    using BooleanValue = PrimitiveValue<bool, Type::KindBoolean>;
    using BooleanValuePtr = std::shared_ptr<BooleanValue>;
    using ByteValue = PrimitiveValue<unsigned char, Type::KindByte>;
    using ByteValuePtr = std::shared_ptr<ByteValue>;
    using Uint16Value = PrimitiveValue<unsigned short, Type::KindUint16>;
    using Uint16ValuePtr = std::shared_ptr<Uint16Value>;
    using Int16Value = PrimitiveValue<short, Type::KindInt16>;
    using Int16ValuePtr = std::shared_ptr<Int16Value>;
    using Uint32Value = PrimitiveValue<unsigned int, Type::KindUint32>;
    using Uint32ValuePtr = std::shared_ptr<Uint32Value>;
    using Int32Value = PrimitiveValue<int, Type::KindInt32>;
    using Int32ValuePtr = std::shared_ptr<Int32Value>;
    using Uint64Value = PrimitiveValue<std::uint64_t, Type::KindUint64>;
    using Uint64ValuePtr = std::shared_ptr<Uint64Value>;
    using Int64Value = PrimitiveValue<std::int64_t, Type::KindInt64>;
    using Int64ValuePtr = std::shared_ptr<Int64Value>;
    using DoubleValue = PrimitiveValue<double, Type::KindDouble>;
    using DoubleValuePtr = std::shared_ptr<DoubleValue>;
    using StringValue = PrimitiveValue<string, Type::KindString>;
    using StringValuePtr = std::shared_ptr<StringValue>;
    using ObjectPathValue = PrimitiveValue<string, Type::KindObjectPath>;
    using ObjectPathValuePtr = std::shared_ptr<ObjectPathValue>;
    using SignatureValue = PrimitiveValue<string, Type::KindSignature>;
    using SignatureValuePtr = std::shared_ptr<SignatureValue>;
    using UnixFDValue = PrimitiveValue<unsigned int, Type::KindUnixFD>;
    using UnixFDValuePtr = std::shared_ptr<UnixFDValue>;

    class VariantValue;
    using VariantValuePtr = std::shared_ptr<VariantValue>;

    class VariantValue : public Value, public std::enable_shared_from_this<VariantValue>
    {
    public:
        VariantValue() : _type(make_shared<VariantType>()) {}

        VariantValue(ValuePtr val) : v(std::move(val)), _type(make_shared<VariantType>()) {}

        TypePtr getType() const override { return _type; }

        ValuePtr clone() const override { return const_cast<VariantValue*>(this)->shared_from_this(); }

        std::string toString() const override { return v ? v->toString() : "nil"; }

        ValuePtr v;

    protected:
        void print(std::ostream& ostr) override { ostr << v; }

    private:
        TypePtr _type;
    };

    class DictEntryValue;
    using DictEntryValuePtr = std::shared_ptr<DictEntryValue>;

    class DictEntryValue : public Value
    {
    public:
        DictEntryValue(DictEntryTypePtr t) : _type(std::move(t)) {}

        DictEntryValue(DictEntryTypePtr t, ValuePtr k, ValuePtr v)
            : key(std::move(k)),
              value(std::move(v)),
              _type(std::move(t))
        {
        }

        [[nodiscard]] TypePtr getType() const override { return _type; }

        [[nodiscard]] ValuePtr clone() const override
        {
            DictEntryValuePtr r = make_shared<DictEntryValue>(_type);
            r->key = key->clone();
            r->value = value->clone();
            return r;
        }

        [[nodiscard]] std::string toString() const override
        {
            std::ostringstream out;
            out << key->toString() << "=" << value->toString();
            return out.str();
        }

        ValuePtr key;
        ValuePtr value;

    protected:
        void print(std::ostream& ostr) override { ostr << '{' << key << ": " << value << '}' << endl; }

    private:
        DictEntryTypePtr _type;
    };

    class ArrayValue;
    using ArrayValuePtr = std::shared_ptr<ArrayValue>;

    class ArrayValue : public Value
    {
    public:
        ArrayValue(TypePtr t) : _type(std::move(t)) {}

        [[nodiscard]] TypePtr getType() const override { return _type; }

        [[nodiscard]] ValuePtr clone() const override
        {
            auto r = make_shared<ArrayValue>(_type);
            for (const auto& element : elements)
            {
                r->elements.push_back(element->clone());
            }
            return r;
        }

        [[nodiscard]] std::string toString() const override
        {
            std::ostringstream out;
            for (auto p = elements.begin(); p != elements.end(); ++p)
            {
                if (p != elements.begin())
                {
                    out << ',';
                }
                out << (*p)->toString();
            }
            return out.str();
        }

        void toStringMap(std::map<std::string, ValuePtr>& m)
        {
            for (const auto& element : elements)
            {
                auto de = dynamic_pointer_cast<DictEntryValue>(element);
                assert(de);
                auto s = dynamic_pointer_cast<StringValue>(de->key);
                assert(s);
                m[s->v] = de->value;
            }
        }

        std::vector<ValuePtr> elements;

    protected:
        void print(std::ostream& ostr) override
        {
            for (const auto& element : elements)
            {
                ostr << element << endl;
            }
        }

    private:
        TypePtr _type;
    };

    class StructValue;
    using StructValuePtr = std::shared_ptr<StructValue>;

    class StructValue final : public Value
    {
    public:
        StructValue(StructTypePtr t) : _type(std::move(t)) {}

        [[nodiscard]] TypePtr getType() const final { return _type; }

        [[nodiscard]] ValuePtr clone() const final
        {
            auto r = make_shared<StructValue>(_type);
            for (const auto& member : members)
            {
                r->members.push_back(member->clone());
            }
            return r;
        }

        [[nodiscard]] std::string toString() const final
        {
            std::ostringstream out;
            for (auto p = members.begin(); p != members.end(); ++p)
            {
                if (p != members.begin())
                {
                    out << ',';
                }
                out << (*p)->toString();
            }
            return out.str();
        }

        std::vector<ValuePtr> members;

    private:
        void print(std::ostream& ostr) final
        {
            for (const auto& member : members)
            {
                ostr << member << endl;
            }
        }

        StructTypePtr _type;
    };

    //
    // Message encapsulates a DBus message. It only provides the functionality required by the IceBT transport.
    //
    class Message;
    using MessagePtr = std::shared_ptr<Message>;

    class Message
    {
    public:
        [[nodiscard]] virtual bool isError() const = 0;
        [[nodiscard]] virtual std::string getErrorName() const = 0;
        virtual void throwException() = 0;

        [[nodiscard]] virtual bool isSignal() const = 0;
        [[nodiscard]] virtual bool isMethodCall() const = 0;
        [[nodiscard]] virtual bool isMethodReturn() const = 0;

        [[nodiscard]] virtual std::string getPath() const = 0;
        [[nodiscard]] virtual std::string getInterface() const = 0;
        [[nodiscard]] virtual std::string getMember() const = 0;
        [[nodiscard]] virtual std::string getDestination() const = 0;

        //
        // Writing arguments.
        //
        virtual void write(const ValuePtr&) = 0;
        virtual void write(const std::vector<ValuePtr>&) = 0;

        //
        // Reading arguments.
        //
        [[nodiscard]] virtual bool checkTypes(const std::vector<TypePtr>&) const = 0;
        virtual ValuePtr read() = 0;
        virtual std::vector<ValuePtr> readAll() = 0;

        static MessagePtr createCall(const string& dest, const string& path, const string& iface, const string& method);
        static MessagePtr createReturn(const MessagePtr& call);
    };

    class AsyncResult;
    using AsyncResultPtr = std::shared_ptr<AsyncResult>;

    class AsyncCallback
    {
    public:
        virtual void completed(const AsyncResultPtr&) = 0;
    };
    using AsyncCallbackPtr = std::shared_ptr<AsyncCallback>;

    //
    // The result of an asynchronous DBus operation.
    //
    class AsyncResult
    {
    public:
        [[nodiscard]] virtual bool isPending() const = 0;
        [[nodiscard]] virtual bool isComplete() const = 0;

        [[nodiscard]] virtual MessagePtr waitUntilFinished() const = 0;

        [[nodiscard]] virtual MessagePtr getReply() const = 0;

        virtual void setCallback(const AsyncCallbackPtr&) = 0;
    };

    class Connection;
    using ConnectionPtr = std::shared_ptr<Connection>;

    //
    // Allows a subclass to intercept DBus messages.
    //
    class Filter
    {
    public:
        //
        // Return `true` if message is handled or `false` otherwise.
        //
        virtual bool handleMessage(const ConnectionPtr&, const MessagePtr&) = 0;
    };
    using FilterPtr = std::shared_ptr<Filter>;

    //
    // Allows a subclass to receive DBus method invocations.
    //
    class Service
    {
    public:
        virtual void handleMethodCall(const ConnectionPtr&, const MessagePtr&) = 0;
    };
    using ServicePtr = std::shared_ptr<Service>;

    //
    // Encapsulates a DBus connection.
    //
    class Connection
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
        virtual AsyncResultPtr callAsync(const MessagePtr&, const AsyncCallbackPtr& = nullptr) = 0;

        //
        // Sends a message without blocking. Use this to send signals and replies.
        //
        virtual void sendAsync(const MessagePtr&) = 0;

        virtual void close() = 0;
    };

    void initThreads();
}

#endif
