// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_BASIC_STREAM_H
#define ICEE_BASIC_STREAM_H

#include <IceE/ProxyF.h>

#include <IceE/Buffer.h>
#include <IceE/Protocol.h>

#ifdef ICEE_HAS_WSTRING
#  include <IceE/StringConverter.h>
#endif

namespace Ice
{

class UserException;
ICE_API void throwUnmarshalOutOfBoundsException(const char*, int);
ICE_API void throwNegativeSizeException(const char*, int);
ICE_API void throwMemoryLimitException(const char*, int);
ICE_API void throwUnsupportedEncodingException(const char*, int, ::Ice::Int, ::Ice::Int, ::Ice::Int, ::Ice::Int);

}

namespace IceInternal
{

class Instance;

class BasicStream : public Buffer
{
public:

#ifdef ICEE_HAS_WSTRING
    class StreamUTF8BufferI : public Ice::UTF8Buffer
    {
    public:

        StreamUTF8BufferI(BasicStream& stream) :
            _stream(stream)
        {
        }

        Ice::Byte*
        getMoreBytes(size_t howMany, Ice::Byte* firstUnused)
        {
            assert(howMany > 0);

            if(firstUnused != 0)
            {
                //
                // Return unused bytes
                //
                _stream.b.resize(firstUnused - _stream.b.begin());
            }

            //
            // Index of first unused byte
            //
            Container::size_type pos = _stream.b.size();

            //
            // Since resize may reallocate the buffer, when firstUnused != 0, the
            // return value can be != firstUnused
            //
            _stream.resize(pos + howMany);

            return &_stream.b[pos];
        }

    private:

        BasicStream& _stream;
    };
#endif

    BasicStream(Instance* instance, int messageSizeMax,
#ifdef ICEE_HAS_WSTRING
                const Ice::StringConverterPtr& stringConverter, const Ice::WstringConverterPtr& wstringConverter,
#endif
                bool unlimited = false) :
	Buffer(messageSizeMax),
	_instance(instance),
	_currentReadEncaps(0),
	_currentWriteEncaps(0),
	_messageSizeMax(messageSizeMax),
        _unlimited(unlimited),
#ifdef ICEE_HAS_WSTRING
        _stringConverter(stringConverter),
        _wstringConverter(wstringConverter),
#endif
	_seqDataStack(0)
    {
	// Inlined for performance reasons.
    }

    ~BasicStream()
    {
	// Inlined for performance reasons.

	if(_currentReadEncaps != &_preAllocatedReadEncaps ||
	   _currentWriteEncaps != &_preAllocatedWriteEncaps ||
	   _seqDataStack)
	{
	    clear(); // Not inlined.
	}
    }

    ICE_API void clear();

    //
    // Must return Instance*, because we don't hold an InstancePtr for
    // optimization reasons (see comments below).
    //
    Instance* instance() const { return _instance; } // Inlined for performance reasons.

    ICE_API void swap(BasicStream&);

    void resize(Container::size_type sz)
    {
	if(!_unlimited && sz > _messageSizeMax)
	{
	    Ice::throwMemoryLimitException(__FILE__, __LINE__);
	}
	
	b.resize(sz);
    }

    void reset() // Inlined for performance reasons.
    {
        b.reset();
	i = b.begin();
    }

    ICE_API void startSeq(int, int);
    void checkSeq()
    {
	checkSeq(static_cast<int>(b.end() - i));
    }
    void checkSeq(int bytesLeft)
    {
	//
	// Check, given the number of elements requested for this sequence,
	// that this sequence, plus the sum of the sizes of the remaining
	// number of elements of all enclosing sequences, would still fit
	// within the message.
	//
	int size = 0;
	SeqData* sd = _seqDataStack;
	do
	{
	    size += (sd->numElements - 1) * sd->minSize;
	    sd = sd->previous;
	}
	while(sd);
	
	if(size > bytesLeft)
	{
	    Ice::throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
	}
    }
    ICE_API void checkFixedSeq(int, int); // For sequences of fixed-size types.
    void endElement()
    {
	assert(_seqDataStack);
	--_seqDataStack->numElements;
    }
    ICE_API void endSeq(int);

    void startWriteEncaps()
    {
	WriteEncaps* oldEncaps = _currentWriteEncaps;
	if(!oldEncaps) // First allocated encaps?
	{
	    _currentWriteEncaps = &_preAllocatedWriteEncaps;
	}
	else
	{
	    _currentWriteEncaps = new WriteEncaps();
	    _currentWriteEncaps->previous = oldEncaps;
	}
	_currentWriteEncaps->start = b.size();

	write(Ice::Int(0)); // Placeholder for the encapsulation length.
	write(encodingMajor);
	write(encodingMinor);
    }
    void endWriteEncaps()
    {
	assert(_currentWriteEncaps);
	Container::size_type start = _currentWriteEncaps->start;
	Ice::Int sz = static_cast<Ice::Int>(b.size() - start); // Size includes size and version.
	Ice::Byte* dest = &(*(b.begin() + start));

#ifdef ICE_BIG_ENDIAN
	const Ice::Byte* src = reinterpret_cast<const Ice::Byte*>(&sz) + sizeof(Ice::Int) - 1;
	*dest++ = *src--;
	*dest++ = *src--;
	*dest++ = *src--;
	*dest = *src;
#else
	const Ice::Byte* src = reinterpret_cast<const Ice::Byte*>(&sz);
	*dest++ = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
	*dest = *src;
#endif

	WriteEncaps* oldEncaps = _currentWriteEncaps;
	_currentWriteEncaps = _currentWriteEncaps->previous;
	if(oldEncaps == &_preAllocatedWriteEncaps)
	{
	    oldEncaps->reset();
	}
	else
	{
	    delete oldEncaps;
	}
    }

    void startReadEncaps()
    {
	ReadEncaps* oldEncaps = _currentReadEncaps;
	if(!oldEncaps) // First allocated encaps?
	{
	    _currentReadEncaps = &_preAllocatedReadEncaps;
	}
	else
	{
	    _currentReadEncaps = new ReadEncaps();
	    _currentReadEncaps->previous = oldEncaps;
	}
	_currentReadEncaps->start = i - b.begin();
	
	//
	// I don't use readSize() and writeSize() for encapsulations,
	// because when creating an encapsulation, I must know in advance
	// how many bytes the size information will require in the data
	// stream. If I use an Int, it is always 4 bytes. For
	// readSize()/writeSize(), it could be 1 or 5 bytes.
	//
	Ice::Int sz;
	read(sz);
	if(sz < 0)
	{
	    Ice::throwNegativeSizeException(__FILE__, __LINE__);
	}
	if(i - sizeof(Ice::Int) + sz > b.end())
	{
	    Ice::throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
	}
	_currentReadEncaps->sz = sz;
	
	Ice::Byte eMajor;
	Ice::Byte eMinor;
	read(eMajor);
	read(eMinor);
	if(eMajor != encodingMajor
	   || static_cast<unsigned char>(eMinor) > static_cast<unsigned char>(encodingMinor))
	{
	    Ice::throwUnsupportedEncodingException(__FILE__, __LINE__, eMajor, eMinor, encodingMajor, encodingMinor);
	}
	_currentReadEncaps->encodingMajor = eMajor;
	_currentReadEncaps->encodingMinor = eMinor;
    }
    void endReadEncaps()
    {
	assert(_currentReadEncaps);
	Container::size_type start = _currentReadEncaps->start;
	Ice::Int sz = _currentReadEncaps->sz;
	i = b.begin() + start + sz;

	ReadEncaps* oldEncaps = _currentReadEncaps;
	_currentReadEncaps = _currentReadEncaps->previous;
	if(oldEncaps == &_preAllocatedReadEncaps)
	{
	    oldEncaps->reset();
	}
	else
	{
	    delete oldEncaps;
	}
    }
    ICE_API Ice::Int getReadEncapsSize();
    ICE_API void skipEncaps();

    ICE_API void startWriteSlice();
    ICE_API void endWriteSlice();

    ICE_API void startReadSlice();
    ICE_API void endReadSlice();
    ICE_API void skipSlice();

    void writeSize(Ice::Int v) // Inlined for performance reasons.
    {
	assert(v >= 0);
	if(v > 254)
	{
	    write(Ice::Byte(255));
	    write(v);
	}
	else
	{
	    write(static_cast<Ice::Byte>(v));
	}
    }

    void rewriteSize(Ice::Int v, Container::iterator dest)
    {
        assert(v >= 0);
        if(v > 254)
        {
            *dest++ = Ice::Byte(255);
#ifdef ICE_BIG_ENDIAN
            const Ice::Byte* src = reinterpret_cast<const Ice::Byte*>(&v) + sizeof(Ice::Int) - 1;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest = *src;
#else
            const Ice::Byte* src = reinterpret_cast<const Ice::Byte*>(&v);
            *dest++ = *src++;
            *dest++ = *src++;
            *dest++ = *src++;
            *dest = *src;
#endif
        }
        else
        {
            *dest = static_cast<Ice::Byte>(v);
        }
    }

    void
    readSize(Ice::Int& v) // Inlined for performance reasons.
    {
	Ice::Byte byte;
	read(byte);
	unsigned val = static_cast<unsigned char>(byte);
	if(val == 255)
	{
	    read(v);
	    if(v < 0)
	    {
		Ice::throwNegativeSizeException(__FILE__, __LINE__);
	    }
	}
	else
	{
	    v = static_cast<Ice::Int>(static_cast<unsigned char>(byte));
	}
    }

    ICE_API void writeBlob(const std::vector<Ice::Byte>&);
    ICE_API void readBlob(std::vector<Ice::Byte>&, Ice::Int);

    void writeBlob(const Ice::Byte* v, Container::size_type sz)
    {
	if(sz > 0)
	{
	    Container::size_type pos = b.size();
	    resize(pos + sz);
	    memcpy(&b[pos], &v[0], sz);
	}
    }

    void readBlob(const Ice::Byte*& v, Container::size_type sz)
    {
	if(sz > 0)
	{
	    v = i;
	    if(static_cast<Container::size_type>(b.end() - i) < sz)
	    {
		Ice::throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
	    }
	    i += sz;
	}
	else
	{
	    v = i;
	}
    }

    void write(Ice::Byte v) // Inlined for performance reasons.
    {
	b.push_back(v);
    }
    void read(Ice::Byte& v) // Inlined for performance reasons.
    {
	if(i >= b.end())
	{
	    Ice::throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
	}
	v = *i++;
    }

    ICE_API void write(const Ice::Byte*, const Ice::Byte*);
    ICE_API void read(std::pair<const Ice::Byte*, const Ice::Byte*>&);

    void write(bool v) // Inlined for performance reasons.
    {
	b.push_back(static_cast<Ice::Byte>(v));
    }
    ICE_API void write(const std::vector<bool>&);
    ICE_API void write(const bool*, const bool*);
    void read(bool& v) // Inlined for performance reasons.
    {
	if(i >= b.end())
	{
	    Ice::throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
	}
	v = *i++;
    }
    ICE_API void read(std::vector<bool>&);
    ICE_API bool* read(std::pair<const bool*, const bool*>&);

    ICE_API void write(Ice::Short);
    ICE_API void read(Ice::Short&);
    ICE_API void write(const Ice::Short*, const Ice::Short*);
    ICE_API void read(std::vector<Ice::Short>&);
    ICE_API Ice::Short* read(std::pair<const Ice::Short*, const Ice::Short*>&);

    void
    write(Ice::Int v) // Inlined for performance reasons.
    {
	Container::size_type pos = b.size();
	resize(pos + sizeof(Ice::Int));
	Ice::Byte* dest = &b[pos];
#ifdef ICE_BIG_ENDIAN
	const Ice::Byte* src = reinterpret_cast<const Ice::Byte*>(&v) + sizeof(Ice::Int) - 1;
	*dest++ = *src--;
	*dest++ = *src--;
	*dest++ = *src--;
	*dest = *src;
#else
	const Ice::Byte* src = reinterpret_cast<const Ice::Byte*>(&v);
	*dest++ = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
	*dest = *src;
#endif
    }

    void read(Ice::Int& v) // Inlined for performance reasons.
    {
	if(b.end() - i < static_cast<int>(sizeof(Ice::Int)))
	{
	    Ice::throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
	}
	const Ice::Byte* src = &(*i);
	i += sizeof(Ice::Int);
#ifdef ICE_BIG_ENDIAN
	Ice::Byte* dest = reinterpret_cast<Ice::Byte*>(&v) + sizeof(Ice::Int) - 1;
	*dest-- = *src++;
	*dest-- = *src++;
	*dest-- = *src++;
	*dest = *src;
#else
	Ice::Byte* dest = reinterpret_cast<Ice::Byte*>(&v);
	*dest++ = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
	*dest = *src;
#endif
    }

    ICE_API void write(const Ice::Int*, const Ice::Int*);
    ICE_API void read(std::vector<Ice::Int>&);
    ICE_API Ice::Int* read(std::pair<const Ice::Int*, const Ice::Int*>&);

    ICE_API void write(Ice::Long);
    ICE_API void read(Ice::Long&);
    ICE_API void write(const Ice::Long*, const Ice::Long*);
    ICE_API void read(std::vector<Ice::Long>&);
    ICE_API Ice::Long* read(std::pair<const Ice::Long*, const Ice::Long*>&);

    ICE_API void write(Ice::Float);
    ICE_API void read(Ice::Float&);
    ICE_API void write(const Ice::Float*, const Ice::Float*);
    ICE_API void read(std::vector<Ice::Float>&);
    ICE_API Ice::Float* read(std::pair<const Ice::Float*, const Ice::Float*>&);

    ICE_API void write(Ice::Double);
    ICE_API void read(Ice::Double&);
    ICE_API void write(const Ice::Double*, const Ice::Double*);
    ICE_API void read(std::vector<Ice::Double>&);
    ICE_API Ice::Double* read(std::pair<const Ice::Double*, const Ice::Double*>&);

    //
    // NOTE: This function is not implemented. It is declared here to
    // catch programming errors that assume a call such as write("")
    // will invoke write(const std::string&), when in fact the compiler
    // will silently select a different overloading. A link error is the
    // intended result.
    //
    ICE_API void write(const char*);

#ifdef ICEE_HAS_WSTRING
    ICE_API void writeConverted(const std::string& v);
#endif
    void write(const std::string& v, bool convert = true)
    {
        Ice::Int sz = static_cast<Ice::Int>(v.size());
#ifdef ICEE_HAS_WSTRING
        if(convert && sz > 0 && _stringConverter != 0)
        {
            writeConverted(v);
        }
        else
#endif
        {
            writeSize(sz);
            if(sz > 0)
            {
                Container::size_type pos = b.size();
                resize(pos + sz);
                memcpy(&b[pos], v.data(), sz);
            }
        }
    }
    ICE_API void write(const std::string*, const std::string*, bool = true);
    void read(std::string& v, bool convert = true)
    {
	Ice::Int sz;
	readSize(sz);
	if(sz > 0)
	{
	    if(b.end() - i < sz)
	    {
		Ice::throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
	    }
#ifdef ICEE_HAS_WSTRING
            if(convert && _stringConverter != 0)
            {
                _stringConverter->fromUTF8(i, i + sz, v);
            }
            else
#endif
            {
                std::string(reinterpret_cast<const char*>(&*i), reinterpret_cast<const char*>(&*i) + sz).swap(v);
//              v.assign(reinterpret_cast<const char*>(&(*i)), sz);
            }
	    i += sz;
	}
	else
	{
	    v.clear();
	}
    }
    ICE_API void read(std::vector<std::string>&, bool = true);

#ifdef ICEE_HAS_WSTRING
    ICE_API void write(const std::wstring& v);
    ICE_API void write(const std::wstring*, const std::wstring*);
    void read(std::wstring& v)
    {
        Ice::Int sz;
        readSize(sz);
        if(sz > 0)
        {
            if(b.end() - i < sz)
            {
                Ice::throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
            }

            _wstringConverter->fromUTF8(i, i + sz, v);
            i += sz;
        }
        else
        {
            v.clear();
        }
    }
    ICE_API void read(std::vector<std::wstring>&);
#endif

    ICE_API void write(const Ice::ObjectPrx&);
    ICE_API void read(Ice::ObjectPrx&);

    ICE_API void write(const Ice::UserException&);
    ICE_API void throwException();

private:

    //
    // Optimization. The instance may not be deleted while a
    // stack-allocated BasicStream still holds it.
    //
    Instance* _instance;

    class ReadEncaps : private ::IceUtil::noncopyable
    {
    public:

	ReadEncaps() : previous(0) { } // Inlined for performance reasons.
	~ReadEncaps() { } // Inlined for performance reasons.

	void reset() { previous = 0; } // Inlined for performance reasons.
	ICE_API void swap(ReadEncaps&);

	Container::size_type start;
	Ice::Int sz;

	Ice::Byte encodingMajor;
	Ice::Byte encodingMinor;

	ReadEncaps* previous;
    };

    class WriteEncaps : private ::IceUtil::noncopyable
    {
    public:

	WriteEncaps() : writeIndex(0), previous(0) { } // Inlined for performance reasons.
	~WriteEncaps() { } // Inlined for performance reasons.

	void reset() { writeIndex = 0; previous = 0; } // Inlined for performance reasons.
	ICE_API void swap(WriteEncaps&);

	Container::size_type start;

	Ice::Int writeIndex;

	WriteEncaps* previous;
    };

    ReadEncaps* _currentReadEncaps;
    WriteEncaps* _currentWriteEncaps;

    ReadEncaps _preAllocatedReadEncaps;
    WriteEncaps _preAllocatedWriteEncaps;

    Container::size_type _readSlice;
    Container::size_type _writeSlice;

    const Container::size_type _messageSizeMax;
    bool _unlimited;

#ifdef ICEE_HAS_WSTRING
    const Ice::StringConverterPtr& _stringConverter;
    const Ice::WstringConverterPtr& _wstringConverter;
#endif

    struct SeqData
    {
	SeqData(int, int);
	int numElements;
	int minSize;
	SeqData* previous;
    };
    SeqData* _seqDataStack;
};

} // End namespace IceInternal

#endif
