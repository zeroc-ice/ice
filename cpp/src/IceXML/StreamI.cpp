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

#include <IceXML/StreamI.h>

#include <list>

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
using namespace IceUtil;

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
struct StreamInputImpl
{
    StreamInputImpl() :
	source(0),
	parser(0),
	errReporter(0)
    {
    }
    ~StreamInputImpl()
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
const string IceXML::StreamI::seqElementName("e");

IceXML::StreamI::StreamI(const ::Ice::CommunicatorPtr& communicator, std::ostream& os) :
    _communicator(communicator),
    _input(0),
    _os(os),
    _level(0),
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


IceXML::StreamI::StreamI(const ::Ice::CommunicatorPtr& communicator, std::istream& is, bool schema) :
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
    
    _input = new StreamInputImpl();
    _input->source = new MemBufInputSource((const XMLByte*)_content.data(), _content.size(), "inputsource");

    //
    //  Create our parser, then attach an error handler to the parser.
    //  The parser will call back to methods of the ErrorHandler if it
    //  discovers errors during the course of parsing the XML
    //  document.
    //
    _input->parser = new DOMParser;
    _input->parser->setValidationScheme(DOMParser::Val_Auto);
    if (schema)
    {
	_input->parser->setDoNamespaces(true);
	_input->parser->setDoSchema(true);
    }
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

IceXML::StreamI::~StreamI()
{
    delete _input;

    //
    // And call the termination method (TODO: is this correct -
    // reference count?)
    //
    XMLPlatformUtils::Terminate();
}

void
IceXML::StreamI::startWriteDictionary(const string& name, ::Ice::Int size)
{
    ostringstream os;
    os << name << " length=\"" << size << "\"";
    startWrite(os.str());
}

void
IceXML::StreamI::endWriteDictionary()
{
    endWrite();
}

void
IceXML::StreamI::startWriteDictionaryElement()
{
    startWrite(seqElementName);
}

void
IceXML::StreamI::endWriteDictionaryElement()
{
    endWrite();
}

void
IceXML::StreamI::startReadDictionary(const string& name, ::Ice::Int& size)
{
    startRead(name);
    size = readLength();
    _input->current = _input->current.getFirstChild();
}

void
IceXML::StreamI::endReadDictionary()
{
    endRead();
}

void
IceXML::StreamI::startReadDictionaryElement()
{
    startRead(seqElementName);
    _input->current = _input->current.getFirstChild();
}

void
IceXML::StreamI::endReadDictionaryElement()
{
    endRead();
}

void
IceXML::StreamI::startWriteSequence(const string& name, ::Ice::Int size)
{
    ostringstream os;
    os << name << " length=\"" << size << "\"";
    startWrite(os.str());
}

void
IceXML::StreamI::endWriteSequence()
{
    endWrite();
}

void
IceXML::StreamI::startWriteSequenceElement()
{
    startWrite(seqElementName);
}

void
IceXML::StreamI::endWriteSequenceElement()
{
    endWrite();
}

void
IceXML::StreamI::startReadSequence(const string& name, ::Ice::Int& size)
{
    startRead(name);
    size = readLength();
    _input->current = _input->current.getFirstChild();
}

void
IceXML::StreamI::endReadSequence()
{
    endRead();
}

void
IceXML::StreamI::startReadSequenceElement()
{
    startRead(seqElementName);
}

void
IceXML::StreamI::endReadSequenceElement()
{
    endRead();
}

void
IceXML::StreamI::startWriteStruct(const string& name)
{
    startWrite(name);
}

void
IceXML::StreamI::endWriteStruct()
{
    endWrite();
}

void
IceXML::StreamI::startReadStruct(const string& name)
{
    startRead(name);
    _input->current = _input->current.getFirstChild();
}

void
IceXML::StreamI::endReadStruct()
{
    endRead();
}

void
IceXML::StreamI::startWriteException(const string& name)
{
    startWrite(name);
}

void
IceXML::StreamI::endWriteException()
{
    endWrite();
}

void
IceXML::StreamI::startReadException(const string& name)
{
    startRead(name);
    _input->current = _input->current.getFirstChild();
}

void
IceXML::StreamI::endReadException()
{
    endRead();
}

void
IceXML::StreamI::writeEnum(const string& name, const ::Ice::StringSeq& table, ::Ice::Int ordinal)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);
    _os << nl << "<" << name << ">" << table[ordinal] << "</" << name << ">";
}

void
IceXML::StreamI::readEnum(const string& name, const ::Ice::StringSeq& table, ::Ice::Int& ordinal)
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
IceXML::StreamI::writeByte(const string& name, ::Ice::Byte value)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);

    // The schema encoding for xs:byte is a value from -127 to 128.
    _os << nl << "<" << name << ">" << (int)value << "</" << name << ">";
}

void
IceXML::StreamI::writeByteSeq(const string& name, const ::Ice::ByteSeq& seq)
{
    startWrite(name);
    for (::Ice::ByteSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
	_os << nl << "<e>" << (int)*p << "</e>";
    }
    endWrite();
}

void
IceXML::StreamI::readByte(const string& name, ::Ice::Byte& value)
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
IceXML::StreamI::readByteSeq(const string& name, ::Ice::ByteSeq& value)
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
IceXML::StreamI::writeBool(const string& name, bool value)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);
    _os << nl << "<" << name << ">" << (value ? "true" : "false") << "</" << name << ">";
}

void
IceXML::StreamI::writeBoolSeq(const string& name, const ::Ice::BoolSeq& seq)
{
    startWrite(name);
    for (::Ice::BoolSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
	_os << nl << "<e>" << ((*p) ? "true" : "false") << "</e>";
    }
    endWrite();
}

void
IceXML::StreamI::readBool(const string& name, bool& value)
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
IceXML::StreamI::readBoolSeq(const string& name, ::Ice::BoolSeq& value)
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
IceXML::StreamI::writeShort(const string& name, ::Ice::Short value)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);
    _os << nl << "<" << name << ">" << value << "</" << name << ">";
}

void
IceXML::StreamI::writeShortSeq(const string& name, const ::Ice::ShortSeq& seq)
{
    startWrite(name);
    for (::Ice::ShortSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
	_os << nl << "<e>" << *p << "</e>";
    }
    endWrite();
}

void
IceXML::StreamI::readShort(const string& name, ::Ice::Short& value)
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
IceXML::StreamI::readShortSeq(const string& name, ::Ice::ShortSeq& value)
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
IceXML::StreamI::writeInt(const string& name, ::Ice::Int value)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);
    _os << nl << "<" << name << ">" << value << "</" << name << ">";
}

void
IceXML::StreamI::writeIntSeq(const string& name, const ::Ice::IntSeq& seq)
{
    startWrite(name);
    for (::Ice::IntSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
	_os << nl << "<e>" << *p << "</e>";
    }
    endWrite();
}

void
IceXML::StreamI::readInt(const string& name, ::Ice::Int& value)
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
IceXML::StreamI::readIntSeq(const string& name, ::Ice::IntSeq& value)
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
IceXML::StreamI::writeLong(const string& name, ::Ice::Long value)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);
    _os << nl << "<" << name << ">" << value << "</" << name << ">";
}

void
IceXML::StreamI::writeLongSeq(const string& name, const ::Ice::LongSeq& seq)
{
    startWrite(name);
    for (::Ice::LongSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
	_os << nl << "<e>" << *p << "</e>";
    }
    endWrite();
}

void
IceXML::StreamI::readLong(const string& name, ::Ice::Long& value)
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
IceXML::StreamI::readLongSeq(const string& name, ::Ice::LongSeq& value)
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
IceXML::StreamI::writeFloat(const string& name, ::Ice::Float value)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);
    _os << nl << "<" << name << ">" << value << "</" << name << ">";
}

void
IceXML::StreamI::writeFloatSeq(const string& name, const ::Ice::FloatSeq& seq)
{
    startWrite(name);
    for (::Ice::FloatSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
	_os << nl << "<e>" << *p << "</e>";
    }
    endWrite();
}

void
IceXML::StreamI::readFloat(const string& name, ::Ice::Float& value)
{
    startRead(name);

    DOM_Node child = _input->current.getFirstChild();
    if (child.isNull() || child.getNodeType() != DOM_Node::TEXT_NODE)
    {
	throw ::Ice::UnmarshalException(__FILE__, __LINE__);
    }

    string s = toString(child.getNodeValue());
    value = (::Ice::Float)atof(s.c_str());

    endRead();
}

void
IceXML::StreamI::readFloatSeq(const string& name, ::Ice::FloatSeq& value)
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
IceXML::StreamI::writeDouble(const string& name, ::Ice::Double value)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);
    _os << nl << "<" << name << ">" << value << "</" << name << ">";
}

void
IceXML::StreamI::writeDoubleSeq(const string& name, const ::Ice::DoubleSeq& seq)
{
    startWrite(name);
    for (::Ice::DoubleSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
	_os << nl << "<e>" << *p << "</e>";
    }
    endWrite();
}

void
IceXML::StreamI::readDouble(const string& name, ::Ice::Double& value)
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
IceXML::StreamI::readDoubleSeq(const string& name, ::Ice::DoubleSeq& value)
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
IceXML::StreamI::writeString(const string& name, const string& value)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);
    string v = value;

    //
    // Find out whether there is a reserved character to avoid
    // conversion if not necessary.
    //
    static const string allReserved = "<>'\"&";
    if (v.find_first_of(allReserved) != string::npos)
    {
	//
	// First convert all & to &amp;
	//
	size_t pos = 0;
	while ((pos = v.find_first_of('&', pos)) != string::npos)
	{
	    v.insert(pos+1, "amp;");
	    pos += 4;
	}

	//
	// Next convert remaining reserved characters.
	//
	static const string reserved = "<>'\"";
	pos = 0;
	while ((pos = v.find_first_of(reserved, pos)) != string::npos)
	{
	    string replace;
	    switch(v[pos])
	    {
	    case '>':
		replace = "&gt;";
		break;

	    case '<':
		replace = "&lt;";
		break;

	    case '\'':
		replace = "&apos;";
		break;

	    case '"':
		replace = "&quot;";
		break;

	    default:
		assert(false);
	    }
	    v.erase(pos, 1);
	    v.insert(pos, replace);
	    pos += replace.size();
	}
    }
    
    _os << nl << "<" << name << ">" << v << "</" << name << ">";
}

void
IceXML::StreamI::writeStringSeq(const string& name, const ::Ice::StringSeq& seq)
{
    startWrite(name);
    for (::Ice::StringSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
	_os << nl << "<e>" << *p << "</e>";
    }
    endWrite();
}

void
IceXML::StreamI::readString(const string& name, string& value)
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
IceXML::StreamI::readStringSeq(const string& name, ::Ice::StringSeq& value)
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
IceXML::StreamI::writeProxy(const string& name, const ::Ice::ObjectPrx& proxy)
{
    // No attributes
    assert(name.find_first_of(" \t") == string::npos);
    string s = _communicator->proxyToString(proxy);
    _os << nl << "<" << name << ">" << s << "</" << name << ">";
}

void
IceXML::StreamI::readProxy(const string& name, ::Ice::ObjectPrx& value)
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
IceXML::StreamI::writeObject(const string& name, const ::Ice::ObjectPtr& obj)
{
    //
    // If at the top level of the document then the object itself must
    // be written, otherwise write a reference.
    //
    bool writeReference = (_level != 0);

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
IceXML::StreamI::readObject(const string& name, const string& signatureType, const ::Ice::ObjectFactoryPtr& factory,
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
IceXML::StreamI::startWrite(const string& element)
{
    _os << se(element);
    ++_level;
}

void
IceXML::StreamI::endWrite()
{
    --_level;

    _os << ee;

    if (_level == 0)
    {
	dumpUnwrittenObjects();
    }
}


void
IceXML::StreamI::startRead(const ::std::string& element)
{
    while (!_input->current.isNull() && _input->current.getNodeType() != DOM_Node::ELEMENT_NODE)
    {
	_input->current = _input->current.getNextSibling();
    }
    if (_input->current.isNull())
    {
	throw ::Ice::UnmarshalException(__FILE__, __LINE__);
    }
    
    string nodeName = toString(_input->current.getNodeName());
    //
    // TODO: Work around for bug in xerces
    //
    static const string facets = "facets";
    static const string facetsNS = "ice:facets";
    if ((element != facetsNS || nodeName != facets) && element != nodeName)
    {
	throw ::Ice::UnmarshalException(__FILE__, __LINE__);
    }

    _input->nodeStack.push_back(_input->current);
}

void
IceXML::StreamI::endRead()
{
    _input->current = _input->nodeStack.back();
    _input->nodeStack.pop_back();
    _input->current = _input->current.getNextSibling();
}

void
IceXML::StreamI::dumpUnwrittenObjects()
{
    //
    // Precondition: Must be at the top-level.
    //
    assert(_level == 0);

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
IceXML::StreamI::writeObjectData(const string& name, const string& id, const Ice::ObjectPtr& obj)
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
IceXML::StreamI::readAttributes(::std::string& id, ::std::string& type, ::std::string& href)
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
IceXML::StreamI::readLength()
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
