// Copyright (c) ZeroC, Inc.

#ifndef ICEPHP_TYPES_H
#define ICEPHP_TYPES_H

#include "Communicator.h"
#include "Ice/OutputUtil.h"
#include "Operation.h"

//
// Global functions.
//
extern "C"
{
    ZEND_FUNCTION(IcePHP_defineEnum);
    ZEND_FUNCTION(IcePHP_defineStruct);
    ZEND_FUNCTION(IcePHP_defineSequence);
    ZEND_FUNCTION(IcePHP_defineDictionary);
    ZEND_FUNCTION(IcePHP_declareProxy);
    ZEND_FUNCTION(IcePHP_defineProxy);
    ZEND_FUNCTION(IcePHP_declareClass);
    ZEND_FUNCTION(IcePHP_defineClass);
    ZEND_FUNCTION(IcePHP_defineException);
    ZEND_FUNCTION(IcePHP_stringify);
    ZEND_FUNCTION(IcePHP_stringifyException);
}

namespace IcePHP
{
    // This class is raised as an exception when object marshaling needs to be aborted.
    class AbortMarshaling
    {
    };

    class ClassInfo;
    using ClassInfoPtr = std::shared_ptr<ClassInfo>;
    using ClassInfoList = std::vector<ClassInfoPtr>;
    using ObjectMap = std::map<unsigned int, std::shared_ptr<Ice::Value>>;

    class ValueReader;

    struct PrintObjectHistory
    {
        int index;
        std::map<unsigned int, int> objects;
    };

    // The delayed nature of class unmarshaling in the Ice protocol requires us to
    // handle unmarshaling using a callback strategy. An instance of UnmarshalCallback
    // is supplied to each type's unmarshal() member function. For all types except
    // classes, the callback is invoked with the unmarshaled value before unmarshal()
    // returns. For class instances, however, the callback may not be invoked until
    // the stream's finished() function is called.
    class UnmarshalCallback
    {
    public:
        virtual ~UnmarshalCallback();

        // The unmarshaled() member function receives the unmarshaled value. The last two arguments are the values
        // passed to unmarshal() for use by UnmarshalCallback implementations.
        virtual void unmarshaled(zval*, zval*, void*) = 0;
    };
    using UnmarshalCallbackPtr = std::shared_ptr<UnmarshalCallback>;

    // ReadObjectCallback retains all of the information necessary to store an unmarshaled Slice value as a PHP object.
    class ReadObjectCallback final
    {
    public:
        ReadObjectCallback(const ClassInfoPtr&, const UnmarshalCallbackPtr&, zval*, void*);
        ~ReadObjectCallback();

        void invoke(const std::shared_ptr<Ice::Value>&);

    private:
        ClassInfoPtr _info;
        UnmarshalCallbackPtr _cb;
        zval _target;
        void* _closure;
    };
    using ReadObjectCallbackPtr = std::shared_ptr<ReadObjectCallback>;

    // This class keeps track of PHP objects (instances of Slice classes and exceptions) that have preserved slices.
    class StreamUtil
    {
    public:
        ~StreamUtil();

        // Keep a reference to a ReadObjectCallback for patching purposes.
        void add(const ReadObjectCallbackPtr&);

        // Keep track of object instances that have preserved slices.
        void add(const std::shared_ptr<ValueReader>&);

        void updateSlicedData(void);

        static void setSlicedDataMember(zval*, const Ice::SlicedDataPtr&);
        static Ice::SlicedDataPtr getSlicedDataMember(zval*, ObjectMap*);

    private:
        std::vector<ReadObjectCallbackPtr> _callbacks;
        std::set<std::shared_ptr<ValueReader>> _readers;
        static zend_class_entry* _slicedDataType;
        static zend_class_entry* _sliceInfoType;
    };

    // Base class for type information.
    class TypeInfo : public UnmarshalCallback, public std::enable_shared_from_this<TypeInfo>
    {
    public:
        virtual std::string getId() const = 0;

        virtual bool validate(zval*, bool) = 0; // Validate type data. Bool enables excpetion throwing.

        virtual bool variableLength() const = 0;
        virtual int wireSize() const = 0;
        virtual Ice::OptionalFormat optionalFormat() const = 0;

        virtual bool usesClasses() const; // Default implementation returns false.

        virtual void unmarshaled(zval*, zval*, void*); // Default implementation is assert(false).

        virtual void destroy();

    protected:
        TypeInfo();

    public:
        // The marshal and unmarshal functions can raise Ice exceptions, and may raise AbortMarshaling if an error
        // occurs.
        virtual void marshal(zval*, Ice::OutputStream*, ObjectMap*, bool) = 0;
        virtual void
        unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&, zval*, void*, bool) = 0;
        virtual void print(zval*, IceInternal::Output&, PrintObjectHistory*) = 0;
    };
    using TypeInfoPtr = std::shared_ptr<TypeInfo>;

    // Primitive type information.
    class PrimitiveInfo final : public TypeInfo
    {
    public:
        std::string getId() const final;

        bool validate(zval*, bool) final;

        bool variableLength() const final;
        int wireSize() const final;
        Ice::OptionalFormat optionalFormat() const final;

        void marshal(zval*, Ice::OutputStream*, ObjectMap*, bool) final;
        void
        unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&, zval*, void*, bool) final;
        void print(zval*, IceInternal::Output&, PrintObjectHistory*) final;

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

        Kind kind;
    };
    using PrimitiveInfoPtr = std::shared_ptr<PrimitiveInfo>;

    // Enum information.
    class EnumInfo final : public TypeInfo
    {
    public:
        EnumInfo(std::string, zval*);

        std::string getId() const final;

        bool validate(zval*, bool) final;

        bool variableLength() const final;
        int wireSize() const final;
        Ice::OptionalFormat optionalFormat() const final;

        void marshal(zval*, Ice::OutputStream*, ObjectMap*, bool) final;
        void
        unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&, zval*, void*, bool) final;
        void print(zval*, IceInternal::Output&, PrintObjectHistory*) final;

        const std::string id;
        const std::map<std::int32_t, std::string> enumerators;
        const std::int32_t maxValue;
    };
    using EnumInfoPtr = std::shared_ptr<EnumInfo>;

    class DataMember final : public UnmarshalCallback
    {
    public:
        void unmarshaled(zval*, zval*, void*) final;

        void setMember(zval*, zval*);

        std::string name;
        TypeInfoPtr type;
        bool optional;
        int tag;
    };
    using DataMemberPtr = std::shared_ptr<DataMember>;
    using DataMemberList = std::vector<DataMemberPtr>;

    // Struct information.
    class StructInfo final : public TypeInfo
    {
    public:
        StructInfo(std::string, const std::string&, zval*);

        std::string getId() const final;
        bool validate(zval*, bool) final;

        bool variableLength() const final;
        int wireSize() const final;
        Ice::OptionalFormat optionalFormat() const final;

        bool usesClasses() const final;

        void marshal(zval*, Ice::OutputStream*, ObjectMap*, bool) final;
        void
        unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&, zval*, void*, bool) final;

        void print(zval*, IceInternal::Output&, PrintObjectHistory*) final;

        virtual void destroy();

        const std::string id;
        const std::string name; // PHP class name
        const DataMemberList members;
        const zend_class_entry* zce;

    private:
        bool _variableLength;
        int _wireSize;
        zval _nullMarshalValue;
    };
    using StructInfoPtr = std::shared_ptr<StructInfo>;

    // Sequence information.
    class SequenceInfo final : public TypeInfo
    {
    public:
        SequenceInfo(std::string, zval*);

        std::string getId() const final;

        bool validate(zval*, bool) final;

        bool variableLength() const final;
        int wireSize() const final;
        Ice::OptionalFormat optionalFormat() const final;

        bool usesClasses() const final;

        void marshal(zval*, Ice::OutputStream*, ObjectMap*, bool) final;
        void
        unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&, zval*, void*, bool) final;
        void print(zval*, IceInternal::Output&, PrintObjectHistory*) final;
        void unmarshaled(zval*, zval*, void*) final;
        void destroy() final;

        const std::string id;
        const TypeInfoPtr elementType;

    private:
        void marshalPrimitiveSequence(const PrimitiveInfoPtr&, zval*, Ice::OutputStream*);
        void unmarshalPrimitiveSequence(
            const PrimitiveInfoPtr&,
            Ice::InputStream*,
            const UnmarshalCallbackPtr&,
            zval*,
            void*);
    };
    using SequenceInfoPtr = std::shared_ptr<SequenceInfo>;

    // Dictionary information.
    class DictionaryInfo final : public TypeInfo
    {
    public:
        DictionaryInfo(std::string, zval*, zval*);

        std::string getId() const final;

        bool validate(zval*, bool) final;

        bool variableLength() const final;
        int wireSize() const final;
        Ice::OptionalFormat optionalFormat() const final;
        bool usesClasses() const final;

        void marshal(zval*, Ice::OutputStream*, ObjectMap*, bool) final;
        void
        unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&, zval*, void*, bool) final;
        void print(zval*, IceInternal::Output&, PrintObjectHistory*) final;
        void destroy() final;

        class KeyCallback final : public UnmarshalCallback
        {
        public:
            KeyCallback();
            ~KeyCallback();

            void unmarshaled(zval*, zval*, void*) final;

            zval key;
        };
        using KeyCallbackPtr = std::shared_ptr<KeyCallback>;

        class ValueCallback final : public UnmarshalCallback
        {
        public:
            ValueCallback(zval*);
            ~ValueCallback();

            void unmarshaled(zval*, zval*, void*) final;

            zval key;
        };
        using ValueCallbackPtr = std::shared_ptr<ValueCallback>;

        std::string id;
        TypeInfoPtr keyType;
        TypeInfoPtr valueType;

    private:
        bool _variableLength;
        int _wireSize;
    };
    using DictionaryInfoPtr = std::shared_ptr<DictionaryInfo>;

    class ExceptionInfo;
    using ExceptionInfoPtr = std::shared_ptr<ExceptionInfo>;
    using ExceptionInfoList = std::vector<ExceptionInfoPtr>;

    using TypeInfoList = std::vector<TypeInfoPtr>;

    class ClassInfo final : public TypeInfo
    {
    public:
        ClassInfo(std::string);

        void define(const std::string&, std::int32_t, zval*, zval*);

        std::string getId() const final;

        bool validate(zval*, bool) final;

        bool variableLength() const final;
        int wireSize() const final;
        Ice::OptionalFormat optionalFormat() const final;

        bool usesClasses() const final;

        void marshal(zval*, Ice::OutputStream*, ObjectMap*, bool) final;
        void
        unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&, zval*, void*, bool) final;
        void print(zval*, IceInternal::Output&, PrintObjectHistory*) final;
        void destroy() final;
        void printMembers(zval*, IceInternal::Output&, PrintObjectHistory*);

        bool isA(std::string_view) const;

        const std::string id;
        const std::string name; // PHP class name
        const std::int32_t compactId;
        ClassInfoPtr base;
        const DataMemberList members;
        const DataMemberList optionalMembers;
        const zend_class_entry* zce;
        bool defined;
    };

    // Proxy information.
    class ProxyInfo;
    using ProxyInfoPtr = std::shared_ptr<ProxyInfo>;
    using ProxyInfoList = std::vector<ProxyInfoPtr>;

    class ProxyInfo final : public TypeInfo
    {
    public:
        ProxyInfo(std::string);

        void define(zval*, zval*);

        std::string getId() const final;

        bool validate(zval*, bool) final;

        bool variableLength() const final;
        int wireSize() const final;
        Ice::OptionalFormat optionalFormat() const final;

        void marshal(zval*, Ice::OutputStream*, ObjectMap*, bool) final;
        void
        unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&, zval*, void*, bool) final;
        void print(zval*, IceInternal::Output&, PrintObjectHistory*) final;

        void destroy() final;
        bool isA(std::string_view) const;

        void addOperation(const std::string&, const OperationPtr&);
        OperationPtr getOperation(const std::string&) const;

        const std::string id;
        ProxyInfoPtr base;
        ProxyInfoList interfaces;
        bool defined;
        typedef std::map<std::string, OperationPtr> OperationMap;
        OperationMap operations;
    };

    // Exception information.
    class ExceptionInfo final : public std::enable_shared_from_this<ExceptionInfo>
    {
    public:
        void unmarshal(Ice::InputStream*, const CommunicatorInfoPtr&, zval*);

        void print(zval*, IceInternal::Output&);
        void printMembers(zval*, IceInternal::Output&, PrintObjectHistory*);

        bool isA(std::string_view) const;

        std::string id;
        std::string name; // PHP class name
        ExceptionInfoPtr base;
        DataMemberList members;
        DataMemberList optionalMembers;
        bool usesClasses;
        zend_class_entry* zce;
    };

    ClassInfoPtr getClassInfoById(std::string_view);
    ClassInfoPtr getClassInfoByName(const std::string&);
    ProxyInfoPtr getProxyInfo(std::string_view);
    ExceptionInfoPtr getExceptionInfo(std::string_view);

    bool isUnset(zval*);
    void assignUnset(zval*);

    bool typesInit(INIT_FUNC_ARGS);
    bool typesRequestInit(void);
    bool typesRequestShutdown(void);

    // ValueWriter wraps a PHP object for marshaling.
    class ValueWriter final : public Ice::Value
    {
    public:
        ValueWriter(zval*, ObjectMap*, ClassInfoPtr);
        ~ValueWriter();

        void ice_preMarshal() final;

        void _iceWrite(Ice::OutputStream*) const final;
        void _iceRead(Ice::InputStream*) final;

    private:
        void writeMembers(Ice::OutputStream*, const DataMemberList&) const;

        zval _object;
        ObjectMap* _map;
        ClassInfoPtr _info;
        ClassInfoPtr _formal;
    };

    // ValueReader unmarshals the state of an Ice value.
    class ValueReader final : public std::enable_shared_from_this<ValueReader>, public Ice::Value
    {
    public:
        ValueReader(zval*, const ClassInfoPtr&, const CommunicatorInfoPtr&);
        ~ValueReader();

        void ice_postUnmarshal() final;

        void _iceWrite(Ice::OutputStream*) const final;
        void _iceRead(Ice::InputStream*) final;

        ClassInfoPtr getInfo() const;

        zval* getObject() const;

        Ice::SlicedDataPtr getSlicedData() const;

    private:
        zval _object;
        ClassInfoPtr _info;
        CommunicatorInfoPtr _communicator;
        Ice::SlicedDataPtr _slicedData;
    };

    // ExceptionReader creates a PHP user exception and unmarshals it.
    class ExceptionReader final : public Ice::UserException
    {
    public:
        ExceptionReader(const CommunicatorInfoPtr&, const ExceptionInfoPtr&);
        ~ExceptionReader();

        ExceptionReader(const ExceptionReader&) = default;

        const char* ice_id() const noexcept final;
        void ice_throw() const final;

        void _write(Ice::OutputStream*) const final;
        void _read(Ice::InputStream*) final;
        bool _usesClasses() const final;

        ExceptionInfoPtr getInfo() const;

        zval* getException() const;

    protected:
        void _writeImpl(Ice::OutputStream*) const final {}
        void _readImpl(Ice::InputStream*) final {}

    private:
        CommunicatorInfoPtr _communicatorInfo;
        ExceptionInfoPtr _info;
        zval _ex;
    };

} // End of namespace IcePHP

#endif
