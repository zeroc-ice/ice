// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/StreamI.h>
#include <Ice/Initialize.h>

using namespace std;
using namespace Ice;

//
// BasicInputStream
//
IceInternal::BasicInputStream::BasicInputStream(IceInternal::Instance* instance, InputStream* in) :
    BasicStream(instance), _in(in)
{
}

//
// BasicOutputStream
//
IceInternal::BasicOutputStream::BasicOutputStream(IceInternal::Instance* instance, OutputStream* out) :
    BasicStream(instance), _out(out)
{
}

//
// InputStreamI
//
Ice::InputStreamI::InputStreamI(const Ice::CommunicatorPtr& communicator, const vector<Byte>& data) :
    _communicator(communicator), _is(IceInternal::getInstance(communicator).get(), this)
{
    _is.writeBlob(data);
    _is.i = _is.b.begin();
}

Ice::InputStreamI::~InputStreamI()
{
}

CommunicatorPtr
Ice::InputStreamI::communicator() const
{
    return _communicator;
}

void
Ice::InputStreamI::sliceObjects(bool b)
{
    _is.sliceObjects(b);
}

bool
Ice::InputStreamI::readBool()
{
    bool v;
    _is.read(v);
    return v;
}

vector<bool>
Ice::InputStreamI::readBoolSeq()
{
    vector<bool> v;
    _is.read(v);
    return v;
}

bool*
Ice::InputStreamI::readBoolSeq(pair<const bool*, const bool*>& p)
{
    return _is.read(p);
}

Byte
Ice::InputStreamI::readByte()
{
    Byte v;
    _is.read(v);
    return v;
}

vector<Byte>
Ice::InputStreamI::readByteSeq()
{
    pair<const Byte*, const Byte*> p;
    _is.read(p);
    vector<Byte> v(p.first, p.second);
    return v;
}

void
Ice::InputStreamI::readByteSeq(pair<const Byte*, const Byte*>& p)
{
    _is.read(p);
}

Short
Ice::InputStreamI::readShort()
{
    Short v;
    _is.read(v);
    return v;
}

vector<Short>
Ice::InputStreamI::readShortSeq()
{
    vector<Short> v;
    _is.read(v);
    return v;
}

Short*
Ice::InputStreamI::readShortSeq(pair<const Short*, const Short*>& p)
{
    return _is.read(p);
}

Int
Ice::InputStreamI::readInt()
{
    Int v;
    _is.read(v);
    return v;
}

vector<Int>
Ice::InputStreamI::readIntSeq()
{
    vector<Int> v;
    _is.read(v);
    return v;
}

Int*
Ice::InputStreamI::readIntSeq(pair<const Int*, const Int*>& p)
{
    return _is.read(p);
}

Long
Ice::InputStreamI::readLong()
{
    Long v;
    _is.read(v);
    return v;
}

vector<Long>
Ice::InputStreamI::readLongSeq()
{
    vector<Long> v;
    _is.read(v);
    return v;
}

Long*
Ice::InputStreamI::readLongSeq(pair<const Long*, const Long*>& p)
{
    return _is.read(p);
}

Float
Ice::InputStreamI::readFloat()
{
    Float v;
    _is.read(v);
    return v;
}

vector<Float>
Ice::InputStreamI::readFloatSeq()
{
    vector<Float> v;
    _is.read(v);
    return v;
}

Float*
Ice::InputStreamI::readFloatSeq(pair<const Float*, const Float*>& p)
{
    return _is.read(p);
}

Double
Ice::InputStreamI::readDouble()
{
    Double v;
    _is.read(v);
    return v;
}

vector<Double>
Ice::InputStreamI::readDoubleSeq()
{
    vector<Double> v;
    _is.read(v);
    return v;
}

Double*
Ice::InputStreamI::readDoubleSeq(pair<const Double*, const Double*>& p)
{
    return _is.read(p);
}

string
Ice::InputStreamI::readString()
{
    string v;
    _is.read(v);
    return v;
}

vector<string>
Ice::InputStreamI::readStringSeq()
{
    vector<string> v;
    _is.read(v);
    return v;
}

wstring
Ice::InputStreamI::readWstring()
{
    wstring v;
    _is.read(v);
    return v;
}

vector<wstring>
Ice::InputStreamI::readWstringSeq()
{
    vector<wstring> v;
    _is.read(v);
    return v;
}

Int
Ice::InputStreamI::readSize()
{
    Int sz;
    _is.readSize(sz);
    return sz;
}

ObjectPrx
Ice::InputStreamI::readProxy()
{
    Ice::ObjectPrx v;
    _is.read(v);
    return v;
}

static void
patchObject(void* addr, ObjectPtr& v)
{
    ReadObjectCallback* cb = static_cast<ReadObjectCallback*>(addr);
    assert(cb);
    cb->invoke(v);
}

void
Ice::InputStreamI::readObject(const ReadObjectCallbackPtr& cb)
{
    _callbacks.push_back(cb); // Keep reference to callback.
    _is.read(patchObject, cb.get());
}

string
Ice::InputStreamI::readTypeId()
{
    string id;
    _is.readTypeId(id);
    return id;
}

void
Ice::InputStreamI::throwException()
{
    _is.throwException();
}

void
Ice::InputStreamI::startSlice()
{
    _is.startReadSlice();
}

void
Ice::InputStreamI::endSlice()
{
    _is.endReadSlice();
}

void
Ice::InputStreamI::startEncapsulation()
{
    _is.startReadEncaps();
}

void
Ice::InputStreamI::endEncapsulation()
{
    _is.endReadEncaps();
}

void
Ice::InputStreamI::skipSlice()
{
    _is.skipSlice();
}

void
Ice::InputStreamI::readPendingObjects()
{
    _is.readPendingObjects();
}

//
// OutputStreamI
//
Ice::OutputStreamI::OutputStreamI(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator), _os(IceInternal::getInstance(communicator).get(), this)
{
}

Ice::OutputStreamI::~OutputStreamI()
{
}

CommunicatorPtr
Ice::OutputStreamI::communicator() const
{
    return _communicator;
}

void
Ice::OutputStreamI::writeBool(bool v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeBoolSeq(const vector<bool>& v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeBoolSeq(const bool* begin, const bool* end)
{
    _os.write(begin, end);
}

void
Ice::OutputStreamI::writeByte(Byte v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeByteSeq(const vector<Byte>& v)
{
    if(v.size() == 0)
    {
        _os.writeSize(0);
    }
    else
    {
        _os.write(&v[0], &v[0] + v.size());
    }
}

void
Ice::OutputStreamI::writeByteSeq(const Byte* begin, const Byte* end)
{
    _os.write(begin, end);
}

void
Ice::OutputStreamI::writeShort(Short v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeShortSeq(const vector<Short>& v)
{
    if(v.size() == 0)
    {
        _os.writeSize(0);
    }
    else
    {
        _os.write(&v[0], &v[0] + v.size());
    }
}

void
Ice::OutputStreamI::writeShortSeq(const Short* begin, const Short* end)
{
    _os.write(begin, end);
}

void
Ice::OutputStreamI::writeInt(Int v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeIntSeq(const vector<Int>& v)
{
    if(v.size() == 0)
    {
        _os.writeSize(0);
    }
    else
    {
        _os.write(&v[0], &v[0] + v.size());
    }
}

void
Ice::OutputStreamI::writeIntSeq(const Int* begin, const Int* end)
{
    _os.write(begin, end);
}

void
Ice::OutputStreamI::writeLong(Long v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeLongSeq(const vector<Long>& v)
{
    if(v.size() == 0)
    {
        _os.writeSize(0);
    }
    else
    {
        _os.write(&v[0], &v[0] + v.size());
    }
}

void
Ice::OutputStreamI::writeLongSeq(const Long* begin, const Long* end)
{
    _os.write(begin, end);
}

void
Ice::OutputStreamI::writeFloat(Float v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeFloatSeq(const vector<Float>& v)
{
    if(v.size() == 0)
    {
        _os.writeSize(0);
    }
    else
    {
        _os.write(&v[0], &v[0] + v.size());
    }
}

void
Ice::OutputStreamI::writeFloatSeq(const Float* begin, const Float* end)
{
    _os.write(begin, end);
}

void
Ice::OutputStreamI::writeDouble(Double v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeDoubleSeq(const vector<Double>& v)
{
    if(v.size() == 0)
    {
        _os.writeSize(0);
    }
    else
    {
        _os.write(&v[0], &v[0] + v.size());
    }
}

void
Ice::OutputStreamI::writeDoubleSeq(const Double* begin, const Double* end)
{
    _os.write(begin, end);
}

void
Ice::OutputStreamI::writeString(const string& v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeStringSeq(const vector<string>& v)
{
    if(v.size() == 0)
    {
        _os.writeSize(0);
    }
    else
    {
        _os.write(&v[0], &v[0] + v.size());
    }
}

void
Ice::OutputStreamI::writeWstring(const wstring& v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeWstringSeq(const vector<wstring>& v)
{
    if(v.size() == 0)
    {
        _os.writeSize(0);
    }
    else
    {
        _os.write(&v[0], &v[0] + v.size());
    }
}

void
Ice::OutputStreamI::writeSize(Int sz)
{
    _os.writeSize(sz);
}

void
Ice::OutputStreamI::writeProxy(const ObjectPrx& v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeObject(const ObjectPtr& v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeTypeId(const string& id)
{
    _os.writeTypeId(id);
}

void
Ice::OutputStreamI::writeException(const UserException& v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::startSlice()
{
    _os.startWriteSlice();
}

void
Ice::OutputStreamI::endSlice()
{
    _os.endWriteSlice();
}

void
Ice::OutputStreamI::startEncapsulation()
{
    _os.startWriteEncaps();
}

void
Ice::OutputStreamI::endEncapsulation()
{
    _os.endWriteEncaps();
}

void
Ice::OutputStreamI::writePendingObjects()
{
    _os.writePendingObjects();
}

void
Ice::OutputStreamI::finished(vector<Byte>& bytes)
{
    vector<Byte>(_os.b.begin(), _os.b.end()).swap(bytes);
}

//
// ObjectReader
//
void
Ice::ObjectReader::__write(::IceInternal::BasicStream*) const
{
    assert(false);
}

void
Ice::ObjectReader::__read(::IceInternal::BasicStream* is, bool rid)
{
    IceInternal::BasicInputStream* bis = dynamic_cast<IceInternal::BasicInputStream*>(is);
    assert(bis);
    read(bis->_in, rid);
}

void
Ice::ObjectReader::__write(const ::Ice::OutputStreamPtr&) const
{
    assert(false);
}

void
Ice::ObjectReader::__read(const ::Ice::InputStreamPtr&, bool)
{
    assert(false);
}

//
// ObjectWriter
//
void
Ice::ObjectWriter::__write(::IceInternal::BasicStream* os) const
{
    IceInternal::BasicOutputStream* bos = dynamic_cast<IceInternal::BasicOutputStream*>(os);
    assert(bos);
    write(bos->_out);
}

void
Ice::ObjectWriter::__read(::IceInternal::BasicStream*, bool)
{
    assert(false);
}

void
Ice::ObjectWriter::__write(const ::Ice::OutputStreamPtr&) const
{
    assert(false);
}

void
Ice::ObjectWriter::__read(const ::Ice::InputStreamPtr&, bool)
{
    assert(false);
}
