// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_XML_STREAM_I_H
#define ICE_XML_STREAM_I_H

#include <Ice/Stream.h>
#include <Ice/CommunicatorF.h>
#include <IceXML/Output.h>

#include <map>

#ifdef WIN32
#   ifdef ICE_XML_API_EXPORTS
#       define ICE_XML_API __declspec(dllexport)
#   else
#       define ICE_XML_API __declspec(dllimport)
#   endif
#else
#   define ICE_XML_API /**/
#endif

namespace IceXML
{

//
// This is to reduce dependencies on Xerces.
//
struct StreamInputImpl;

class ICE_XML_API StreamI : public ::Ice::Stream
{
    StreamI();
    StreamI(const StreamI&);
    void operator=(const StreamI&);

public:

    StreamI(const ::Ice::CommunicatorPtr&, std::ostream&);

    StreamI(const ::Ice::CommunicatorPtr&, std::istream&, bool = true);

    ~StreamI();

    virtual void startWriteDictionary(const ::std::string&, ::Ice::Int);

    virtual void endWriteDictionary();

    virtual void startWriteDictionaryElement();

    virtual void endWriteDictionaryElement();

    virtual void startReadDictionary(const ::std::string&, ::Ice::Int&);

    virtual void endReadDictionary();

    virtual void startReadDictionaryElement();

    virtual void endReadDictionaryElement();

    virtual void startWriteSequence(const ::std::string&, ::Ice::Int);

    virtual void endWriteSequence();

    virtual void startWriteSequenceElement();

    virtual void endWriteSequenceElement();

    virtual void startReadSequence(const ::std::string&, ::Ice::Int&);

    virtual void endReadSequence();

    virtual void startReadSequenceElement();

    virtual void endReadSequenceElement();

    virtual void startWriteStruct(const ::std::string&);

    virtual void endWriteStruct();

    virtual void startReadStruct(const ::std::string&);

    virtual void endReadStruct();

    virtual void startWriteException(const ::std::string&);

    virtual void endWriteException();

    virtual void startReadException(const ::std::string&);

    virtual void endReadException();

    virtual void writeEnum(const ::std::string&, const ::Ice::StringSeq&, ::Ice::Int);

    virtual void readEnum(const ::std::string&, const ::Ice::StringSeq&, ::Ice::Int&);

    virtual void writeByte(const ::std::string&, ::Ice::Byte);

    virtual void writeByteSeq(const ::std::string&, const ::Ice::ByteSeq&);

    virtual void readByte(const ::std::string&, ::Ice::Byte&);

    virtual void readByteSeq(const ::std::string&, ::Ice::ByteSeq&);

    virtual void writeBool(const ::std::string&, bool);

    virtual void writeBoolSeq(const ::std::string&, const ::Ice::BoolSeq&);

    virtual void readBool(const ::std::string&, bool&);

    virtual void readBoolSeq(const ::std::string&, ::Ice::BoolSeq&);

    virtual void writeShort(const ::std::string&, ::Ice::Short);

    virtual void writeShortSeq(const ::std::string&, const ::Ice::ShortSeq&);

    virtual void readShort(const ::std::string&, ::Ice::Short&);

    virtual void readShortSeq(const ::std::string&, ::Ice::ShortSeq&);

    virtual void writeInt(const ::std::string&, ::Ice::Int);

    virtual void writeIntSeq(const ::std::string&, const ::Ice::IntSeq&);

    virtual void readInt(const ::std::string&, ::Ice::Int&);

    virtual void readIntSeq(const ::std::string&, ::Ice::IntSeq&);

    virtual void writeLong(const ::std::string&, ::Ice::Long);

    virtual void writeLongSeq(const ::std::string&, const ::Ice::LongSeq&);

    virtual void readLong(const ::std::string&, ::Ice::Long&);

    virtual void readLongSeq(const ::std::string&, ::Ice::LongSeq&);

    virtual void writeFloat(const ::std::string&, ::Ice::Float);

    virtual void writeFloatSeq(const ::std::string&, const ::Ice::FloatSeq&);

    virtual void readFloat(const ::std::string&, ::Ice::Float&);

    virtual void readFloatSeq(const ::std::string&, ::Ice::FloatSeq&);

    virtual void writeDouble(const ::std::string&, ::Ice::Double);

    virtual void writeDoubleSeq(const ::std::string&, const ::Ice::DoubleSeq&);

    virtual void readDouble(const ::std::string&, ::Ice::Double&);

    virtual void readDoubleSeq(const ::std::string&, ::Ice::DoubleSeq&);

    virtual void writeString(const ::std::string&, const ::std::string&);

    virtual void writeStringSeq(const ::std::string&, const ::Ice::StringSeq&);

    virtual void readString(const ::std::string&, ::std::string&);

    virtual void readStringSeq(const ::std::string&, ::Ice::StringSeq&);

    virtual void writeProxy(const ::std::string&, const ::Ice::ObjectPrx&);

    virtual void readProxy(const ::std::string&, ::Ice::ObjectPrx&);

    virtual void writeObject(const ::std::string&, const ::Ice::ObjectPtr&);

    virtual void readObject(const ::std::string&, const ::std::string&, const ::Ice::ObjectFactoryPtr&,
			    ::Ice::ObjectPtr&);

private:

    void startWrite(const std::string&);
    void endWrite();

    void startRead(const std::string&);
    void endRead();

    void dumpUnwrittenObjects();
    void writeObjectData(const ::std::string&, const ::std::string&, const ::Ice::ObjectPtr&);
    void readAttributes(::std::string&, ::std::string&, ::std::string&);
    ::Ice::Int readLength();

    ::Ice::CommunicatorPtr _communicator;

    //
    // Name of the sequence element name
    //
    static const ::std::string seqElementName;

    //
    // For reading.
    //
    ::std::string _content;
    StreamInputImpl* _input;
    
    //
    // For writing.
    //

    //
    // XML Output stream.
    //
    Output _os;

    //
    // Current document level.
    //
    int _level;

    //
    // Next id for marshaling objects.
    //
    int _nextId;
    struct ObjectInfo
    {
	ObjectInfo(const ::std::string& i, bool w) :
	    id(i), written(w) { }
	::std::string id; // The id of the associated object
	bool written; // Has the object been written?
    };
    ::std::map<Ice::ObjectPtr, ObjectInfo> _objects;
};

} // End namespace IceXML

#endif
