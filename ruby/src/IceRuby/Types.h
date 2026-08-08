// Copyright (c) ZeroC, Inc.

#ifndef ICE_RUBY_TYPES_H
#define ICE_RUBY_TYPES_H

#include "Config.h"
#include "Util.h"

#include "Ice/Object.h"
#include "Ice/OutputUtil.h"
#include "Ice/SlicedData.h"

namespace IceRuby
{
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
    class StreamUtil;

    // Tracks the class instances already printed during a stringification, to detect shared instances and cycles.
    // The instances are keyed in a Ruby identity hash (instance => index), whose keys remain valid when GC
    // compaction runs during the stringification.
    struct PrintObjectHistory
    {
        PrintObjectHistory();

        int index{0};  // the index to assign to the next class instance printed for the first time
        VALUE objects; // Ruby identity hash
    };

    //
    // Slice unmarshaling uses a callback strategy: an instance of UnmarshalCallback is
    // supplied to each type's unmarshal() member function. For all types except classes,
    // the callback is invoked with the unmarshaled value before unmarshal() returns. For
    // class instances, the callback may not be invoked until later, because the Slice 1.0
    // encoding writes instances after the data that references them. The 1.1 encoding is
    // much more linear, but this extension unmarshals both encodings with the same
    // callback logic.
    //
    class UnmarshalCallback
    {
    public:
        virtual ~UnmarshalCallback();

        //
        // The unmarshaled() member function receives the unmarshaled value. The
        // next two arguments are the values passed to unmarshal() for use by
        // UnmarshalCallback implementations, and the last argument is the
        // StreamUtil attached to the stream.
        //
        virtual void unmarshaled(VALUE, VALUE, void*, StreamUtil*) = 0;
    };
    using UnmarshalCallbackPtr = std::shared_ptr<UnmarshalCallback>;

    //
    // ReadValueCallback retains all of the information necessary to store an unmarshaled
    // Slice value as a Ruby object. Its only owner is the StreamUtil attached to the
    // stream (the stream's patch entries hold non-owning pointers), so it never outlives
    // the StreamUtil and _util remains valid for its whole lifetime.
    //
    class ReadValueCallback final
    {
    public:
        ReadValueCallback(const ClassInfoPtr&, const UnmarshalCallbackPtr&, VALUE, void*, StreamUtil*);

        void invoke(const std::shared_ptr<Ice::Value>&);

    private:
        ClassInfoPtr _info;
        UnmarshalCallbackPtr _cb;
        StreamUtil* _util;
        long _targetIndex;
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

        // Registers a GC root tied to the address of _held, so it's neither copyable nor movable.
        StreamUtil(const StreamUtil&) = delete;
        StreamUtil(StreamUtil&&) = delete;
        StreamUtil& operator=(const StreamUtil&) = delete;
        StreamUtil& operator=(StreamUtil&&) = delete;

        //
        // Keep a reference to a ReadValueCallback for patching purposes.
        //
        void add(const ReadValueCallbackPtr&);

        //
        // Keep track of object instances that have preserved slices.
        //
        void add(const std::shared_ptr<ValueReader>&);

        //
        // Keep a Ruby object reachable for the lifetime of the stream and return an index
        // that recovers it via getHeldValue. The objects are held in a Ruby array whose
        // elements the GC updates when compaction moves them, so getHeldValue remains
        // valid where a raw VALUE copy in native code would go stale.
        //
        long hold(VALUE);
        VALUE getHeldValue(long index) const;

        //
        // Updated the sliced data information for all stored object instances.
        //
        void updateSlicedData();

        static void setSlicedDataMember(VALUE, const Ice::SlicedDataPtr&);
        static Ice::SlicedDataPtr getSlicedDataMember(VALUE, ValueMap*);

    private:
        std::vector<ReadValueCallbackPtr> _callbacks;
        std::set<std::shared_ptr<ValueReader>> _readers;
        VALUE _held; // Ruby array created on first use by hold()
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

        void unmarshaled(VALUE, VALUE, void*, StreamUtil*) override; // Default implementation is assert(false).

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
        void unmarshaled(VALUE, VALUE, void*, StreamUtil*) final;

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
        void unmarshaled(VALUE, VALUE, void*, StreamUtil*) final;

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
        void unmarshaled(VALUE, VALUE, void*, StreamUtil*) final;

        void print(VALUE, IceInternal::Output&, PrintObjectHistory*) final;
        void printElement(VALUE, VALUE, IceInternal::Output&, PrintObjectHistory*);

        void destroy() final;

        class KeyCallback final : public UnmarshalCallback
        {
        public:
            void unmarshaled(VALUE, VALUE, void*, StreamUtil*) final;

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

        const std::string id;
        const ProxyInfoPtr base;
        const ProxyInfoList interfaces; // TODO this field is also dead but affects public API.
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

        // Registers a GC root tied to the address of _object, so it's neither copyable nor movable.
        ValueWriter(const ValueWriter&) = delete;
        ValueWriter(ValueWriter&&) = delete;
        ValueWriter& operator=(const ValueWriter&) = delete;
        ValueWriter& operator=(ValueWriter&&) = delete;

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

        // Registers a GC root tied to the address of _object, so it's neither copyable nor movable.
        ValueReader(const ValueReader&) = delete;
        ValueReader(ValueReader&&) = delete;
        ValueReader& operator=(const ValueReader&) = delete;
        ValueReader& operator=(ValueReader&&) = delete;

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
        // Each constructor registers a GC root tied to the address of the new instance's _ex member, and the
        // destructor unregisters it. The copy constructor must remain available: the exception machinery copies the
        // reader into the exception storage. The other special members are deleted so that none can transfer or
        // assign _ex without the matching registration.
        ExceptionReader(const ExceptionInfoPtr&);
        ExceptionReader(const ExceptionReader&);
        ~ExceptionReader();

        ExceptionReader(ExceptionReader&&) = delete;
        ExceptionReader& operator=(const ExceptionReader&) = delete;
        ExceptionReader& operator=(ExceptionReader&&) = delete;

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
