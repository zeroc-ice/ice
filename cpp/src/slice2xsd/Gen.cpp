// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
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

static const string internalId = "_internal.";

Slice::Gen::Gen(const string& name, const string& base, const string& include,
		const vector<string>& includePaths, const string& dir) :
    _base(base),
    _include(include),
    _includePaths(includePaths)
{
    _orgName = "http://www.noorg.org"; // TODO: argument!
    for (vector<string>::iterator p = _includePaths.begin(); p != _includePaths.end(); ++p)
    {
	if (p->length() && (*p)[p->length() - 1] != '/')
	{
	    *p += '/';
	}
    }

    string::size_type pos = _base.rfind('/');
    if (pos != string::npos)
    {
	_base.erase(0, pos + 1);
    }

    string fileO = _base + ".xsd";
    if (!dir.empty())
    {
	fileO = dir + '/' + fileO;
    }

    O.open(fileO.c_str());
    if (!O)
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
    // I don't want the top-level module to be sorted, therefore no
    // unit->sort() before or after the unit->sortContents().
    //
    //unit->sortContents();

    //
    // TODO: It would be better if start() aligned the attributes
    // correctly.
    //
    string startString = "xs:schema";
    startString += " xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"";
    startString += "\n           elementFormDefault=\"qualified\"";
    startString += "\n           xmlns:ice=\"http://www.mutablerealms.com\"";
    startString += "\n           xmlns:tns=\"";
    startString += _orgName;
    startString += "\"";
    startString += "\n           targetNamespace=\"";
    startString += _orgName;
    startString += "\"";

    start(startString);
    // TODO: schemaLocation?
    O << nl << "<xs:import namespace=\"http://www.mutablerealms.com\" schemaLocation=\"ice.xsd\"/>";

    StringList includes = unit->includeFiles();
    for (StringList::const_iterator q = includes.begin(); q != includes.end(); ++q)
    {
	O << sp;
	O << nl << "<xs:include schemaLocation=\"" << changeInclude(*q, _includePaths) << ".xsd\"/>";
    }

    unit->visit(this);

    end();
    O << nl;
}

bool
Slice::Gen::visitClassDefStart(const ClassDefPtr& p)
{
    O << sp;

    string scopeId = containedToId(p);

    //
    // Emit class-name-data
    //
    string startString = "xs:group";
    startString += " name=\"";
    startString += internalId + scopeId + p->name();
    startString += "Data\"";
    start(startString);

    DataMemberList dataMembers = p->dataMembers();
    if (!dataMembers.empty())
    {
	start("xs:sequence");
	
	for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
	{
	    O << nl << "<xs:element name=\"" << (*q)->name() << "\" type=\"";
	    O << toString((*q)->type());
	    O << "\"/>";
	}

	end();
    }
    else
    {
	O << nl << "<xs:sequence/>";
    }
    
    end();

    O << sp;

    //
    // Emit class-name-type
    //
    startString = "xs:complexType";
    startString += " name=\"";
    startString += internalId + scopeId + p->name();
    startString += "Type\"";
    startString += " id=\"";
    startString += p->scoped();
    startString += "\"";
    start(startString);
    start("xs:complexContent");

    start("xs:annotation");
    start("xs:appinfo");
    O << nl << "<type>class</type>";
    end(); // xs:annotation
    end(); // xs:appinfo
    
    start("xs:extension base=\"ice:_internal.objectType\"");
    start("xs:sequence");

    //
    // Emit base Data
    //
    ClassList bases = p->bases();
    while (!bases.empty())
    {
	ClassDefPtr base = bases.front();
	if (base->isInterface())
	    break;
	string baseScopeId = containedToId(base);
	O << nl << "<xs:group ref=\"tns:" << internalId << baseScopeId << base->name() << "Data\"/>";
	bases = base->bases();
    }
    O << nl << "<xs:group ref=\"tns:" << internalId << scopeId << p->name() << "Data\"/>";

    end(); // xs:sequence
    end(); // xs:extension
    end(); // xs:complexContent
    end(); // xs:complexType

    O << sp << nl << "<xs:element name=\"" << scopeId << p->name()
      << "\" type=\"tns:" << internalId << scopeId << p->name() << "Type\"/>";

    return true;
}

bool
Slice::Gen::visitExceptionStart(const ExceptionPtr& p)
{
    O << sp;

    string scopeId = containedToId(p);

    //
    // Emit exception-name-data
    //
    string startString = "xs:group";
    startString += " name=\"";
    startString += internalId + scopeId + p->name();
    startString += "Data\"";
    start(startString);

    DataMemberList dataMembers = p->dataMembers();
    if (!dataMembers.empty())
    {
	start("xs:sequence");
	
	for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
	{
	    O << nl << "<xs:element name=\"" << (*q)->name() << "\" type=\"";
	    O << toString((*q)->type());
	    O << "\"/>";
	}

	end();
    }
    else
    {
	O << nl << "<xs:sequence/>";
    }
    
    end();

    O << sp;

    //
    // Emit exception-name-type
    //
    startString = "xs:complexType";
    startString += " name=\"";
    startString += internalId + scopeId + p->name();
    startString += "Type\"";
    startString += " id=\"";
    startString += p->scoped();
    startString += "\"";
    start(startString);

    start("xs:annotation");
    start("xs:appinfo");
    O << nl << "<type>exception</type>";
    end(); // xs:annotation
    end(); // xs:appinfo


    start("xs:sequence");

    //
    // Emit base Data
    //
    ExceptionPtr base = p->base();
    while (base)
    {
	string baseScopeId = containedToId(base);
	O << nl << "<xs:group ref=\"tns:" << internalId << baseScopeId << base->name() << "Data\"/>";
	base = base->base();
    }
    O << nl << "<xs:group ref=\"tns:" << internalId << scopeId << p->name() << "Data\"/>";

    end();
    end();

    O << sp << nl << "<xs:element name=\"" << scopeId << p->name()
      << "\" type=\"tns:" << internalId << scopeId << p->name() << "Type\"/>";

    return true;
}

bool
Slice::Gen::visitStructStart(const StructPtr& p)
{
    O << sp;

    string scopeId = containedToId(p);

    string startString = "xs:complexType";
    startString += " name=\"";
    startString += internalId + scopeId + p->name();
    startString += "Type\"";
    startString += " id=\"";
    startString += p->scoped();
    startString += "\"";
    start(startString);

    // TODO: refactor into method
    start("xs:annotation");
    start("xs:appinfo");
    O << nl << "<type>struct</type>";
    end(); // xs:annotation
    end(); // xs:appinfo

    DataMemberList dataMembers = p->dataMembers();
    if (!dataMembers.empty())
    {
	start("xs:sequence");
	
	for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
	{
	    O << nl << "<xs:element name=\"" << (*q)->name() << "\" type=\"";
	    O << toString((*q)->type());
	    O << "\"/>";
	}

	end(); // xs:sequence
    }
    else
    {
	O << nl << "<xs:sequence/>";
    }
    
    end(); // xs:complexType

    O << sp << nl << "<xs:element name=\"" << scopeId << p->name()
      << "\" type=\"tns:" << internalId << scopeId << p->name() << "Type\"/>";

    return true;
}

void
Slice::Gen::visitEnum(const EnumPtr& p)
{
    string scopeId = containedToId(p);

    O << sp;

    string startString = "xs:simpleType";
    startString += " name=\"";
    startString += internalId + scopeId + p->name();
    startString += "Type\"";
    startString += " id=\"";
    startString += p->scoped();
    startString += "\"";
    start(startString);

    start("xs:annotation");
    start("xs:appinfo");
    O << nl << "<type>enumeration</type>";
    end(); // xs:annotation
    end(); // xs:appinfo

    EnumeratorList enumerators = p->getEnumerators();
    assert (!enumerators.empty());

    start("xs:restriction base=\"xs:string\"");

    for (EnumeratorList::const_iterator q = enumerators.begin(); q != enumerators.end(); ++q)
    {
	O << nl << "<xs:enumeration value=\"" << (*q)->name() << "\"/>";
    }
    
    end(); // xs:restriction
    end(); // xs:simpleType

    O << sp;

    O << sp << nl << "<xs:element name=\"" << scopeId << p->name()
      << "\" type=\"tns:" << internalId << scopeId << p->name() << "Type\"/>";
}

void
Slice::Gen::visitSequence(const SequencePtr& p)
{
    O << sp;

    string scopeId = containedToId(p);

    string startString = "xs:complexType";
    startString += " name=\"";
    startString += internalId + scopeId + p->name();
    startString += "Type\"";
    startString += " id=\"";
    startString += p->scoped();
    startString += "\"";
    start(startString);
    start("xs:sequence");

    start("xs:annotation");
    start("xs:appinfo");
    O << nl << "<type>sequence</type>";
    end(); // xs:annotation
    end(); // xs:appinfo

    O << nl << "<xs:element name=\"e\" type=\"" << toString(p->type())
      << "\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>";

    end(); // xs:sequence
    end(); // xs:complexType

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
    string startString = "xs:complexType";
    startString += " name=\"";
    startString += internalId + scopeId + p->name();
    startString += "ContentType\"";
    start(startString);
    start("xs:sequence");

    O.inc();
    O << nl << "<xs:element name=\"key\" type=\"" << toString(p->keyType()) << "\"/>";
    O << nl << "<xs:element name=\"value\" type=\"" << toString(p->valueType()) << "\"/>";
    O.dec();

    end(); // xs:sequence
    end(); // xs:complexType

    O << sp;

    //
    // Next the dictionary sequence data.
    //
    startString = "xs:complexType";
    startString += " name=\"";
    startString += internalId + scopeId + p->name();
    startString += "Type\"";
    startString += " id=\"";
    startString += p->scoped();
    startString += "\"";
    start(startString);
    start("xs:sequence");

    start("xs:annotation");
    start("xs:appinfo");
    O << nl << "<type>dictionary</type>";
    end(); // xs:annotation
    end(); // xs:appinfo


    O << nl << "<xs:element name=\"e\" type=\"tns:" << internalId << scopeId << p->name() << "ContentType\""
      << " minOccurs=\"0\" maxOccurs=\"unbounded\"/>";

    end(); // xs:sequence
    end(); // xs:complexType

    O << sp;

    O << nl << "<xs:element name=\"" << scopeId << p->name() << "\" type=\"tns:"
      << internalId << scopeId << p->name() << "Type\"/>";
}

/* sequence, dictionary */

void
Slice::Gen::printHeader()
{
    static const char* header =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<!--\n"
"**********************************************************************\n"
"Copyright (c) 2001\n"
"MutableRealms, Inc.\n"
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
Slice::Gen::start(const std::string& element)
{
    O << nl << '<' << element << '>';
    O.inc();

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
Slice::Gen::end()
{
    string element = _elementStack.top();
    _elementStack.pop();

    O.dec();
    O << nl << "</" << element << '>';
}

string
Slice::Gen::containedToId(const ContainedPtr& contained)
{
    assert(contained);

    string scoped = contained->scope();
    if (scoped[0] == ':')
    {
	scoped.erase(0, 2);
    }

    string id;

    id.reserve(scoped.size());

    for (unsigned int i = 0; i < scoped.size(); ++i)
    {
	if (scoped[i] == ':')
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
	"xs;short",
	"xs:int",
	"xs:long",
	"xs:float",
	"xs:double",
	"xs:string",
	"ice:_internal.objectType", /* Object */
	"ice:_internal.proxyType", /* Object* */
	"???" /* LocalObject */
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    if (builtin)
    {
	s = builtinTable[builtin->kind()];
	//tag = "type";
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(p);
    if (proxy)
    {
	s = "ice:_internal.proxyType";
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(p);
    if (cl)
    {
	string scopeId = containedToId(cl);
	//s = "tns:" + internalId + scopeId + cl->name() + "Type";
	s = "ice:_internal.reference";
    }

    ExceptionPtr ex = ExceptionPtr::dynamicCast(p);
    if (ex)
    {
	string scopeId = containedToId(ex);
	s = "tns:" + internalId + scopeId + ex->name() + "Type";
    }

    StructPtr st = StructPtr::dynamicCast(p);
    if (st)
    {
	string scopeId = containedToId(st);
	s = "tns:" + internalId + scopeId + st->name() + "Type";
    }

    EnumeratorPtr en = EnumeratorPtr::dynamicCast(p);
    if (en)
    {
	string scopeId = containedToId(en);
	s = "tns:" + internalId + scopeId + en->name() + "Type";
    }

    SequencePtr sq = SequencePtr::dynamicCast(p);
    if (sq)
    {
	string scopeId = containedToId(sq);
	s = "tns:" + internalId + scopeId + sq->name() + "Type";
    }

    DictionaryPtr di = DictionaryPtr::dynamicCast(p);
    if (di)
    {
	string scopeId = containedToId(di);
	s = "tns:" + internalId + scopeId + di->name() + "Type";
    }

    EnumPtr em = EnumPtr::dynamicCast(p);
    if (em)
    {
	string scopeId = containedToId(em);
	s = "tns:" + internalId + scopeId + em->name() + "Type";
    }

    return s;
}
