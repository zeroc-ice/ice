// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <XMLTransform/XMLTransform.h>
#include <XMLTransform/ErrorReporter.h>
#include <Freeze/DB.h>

#include <util/PlatformUtils.hpp>
#include <util/XMLString.hpp>
#include <util/XMLUniDefs.hpp>
#include <util/TranscodingException.hpp>
#include <framework/MemBufInputSource.hpp>
#include <framework/XMLFormatter.hpp>
#include <dom/DOMString.hpp>
#include <dom/DOM.hpp>
#include <parsers/DOMParser.hpp>
#include <sax/SAXParseException.hpp>

#include <sys/stat.h>

using namespace std;
using namespace Freeze;
using namespace XMLTransform;

//
// Implementation of exception types.
//
XMLTransform::IllegalTransform::IllegalTransform(const char* file, int line) :
    Exception(file, line)
{
}

string
XMLTransform::IllegalTransform::ice_name() const
{
    return "IllegalTransform";
}

void
XMLTransform::IllegalTransform::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n" << reason;
}

::IceUtil::Exception*
XMLTransform::IllegalTransform::ice_clone() const
{
    return new IllegalTransform(*this);
}

void
XMLTransform::IllegalTransform::ice_throw() const
{
    throw *this;
}

XMLTransform::IncompatibleSchema::IncompatibleSchema(const char* file, int line) :
    Exception(file, line)
{
}

string
XMLTransform::IncompatibleSchema::ice_name() const
{
    return "IncompatibleSchema";
}

void
XMLTransform::IncompatibleSchema::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n" << reason;
}

::IceUtil::Exception*
XMLTransform::IncompatibleSchema::ice_clone() const
{
    return new IncompatibleSchema(*this);
}

void
XMLTransform::IncompatibleSchema::ice_throw() const
{
    throw *this;
}

XMLTransform::InvalidSchema::InvalidSchema(const char* file, int line) :
    Exception(file, line)
{
}

string
XMLTransform::InvalidSchema::ice_name() const
{
    return "InvalidSchema";
}

void
XMLTransform::InvalidSchema::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n" << reason;
}

::IceUtil::Exception*
XMLTransform::InvalidSchema::ice_clone() const
{
    return new InvalidSchema(*this);
}

void
XMLTransform::InvalidSchema::ice_throw() const
{
    throw *this;
}

XMLTransform::SchemaViolation::SchemaViolation(const char* file, int line) :
    Exception(file, line)
{
}

string
XMLTransform::SchemaViolation::ice_name() const
{
    return "SchemaViolation";
}

void
XMLTransform::SchemaViolation::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n" << reason;
}

::IceUtil::Exception*
XMLTransform::SchemaViolation::ice_clone() const
{
    return new SchemaViolation(*this);
}

void
XMLTransform::SchemaViolation::ice_throw() const
{
    throw *this;
}

XMLTransform::TransformException::TransformException(const char* file, int line) :
    Exception(file, line)
{
}

string
XMLTransform::TransformException::ice_name() const
{
    return "TransformException";
}

void
XMLTransform::TransformException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n" << reason;
}

::IceUtil::Exception*
XMLTransform::TransformException::ice_clone() const
{
    return new TransformException(*this);
}

void
XMLTransform::TransformException::ice_throw() const
{
    throw *this;
}

//
// Helper to convert a DOMString to a native string.
//
static string
toString(const DOMString& s)
{
    char* t = s.transcode();
    string r(t);
    delete[] t;
    return r;
}

//
// Helpers for finding named child nodes.
//
static DOM_Node
findChild(const DOM_Node& parent, const string& namespaceURI, const string& localname)
{
    DOM_NodeList children = parent.getChildNodes();
    for(unsigned int i = 0; i < children.getLength(); ++i)
    {
	DOM_Node child = children.item(i);
	if(toString(child.getNamespaceURI()) == namespaceURI && toString(child.getLocalName()) == localname)
	{
	    return child;
	}
    }

    return DOM_Node();
}

static DOM_Node
findChild(const DOM_Node& parent, const string& name)
{
    DOM_NodeList children = parent.getChildNodes();
    for(unsigned int i = 0; i < children.getLength(); ++i)
    {
	DOM_Node child = children.item(i);
	if(toString(child.getNodeName()) == name)
	{
	    return child;
	}
    }
    return DOM_Node();
}

//
// Helpers to retrieve a named attribute.
//
static string
getAttributeByName(const DOM_Node& node, const string& name)
{
    DOM_NamedNodeMap attributes = node.getAttributes();
    string value;
    DOM_Node n = attributes.getNamedItem(DOMString(name.c_str()));
    if(!n.isNull())
    {
	value = toString(n.getNodeValue());
    }

    return value;
}

//
// Helpers for specific attributes.
//
static string
getTypeAttribute(const DOM_Node& node)
{
    static const string typeName = "type";
    return getAttributeByName(node, typeName);
}

static string
getNameAttribute(const DOM_Node& node)
{
    static const string nameName = "name";
    return getAttributeByName(node, nameName);
}

//
// Find a file in one of the given directories.
//
static string
findFile(const string& name, const Ice::StringSeq& paths)
{
    struct stat st;

    //
    // If the file is found in the current working directory,
    // return now.
    //
    if(stat(name.c_str(), &st) == 0)
    {
        return name;
    }

    for(Ice::StringSeq::size_type i = 0; i < paths.size(); i++)
    {
        string file = paths[i] + "/" + name;
        if(stat(file.c_str(), &st) == 0)
        {
            return file;
        }
    }

    return name;
}


//
// DocumentMap. This maps namespace uri -> a set of documents.
//
typedef multimap< string, DocumentInfoPtr> DocumentMap;

//
// Document info holds the DOM root of the document, a map of prefix to namespace URI & the documents
// targetNamespace.
//
XMLTransform::DocumentInfo::DocumentInfo(const DOM_Document& document, const DOM_Node& root,
                                         const string& targetNamespace) :
    _document(document),
    _targetNamespace(targetNamespace)
{
    static const string targetNamespaceAttrName = "targetNamespace";
    static const string xmlnsAttrName = "xmlns";
	
    DOM_NamedNodeMap attributes = root.getAttributes();
    unsigned int max = attributes.getLength();
    for(unsigned int i = 0; i < max; ++i)
    {
	DOM_Node attribute = attributes.item(i);
	string attrName = toString(attribute.getNodeName());
	if(attrName.substr(0, 5) == xmlnsAttrName)
	{
	    string ns;
	    if(attrName.size() > 5)
	    {
		ns = attrName.substr(6);
	    }
	    string uri = toString(attribute.getNodeValue());
	    _nsMap.insert(make_pair(ns, uri));
	}
	else if(attrName == targetNamespaceAttrName)
	{
	    _targetNamespace = toString(attribute.getNodeValue());
	}
    }
}

DOM_Document
XMLTransform::DocumentInfo::getDocument() const
{
    return _document;
}

string
XMLTransform::DocumentInfo::findURI(const string& prefix) const
{
    string uri;
    PrefixURIMap::const_iterator q = _nsMap.find(prefix);
    if(q != _nsMap.end())
    {
	uri = q->second;
    }
    return uri;
}

string
XMLTransform::DocumentInfo::getTargetNamespace() const
{
    return _targetNamespace;
}

//
// Convert a QName from prefix:local to local@namespaceURI.
//
static string
convertQName(const string& qname, const DocumentInfoPtr& info)
{
    size_t pos = qname.find(':');
    string prefix;
    string localName;
    if(pos != string::npos)
    {
	prefix = qname.substr(0, pos);
	localName = qname.substr(pos+1);
    }
    else
    {
	localName = qname;
    }

    string uri = info->findURI(prefix);
    if(uri.empty())
    {
	//
	// No namespace - TODO: not InvalidSchema - it's an invalid instance document.
	//
	throw InvalidSchema(__FILE__, __LINE__);
    }
	
    string n = localName;
    n += '@';
    n += uri;

    return n;
}

//
// TODO: These cannot be allocated as static strings since xerces API calls cannot be made until
// XMLPlatformUtils::Initialize has been called.
//
//static DOMString schemaURI("http://www.w3.org/2001/XMLSchema");
//static DOMString schemaElementName("schema");

//
// TODO: All the xs: searching should be replaced by correct searches.
//
static const string sequenceElementName = "xs:sequence";
static const string annotationElementName = "xs:annotation";
static const string appinfoElementName = "xs:appinfo";
static const string importElementName = "xs:import";
static const string elementElementName = "xs:element";
static const string complexContentElementName = "xs:complexContent";
static const string extensionElementName = "xs:extension";
static const string restrictionElementName = "xs:restriction";
static const string enumerationElementName = "xs:enumeration";

//
// TODO: have print do something sensible.
//

//
// Transform implementations.
//
XMLTransform::Transform::Transform()
{
}

XMLTransform::Transform::~Transform()
{
}

static ::IceUtil::XMLOutput&
operator<<(::IceUtil::XMLOutput& os, const DOM_Node& node)
{
    switch(node.getNodeType())
    {
    case DOM_Node::TEXT_NODE:
    {
	os << ::IceUtil::startEscapes << toString(node.getNodeValue()) << ::IceUtil::endEscapes;
	break;
    }

    case DOM_Node::ELEMENT_NODE:
    {
	os << ::IceUtil::se(toString(node.getNodeName()));
	DOM_NamedNodeMap attributes = node.getAttributes();
	for(unsigned int i = 0; i < attributes.getLength(); ++i)
	{
	    DOM_Node attribute = attributes.item(i);
	    os << ::IceUtil::attr(toString(attribute.getNodeName()), toString(attribute.getNodeValue()));
	}

	DOM_Node child = node.getFirstChild();
	while(!child.isNull())
	{
	    os << child;
	    child = child.getNextSibling();
	}

	os << ::IceUtil::ee;
	break;
    }

    default:
	assert(false);
    }
    return os;
}

namespace XMLTransform
{

//
// Nil transform - this transform does nothing.
//
class NilTransform : public Transform
{
public:

    NilTransform()
    {
    }

    virtual void
    transform(::IceUtil::XMLOutput& os, const DocumentInfoPtr&, const string&, const DOM_Node& node)
    {
	if(node.isNull())
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}

	os << node;
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[nil transform]\n";
	return os;
    }
};

typedef ::IceUtil::Handle<NilTransform> NilTransformPtr;

//
// This transform is the root transform for marshaled documents.
//
// TODO: complete, rename
//
class InternalTransform : public Transform
{
public:

    InternalTransform()
    {
    }

    virtual void
    transform(::IceUtil::XMLOutput& os, const DocumentInfoPtr&, const string&, const DOM_Node& node)
    {
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[internal transform]\n";
	return os;
    }
};

typedef ::IceUtil::Handle<InternalTransform> InternalTransformPtr;

//
// This transform applies to an element. It holds the element name, and applies the contained transform.
//
class ElementTransform : public Transform
{
public:
    
    ElementTransform(const string& namespaceURI, const string& name, TransformPtr transform) :
        _ns(namespaceURI),
	_name(name),
        _transform(transform)
    {
    }

    virtual void
    transform(::IceUtil::XMLOutput& os, const DocumentInfoPtr& info, const string&, const DOM_Node& node)
    {
	_transform->transform(os, info, _name, node);
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[element: " << _name << "]\n";
	_transform->print(os);
	os << "[/element: " << _name << "]\n";
	return os;
    }

    const string& namespaceURI() const { return _ns; }
    const string& name() const { return _name; }

private:

    string _ns;
    string _name;

    TransformPtr _transform;
};

typedef ::IceUtil::Handle<ElementTransform> ElementTransformPtr;

//
// This transform validates that the contained enumeration value is still valid.
//
class ValidateEnumerationTransform : public Transform
{
public:

    ValidateEnumerationTransform(const vector<string>& values) :
	_values(values)
    {
    }

    virtual void
    transform(::IceUtil::XMLOutput& os, const DocumentInfoPtr&, const string&, const DOM_Node& node)
    {
	if(node.isNull())
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}

	DOM_Node child = node.getFirstChild();
	if(child.isNull() || child.getNodeType() != DOM_Node::TEXT_NODE)
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}
	string value = toString(child.getNodeValue());
	if(find(_values.begin(), _values.end(), value) == _values.end())
	{
	    throw IllegalTransform(__FILE__, __LINE__);
	}
	os << node;
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[validate enumeration]\n";
	return os;
    }

private:
  
    vector<string> _values;
};    

typedef ::IceUtil::Handle<ValidateEnumerationTransform> ValidateEnumerationTransformPtr;

//
// This transform validates the transform of an integer type.
//
class ValidateIntegerTransform : public Transform
{
public:

    ValidateIntegerTransform(const string& from, const string& to) :
	_from(from),
	_to(to)
    {
    }

    virtual void
    transform(::IceUtil::XMLOutput& os, const DocumentInfoPtr&, const string&, const DOM_Node& node)
    {
	if(node.isNull())
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}

	DOM_Node child = node.getFirstChild();
	if(child.isNull() || child.getNodeType() != DOM_Node::TEXT_NODE)
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}
	string value = toString(child.getNodeValue());
#ifdef WIN32
        Ice::Long v = _atoi64(value.c_str());
#else
        Ice::Long v = atoll(value.c_str());
#endif
	if(_to == "xs:byte")
	{
	    if(v < SCHAR_MIN || v > SCHAR_MAX)
	    {
		throw IllegalTransform(__FILE__, __LINE__);
	    }
	}
	else if(_to == "xs:short")
	{
	    if(v < SHRT_MIN || v > SHRT_MAX)
	    {
		throw IllegalTransform(__FILE__, __LINE__);
	    }
	}
	else if(_to == "xs:int")
	{
	    if(v < INT_MIN || v > INT_MAX)
	    {
		throw IllegalTransform(__FILE__, __LINE__);
	    }
	}
	else if(_to == "xs:long")
	{
	}
	os << node;
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[validate integer: from=" << _from << " to=" << _to << "]\n";
	return os;
    }

private:
    
    string _from;
    string _to;
};

typedef ::IceUtil::Handle<ValidateIntegerTransform> ValidateIntegerTransformPtr;

//
// This transform validates a transformation of a floating point type.
//
class ValidateFloatTransform : public Transform
{
public:

    ValidateFloatTransform(const string& from, const string& to) :
	_from(from),
	_to(to)
    {
    }

    virtual void
    transform(::IceUtil::XMLOutput& os, const DocumentInfoPtr&, const string&, const DOM_Node& node)
    {
	if(node.isNull())
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}

	DOM_Node child = node.getFirstChild();
	if(child.isNull() || child.getNodeType() != DOM_Node::TEXT_NODE)
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}
	os << node;
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[validate float: from=" << _from << " to=" << _to << "]\n";
	return os;
    }

private:
    
    string _from;
    string _to;
};

typedef ::IceUtil::Handle<ValidateFloatTransform> ValidateFloatTransformPtr;

//
// Transform a sequence. The transform holds the transform for the contained elements in the sequence.
//
class SequenceTransform : public Transform
{
public:

    SequenceTransform(TransformPtr transform) :
	_transform(transform)
    {
    }

    virtual void
    transform(::IceUtil::XMLOutput& os, const DocumentInfoPtr& info, const string&, const DOM_Node& node)
    {
	if(node.isNull())
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}
	string name = toString(node.getNodeName());
	os << ::IceUtil::se(name);
	string length = getAttributeByName(node, "length");

	os << ::IceUtil::attr("length", length);

	long l = atol(length.c_str());

	DOM_NodeList children = node.getChildNodes();
	for(unsigned int i = 0; i < children.getLength(); ++i)
	{
	    DOM_Node child = children.item(i);
	    if(child.getNodeType() != DOM_Node::ELEMENT_NODE)
	    {
		continue;
	    }

	    static const string sequenceElementName = "e";
	    string nodeName = toString(child.getLocalName());
	    if(l == 0 || nodeName != sequenceElementName)
	    {
		throw SchemaViolation(__FILE__, __LINE__);
	    }
	    _transform->transform(os, info, nodeName, child);
	    --l;
	}

	os << ::IceUtil::ee;
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[sequence]\n";
	_transform->print(os);
	os << "[/sequence]\n";
	return os;
    }

private:

    TransformPtr _transform;
};

typedef ::IceUtil::Handle<SequenceTransform> SequenceTransformPtr;

//
// Transform the content of a struct. The transform holds a list of name->transform pairs in new instance
// document order. For a given transform the corresponding node is found and the transform is applied.
//
class StructTransform : public Transform
{
public:

    StructTransform()
    {
    }

    StructTransform(const vector<ElementTransformPtr>& transforms) :
	_transforms(transforms)
    {
    }

    void
    setTransforms(const vector<ElementTransformPtr>& transforms)
    {
        _transforms = transforms;
    }

    virtual void
    transform(::IceUtil::XMLOutput& os, const DocumentInfoPtr& info, const string&, const DOM_Node& node)
    {
	if(node.isNull())
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}
	os << ::IceUtil::se(toString(node.getNodeName()));
	DOM_NamedNodeMap attributes = node.getAttributes();
	for(unsigned int i = 0; i < attributes.getLength(); ++i)
	{
	    DOM_Node attribute = attributes.item(i);
	    os << ::IceUtil::attr(toString(attribute.getNodeName()), toString(attribute.getNodeValue()));
	}

	DOM_NodeList children = node.getChildNodes();
	for(vector<ElementTransformPtr>::const_iterator p = _transforms.begin(); p != _transforms.end(); ++p)
	{
	    DOM_Node child = findChild(node, (*p)->namespaceURI(), (*p)->name());
	    (*p)->transform(os, info, (*p)->name(), child);
	}

	os << ::IceUtil::ee;
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[struct]\n";
	for(unsigned int i = 0; i < _transforms.size(); ++i)
	{
	    _transforms[i]->print(os);
	}
	os << "[/struct]\n";
	return os;
    }

private:

    vector<ElementTransformPtr> _transforms;
};

typedef ::IceUtil::Handle<StructTransform> StructTransformPtr;

//
// Transform for an object instance. This reads the object type in the xsi:type attribute. Next the specific
// transform is retrieved from the map and applied. The holder of the actual maps owns the lifecycle of the
// transform map.
//
class ClassTransform : public Transform
{
public:

    ClassTransform(TransformMap* transforms) :
	_transforms(transforms)
    {
    }

    virtual void
    transform(::IceUtil::XMLOutput& os, const DocumentInfoPtr& info, const string& name, const DOM_Node& node)
    {
        //
        // First check for the "href" attribute, which indicates a
        // reference to another object. We simply emit the node
        // as-is. The referenced object, which is a child of the
        // root node, will be transformed automatically.
        //
        static const string hrefAttrName = "href";
        string id = getAttributeByName(node, hrefAttrName);
        if(!id.empty())
        {
            os << node;
            return;
        }

        //
        // Otherwise, xsi:type must be present.
        //
	static const string xsitypeAttrName = "xsi:type";
	string type = getAttributeByName(node, xsitypeAttrName);
	if(type.empty())
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}

	string n = convertQName(type, info);

	//
	// Technically this is only permitted to be a more derived
	// type - however, this will not be enforced here.
	//
	TransformMap::const_iterator p = _transforms->find(n);
	if(p == _transforms->end())
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}
	p->second->transform(os, info, name, node);
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[class]\n";
	return os;
    }

private:

    TransformMap* _transforms;
    
};

typedef ::IceUtil::Handle<ClassTransform> ClassTransformPtr;

//
// This transform produces a new struct. The contents of the struct will contain whatever default value is
// appropriate.
//
class DefaultInitializedStructTransform : public Transform
{
public:

    DefaultInitializedStructTransform(const vector<ElementTransformPtr>& transforms) :
	_transforms(transforms)
    {
    }

    virtual void
    transform(::IceUtil::XMLOutput& os, const DocumentInfoPtr& info, const string& name, const DOM_Node& node)
    {
	os << ::IceUtil::se(name);

	DOM_Node child; // Nil

	for(vector<ElementTransformPtr>::const_iterator p = _transforms.begin(); p != _transforms.end(); ++p)
	{
	    (*p)->transform(os, info, (*p)->name(), child);
	}

	os << ::IceUtil::ee;
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[empty struct]\n";
	for(unsigned int i = 0; i < _transforms.size(); ++i)
	{
	    _transforms[i]->print(os);
	}
	os << "[/struct end]\n";
	return os;
    }

private:

    vector<ElementTransformPtr> _transforms;
};

typedef ::IceUtil::Handle<DefaultInitializedStructTransform> DefaultInitializedStructTransformPtr;

//
// This transform is used in building up new values. It emits a known tag & string.
//
class EmitStringTransform : public Transform
{
public:

    EmitStringTransform(const string& s) :
	_s(s)
    {
    }

    virtual void
    transform(::IceUtil::XMLOutput& os, const DocumentInfoPtr&, const string& name, const DOM_Node&)
    {
	os << ::IceUtil::se(name);
	os << ::IceUtil::startEscapes << _s << ::IceUtil::endEscapes;
	os << ::IceUtil::ee;
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[emit string: \"" << _s << "\"]\n";
	return os;
    }

private:

    string _s;
};

typedef ::IceUtil::Handle<EmitStringTransform> EmitStringTransformPtr;

//
// This transform is used in building up new values. It emits a known tag with a given attribute.
//
class EmitAttributeTransform : public Transform
{
public:

    EmitAttributeTransform(const string& attrName, const string& attrValue) :
	_attrName(attrName),
        _attrValue(attrValue)
    {
    }

    virtual void
    transform(::IceUtil::XMLOutput& os, const DocumentInfoPtr&, const string& name, const DOM_Node&)
    {
	os << ::IceUtil::se(name);
        os << ::IceUtil::attr(_attrName, _attrValue);
	os << ::IceUtil::ee;
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[emit attribute: " << _attrName << "=\"" << _attrValue << "\"]\n";
	return os;
    }

private:

    string _attrName;
    string _attrValue;

};

typedef ::IceUtil::Handle<EmitAttributeTransform> EmitAttributeTransformPtr;

//
// Transform factory. Process old & new schema document to produce set of transforms between old & new
// instance documents for the described types.
//
class TransformFactory
{
public:

    TransformFactory(const Ice::StringSeq&);
    ~TransformFactory();

    void create(const DOM_Document&, const DOM_Document&, TransformMap*, TransformMap*);

private:

    friend struct StringTypeTable;
    enum Type
    {
	TypeInteger, // byte, short, int, long
	TypeFloat, // float, double
	TypeString,
	TypeEnumeration,
	TypeStruct,
	TypeClass,
	TypeException,
	TypeDictionary,
	TypeSequence,
	TypeProxy,
	TypeReference,
	TypeInternal
    };

    //
    // Schema import handling.
    //
    void import(DocumentMap&, const string&, const string&);
    void processImport(const DOM_Document&, DocumentMap&);

    //
    // Element processing.
    //
    void processElements(const DocumentInfoPtr& info);

    //
    // Type searching primitives.
    //
    bool findTypeInDocument(const DOM_Document& doc, const string& local, DOM_Node&);
    bool findType(const DocumentMap&, const DocumentInfoPtr&, const string&, DOM_Node&, DocumentInfoPtr&);

    Type getType(const DOM_Node&);
    Type getTypeByName(const DocumentMap&, const DocumentInfoPtr&, const string&, DOM_Node&, DocumentInfoPtr&);

    //
    // Top-level transform creation routine.
    //
    TransformPtr createTransform(const DocumentInfoPtr&, const string&,
				 const DocumentInfoPtr&, const string&);

    //
    // Subsequent creation routines.
    //
    TransformPtr createComplexTypeTransform(const DOM_Node&, const DOM_Node&);
    TransformPtr createSimpleTypeTransform(const DOM_Node&, const DOM_Node&);

    TransformPtr createStaticClassTransform(const DOM_Node&, const DOM_Node&);
    void createEnumValues(const DOM_Node&, vector< string>&);
    TransformPtr createEnumerationTransform(const DOM_Node&);

    void createSequenceElementTransform(const DocumentInfoPtr&, const DOM_Node&,
					const DocumentInfoPtr&, const DOM_Node&,
					vector<ElementTransformPtr>&);
    void createClassContentTransform(const DocumentInfoPtr&, const DOM_Node&,
				     const DocumentInfoPtr&, const DOM_Node&,
				     vector<ElementTransformPtr>&);

    //
    // Top-level transform creation routine for new datatypes (that is datatypes which were not contained in
    // the old schema).
    //
    TransformPtr createDefaultInitializedTransform(const DocumentInfoPtr&, const string&);

    //
    // Subsequent creation routines for new datatypes.
    //
    void createDefaultInitializedSequenceElementTransform(const DocumentInfoPtr&, const DOM_Node&,
							  vector<ElementTransformPtr>&);

    //
    // Utility routines.
    //
    DOM_Node findSchemaRoot(const DOM_Document&);

    //
    // Search paths for importing schemas.
    //
    Ice::StringSeq _paths;

    //
    // Map of local@uri class transforms (based on static type). This information cached for creation of the
    // transform.
    //
    TransformMap* _staticClassTransforms;

    //
    // Map of local@uri element names to transforms. Needed for actual transform.
    //
    TransformMap* _elements;
    
    //
    // Set of documents. The set of all documents is the entire document set.
    //
    // The map maps from targetNamespace to DocumentInfo.
    //
    DocumentMap _fromDocs;
    DocumentMap _toDocs;

    //
    // Map of qualified type names to transform. This information cached for creation of the transform.
    //
    TransformMap _types;

    //
    // Map of local@uri transforms for creating new types. This information cached for creation of the
    // transform.
    //
    TransformMap _defaultInitializedTransforms;
};

} // End of namespace XMLTransform

//
// Constructor & destructor.
//
XMLTransform::TransformFactory::TransformFactory(const Ice::StringSeq& paths) :
    _paths(paths)
{
}

XMLTransform::TransformFactory::~TransformFactory()
{
}

//
// Member function implementation.
//

//
// Create a transform set for the given old schema (fromDoc) and the new schema (toDoc). The result is two
// maps: A map of local@uri -> element transform and a map of transforms for specific class types.
//
void
XMLTransform::TransformFactory::create(const DOM_Document& fromDoc, const DOM_Document& toDoc, TransformMap* elements,
                                       TransformMap* staticClassTransforms)
{
    //
    // Setup member state.
    //
    _elements = elements;
    _staticClassTransforms = staticClassTransforms;
    _fromDocs.clear();
    _toDocs.clear();
    _types.clear();
    _defaultInitializedTransforms.clear();

    //
    // Create both of the document infos for the old & new schemas.
    // Add the root documents to the document map.
    //
    DOM_Node fromSchema = findSchemaRoot(fromDoc);
    assert(!fromSchema.isNull());
    DocumentInfoPtr fromInfo = new DocumentInfo(fromDoc, fromSchema);

    DOM_Node toSchema = findSchemaRoot(toDoc);
    assert(!toSchema.isNull());
    DocumentInfoPtr toInfo = new DocumentInfo(toDoc, toSchema);
    
    _fromDocs.insert(make_pair(fromInfo->getTargetNamespace(), fromInfo));
    _toDocs.insert(make_pair(toInfo->getTargetNamespace(), toInfo));

    //
    // Process the import declarations for the source schema documents.
    //
    processImport(fromDoc, _fromDocs);
    processImport(toDoc, _toDocs);

    //
    // Finally process each element from the old schema document.
    //
    for(DocumentMap::const_iterator p = _fromDocs.begin(); p != _fromDocs.end(); ++p)
    {
	processElements(p->second);
    }
}

void
XMLTransform::TransformFactory::import(DocumentMap& documents, const string& ns, const string& loc)
{
    DOMTreeErrorReporter errorReporter;
    DOMParser parser;
    parser.setValidationScheme(DOMParser::Val_Never);
    parser.setDoNamespaces(true);
    parser.setErrorHandler(&errorReporter);

    try
    {
        string file = findFile(loc, _paths);
        parser.parse(file.c_str());
	if(errorReporter.getSawErrors())
	{
	    InvalidSchema ex(__FILE__, __LINE__);
            ex.reason = errorReporter.getErrors();
            throw ex;
	}
    }
    catch(const XMLException& ex)
    {
	InvalidSchema e(__FILE__, __LINE__);
        e.reason = "XML exception while importing " + loc + ": " + toString(ex.getMessage());
	throw e;
    }
    catch(const SAXParseException& ex)
    {
	InvalidSchema e(__FILE__, __LINE__);
        e.reason = "SAX exception while importing " + loc + ": " + toString(ex.getMessage());
	throw e;
    }
    catch(...)
    {
	InvalidSchema e(__FILE__, __LINE__);
        e.reason = "unknown exception while importing " + loc;
	throw e;
    }

    DOM_Document document = parser.getDocument();
    DOM_Node schema = findSchemaRoot(document);

    DocumentInfoPtr info = new DocumentInfo(document, schema, ns);
    documents.insert(make_pair(info->getTargetNamespace(), info));

    //
    // Process any imports in the imported document.
    //
    processImport(document, documents);
}

void
XMLTransform::TransformFactory::processImport(const DOM_Document& parent, DocumentMap& documents)
{
    DOM_Node schema = findSchemaRoot(parent);
    assert(!schema.isNull());

    DOM_Node child = schema.getFirstChild();
    while(!child.isNull())
    {
	string nodeName = toString(child.getNodeName());
	if(nodeName == importElementName)
	{
	    string ns = getAttributeByName(child, "namespace");
	    string loc = getAttributeByName(child, "schemaLocation");
	    
	    import(documents, ns, loc);
	}
	child = child.getNextSibling();
    }
}

void
XMLTransform::TransformFactory::processElements(const DocumentInfoPtr& info)
{
    DOM_Node schema = findSchemaRoot(info->getDocument());

    DOM_NodeList children = schema.getChildNodes();
    for(unsigned int i = 0; i < children.getLength(); ++i)
    {
	DOM_Node child = children.item(i);
	if(child.getNodeType() != DOM_Node::ELEMENT_NODE)
	{
	    continue;
	}

	string nodeName = toString(child.getNodeName());
	if(nodeName != elementElementName)
	{
	    continue;
	}

	//
	// The element name must be an unqualified name.
	//
	string nameAttr = getNameAttribute(child);
	assert(nameAttr.find(':') == string::npos);

	DOM_Node to;
	DocumentInfoPtr toNodeInfo; // Overrides the top-level toInfo.
	if(!findType(_toDocs, info, nameAttr, to, toNodeInfo))
	{
	    //
	    // No equivalent in the new schema.
	    //
	    continue;
	}
	assert(!to.isNull() && toNodeInfo);

	string toNodeName = toString(to.getNodeName());
	string toNameAttr = getNameAttribute(to);
	assert(toNameAttr == nameAttr); // Sanity check.
	
	//
	// Construct the full element name - local@uri.
 	//
	string fullElementName = nameAttr;
	fullElementName += '@';
	fullElementName += info->getTargetNamespace();
	
	//
	// Redefinitions of elements is not permitted.
	//
	if(_elements->find(fullElementName) != _elements->end())
	{
	    InvalidSchema ex(__FILE__, __LINE__);
            ex.reason = "redefinition of element " + nameAttr;
	    throw ex;
	}
	
	string fromTypeName = getTypeAttribute(child);
	string toTypeName = getTypeAttribute(to);

	//
	// Ignore anonymous elements (operation contents).
	//
	if(fromTypeName.empty() && toTypeName.empty())
	{
	    continue;
	}

	//
	// However, it's not legal for an element to change type.
	//
	if(fromTypeName.empty() || toTypeName.empty())
	{
	    InvalidSchema ex(__FILE__, __LINE__);
            ex.reason = "element " + nameAttr + " has changed type";
	    throw ex;
	}

	TransformPtr transform = createTransform(info, fromTypeName, toNodeInfo, toTypeName);

	//
	// Add association between name & transform
	//
	_elements->insert(make_pair(fullElementName, transform));
    }
}

//
// Search for a type in a particular document. Return true if the type is present, false otherwise.
//
bool
XMLTransform::TransformFactory::findTypeInDocument(const DOM_Document& doc, const string& local, DOM_Node& target)
{
    DOM_Node schema = findSchemaRoot(doc);

    DOM_NodeList children = schema.getChildNodes();
    unsigned int i;
    for(i = 0; i < children.getLength(); ++i)
    {
	DOM_Node child = children.item(i);
	if(child.getNodeType() == DOM_Node::ELEMENT_NODE)
	{
	    string na = getNameAttribute(child);
	    if(na == local)
	    {
		target = child;
		return true;
	    }
	}
    }

    return false;
}

//
// This find method takes the type QName and then maps the prefix portion to a namespace in the current
// document (info). Next the documents which have the given targetNamespace are searched for an element node
// with that name. Return true if the type is found, false otherwise.
//
bool
XMLTransform::TransformFactory::findType(const DocumentMap& docs, const DocumentInfoPtr& info, const string& type,
                                         DOM_Node& n, DocumentInfoPtr& nInfo)
{
    string uri;
    string local;

    size_t pos = type.find(':');
    if(pos != string::npos)
    {
	string prefix = type.substr(0, pos);
	uri = info->findURI(prefix);
	if(uri.empty())
	{
	    InvalidSchema ex(__FILE__, __LINE__);
            ex.reason = "no URI found for prefix " + prefix;
	    throw ex;
	}
	local = type.substr(pos + 1);
    }
    else
    {
	uri = info->getTargetNamespace();
	local = type;
    }

    //
    // Run through each document with targetNamespace == uri. If the type is contained in the document, then
    // return the node & the document info.
    //
    pair<DocumentMap::const_iterator, DocumentMap::const_iterator> range = docs.equal_range(uri);
    for(DocumentMap::const_iterator q = range.first; q != range.second; ++q)
    {
	if(findTypeInDocument(q->second->getDocument(), local, n))
	{
	    nInfo = q->second;
	    return true;
	}
    }

    return false;
}

namespace XMLTransform
{

//
// Why won't MSVC allow name to be a string?
//
struct StringTypeTable
{
    const char* name; // COMPILERFIX: const string name;
    TransformFactory::Type type;
    
    bool operator==(const string& rhs) const { return string(name) == rhs; }
};

}

TransformFactory::Type
XMLTransform::TransformFactory::getType(const DOM_Node& node)
{
    //
    // Check the appinfo element for the actual type.
    //
    static const StringTypeTable items[] =
    {
	{ "enumeration", TypeEnumeration },
	{ "struct", TypeStruct },
	{ "class", TypeClass },
	{ "exception", TypeException },
	{ "dictionary", TypeDictionary },
	{ "sequence", TypeSequence },
	{ "proxy", TypeProxy },
	{ "reference", TypeReference },
	{ "internal", TypeInternal }
    };
    static const StringTypeTable* begin = &items[0];
    static const StringTypeTable* end = &items[sizeof(items)/sizeof(items[0])];

    string type;

    //
    // Locate the annotation/appinfo/type node.
    //
    DOM_Node child = findChild(node, annotationElementName);
    if(!child.isNull())
    {
	child = findChild(child, appinfoElementName);
	if(!child.isNull())
	{
	    child = findChild(child, "type");
	    if(!child.isNull())
	    {
		child = child.getFirstChild();
		if(!child.isNull())
		{
		    type = toString(child.getNodeValue());
		}
	    }
	}
    }

    const StringTypeTable* p = find(begin, end, type);
    if(p == end)
    {
        InvalidSchema ex(__FILE__, __LINE__);
        ex.reason = "no type found for element " + getNameAttribute(node);
	throw ex;
    }

    return p->type;
}

TransformFactory::Type
XMLTransform::TransformFactory::getTypeByName(const DocumentMap& docs, const DocumentInfoPtr& info, const string& type,
                                              DOM_Node& n, DocumentInfoPtr& nInfo)
{
    //
    // First check to see if the type is a primitive schema type.
    //
    static const StringTypeTable items[] =
    {
	{ "xs:byte", TypeInteger },
	{ "xs:short", TypeInteger },
	{ "xs:int", TypeInteger },
	{ "xs:long", TypeInteger },
	{ "xs:float", TypeFloat },
	{ "xs:double", TypeFloat },
	{ "xs:string", TypeString },
    };
    static const StringTypeTable* begin = &items[0];
    static const StringTypeTable* end = &items[sizeof(items)/sizeof(items[0])];

    const StringTypeTable* p = find(begin, end, type);
    if(p != end)
    {
	return p->type;
    }

    if(!findType(docs, info, type, n, nInfo))
    {
	InvalidSchema ex(__FILE__, __LINE__);
        ex.reason = "no node found for type " + type;
	throw ex;
    }
    assert(!n.isNull() && nInfo);

    return getType(n);
}

TransformPtr
XMLTransform::TransformFactory::createTransform(const DocumentInfoPtr& fromTypeInfo, const string& fromTypeName,
                                                const DocumentInfoPtr& toTypeInfo, const string& toTypeName)
{
    DOM_Node from;
    DocumentInfoPtr fromInfo;
    Type fromType = getTypeByName(_fromDocs, fromTypeInfo, fromTypeName, from, fromInfo);

    DOM_Node to;
    DocumentInfoPtr toInfo;
    Type toType = getTypeByName(_toDocs, toTypeInfo, toTypeName, to, toInfo);

    TransformMap::const_iterator p = _types.find(fromTypeName);
    if(p != _types.end())
    {
	return p->second;
    }

    TransformPtr transform = 0;

    //
    // First handle transforms where the types are equivalent.
    //
    if(fromType == toType)
    {
	switch(fromType)
	{
	case TypeInteger:
	{
            //
            // Don't cache this transform.
            //
	    return new ValidateIntegerTransform(fromTypeName, toTypeName);
	}

	case TypeFloat:
	{
            //
            // Don't cache this transform.
            //
	    return new ValidateFloatTransform(fromTypeName, toTypeName);
	}

	case TypeString:
	case TypeProxy: // Same as string
	case TypeReference: // Same as string
	{
	    transform = new NilTransform();
	    break;
	}

	case TypeEnumeration:
	{
	    //
	    // If the type names are not the same then it's illegal.
	    //
	    // TODO: This doesn't allow the renaming of types. By removing this comparison renaming of types
	    // would be permitted. Should this be permitted?
	    //
	    if(fromTypeName != toTypeName)
	    {
		IllegalTransform ex(__FILE__, __LINE__);
                ex.reason = "renaming types is not supported (from " + fromTypeName + " to " + toTypeName + ")";
		throw ex;
	    }

	    transform = createEnumerationTransform(to);
	    break;
	}

	case TypeStruct:
	{
	    //
	    // If the type names are not the same then it's illegal.
	    //
	    // TODO: This doesn't allow the renaming of types. By removing this comparison renaming of types
	    // would be permitted. Should this be permitted?
	    //
	    if(fromTypeName != toTypeName)
	    {
		IllegalTransform ex(__FILE__, __LINE__);
                ex.reason = "renaming types is not supported (from " + fromTypeName + " to " + toTypeName + ")";
		throw ex;
	    }

	    vector<ElementTransformPtr> v;
	    createSequenceElementTransform(fromInfo, findChild(from, sequenceElementName),
					   toInfo, findChild(to, sequenceElementName), v);
	    transform = new StructTransform(v);
	    break;
	}

	case TypeDictionary:
	{
	    //
	    // If the type names are not the same then it's illegal.
	    //
	    // TODO: This doesn't allow the renaming of types. By removing this comparison renaming of types
	    // would be permitted. Should this be permitted?
	    //
	    if(fromTypeName != toTypeName)
	    {
		IllegalTransform ex(__FILE__, __LINE__);
                ex.reason = "renaming types is not supported (from " + fromTypeName + " to " + toTypeName + ")";
		throw ex;
	    }

	    DOM_Node fromSeq = findChild(from, sequenceElementName);
	    DOM_Node toSeq = findChild(to, sequenceElementName);
	    if(fromSeq.isNull() || toSeq.isNull())
	    {
		InvalidSchema ex(__FILE__, __LINE__);
                ex.reason = "missing sequence element in " + fromTypeName;
		throw ex;
	    }
	    
	    //
	    // Sequences have one element - which contains the type of the sequence.
	    //
	    DOM_Node fromElement = findChild(fromSeq, elementElementName);
	    DOM_Node toElement = findChild(toSeq, elementElementName);
	    
	    if(fromElement.isNull() || toElement.isNull())
	    {
		InvalidSchema ex(__FILE__, __LINE__);
                ex.reason = "invalid sequence element in " + fromTypeName;
		throw ex;
	    }
	    
	    transform = new SequenceTransform(createTransform(fromInfo, getTypeAttribute(fromElement),
							      toInfo, getTypeAttribute(toElement)));
	    break;
	}

	case TypeSequence:
	{
	    //
	    // If the type names are not the same then it's illegal.
	    //
	    // TODO: This doesn't allow the renaming of types. By removing this comparison renaming of types
	    // would be permitted. Should this be permitted?
	    //
	    if(fromTypeName != toTypeName)
	    {
		IllegalTransform ex(__FILE__, __LINE__);
                ex.reason = "renaming types is not supported (from " + fromTypeName + " to " + toTypeName + ")";
		throw ex;
	    }

	    DOM_Node fromSeq = findChild(from, sequenceElementName);
	    DOM_Node toSeq = findChild(to, sequenceElementName);
	    if(fromSeq.isNull() || toSeq.isNull())
	    {
		InvalidSchema ex(__FILE__, __LINE__);
                ex.reason = "missing sequence element in " + fromTypeName;
		throw ex;
	    }
	    
	    //
	    // Sequences have one element - which contains the type of the sequence.
	    //
	    DOM_Node fromElement = findChild(fromSeq, elementElementName);
	    DOM_Node toElement = findChild(toSeq, elementElementName);
	    if(fromElement.isNull() || toElement.isNull())
	    {
		InvalidSchema ex(__FILE__, __LINE__);
                ex.reason = "invalid sequence element in " + fromTypeName;
		throw ex;
	    }
	    
	    transform = new SequenceTransform(createTransform(fromInfo, getTypeAttribute(fromElement),
							      toInfo, getTypeAttribute(toElement)));
	    break;
	}

	case TypeClass:
	{
	    //
	    // If the type names are not the same then it's illegal.
	    //
	    // TODO: This doesn't allow the renaming of types. By removing this comparison renaming of types
	    // would be permitted. Should this be permitted?
	    //
	    if(fromTypeName != toTypeName)
	    {
		IllegalTransform ex(__FILE__, __LINE__);
                ex.reason = "renaming types is not supported (from " + fromTypeName + " to " + toTypeName + ")";
		throw ex;
	    }

	    string type = getNameAttribute(to);
	    type += '@';
	    type += toInfo->getTargetNamespace();
	    
	    if(_staticClassTransforms->find(type) == _staticClassTransforms->end())
	    {
                StructTransform* st = new StructTransform;
                _staticClassTransforms->insert(make_pair(type, st));

                vector<ElementTransformPtr> v;
                createClassContentTransform(fromInfo, from, toInfo, to, v);

                st->setTransforms(v);
            }

	    return new ClassTransform(_staticClassTransforms);
	}

	case TypeInternal:
	{
	    //
	    // No transformation created for internal stuff.
	    //
	    transform = new InternalTransform();
	    break;
	}

	case TypeException:
	default:
        {
	    IllegalTransform ex(__FILE__, __LINE__);
            ex.reason = "invalid type";
	    throw ex;
        }
	}
    }

    if(!transform)
    {
	//
	// Next we have transforms from type-to-type.
	//
	if(fromType == TypeString && toType == TypeEnumeration)
	{
	    //
	    // String to enumeration transform needs to validate the string as a member of the enumeration
	    // values.
	    //
	    transform = createEnumerationTransform(to);
	}
	else if(fromType == TypeEnumeration && toType == TypeString)
	{
	    //
	    // Enumeration to string transform is nil transform.
	    //
	    transform = new NilTransform();
	}
    }

    //
    // TODO: struct->class, class->struct.
    //
    
    if(!transform)
    {
	IllegalTransform ex(__FILE__, __LINE__);
        ex.reason = "cannot transform from " + fromTypeName + " to " + toTypeName;
	throw ex;
    }

    _types.insert(make_pair(fromTypeName, transform));

    return transform;
}

void
XMLTransform::TransformFactory::createSequenceElementTransform(const DocumentInfoPtr& fromInfo, const DOM_Node& from,
                                                               const DocumentInfoPtr& toInfo, const DOM_Node& to,
                                                               vector<ElementTransformPtr>& v)
{
    //
    // Precondition: The nodes must not be nil.
    //
    if(from.isNull() || to.isNull())
    {
	throw InvalidSchema(__FILE__, __LINE__);
    }

    //
    // Allowable transforms:
    //
    // * Node added.
    // * Node removed.
    // * Node moved.
    //

    DOM_NodeList fromSeqChildren = from.getChildNodes();
    DOM_NodeList toSeqChildren = to.getChildNodes();

    //
    // First run through the to set. This loop handles the node
    // remove, and node changed transforms (plus allowable type
    // changes).
    //
    for(unsigned int i = 0; i < toSeqChildren.getLength(); ++i)
    {
	ElementTransformPtr transform;
	DOM_Node toChild = toSeqChildren.item(i);
	if(toChild.getNodeType() != DOM_Node::ELEMENT_NODE)
	{
	    continue;
	}

	if(toString(toChild.getNodeName()) != elementElementName)
	{
	    InvalidSchema ex(__FILE__, __LINE__);
            ex.reason = "expected " + elementElementName + " but found " + toString(toChild.getNodeName());
	    throw ex;
	}

	string toElementName = getNameAttribute(toChild);
	string toTypeName = getTypeAttribute(toChild);

	//
	// Search for the node in the fromSeqChildren list.
	//
	for(unsigned int j = 0; j < fromSeqChildren.getLength(); ++j)
	{
	    DOM_Node fromChild = fromSeqChildren.item(j);

	    if(fromChild.getNodeType() != DOM_Node::ELEMENT_NODE)
	    {
		// Skip non element nodes.
		continue;
	    }

	    if(toString(fromChild.getNodeName()) != elementElementName)
	    {
		InvalidSchema ex(__FILE__, __LINE__);
                ex.reason = "expected " + elementElementName + " but found " + toString(fromChild.getNodeName());
		throw ex;
	    }

	    string fromElementName = getNameAttribute(fromChild);

	    if(fromElementName == toElementName)
	    {
		transform = new ElementTransform(toInfo->getTargetNamespace(), toElementName,
						 createTransform(fromInfo, getTypeAttribute(fromChild),
								 toInfo, toTypeName));
	    }
	}

	//
	// If there is no transform then this is a new node type. Create a transform to add an empty element
	// of the appropriate type.
	//
	if(!transform)
	{
	    transform = new ElementTransform(toInfo->getTargetNamespace(), toElementName,
					     createDefaultInitializedTransform(toInfo, toTypeName));
	}

	v.push_back(transform);
    }
}

void
XMLTransform::TransformFactory::createClassContentTransform(const DocumentInfoPtr& fromInfo, const DOM_Node& from,
                                                            const DocumentInfoPtr& toInfo, const DOM_Node& to,
                                                            vector<ElementTransformPtr>& v)
{
    static const string baseAttrName = "base";

    DOM_Node fromContent = findChild(from, complexContentElementName);
    DOM_Node toContent = findChild(to, complexContentElementName);

    if(fromContent.isNull() && toContent.isNull())
    {
	//
	// Must be base of a class hierarchy (while this implementation is a little more flexible, with Ice it
	// is limited to ::Ice::Object).
	//
	createSequenceElementTransform(fromInfo, findChild(from, sequenceElementName),
				       toInfo, findChild(to, sequenceElementName), v);
	return;
    }

    if(fromContent.isNull() || toContent.isNull())
    {
	throw InvalidSchema(__FILE__, __LINE__);
    }

    DOM_Node fromExtension = findChild(fromContent, extensionElementName);
    DOM_Node toExtension = findChild(toContent, extensionElementName);

    if(fromExtension.isNull() || toExtension.isNull())
    {
	throw InvalidSchema(__FILE__, __LINE__);
    }

    string fromBaseName = getAttributeByName(fromExtension, baseAttrName);
    string toBaseName = getAttributeByName(toExtension, baseAttrName);

    //
    // It's not legal to change the base class.
    //
    if(fromBaseName != toBaseName)
    {
	IllegalTransform ex(__FILE__, __LINE__);
        ex.reason = "can't change base class from " + fromBaseName + " to " + toBaseName;
	throw ex;
    }

    DOM_Node fromBaseNode;
    DocumentInfoPtr fromBaseInfo;
    if(!findType(_fromDocs, fromInfo, fromBaseName, fromBaseNode, fromBaseInfo))
    {
	SchemaViolation ex(__FILE__, __LINE__);
        ex.reason = "unable to find type for " + fromBaseName;
	throw ex;
    }
    assert(!fromBaseNode.isNull() && fromBaseInfo);

    DOM_Node toBaseNode;
    DocumentInfoPtr toBaseInfo;
    if(!findType(_toDocs, toInfo, toBaseName, toBaseNode, toBaseInfo))
    {
	SchemaViolation ex(__FILE__, __LINE__);
        ex.reason = "unable to find type for " + toBaseName;
	throw ex;
    }
    assert(!toBaseNode.isNull() && toBaseInfo);

    //
    // Find the content transform for the base type.
    //
    createClassContentTransform(fromBaseInfo, fromBaseNode, toBaseInfo, toBaseNode, v);

    createSequenceElementTransform(fromInfo, findChild(fromExtension, sequenceElementName),
				   toInfo, findChild(toExtension, sequenceElementName), v);
}

void
XMLTransform::TransformFactory::createEnumValues(const DOM_Node& to, vector<string>& values)
{
    DOM_Node toRes = findChild(to, restrictionElementName);
    if(toRes.isNull())
    {
	InvalidSchema ex(__FILE__, __LINE__);
        ex.reason = "didn't find " + restrictionElementName + " in " + toString(to.getNodeName());
	throw ex;
    }

    //
    // Gather up a list of allowable values.
    //
    DOM_NodeList toResChildren = toRes.getChildNodes();

    for(unsigned int i = 0; i < toResChildren.getLength(); ++i)
    {
	DOM_Node toChild = toResChildren.item(i);

	if(toChild.getNodeType() != DOM_Node::ELEMENT_NODE)
	{
	    continue;
	}

	if(toString(toChild.getNodeName()) != enumerationElementName)
	{
	    InvalidSchema ex(__FILE__, __LINE__);
            ex.reason = "expected " + enumerationElementName + " but found " + toString(toChild.getNodeName());
	    throw ex;
	}

	string value = getAttributeByName(toChild, "value");
	if(value.empty())
	{
	    InvalidSchema ex(__FILE__, __LINE__);
            ex.reason = "didn't find value attribute in " + toString(toChild.getNodeName());
	    throw ex;
	}

	values.push_back(value);
    }
}

TransformPtr
XMLTransform::TransformFactory::createEnumerationTransform(const DOM_Node& to)
{
    vector<string> values;
    createEnumValues(to, values);

    return new ValidateEnumerationTransform(values);
}

TransformPtr
XMLTransform::TransformFactory::createDefaultInitializedTransform(const DocumentInfoPtr& info, const string& typeName)
{
    string fullTypeName = convertQName(typeName, info);
    TransformMap::const_iterator p = _defaultInitializedTransforms.find(fullTypeName);
    if(p != _defaultInitializedTransforms.end())
    {
	//
	// Return cached empty transform.
	//
	return p->second;
    }

    DOM_Node n;
    DocumentInfoPtr nInfo;
    Type type = getTypeByName(_toDocs, info, typeName, n, nInfo);

    TransformPtr transform = 0;
    switch(type)
    {
    case TypeInteger:
	transform = new EmitStringTransform("0");
	break;

    case TypeFloat:
	transform = new EmitStringTransform("0.0");
	break;

    case TypeProxy:
    case TypeReference:
    case TypeString:
	//
	// Default string, reference & proxy is empty.
	//
	transform = new EmitStringTransform("");
	break;

    case TypeEnumeration:
    {
	if(n.isNull())
	{
	    throw InvalidSchema(__FILE__, __LINE__);
	}

	vector<string> values;
	createEnumValues(n, values);
	transform = new EmitStringTransform(values[0]);
	break;
    }
	
    case TypeStruct:
    {
	if(n.isNull())
	{
	    throw InvalidSchema(__FILE__, __LINE__);
	}

	DOM_Node seq = findChild(n, sequenceElementName);
	if(seq.isNull())
	{
	    InvalidSchema ex(__FILE__, __LINE__);
            ex.reason = "didn't find " + sequenceElementName + " in struct";
	    throw ex;
	}
	
	vector<ElementTransformPtr> v;
	createDefaultInitializedSequenceElementTransform(info, seq, v);
	transform = new DefaultInitializedStructTransform(v);
	break;
    }

    case TypeDictionary:
    case TypeSequence:
    {
	if(n.isNull())
	{
	    throw InvalidSchema(__FILE__, __LINE__);
	}
	transform = new EmitAttributeTransform("length", "0");
	break;
    }

    case TypeInternal:
    case TypeException:
    case TypeClass:
    default:
    {
	IllegalTransform ex(__FILE__, __LINE__);
        ex.reason = "cannot create default initialized value";
	throw ex;
    }
    }

    if(!transform)
    {
	IllegalTransform ex(__FILE__, __LINE__);
        ex.reason = "can't create default transform for " + typeName;
	throw ex;
    }

    _defaultInitializedTransforms.insert(make_pair(fullTypeName, transform));

    return transform;
}

void
XMLTransform::TransformFactory::createDefaultInitializedSequenceElementTransform(const DocumentInfoPtr& info,
                                                                                 const DOM_Node& node,
                                                                                 vector<ElementTransformPtr>& v)
{
    //
    // Allowable transforms:
    //
    // * Node added.
    // * Node removed.
    // * Node moved.
    //

    DOM_NodeList seqChild = node.getChildNodes();
    for(unsigned int i = 0; i < seqChild.getLength(); ++i)
    {
	DOM_Node child = seqChild.item(i);
	if(child.getNodeType() != DOM_Node::ELEMENT_NODE)
	{
	    continue;
	}

	if(toString(child.getNodeName()) != elementElementName)
	{
	    InvalidSchema ex(__FILE__, __LINE__);
            ex.reason = "expected " + elementElementName + " but found " + toString(child.getNodeName());
	    throw ex;
	}

	v.push_back(new ElementTransform(info->getTargetNamespace(), getNameAttribute(child),
					 createDefaultInitializedTransform(info, getTypeAttribute(child))));
    }
}

DOM_Node
XMLTransform::TransformFactory::findSchemaRoot(const DOM_Document& root)
{
    DOMString schemaURI("http://www.w3.org/2001/XMLSchema");
    DOMString schemaLocalName("schema");
    DOM_Node n;

    DOM_NodeList nodes = root.getElementsByTagNameNS(schemaURI, schemaLocalName);
    if(nodes.getLength() != 1)
    {
        InvalidSchema ex(__FILE__, __LINE__);
        ex.reason = "unable to find schema root";
	throw ex;
    }
    return nodes.item(0);
}


XMLTransform::Transformer::Transformer(const Ice::StringSeq& paths, const DOM_Document& fromDoc,
                                       const DOM_Document& toDoc)
{
    TransformFactory factory(paths);
    factory.create(fromDoc, toDoc, &_elements, &_staticClassTransforms);
}

XMLTransform::Transformer::~Transformer()
{
}

void
XMLTransform::Transformer::transform(::IceUtil::XMLOutput& os, const DOM_Document& doc, bool emitRoot)
{
    DOM_Node root = doc.getFirstChild();

    unsigned int i;

    if(emitRoot)
    {
        os << ::IceUtil::se(toString(root.getNodeName()));
        DOM_NamedNodeMap attributes = root.getAttributes();
        for(i = 0; i < attributes.getLength(); ++i)
        {
            DOM_Node attribute = attributes.item(i);
            os << ::IceUtil::attr(toString(attribute.getNodeName()), toString(attribute.getNodeValue()));
        }
    }

    DocumentInfoPtr info = new DocumentInfo(doc, root);
    
    DOM_NodeList children = root.getChildNodes();
    for(i = 0; i < children.getLength(); ++i)
    {
	DOM_Node child = children.item(i);
	if(child.getNodeType() != DOM_Node::ELEMENT_NODE)
	{
	    continue;
	}

	string nodeName = toString(child.getNodeName());

	//
	// Create local@namespace version of the element name.
	//
	string n = convertQName(nodeName, info);

	TransformMap::const_iterator p = _elements.find(n);
	if(p == _elements.end())
	{
	    SchemaViolation ex(__FILE__, __LINE__);
	    ex.reason = "cannot find element " + n;
	    throw ex;
	}

	p->second->transform(os, info, nodeName, child);
    }

    if(emitRoot)
    {
        os << ::IceUtil::ee;
    }
}

XMLTransform::DBTransformer::DBTransformer()
{
    try
    {
        XMLPlatformUtils::Initialize();
    }
    catch(const XMLException& ex)
    {
        cerr << "Error during xerces initialization: " << toString(ex.getMessage()) << endl;
        throw Ice::SystemException(__FILE__, __LINE__); // TODO: Better exception?
    }
}

XMLTransform::DBTransformer::~DBTransformer()
{
    XMLPlatformUtils::Terminate();
}

void
XMLTransform::DBTransformer::transform(const DBEnvironmentPtr& dbEnv, const DBPtr& db, const Ice::StringSeq& paths,
                                       const string& oldSchemaFile, const string& newSchemaFile)
{
    DOMParser parser;
    parser.setValidationScheme(DOMParser::Val_Auto);
    parser.setDoNamespaces(true);
            
    DOMTreeErrorReporter errReporter;
    parser.setErrorHandler(&errReporter);

    DOM_Document oldSchema, newSchema;

    try
    {
        parser.parse(oldSchemaFile.c_str());
        oldSchema = parser.getDocument();

        parser.parse(newSchemaFile.c_str());
        newSchema = parser.getDocument();
    }
    catch(const XMLException& ex)
    {
        InvalidSchema e(__FILE__, __LINE__);
        e.reason = "XML exception: " + toString(ex.getMessage());
        throw e;
    }   
    catch(const SAXException& ex)
    {
        InvalidSchema e(__FILE__, __LINE__);
        e.reason = "SAX exception: " + toString(ex.getMessage());
        throw e;
    }
    catch(...)
    {   
        InvalidSchema e(__FILE__, __LINE__);
        e.reason = "Unexpected exception";
        throw e;
    }

    DBCursorPtr cursor;
    DBTransactionPtr txn;
    string reason;
    try
    {
        Transformer transformer(paths, oldSchema, newSchema);

        const string header = "<ice:data xmlns=\"http://www.noorg.org/schemas\""
                              " xmlns:ice=\"http://www.mutablerealms.com/schemas\""
                              " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                              " xsi:schemaLocation=\"http://www.noorg.org/schemas " + oldSchemaFile + "\">";
        const string footer = "</ice:data>";

        //
        // The database will be modified in place. Since keys can be changed,
        // we can't use a cursor to perform the changes. We collect all of the
        // keys first, then update the records.
        //
        vector<Key> keys;
        cursor = db->getCursor();
        do
        {
            Key k;
            Value v;
            cursor->curr(k, v);
            keys.push_back(k);
        }
        while(cursor->next());
        cursor->close();
        cursor = 0;

        txn = dbEnv->startTransaction();

        vector<Key>::const_iterator p;
        for(p = keys.begin(); p != keys.end(); ++p)
        {
            const Key& k = *p;

            //
            // Transform key
            //
            string fullKey;
            fullKey.append(header);
            fullKey.append(&k[0], k.size());
            fullKey.append(footer);
            MemBufInputSource keySource((const XMLByte*)fullKey.data(), fullKey.size(), "key");
            parser.parse(keySource);
            DOM_Document keyDoc = parser.getDocument();

            ostringstream keyStream;
            IceUtil::XMLOutput keyOut(keyStream);
            transformer.transform(keyOut, keyDoc, false);

            Key newKey;
            const std::string& keyStr = keyStream.str();
            newKey.resize(keyStr.size());
            std::copy(keyStr.begin(), keyStr.end(), newKey.begin());

            //
            // Transform value
            //
            Value value = db->get(k);
            string fullValue;
            fullValue.append(header);
            fullValue.append(&value[0], value.size());
            fullValue.append(footer);
            MemBufInputSource valueSource((const XMLByte*)fullValue.data(), fullValue.size(), "value");
            parser.parse(valueSource);
            DOM_Document valueDoc = parser.getDocument();

            ostringstream valueStream;
            IceUtil::XMLOutput valueOut(valueStream);
            transformer.transform(valueOut, valueDoc, false);

            Value newValue;
            const std::string& valueStr = valueStream.str();
            newValue.resize(valueStr.size());
            std::copy(valueStr.begin(), valueStr.end(), newValue.begin());

            //
            // Update database - only insert new key,value pair if the transformed
            // key doesn't match an existing key.
            //
            db->del(k);
            if(db->contains(newKey))
            {
                reason = "transformed key matches an existing record:\n" + keyStr;
                txn->abort();
                txn = 0;
                break;
            }
            db->put(newKey, newValue);
        }

        if(txn)
        {
            txn->commit();
            txn = 0;
        }
    }
    catch(const DBNotFoundException&)
    {
        // Database is empty
    }
    catch(const XMLException& ex)
    {
        reason = "XML exception: " + toString(ex.getMessage());
    }   
    catch(const SAXException& ex)
    {
        reason = "SAX exception: " + toString(ex.getMessage());
    }
    catch(const DOM_DOMException& ex)
    {
        ostringstream out;
        out << "DOM exception (" << ex.code << ") " << toString(ex.msg);
        reason = out.str();
    }
    catch(...)
    {
        if(cursor)
        {
            cursor->close();
        }
        if(txn)
        {
            txn->abort();
        }
        throw;
    }

    if(cursor)
    {
        cursor->close();
    }

    if(txn)
    {
        txn->abort();
    }

    if(!reason.empty())
    {
        TransformException ex(__FILE__, __LINE__);
        ex.reason = reason;
        throw ex;
    }
}
