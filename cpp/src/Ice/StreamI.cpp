// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/StreamI.h>
#include <Ice/Instance.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>
#include <Ice/UserExceptionFactory.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

//
// This class implements the internal interface UserExceptionFactory and delegates to
// the user-supplied instance of UserExceptionReaderFactory.
//
class UserExceptionFactoryI : public IceInternal::UserExceptionFactory
{
public:

    UserExceptionFactoryI(const UserExceptionReaderFactoryPtr& factory) :
        _factory(factory)
    {
    }

    virtual void createAndThrow(const string& id)
    {
        _factory->createAndThrow(id);
    }

private:

    const UserExceptionReaderFactoryPtr _factory;
};

}

//
// UserExceptionReader
//
UserExceptionReader::UserExceptionReader(const CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

UserExceptionReader::~UserExceptionReader() throw()
{
}

void
UserExceptionReader::__write(BasicStream*) const
{
    assert(false);
}

void
UserExceptionReader::__read(BasicStream* is)
{
    InputStreamI* stream = reinterpret_cast<InputStreamI*>(is->closure());
    assert(stream);
    read(stream);
}

//
// InputStreamI
//
InputStreamI::InputStreamI(const CommunicatorPtr& communicator, const pair<const Byte*, const Byte*>& data,
                           bool copyData) :
    _communicator(communicator),
    _closure(0)
{
    Instance* instance = getInstance(communicator).get();
    initialize(instance, data, instance->defaultsAndOverrides()->defaultEncoding, copyData);
}

InputStreamI::InputStreamI(const CommunicatorPtr& communicator, const pair<const Byte*, const Byte*>& data,
                           const EncodingVersion& v, bool copyData) :
    _communicator(communicator),
    _closure(0)
{
    initialize(getInstance(communicator).get(), data, v, copyData);
}

InputStreamI::~InputStreamI()
{
    delete _is;
}

CommunicatorPtr
InputStreamI::communicator() const
{
    return _communicator;
}

void
InputStreamI::sliceObjects(bool b)
{
    _is->sliceObjects(b);
}

Int
InputStreamI::readSize()
{
    return _is->readSize();
}

Int
InputStreamI::readAndCheckSeqSize(int minSize)
{
    return _is->readAndCheckSeqSize(minSize);
}

ObjectPrx
InputStreamI::readProxy()
{
    ObjectPrx v;
    _is->read(v);
    return v;
}

namespace
{

void
patchObject(void* addr, const ObjectPtr& v)
{
    ReadObjectCallback* cb = static_cast<ReadObjectCallback*>(addr);
    assert(cb);
    cb->invoke(v);
}

}

void
InputStreamI::readObject(const ReadObjectCallbackPtr& cb)
{
    _callbacks.push_back(cb); // Keep reference to callback.
    _is->read(patchObject, cb.get());
}

void
InputStreamI::read(bool& v)
{
    _is->read(v);
}

void
InputStreamI::read(Byte& v)
{
    _is->read(v);
}

void
InputStreamI::read(Short& v)
{
    _is->read(v);
}

void
InputStreamI::read(Int& v)
{
    _is->read(v);
}

void
InputStreamI::read(Long& v)
{
    _is->read(v);
}

void
InputStreamI::read(Float& v)
{
    _is->read(v);
}

void
InputStreamI::read(Double& v)
{
    _is->read(v);
}

void
InputStreamI::read(string& v, bool convert)
{
    _is->read(v, convert);
}

void
InputStreamI::read(const char*& vdata, size_t& vsize)
{
    _is->read(vdata, vsize);
}

void
InputStreamI::read(const char*& vdata, size_t& vsize, string& holder)
{
    _is->read(vdata, vsize, holder);
}

void
InputStreamI::read(vector<string>& v, bool convert)
{
    _is->read(v, convert);
}

void
InputStreamI::read(wstring& v)
{
    _is->read(v);
}

void
InputStreamI::read(vector<bool>& v)
{
    _is->read(v);
}

void
InputStreamI::read(pair<const bool*, const bool*>& p, ::IceUtil::ScopedArray<bool>& result)
{
    _is->read(p, result);
}

void
InputStreamI::read(pair<const Byte*, const Byte*>& p)
{
    _is->read(p);
}
void
InputStreamI::read(pair<const Short*, const Short*>& p, ::IceUtil::ScopedArray<Short>& result)
{
    _is->read(p, result);
}

void
InputStreamI::read(pair<const Int*, const Int*>& p, ::IceUtil::ScopedArray<Int>& result)
{
    _is->read(p, result);
}

void
InputStreamI::read(pair<const Long*, const Long*>& p, ::IceUtil::ScopedArray<Long>& result)
{
    _is->read(p, result);
}

void
InputStreamI::read(pair<const Float*, const Float*>& p, ::IceUtil::ScopedArray<Float>& result)
{
    _is->read(p, result);
}

void
InputStreamI::read(pair<const Double*, const Double*>& p, ::IceUtil::ScopedArray<Double>& result)
{
    _is->read(p, result);
}

bool
InputStreamI::readOptional(Int tag, OptionalFormat format)
{
    return _is->readOpt(tag, format);
}

void
InputStreamI::throwException()
{
    _is->throwException();
}

void
InputStreamI::throwException(const UserExceptionReaderFactoryPtr& factory)
{
    UserExceptionFactoryPtr del = new UserExceptionFactoryI(factory);
    _is->throwException(del);
}

void
InputStreamI::startObject()
{
    _is->startReadObject();
}

SlicedDataPtr
InputStreamI::endObject(bool preserve)
{
    return _is->endReadObject(preserve);
}

void
InputStreamI::startException()
{
    _is->startReadException();
}

SlicedDataPtr
InputStreamI::endException(bool preserve)
{
    return _is->endReadException(preserve);
}

string
InputStreamI::startSlice()
{
    return _is->startReadSlice();
}

void
InputStreamI::endSlice()
{
    _is->endReadSlice();
}

void
InputStreamI::skipSlice()
{
    _is->skipSlice();
}

EncodingVersion
InputStreamI::startEncapsulation()
{
    return _is->startReadEncaps();
}

void
InputStreamI::endEncapsulation()
{
    _is->endReadEncapsChecked();
}

EncodingVersion
InputStreamI::skipEncapsulation()
{
    return _is->skipEncaps();
}

EncodingVersion
InputStreamI::getEncoding() const
{
    return _is->getReadEncoding();
}

void
InputStreamI::readPendingObjects()
{
    _is->readPendingObjects();
}

InputStream::size_type
InputStreamI::pos()
{
    return _is->i - _is->b.begin();
}

void
InputStreamI::rewind()
{
    _is->clear();
    _is->i = _is->b.begin();
}

void
InputStreamI::skip(Int sz)
{
    _is->skip(sz);
}

void
InputStreamI::skipSize()
{
    _is->skipSize();
}

void
InputStreamI::closure(void* p)
{
    _closure = p;
}

void*
InputStreamI::closure() const
{
    return _closure;
}

void
InputStreamI::initialize(Instance* instance, const pair<const Byte*, const Byte*>& buf, const EncodingVersion& v,
                         bool copyData)
{
    if(copyData)
    {
        _is = new BasicStream(instance, v);
        _is->writeBlob(buf.first, buf.second - buf.first);
        _is->i = _is->b.begin();
    }
    else
    {
        _is = new BasicStream(instance, v, buf.first, buf.second);
    }
    _is->closure(this);
}

//
// OutputStreamI
//
OutputStreamI::OutputStreamI(const CommunicatorPtr& communicator) :
    _communicator(communicator), _own(true)
{
    Instance* instance = getInstance(communicator).get();
    _os = new BasicStream(instance, instance->defaultsAndOverrides()->defaultEncoding);
    _os->closure(this);
}

OutputStreamI::OutputStreamI(const CommunicatorPtr& communicator, const EncodingVersion& v) :
    _communicator(communicator), _own(true)
{
    Instance* instance = getInstance(communicator).get();
    _os = new BasicStream(instance, v);
    _os->closure(this);
}

OutputStreamI::OutputStreamI(const CommunicatorPtr& communicator, BasicStream* os) :
    _communicator(communicator), _os(os), _own(false)
{
    assert(os);
    _os->closure(this);
}

OutputStreamI::~OutputStreamI()
{
    if(_own)
    {
        delete _os;
    }
}

CommunicatorPtr
OutputStreamI::communicator() const
{
    return _communicator;
}

void
OutputStreamI::writeObject(const ObjectPtr& v)
{
    _os->write(v);
}

void
OutputStreamI::writeException(const UserException& v)
{
    _os->write(v);
}

void
OutputStreamI::writeProxy(const ObjectPrx& v)
{
    _os->write(v);
}

void
OutputStreamI::writeSize(Int sz)
{
    if(sz < 0)
    {
        throw MarshalException(__FILE__, __LINE__);
    }

    _os->writeSize(sz);
}

void
OutputStreamI::write(bool v)
{
    _os->write(v);
}

void
OutputStreamI::write(Byte v)
{
    _os->write(v);
}

void
OutputStreamI::write(Short v)
{
    _os->write(v);
}

void
OutputStreamI::write(Int v)
{
    _os->write(v);
}

void
OutputStreamI::write(Long v)
{
    _os->write(v);
}

void
OutputStreamI::write(Float v)
{
    _os->write(v);
}

void
OutputStreamI::write(Double v)
{
    _os->write(v);
}

void
OutputStreamI::write(const string& v, bool convert)
{
    _os->write(v, convert);
}

void
OutputStreamI::write(const char* vdata, size_t vsize, bool convert)
{
    _os->write(vdata, vsize, convert);
}

void
OutputStreamI::write(const char* v, bool convert)
{
    _os->write(v, strlen(v), convert);
}

void
OutputStreamI::write(const vector<string>& v, bool convert)
{
    if(v.size() == 0)
    {
        _os->writeSize(0);
    }
    else
    {
        _os->write(&v[0], &v[0] + v.size(), convert);
    }
}

void
OutputStreamI::write(const wstring& v)
{
    _os->write(v);
}

void
OutputStreamI::write(const vector<bool>& v)
{
    _os->write(v);
}

void
OutputStreamI::write(const bool* begin, const bool* end)
{
    _os->write(begin, end);
}

void
OutputStreamI::write(const Byte* begin, const Byte* end)
{
    _os->write(begin, end);
}

void
OutputStreamI::write(const Short* begin, const Short* end)
{
    _os->write(begin, end);
}

void
OutputStreamI::write(const Int* begin, const Int* end)
{
    _os->write(begin, end);
}

void
OutputStreamI::write(const Long* begin, const Long* end)
{
    _os->write(begin, end);
}

void
OutputStreamI::write(const Float* begin, const Float* end)
{
    _os->write(begin, end);
}

void
OutputStreamI::write(const Double* begin, const Double* end)
{
    _os->write(begin, end);
}

bool
OutputStreamI::writeOptional(Int tag, OptionalFormat format)
{
    return _os->writeOpt(tag, format);
}

void
OutputStreamI::startObject(const SlicedDataPtr& slicedData)
{
    _os->startWriteObject(slicedData);
}

void
OutputStreamI::endObject()
{
    _os->endWriteObject();
}

void
OutputStreamI::startException(const SlicedDataPtr& slicedData)
{
    _os->startWriteException(slicedData);
}

void
OutputStreamI::endException()
{
    _os->endWriteException();
}

void
OutputStreamI::startSlice(const string& typeId, int compactId, bool lastSlice)
{
    _os->startWriteSlice(typeId, compactId, lastSlice);
}

void
OutputStreamI::endSlice()
{
    _os->endWriteSlice();
}

void
OutputStreamI::startEncapsulation(const EncodingVersion& version, FormatType format)
{
    _os->startWriteEncaps(version, format);
}

void
OutputStreamI::startEncapsulation()
{
    _os->startWriteEncaps();
}

void
OutputStreamI::endEncapsulation()
{
    _os->endWriteEncapsChecked();
}

EncodingVersion
OutputStreamI::getEncoding() const
{
    return _os->getWriteEncoding();
}

void
OutputStreamI::writePendingObjects()
{
    _os->writePendingObjects();
}

void
OutputStreamI::finished(vector<Byte>& bytes)
{
    vector<Byte>(_os->b.begin(), _os->b.end()).swap(bytes);
}

pair<const Byte*, const Byte*>
OutputStreamI::finished()
{
    if(_os->b.empty())
    {
        return pair<const Byte*, const Byte*>(reinterpret_cast<Ice::Byte*>(0), reinterpret_cast<Ice::Byte*>(0));
    }
    else
    {
        return pair<const Byte*, const Byte*>(&_os->b[0], &_os->b[0] + _os->b.size());
    }
}

void
OutputStreamI::reset(bool clearBuffer)
{
    _os->clear();

    if(clearBuffer)
    {
        _os->b.clear();
    }
    else
    {
        _os->b.reset();
    }

    _os->i = _os->b.begin();
}

OutputStream::size_type
OutputStreamI::pos()
{
    return _os->b.size();
}

void
OutputStreamI::rewrite(Int sz, size_type p)
{
    _os->rewrite(sz, p);
}

OutputStream::size_type
OutputStreamI::startSize()
{
    return _os->startSize();
}

void
OutputStreamI::endSize(OutputStream::size_type pos)
{
    _os->endSize(pos);
}

//
// ObjectReader
//
void
ObjectReader::__write(BasicStream*) const
{
    assert(false);
}

void
ObjectReader::__read(BasicStream* is)
{
    InputStreamI* stream = reinterpret_cast<InputStreamI*>(is->closure());
    assert(stream);
    read(stream);
}

void
ObjectReader::__write(const OutputStreamPtr&) const
{
    assert(false);
}

void
ObjectReader::__read(const InputStreamPtr&)
{
    assert(false);
}

//
// ObjectWriter
//
void
ObjectWriter::__write(BasicStream* os) const
{
    OutputStreamI* stream = reinterpret_cast<OutputStreamI*>(os->closure());
    assert(stream);
    write(stream);
}

void
ObjectWriter::__read(BasicStream*)
{
    assert(false);
}

void
ObjectWriter::__write(const OutputStreamPtr&) const
{
    assert(false);
}

void
ObjectWriter::__read(const InputStreamPtr&)
{
    assert(false);
}

//
// UserExceptionWriter
//
UserExceptionWriter::UserExceptionWriter(const CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

UserExceptionWriter::~UserExceptionWriter() throw()
{
}

void
UserExceptionWriter::__write(BasicStream* os) const
{
    OutputStreamI* stream = reinterpret_cast<OutputStreamI*>(os->closure());
    if(!stream)
    {
        //
        // Required for IcePy usage
        //
        stream = new OutputStreamI(_communicator, os);
    }
    write(stream);
}

void
UserExceptionWriter::__read(BasicStream*)
{
    assert(false);
}
