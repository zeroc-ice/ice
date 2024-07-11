//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEPY_TYPES_H
#define ICEPY_TYPES_H

#include "Config.h"
#include "Ice/FactoryTable.h"
#include "Ice/InputStream.h"
#include "Ice/OutputStream.h"
#include "Ice/OutputUtil.h"
#include "Ice/SlicedDataF.h"
#include "Ice/Value.h"
#include "Util.h"

#include <memory>
#include <set>

namespace IcePy
{
    class Buffer;
    using BufferPtr = std::shared_ptr<Buffer>;

    class ExceptionInfo;
    using ExceptionInfoPtr = std::shared_ptr<ExceptionInfo>;
    using ExceptionInfoList = std::vector<ExceptionInfoPtr>;

    class ClassInfo;
    using ClassInfoPtr = std::shared_ptr<ClassInfo>;
    using ClassInfoList = std::vector<ClassInfoPtr>;

    class ValueInfo;
    using ValueInfoPtr = std::shared_ptr<ValueInfo>;

    //
    // This class is raised as an exception when object marshaling needs to be aborted.
    //
    class AbortMarshaling
    {
    };

    using ObjectMap = std::map<PyObject*, std::shared_ptr<Ice::Value>>;

    class ValueReader;
    using ValueReaderPtr = std::shared_ptr<ValueReader>;

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
        virtual void unmarshaled(PyObject*, PyObject*, void*) = 0;
    };
    using UnmarshalCallbackPtr = std::shared_ptr<UnmarshalCallback>;

    //
    // ReadValueCallback retains all of the information necessary to store an unmarshaled
    // Slice value as a Python object.
    //
    class ReadValueCallback
    {
    public:
        ReadValueCallback(const ValueInfoPtr&, const UnmarshalCallbackPtr&, PyObject*, void*);
        ~ReadValueCallback();

        void invoke(const ::std::shared_ptr<Ice::Value>&);

    private:
        ValueInfoPtr _info;
        UnmarshalCallbackPtr _cb;
        PyObject* _target;
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
        void add(const ValueReaderPtr&);

        //
        // Updated the sliced data information for all stored object instances.
        //
        void updateSlicedData();

        static void setSlicedDataMember(PyObject*, const Ice::SlicedDataPtr&);
        static Ice::SlicedDataPtr getSlicedDataMember(PyObject*, ObjectMap*);

    private:
        std::vector<ReadValueCallbackPtr> _callbacks;
        std::set<ValueReaderPtr> _readers;
        static PyObject* _slicedDataType;
        static PyObject* _sliceInfoType;
    };

    struct PrintObjectHistory
    {
        int index;
        std::map<PyObject*, int> objects;
    };

    //
    // Base class for type information.
    //
    class TypeInfo : public UnmarshalCallback
    {
    public:
        TypeInfo();
        virtual std::string getId() const = 0;

        virtual bool validate(PyObject*) = 0;

        virtual bool variableLength() const = 0;
        virtual int wireSize() const = 0;
        virtual Ice::OptionalFormat optionalFormat() const = 0;

        virtual bool usesClasses() const; // Default implementation returns false.

        virtual void unmarshaled(PyObject*, PyObject*, void*); // Default implementation is assert(false).

        virtual void destroy();
        //
        // The marshal and unmarshal functions can raise Ice exceptions, and may raise
        // AbortMarshaling if an error occurs.
        //
        virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0) = 0;
        virtual void unmarshal(
            Ice::InputStream*,
            const UnmarshalCallbackPtr&,
            PyObject*,
            void*,
            bool,
            const Ice::StringSeq* = 0) = 0;

        virtual void print(PyObject*, IceInternal::Output&, PrintObjectHistory*) = 0;
    };
    using TypeInfoPtr = std::shared_ptr<TypeInfo>;

    //
    // Primitive type information.
    //
    class PrimitiveInfo : public TypeInfo
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

        virtual std::string getId() const;

        virtual bool validate(PyObject*);

        virtual bool variableLength() const;
        virtual int wireSize() const;
        virtual Ice::OptionalFormat optionalFormat() const;

        virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0);
        virtual void
        unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool, const Ice::StringSeq* = 0);

        virtual void print(PyObject*, IceInternal::Output&, PrintObjectHistory*);

        const Kind kind;
    };
    using PrimitiveInfoPtr = std::shared_ptr<PrimitiveInfo>;

    //
    // Enum information.
    //
    using EnumeratorMap = std::map<std::int32_t, PyObjectHandle>;

    class EnumInfo final : public TypeInfo
    {
    public:
        EnumInfo(std::string, PyObject*, PyObject*);

        std::string getId() const final;

        virtual bool validate(PyObject*);

        virtual bool variableLength() const;
        virtual int wireSize() const;
        virtual Ice::OptionalFormat optionalFormat() const;

        virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0);
        virtual void
        unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool, const Ice::StringSeq* = 0);

        virtual void print(PyObject*, IceInternal::Output&, PrintObjectHistory*);

        virtual void destroy();

        std::int32_t valueForEnumerator(PyObject*) const;
        PyObject* enumeratorForValue(std::int32_t) const;

        const std::string id;
        PyObject* pythonType; // Borrowed reference - the enclosing Python module owns the reference.
        const std::int32_t maxValue;
        const EnumeratorMap enumerators;
    };
    using EnumInfoPtr = std::shared_ptr<EnumInfo>;

    class DataMember : public UnmarshalCallback
    {
    public:
        virtual void unmarshaled(PyObject*, PyObject*, void*);

        std::string name;
        std::vector<std::string> metaData;
        TypeInfoPtr type;
        bool optional;
        int tag;
    };
    using DataMemberPtr = std::shared_ptr<DataMember>;
    using DataMemberList = std::vector<DataMemberPtr>;

    //
    // Struct information.
    //
    class StructInfo : public TypeInfo
    {
    public:
        StructInfo(std::string, PyObject*, PyObject*);

        virtual std::string getId() const;

        virtual bool validate(PyObject*);

        virtual bool variableLength() const;
        virtual int wireSize() const;
        virtual Ice::OptionalFormat optionalFormat() const;

        virtual bool usesClasses() const;

        virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0);
        virtual void
        unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool, const Ice::StringSeq* = 0);

        virtual void print(PyObject*, IceInternal::Output&, PrintObjectHistory*);

        virtual void destroy();

        static PyObject* instantiate(PyObject*);

        const std::string id;
        const DataMemberList members;
        PyObject* pythonType; // Borrowed reference - the enclosing Python module owns the reference.

    private:
        bool _variableLength;
        int _wireSize;
        PyObjectHandle _nullMarshalValue;
    };
    using StructInfoPtr = std::shared_ptr<StructInfo>;

    //
    // Sequence information.
    //
    class SequenceInfo : public TypeInfo
    {
    public:
        SequenceInfo(std::string, PyObject*, PyObject*);

        virtual std::string getId() const;

        virtual bool validate(PyObject*);

        virtual bool variableLength() const;
        virtual int wireSize() const;
        virtual Ice::OptionalFormat optionalFormat() const;

        virtual bool usesClasses() const;

        virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0);
        virtual void
        unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool, const Ice::StringSeq* = 0);

        virtual void print(PyObject*, IceInternal::Output&, PrintObjectHistory*);

        virtual void destroy();

        enum BuiltinType
        {
            BuiltinTypeBool = 0,
            BuiltinTypeByte = 1,
            BuiltinTypeShort = 2,
            BuiltinTypeInt = 3,
            BuiltinTypeLong = 4,
            BuiltinTypeFloat = 5,
            BuiltinTypeDouble = 6
        };

    private:
        struct SequenceMapping : public UnmarshalCallback
        {
            enum Type
            {
                SEQ_DEFAULT,
                SEQ_TUPLE,
                SEQ_LIST,
                SEQ_ARRAY,
                SEQ_NUMPYARRAY,
                SEQ_MEMORYVIEW
            };

            SequenceMapping(Type);
            SequenceMapping(const Ice::StringSeq&);

            void init(const Ice::StringSeq&);

            static bool getType(const Ice::StringSeq&, Type&);

            virtual void unmarshaled(PyObject*, PyObject*, void*);

            PyObject* createContainer(int) const;
            void setItem(PyObject*, int, PyObject*) const;

            Type type;
            PyObject* factory;
        };
        using SequenceMappingPtr = std::shared_ptr<SequenceMapping>;

        PyObject* getSequence(const PrimitiveInfoPtr&, PyObject*);
        void marshalPrimitiveSequence(const PrimitiveInfoPtr&, PyObject*, Ice::OutputStream*);
        void unmarshalPrimitiveSequence(
            const PrimitiveInfoPtr&,
            Ice::InputStream*,
            const UnmarshalCallbackPtr&,
            PyObject*,
            void*,
            const SequenceMappingPtr&);

        PyObject* createSequenceFromMemory(const SequenceMappingPtr&, const char*, Py_ssize_t, BuiltinType);

    public:
        const std::string id;
        const SequenceMappingPtr mapping;
        const TypeInfoPtr elementType;
    };
    using SequenceInfoPtr = std::shared_ptr<SequenceInfo>;

    //
    // Custom information.
    //
    class CustomInfo : public TypeInfo
    {
    public:
        CustomInfo(std::string, PyObject*);

        virtual std::string getId() const;

        virtual bool validate(PyObject*);

        virtual bool variableLength() const;
        virtual int wireSize() const;
        virtual Ice::OptionalFormat optionalFormat() const;

        virtual bool usesClasses() const;

        virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0);
        virtual void
        unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool, const Ice::StringSeq* = 0);

        virtual void print(PyObject*, IceInternal::Output&, PrintObjectHistory*);

        const std::string id;
        PyObject* pythonType; // Borrowed reference - the enclosing Python module owns the reference.
    };
    using CustomInfoPtr = std::shared_ptr<CustomInfo>;

    //
    // Dictionary information.
    //
    class DictionaryInfo : public TypeInfo, public std::enable_shared_from_this<DictionaryInfo>
    {
    public:
        DictionaryInfo(std::string, PyObject*, PyObject*);

        virtual std::string getId() const;

        virtual bool validate(PyObject*);

        virtual bool variableLength() const;
        virtual int wireSize() const;
        virtual Ice::OptionalFormat optionalFormat() const;

        virtual bool usesClasses() const;

        virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0);
        virtual void
        unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool, const Ice::StringSeq* = 0);
        virtual void unmarshaled(PyObject*, PyObject*, void*);

        virtual void print(PyObject*, IceInternal::Output&, PrintObjectHistory*);

        virtual void destroy();

        class KeyCallback : public UnmarshalCallback
        {
        public:
            virtual void unmarshaled(PyObject*, PyObject*, void*);

            PyObjectHandle key;
        };
        using KeyCallbackPtr = std::shared_ptr<KeyCallback>;

        std::string id;
        TypeInfoPtr keyType;
        TypeInfoPtr valueType;

    private:
        bool _variableLength;
        int _wireSize;
    };
    using DictionaryInfoPtr = std::shared_ptr<DictionaryInfo>;
    using TypeInfoList = std::vector<TypeInfoPtr>;

    class ClassInfo final : public TypeInfo, public std::enable_shared_from_this<ClassInfo>
    {
    public:
        ClassInfo(std::string);
        void init();

        void define(PyObject*, PyObject*, PyObject*);

        virtual std::string getId() const;

        virtual bool validate(PyObject*);

        virtual bool variableLength() const;
        virtual int wireSize() const;
        virtual Ice::OptionalFormat optionalFormat() const;

        virtual bool usesClasses() const;

        virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0);
        virtual void
        unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool, const Ice::StringSeq* = 0);

        virtual void print(PyObject*, IceInternal::Output&, PrintObjectHistory*);

        virtual void destroy();

        const std::string id;
        const ClassInfoPtr base;
        const ClassInfoList interfaces;
        PyObject* pythonType; // Borrowed reference - the enclosing Python module owns the reference.
        PyObject* typeObj;    // Borrowed reference - the "_t_XXX" variable owns the reference.
        const bool defined;
    };

    //
    // Value type information
    //

    class ValueInfo final : public TypeInfo, public std::enable_shared_from_this<ValueInfo>
    {
    public:
        ValueInfo(std::string);
        void init();

        void define(PyObject*, int, bool, PyObject*, PyObject*);

        virtual std::string getId() const;

        virtual bool validate(PyObject*);

        virtual bool variableLength() const;
        virtual int wireSize() const;
        virtual Ice::OptionalFormat optionalFormat() const;

        virtual bool usesClasses() const;

        virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0);
        virtual void
        unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool, const Ice::StringSeq* = 0);

        virtual void print(PyObject*, IceInternal::Output&, PrintObjectHistory*);

        virtual void destroy();

        void printMembers(PyObject*, IceInternal::Output&, PrintObjectHistory*);

        const std::string id;
        const std::int32_t compactId;
        const bool interface;
        const ValueInfoPtr base;
        const DataMemberList members;
        const DataMemberList optionalMembers;
        PyObject* pythonType; // Borrowed reference - the enclosing Python module owns the reference.
        PyObject* typeObj;    // Borrowed reference - the "_t_XXX" variable owns the reference.
        const bool defined;
    };

    //
    // Proxy information.
    //
    class ProxyInfo final : public TypeInfo, public std::enable_shared_from_this<ProxyInfo>
    {
    public:
        ProxyInfo(std::string);
        void init();

        void define(PyObject*);

        virtual std::string getId() const;

        virtual bool validate(PyObject*);

        virtual bool variableLength() const;
        virtual int wireSize() const;
        virtual Ice::OptionalFormat optionalFormat() const;

        virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0);
        virtual void
        unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool, const Ice::StringSeq* = 0);

        virtual void print(PyObject*, IceInternal::Output&, PrintObjectHistory*);

        const std::string id;
        PyObject* pythonType; // Borrowed reference - the enclosing Python module owns the reference.
        PyObject* typeObj;    // Borrowed reference - the "_t_XXX" variable owns the reference.
    };
    using ProxyInfoPtr = std::shared_ptr<ProxyInfo>;

    //
    // Exception information.
    //
    class ExceptionInfo : public std::enable_shared_from_this<ExceptionInfo>
    {
    public:
        void marshal(PyObject*, Ice::OutputStream*, ObjectMap*);
        PyObject* unmarshal(Ice::InputStream*);

        void print(PyObject*, IceInternal::Output&);
        void printMembers(PyObject*, IceInternal::Output&, PrintObjectHistory*);

        std::string id;
        ExceptionInfoPtr base;
        DataMemberList members;
        DataMemberList optionalMembers;
        bool usesClasses;
        PyObject* pythonType; // Borrowed reference - the enclosing Python module owns the reference.

    private:
        void writeMembers(PyObject*, Ice::OutputStream*, const DataMemberList&, ObjectMap*) const;
    };

    //
    // ValueWriter wraps a Python object for marshaling.
    //
    class ValueWriter : public Ice::Value
    {
    public:
        ValueWriter(PyObject*, ObjectMap*, const ValueInfoPtr&);
        ~ValueWriter();

        virtual void ice_preMarshal();

        virtual void _iceWrite(Ice::OutputStream*) const;
        virtual void _iceRead(Ice::InputStream*);

    private:
        void writeMembers(Ice::OutputStream*, const DataMemberList&) const;

        PyObject* _object;
        ObjectMap* _map;
        ValueInfoPtr _info;
        ValueInfoPtr _formal;
    };

    //
    // ValueReader unmarshals the state of an Ice object.
    //
    class ValueReader : public std::enable_shared_from_this<ValueReader>, public Ice::Value
    {
    public:
        ValueReader(PyObject*, const ValueInfoPtr&);
        ~ValueReader();

        virtual void ice_postUnmarshal();

        virtual void _iceWrite(Ice::OutputStream*) const;
        virtual void _iceRead(Ice::InputStream*);

        virtual ValueInfoPtr getInfo() const;

        PyObject* getObject() const; // Borrowed reference.

        Ice::SlicedDataPtr getSlicedData() const;

    private:
        PyObject* _object;
        ValueInfoPtr _info;
        Ice::SlicedDataPtr _slicedData;
    };

    //
    // ExceptionWriter wraps a Python user exception for marshaling.
    //
    class ExceptionWriter final : public Ice::UserException
    {
    public:
        ExceptionWriter(const PyObjectHandle&, const ExceptionInfoPtr& = 0) noexcept;
        ~ExceptionWriter();

        ExceptionWriter(const ExceptionWriter&) = default;

        const char* ice_id() const noexcept final;
        void ice_throw() const final;

        void _write(Ice::OutputStream*) const final;
        void _read(Ice::InputStream*) final;

        virtual bool _usesClasses() const;

    protected:
        void _writeImpl(Ice::OutputStream*) const final {}
        void _readImpl(Ice::InputStream*) final {}

    private:
        PyObjectHandle _ex;
        ExceptionInfoPtr _info;
        ObjectMap _objects;
    };

    //
    // ExceptionReader creates a Python user exception and unmarshals it.
    //
    class ExceptionReader final : public Ice::UserException
    {
    public:
        ExceptionReader(const ExceptionInfoPtr&) noexcept;
        ~ExceptionReader();

        ExceptionReader(const ExceptionReader&) = default;

        const char* ice_id() const noexcept final;
        void ice_throw() const final;

        void _write(Ice::OutputStream*) const final;
        void _read(Ice::InputStream*) final;

        bool _usesClasses() const final;

        PyObject* getException() const; // Borrowed reference.

    protected:
        void _writeImpl(Ice::OutputStream*) const final {}
        void _readImpl(Ice::InputStream*) final {}

    private:
        ExceptionInfoPtr _info;
        PyObjectHandle _ex;
    };

    std::string resolveCompactId(std::int32_t id);

    ClassInfoPtr lookupClassInfo(std::string_view);
    ValueInfoPtr lookupValueInfo(std::string_view);
    ExceptionInfoPtr lookupExceptionInfo(std::string_view);

    extern PyObject* Unset;

    bool initTypes(PyObject*);

    PyObject* createType(const TypeInfoPtr&);
    TypeInfoPtr getType(PyObject*);

    PyObject* createException(const ExceptionInfoPtr&);
    ExceptionInfoPtr getException(PyObject*);

    PyObject* createBuffer(const BufferPtr&);
}

extern "C" PyObject* IcePy_defineEnum(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineStruct(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineSequence(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineCustom(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineDictionary(PyObject*, PyObject*);
extern "C" PyObject* IcePy_declareProxy(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineProxy(PyObject*, PyObject*);
extern "C" PyObject* IcePy_declareClass(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineClass(PyObject*, PyObject*);
extern "C" PyObject* IcePy_declareValue(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineValue(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineException(PyObject*, PyObject*);
extern "C" PyObject* IcePy_stringify(PyObject*, PyObject*);
extern "C" PyObject* IcePy_stringifyException(PyObject*, PyObject*);

#endif
