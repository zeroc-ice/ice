// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Object.h>
#include <Ice/Communicator.h>
#include <Ice/Logger.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectFactory.h>

#include <IceXML/XMLStream.h>

//
// For input streaming
//
#include <framework/MemBufInputSource.hpp>
#include <util/PlatformUtils.hpp>
#include <util/XMLString.hpp>
#include <util/XMLUniDefs.hpp>
#include <framework/XMLFormatter.hpp>
#include <util/TranscodingException.hpp>

#include <dom/DOM_DOMException.hpp>
#include <parsers/DOMParser.hpp>

#include <dom/DOM_Node.hpp>
#include <dom/DOM_NamedNodeMap.hpp>

#include <sax/ErrorHandler.hpp>
#include <sax/SAXParseException.hpp>
#include <dom/DOMString.hpp>

using namespace std;
using namespace IceXML;

//
// Utilities to make the usage of xerces easier.
//
static string
toString(const DOMString& s)
{
    char* t = s.transcode();
    string r(t);
    delete[] t;
    return r;
}

static string
toString(const XMLCh* s)
{
    char* t = XMLString::transcode(s);
    string r(t);
    delete[] t;
    return r;
}

//
// Error handling class for xerces.
//
class DOMTreeErrorReporter : public ErrorHandler
{
public:

    DOMTreeErrorReporter(const ::Ice::LoggerPtr logger) :
       _logger(logger),
       _sawErrors(false)
    {
    }

    virtual void
    warning(const SAXParseException& ex)
    {
	ostringstream os;
	os << "xerces:  file: \"" << toString(ex.getSystemId())
	   << "\", line:" << ex.getLineNumber()
	   << ", column: " << ex.getColumnNumber()
	   << "\n,essage: " << toString(ex.getMessage()) << endl;
	_logger->warning(os.str());
    }
    
    virtual void
    error(const SAXParseException& ex)
    {
	_sawErrors = true;
	ostringstream os;
	os << "xerces: file: \"" << toString(ex.getSystemId())
	     << "\", line: " << ex.getLineNumber()
	     << ", column: " << ex.getColumnNumber()
	     << "\nmessage: " << toString(ex.getMessage()) << endl;
	_logger->error(os.str());
    }

    virtual void
    fatalError(const SAXParseException& ex)
    {
	_sawErrors = true;
	ostringstream os;
	os << "xerces: file: \"" << toString(ex.getSystemId())
	     << "\", line: " << ex.getLineNumber()
	     << ", column: " << ex.getColumnNumber()
	     << "\nmessage: " << toString(ex.getMessage()) << endl;
	_logger->error(os.str());
    }

    virtual void
    resetErrors()
    {
    }

    bool
    getSawErrors() const
    {
	return _sawErrors;
    }

private:

    ::Ice::LoggerPtr _logger;
    bool _sawErrors;
};

namespace IceXML
{

//
// This is used to reduce the external dependencies on the xerces
// headers.
//
struct XMLStreamInputImpl
{
    XMLStreamInputImpl() :
	source(0),
	parser(0),
	errReporter(0)
    {
    }
    ~XMLStreamInputImpl()
    {
	delete parser;
	delete errReporter;
	delete source;
    }
    
    InputSource* source;
    DOMParser* parser;
    DOMTreeErrorReporter* errReporter;
    DOM_Node current;

    list<DOM_Node> nodeStack;

    map<string, ::Ice::ObjectPtr> readObjects;
};

} // End namespace IceXML

//
// A constant string which is used as the name of collection elements.
//
const string XMLStream::seqElementName("e");

XMLStream::XMLStream(const ::Ice::CommunicatorPtr& communicator, std::ostream& os) :
    _communicator(communicator),
    _input(0),
    _os(os),
    _nextId(0)
{
    ::Ice::LoggerPtr logger = communicator->getLogger();
    //
    // Initialize the XML4C2 system.  TODO: This really needs to be
    // dealt with as global system state.
    //
    try
    {
         XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& ex)
    {
	string err = "xerces: initialize failed: ";
	err += toString(ex.getMessage());
	logger->error(err);
	throw ::Ice::SystemException(__FILE__, __LINE__);
    }
}


XMLStream::XMLStream(const ::Ice::CommunicatorPtr& communicator, std::istream& is) :
    _communicator(communicator),
    _nextId(0)
{
    ::Ice::LoggerPtr logger = communicator->getLogger();

    //
    // Initialize the XML4C2 system.  TODO: This really needs to be
    // dealt with as global system state.
    //
    try
    {
         XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& ex)
    {
	string err = "xerces: initialize failed: ";
	err += toString(ex.getMessage());
	logger->error(err);
	throw ::Ice::SystemException(__FILE__, __LINE__);
    }

    //
    // Read the contents of the stream into memory.
    //
    while (!is.eof())
    {
	char buf[1000];
	is.read(buf, sizeof(buf));
	_content.append(buf, is.gcount());
    }
    
    _input = new XMLStreamInputImpl();
    _input->source = new MemBufInputSource((const XMLByte*)_content.data(), _content.size(), "inputsource");

    //
    //  Create our parser, then attach an error handler to the parser.
    //  The parser will call back to methods of the ErrorHandler if it
    //  discovers errors during the course of parsing the XML
    //  document.
    //
    _input->parser = new DOMParser;
    _input->parser->setValidationScheme(DOMParser::Val_Auto);
    _input->parser->setDoNamespaces(true);
    _input->parser->setDoSchema(true);
    //_input->parser->setValidationSchemaFullChecking(true);

    _input->errReporter = new DOMTreeErrorReporter(logger);
    _input->parser->setErrorHandler(_input->errReporter);
    _input->parser->setCreateEntityReferenceNodes(false);
    _input->parser->setToCreateXMLDeclTypeNode(true);

    // TODO:
    // parser->setEntityResolver

    //
    //  Parse the XML file, catching any XML exceptions that might propogate
    //  out of it.
    //
    bool errorsOccured = false;
    try
    {
        _input->parser->parse(*_input->source);
        int errorCount = _input->parser->getErrorCount();
        if (errorCount > 0)
	{
            errorsOccured = true;
	}
    }
    catch (const XMLException& ex)
    {
	ostringstream os;
	os << "xerces: parsing error: " << toString(ex.getMessage());
	logger->error(os.str());
        errorsOccured = true;
    }
    catch (const DOM_DOMException& ex)
    {
	ostringstream os;
	os << "xerces: DOM parsing error: " << toString(ex.msg);
	logger->error(os.str());
        errorsOccured = true;
    }

    if (errorsOccured)
    {
	delete _input;
	throw ::Ice::UnmarshalException(__FILE__, __LINE__);
    }

    //
    // The first child of the document is the root node - ignore
    // that. Move to the top-level node in the document content.
    //
    _input->current = _input->parser->getDocument().getFirstChild();
    _input->current = _input->current.getFirstChild();
}

XMLStream::~XMLStream()
{
    delete _input;

    //
    // And call the termination method (TODO: is this correct -
    // reference count?)
    //
    XMLPlatformUtils::Terminate();
}

void
XMLStream::startWriteDictionary(const string& name, ::Ice::Int size)
{
    ostringstream os;
    os << name << " length=\"" << size << "\"";
    startWrite(os.str());
}

void
XMLStream::endWriteDictionary()
{
    endWrite();
}

void
XMLStream::startWriteDictionaryElement()
{
    startWrite(seqElementName);
}

void
XMLStream::endWriteDictionaryElement()
{
    endWrite();
}

void
XMLStream::startReadDictionary(const string& name, ::Ice::Int& size)
{
    startRead(name);
    size = readLength();
    _input->current = _input->current.getFirstChild();
}

void
XMLStream::endReadDictionary()
{
    endRead();
}

void
XMLStream::startReadDictionaryElement()
{
    startRead(seqElementName);
}

void
XMLStream::endReadDictionaryElement()
{
    endRead();
}

void
XMLStream::startWriteSequence(const string& name, ::Ice::Int size)
{
    ostringstream os;
    os << name << " length=\"" << size << "\"";
    startWrite(os.str());
}

void
XMLStream::endWriteSequence()
{
    endWrite();
}

void
XMLStream::startWriteSequenceElement()
{
    startWrite(seqElementName);
}

void
XMLStream::endWriteSequenceElement()
{
    endWrite();
}

void
XMLStream::startReadSequence(const string& name, ::Ice::Int& size)
{
    startRead(name);
    size = readLength();
    _input->current = _input->current.getFirstChild();
}

void
XMLStream::endReadSequence()
{
    endRead();
}

void
XMLStream::startReadSequenceElement()
{
    startRead(seqElementName);
}

void
XMLStream::endReadSequenceElement()
{
    endRead();
}

void
XMLStream::startWriteStruct(const string& name)
{
    startWrite(name);
}

void
XMLStream::endWriteStruct()
{
    endWrite();
}

void
XMLStream::startReadStruct(const string& name)
{
    startRead(name);
    _input->current = _input->current.getFirstChild();
}

void
XMLStream::endReadStruct()
{
    endRead();
}

void
XMLStream::startWriteException(const string& name)
{
    startWrite(name);
}

void
XMLStream::endWriteException()
{
    endWrite();
}

void
XMLStream::startReadException(const string& name)
{
    startRead(name);
    _input->current = _input->current.getFirstChild();
}

void
XMLStream::endReadException()
{
    endRead();
}

void
XMLStream::writeEnum(const string& name, const ::Ice::StringSeq& table, ::Ice::Int ordinal)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);
    _os << nl << "<" << name << ">" << table[ordinal] << "</" << name << ">";
}

void
XMLStream::readEnum(const string& name, const ::Ice::StringSeq& table, ::Ice::Int& ordinal)
{
    startRead(name);

    DOM_Node child = _input->current.getFirstChild();
    if (child.isNull() || child.getNodeType() != DOM_Node::TEXT_NODE)
    {
	throw ::Ice::UnmarshalException(__FILE__, __LINE__);
    }

    string value = toString(child.getNodeValue());
    ::Ice::StringSeq::const_iterator p = find(table.begin(), table.end(), value);
    if (p == table.end())
    {
	throw ::Ice::UnmarshalException(__FILE__, __LINE__);
    }
    ordinal = p - table.begin();

    endRead();
}

void
XMLStream::writeByte(const string& name, ::Ice::Byte value)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);

    // The schema encoding for xs:byte is a value from -127 to 128.
    _os << nl << "<" << name << ">" << (int)value << "</" << name << ">";
}

void
XMLStream::writeByteSeq(const string& name, const ::Ice::ByteSeq& seq)
{
    startWrite(name);
    for (::Ice::ByteSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
	_os << nl << "<e>" << (int)*p << "</e>";
    }
    endWrite();
}

void
XMLStream::readByte(const string& name, ::Ice::Byte& value)
{
    startRead(name);

    DOM_Node child = _input->current.getFirstChild();
    if (child.isNull() || child.getNodeType() != DOM_Node::TEXT_NODE)
    {
	throw ::Ice::UnmarshalException(__FILE__, __LINE__);
    }

    string s = toString(child.getNodeValue());
    ::Ice::Int i = atoi(s.c_str());
    if (i < -127 || i > 128)
    {
	throw ::Ice::UnmarshalException(__FILE__, __LINE__);
    }
    value = (::Ice::Byte)i;

    endRead();
}

//
// None of the read*Seq methods are very efficient. It's possible to
// write them as follows in a much more efficient manner - but it's
// not as maintainable.
//
//    startRead(name);
//    ::Ice::Int size = readLength();
//    value.resize(size);
//    value_type::iterator p = value.begin();
//    DOM_NodeList children = _input->current.getChildNodes();
//
//    int nchildren = children.getLength();
//    for (int i = 0; i < nchildren; ++i)
//    {
//	DOM_Node child = children.item(i);
//	while (child.getNodeType() != DOM_Node::ELEMENT_NODE)
//	{
//	    child = child.getNextSibling();
//	}	
//      string name = toString(child.getNodeName());
//	if (name != seqElementName)
//	{
//	    throw ::Ice::UnmarshalException(__FILE__, __LINE__);
//	}
//
//	child = child.getFirstChild();
//	if (child.isNull() || child.getNodeType() != DOM_Node::TEXT_NODE)
//	{
//	    throw ::Ice::UnmarshalException(__FILE__, __LINE__);
//	}
//	
//	string s = toString(child.getNodeValue());
//	*p++ = ???;
//    }
//    
//    endRead();
//
//
//

void
XMLStream::readByteSeq(const string& name, ::Ice::ByteSeq& value)
{
    startRead(name);

    ::Ice::Int size = readLength();
    value.resize(size);
    if (size > 0)
    {
	_input->current = _input->current.getFirstChild();
	for (int i = 0; i < size; ++i)
	{
	    readByte(seqElementName, value[i]);
	}
    }
    
    endRead();
}

void
XMLStream::writeBool(const string& name, bool value)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);
    _os << nl << "<" << name << ">" << (value ? "true" : "false") << "</" << name << ">";
}

void
XMLStream::writeBoolSeq(const string& name, const ::Ice::BoolSeq& seq)
{
    startWrite(name);
    for (::Ice::BoolSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
	_os << nl << "<e>" << ((*p) ? "true" : "false") << "</e>";
    }
    endWrite();
}

void
XMLStream::readBool(const string& name, bool& value)
{
    startRead(name);

    DOM_Node child = _input->current.getFirstChild();
    if (child.isNull() || child.getNodeType() != DOM_Node::TEXT_NODE)
    {
	throw ::Ice::UnmarshalException(__FILE__, __LINE__);
    }

    string s = toString(child.getNodeValue());
    value = (s == "true") ? true : false;

    endRead();
}

void
XMLStream::readBoolSeq(const string& name, ::Ice::BoolSeq& value)
{
    startRead(name);

    ::Ice::Int size = readLength();
    value.resize(size);
    if (size > 0)
    {
	_input->current = _input->current.getFirstChild();
	for (int i = 0; i < size; ++i)
	{
	    bool v;
	    readBool(seqElementName, v);
	    value[i] = v;
	}
    }
    
    endRead();
}

void
XMLStream::writeShort(const string& name, ::Ice::Short value)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);
    _os << nl << "<" << name << ">" << value << "</" << name << ">";
}

void
XMLStream::writeShortSeq(const string& name, const ::Ice::ShortSeq& seq)
{
    startWrite(name);
    for (::Ice::ShortSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
	_os << nl << "<e>" << *p << "</e>";
    }
    endWrite();
}

void
XMLStream::readShort(const string& name, ::Ice::Short& value)
{
    startRead(name);

    DOM_Node child = _input->current.getFirstChild();
    if (child.isNull() || child.getNodeType() != DOM_Node::TEXT_NODE)
    {
	throw ::Ice::UnmarshalException(__FILE__, __LINE__);
    }

    string s = toString(child.getNodeValue());
    value = atoi(s.c_str());

    endRead();
}

void
XMLStream::readShortSeq(const string& name, ::Ice::ShortSeq& value)
{
    startRead(name);

    ::Ice::Int size = readLength();
    value.resize(size);
    if (size > 0)
    {
	_input->current = _input->current.getFirstChild();
	for (int i = 0; i < size; ++i)
	{
	    readShort(seqElementName, value[i]);
	}
    }
    
    endRead();
}

void
XMLStream::writeInt(const string& name, ::Ice::Int value)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);
    _os << nl << "<" << name << ">" << value << "</" << name << ">";
}

void
XMLStream::writeIntSeq(const string& name, const ::Ice::IntSeq& seq)
{
    startWrite(name);
    for (::Ice::IntSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
	_os << nl << "<e>" << *p << "</e>";
    }
    endWrite();
}

void
XMLStream::readInt(const string& name, ::Ice::Int& value)
{
    startRead(name);

    DOM_Node child = _input->current.getFirstChild();
    if (child.isNull() || child.getNodeType() != DOM_Node::TEXT_NODE)
    {
	throw ::Ice::UnmarshalException(__FILE__, __LINE__);
    }

    string s = toString(child.getNodeValue());
    value = atoi(s.c_str());

    endRead();
}

void
XMLStream::readIntSeq(const string& name, ::Ice::IntSeq& value)
{
    startRead(name);

    ::Ice::Int size = readLength();
    value.resize(size);
    if (size > 0)
    {
	_input->current = _input->current.getFirstChild();
	for (int i = 0; i < size; ++i)
	{
	    readInt(seqElementName, value[i]);
	}
    }
    
    endRead();
}

void
XMLStream::writeLong(const string& name, ::Ice::Long value)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);
    _os << nl << "<" << name << ">" << value << "</" << name << ">";
}

void
XMLStream::writeLongSeq(const string& name, const ::Ice::LongSeq& seq)
{
    startWrite(name);
    for (::Ice::LongSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
	_os << nl << "<e>" << *p << "</e>";
    }
    endWrite();
}

void
XMLStream::readLong(const string& name, ::Ice::Long& value)
{
    startRead(name);

    DOM_Node child = _input->current.getFirstChild();
    if (child.isNull() || child.getNodeType() != DOM_Node::TEXT_NODE)
    {
	throw ::Ice::UnmarshalException(__FILE__, __LINE__);
    }

    string s = toString(child.getNodeValue());
    value = atol(s.c_str());

    endRead();
}

void
XMLStream::readLongSeq(const string& name, ::Ice::LongSeq& value)
{
    startRead(name);

    ::Ice::Int size = readLength();
    value.resize(size);
    if (size > 0)
    {
	_input->current = _input->current.getFirstChild();
	for (int i = 0; i < size; ++i)
	{
	    readLong(seqElementName, value[i]);
	}
    }
    
    endRead();
}

void
XMLStream::writeFloat(const string& name, ::Ice::Float value)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);
    _os << nl << "<" << name << ">" << value << "</" << name << ">";
}

void
XMLStream::writeFloatSeq(const string& name, const ::Ice::FloatSeq& seq)
{
    startWrite(name);
    for (::Ice::FloatSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
	_os << nl << "<e>" << *p << "</e>";
    }
    endWrite();
}

void
XMLStream::readFloat(const string& name, ::Ice::Float& value)
{
    startRead(name);

    DOM_Node child = _input->current.getFirstChild();
    if (child.isNull() || child.getNodeType() != DOM_Node::TEXT_NODE)
    {
	throw ::Ice::UnmarshalException(__FILE__, __LINE__);
    }

    string s = toString(child.getNodeValue());
    value = atof(s.c_str());

    endRead();
}

void
XMLStream::readFloatSeq(const string& name, ::Ice::FloatSeq& value)
{
    startRead(name);

    ::Ice::Int size = readLength();
    value.resize(size);
    if (size > 0)
    {
	_input->current = _input->current.getFirstChild();
	for (int i = 0; i < size; ++i)
	{
	    readFloat(seqElementName, value[i]);
	}
    }
    
    endRead();
}

void
XMLStream::writeDouble(const string& name, ::Ice::Double value)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);
    _os << nl << "<" << name << ">" << value << "</" << name << ">";
}

void
XMLStream::writeDoubleSeq(const string& name, const ::Ice::DoubleSeq& seq)
{
    startWrite(name);
    for (::Ice::DoubleSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
	_os << nl << "<e>" << *p << "</e>";
    }
    endWrite();
}

void
XMLStream::readDouble(const string& name, ::Ice::Double& value)
{
    startRead(name);

    DOM_Node child = _input->current.getFirstChild();
    if (child.isNull() || child.getNodeType() != DOM_Node::TEXT_NODE)
    {
	throw ::Ice::UnmarshalException(__FILE__, __LINE__);
    }

    string s = toString(child.getNodeValue());
    value = atof(s.c_str());

    endRead();
}

void
XMLStream::readDoubleSeq(const string& name, ::Ice::DoubleSeq& value)
{
    startRead(name);

    ::Ice::Int size = readLength();
    value.resize(size);
    if (size > 0)
    {
	_input->current = _input->current.getFirstChild();
	for (int i = 0; i < size; ++i)
	{
	    readDouble(seqElementName, value[i]);
	}
    }
    
    endRead();
}

void
XMLStream::writeString(const string& name, const string& value)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);
    _os << nl << "<" << name << ">" << value << "</" << name << ">";
}

void
XMLStream::writeStringSeq(const string& name, const ::Ice::StringSeq& seq)
{
    startWrite(name);
    for (::Ice::StringSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
	_os << nl << "<e>" << *p << "</e>";
    }
    endWrite();
}

void
XMLStream::readString(const string& name, string& value)
{
    startRead(name);

    DOM_Node child = _input->current.getFirstChild();
    if (!child.isNull())
    {
	if (child.getNodeType() != DOM_Node::TEXT_NODE)
	{
	    throw ::Ice::UnmarshalException(__FILE__, __LINE__);
	}
	value = toString(child.getNodeValue());
    }
    else
    {
	static const string empty("");
	value = empty;
    }

    endRead();
}

void
XMLStream::readStringSeq(const string& name, ::Ice::StringSeq& value)
{
    startRead(name);

    ::Ice::Int size = readLength();
    value.resize(size);
    if (size > 0)
    {
	_input->current = _input->current.getFirstChild();
	for (int i = 0; i < size; ++i)
	{
	    readString(seqElementName, value[i]);
	}
    }
    
    endRead();
}

void
XMLStream::writeProxy(const string& name, const ::Ice::ObjectPrx& proxy)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);
    string s = _communicator->proxyToString(proxy);
    _os << nl << "<" << name << ">" << s << "</" << name << ">";
}

void
XMLStream::readProxy(const string& name, ::Ice::ObjectPrx& value)
{
    startRead(name);

    DOM_Node child = _input->current.getFirstChild();
    string s;
    if (!child.isNull())
    {
	if (child.getNodeType() != DOM_Node::TEXT_NODE)
	{
	    throw ::Ice::UnmarshalException(__FILE__, __LINE__);
	}
	s = toString(child.getNodeValue());
    }

    value = _communicator->stringToProxy(s);

    endRead();
}

void
XMLStream::writeObject(const string& name, const ::Ice::ObjectPtr& obj)
{
    //
    // If at the top level of the document then the object itself must
    // be written, otherwise write a reference.
    //
    bool writeReference = (_elementStack.size() != 0);

    //
    // If the object doesn't exist in the map add it.
    //
    map<Ice::ObjectPtr, ObjectInfo>::iterator p = _objects.find(obj);
    if (p == _objects.end())
    {
	ostringstream os;
	os << "object" << _nextId++;
	p = _objects.insert(map<Ice::ObjectPtr, ObjectInfo>::value_type(
				obj, ObjectInfo(os.str(), !writeReference))).first;
    }

    if (writeReference)
    {
	_os << nl << "<" << name << " href=\"#" << p->second.id << "\"/>";
    }
    else
    {
	//
	// Precondition: If the actual object is being written the
	// written flag should be true.
	//
	assert(p->second.written);
	writeObjectData(name, p->second.id, obj);
    }
}

void
XMLStream::readObject(const string& name, const string& signatureType, const ::Ice::ObjectFactoryPtr& factory,
		      ::Ice::ObjectPtr& value)
{
    startRead(name);

    //
    // Node attributes. Either id & type will be set, or href.
    //
    string id, type, href;
    readAttributes(id, type, href);

    //
    // If this object is an href then locate the object. If the object
    // has been previously read then it will be contained in the
    // readObjects list. Otherwise, it should be in the object-set at
    // the top-level of the document.
    //
    if (!href.empty())
    {
	href = href.substr(1); // Skip the '#'

	map<string, ::Ice::ObjectPtr>::const_iterator p = _input->readObjects.find(href);
	if (p != _input->readObjects.end())
	{
	    value = p->second;
	    endRead();
	    return;
	}

	//
	// The first child of the document is the root node - ignore
	// that. Move to the top-level node in the document content.
	//
	_input->current = _input->parser->getDocument().getFirstChild();
	_input->current = _input->current.getFirstChild();
	while (!_input->current.isNull())
	{
	    if (_input->current.getNodeType() == DOM_Node::ELEMENT_NODE)
	    {
		string dummy;
		readAttributes(id, type, dummy);
		if (id == href)
		{
		    break;
		}
	    }
	    _input->current = _input->current.getNextSibling();
	}
	//
	// If the object isn't found, that's an error.
	//
	if (_input->current.isNull())
	{
	    throw ::Ice::UnmarshalException(__FILE__, __LINE__);
	}
    }

    //
    // Empty type indicates nil object.
    //
    if (type.empty())
    {
	value = 0;
    }
    else
    {
	static const string iceObject("::Ice::Object");

	if (type == iceObject)
	{
	    value = new ::Ice::Object;
	}
	else
	{
	    //
	    // First, determine if there is an installed factory for this
	    // type. If so use that. If that factory doesn't exist, or it
	    // doesn't create an object of that type then use the supplied
	    // factory provided that the types are the same. If no value
	    // can be created then throw a NoObjectFactoryException.
	    //
	    ::Ice::ObjectFactoryPtr userFactory = _communicator->findObjectFactory(type);
	    if (userFactory)
	    {
		value = userFactory->create(type);
	    }

	    if (!value && type == signatureType)
	    {
		assert(factory);
		value = factory->create(type);
		assert(value);
	    }

	    if (!value)
	    {
		throw ::Ice::NoObjectFactoryException(__FILE__, __LINE__);
	    }
	}

	//
	// Add the object to the readObjects map, move to the first
	// child node & unmarshal the object.
	//
	_input->readObjects.insert(map<string, ::Ice::ObjectPtr>::value_type(id, value));
	_input->current = _input->current.getFirstChild();
	value->__unmarshal(this);
    }

    endRead();
}

void
XMLStream::startWrite(const string& element)
{
    _os << nl << '<' << element << '>';
    _os.inc();

    string::size_type pos = element.find_first_of(" \t");
    if (pos == string::npos)
    {
	_elementStack.push(element);
    }
    else
    {
	_elementStack.push(element.substr(0, pos));
    }
}

void
XMLStream::endWrite()
{
    string element = _elementStack.top();
    _elementStack.pop();

    _os.dec();
    _os << nl << "</" << element << '>';

    if (_elementStack.size() == 0)
    {
	dumpUnwrittenObjects();
    }
}


void
XMLStream::startRead(const ::std::string& element)
{
    while (!_input->current.isNull() && _input->current.getNodeType() != DOM_Node::ELEMENT_NODE)
    {
	_input->current = _input->current.getNextSibling();
    }
    if (_input->current.isNull())
    {
	cout << "element: " << element << endl;
	throw ::Ice::UnmarshalException(__FILE__, __LINE__);
    }
    
    string nodeName = toString(_input->current.getNodeName());
    if (element != nodeName)
    {
	throw ::Ice::UnmarshalException(__FILE__, __LINE__);
    }

    _input->nodeStack.push_back(_input->current);
}

void
XMLStream::endRead()
{
    _input->current = _input->nodeStack.back();
    _input->nodeStack.pop_back();
    _input->current = _input->current.getNextSibling();
}

void
XMLStream::dumpUnwrittenObjects()
{
    //
    // Precondition: Must be at the top-level.
    //
    assert(_elementStack.size() == 0);

    //
    // It's necessary to run through the set of unwritten objects
    // repeatedly until all referenced objects have been written
    // (since referenced objects can themselves reference other
    // objects).
    //
    unsigned int nwritten;
    do
    {
	nwritten = 0;
	map<Ice::ObjectPtr, ObjectInfo>::iterator p;
	for (p = _objects.begin(); p != _objects.end(); ++p)
	{
	    if (!p->second.written)
	    {
		p->second.written = true;
		writeObjectData("ice:object", p->second.id, p->first);
		++nwritten;
	    }
	    else
	    {
		++nwritten;
	    }
	}
    }
    while (_objects.size() != nwritten);
}

void
XMLStream::writeObjectData(const string& name, const string& id, const Ice::ObjectPtr& obj)
{
    string xsdType;
    string typeId;

    if (obj)
    {
	//
	// Convert the type-id to the equivalent schema type
	// definition name.
	//
	typeId = obj->__getClassIds()[0];

	static const string xsdPrefix = "_internal.";
	static const string xsdSuffix = "Type";
	xsdType.reserve(typeId.size() + xsdPrefix.size() + xsdSuffix.size());
	xsdType += xsdPrefix;
	unsigned int i = 0;
	if (typeId[0] == ':') // Skip the preceeding "::".
	{
	    i = 2;
	}
	for (; i < typeId.size(); ++i)
	{
	    if (typeId[i] == ':')
	    {
		xsdType += '.';
		++i;
	    }
	    else
	    {
		xsdType += typeId[i];
	    }
	}
	xsdType += xsdSuffix;
    }
    else
    {
	xsdType = "";
    }

    ostringstream os;
    os << name << " id=\"" << id << "\" type=\"" << typeId << "\""
       << " xsi:type=\"" << xsdType << "\"";
    if (!obj)
    {
	os << " xsi:nil=\"true\"";
    }

    startWrite(os.str());
    if (obj)
    {
	obj->__marshal(this);
    }
    endWrite();
}

void
XMLStream::readAttributes(::std::string& id, ::std::string& type, ::std::string& href)
{
    static const string idStr("id");
    static const string typeStr("type");
    static const string hrefStr("href");

    DOM_NamedNodeMap attributes = _input->current.getAttributes();
    int attrCount = attributes.getLength();
    for (int i = 0; i < attrCount; i++)
    {
	DOM_Node attribute = attributes.item(i);
	string name = toString(attribute.getNodeName());
	if (name == idStr)
	{
	    id = toString(attribute.getNodeValue());
	}
	else if (name == typeStr)
	{
	    type = toString(attribute.getNodeValue());
	}
	else if (name == hrefStr)
	{
	    href = toString(attribute.getNodeValue());
	}
    }
}

::Ice::Int
XMLStream::readLength()
{
    static const string lengthStr("length");

    DOM_NamedNodeMap attributes = _input->current.getAttributes();
    int attrCount = attributes.getLength();
    for (int i = 0; i < attrCount; i++)
    {
	DOM_Node attribute = attributes.item(i);
	string name = toString(attribute.getNodeName());
	if (name == lengthStr)
	{
	    return atoi(toString(attribute.getNodeValue()).c_str());
	}
    }

    throw ::Ice::UnmarshalException(__FILE__, __LINE__);
}
