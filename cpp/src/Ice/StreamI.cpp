// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
    _is.b.copyFromVector(data);
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
    vector<Byte> v;
    _is.read(v);
    return v;
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
Ice::OutputStreamI::writeByte(Byte v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeByteSeq(const vector<Byte>& v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeShort(Short v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeShortSeq(const vector<Short>& v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeInt(Int v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeIntSeq(const vector<Int>& v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeLong(Long v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeLongSeq(const vector<Long>& v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeFloat(Float v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeFloatSeq(const vector<Float>& v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeDouble(Double v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeDoubleSeq(const vector<Double>& v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeString(const string& v)
{
    _os.write(v);
}

void
Ice::OutputStreamI::writeStringSeq(const vector<string>& v)
{
    _os.write(v);
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
    _os.b.copyToVector(bytes);
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
