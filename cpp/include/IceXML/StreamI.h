// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_XML_STREAM_I_H
#define ICE_XML_STREAM_I_H

#include <Ice/Stream.h>
#include <Ice/CommunicatorF.h>
#include <IceUtil/OutputUtil.h>

#include <map>

#ifndef ICE_XML_API 
#   ifdef ICE_XML_API_EXPORTS
#       define ICE_XML_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_XML_API ICE_DECLSPEC_IMPORT
#   endif
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

    virtual ::Ice::Int startReadDictionary(const ::std::string&);

    virtual void endReadDictionary();

    virtual void startReadDictionaryElement();

    virtual void endReadDictionaryElement();

    virtual void startWriteSequence(const ::std::string&, ::Ice::Int);

    virtual void endWriteSequence();

    virtual void startWriteSequenceElement();

    virtual void endWriteSequenceElement();

    virtual ::Ice::Int startReadSequence(const ::std::string&);

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

    virtual ::Ice::Int readEnum(const ::std::string&, const ::Ice::StringSeq&);

    virtual void writeByte(const ::std::string&, ::Ice::Byte);

    virtual void writeByteSeq(const ::std::string&, const ::Ice::ByteSeq&);

    virtual ::Ice::Byte readByte(const ::std::string&);

    virtual ::Ice::ByteSeq readByteSeq(const ::std::string&);

    virtual void writeBool(const ::std::string&, bool);

    virtual void writeBoolSeq(const ::std::string&, const ::Ice::BoolSeq&);

    virtual bool readBool(const ::std::string&);

    virtual ::Ice::BoolSeq readBoolSeq(const ::std::string&);

    virtual void writeShort(const ::std::string&, ::Ice::Short);

    virtual void writeShortSeq(const ::std::string&, const ::Ice::ShortSeq&);

    virtual ::Ice::Short readShort(const ::std::string&);

    virtual ::Ice::ShortSeq readShortSeq(const ::std::string&);

    virtual void writeInt(const ::std::string&, ::Ice::Int);

    virtual void writeIntSeq(const ::std::string&, const ::Ice::IntSeq&);

    virtual ::Ice::Int readInt(const ::std::string&);

    virtual ::Ice::IntSeq readIntSeq(const ::std::string&);

    virtual void writeLong(const ::std::string&, ::Ice::Long);

    virtual void writeLongSeq(const ::std::string&, const ::Ice::LongSeq&);

    virtual ::Ice::Long readLong(const ::std::string&);

    virtual ::Ice::LongSeq readLongSeq(const ::std::string&);

    virtual void writeFloat(const ::std::string&, ::Ice::Float);

    virtual void writeFloatSeq(const ::std::string&, const ::Ice::FloatSeq&);

    virtual ::Ice::Float readFloat(const ::std::string&);

    virtual ::Ice::FloatSeq readFloatSeq(const ::std::string&);

    virtual void writeDouble(const ::std::string&, ::Ice::Double);

    virtual void writeDoubleSeq(const ::std::string&, const ::Ice::DoubleSeq&);

    virtual ::Ice::Double readDouble(const ::std::string&);

    virtual ::Ice::DoubleSeq readDoubleSeq(const ::std::string&);

    virtual void writeString(const ::std::string&, const ::std::string&);

    virtual void writeStringSeq(const ::std::string&, const ::Ice::StringSeq&);

    virtual ::std::string readString(const ::std::string&);

    virtual ::Ice::StringSeq readStringSeq(const ::std::string&);

    virtual void writeProxy(const ::std::string&, const ::Ice::ObjectPrx&);

    virtual ::Ice::ObjectPrx readProxy(const ::std::string&);

    virtual void writeObject(const ::std::string&, const ::Ice::ObjectPtr&);

    virtual ::Ice::ObjectPtr readObject(const ::std::string&, const ::std::string&, const ::Ice::ObjectFactoryPtr&);

    virtual void marshalFacets(bool setting);

private:

    void startWrite(const std::string&);
    void endWrite();

    void startRead(const std::string&);
    void endRead();

    void dumpUnwrittenObjects();
    void writeObjectData(const ::std::string&, const ::std::string&, const ::Ice::ObjectPtr&);
    void readAttributes(::std::string&, ::std::string&, ::std::string&);
    ::Ice::Int readLength();

    ::std::string getWritePrefix() const;
    ::std::string getReadPrefix() const;
    static ::std::string getPrefix(const ::std::string&);

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
    ::IceUtil::XMLOutput _os;

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
    bool _dump;

    bool _marshalFacets;
};

} // End namespace IceXML

#endif
