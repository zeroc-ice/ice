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
#include <Gen.h>

using namespace std;
using namespace Slice;

Slice::Gen::Gen(const string& name, const string& file, bool standAlone, bool noGlobals) :
    _standAlone(standAlone),
    _noGlobals(noGlobals),
    _chapter("section"), // Could also be "chapter"
    _nextId(0)
{
    O.open(file.c_str());
    if (!O)
    {
	cerr << name << ": can't open `" << file << "' for writing: " << strerror(errno) << endl;
	return;
    }

    _idPrefix = file;
    string::size_type pos = _idPrefix.find_last_of("/\\");
    if(pos != string::npos)
    {
	_idPrefix.erase(0, pos + 1);
    }
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
    unit->sort();
    unit->visit(this);
}

void
Slice::Gen::visitUnitStart(const UnitPtr& p)
{
    if (_standAlone)
    {
	O << "<!DOCTYPE article PUBLIC \"-//OASIS//DTD DocBook V3.1//EN\">";
	printHeader();
	start("article");
    }
    else
    {
	printHeader();
    }

    if (!_noGlobals)
    {
	start(_chapter, "Global Module");
	start("section", "Overview");
	visitContainer(p);
    }
}

void
Slice::Gen::visitUnitEnd(const UnitPtr& p)
{
    if (_standAlone)
    {
	end();
    }
}

void
Slice::Gen::visitModuleStart(const ModulePtr& p)
{
    start(_chapter + " id=" + scopedToId(p->scoped()), p->scoped().substr(2));
    start("section", "Overview");
    O.zeroIndent();
    O << nl << "<synopsis>module <classname>" << p->name() << "</classname></synopsis>";
    O.restoreIndent();
    printComment(p);
    visitContainer(p);
}

void
Slice::Gen::visitContainer(const ContainerPtr& p)
{
    ModuleList modules = p->modules();
    modules.sort();
    if (!modules.empty())
    {
	start("section", "Module Index");
	start("variablelist");
	
	for (ModuleList::iterator q = modules.begin(); q != modules.end(); ++q)
	{
	    start("varlistentry");
	    start("term");
	    O << nl << toString(*q, p);
	    end();
	    start("listitem");
	    printSummary(*q);
	    end();
	    end();
	}

	end();
	end();
    }

    ClassList classes = p->classes();
    ClassList interfaces;
    interfaces.splice(interfaces.end(),
		      classes,
		      remove_if(classes.begin(), classes.end(), ::IceUtil::memFun(&ClassDef::isInterface)),
		      classes.end());

    classes.sort();
    if (!classes.empty())
    {
	start("section", "Class Index");
	start("variablelist");
	
	for (ClassList::iterator q = classes.begin(); q != classes.end(); ++q)
	{
	    start("varlistentry");
	    start("term");
	    O << nl << toString(*q, p);
	    end();
	    start("listitem");
	    printSummary(*q);
	    end();
	    end();
	}

	end();
	end();
    }

    interfaces.sort();
    if (!interfaces.empty())
    {
	start("section", "Interface Index");
	start("variablelist");
	
	for (ClassList::iterator q = interfaces.begin(); q != interfaces.end(); ++q)
	{
	    start("varlistentry");
	    start("term");
	    O << nl << toString(*q, p);
	    end();
	    start("listitem");
	    printSummary(*q);
	    end();
	    end();
	}

	end();
	end();
    }

    StructList structs = p->structs();
    structs.sort();
    if (!structs.empty())
    {
	start("section", "Struct Index");
	start("variablelist");
	
	for (StructList::iterator q = structs.begin(); q != structs.end(); ++q)
	{
	    start("varlistentry");
	    start("term");
	    O << nl << toString(*q, p);
	    end();
	    start("listitem");
	    printSummary(*q);
	    end();
	    end();
	}

	end();
	end();
    }

    SequenceList sequences = p->sequences();
    sequences.sort();
    if (!sequences.empty())
    {
	start("section", "Sequence Index");
	start("variablelist");
	
	for (SequenceList::iterator q = sequences.begin(); q != sequences.end(); ++q)
	{
	    start("varlistentry");
	    start("term");
	    O << nl << toString(*q, p);
	    end();
	    start("listitem");
	    printSummary(*q);
	    end();
	    end();
	}

	end();
	end();
    }

    DictionaryList dictionaries = p->dictionaries();
    dictionaries.sort();
    if (!dictionaries.empty())
    {
	start("section", "Dictionary Index");
	start("variablelist");
	
	for (DictionaryList::iterator q = dictionaries.begin(); q != dictionaries.end(); ++q)
	{
	    start("varlistentry");
	    start("term");
	    O << nl << toString(*q, p);
	    end();
	    start("listitem");
	    printSummary(*q);
	    end();
	    end();
	}

	end();
	end();
    }

    EnumList enums = p->enums();
    enums.sort();
    if (!enums.empty())
    {
	start("section", "Enum Index");
	start("variablelist");
	
	for (EnumList::iterator q = enums.begin(); q != enums.end(); ++q)
	{
	    start("varlistentry");
	    start("term");
	    O << nl << toString(*q, p);
	    end();
	    start("listitem");
	    printSummary(*q);
	    end();
	    end();
	}

	end();
	end();
    }

    NativeList natives = p->natives();
    natives.sort();
    if (!natives.empty())
    {
	start("section", "Native Index");
	start("variablelist");
	
	for (NativeList::iterator q = natives.begin(); q != natives.end(); ++q)
	{
	    start("varlistentry");
	    start("term");
	    O << nl << toString(*q, p);
	    end();
	    start("listitem");
	    printSummary(*q);
	    end();
	    end();
	}

	end();
	end();
    }

    end();

    {
	for (SequenceList::iterator q = sequences.begin(); q != sequences.end(); ++q)
	{
	    TypePtr type = (*q)->type();
	    
	    start("section id=" + scopedToId((*q)->scoped()), (*q)->name());
	    
	    O.zeroIndent();
	    O << nl << "<synopsis>sequence&lt; " << toString(type, p) << " &gt; <type>" << (*q)->name()
	      << "</type>;</synopsis>";
	    O.restoreIndent();
	    
	    printComment(*q);
	    
	    end();
	}
    }
    
    {
	for (DictionaryList::iterator q = dictionaries.begin(); q != dictionaries.end(); ++q)
	{
	    TypePtr keyType = (*q)->keyType();
	    TypePtr valueType = (*q)->valueType();
	    
	    start("section id=" + scopedToId((*q)->scoped()), (*q)->name());
	    
	    O.zeroIndent();
	    O << nl << "<synopsis>dictionary&lt; " << toString(keyType, p) << ", " << toString(valueType, p)
	      << " &gt; <type>" << (*q)->name() << "</type>;</synopsis>";
	    O.restoreIndent();
	    
	    printComment(*q);
	    
	    end();
	}
    }
    
    {
	for (EnumList::iterator q = enums.begin(); q != enums.end(); ++q)
	{
	    start("section id=" + scopedToId((*q)->scoped()), (*q)->name());
	    
	    O.zeroIndent();
	    O << nl << "<synopsis>enum <type>" << (*q)->name() << "</type>";
	    O << sb;
	    StringList enumerators = (*q)->enumerators();
	    StringList::iterator r = enumerators.begin();
	    while (r != enumerators.end())
	    {
		O << nl << "<structfield>" << *r << "</structfield>";
		if (++r != enumerators.end())
		{
		    O << ',';
		}
	    }
	    O << eb << ";</synopsis>";
	    O.restoreIndent();
	    
	    printComment(*q);
	    
	    end();
	}
    }

    {
	for (NativeList::iterator q = natives.begin(); q != natives.end(); ++q)
	{
	    start("section id=" + scopedToId((*q)->scoped()), (*q)->name());
	    
	    O.zeroIndent();
	    O << nl << "<synopsis>native <type>" << (*q)->name() << "</type>;</synopsis>";
	    O.restoreIndent();
	    
	    printComment(*q);
	    
	    end();
	}
    }

    end();
}

void
Slice::Gen::visitClassDefStart(const ClassDefPtr& p)
{
    start(_chapter + " id=" + scopedToId(p->scoped()), p->scoped().substr(2));

    start("section", "Overview");
    O.zeroIndent();
    O << nl << "<synopsis>";
    if (p->isLocal())
    {
	O << "local ";
    }
    if (p->isInterface())
    {
	O << "interface";
    }
    else
    {
	O << "class";
    }
    O << " <classname>" << p->name() << "</classname>";
    ClassList bases = p->bases();
    if (!bases.empty() && !bases.front()->isInterface())
    {
	O.inc();
	O << nl << "extends ";
	O.inc();
	O << nl << toString(bases.front(), p);
	bases.pop_front();
	O.dec();
	O.dec();
    }
    if (!bases.empty())
    {
	O.inc();
	if (p->isInterface())
	{
	    O << nl << "extends ";
	}
	else
	{
	    O << nl << "implements ";
	}
	O.inc();
	ClassList::iterator q = bases.begin();
	while (q != bases.end())
	{
	    O << nl << toString(*q, p);
	    if (++q != bases.end())
	    {
		O << ",";
	    }
	}
	O.dec();
	O.dec();
    }
    O << "</synopsis>";
    O.restoreIndent();

    printComment(p);

    OperationList operations = p->operations();
    operations.sort();
    if (!operations.empty())
    {
	start("section", "Operation Index");
	start("variablelist");
	
	for (OperationList::iterator q = operations.begin(); q != operations.end(); ++q)
	{
	    start("varlistentry");
	    start("term");
	    O << nl << toString(*q, p);
	    end();
	    start("listitem");
	    printSummary(*q);
	    end();
	    end();
	}

	end();
	end();
    }

    DataMemberList dataMembers = p->dataMembers();
    dataMembers.sort();
    if (!dataMembers.empty())
    {
	start("section", "Data Member Index");
	start("variablelist");
	
	for (DataMemberList::iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
	{
	    start("varlistentry");
	    start("term");
	    O << nl << toString(*q, p);
	    end();
	    start("listitem");
	    printSummary(*q);
	    end();
	    end();
	}

	end();
	end();
    }

    end();

    {
	for (OperationList::iterator q = operations.begin(); q != operations.end(); ++q)
	{
	    bool nonmutating = (*q)->nonmutating();
	    TypePtr returnType = (*q)->returnType();
	    TypeStringList inputParams = (*q)->inputParameters();
	    TypeStringList outputParams = (*q)->outputParameters();
	    TypeList throws =  (*q)->throws();
	    
	    start("section id=" + scopedToId((*q)->scoped()), (*q)->name());
	    
	    O.zeroIndent();
	    O << nl << "<synopsis>" << (nonmutating ? "nonmutating " : "")
	      << (returnType ? toString(returnType, p) : "<type>void</type>")
	      << " <function>" << (*q)->name() << "</function>(";
	    O.inc();
	    TypeStringList::iterator r = inputParams.begin();
	    while (r != inputParams.end())
	    {
		O << nl << toString(r->first, p) << " <parameter>" << r->second << "</parameter>";
		if (++r != inputParams.end())
		{
		    O << ',';
		}
	    }
	    if (!outputParams.empty())
	    {
		O << ';';
		r = outputParams.begin();
		while (r != outputParams.end())
		{
		    O << nl << toString(r->first, p) << " <parameter>" << r->second << "</parameter>";
		    if (++r != outputParams.end())
		    {
			O << ',';
		    }
		}
	    }
	    O << ')';
	    O.dec();
	    if (!throws.empty())
	    {
		O.inc();
		O << nl << "throws";
		O.inc();
		TypeList::iterator r = throws.begin();
		while (r != throws.end())
		{
		    O << nl << toString(*r, p);
		    if (++r != throws.end())
		    {
			O << ',';
		    }
		}
		O.dec();
		O.dec();
	    }
	    O << ";</synopsis>";
	    O.restoreIndent();
	    
	    printComment(*q);
	    
	    end();
	}
    }

    {
	for (DataMemberList::iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
	{
	    TypePtr type = (*q)->type();
	    
	    start("section id=" + scopedToId((*q)->scoped()), (*q)->name());
	    
	    O.zeroIndent();
	    O << nl << "<synopsis>" << toString(type, p) << " <structfield>" << (*q)->name()
	      << "</structfield>;</synopsis>";
	    O.restoreIndent();
	    
	    printComment(*q);
	    
	    end();
	}
    }
	
    end();
}

void
Slice::Gen::visitStructStart(const StructPtr& p)
{
    start(_chapter + " id=" + scopedToId(p->scoped()), p->scoped().substr(2));

    start("section", "Overview");
    O.zeroIndent();
    O << nl << "<synopsis>";
    O << "struct <structname>" << p->name() << "</structname>";
    O << "</synopsis>";
    O.restoreIndent();

    printComment(p);

    DataMemberList dataMembers = p->dataMembers();
    dataMembers.sort();
    if (!dataMembers.empty())
    {
	start("section", "Data Member Index");
	start("variablelist");
	
	for (DataMemberList::iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
	{
	    start("varlistentry");
	    start("term");
	    O << nl << toString(*q, p);
	    end();
	    start("listitem");
	    printSummary(*q);
	    end();
	    end();
	}

	end();
	end();
    }

    end();

    {
	for (DataMemberList::iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
	{
	    TypePtr type = (*q)->type();
	    
	    start("section id=" + scopedToId((*q)->scoped()), (*q)->name());
	    
	    O.zeroIndent();
	    O << nl << "<synopsis>" << toString(type, p) << " <structfield>" << (*q)->name()
	      << "</structfield>;</synopsis>";
	    O.restoreIndent();
	    
	    printComment(*q);
	    
	    end();
	}
    }
	
    end();
}

void
Slice::Gen::printHeader()
{
    static const char* header =
"<!--\n"
"**********************************************************************\n"
"Copyright (c) 2001\n"
"MutableRealms, Inc.\n"
"Huntsville, AL, USA\n"
"\n"
"All Rights Reserved\n"
"\n"
"Generated by the `slice2docbook' converter\n"
"**********************************************************************\n"
"-->";
    
    O.zeroIndent();
    O << header;
    O << "\n<!-- Ice version " << ICE_STRING_VERSION << " -->";
    O.restoreIndent();
}

StringList
Slice::Gen::getTagged(const string& tag, string& comment)
{
    StringList result;
    string::size_type begin = 0;
    while (begin < comment.size())
    {
	begin = comment.find("@" + tag, begin);
	if (begin == string::npos)
	{
	    return result;
	}
	
	string::size_type pos1 = comment.find_first_not_of(" \t\r\n", begin + tag.size() + 1);
	if (pos1 == string::npos)
	{
	    comment.erase(begin);
	    return result;
	}
	
	string::size_type pos2 = comment.find('@', pos1);
	string line = comment.substr(pos1, pos2 - pos1);
	comment.erase(begin, pos2 - 1 - begin);

	string::size_type pos3 = line.find_last_not_of(" \t\r\n");
	if (pos3 != string::npos)
	{
	    line.erase(pos3 + 1);
	}
	result.push_back(line);
    }

    return result;
}

void
Slice::Gen::printComment(const ContainedPtr& p)
{
    ContainerPtr container = ContainerPtr::dynamicCast(p);
    if (!container)
    {
	container = p->container();
    }

    string comment = p->comment();
    StringList par = getTagged("param", comment);
    StringList ret = getTagged("return", comment);
    StringList throws = getTagged("throws", comment);
    StringList see = getTagged("see", comment);

    start("para");
    string::size_type pos = comment.find_last_not_of(" \t\r\n");
    if (pos != string::npos)
    {
	comment.erase(pos + 1);
	O.zeroIndent();
	O << nl << comment;
	O.restoreIndent();
    }
    end();

    if (!par.empty())
    {
	start("section", "Parameters");
	start("variablelist");
	for (StringList::iterator q = par.begin(); q != par.end(); ++q)
	{
	    string::size_type pos;
	    string term;
	    pos = q->find_first_of(" \t\r\n");
	    if (pos != string::npos)
	    {
		term = q->substr(0, pos);
	    }
	    string item;
	    pos = q->find_first_not_of(" \t\r\n", pos);
	    if (pos != string::npos)
	    {
		item = q->substr(pos);
	    }
	    
	    start("varlistentry");
	    start("term");
	    start("parameter");
	    O << nl << term;
	    end();
	    end();
	    start("listitem");
	    start("para");
	    O << nl << item;
	    end();
	    end();
	    end();
	}

	end();
	end();
    }

    if (!ret.empty())
    {
	start("section", "Return Value");
	start("para");
	O << nl << ret.front();
	end();
	end();
    }

    if (!throws.empty())
    {
	start("section", "Exceptions");
	start("variablelist");
	
	for (StringList::iterator q = throws.begin(); q != throws.end(); ++q)
	{
	    string::size_type pos;
	    string term;
	    pos = q->find_first_of(" \t\r\n");
	    if (pos != string::npos)
	    {
		term = q->substr(0, pos);
	    }
	    string item;
	    pos = q->find_first_not_of(" \t\r\n", pos);
	    if (pos != string::npos)
	    {
		item = q->substr(pos);
	    }
	    
	    start("varlistentry");
	    start("term");
	    O << nl << toString(term, container);
	    end();
	    start("listitem");
	    start("para");
	    O << nl << item;
	    end();
	    end();
	    end();
	}

	end();
	end();
    }

    if (!see.empty())
    {
	start("section", "See Also");
	start("para");
	start("simplelist type=\"inline\"");
	
	for (StringList::iterator q = see.begin(); q != see.end(); ++q)
	{
	    start("member");
	    O << nl << toString(*q, container);
	    end();
	}

	end();
	end();
	end();
    }
}

void
Slice::Gen::printSummary(const ContainedPtr& p)
{
    string comment = p->comment();

    start("para");
    string::size_type pos = comment.find('.');
    if (pos != string::npos)
    {
	comment.erase(pos + 1);
	O.zeroIndent();
	O << nl << comment;
	O.restoreIndent();
    }
    end();
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
Slice::Gen::start(const std::string& element, const std::string& title)
{
    start(element);
    start("title");
    O << nl << title;
    end();
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
Slice::Gen::scopedToId(const string& scoped)
{
    string s;
    if (scoped[0] == ':')
    {
	s = scoped.substr(2);
    }
    else
    {
	s = scoped;
    }

    int id = _idMap[s];
    if (id == 0)
    {
	id = ++_nextId;
	_idMap[s] = id;
    }

    ostringstream result;
    result << '"' << _idPrefix << '.' << id << '"';
    return result.str();
}

string
Slice::Gen::getScopedMinimized(const ContainedPtr& contained, const ContainerPtr& container)
{
    string s = contained->scoped();
    ContainerPtr p = container;
    ContainedPtr q;

    while((q = ContainedPtr::dynamicCast(p)))
    {
	string s2 = q->scoped();
	s2 += "::";

	if (s.find(s2) == 0)
	{
	    return s.substr(s2.size());
	}

	p = q->container();
    }

    return s;
}

string
Slice::Gen::toString(const SyntaxTreeBasePtr& p, const ContainerPtr& container)
{
    static const char* builtinTable[] =
    {
	"byte",
	"bool",
	"short",
	"int",
	"long",
	"float",
	"double",
	"string",
	"wstring",
	"Object",
	"Object*",
	"LocalObject"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    if (builtin)
    {
	return "<type>" + string(builtinTable[builtin->kind()]) + "</type>";
    }

    string tag;
    string linkend;
    string s;

    ProxyPtr proxy = ProxyPtr::dynamicCast(p);
    if (proxy)
    {
	tag = "classname";
	linkend = scopedToId(proxy->_class()->scoped());
	s = getScopedMinimized(proxy->_class(), container);
	s += "*";
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(p);
    if (cl)
    {
	tag = "classname";
	linkend = scopedToId(cl->scoped());
	s = getScopedMinimized(cl, container);
    }

    StructPtr st = StructPtr::dynamicCast(p);
    if (st)
    {
	tag = "structname";
	linkend = scopedToId(st->scoped());
	s = getScopedMinimized(st, container);
    }

    if (s.empty())
    {
	ContainedPtr contained = ContainedPtr::dynamicCast(p);
	assert(contained);
	tag = "type";
	linkend = scopedToId(contained->scoped());
	s = getScopedMinimized(contained, container);
    }

    return "<link linkend=" + linkend + "><" + tag + ">" + s + "</" + tag + "></link>";
}

string
Slice::Gen::toString(const string& str, const ContainerPtr& container)
{
    string s = str;

    TypeList types = container->lookupType(s, false);
    if (!types.empty())
    {
	return toString(types.front(), container);
    }

    ContainedList contList = container->lookupContained(s, false);
    if (!contList.empty())
    {
	return toString(contList.front(), container);
    }

    return s;
}
