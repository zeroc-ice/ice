// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/Functional.h>
#include <Slice/CPlusPlusUtil.h> // TODO: ???
#include <Gen.h>

using namespace std;
using namespace Slice;
using namespace IceUtil;

static const string internalId = "_internal.";

Slice::Gen::Gen(const string& name, const string& base, const string& include,
		const vector<string>& includePaths, const string& dir) :
    _base(base),
    _include(include),
    _includePaths(includePaths)
{
    _orgName = "http://www.noorg.org"; // TODO: argument!
    for(vector<string>::iterator p = _includePaths.begin(); p != _includePaths.end(); ++p)
    {
	if(p->length() && (*p)[p->length() - 1] != '/')
	{
	    *p += '/';
	}
    }

    string::size_type pos = _base.rfind('/');
    if(pos != string::npos)
    {
	_base.erase(0, pos + 1);
    }

    string fileO = _base + ".xsd";
    if(!dir.empty())
    {
	fileO = dir + '/' + fileO;
    }

    O.open(fileO.c_str());
    if(!O)
    {
	cerr << name << ": can't open `" << fileO << "' for writing: " << strerror(errno) << endl;
	return;
    }

    printHeader();
    O << "\n<!-- Generated from file `" << changeInclude(_base, _includePaths) << ".ice' -->\n";
}

Slice::Gen::~Gen()
{
}

bool
Slice::Gen::operator!() const
{
    return !O;
}

void
Slice::Gen::generate(const UnitPtr& unit)
{
    unit->mergeModules();

    //
    // TODO: It would be better if start() aligned the attributes
    // correctly.
    //
    ostringstream os;
    os << "xs:schema"
       << " xmlns:xs=\"http://www.w3.org/2001/XMLSchema\""
       << "\n           elementFormDefault=\"qualified\""
       << "\n           xmlns:ice=\"http://www.mutablerealms.com/schemas\""
       << "\n           xmlns:tns=\"" << _orgName << "/schemas\""
       << "\n           targetNamespace=\"" << _orgName << "/schemas\"";

    O << se(os.str());
    // TODO: schemaLocation?
    O << nl << "<xs:import namespace=\"http://www.mutablerealms.com/schemas\" schemaLocation=\"ice.xsd\"/>";

    StringList includes = unit->includeFiles();
    for(StringList::const_iterator q = includes.begin(); q != includes.end(); ++q)
    {
	O << sp;
	O << nl << "<xs:include schemaLocation=\"" << changeInclude(*q, _includePaths) << ".xsd\"/>";
    }

    unit->visit(this);

    O << ee;
    O << nl;
}

bool
Slice::Gen::visitClassDefStart(const ClassDefPtr& p)
{
    O << sp;

    string scopeId = containedToId(p);

    //
    // Emit class-name-type
    //
    ostringstream os;
    os << "xs:complexType name=\"" <<  internalId << scopeId << p->name() << "Type\" id=\"" << p->scoped() << "\"";
    O << se(os.str());

    annotate("class");

    O << se("xs:complexContent");
    
    string extension = "xs:extension base=\"";
    ClassList bases = p->bases();
    if(bases.empty() || bases.front()->isInterface())
    {
	extension += "ice:_internal.object";
    }
    else
    {
	extension += "tns:";
	ClassDefPtr base = bases.front();
	extension += internalId + containedToId(base) + base->name();
    }
    extension += "Type\"";
    
    O << se(extension);

    DataMemberList dataMembers = p->dataMembers();
    O << se("xs:sequence");
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
	emitElement((*q)->name(), (*q)->type());
    }
    O << ee; // xs:sequence

    O << ee; // xs:extension
    O << ee; // xs:complexContent
    O << ee; // xs:complexType

    O << sp << nl << "<xs:element name=\"" << scopeId << p->name()
      << "\" type=\"tns:" << internalId << scopeId << p->name() << "Type\" nillable=\"true\"/>";

    return true;
}

bool
Slice::Gen::visitExceptionStart(const ExceptionPtr& p)
{
    O << sp;

    string scopeId = containedToId(p);

    //
    // Emit exception-name-type
    //
    ostringstream os;
    os << "xs:complexType name=\"" <<  internalId << scopeId << p->name() << "Type\" id=\"" << p->scoped() << "\"";
    O << se(os.str());

    annotate("exception");

    //
    // Emit base Data
    //
    ExceptionPtr base = p->base();
    if(base)
    {
	string baseScopeId = containedToId(base);

	O << se("xs:complexContent");

	string extension = "xs:extension base=\"";
	extension += "tns:";
	extension += internalId + baseScopeId + base->name();
	extension += "Type\"";
	O << se(extension);
    }

    DataMemberList dataMembers = p->dataMembers();

    O << se("xs:sequence");
    
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
	emitElement((*q)->name(), (*q)->type());
    }
    
    O << ee; // xs:sequence

    if(base)
    {
	O << ee; // xs:extension
	O << ee; // xs:complexContent
    }

    O << ee; // xs:complexType

    O << sp << nl << "<xs:element name=\"" << scopeId << p->name()
      << "\" type=\"tns:" << internalId << scopeId << p->name() << "Type\"/>";

    return true;
}

bool
Slice::Gen::visitStructStart(const StructPtr& p)
{
    O << sp;

    string scopeId = containedToId(p);

    ostringstream os;
    os << "xs:complexType name=\"" <<  internalId << scopeId << p->name() << "Type\" id=\"" << p->scoped() << "\"";
    O << se(os.str());

    annotate("struct");

    DataMemberList dataMembers = p->dataMembers();
    O << se("xs:sequence");
    
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
	emitElement((*q)->name(), (*q)->type());
    }
    
    O << ee; // xs:sequence
    
    O << ee; // xs:complexType

    O << sp << nl << "<xs:element name=\"" << scopeId << p->name()
      << "\" type=\"tns:" << internalId << scopeId << p->name() << "Type\"/>";

    return true;
}

void
Slice::Gen::visitOperation(const OperationPtr& p)
{
    TypeStringList in = p->inputParameters();
    TypeStringList out = p->outputParameters();
    TypePtr ret = p->returnType();
    string scopeId = containedToId(p);
    ostringstream os;

    O << sp;

    os << "xs:element name=\"" << scopeId << "request." << p->name() << "\"";
    O << se(os.str());
    O << se("xs:complexType");

    annotate("operation");

    O << se("xs:sequence");
    TypeStringList::const_iterator q;
    for(q = in.begin(); q != in.end(); ++q)
    {
	emitElement(q->second, q->first);
    }
    O << ee; // xs:sequence

    O << ee; // xs:complexType
    O << ee; // xs:element

    os.str(""); // Reset stream

    O << sp;

    os << "xs:element name=\"" << scopeId << "reply." << p->name() << "\"";
    O << se(os.str());
    O << se("xs:complexType");

    annotate("operation");

    O << se("xs:sequence");
    if(ret)
    {
	emitElement("__return", ret);
	//O << nl << "<xs:element name=\"__return\" type=\"" << toString(ret) << "\"/>";
    }
    for(q = out.begin(); q != out.end(); ++q)
    {
	emitElement(q->second, q->first);
    }
    O << ee; // xs:sequence

    O << ee; // xs:complexType
    O << ee; // xs:element
}

void
Slice::Gen::visitEnum(const EnumPtr& p)
{
    string scopeId = containedToId(p);

    O << sp;

    ostringstream os;
    os << "xs:simpleType name=\"" <<  internalId << scopeId << p->name() << "Type\" id=\"" << p->scoped() << "\"";
    O << se(os.str());

    annotate("enumeration");

    EnumeratorList enumerators = p->getEnumerators();
    assert (!enumerators.empty());

    O << se("xs:restriction base=\"xs:string\"");

    for(EnumeratorList::const_iterator q = enumerators.begin(); q != enumerators.end(); ++q)
    {
	O << nl << "<xs:enumeration value=\"" << (*q)->name() << "\"/>";
    }
    
    O << ee; // xs:restriction
    O << ee; // xs:simpleType

    O << sp;

    O << sp << nl << "<xs:element name=\"" << scopeId << p->name()
      << "\" type=\"tns:" << internalId << scopeId << p->name() << "Type\"/>";
}

void
Slice::Gen::visitConstDef(const ConstDefPtr& p)
{
    // TODO: deal with constant definitions here
}

void
Slice::Gen::visitSequence(const SequencePtr& p)
{
    O << sp;

    string scopeId = containedToId(p);

    ostringstream os;
    os << "xs:complexType name=\"" <<  internalId << scopeId << p->name() << "Type\" id=\"" << p->scoped() << "\"";

    O << se(os.str());

    annotate("sequence");

    O << se("xs:sequence");

    os.str("");
    os << "xs:element name=\"e\" type=\"" << toString(p->type()) 
       << "\" minOccurs=\"0\" maxOccurs=\"unbounded\"";
    O << se(os.str());
    ProxyPtr proxy = ProxyPtr::dynamicCast(p->type());
    if(proxy)
    {
	annotate(proxy->_class()->scoped());
    }
    O << ee; // xs:element

    O << ee; // xs:sequence

    O << nl << "<xs:attribute name=\"length\" type=\"xs:long\"/>";

    O << ee; // xs:complexType

    O << sp;

    O << nl << "<xs:element name=\"" << scopeId << p->name() << "\" type=\"tns:"
      << internalId << scopeId << p->name() << "Type\"/>";
}

void
Slice::Gen::visitDictionary(const DictionaryPtr& p)
{
    O << sp;

    string scopeId = containedToId(p);

    //
    // First the dictionary content.
    //
    ostringstream os;
    os << "xs:complexType name=\"" <<  internalId << scopeId << p->name() << "ContentType\"";
    O << se(os.str());

    O << se("xs:sequence");

    O.inc();
    os.str("");
    os << "xs:element name=\"key\" type=\"" << toString(p->keyType()) << "\"";
    O << se(os.str());
    ProxyPtr proxy = ProxyPtr::dynamicCast(p->keyType());
    if(proxy)
    {
	annotate(proxy->_class()->scoped());
    }
    O << ee; // xs:element
    os.str("");
    os << "xs:element name=\"value\" type=\"" << toString(p->valueType()) << "\"";
    O << se(os.str());
    proxy = ProxyPtr::dynamicCast(p->valueType());
    if(proxy)
    {
	annotate(proxy->_class()->scoped());
    }
    O << ee; // xs:element

    //O << nl << "<xs:element name=\"key\" type=\"" << toString(p->keyType()) << "\"/>";
    //O << nl << "<xs:element name=\"value\" type=\"" << toString(p->valueType()) << "\"/>";
    O.dec();

    O << ee; // xs:sequence
    O << ee; // xs:complexType

    O << sp;

    //
    // Next the dictionary sequence data.
    //
    os.str("");
    os << "xs:complexType name=\"" <<  internalId << scopeId << p->name() << "Type\" id=\"" << p->scoped() << "\"";
    O << se(os.str());

    annotate("dictionary");

    O << se("xs:sequence");

    O << nl << "<xs:element name=\"e\" type=\"tns:" << internalId << scopeId << p->name() << "ContentType\""
      << " minOccurs=\"0\" maxOccurs=\"unbounded\"/>";

    O << ee; // xs:sequence

    O << nl << "<xs:attribute name=\"length\" type=\"xs:long\"/>";

    O << ee; // xs:complexType

    O << sp;

    O << nl << "<xs:element name=\"" << scopeId << p->name() << "\" type=\"tns:"
      << internalId << scopeId << p->name() << "Type\"/>";
}

void
Slice::Gen::printHeader()
{
    static const char* header =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<!--\n"
"**********************************************************************\n"
"Copyright (c) 2001\n"
"Mutable Realms, Inc.\n"
"Huntsville, AL, USA\n"
"\n"
"All Rights Reserved\n"
"\n"
"Generated by the `slice2xsd' converter\n"
"**********************************************************************\n"
"-->";
    
    O.zeroIndent();
    O << header;
    O << "\n<!-- Ice version " << ICE_STRING_VERSION << " -->";
    O.restoreIndent();
}

void
Slice::Gen::annotate(const ::std::string& type)
{
    O << se("xs:annotation");
    O << se("xs:appinfo");
    O << nl << "<type>" << type << "</type>";
    O << ee; // xs:annotation
    O << ee; // xs:appinfo
}

void
Slice::Gen::emitElement(const string& name, const TypePtr& type)
{
    ostringstream os;
    os << "xs:element name=\"" << name << "\" type=\"" << toString(type) << '"';
    O << se(os.str());
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
	annotate(proxy->_class()->scoped());
    }
    O << ee; // xs:element
}

string
Slice::Gen::containedToId(const ContainedPtr& contained)
{
    assert(contained);

    string scoped = contained->scope();
    if(scoped[0] == ':')
    {
	scoped.erase(0, 2);
    }

    string id;

    id.reserve(scoped.size());

    for(unsigned int i = 0; i < scoped.size(); ++i)
    {
	if(scoped[i] == ':')
	{
	    id += '.';
	    ++i;
	}
	else
	{
	    id += scoped[i];
	}
    }

    return id;
}

string
Slice::Gen::toString(const SyntaxTreeBasePtr& p)
{
    string tag;
    string linkend;
    string s;

    static const char* builtinTable[] =
    {
	"xs:byte",
	"xs:boolean",
	"xs:short",
	"xs:int",
	"xs:long",
	"xs:float",
	"xs:double",
	"xs:string",
	"ice:_internal.reference", /* Object */
	"ice:_internal.proxyType", /* Object* */
	"???" /* LocalObject */
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    if(builtin)
    {
	s = builtinTable[builtin->kind()];
	//tag = "type";
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(p);
    if(proxy)
    {
	s = "ice:_internal.proxyType";
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(p);
    if(cl)
    {
	string scopeId = containedToId(cl);
	//s = "tns:" + internalId + scopeId + cl->name() + "Type";
	s = "ice:_internal.reference";
    }

    ExceptionPtr ex = ExceptionPtr::dynamicCast(p);
    if(ex)
    {
	string scopeId = containedToId(ex);
	s = "tns:" + internalId + scopeId + ex->name() + "Type";
    }

    StructPtr st = StructPtr::dynamicCast(p);
    if(st)
    {
	string scopeId = containedToId(st);
	s = "tns:" + internalId + scopeId + st->name() + "Type";
    }

    EnumeratorPtr en = EnumeratorPtr::dynamicCast(p);
    if(en)
    {
	string scopeId = containedToId(en);
	s = "tns:" + internalId + scopeId + en->name() + "Type";
    }

    SequencePtr sq = SequencePtr::dynamicCast(p);
    if(sq)
    {
	string scopeId = containedToId(sq);
	s = "tns:" + internalId + scopeId + sq->name() + "Type";
    }

    DictionaryPtr di = DictionaryPtr::dynamicCast(p);
    if(di)
    {
	string scopeId = containedToId(di);
	s = "tns:" + internalId + scopeId + di->name() + "Type";
    }

    EnumPtr em = EnumPtr::dynamicCast(p);
    if(em)
    {
	string scopeId = containedToId(em);
	s = "tns:" + internalId + scopeId + em->name() + "Type";
    }

    return s;
}
