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

Slice::Gen::Gen(const string& name, const string& file, bool standAlone, bool noGlobals, bool chapter) :
    _standAlone(standAlone),
    _noGlobals(noGlobals)
{
    if (chapter)
    {
	_chapter = "chapter";
    }
    else
    {
	_chapter = "section";
    }
	
    O.open(file.c_str());
    if (!O)
    {
	cerr << name << ": can't open `" << file << "' for writing: " << strerror(errno) << endl;
	return;
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

    //
    // I don't want the top-level module to be sorted, therefore no
    // unit->sort() before or after the unit->sortContents().
    //
    unit->sortContents();

    unit->visit(this);
}

bool
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
//	start("section", "Overview");
	visitContainer(p);
//	end();
    }

    return true;
}

void
Slice::Gen::visitUnitEnd(const UnitPtr& p)
{
    if (_standAlone)
    {
	end();
    }
}

bool
Slice::Gen::visitModuleStart(const ModulePtr& p)
{
    start(_chapter + " id=" + containedToId(p), p->scoped().substr(2));
    start("section", "Overview");
    O.zeroIndent();
    O << nl << "<synopsis>module <classname>" << p->name() << "</classname></synopsis>";
    O.restoreIndent();
    printComment(p);
    visitContainer(p);
    end();

    return true;
}

void
Slice::Gen::visitContainer(const ContainerPtr& p)
{
    ModuleList modules = p->modules();
    if (!modules.empty())
    {
	start("section", "Module Index");
	start("variablelist");
	
	for (ModuleList::const_iterator q = modules.begin(); q != modules.end(); ++q)
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

    ClassList classesAndInterfaces = p->classes();
    ClassList classes;
    ClassList interfaces;
    remove_copy_if(classesAndInterfaces.begin(), classesAndInterfaces.end(), back_inserter(classes),
		   ::IceUtil::memFun(&ClassDef::isInterface));
    remove_copy_if(classesAndInterfaces.begin(), classesAndInterfaces.end(), back_inserter(interfaces),
		   not1(::IceUtil::memFun(&ClassDef::isInterface)));
    
    if (!classes.empty())
    {
	start("section", "Class Index");
	start("variablelist");
	
	for (ClassList::const_iterator q = classes.begin(); q != classes.end(); ++q)
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

    if (!interfaces.empty())
    {
	start("section", "Interface Index");
	start("variablelist");
	
	for (ClassList::const_iterator q = interfaces.begin(); q != interfaces.end(); ++q)
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

    ExceptionList exceptions = p->exceptions();
    if (!exceptions.empty())
    {
	start("section", "Exception Index");
	start("variablelist");
	
	for (ExceptionList::const_iterator q = exceptions.begin(); q != exceptions.end(); ++q)
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
    if (!structs.empty())
    {
	start("section", "Struct Index");
	start("variablelist");
	
	for (StructList::const_iterator q = structs.begin(); q != structs.end(); ++q)
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
    if (!sequences.empty())
    {
	start("section", "Sequence Index");
	start("variablelist");
	
	for (SequenceList::const_iterator q = sequences.begin(); q != sequences.end(); ++q)
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
    if (!dictionaries.empty())
    {
	start("section", "Dictionary Index");
	start("variablelist");
	
	for (DictionaryList::const_iterator q = dictionaries.begin(); q != dictionaries.end(); ++q)
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
    if (!enums.empty())
    {
	start("section", "Enum Index");
	start("variablelist");
	
	for (EnumList::const_iterator q = enums.begin(); q != enums.end(); ++q)
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
	for (SequenceList::const_iterator q = sequences.begin(); q != sequences.end(); ++q)
	{
	    TypePtr type = (*q)->type();
	    
	    start("section id=" + containedToId(*q), (*q)->name());
	    
	    O.zeroIndent();
	    O << nl << "<synopsis>";
	    if ((*q)->isLocal())
	    {
		O << "local ";
	    }
	    O << "sequence&lt; " << toString(type, p) << " &gt; <type>" << (*q)->name() << "</type>;</synopsis>";
	    O.restoreIndent();
	    
	    printComment(*q);
	    
	    end();
	}
    }
    
    {
	for (DictionaryList::const_iterator q = dictionaries.begin(); q != dictionaries.end(); ++q)
	{
	    TypePtr keyType = (*q)->keyType();
	    TypePtr valueType = (*q)->valueType();
	    
	    start("section id=" + containedToId(*q), (*q)->name());
	    
	    O.zeroIndent();
	    O << nl << "<synopsis>";
	    if ((*q)->isLocal())
	    {
		O << "local ";
	    }
	    O << "dictionary&lt; " << toString(keyType, p) << ", " << toString(valueType, p) << " &gt; <type>"
	      << (*q)->name() << "</type>;</synopsis>";
	    O.restoreIndent();
	    
	    printComment(*q);
	    
	    end();
	}
    }
}

bool
Slice::Gen::visitClassDefStart(const ClassDefPtr& p)
{
    start(_chapter + " id=" + containedToId(p), p->scoped().substr(2));

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
	ClassList::const_iterator q = bases.begin();
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
    if (!operations.empty())
    {
	start("section", "Operation Index");
	start("variablelist");
	
	for (OperationList::const_iterator q = operations.begin(); q != operations.end(); ++q)
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
    if (!dataMembers.empty())
    {
	start("section", "Data Member Index");
	start("variablelist");
	
	for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
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
	for (OperationList::const_iterator q = operations.begin(); q != operations.end(); ++q)
	{
	    bool nonmutating = (*q)->nonmutating();
	    TypePtr returnType = (*q)->returnType();
	    TypeStringList inputParams = (*q)->inputParameters();
	    TypeStringList outputParams = (*q)->outputParameters();
	    ExceptionList throws =  (*q)->throws();
	    
	    start("section id=" + containedToId(*q), (*q)->name());
	    
	    O.zeroIndent();
	    O << nl << "<synopsis>" << (nonmutating ? "nonmutating " : "")
	      << (returnType ? toString(returnType, p) : "<type>void</type>")
	      << " <function>" << (*q)->name() << "</function>(";
	    O.inc();
	    TypeStringList::const_iterator r = inputParams.begin();
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
		ExceptionList::const_iterator r = throws.begin();
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
	for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
	{
	    TypePtr type = (*q)->type();
	    
	    start("section id=" + containedToId(*q), (*q)->name());
	    
	    O.zeroIndent();
	    O << nl << "<synopsis>" << toString(type, p) << " <structfield>" << (*q)->name()
	      << "</structfield>;</synopsis>";
	    O.restoreIndent();
	    
	    printComment(*q);
	    
	    end();
	}
    }
	
    end();

    return true;
}

bool
Slice::Gen::visitExceptionStart(const ExceptionPtr& p)
{
    start(_chapter + " id=" + containedToId(p), p->scoped().substr(2));

    start("section", "Overview");
    O.zeroIndent();
    O << nl << "<synopsis>";
    if (p->isLocal())
    {
	O << "local ";
    }
    O << "exception <classname>" << p->name() << "</classname>";
    ExceptionPtr base = p->base();
    if (base)
    {
	O.inc();
	O << nl << "extends ";
	O.inc();
	O << nl << toString(base, p);
	O.dec();
	O.dec();
    }
    O << "</synopsis>";
    O.restoreIndent();

    printComment(p);

    DataMemberList dataMembers = p->dataMembers();
    if (!dataMembers.empty())
    {
	start("section", "Data Member Index");
	start("variablelist");
	
	for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
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
	for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
	{
	    TypePtr type = (*q)->type();
	    
	    start("section id=" + containedToId(*q), (*q)->name());
	    
	    O.zeroIndent();
	    O << nl << "<synopsis>" << toString(type, p) << " <structfield>" << (*q)->name()
	      << "</structfield>;</synopsis>";
	    O.restoreIndent();
	    
	    printComment(*q);
	    
	    end();
	}
    }
	
    end();

    return true;
}

bool
Slice::Gen::visitStructStart(const StructPtr& p)
{
    start(_chapter + " id=" + containedToId(p), p->scoped().substr(2));

    start("section", "Overview");
    O.zeroIndent();
    O << nl << "<synopsis>";
    if (p->isLocal())
    {
	O << "local ";
    }
    O << "struct <structname>" << p->name() << "</structname>";
    O << "</synopsis>";
    O.restoreIndent();

    printComment(p);

    DataMemberList dataMembers = p->dataMembers();
    if (!dataMembers.empty())
    {
	start("section", "Data Member Index");
	start("variablelist");
	
	for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
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
	for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
	{
	    TypePtr type = (*q)->type();
	    
	    start("section id=" + containedToId(*q), (*q)->name());
	    
	    O.zeroIndent();
	    O << nl << "<synopsis>" << toString(type, p) << " <structfield>" << (*q)->name()
	      << "</structfield>;</synopsis>";
	    O.restoreIndent();
	    
	    printComment(*q);
	    
	    end();
	}
    }
	
    end();

    return true;
}

void
Slice::Gen::visitEnum(const EnumPtr& p)
{
    start(_chapter + " id=" + containedToId(p), p->scoped().substr(2));

    start("section", "Overview");
    O.zeroIndent();
    O << nl << "<synopsis>";
    if (p->isLocal())
    {
	O << "local ";
    }
    O << "enum <type>" << p->name() << "</type>";
    O << "</synopsis>";
    O.restoreIndent();

    printComment(p);

    EnumeratorList enumerators = p->getEnumerators();
    if (!enumerators.empty())
    {
	start("section", "Enumerator Index");
	start("variablelist");
	
	for (EnumeratorList::const_iterator q = enumerators.begin(); q != enumerators.end(); ++q)
	{
	    start("varlistentry");
	    start("term");
	    O << nl << toString(*q, p->container());
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
	for (EnumeratorList::const_iterator q = enumerators.begin(); q != enumerators.end(); ++q)
	{
	    start("section id=" + containedToId(*q), (*q)->name());
	    
	    O.zeroIndent();
	    O << nl << "<synopsis><constant>" << (*q)->name() << "</constant></synopsis>";
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

string
Slice::Gen::getComment(const ContainedPtr& contained, const ContainerPtr& container, bool summary)
{
    string s = contained->comment();
    string comment;
    for (unsigned int i = 0; i < s.size(); ++i)
    {
	if (s[i] == '\\' && i + 1 < s.size() && s[i + 1] == '[')
	{
	    comment += '[';
	    ++i;
	}
	else if (s[i] == '[')
	{
	    ++i;
	    string literal;
	    while (s[i] != ']')
	    {
		literal += s[i++];
		if (i >= s.size())
		{
		    break;
		}
	    }
	    comment += toString(literal, container);
	}
	else if(summary && s[i] == '.' && (i + 1 >= s.size() || isspace(s[i + 1])))
	{
	    comment += '.';
	    break;
	}
	else
	{
	    comment += s[i];
	}

    }
    return comment;
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

    string comment = getComment(p, container, false);
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
	for (StringList::const_iterator q = par.begin(); q != par.end(); ++q)
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
	
	for (StringList::const_iterator q = throws.begin(); q != throws.end(); ++q)
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

    ClassList derivedClasses;
    ClassDefPtr def = ClassDefPtr::dynamicCast(p);
    if (def)
    {
	derivedClasses = p->unit()->findDerivedClasses(def);
    }
    if (!derivedClasses.empty())
    {
	start("section", "Derived Classes and Interfaces");
	start("para");
	start("simplelist type=\"inline\"");
	
	for (ClassList::const_iterator q = derivedClasses.begin(); q != derivedClasses.end(); ++q)
	{
	    start("member");
	    O << nl << toString(*q, container);
	    end();
	}
	
	end();
	end();
	end();
    }

    ExceptionList derivedExceptions;
    ExceptionPtr ex = ExceptionPtr::dynamicCast(p);
    if (ex)
    {
	derivedExceptions = p->unit()->findDerivedExceptions(ex);
    }
    if (!derivedExceptions.empty())
    {
	start("section", "Derived Exceptions");
	start("para");
	start("simplelist type=\"inline\"");
	
	for (ExceptionList::const_iterator q = derivedExceptions.begin(); q != derivedExceptions.end(); ++q)
	{
	    start("member");
	    O << nl << toString(*q, container);
	    end();
	}
	
	end();
	end();
	end();
    }

    ContainedList usedBy;
    ConstructedPtr constructed;
    if (def)
    {
	constructed = def->declaration();
    }
    else
    {
	constructed = ConstructedPtr::dynamicCast(p);
    }
    if (constructed)
    {
	usedBy = p->unit()->findUsedBy(constructed);
    }
    if (!usedBy.empty())
    {
	start("section", "Used By");
	start("para");
	start("simplelist type=\"inline\"");
	
	for (ContainedList::const_iterator q = usedBy.begin(); q != usedBy.end(); ++q)
	{
	    start("member");
	    O << nl << toString(*q, container);
	    end();
	}

	end();
	end();
	end();
    }

    if (!see.empty())
    {
	start("section", "See Also");
	start("para");
	start("simplelist type=\"inline\"");
	
	for (StringList::const_iterator q = see.begin(); q != see.end(); ++q)
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
    ContainerPtr container = ContainerPtr::dynamicCast(p);
    if (!container)
    {
	container = p->container();
    }

    string summary = getComment(p, container, true);
    start("para");
    O.zeroIndent();
    O << nl << summary;
    O.restoreIndent();
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
Slice::Gen::containedToId(const ContainedPtr& contained)
{
    assert(contained);

    string scoped = contained->scoped();
    if (scoped[0] == ':')
    {
	scoped.erase(0, 2);
    }

    string id = "\"";

    id.reserve(scoped.size() + 2); // Also reserve space for the two '"'

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

    //
    // TODO: At present, docbook tools limit link names (NAMELEN) to
    // 44 characters.
    //
    if(id.size() > 1 + 44)
    {
	id.erase(1 + 44);
    }

    id += '"';
    return id;
}

string
Slice::Gen::getScopedMinimized(const ContainedPtr& contained, const ContainerPtr& container)
{
    string s = contained->scoped();
    ContainerPtr p = container;
    ContainedPtr q = ContainedPtr::dynamicCast(p);

    if (!q) // Container is the global module
    {
	return s.substr(2);
    }
    
    do
    {
	string s2 = q->scoped();
	s2 += "::";

	if (s.find(s2) == 0)
	{
	    return s.substr(s2.size());
	}

	p = q->container();
	q = ContainedPtr::dynamicCast(p);
    }
    while(q);

    return s;
}

string
Slice::Gen::toString(const SyntaxTreeBasePtr& p, const ContainerPtr& container)
{
    string tag;
    string linkend;
    string s;

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
	s = builtinTable[builtin->kind()];
	tag = "type";
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(p);
    if (proxy)
    {
	linkend = containedToId(proxy->_class());
	s = getScopedMinimized(proxy->_class(), container);
	s += "*";
	tag = "classname";
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(p);
    if (cl)
    {
	//
        // We must generate the id from the definition, not from the
        // declaration, provided that a definition is available.
	//
	ContainedPtr definition = cl->definition();
	if (definition)
	{
	    linkend = containedToId(definition);
	}
	s = getScopedMinimized(cl, container);
	tag = "classname";
    }

    ExceptionPtr ex = ExceptionPtr::dynamicCast(p);
    if (ex)
    {
	linkend = containedToId(ex);
	s = getScopedMinimized(ex, container);
	tag = "classname";
    }

    StructPtr st = StructPtr::dynamicCast(p);
    if (st)
    {
	linkend = containedToId(st);
	s = getScopedMinimized(st, container);
	tag = "structname";
    }

    EnumeratorPtr en = EnumeratorPtr::dynamicCast(p);
    if (en)
    {
	linkend = containedToId(en);
	s = getScopedMinimized(en, container);
	tag = "constant";
    }

    if (s.empty())
    {
	ContainedPtr contained = ContainedPtr::dynamicCast(p);
	assert(contained);
	linkend = containedToId(contained);
	s = getScopedMinimized(contained, container);
	tag = "type";
    }

    if (linkend.empty())
    {
	return "<" + tag + ">" + s + "</" + tag + ">";
    }
    else
    {
	return "<link linkend=" + linkend + "><" + tag + ">" + s + "</" + tag + "></link>";
    }
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

    //
    // If we can't find the string, printing it as "literal" is the
    // best we can do.
    //
    return "<literal>" + s + "</literal>";
}
