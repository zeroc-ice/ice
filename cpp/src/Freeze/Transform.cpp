// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Freeze/Transform.h>

#include <vector>
#include <assert.h>
#include <iostream>
#include <Freeze/Print.h>

#include <Freeze/ErrorHandler.h>
#include <parsers/DOMParser.hpp>

using namespace std;

ostream&
operator<<(ostream& os, const IllegalTransform& ex)
{
    os << "IllegalTransform: file: " << ex.file() << " line: " << ex.line();
    return os;
}

ostream&
operator<<(ostream& os, const IncompatibleSchema& ex)
{
    os << "IncompatibleSchema: file: " << ex.file() << " line: " << ex.line();
    return os;
}

ostream&
operator<<(ostream& os, const InvalidSchema& ex)
{
    os << "InvalidSchema: file: " << ex.file() << " line: " << ex.line();
    return os;
}

ostream&
operator<<(ostream& os, const SchemaViolation& ex)
{
    os << "SchemaViolation: file: " << ex.file() << " line: " << ex.line();
    return os;
}

static string
toString(const DOMString& s)
{
    char* t = s.transcode();
    string r(t);
    delete[] t;
    return r;
}


typedef ::std::map< ::std::string, DocumentInfo*> DocumentMap;

typedef ::std::map< ::std::string, ::std::string> PrefixURIMap;

struct DocumentInfo
{
    DOM_Document document;
    PrefixURIMap nsMap;
    ::std::string targetNamespace;
};

//
// Convert a QName from prefix:local to local@namespaceURI.
//
static string
convertQName(const string& qname, const DocumentInfo* info)
{
    size_t pos = qname.find(':');
    string prefix;
    string localName;
    if (pos != string::npos)
    {
	prefix = qname.substr(0, pos);
	localName = qname.substr(pos+1);
    }
    else
    {
	localName = qname;
    }

    PrefixURIMap::const_iterator q = info->nsMap.find(prefix);
    if (q == info->nsMap.end())
    {
	cout << qname << endl;
	//
	// No namespace - TODO: not InvalidSchema - it's an
	// invalid instance document.
	//
	throw InvalidSchema(__FILE__, __LINE__);
    }
	
    string n = localName;
    n += '@';
    n += q->second;

    return n;
}

static void
createNSMap(DocumentInfo* info, DOM_Node& root)
{
    static const string targetNamespaceAttrName = "targetNamespace";
    static const string xmlnsAttrName = "xmlns";

    DOM_NamedNodeMap attributes = root.getAttributes();
    unsigned int max = attributes.getLength();
    for (unsigned int i = 0; i < max; ++i)
    {
	DOM_Node attribute = attributes.item(i);
	string attrName = toString(attribute.getNodeName());
	if (attrName.substr(0, 5) == xmlnsAttrName)
	{
	    string ns;
	    if (attrName.size() > 5)
	    {
		ns = attrName.substr(6);
	    }
	    string uri = toString(attribute.getNodeValue());
	    //cout << "adding: " << ns << " " << uri << endl;
	    info->nsMap.insert(make_pair(ns, uri));
	}
	else if (attrName == targetNamespaceAttrName)
	{
	    info->targetNamespace = toString(attribute.getNodeValue());
	}
    }
}

static DOM_Node
findChild(const DOM_Node& parent, const string& namespaceURI, const string& localname)
{
    DOM_NodeList children = parent.getChildNodes();
    for (unsigned int i = 0; i < children.getLength(); ++i)
    {
	DOM_Node child = children.item(i);
	if (toString(child.getNamespaceURI()) == namespaceURI && toString(child.getLocalName()) == localname)
	{
	    return child;
	}
    }

    return DOM_Node();
}

//
// TODO: This is used for abuse. Replace by a real search.
//
static DOM_Node
findChild(const DOM_Node& parent, const string& name)
{
    DOM_NodeList children = parent.getChildNodes();
    for (unsigned int i = 0; i < children.getLength(); ++i)
    {
	DOM_Node child = children.item(i);
	if (toString(child.getNodeName()) == name)
	{
	    return child;
	}
    }
    return DOM_Node();
}

static string
getAttributeByName(DOM_Node& node, const string& name)
{
    DOM_NamedNodeMap attributes = node.getAttributes();
    unsigned int max = attributes.getLength();
    for (unsigned int i = 0; i < max; ++i)
    {
	DOM_Node attribute = attributes.item(i);
	string nodeName = toString(attribute.getNodeName());
	if (nodeName == name)
	{
	    return toString(attribute.getNodeValue());
	}
    }

    return string("");
}

static string
getLengthAttribute(DOM_Node& node)
{
    static const string lengthName = "length";
    return getAttributeByName(node, lengthName);
}

static string
getTypeAttribute(DOM_Node& node)
{
    static const string typeName = "type";
    return getAttributeByName(node, typeName);
}

static string
getNameAttribute(DOM_Node& node)
{
    static const string nameName = "name";
    return getAttributeByName(node, nameName);
}

static string
getValueAttribute(DOM_Node& node)
{
    static const string valueName = "value";
    return getAttributeByName(node, valueName);
}

static string
findAppinfoType(DOM_Node& node)
{
    DOM_Node child = findChild(node, "xs:annotation");
    if (!child.isNull())
    {
	child = findChild(child, "xs:appinfo");
	if (!child.isNull())
	{
	    child = findChild(child, "type");
	    if (!child.isNull())
	    {
		child = child.getFirstChild();
		if (!child.isNull())
		{
		    return toString(child.getNodeValue());
		}
	    }
	}
    }

    return string("");
}

//
// Specific transforms.
//
class NilTransform : public Transform
{
public:

    NilTransform()
    {
    }

    virtual void
    transform(ostream& os, const DocumentInfo*, const string&, DOM_Node node)
    {
	if (node.isNull())
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}

	os << node;
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[nil transform]";
	return os;
    }
};

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
    transform(ostream& os, const DocumentInfo*, const string&, DOM_Node node)
    {
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[internal transform]";
	return os;
    }
};

class ElementTransform : public Transform
{
public:
    
    ElementTransform(const string& namespaceURI, const string& name, Transform* transform) :
        _ns(namespaceURI),
	_name(name),
        _transform(transform)
    {
    }

    virtual void
    transform(ostream& os, const DocumentInfo* info, const string&, DOM_Node node)
    {
	_transform->transform(os, info, _name, node);
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[ element: " << name() << ": " << _transform->print(os) << "]";
	return os;
    }

    const string& namespaceURI() const { return _ns; }
    const string& name() const { return _name; }

private:

    string _ns;
    string _name;

    Transform* _transform;
};

class ValidateEnumerationTransform : public Transform
{
public:

    ValidateEnumerationTransform(const vector<string>& values) :
	_values(values)
    {
    }

    virtual void
    transform(ostream& os, const DocumentInfo*, const string&, DOM_Node node)
    {
	if (node.isNull())
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}

	DOM_Node child = node.getFirstChild();
	if (child.isNull() || child.getNodeType() != DOM_Node::TEXT_NODE)
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}
	string value = toString(child.getNodeValue());
	if (find(_values.begin(), _values.end(), value) == _values.end())
	{
	    throw IllegalTransform(__FILE__, __LINE__);
	}
	os << node;
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[validate enumeration]";
	return os;
    }

private:
  
    vector<string> _values;
};    

class ValidateIntegerTransform : public Transform
{
public:

    ValidateIntegerTransform(const string& from, const string& to) :
	_from(from),
	_to(to)
    {
    }

    virtual void
    transform(ostream& os, const DocumentInfo*, const string&, DOM_Node node)
    {
	if (node.isNull())
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}

	DOM_Node child = node.getFirstChild();
	if (child.isNull() || child.getNodeType() != DOM_Node::TEXT_NODE)
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}
	string value = toString(child.getNodeValue());
	long v = atol(value.c_str());
	if (_to == "xs:byte")
	{
	    if (v < SCHAR_MIN || v > SCHAR_MAX)
	    {
		throw IllegalTransform(__FILE__, __LINE__);
	    }
	}
	else if (_to == "xs:short")
	{
	    if (v < SHRT_MIN || v > SHRT_MAX)
	    {
		throw IllegalTransform(__FILE__, __LINE__);
	    }
	}
	else if (_to == "xs:int")
	{
	    if (v < INT_MIN || v > INT_MAX)
	    {
		throw IllegalTransform(__FILE__, __LINE__);
	    }
	}
	else if (_to == "xs:long")
	{
	}
	os << node;
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[validate integer: from=" << _from << " to=" << _to << " ]";
	return os;
    }

private:
    
    string _from;
    string _to;
};

class ValidateFloatTransform : public Transform
{
public:

    ValidateFloatTransform(const string& from, const string& to) :
	_from(from),
	_to(to)
    {
    }

    virtual void
    transform(ostream& os, const DocumentInfo*, const string&, DOM_Node node)
    {
	if (node.isNull())
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}

	DOM_Node child = node.getFirstChild();
	if (child.isNull() || child.getNodeType() != DOM_Node::TEXT_NODE)
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}
	os << node;
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[validate float: from=" << _from << " to=" << _to << " ]";
	return os;
    }

private:
    
    string _from;
    string _to;
};

class SequenceTransform : public Transform
{
public:

    SequenceTransform(Transform* transform) :
	_transform(transform)
    {
    }

    virtual void
    transform(ostream& os, const DocumentInfo* info, const string&, DOM_Node node)
    {
	if (node.isNull())
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}
	string name = toString(node.getNodeName());
	os << "<" << name;
	string length = getLengthAttribute(node);
	os << " length=\"" << length << "\"";
	// TODO: print attributes
	os << ">";

	long l = atol(length.c_str());

	DOM_NodeList children = node.getChildNodes();
	for (unsigned int i = 0; i < children.getLength(); ++i)
	{
	    DOM_Node child = children.item(i);
	    if (child.getNodeType() != DOM_Node::ELEMENT_NODE)
	    {
		continue;
	    }

	    static const string sequenceElementName = "e";
	    string nodeName = toString(child.getNodeName());
	    if (l == 0 || nodeName != sequenceElementName)
	    {
		throw SchemaViolation(__FILE__, __LINE__);
	    }
	    _transform->transform(os, info, nodeName, child);
	    --l;
	}

	os << "</" << name << ">";
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[sequence]\n";
	os << "\telement:" << _transform->print(os);
	os << "[sequence]\n";
	return os;
    }

private:

    Transform* _transform;
};

class StructTransform : public Transform
{
public:

    StructTransform()
    {
    }

    virtual void
    transform(ostream& os, const DocumentInfo* info, const string&, DOM_Node node)
    {
	if (node.isNull())
	{
	    throw SchemaViolation(__FILE__, __LINE__);
	}
	string name = toString(node.getNodeName());
	os << "<" << name;
	// TODO: print attributes
	os << ">";

	DOM_NodeList children = node.getChildNodes();
	for (vector<ElementTransform*>::const_iterator p = _transforms.begin(); p != _transforms.end(); ++p)
	{
	    DOM_Node child = findChild(node, (*p)->namespaceURI(), (*p)->name());
	    (*p)->transform(os, info, (*p)->name(), child);
	}

	os << "</" << name << ">";
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[struct start]\n";
	for (unsigned int i = 0; i < _transforms.size(); ++i)
	{
	    os << "\t";
	    _transforms[i]->print(os);
	    os << "\n";
	}
	os << "[struct end]\n";
	return os;
    }

    void
    append(ElementTransform* transform)
    {
	_transforms.push_back(transform);
    }

private:

    vector<ElementTransform*> _transforms;
};

class DefaultInitializedStructTransform : public Transform
{
public:

    DefaultInitializedStructTransform()
    {
    }

    virtual void
    transform(ostream& os, const DocumentInfo* info, const string& name, DOM_Node node)
    {
	os << "<" << name;
	// TODO: print attributes
	os << ">";

	DOM_Node child; // Nil

	for (vector<ElementTransform*>::const_iterator p = _transforms.begin(); p != _transforms.end(); ++p)
	{
	    (*p)->transform(os, info, (*p)->name(), child);
	}

	os << "</" << name << ">";
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[empty struct start]\n";
	for (unsigned int i = 0; i < _transforms.size(); ++i)
	{
	    os << "\t";
	    _transforms[i]->print(os);
	    os << "\n";
	}
	os << "[struct end]\n";
	return os;
    }

    void
    append(ElementTransform* transform)
    {
	_transforms.push_back(transform);
    }

private:

    vector<ElementTransform*> _transforms;
};

class ClassTransform : public Transform
{
public:

    ClassTransform(TransformMap& transforms) :
	_transforms(transforms)
    {
    }

    virtual void
    transform(ostream& os, const DocumentInfo* info, const string& name, DOM_Node node)
    {
	static const string xsitypeAttrName = "xsi:type";
	string type = getAttributeByName(node, xsitypeAttrName);
	if (type.empty())
	{
	    static const string xsinilAttrName = "xsi:nil";
	    string nil = getAttributeByName(node, xsinilAttrName);
	    if (nil.empty())
	    {
		throw SchemaViolation(__FILE__, __LINE__);
	    }
	    //
	    // Act as NilTransform
	    //
	    os << node;
	    return;
	}

	string n = convertQName(type, info);

	//
	// Technically this is only permitted to be a more derived
	// type - however, this will not be enforced here.
	//
	TransformMap::const_iterator p = _transforms.find(n);
	if (p == _transforms.end())
	{
	    cout << n << endl;
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

    TransformMap& _transforms;
    
};

class EmitStringTransform : public Transform
{
public:

    EmitStringTransform(const string& s) :
	_s(s)
    {
    }

    virtual void
    transform(ostream& os, const DocumentInfo*, const string& name, DOM_Node)
    {
	if (!_s.empty())
	{
	    os << "<" << name << ">" << _s << "</" << name << ">";
	}
	else
	{
	    os << "<" << name << "/>";
	}
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[emit string: \"" << _s << "\"]";
	return os;
    }

private:

    string _s;
};

class EmitAttributeTransform : public Transform
{
public:

    EmitAttributeTransform(const string& s) :
	_s(s)
    {
    }

    virtual void
    transform(ostream& os, const DocumentInfo*, const string& name, DOM_Node)
    {
	os << "<" << name << " " << _s << "/>";
    }

    virtual ostream&
    print(ostream& os)
    {
	os << "[emit attribute: \"" << _s << "\"]";
	return os;
    }

private:

    string _s;

};

//
// TODO: all the references are dangerous (DOM_Node&). Should either
// be const DOM_Node& or DOM_Node.
//
class TransformFactory
{
public:

    TransformFactory(DOM_Document&, DOM_Document&, TransformMap&, TransformMap&);
    ~TransformFactory();

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
	TypeDictionaryContent,
	TypeSequence,
	TypeProxy,
	TypeReference,
	TypeInternal
    };

private:

    DOM_Node findTypeInDocument(DOM_Document& doc, const ::std::string& local);
    void findType(const DocumentMap&, const DocumentInfo*, const ::std::string&, DOM_Node&, DocumentInfo*&);

    Transform* createTransform(const DocumentInfo*, DOM_Node&, Type, const ::std::string&,
			       const DocumentInfo*, DOM_Node&, Type, const ::std::string&);

    Transform* createTransformByTypeName(const DocumentInfo*, const ::std::string&,
					 const DocumentInfo*, const ::std::string&);

    Transform* createComplexTypeTransform(DOM_Node&, DOM_Node&);
    Transform* createSimpleTypeTransform(DOM_Node&, DOM_Node&);

    Transform* createStructTransform(const DocumentInfo*, DOM_Node&, const DocumentInfo*, DOM_Node&);
    Transform* createStaticClassTransform(DOM_Node&, DOM_Node&);
    Transform* createClassTransform(const DocumentInfo*, DOM_Node&, const DocumentInfo*, DOM_Node&);
    Transform* createSequenceTransform(const DocumentInfo*, DOM_Node&, const DocumentInfo*, DOM_Node&);
    Transform* createDictionaryTransform(const DocumentInfo*, DOM_Node&, const DocumentInfo*, DOM_Node&);
    void createEnumValues(DOM_Node&, ::std::vector< ::std::string>&);
    Transform* createEnumerationTransform(DOM_Node&);

    void createSequenceElementTransform(const DocumentInfo*, DOM_Node&, const DocumentInfo*, DOM_Node&,
					::std::vector<ElementTransform*>&);
    void createClassContentTransform(const DocumentInfo*, DOM_Node&,
				     const DocumentInfo*, DOM_Node&,
				     ::std::vector<ElementTransform*>&);

    Transform* createDefaultInitializedStructTransform(const DocumentInfo*, DOM_Node&);
    void createDefaultInitializedSequenceElementTransform(const DocumentInfo*,
							  DOM_Node&, ::std::vector<ElementTransform*>&);
    Transform* createDefaultInitializedTransform(const DocumentInfo*, const ::std::string&);

    Type getType(DOM_Node&);
    Type getTypeByName(const DocumentMap&, const DocumentInfo*, const ::std::string&, DOM_Node&, DocumentInfo*&);

    DOM_Node findSchemaRoot(const DOM_Document&);

    void import(DocumentMap&, const ::std::string&, const ::std::string&);
    void processImport(DOM_Document&, DocumentMap&);

    void processElements(const DocumentInfo* info);

    //
    // Set of documents. The set of all documents is the entire
    // document set.
    //
    // The map maps from targetNamespace to DocumentInfo.
    //
    DocumentMap _fromDocs;
    DocumentMap _toDocs;

    //
    // Map of local@uri element names to transforms. Needed for actual
    // transform.
    //
    TransformMap& _elements;
    
    //
    // Map of local@uri type names to transform. This information
    // cached for creation of the transform.
    //
    TransformMap _types;

    //
    // Map of local@uri class transforms (based on static type). This
    // information cached for creation of the transform.
    //
    TransformMap& _staticClassTransforms;

    //
    // Map of local@uri transforms for creating new types. This
    // information cached for creation of the transform.
    //
    TransformMap _defaultInitializedTransforms;
};

TransformFactory::TransformFactory(DOM_Document& fromDoc, DOM_Document& toDoc,
				   TransformMap& elements, 
				   TransformMap& staticClassTransforms) :
    _elements(elements),
    _staticClassTransforms(staticClassTransforms)
{
    DocumentInfo* fromInfo = new DocumentInfo();
    fromInfo->document = fromDoc;

    DocumentInfo* toInfo = new DocumentInfo();
    toInfo->document = toDoc;
    
    DOM_Node fromSchema = findSchemaRoot(fromDoc);
    assert(!fromSchema.isNull());

    DOM_Node toSchema = findSchemaRoot(toDoc);
    assert(!toSchema.isNull());

    //
    // Create the namespace maps for the two schemas.
    //
    createNSMap(fromInfo, fromSchema);
    createNSMap(toInfo, toSchema);

    //
    // Add the root document to the document map.
    //
    _fromDocs.insert(make_pair(fromInfo->targetNamespace, fromInfo));
    _toDocs.insert(make_pair(toInfo->targetNamespace, toInfo));

    processImport(fromDoc, _fromDocs);
    processImport(toDoc, _toDocs);

    for (DocumentMap::const_iterator p = _fromDocs.begin(); p != _fromDocs.end(); ++p)
    {
	processElements(p->second);
    }
}

TransformFactory::~TransformFactory()
{
}

struct StringTypeTable
{
    string name;
    TransformFactory::Type type;
    
    bool operator==(const string& rhs) const { return name == rhs; }
};

TransformFactory::Type
TransformFactory::getType(DOM_Node& node)
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
	{ "dictionaryContent", TypeDictionaryContent },
	{ "sequence", TypeSequence },
	{ "proxy", TypeProxy },
	{ "reference", TypeReference },
	{ "internal", TypeInternal }
    };
    static const StringTypeTable* begin = &items[0];
    static const StringTypeTable* end = &items[sizeof(items)/sizeof(items[0])];

    string type = findAppinfoType(node);

    const StringTypeTable* p = find(begin, end, type);
    if (p == end)
    {
	throw InvalidSchema(__FILE__, __LINE__);
    }

    return p->type;
}

TransformFactory::Type
TransformFactory::getTypeByName(const DocumentMap& docs, const DocumentInfo* info, const string& type,
			   DOM_Node& n, DocumentInfo*& nInfo)
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
    if (p != end)
    {
	return p->type;
    }

    findType(docs, info, type, n, nInfo);
    if (n.isNull())
    {
	throw InvalidSchema(__FILE__, __LINE__);
    }

    return getType(n);
}

Transform*
TransformFactory::createTransformByTypeName(const DocumentInfo* fromTypeInfo, const string& fromTypeName,
				       const DocumentInfo* toTypeInfo, const string& toTypeName)
{
    DOM_Node from;
    DocumentInfo* fromInfo;
    Type fromType = getTypeByName(_fromDocs, fromTypeInfo, fromTypeName, from, fromInfo);

    DOM_Node to;
    DocumentInfo* toInfo;
    Type toType = getTypeByName(_toDocs, toTypeInfo, toTypeName, to, toInfo);

    return createTransform(fromInfo, from, fromType, fromTypeName, toInfo, to, toType, toTypeName);
}

//
// This really needs to do more or less depending on the type of
// transform (element, or dealing with complexType/simpleType.
//
Transform*
TransformFactory::createTransform(const DocumentInfo* fromInfo, DOM_Node& from,
				  Type fromType, const string& fromTypeName,
				  const DocumentInfo* toInfo, DOM_Node& to,
				  Type toType, const string& toTypeName)
{
    
    TransformMap::const_iterator p = _types.find(fromTypeName);
    if (p != _types.end())
    {
	//cout << "returning previously cached transform: " << fromTypeName << endl;
	return p->second;
    }

    Transform* transform = 0;

    //
    // First handle transforms where the types are equivalent.
    //
    if (fromType == toType)
    {
	switch(fromType)
	{
	case TypeInteger:
	{
	    transform = new ValidateIntegerTransform(fromTypeName, toTypeName);
	    break;
	}

	case TypeFloat:
	{
	    transform = new ValidateFloatTransform(fromTypeName, toTypeName);
	    break;
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
	    // TODO: This doesn't allow the renaming of types. By
	    // removing this comparison renaming of types would be
	    // permitted. Should this be permitted?
	    //
	    if (fromTypeName != toTypeName)
	    {
		throw IllegalTransform(__FILE__, __LINE__);
	    }

	    transform = createEnumerationTransform(to);
	    break;
	}

	case TypeStruct:
	{
	    //
	    // If the type names are not the same then it's illegal.
	    //
	    // TODO: This doesn't allow the renaming of types. By
	    // removing this comparison renaming of types would be
	    // permitted. Should this be permitted?
	    //
	    if (fromTypeName != toTypeName)
	    {
		throw IllegalTransform(__FILE__, __LINE__);
	    }

	    transform = createStructTransform(fromInfo, from, toInfo, to);
	    break;
	}

	case TypeDictionaryContent:
	{
	    //
	    // If the type names are not the same then it's illegal.
	    //
	    // TODO: This doesn't allow the renaming of types. By
	    // removing this comparison renaming of types would be
	    // permitted. Should this be permitted?
	    //
	    if (fromTypeName != toTypeName)
	    {
		throw IllegalTransform(__FILE__, __LINE__);
	    }

	    transform = createStructTransform(fromInfo, from, toInfo, to);
	    break;
	}

	case TypeDictionary:
	{
	    //
	    // If the type names are not the same then it's illegal.
	    //
	    // TODO: This doesn't allow the renaming of types. By
	    // removing this comparison renaming of types would be
	    // permitted. Should this be permitted?
	    //
	    if (fromTypeName != toTypeName)
	    {
		throw IllegalTransform(__FILE__, __LINE__);
	    }

	    transform = createDictionaryTransform(fromInfo, from, toInfo, to);
	    break;
	}

	case TypeSequence:
	{
	    //
	    // If the type names are not the same then it's illegal.
	    //
	    // TODO: This doesn't allow the renaming of types. By
	    // removing this comparison renaming of types would be
	    // permitted. Should this be permitted?
	    //
	    if (fromTypeName != toTypeName)
	    {
		throw IllegalTransform(__FILE__, __LINE__);
	    }

	    transform = createSequenceTransform(fromInfo, from, toInfo, to);
	    break;
	}

	case TypeClass:
	{
	    //
	    // If the type names are not the same then it's illegal.
	    //
	    // TODO: This doesn't allow the renaming of types. By
	    // removing this comparison renaming of types would be
	    // permitted. Should this be permitted?
	    //
	    if (fromTypeName != toTypeName)
	    {
		throw IllegalTransform(__FILE__, __LINE__);
	    }

	    Transform* staticTransform = createClassTransform(fromInfo, from, toInfo, to);
	    return new ClassTransform(_staticClassTransforms);
	    break;
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
	    throw IllegalTransform(__FILE__, __LINE__);
	}
    }

    if (transform == 0)
    {
	//
	// Next we have transforms from type-to-type.
	//
	if (fromType == TypeString && toType == TypeEnumeration)
	{
	    //
	    // String to enumeration transform needs to validate the
	    // string as a member of the enumeration values.
	    //
	    transform = createEnumerationTransform(to);
	}
	else if (fromType == TypeEnumeration && toType == TypeString)
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
    
    if (transform == 0)
    {
	throw IllegalTransform(__FILE__, __LINE__);
    }

    _types.insert(make_pair(fromTypeName, transform));

    return transform;
}

Transform*
TransformFactory::createStructTransform(const DocumentInfo* fromInfo, DOM_Node& from,
					const DocumentInfo* toInfo, DOM_Node& to)
{
    static const string sequenceName = "xs:sequence";

    DOM_Node fromSeq = findChild(from, sequenceName);
    DOM_Node toSeq = findChild(to, sequenceName);
    if (fromSeq.isNull() || toSeq.isNull())
    {
	throw InvalidSchema(__FILE__, __LINE__);
    }

    StructTransform* nodeTransform = new StructTransform();

    vector<ElementTransform*> v;
    createSequenceElementTransform(fromInfo, fromSeq, toInfo, toSeq, v);
    for(vector<ElementTransform*>::const_iterator p = v.begin(); p != v.end(); ++p)
    {
	nodeTransform->append(*p);
    }

    return nodeTransform;
}

void
TransformFactory::createSequenceElementTransform(const DocumentInfo* fromInfo, DOM_Node& from,
						 const DocumentInfo* toInfo, DOM_Node& to,
						 vector<ElementTransform*>& v)
{
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
    for (unsigned int i = 0; i < toSeqChildren.getLength(); ++i)
    {
	ElementTransform* transform = 0;
	DOM_Node toChild = toSeqChildren.item(i);
	if (toChild.getNodeType() != DOM_Node::ELEMENT_NODE)
	{
	    continue;
	}

	static const string element = "xs:element";
	if (toString(toChild.getNodeName()) != element)
	{
	    throw InvalidSchema(__FILE__, __LINE__);
	}

	string toElementName = getNameAttribute(toChild);
	string toTypeName = getTypeAttribute(toChild);

	//
	// Search for the node in the fromSeqChildren list.
	//
	for (unsigned int j = 0; j < fromSeqChildren.getLength(); ++j)
	{
	    DOM_Node fromChild = fromSeqChildren.item(j);

	    if (fromChild.getNodeType() != DOM_Node::ELEMENT_NODE)
	    {
		// Skip non element nodes.
		continue;
	    }

	    if (toString(fromChild.getNodeName()) != element)
	    {
		throw InvalidSchema(__FILE__, __LINE__);
	    }

	    string fromElementName = getNameAttribute(fromChild);

	    if (fromElementName == toElementName)
	    {
		string fromTypeName = getTypeAttribute(fromChild);
		Transform* t = createTransformByTypeName(fromInfo, fromTypeName, toInfo, toTypeName);

		transform = new ElementTransform(toInfo->targetNamespace, toElementName, t);
		assert(transform);
	    }
	}

	//
	// If there is no transform then this is a new node
	// type. Create a transform to add an empty element of the
	// appropriate type.
	//
	if (!transform)
	{
	    Transform* t = createDefaultInitializedTransform(toInfo, toTypeName);
	    transform = new ElementTransform(toInfo->targetNamespace, toElementName, t);
	}

	v.push_back(transform);
    }
}

void
TransformFactory::createClassContentTransform(const DocumentInfo* fromInfo, DOM_Node& from,
					      const DocumentInfo* toInfo, DOM_Node& to,
					      vector<ElementTransform*>& v)
{
    static const string complexContentName = "xs:complexContent";
    static const string sequenceName = "xs:sequence";
    static const string extension = "xs:extension";
    static const string baseAttrName = "base";

    DOM_Node fromContent = findChild(from, complexContentName);
    DOM_Node toContent = findChild(to, complexContentName);

    if (fromContent.isNull() && toContent.isNull())
    {
	//
	// Must be base of a class heirarchy (while this
	// implementation is a little more flexible, with Ice it is
	// limited to ::Ice::Object).
	//
	
	//
	// Look for xs:sequence
	//
	DOM_Node fromSeq = findChild(from, sequenceName);
	DOM_Node toSeq = findChild(from, sequenceName);

	if (fromSeq.isNull() || toSeq.isNull())
	{
	    throw InvalidSchema(__FILE__, __LINE__);
	}

	//
	// TODO: Create cache of base class transforms
	//
	createSequenceElementTransform(fromInfo, fromSeq, toInfo, toSeq, v);
	return;
    }

    if (fromContent.isNull() || toContent.isNull())
    {
	throw InvalidSchema(__FILE__, __LINE__);
    }

    DOM_Node fromExtension = findChild(fromContent, extension);
    DOM_Node toExtension = findChild(toContent, extension);

    if (fromExtension.isNull() || toExtension.isNull())
    {
	throw InvalidSchema(__FILE__, __LINE__);
    }

    // TOOD: getAttributeBase?
    string fromBaseName = getAttributeByName(fromExtension, baseAttrName);
    string toBaseName = getAttributeByName(toExtension, baseAttrName);

    //
    // Cannot change base class.
    //
    if (fromBaseName != toBaseName)
    {
	throw IllegalTransform(__FILE__, __LINE__);
    }

    DOM_Node fromBaseNode;
    DocumentInfo* fromBaseInfo;
    findType(_fromDocs, fromInfo, fromBaseName, fromBaseNode, fromBaseInfo);
    DOM_Node toBaseNode;
    DocumentInfo* toBaseInfo;
    findType(_toDocs, toInfo, toBaseName, toBaseNode, toBaseInfo);

    if (fromBaseNode.isNull() || toBaseNode.isNull())
    {
	throw SchemaViolation(__FILE__, __LINE__);
    }
    
    //
    // Find the content transform for the base type.
    //
    createClassContentTransform(fromBaseInfo, fromBaseNode, toBaseInfo, toBaseNode, v);

    //
    // Look for xs:sequence
    //
    DOM_Node fromSeq = findChild(fromExtension, sequenceName);
    DOM_Node toSeq = findChild(toExtension, sequenceName);
    
    if (fromSeq.isNull() || toSeq.isNull())
    {
	throw InvalidSchema(__FILE__, __LINE__);
    }

    createSequenceElementTransform(fromInfo, fromSeq, toInfo, toSeq, v);
}

//
// Schema for class looks like:
//
// <xs:complexContent>
//    <xs:extension base=...>
//       <xs:sequence> ...
//
Transform*
TransformFactory::createClassTransform(const DocumentInfo* fromInfo, DOM_Node& from,
				       const DocumentInfo* toInfo, DOM_Node& to)
{
    string type = getNameAttribute(to);
    type += '@';
    type += toInfo->targetNamespace;

    TransformMap::const_iterator p = _staticClassTransforms.find(type);
    if (p != _staticClassTransforms.end())
    {
	cout << "returning cached static class transform: " << type << endl;
	return p->second;
    }

    vector<ElementTransform*> v;
    createClassContentTransform(fromInfo, from, toInfo, to, v);

    StructTransform* nodeTransform = new StructTransform();
    for(vector<ElementTransform*>::const_iterator p = v.begin(); p != v.end(); ++p)
    {
	nodeTransform->append(*p);
    }

    _staticClassTransforms.insert(make_pair(type, nodeTransform));

    return nodeTransform;
}

Transform*
TransformFactory::createSequenceTransform(const DocumentInfo* fromInfo, DOM_Node& from,
					  const DocumentInfo* toInfo, DOM_Node& to)
{
    static const string sequenceName = "xs:sequence";

    DOM_Node fromSeq = findChild(from, sequenceName);
    DOM_Node toSeq = findChild(to, sequenceName);
    if (fromSeq.isNull() || toSeq.isNull())
    {
	throw InvalidSchema(__FILE__, __LINE__);
    }

    //
    // Sequences have one element - which contains the type of the sequence.
    //
    static const string element = "xs:element";

    DOM_Node fromElement = findChild(fromSeq, element);
    DOM_Node toElement = findChild(toSeq, element);

    if (fromElement.isNull() || toElement.isNull())
    {
	throw InvalidSchema(__FILE__, __LINE__);
    }

    string fromTypeName = getTypeAttribute(fromElement);
    string toTypeName = getTypeAttribute(toElement);

    Transform* transform = createTransformByTypeName(fromInfo, fromTypeName, toInfo, toTypeName);
    return new SequenceTransform(transform);
}

Transform*
TransformFactory::createDictionaryTransform(const DocumentInfo* fromInfo, DOM_Node& from,
					    const DocumentInfo* toInfo, DOM_Node& to)
{
    static const string sequenceName = "xs:sequence";

    DOM_Node fromSeq = findChild(from, sequenceName);
    DOM_Node toSeq = findChild(to, sequenceName);
    if (fromSeq.isNull() || toSeq.isNull())
    {
	throw InvalidSchema(__FILE__, __LINE__);
    }

    //
    // Sequences have one element - which contains the type of the sequence.
    //
    static const string element = "xs:element";

    DOM_Node fromElement = findChild(fromSeq, element);
    DOM_Node toElement = findChild(toSeq, element);

    if (fromElement.isNull() || toElement.isNull())
    {
	throw InvalidSchema(__FILE__, __LINE__);
    }

    string fromTypeName = getTypeAttribute(fromElement);
    string toTypeName = getTypeAttribute(toElement);

    Transform* transform = createTransformByTypeName(fromInfo, fromTypeName, toInfo, toTypeName);
    return new SequenceTransform(transform);
}

void
TransformFactory::createEnumValues(DOM_Node& to, vector<string>& values)
{
    static const string restriction = "xs:restriction";

    DOM_Node toRes = findChild(to, restriction);
    if (toRes.isNull())
    {
	throw InvalidSchema(__FILE__, __LINE__);
    }

    //
    // Gather up a list of allowable values.
    //
    DOM_NodeList toResChildren = toRes.getChildNodes();

    for (unsigned int i = 0; i < toResChildren.getLength(); ++i)
    {
	DOM_Node toChild = toResChildren.item(i);

	if (toChild.getNodeType() != DOM_Node::ELEMENT_NODE)
	{
	    continue;
	}

	if (toString(toChild.getNodeName()) != "xs:enumeration")
	{
	    throw InvalidSchema(__FILE__, __LINE__);
	}

	string value = getValueAttribute(toChild);
	if (value.empty())
	{
	    throw InvalidSchema(__FILE__, __LINE__);
	}

	values.push_back(value);
    }
}

Transform*
TransformFactory::createEnumerationTransform(DOM_Node& to)
{
    vector<string> values;
    createEnumValues(to, values);

    return new ValidateEnumerationTransform(values);
}

void
TransformFactory::createDefaultInitializedSequenceElementTransform(const DocumentInfo* info, DOM_Node& node,
						 vector<ElementTransform*>& v)
{
    //
    // Allowable transforms:
    //
    // * Node added.
    // * Node removed.
    // * Node moved.
    //

    DOM_NodeList seqChild = node.getChildNodes();
    for (unsigned int i = 0; i < seqChild.getLength(); ++i)
    {
	DOM_Node child = seqChild.item(i);
	if (child.getNodeType() != DOM_Node::ELEMENT_NODE)
	{
	    continue;
	}

	static const string element = "xs:element";
	if (toString(child.getNodeName()) != element)
	{
	    throw InvalidSchema(__FILE__, __LINE__);
	}

	string elementName = getNameAttribute(child);
	string typeName = getTypeAttribute(child);

	Transform* t = createDefaultInitializedTransform(info, typeName);
	ElementTransform* transform = new ElementTransform(info->targetNamespace, elementName, t);

	v.push_back(transform);
    }
}

Transform*
TransformFactory::createDefaultInitializedStructTransform(const DocumentInfo* info, DOM_Node& node)
{
    static const string sequenceName = "xs:sequence";

    DOM_Node seq = findChild(node, sequenceName);
    if (seq.isNull())
    {
	throw InvalidSchema(__FILE__, __LINE__);
    }

    DefaultInitializedStructTransform* transform = new DefaultInitializedStructTransform();

    vector<ElementTransform*> v;
    createDefaultInitializedSequenceElementTransform(info, seq, v);
    for(vector<ElementTransform*>::const_iterator p = v.begin(); p != v.end(); ++p)
    {
	transform->append(*p);
    }

    return transform;
}

Transform*
TransformFactory::createDefaultInitializedTransform(const DocumentInfo* info, const string& typeName)
{
    string fullTypeName = convertQName(typeName, info);
    TransformMap::const_iterator p = _defaultInitializedTransforms.find(fullTypeName);
    if (p != _defaultInitializedTransforms.end())
    {
	//
	// Return cached empty transform.
	//
	return p->second;
    }

    DOM_Node n;
    DocumentInfo* nInfo;
    Type type = getTypeByName(_toDocs, info, typeName, n, nInfo);

    Transform* transform = 0;
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
	vector<string> values;
	createEnumValues(n, values);
	transform = new EmitStringTransform(values[0]);
	break;
    }
	
    case TypeStruct:
    {
	if (n.isNull())
	{
	    throw InvalidSchema(__FILE__, __LINE__);
	}
	transform = createDefaultInitializedStructTransform(info, n);
	break;
    }

    case TypeDictionary:
    case TypeSequence:
    {
	if (n.isNull())
	{
	    throw InvalidSchema(__FILE__, __LINE__);
	}
	transform = new EmitAttributeTransform("length=\"0\"");
	break;
    }

    case TypeInternal:
    case TypeException:
    case TypeDictionaryContent:
    case TypeClass:
    default:
	throw IllegalTransform(__FILE__, __LINE__);
    }

    if (transform == 0)
    {
	throw IllegalTransform(__FILE__, __LINE__);
    }

    _defaultInitializedTransforms.insert(make_pair(fullTypeName, transform));

    return transform;
}

void
print(DOM_Node& node, unsigned int level)
{
    if (node.isNull())
    {
	return;
    }
    unsigned int i;
    for (i = 0; i < level; ++i)
    {
	cout << "  ";
    }
    ++level;
    cout << toString(node.getNodeName()) << endl;

    DOM_NodeList children = node.getChildNodes();
    
    for (i = 0; i < children.getLength(); ++i)
    {
	for (unsigned int l = 0; l < level; ++l)
	{
	    cout << "  ";
	}
	DOM_Node child = children.item(i);
	print(child, level);
    }
}

DOM_Node
TransformFactory::findTypeInDocument(DOM_Document& doc, const string& local)
{
    DOM_Node schema = findSchemaRoot(doc);

    DOM_NodeList children = schema.getChildNodes();
    unsigned int i;
    for (i = 0; i < children.getLength(); ++i)
    {
	DOM_Node child = children.item(i);
	if (child.getNodeType() == DOM_Node::ELEMENT_NODE)
	{
	    string na = getNameAttribute(child);
	    if (na == local)
	    {
		return child;
	    }
	}
    }

    return DOM_Node();
}

//
// This find method takes the type QName and then maps the prefix
// portion to a namespace in the current document (info). Next the
// documents which have the given targetNamespace are searched for an
// element node with that name.
//
void
TransformFactory::findType(const DocumentMap& docs, const DocumentInfo* info, const string& type,
			   DOM_Node& n, DocumentInfo*& nInfo)
{
    string uri;
    string local;

    size_t pos = type.find(':');
    if (pos != string::npos)
    {
	string prefix = type.substr(0, pos);
	PrefixURIMap::const_iterator p = info->nsMap.find(prefix);
	if (p == info->nsMap.end())
	{
	    throw InvalidSchema(__FILE__, __LINE__);
	}
	uri = p->second;
	local = type.substr(pos+1);

	//cout << "looking for : " << local << " @ " << uri << endl;
    }
    else
    {
	uri = info->targetNamespace;
	local = type;

	//cout << "looking for : " << local << " @ " << uri << endl;
    }

    //
    // Find all documents with targetNamespace == uri
    //
    // TODO: multimap
    //
    DocumentMap::const_iterator p = docs.find(uri);
    if (p != docs.end())
    {
	n = findTypeInDocument(p->second->document, local);
	nInfo = p->second;
    }
}

DOM_Node
TransformFactory::findSchemaRoot(const DOM_Document& root)
{
    //
    // TODO: Can this be static const?
    //
    DOMString schemaURI("http://www.w3.org/2001/XMLSchema");
    DOMString schemaLocalName("schema");
    DOM_Node n;

    DOM_NodeList nodes = root.getElementsByTagNameNS(schemaURI, schemaLocalName);
    if (nodes.getLength() != 1)
    {
	throw InvalidSchema(__FILE__, __LINE__);
    }
    return nodes.item(0);
}

void
TransformFactory::import(DocumentMap& documents, const string& ns, const string& loc)
{
    DOMTreeErrorReporter errorReporter;
    DOMParser parser;
    parser.setValidationScheme(DOMParser::Val_Never);
    parser.setDoNamespaces(true);
    parser.setErrorHandler(&errorReporter);
    // Entity resolver?

    try
    {
	parser.parse(loc.c_str());
	if (errorReporter.getSawErrors())
	{
	    throw InvalidSchema(__FILE__, __LINE__);
	}
    }
    catch (const XMLException& toCatch)
    {
        cout << "Exception message is: \n"
             << DOMString(toCatch.getMessage()) << "\n" ;
	throw InvalidSchema(__FILE__, __LINE__);
    }
    catch (const SAXParseException& toCatch)
    {
        cout << "Exception message is: \n"
             << DOMString(toCatch.getMessage()) << "\n" ;
	throw InvalidSchema(__FILE__, __LINE__);
    }
    catch (...)
    {
        cout << "Unexpected Exception \n" ;
	throw InvalidSchema(__FILE__, __LINE__);
    }
    DOM_Document document = parser.getDocument();

    DOM_Node schema = findSchemaRoot(document);

    DocumentInfo* info = new DocumentInfo();
    info->document = document;

    createNSMap(info, schema);

    //
    // Rename the targetNamespace:
    //
    info->targetNamespace = ns;

    documents.insert(make_pair(info->targetNamespace, info));

    //
    // Process any imports in the imported document.
    //
    processImport(document, documents);
}

void
TransformFactory::processImport(DOM_Document& parent, DocumentMap& documents)
{
    DOM_Node schema = findSchemaRoot(parent);
    assert(!schema.isNull());
    
    DOM_Node child = schema.getFirstChild();
    while (!child.isNull())
    {
	static const string importName = "xs:import";
	string nodeName = toString(child.getNodeName());
	if (nodeName == importName)
	{
	    string ns = getAttributeByName(child, "namespace");
	    string loc = getAttributeByName(child, "schemaLocation");
	    
	    import(documents, ns, loc);
	}
	child = child.getNextSibling();
    }
}

void
TransformFactory::processElements(const DocumentInfo* info)
{
    DOM_Node schema = findSchemaRoot(info->document);

    DOM_NodeList children = schema.getChildNodes();
    unsigned int i;
    for (i = 0; i < children.getLength(); ++i)
    {
	DOM_Node child = children.item(i);
	if (child.getNodeType() != DOM_Node::ELEMENT_NODE)
	{
	    continue;
	}
	string nodeName = toString(child.getNodeName());
	    
	static const string element = "xs:element";
	
	if (nodeName != element)
	{
	    continue;
	}

	string nameAttr = getNameAttribute(child);
	    
	DOM_Node to;
	DocumentInfo* toNodeInfo; // Overrides the top-level toInfo.
	
	//
	// In this case info is correct since the element name must be
	// an unqualified name.
	//
	assert(nameAttr.find(':') == string::npos);

	findType(_toDocs, info, nameAttr, to, toNodeInfo);
	if (to.isNull())
	{
	    cout << "ignoring " << nameAttr << endl;
	    //
	    // No equivalent in the new schema.
	    //
	    continue;
	}

	string toNodeName = toString(to.getNodeName());
	string toNameAttr = getNameAttribute(to);
	
	//
	// Sanity check.
	//
	assert(toNameAttr == nameAttr);
	
	//
	// Construct the full element name - local@uri
 	//
	string fullElementName = nameAttr;
	fullElementName += '@';
	fullElementName += info->targetNamespace;
	
	if (_elements.find(fullElementName) != _elements.end())
	{
	    cout << "redefinition of element: " << fullElementName << endl;
	    throw InvalidSchema(__FILE__, __LINE__);
	}
	
	string fromTypeName = getTypeAttribute(child);
	string toTypeName = getTypeAttribute(to);

	//
	// Ignore anonymous elements (operation contents).
	//
	if (fromTypeName.empty() && toTypeName.empty())
	{
	    continue;
	}

	//
	// However, it's not legal for an element to change
	// type.
	//
	if (fromTypeName.empty() || toTypeName.empty())
	{
	    throw InvalidSchema(__FILE__, __LINE__);
	}

	Transform* transform = createTransformByTypeName(info, fromTypeName, toNodeInfo, toTypeName);
	//cout << "new element: " << fullElementName << endl;

	//
	// Add association between name & transform
	//
	_elements.insert(make_pair(fullElementName, transform));
    }
}


Transformer::Transformer(DOM_Document& fromDoc, DOM_Document& toDoc)
{
    TransformFactory factory(fromDoc, toDoc, _elements, _staticClassTransforms);
}

Transformer::~Transformer()
{
}

void
Transformer::transform(ostream& os, DOM_Document& doc)
{
    DOM_Node root = doc.getFirstChild();
    DocumentInfo info;
    info.document = doc;
    createNSMap(&info, root);

    DOM_NodeList children = root.getChildNodes();
    for (unsigned int i = 0; i < children.getLength(); ++i)
    {
	DOM_Node child = children.item(i);
	if (child.getNodeType() != DOM_Node::ELEMENT_NODE)
	{
	    continue;
	}

	string nodeName = toString(child.getNodeName());

	//
	// Create local@namespace version of the element name.
	//
	string n = convertQName(nodeName, &info);

	//cout << "looking for : " << n << endl;

	TransformMap::const_iterator p = _elements.find(n);
	if (p == _elements.end())
	{
	    cerr << "cannot find element: " << n << endl;
	    throw SchemaViolation(__FILE__, __LINE__);
	}

	p->second->transform(os, &info, nodeName, child);
    }
}

