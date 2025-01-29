// Copyright (c) ZeroC, Inc.

#ifndef ICE_RUBY_TYPES_H
#define ICE_RUBY_TYPES_H

#include "Config.h"
#include "Util.h"

#include "Ice/FactoryTable.h"
#include "Ice/Object.h"
#include "Ice/OutputUtil.h"
#include "Ice/SlicedData.h"

namespace IceRuby
{
    std::string resolveCompactId(int id);

    class ExceptionInfo;
    using ExceptionInfoPtr = std::shared_ptr<ExceptionInfo>;
    using ExceptionInfoList = std::vector<ExceptionInfoPtr>;

    class ClassInfo;
    using ClassInfoPtr = std::shared_ptr<ClassInfo>;

    class ProxyInfo;
    using ProxyInfoPtr = std::shared_ptr<ProxyInfo>;
    using ProxyInfoList = std::vector<ProxyInfoPtr>;

    //
    // This class is raised as an exception when object marshaling needs to be aborted.
    //
    // TODO: Need an equivalent Ruby exception.
    //
    class AbortMarshaling
    {
    };

    using ValueMap = std::map<VALUE, std::shared_ptr<Ice::Value>>;

    class ValueReader;

    struct PrintObjectHistory
    {
        int index;
        std::map<VALUE, int> objects;
    };

    //
    // The delayed nature of class unmarshaling in the Ice protocol requires us to
    // handle unmarshaling using a callback strategy. An instance of UnmarshalCallback
    // is supplied to each type's unmarshal() member function. For all types except
    // classes, the callback is invoked with the unmarshaled value before unmarshal()
    // returns. For class instances, however, the callback may not be invoked until
    // the stream's finished() function is called.
    //
    class UnmarshalCallback
    {
    public:
        virtual ~UnmarshalCallback();

        //
        // The unmarshaled() member function receives the unmarshaled value. The
        // last two arguments are the values passed to unmarshal() for use by
        // UnmarshalCallback implementations.
        //
        virtual void unmarshaled(VALUE, VALUE, void*) = 0;
    };
    using UnmarshalCallbackPtr = std::shared_ptr<UnmarshalCallback>;

    //
    // ReadValueCallback retains all of the information necessary to store an unmarshaled
    // Slice value as a Ruby object.
    //
    class ReadValueCallback final
    {
    public:
        ReadValueCallback(const ClassInfoPtr&, const UnmarshalCallbackPtr&, VALUE, void*);

        void invoke(const std::shared_ptr<Ice::Value>&);

    private:
        ClassInfoPtr _info;
        UnmarshalCallbackPtr _cb;
        VALUE _target;
        void* _closure;
    };
    using ReadValueCallbackPtr = std::shared_ptr<ReadValueCallback>;

    //
    // This class assists during unmarshaling of Slice classes and exceptions.
    // We attach an instance to a stream.
    //
    class StreamUtil
    {
    public:
        StreamUtil();
        ~StreamUtil();

        //
        // Keep a reference to a ReadValueCallback for patching purposes.
        //
        void add(const ReadValueCallbackPtr&);

        //
        // Keep track of object instances that have preserved slices.
        //
        void add(const std::shared_ptr<ValueReader>&);

        //
        // Updated the sliced data information for all stored object instances.
        //
        void updateSlicedData();

        static void setSlicedDataMember(VALUE, const Ice::SlicedDataPtr&);
        static Ice::SlicedDataPtr getSlicedDataMember(VALUE, ValueMap*);

    private:
        std::vector<ReadValueCallbackPtr> _callbacks;
        std::set<std::shared_ptr<ValueReader>> _readers;
        static VALUE _slicedDataType;
        static VALUE _sliceInfoType;
    };

    //
    // Base class for type information.
    //
    class TypeInfo : public UnmarshalCallback
    {
    public:
        virtual std::string getId() const = 0;

        virtual bool validate(VALUE) = 0;

        virtual bool variableLength() const = 0;
        virtual int wireSize() const = 0;
        virtual Ice::OptionalFormat optionalFormat() const = 0;

        virtual bool usesClasses() const; // Default implementation returns false.

        void unmarshaled(VALUE, VALUE, void*) override; // Default implementation is assert(false).

        virtual void destroy();

    protected:
        TypeInfo();

    public:
        // The marshal and unmarshal functions can raise Ice exceptions, and may raise AbortMarshaling if an error
        // occurs.
        virtual void marshal(VALUE, Ice::OutputStream*, ValueMap*, bool) = 0;
        virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, VALUE, void*, bool) = 0;

        virtual void print(VALUE, IceInternal::Output&, PrintObjectHistory*) = 0;
    };
    using TypeInfoPtr = std::shared_ptr<TypeInfo>;

    // Primitive type information.
    class PrimitiveInfo final : public TypeInfo
    {
    public:
        enum Kind
        {
            KindBool,
            KindByte,
            KindShort,
            KindInt,
            KindLong,
            KindFloat,
            KindDouble,
            KindString
        };

        PrimitiveInfo();
        PrimitiveInfo(Kind);

        std::string getId() const final;

        bool validate(VALUE) final;

        bool variableLength() const;
        int wireSize() const final;
        Ice::OptionalFormat optionalFormat() const final;

        void marshal(VALUE, Ice::OutputStream*, ValueMap*, bool) final;
        void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, VALUE, void*, bool) final;

        void print(VALUE, IceInternal::Output&, PrintObjectHistory*) final;

        static double toDouble(VALUE);

        Kind kind;
    };
    using PrimitiveInfoPtr = std::shared_ptr<PrimitiveInfo>;

    // Enum information.
    using EnumeratorMap = std::map<std::int32_t, VALUE>;

    class EnumInfo final : public TypeInfo
    {
    public:
        EnumInfo(VALUE, VALUE, VALUE);

        std::string getId() const final;

        bool validate(VALUE) final;

        bool variableLength() const final;
        int wireSize() const final;
        Ice::OptionalFormat optionalFormat() const final;

        void marshal(VALUE, Ice::OutputStream*, ValueMap*, bool) final;
        void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, VALUE, void*, bool) final;

        void print(VALUE, IceInternal::Output&, PrintObjectHistory*) final;

        const std::string id;
        const VALUE rubyClass;
        const std::int32_t maxValue;
        const EnumeratorMap enumerators;
    };
    using EnumInfoPtr = std::shared_ptr<EnumInfo>;

    class DataMember final : public UnmarshalCallback
    {
    public:
        void unmarshaled(VALUE, VALUE, void*) final;

        std::string name;
        TypeInfoPtr type;
        ID rubyID;
        bool optional;
        int tag;
    };
    using DataMemberPtr = std::shared_ptr<DataMember>;
    using DataMemberList = std::vector<DataMemberPtr>;

    // Struct information.
    class StructInfo final : public TypeInfo
    {
    public:
        StructInfo(VALUE, VALUE, VALUE);

        std::string getId() const final;

        bool validate(VALUE) final;

        bool variableLength() const final;
        int wireSize() const final;
        Ice::OptionalFormat optionalFormat() const final;

        bool usesClasses() const final; // Default implementation returns false.

        void marshal(VALUE, Ice::OutputStream*, ValueMap*, bool) final;
        void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, VALUE, void*, bool) final;

        void print(VALUE, IceInternal::Output&, PrintObjectHistory*) final;

        void destroy() final;

        const std::string id;
        const DataMemberList members;
        const VALUE rubyClass;

    private:
        bool _variableLength;
        int _wireSize;
    };
    using StructInfoPtr = std::shared_ptr<StructInfo>;

    // Sequence information.
    class SequenceInfo final : public TypeInfo, public std::enable_shared_from_this<SequenceInfo>
    {
    public:
        SequenceInfo(VALUE, VALUE);

        std::string getId() const final;

        bool validate(VALUE) final;

        bool variableLength() const final;
        int wireSize() const final;
        Ice::OptionalFormat optionalFormat() const final;

        bool usesClasses() const final; // Default implementation returns false.

        void marshal(VALUE, Ice::OutputStream*, ValueMap*, bool) final;
        void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, VALUE, void*, bool) final;
        void unmarshaled(VALUE, VALUE, void*) final;

        void print(VALUE, IceInternal::Output&, PrintObjectHistory*) final;

        void destroy() final;

        const std::string id;
        const TypeInfoPtr elementType;

    private:
        void marshalPrimitiveSequence(const PrimitiveInfoPtr&, VALUE, Ice::OutputStream*);
        void unmarshalPrimitiveSequence(
            const PrimitiveInfoPtr&,
            Ice::InputStream*,
            const UnmarshalCallbackPtr&,
            VALUE,
            void*);
    };
    using SequenceInfoPtr = std::shared_ptr<SequenceInfo>;

    // Dictionary information.
    class DictionaryInfo final : public TypeInfo, public std::enable_shared_from_this<DictionaryInfo>
    {
    public:
        DictionaryInfo(VALUE, VALUE, VALUE);

        std::string getId() const final;

        bool validate(VALUE) final;

        bool variableLength() const final;
        int wireSize() const final;
        Ice::OptionalFormat optionalFormat() const final;

        bool usesClasses() const final; // Default implementation returns false.

        void marshal(VALUE, Ice::OutputStream*, ValueMap*, bool) final;
        void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, VALUE, void*, bool) final;
        void marshalElement(VALUE, VALUE, Ice::OutputStream*, ValueMap*);
        void unmarshaled(VALUE, VALUE, void*) final;

        void print(VALUE, IceInternal::Output&, PrintObjectHistory*) final;
        void printElement(VALUE, VALUE, IceInternal::Output&, PrintObjectHistory*);

        void destroy() final;

        class KeyCallback final : public UnmarshalCallback
        {
        public:
            void unmarshaled(VALUE, VALUE, void*) final;

            VALUE key;
        };
        using KeyCallbackPtr = std::shared_ptr<KeyCallback>;

        const std::string id;
        const TypeInfoPtr keyType;
        const TypeInfoPtr valueType;

    private:
        bool _variableLength;
        int _wireSize;
    };
    using DictionaryInfoPtr = std::shared_ptr<DictionaryInfo>;
    using TypeInfoList = std::vector<TypeInfoPtr>;

    class ClassInfo final : public TypeInfo, public std::enable_shared_from_this<ClassInfo>
    {
    public:
        static ClassInfoPtr create(VALUE);

        void define(VALUE, VALUE, VALUE, VALUE, VALUE);

        std::string getId() const final;

        bool validate(VALUE) final;

        bool variableLength() const final;
        int wireSize() const final;
        Ice::OptionalFormat optionalFormat() const final;

        bool usesClasses() const final; // Default implementation returns false.

        void marshal(VALUE, Ice::OutputStream*, ValueMap*, bool) final;
        void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, VALUE, void*, bool) final;

        void print(VALUE, IceInternal::Output&, PrintObjectHistory*) final;

        void destroy() final;

        void printMembers(VALUE, IceInternal::Output&, PrintObjectHistory*);

        bool isA(const ClassInfoPtr&);

        const std::string id;
        const std::int32_t compactId;
        const bool isBase; // Is this the ClassInfo for Value?
        const bool interface;
        const ClassInfoPtr base;
        const DataMemberList members;
        const DataMemberList optionalMembers;
        const VALUE rubyClass;
        const VALUE typeObj;
        const bool defined;

    private:
        ClassInfo(VALUE);
    };

    // Proxy information.
    class ProxyInfo final : public TypeInfo
    {
    public:
        static ProxyInfoPtr create(VALUE);

        void define(VALUE, VALUE, VALUE);

        std::string getId() const final;

        bool validate(VALUE) final;

        bool variableLength() const final;
        int wireSize() const final;
        Ice::OptionalFormat optionalFormat() const final;

        void marshal(VALUE, Ice::OutputStream*, ValueMap*, bool) final;
        void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, VALUE, void*, bool) final;

        void print(VALUE, IceInternal::Output&, PrintObjectHistory*) final;

        void destroy() final;

        bool isA(const ProxyInfoPtr&);

        const std::string id;
        const bool isBase; // Is this the ClassInfo for Ice::ObjectPrx?
        const ProxyInfoPtr base;
        const ProxyInfoList interfaces;
        const VALUE rubyClass;
        const VALUE typeObj;

    private:
        ProxyInfo(VALUE);
    };

    // Exception information.
    class ExceptionInfo final : public std::enable_shared_from_this<ExceptionInfo>
    {
    public:
        VALUE unmarshal(Ice::InputStream*);

        void print(VALUE, IceInternal::Output&);
        void printMembers(VALUE, IceInternal::Output&, PrintObjectHistory*);

        std::string id;
        ExceptionInfoPtr base;
        DataMemberList members;
        DataMemberList optionalMembers;
        bool usesClasses;
        VALUE rubyClass;
    };

    // ValueWriter wraps a Ruby object for marshaling.
    class ValueWriter final : public Ice::Value
    {
    public:
        ValueWriter(VALUE, ValueMap*, const ClassInfoPtr&);
        ~ValueWriter();

        void ice_preMarshal() final;

        void _iceWrite(Ice::OutputStream*) const final;
        void _iceRead(Ice::InputStream*) final;

    private:
        void writeMembers(Ice::OutputStream*, const DataMemberList&) const;

        VALUE _object;
        ValueMap* _map;
        ClassInfoPtr _info;
        ClassInfoPtr _formal;
    };

    //
    // ValueReader unmarshals the state of an Ice object.
    //
    class ValueReader final : public std::enable_shared_from_this<ValueReader>, public Ice::Value
    {
    public:
        ValueReader(VALUE, const ClassInfoPtr&);
        ~ValueReader();

        void ice_postUnmarshal() final;

        void _iceWrite(Ice::OutputStream*) const final;
        void _iceRead(Ice::InputStream*) final;

        ClassInfoPtr getInfo() const;

        VALUE getObject() const; // Borrowed reference.

        Ice::SlicedDataPtr getSlicedData() const;

    private:
        VALUE _object;
        ClassInfoPtr _info;
        Ice::SlicedDataPtr _slicedData;
    };

    // ExceptionReader creates a Ruby user exception and unmarshals it.
    class ExceptionReader final : public Ice::UserException
    {
    public:
        ExceptionReader(const ExceptionInfoPtr&);
        ExceptionReader(const ExceptionReader&);
        ~ExceptionReader();

        const char* ice_id() const noexcept final;
        void ice_throw() const final;

        void _write(Ice::OutputStream*) const final;
        void _read(Ice::InputStream*) final;

        bool _usesClasses() const final;

        VALUE getException() const;

    protected:
        void _writeImpl(Ice::OutputStream*) const final {}
        void _readImpl(Ice::InputStream*) final {}

    private:
        ExceptionInfoPtr _info;
        VALUE _ex;
    };

    ClassInfoPtr lookupClassInfo(std::string_view);
    ExceptionInfoPtr lookupExceptionInfo(std::string_view);

    bool initTypes(VALUE);

    VALUE createType(const TypeInfoPtr&);
    TypeInfoPtr getType(VALUE);

    VALUE createException(const ExceptionInfoPtr&);
    ExceptionInfoPtr getException(VALUE);
}

#endif
