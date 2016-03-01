// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceBT/DBus.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>

#include <dbus/dbus.h>
#include <stack>

using namespace std;
using namespace IceBT::DBus;

namespace
{

class ErrorWrapper
{
public:

    ErrorWrapper()
    {
        ::dbus_error_init(&err);
    }

    ~ErrorWrapper()
    {
        ::dbus_error_free(&err);
    }

    bool isSet() const
    {
        return ::dbus_error_is_set(&err);
    }

    DBusError err;
};

class ExceptionI : public IceBT::DBus::Exception
{
public:

    ExceptionI(const ErrorWrapper& w)
    {
        init(w.err);
    }

    ExceptionI(const DBusError& err)
    {
        init(err);
    }

    ExceptionI(const string& s) :
        Exception(s)
    {
    }

    string reason;

private:

    void init(const DBusError& err)
    {
        assert(::dbus_error_is_set(&err));
        reason = err.message;
    }
};

class PrimitiveType : public Type
{
public:

    PrimitiveType(Kind k) :
        _kind(k)
    {
    }

    virtual Kind getKind() const
    {
        return _kind;
    }

    virtual std::string getSignature() const
    {
        switch(_kind)
        {
        case KindBoolean:
            return DBUS_TYPE_BOOLEAN_AS_STRING;
        case KindByte:
            return DBUS_TYPE_BYTE_AS_STRING;
        case KindUint16:
            return DBUS_TYPE_UINT16_AS_STRING;
        case KindInt16:
            return DBUS_TYPE_INT16_AS_STRING;
        case KindUint32:
            return DBUS_TYPE_UINT32_AS_STRING;
        case KindInt32:
            return DBUS_TYPE_INT32_AS_STRING;
        case KindUint64:
            return DBUS_TYPE_UINT64_AS_STRING;
        case KindInt64:
            return DBUS_TYPE_INT64_AS_STRING;
        case KindDouble:
            return DBUS_TYPE_DOUBLE_AS_STRING;
        case KindString:
            return DBUS_TYPE_STRING_AS_STRING;
        case KindObjectPath:
            return DBUS_TYPE_OBJECT_PATH_AS_STRING;
        case KindSignature:
            return DBUS_TYPE_SIGNATURE_AS_STRING;
        case KindUnixFD:
            return DBUS_TYPE_UNIX_FD_AS_STRING;
        case KindInvalid:
        case KindArray:
        case KindVariant:
        case KindStruct:
        case KindDictEntry:
        default:
            assert(false);
            return "";
        }
    }

private:

    Kind _kind;
};

class MessageI : public Message
{
public:

    static MessagePtr wrap(DBusMessage* m)
    {
        return new MessageI(m, false);
    }

    static MessagePtr adopt(DBusMessage* m)
    {
        return new MessageI(m, true);
    }

    virtual ~MessageI()
    {
        ::dbus_message_unref(_message);
    }

    virtual bool isError() const
    {
        const int t = ::dbus_message_get_type(const_cast<DBusMessage*>(_message));
        return t == DBUS_MESSAGE_TYPE_ERROR;
    }

    virtual string getErrorName() const
    {
        const char* name = ::dbus_message_get_error_name(const_cast<DBusMessage*>(_message));
        return name ? name : string();
    }

    virtual void throwException()
    {
        assert(isError());

        //
        // Format the error name and any arguments into a string.
        //
        ostringstream ostr;
        ostr << getErrorName();
        ValuePtr v = read();
        if(v)
        {
            ostr << ":" << endl << v;
        }
        throw ExceptionI(ostr.str());
    }

    virtual bool isSignal() const
    {
        const int t = ::dbus_message_get_type(const_cast<DBusMessage*>(_message));
        return t == DBUS_MESSAGE_TYPE_SIGNAL;
    }

    virtual bool isMethodCall() const
    {
        const int t = ::dbus_message_get_type(const_cast<DBusMessage*>(_message));
        return t == DBUS_MESSAGE_TYPE_METHOD_CALL;
    }

    virtual bool isMethodReturn() const
    {
        const int t = ::dbus_message_get_type(const_cast<DBusMessage*>(_message));
        return t == DBUS_MESSAGE_TYPE_METHOD_RETURN;
    }

    virtual string getPath() const
    {
        const char* s = ::dbus_message_get_path(const_cast<DBusMessage*>(_message));
        return s ? string(s) : string();
    }

    virtual string getInterface() const
    {
        const char* s = ::dbus_message_get_interface(const_cast<DBusMessage*>(_message));
        return s ? string(s) : string();
    }

    virtual string getMember() const
    {
        const char* s = ::dbus_message_get_member(const_cast<DBusMessage*>(_message));
        return s ? string(s) : string();
    }

    virtual string getDestination() const
    {
        const char* s = ::dbus_message_get_destination(const_cast<DBusMessage*>(_message));
        return s ? string(s) : string();
    }

    virtual void write(const ValuePtr& v)
    {
        DBusMessageIter iter;
        ::dbus_message_iter_init_append(_message, &iter);
        writeValue(v, &iter);
    }

    virtual void write(const vector<ValuePtr>& v)
    {
        DBusMessageIter iter;
        ::dbus_message_iter_init_append(_message, &iter);
        for(vector<ValuePtr>::const_iterator p = v.begin(); p != v.end(); ++p)
        {
            writeValue(*p, &iter);
        }
    }

    virtual bool checkTypes(const vector<TypePtr>& types) const
    {
        string msgSig = ::dbus_message_get_signature(_message);
        string sig;
        for(vector<TypePtr>::const_iterator p = types.begin(); p != types.end(); ++p)
        {
            sig += (*p)->getSignature();
        }
        return sig == msgSig;
    }

    virtual ValuePtr read()
    {
        //
        // Read a single value.
        //

        TypePtr type = buildType(); // Build a type from the message's signature.
        if(!type)
        {
            return 0;
        }
        assert(_iterators.empty());
        _iterators.push(DBusMessageIter());
        _iter = &_iterators.top();
        ::dbus_message_iter_init(_message, _iter);
        ValuePtr v = readValue(type);
        assert(_iterators.size() == 1);
        _iterators.pop();
        return v;
    }

    virtual vector<ValuePtr> readAll()
    {
        vector<TypePtr> types = buildTypes(); // Build types from the message's signature.

        assert(_iterators.empty());
        _iterators.push(DBusMessageIter());
        _iter = &_iterators.top();
        ::dbus_message_iter_init(_message, _iter);

        vector<ValuePtr> values;
        for(vector<TypePtr>::iterator p = types.begin(); p != types.end(); ++p)
        {
            values.push_back(readValue(*p));
            next();
        }

        assert(_iterators.size() == 1);
        _iterators.pop();

        return values;
    }

    DBusMessage* message()
    {
        return _message;
    }

private:

    MessageI(DBusMessage* m, bool adopt) :
        _message(m),
        _iter(0)
    {
        assert(_message);
        if(!adopt)
        {
            ::dbus_message_ref(m); // Bump the reference count.
        }
    }

    vector<TypePtr> buildTypes()
    {
        vector<TypePtr> types;

        string sig = ::dbus_message_get_signature(_message);

        string::iterator p = sig.begin();
        while(p != sig.end())
        {
            types.push_back(buildType(p));
        }

        return types;
    }

    TypePtr buildType()
    {
        string sig = ::dbus_message_get_signature(_message);
        if(sig.empty())
        {
            return 0;
        }
        string::iterator p = sig.begin();
        return buildType(p);
    }

    TypePtr buildType(string::iterator& iter)
    {
        string::value_type ch = *iter++;
        switch(ch)
        {
            case DBUS_TYPE_BOOLEAN:
            case DBUS_TYPE_BYTE:
            case DBUS_TYPE_UINT16:
            case DBUS_TYPE_INT16:
            case DBUS_TYPE_UINT32:
            case DBUS_TYPE_INT32:
            case DBUS_TYPE_UINT64:
            case DBUS_TYPE_INT64:
            case DBUS_TYPE_DOUBLE:
            case DBUS_TYPE_STRING:
            case DBUS_TYPE_OBJECT_PATH:
            case DBUS_TYPE_SIGNATURE:
            case DBUS_TYPE_UNIX_FD:
                return Type::getPrimitive(convertKind(ch));
            case DBUS_TYPE_ARRAY:
            {
                TypePtr elem = buildType(iter);
                return new ArrayType(elem);
            }
            case DBUS_TYPE_VARIANT:
            {
                return new VariantType;
            }
            case '(': // Struct
            {
                vector<TypePtr> types;
                while(*iter != ')')
                {
                    types.push_back(buildType(iter));
                }
                assert(*iter == ')');
                ++iter;
                return new StructType(types);
            }
            case '{': // Dict entry
            {
                TypePtr key, value;
                key = buildType(iter);
                value = buildType(iter);
                assert(*iter == '}');
                ++iter;
                return new DictEntryType(key, value);
            }
            case DBUS_TYPE_INVALID:
                assert(false);
                break;
        }

        return 0;
    }

    void writeValue(const ValuePtr& p, DBusMessageIter* iter)
    {
        switch(p->getType()->getKind())
        {
        case Type::KindBoolean:
        {
            BooleanValuePtr v = BooleanValuePtr::dynamicCast(p);
            assert(v);
            const dbus_bool_t b = v->v ? TRUE : FALSE;
            ::dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &b);
            break;
        }
        case Type::KindByte:
        {
            ByteValuePtr v = ByteValuePtr::dynamicCast(p);
            assert(v);
            ::dbus_message_iter_append_basic(iter, DBUS_TYPE_BYTE, &v->v);
            break;
        }
        case Type::KindUint16:
        {
            Uint16ValuePtr v = Uint16ValuePtr::dynamicCast(p);
            assert(v);
            ::dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT16, &v->v);
            break;
        }
        case Type::KindInt16:
        {
            Int16ValuePtr v = Int16ValuePtr::dynamicCast(p);
            assert(v);
            ::dbus_message_iter_append_basic(iter, DBUS_TYPE_INT16, &v->v);
            break;
        }
        case Type::KindUint32:
        {
            Uint32ValuePtr v = Uint32ValuePtr::dynamicCast(p);
            assert(v);
            ::dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT32, &v->v);
            break;
        }
        case Type::KindInt32:
        {
            Int32ValuePtr v = Int32ValuePtr::dynamicCast(p);
            assert(v);
            ::dbus_message_iter_append_basic(iter, DBUS_TYPE_INT32, &v->v);
            break;
        }
        case Type::KindUint64:
        {
            Uint64ValuePtr v = Uint64ValuePtr::dynamicCast(p);
            assert(v);
            ::dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT64, &v->v);
            break;
        }
        case Type::KindInt64:
        {
            Int64ValuePtr v = Int64ValuePtr::dynamicCast(p);
            assert(v);
            ::dbus_message_iter_append_basic(iter, DBUS_TYPE_INT64, &v->v);
            break;
        }
        case Type::KindDouble:
        {
            DoubleValuePtr v = DoubleValuePtr::dynamicCast(p);
            assert(v);
            ::dbus_message_iter_append_basic(iter, DBUS_TYPE_DOUBLE, &v->v);
            break;
        }
        case Type::KindString:
        {
            StringValuePtr v = StringValuePtr::dynamicCast(p);
            assert(v);
            const char* s = v->v.c_str();
            ::dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &s);
            break;
        }
        case Type::KindObjectPath:
        {
            ObjectPathValuePtr v = ObjectPathValuePtr::dynamicCast(p);
            assert(v);
            const char* s = v->v.c_str();
            ::dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &s);
            break;
        }
        case Type::KindSignature:
        {
            SignatureValuePtr v = SignatureValuePtr::dynamicCast(p);
            assert(v);
            const char* s = v->v.c_str();
            ::dbus_message_iter_append_basic(iter, DBUS_TYPE_SIGNATURE, &s);
            break;
        }
        case Type::KindUnixFD:
        {
            UnixFDValuePtr v = UnixFDValuePtr::dynamicCast(p);
            assert(v);
            ::dbus_message_iter_append_basic(iter, DBUS_TYPE_UNIX_FD, &v->v);
            break;
        }
        case Type::KindArray:
        {
            ArrayTypePtr t = ArrayTypePtr::dynamicCast(p->getType());
            assert(t);
            ArrayValuePtr arr = ArrayValuePtr::dynamicCast(p);
            assert(arr);

            string sig = t->elementType->getSignature();

            DBusMessageIter sub;
            if(!::dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, sig.c_str(), &sub))
            {
                throw ExceptionI("out of memory while calling dbus_message_iter_open_container");
            }
            for(vector<ValuePtr>::iterator q = arr->elements.begin(); q != arr->elements.end(); ++q)
            {
                writeValue(*q, &sub);
            }
            if(!::dbus_message_iter_close_container(iter, &sub))
            {
                throw ExceptionI("out of memory while calling dbus_message_iter_close_container");
            }
            break;
        }
        case Type::KindVariant:
        {
            VariantValuePtr v = VariantValuePtr::dynamicCast(p);
            assert(v);

            string sig = v->v->getType()->getSignature();

            DBusMessageIter sub;
            if(!::dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, sig.c_str(), &sub))
            {
                throw ExceptionI("out of memory while calling dbus_message_iter_open_container");
            }
            writeValue(v->v, &sub);
            if(!::dbus_message_iter_close_container(iter, &sub))
            {
                throw ExceptionI("out of memory while calling dbus_message_iter_close_container");
            }
            break;
        }
        case Type::KindStruct:
        {
            StructValuePtr v = StructValuePtr::dynamicCast(p);
            assert(v);

            DBusMessageIter sub;
            if(!::dbus_message_iter_open_container(iter, DBUS_TYPE_STRUCT, 0, &sub))
            {
                throw ExceptionI("out of memory while calling dbus_message_iter_open_container");
            }
            for(vector<ValuePtr>::iterator q = v->members.begin(); q != v->members.end(); ++q)
            {
                writeValue(*q, &sub);
            }
            if(!::dbus_message_iter_close_container(iter, &sub))
            {
                throw ExceptionI("out of memory while calling dbus_message_iter_close_container");
            }
            break;
        }
        case Type::KindDictEntry:
        {
            DictEntryValuePtr v = DictEntryValuePtr::dynamicCast(p);
            assert(v);

            DBusMessageIter sub;
            if(!::dbus_message_iter_open_container(iter, DBUS_TYPE_DICT_ENTRY, 0, &sub))
            {
                throw ExceptionI("out of memory while calling dbus_message_iter_open_container");
            }
            writeValue(v->key, &sub);
            writeValue(v->value, &sub);
            if(!::dbus_message_iter_close_container(iter, &sub))
            {
                throw ExceptionI("out of memory while calling dbus_message_iter_close_container");
            }
            break;
        }
        case Type::KindInvalid:
        default:
            assert(false);
            break;
        }
    }

    void next()
    {
        ::dbus_message_iter_next(_iter);
    }

    static Type::Kind convertKind(int t)
    {
        switch(t)
        {
        case DBUS_TYPE_INVALID:
            return Type::KindInvalid;
        case DBUS_TYPE_BOOLEAN:
            return Type::KindBoolean;
        case DBUS_TYPE_BYTE:
            return Type::KindByte;
        case DBUS_TYPE_UINT16:
            return Type::KindUint16;
        case DBUS_TYPE_INT16:
            return Type::KindInt16;
        case DBUS_TYPE_UINT32:
            return Type::KindUint32;
        case DBUS_TYPE_INT32:
            return Type::KindInt32;
        case DBUS_TYPE_UINT64:
            return Type::KindUint64;
        case DBUS_TYPE_INT64:
            return Type::KindInt64;
        case DBUS_TYPE_DOUBLE:
            return Type::KindDouble;
        case DBUS_TYPE_STRING:
            return Type::KindString;
        case DBUS_TYPE_OBJECT_PATH:
            return Type::KindObjectPath;
        case DBUS_TYPE_SIGNATURE:
            return Type::KindSignature;
        case DBUS_TYPE_UNIX_FD:
            return Type::KindUnixFD;
        case DBUS_TYPE_ARRAY:
            return Type::KindArray;
        case DBUS_TYPE_VARIANT:
            return Type::KindVariant;
        case DBUS_TYPE_STRUCT:
            return Type::KindStruct;
        case DBUS_TYPE_DICT_ENTRY:
            return Type::KindDictEntry;
        default:
            throw ExceptionI("unknown arg type");
            return Type::KindInvalid;
        }
    }

    Type::Kind currentKind() const
    {
        int t = ::dbus_message_iter_get_arg_type(const_cast<DBusMessageIter*>(_iter));
        return convertKind(t);
    }

    ValuePtr readValue(const TypePtr& t)
    {
        switch(t->getKind())
        {
        case Type::KindInvalid:
            assert(false);
            return 0;
        case Type::KindBoolean:
        {
            bool v;
            ::dbus_message_iter_get_basic(_iter, &v);
            return new BooleanValue(v);
        }
        case Type::KindByte:
        {
            unsigned char v;
            ::dbus_message_iter_get_basic(_iter, &v);
            return new ByteValue(v);
        }
        case Type::KindUint16:
        {
            unsigned short v;
            ::dbus_message_iter_get_basic(_iter, &v);
            return new Uint16Value(v);
        }
        case Type::KindInt16:
        {
            short v;
            ::dbus_message_iter_get_basic(_iter, &v);
            return new Int16Value(v);
        }
        case Type::KindUint32:
        {
            unsigned int v;
            ::dbus_message_iter_get_basic(_iter, &v);
            return new Uint32Value(v);
        }
        case Type::KindInt32:
        {
            int v;
            ::dbus_message_iter_get_basic(_iter, &v);
            return new Int32Value(v);
        }
        case Type::KindUint64:
        {
            unsigned long long v;
            ::dbus_message_iter_get_basic(_iter, &v);
            return new Uint64Value(v);
        }
        case Type::KindInt64:
        {
            long long v;
            ::dbus_message_iter_get_basic(_iter, &v);
            return new Int64Value(v);
        }
        case Type::KindDouble:
        {
            double v;
            ::dbus_message_iter_get_basic(_iter, &v);
            return new DoubleValue(v);
        }
        case Type::KindString:
        {
            char* str;
            ::dbus_message_iter_get_basic(_iter, &str);
            return new StringValue(str);
        }
        case Type::KindObjectPath:
        {
            char* str;
            ::dbus_message_iter_get_basic(_iter, &str);
            return new ObjectPathValue(str);
        }
        case Type::KindSignature:
        {
            char* str;
            ::dbus_message_iter_get_basic(_iter, &str);
            return new SignatureValue(str);
        }
        case Type::KindUnixFD:
        {
            unsigned int v;
            ::dbus_message_iter_get_basic(_iter, &v);
            return new UnixFDValue(v);
        }
        case Type::KindArray:
        {
            ArrayTypePtr arr = ArrayTypePtr::dynamicCast(t);
            assert(arr);
            pushIter();
            ArrayValuePtr v = new ArrayValue(arr);
            while(true)
            {
                Type::Kind k = currentKind();
                if(k == Type::KindInvalid)
                {
                    break;
                }
                assert(k == arr->elementType->getKind());
                v->elements.push_back(readValue(arr->elementType));
                next();
            }
            popIter();
            return v;
        }
        case Type::KindVariant:
        {
            pushIter();
            //
            // Get the type signature of this variant's value.
            //
            string sig = ::dbus_message_iter_get_signature(_iter);
            string::iterator p = sig.begin();
            TypePtr t = buildType(p);
            VariantValuePtr v = new VariantValue;
            v->v = readValue(t);
            popIter();
            return v;
        }
        case Type::KindStruct:
        {
            StructTypePtr st = StructTypePtr::dynamicCast(t);
            assert(st);
            pushIter();
            StructValuePtr v = new StructValue(st);
            for(vector<TypePtr>::iterator p = st->memberTypes.begin(); p != st->memberTypes.end(); ++p)
            {
                v->members.push_back(readValue(*p));
                next();
            }
            popIter();
            return v;
        }
        case Type::KindDictEntry:
        {
            DictEntryTypePtr dt = DictEntryTypePtr::dynamicCast(t);
            assert(dt);
            pushIter();
            DictEntryValuePtr v = new DictEntryValue(dt);
            v->key = readValue(dt->keyType);
            next();
            v->value = readValue(dt->valueType);
            popIter();
            return v;
        }
        default:
            assert(false);
            return 0;
        }
    }

    void pushIter()
    {
        DBusMessageIter* parent = _iter;
        _iterators.push(DBusMessageIter());
        _iter = &_iterators.top();
        ::dbus_message_iter_recurse(parent, _iter);
    }

    void popIter()
    {
        assert(_iterators.size() > 1);
        _iterators.pop();
        _iter = &_iterators.top();
    }

    DBusMessage* _message;
    stack<DBusMessageIter> _iterators;
    DBusMessageIter* _iter;
};
typedef IceUtil::Handle<MessageI> MessageIPtr;

static void pendingCallCompletedCallback(DBusPendingCall*, void*);
static void pendingCallFree(void*);

class AsyncResultI : public AsyncResult
{
public:

    AsyncResultI(DBusPendingCall* call, const AsyncCallbackPtr& cb) :
        _call(call),
        _callback(cb),
        _status(StatusPending)
    {
        //
        // Bump our refcount to ensure this object lives until the reply is received.
        // The pendingFree function will decrement the refcount.
        //
        __incRef();

        if(!::dbus_pending_call_set_notify(_call, pendingCallCompletedCallback, this, pendingCallFree))
        {
            ::dbus_pending_call_cancel(_call);
            ::dbus_pending_call_unref(_call);
            throw ExceptionI("dbus_pending_call_set_notify failed");
        }

        //
        // There's a potential race condition with dbus_pending_call_set_notify. If the
        // pending call is already completed when we call dbus_pending_call_set_notify,
        // our callback will NOT be invoked. We manually check the completion status
        // here and handle the reply if necessary.
        //
        bool complete;
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);
            complete = (::dbus_pending_call_get_completed(_call) && _status == StatusPending);
        }

        if(complete)
        {
            replyReceived();
        }
    }

    ~AsyncResultI()
    {
        ::dbus_pending_call_unref(_call);
    }

    virtual bool isPending() const
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);
        return _status == StatusPending;
    }

    virtual bool isComplete() const
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);
        return _status == StatusComplete;
    }

    virtual MessagePtr waitUntilFinished() const
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);
        while(_status == StatusPending)
        {
            _lock.wait();
        }
        return _reply;
    }

    virtual MessagePtr getReply() const
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);
        return _reply;
    }

    virtual void setCallback(const AsyncCallbackPtr& cb)
    {
        bool call = false;

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);
            _callback = cb;
            if(_status == StatusComplete)
            {
                call = true;
            }
        }

        if(call)
        {
            try
            {
                cb->completed(this);
            }
            catch(...)
            {
            }
        }
    }

    void replyReceived()
    {
        assert(::dbus_pending_call_get_completed(_call));

        AsyncCallbackPtr cb;

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

            //
            // Make sure we haven't already handled the reply (see constructor).
            //
            if(_status == StatusPending)
            {
                DBusMessage* m = ::dbus_pending_call_steal_reply(_call);
                assert(m);
                _reply = MessageI::adopt(m);
                _status = StatusComplete;
                cb = _callback;
                _lock.notifyAll();
            }
        }

        if(cb)
        {
            try
            {
                cb->completed(this);
            }
            catch(...)
            {
            }
        }
    }

private:

    IceUtil::Monitor<IceUtil::Mutex> _lock;
    DBusPendingCall* _call;
    AsyncCallbackPtr _callback;

    enum Status { StatusPending, StatusComplete };
    Status _status;

    MessagePtr _reply;
};

static void
pendingCallCompletedCallback(DBusPendingCall*, void* userData)
{
    AsyncResultI* r = static_cast<AsyncResultI*>(userData);
    assert(r);
    r->replyReceived();
}

static void
pendingCallFree(void* userData)
{
    AsyncResultI* r = static_cast<AsyncResultI*>(userData);
    assert(r);
    r->__decRef();
}

static DBusHandlerResult filterCallback(DBusConnection*, DBusMessage*, void*);
static void freeConnection(void*);

class ConnectionI;
typedef IceUtil::Handle<ConnectionI> ConnectionIPtr;

class ConnectionI : public Connection
{
public:

    ConnectionI() :
        _connection(0),
        _closed(false)
    {
    }

    virtual ~ConnectionI()
    {
        if(_connection)
        {
            ::dbus_connection_unref(_connection);
        }
    }

    virtual void addFilter(const FilterPtr& f)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

        _filters.push_back(f);
    }

    virtual void removeFilter(const FilterPtr& f)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

        for(vector<FilterPtr>::iterator p = _filters.begin(); p != _filters.end(); ++p)
        {
            if(p->get() == f.get())
            {
                _filters.erase(p);
                break;
            }
        }
    }

    virtual void addService(const string& path, const ServicePtr& s)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

        map<string, ServicePtr>::iterator p = _services.find(path);
        if(p != _services.end())
        {
            throw ExceptionI("service with path `" + path + "' already registered");
        }
        _services[path] = s;
    }

    virtual void removeService(const string& path)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

        map<string, ServicePtr>::iterator p = _services.find(path);
        if(p != _services.end())
        {
            _services.erase(p);
        }
    }

    virtual AsyncResultPtr callAsync(const MessagePtr& m, const AsyncCallbackPtr& cb)
    {
        MessageIPtr mi = MessageIPtr::dynamicCast(m);
        assert(mi);

        DBusPendingCall* call;
        if(!::dbus_connection_send_with_reply(_connection, mi->message(), &call, -1))
        {
            throw ExceptionI("dbus_connection_send_with_reply failed");
        }
        if(!call)
        {
            throw ExceptionI("dbus_connection_send_with_reply failed - disconnected?");
        }
        return new AsyncResultI(call, cb);
    }

    virtual void sendAsync(const MessagePtr& m)
    {
        MessageIPtr mi = MessageIPtr::dynamicCast(m);
        assert(mi);

        //
        // D-Bus queues the message without blocking.
        //
        dbus_uint32_t serial;
        if(!::dbus_connection_send(_connection, mi->message(), &serial))
        {
            throw ExceptionI("dbus_connection_send failed");
        }
    }

    virtual void close()
    {
        ::dbus_connection_close(_connection);

        //
        // Send the "close" message.
        //
        while(::dbus_connection_dispatch(_connection) == DBUS_DISPATCH_DATA_REMAINS)
            ;

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);
            _closed = true;
            _services.clear();
        }

        _thread->getThreadControl().join();
    }

    void connect(bool system)
    {
        ErrorWrapper err;

        _connection = ::dbus_bus_get_private(system ? DBUS_BUS_SYSTEM : DBUS_BUS_SESSION, &err.err);
        if(err.isSet())
        {
            throw ExceptionI(err);
        }
        assert(_connection);

        ::dbus_connection_set_exit_on_disconnect(_connection, FALSE);

        if(!::dbus_connection_add_filter(_connection, filterCallback, this, freeConnection))
        {
            throw ExceptionI("out of memory calling dbus_connection_add_filter");
        }

        //
        // The filter function will only see the message types that we add below.
        //
        ::dbus_bus_add_match(_connection, "type='signal'", 0);
        //::dbus_bus_add_match(_connection, "type='method_call'", 0);

        __incRef(); // __decRef called in freeConnection.

        _thread = new HelperThread(this);
        _thread->start();
    }

    DBusConnection* connection()
    {
        return _connection;
    }

    DBusHandlerResult handleMessage(DBusMessage* m)
    {
        vector<FilterPtr> filters;
        map<string, ServicePtr> services;
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);
            filters = _filters;
            services = _services;
        }

        MessagePtr msg = MessageI::wrap(m);
        for(vector<FilterPtr>::iterator p = filters.begin(); p != filters.end(); ++p)
        {
            try
            {
                if((*p)->handleMessage(this, msg))
                {
                    return DBUS_HANDLER_RESULT_HANDLED;
                }
            }
            catch(...)
            {
                // Ignore.
            }
        }

        if(msg->isMethodCall())
        {
            map<string, ServicePtr>::iterator p = services.find(msg->getPath());
            if(p != _services.end())
            {
                try
                {
                    p->second->handleMethodCall(this, msg);
                }
                catch(...)
                {
                    // Ignore.
                }
                return DBUS_HANDLER_RESULT_HANDLED;
            }
        }

        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

private:

    void run()
    {
        while(::dbus_connection_read_write_dispatch(_connection, 200))
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);
            if(_closed)
            {
                break;
            }
        }
    }

    class HelperThread : public IceUtil::Thread
    {
    public:

        HelperThread(ConnectionI* con) :
            _con(con)
        {
        }

        void run()
        {
            _con->run();
        }

    private:

        ConnectionI* _con;
    };

    friend class HelperThread;

    DBusConnection* _connection;
    IceUtil::ThreadPtr _thread;
    IceUtil::Monitor<IceUtil::Mutex> _lock;
    bool _closed;
    vector<FilterPtr> _filters;
    map<string, ServicePtr> _services;
};

static DBusHandlerResult
filterCallback(DBusConnection*, DBusMessage* message, void* userData)
{
    ConnectionI* c = static_cast<ConnectionI*>(userData);
    assert(c);
    return c->handleMessage(message);
}

static void freeConnection(void* p)
{
    ConnectionI* c = static_cast<ConnectionI*>(p);
    assert(c);
    c->__decRef();
}

}

TypePtr
IceBT::DBus::Type::getPrimitive(Kind k)
{
    switch(k)
    {
    case KindBoolean:
    case KindByte:
    case KindUint16:
    case KindInt16:
    case KindUint32:
    case KindInt32:
    case KindUint64:
    case KindInt64:
    case KindDouble:
    case KindString:
    case KindObjectPath:
    case KindSignature:
    case KindUnixFD:
        return new PrimitiveType(k);
    case KindInvalid:
    case KindArray:
    case KindVariant:
    case KindStruct:
    case KindDictEntry:
    default:
        assert(false);
        return 0;
    }
}

string
IceBT::DBus::ArrayType::getSignature() const
{
    string r = DBUS_TYPE_ARRAY_AS_STRING;
    r += elementType->getSignature();
    return r;
}

string
IceBT::DBus::VariantType::getSignature() const
{
    return DBUS_TYPE_VARIANT_AS_STRING;
}

string
IceBT::DBus::StructType::getSignature() const
{
    string r = "(";
    for(vector<TypePtr>::const_iterator p = memberTypes.begin(); p != memberTypes.end(); ++p)
    {
        r += (*p)->getSignature();
    }
    r += ")";
    return r;
}

string
IceBT::DBus::DictEntryType::getSignature() const
{
    string r = "{";
    r += keyType->getSignature();
    r += valueType->getSignature();
    r += "}";
    return r;
}

MessagePtr
IceBT::DBus::Message::createCall(const string& dest, const string& path, const string& iface, const string& method)
{
    assert(!path.empty() && !method.empty());
    const char* sdest = dest.empty() ? 0 : dest.c_str();
    const char* spath = path.c_str();
    const char* siface = iface.empty() ? 0 : iface.c_str();
    const char* smethod = method.c_str();
    DBusMessage* m = ::dbus_message_new_method_call(sdest, spath, siface, smethod);
    if(!m)
    {
        throw ExceptionI("failure creating DBus method call");
    }
    return MessageI::adopt(m);
}

MessagePtr
IceBT::DBus::Message::createReturn(const MessagePtr& call)
{
    MessageIPtr c = MessageIPtr::dynamicCast(call);
    assert(c);
    DBusMessage* r = ::dbus_message_new_method_return(c->message());
    if(!r)
    {
        throw ExceptionI("failure creating DBus method return");
    }
    return MessageI::adopt(r);
}

ConnectionPtr
IceBT::DBus::Connection::getSystemBus()
{
    ConnectionI* conn = new ConnectionI;
    conn->connect(true);
    return conn;
}

ConnectionPtr
IceBT::DBus::Connection::getSessionBus()
{
    ConnectionI* conn = new ConnectionI;
    conn->connect(false);
    return conn;
}

void
IceBT::DBus::initThreads()
{
    ::dbus_threads_init_default();
}
