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
#include <Slice/Parser.h>

using namespace std;
using namespace Slice;

extern FILE* yyin;
extern int yydebug;

namespace Slice
{

Unit* unit;

}

// ----------------------------------------------------------------------
// to_lower() helper function
// ----------------------------------------------------------------------

// TODO: ML: Should be toLower().
static void
to_lower(string& s)
{
    transform(s.begin(), s.end(), s.begin(), tolower);
}

// ----------------------------------------------------------------------
// SyntaxTreeBase
// ----------------------------------------------------------------------

void
Slice::SyntaxTreeBase::destroy()
{
    _unit = 0;
}

UnitPtr
Slice::SyntaxTreeBase::unit() const
{
    return _unit;
}

void
Slice::SyntaxTreeBase::visit(ParserVisitor*)
{
}

Slice::SyntaxTreeBase::SyntaxTreeBase(const UnitPtr& unit) :
    _unit(unit)
{
}

// ----------------------------------------------------------------------
// Type
// ----------------------------------------------------------------------

Slice::Type::Type(const UnitPtr& unit) :
    SyntaxTreeBase(unit)
{
}

// ----------------------------------------------------------------------
// Builtin
// ----------------------------------------------------------------------

Builtin::Kind
Slice::Builtin::kind()
{
    return _kind;
}

Slice::Builtin::Builtin(const UnitPtr& unit, Kind kind) :
    Type(unit),
    SyntaxTreeBase(unit),
    _kind(kind)
{
}

// ----------------------------------------------------------------------
// Contained
// ----------------------------------------------------------------------

ContainerPtr
Slice::Contained::container() const
{
    return _container;
}

string
Slice::Contained::name()
{
    return _name;
}

string
Slice::Contained::scoped()
{
    return _scoped;
}

string
Slice::Contained::scope() const
{
    string::size_type idx = _scoped.rfind("::");
    assert(idx != string::npos);
    return string(_scoped, 0, idx + 2);
}

string
Slice::Contained::file() const
{
    return _file;
}

string
Slice::Contained::comment() const
{
    return _comment;
}

int
Slice::Contained::includeLevel()
{
    return _includeLevel;
}

void
Slice::Contained::updateIncludeLevel()
{
    _includeLevel = min(_includeLevel, _unit->currentIncludeLevel());
}

list<string>
Slice::Contained::getMetaData() const
{
    return _metaData;
}

void
Slice::Contained::setMetaData(const list<string>& metaData)
{
    _metaData = metaData;
}

bool
Slice::Contained::operator<(const Contained& rhs) const
{
    return _scoped < rhs._scoped;
}

bool
Slice::Contained::operator==(const Contained& rhs) const
{
    return _scoped == rhs._scoped;
}

Slice::Contained::Contained(const ContainerPtr& container, const string& name) :
    SyntaxTreeBase(container->unit()),
    _container(container),
    _name(name)
{
    ContainedPtr cont = ContainedPtr::dynamicCast(_container);
    if(cont)
    {
	_scoped = cont->scoped();
    }
    _scoped += "::" + _name;
    assert(_unit);
    _unit->addContent(this);
    _file = _unit->currentFile();
    _comment = _unit->currentComment();
    _includeLevel = _unit->currentIncludeLevel();
}

// ----------------------------------------------------------------------
// Container
// ----------------------------------------------------------------------

void
Slice::Container::destroy()
{
    for_each(_contents.begin(), _contents.end(), ::IceUtil::voidMemFun(&Contained::destroy));
    _contents.clear();
    SyntaxTreeBase::destroy();
}

ModulePtr
Slice::Container::createModule(const string& name)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    for(ContainedList::const_iterator p = matches.begin(); p != matches.end(); ++p)
    {
	string msg;
	bool differsOnlyInCase = matches.front()->name() != name;
	ModulePtr module = ModulePtr::dynamicCast(*p);
	if(module)
	{
	    if(!differsOnlyInCase)
	    {
		continue;	// Reopening modules is permissible...
	    }
	    else		// ... but only if they are capitalized correctly
	    {
		msg += "module `" + name + "' is capitalized inconsistently with its previous name: `";
		msg += module->name() + "'";
		_unit->warning(msg);	// TODO: Change to error in stable_39
		// TODO: ML: continue (or later return 0) missing;
	    }
	}
	
	if(differsOnlyInCase)
	{
	    msg = "module `" + name + "' differs only in capitalization from ";
	    msg += matches.front()->kindOf() + " name `" + matches.front()->name() + "'";
	    _unit->warning(msg);		// TODO: Change to error in stable_39
	}
	else
	{
	    msg = "redefinition of " + matches.front()->kindOf() + " `" + matches.front()->name();
	    msg += "' as module";
	    _unit->error(msg);
	    return 0;
	}
    }

    ModulePtr q = new Module(this, name);
    _contents.push_back(q);
    return q;
}

ClassDefPtr
Slice::Container::createClassDef(const string& name, bool intf, const ClassList& bases, bool local)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    for(ContainedList::const_iterator p = matches.begin(); p != matches.end(); ++p)
    {
	ClassDeclPtr decl = ClassDeclPtr::dynamicCast(*p);
	if(decl)
	{
	    if(checkInterfaceAndLocal(name, false, intf, decl->isInterface(), local, decl->isLocal()))
	    {
		continue;
	    }
	    return 0;
	}

	string msg;
	bool differsOnlyInCase = matches.front()->name() != name;
	ClassDefPtr def = ClassDefPtr::dynamicCast(*p);
	if(def)
	{
	    if(_unit->ignRedefs())
	    {
		def->updateIncludeLevel();
		return def;
	    }
	    if (!differsOnlyInCase)
	    {
		string msg = "redefinition of ";
		msg += intf ? "interface" : "class";
		msg += " `" + name + "'";
		_unit->error(msg);
		return 0;
	    }
	    else
	    {
		msg = intf ? "interface" : "class";
		msg += " definition `" + name + "' is capitalized inconsistently with its previous name: `";
		msg += def->name() + "'";
		_unit->warning(msg);	// TODO: Change to error in stable_39
	    }
	}

	if(differsOnlyInCase)
	{
	    msg = intf ? "interface" : "class";
	    msg = " definition `" + name + "' differs only in capitalization from ";
	    msg += matches.front()->kindOf() + " name `" + matches.front()->name() + "'";
	    _unit->warning(msg);		// TODO: Change to error in stable_39
	}
	else
	{
	    msg = "redefinition of " + matches.front()->kindOf() + " `" + matches.front()->name() + "' as ";
	    msg += intf ? "interface" : "class";
	    _unit->error(msg);
	    return 0;
	}
	
	return 0;
    }

    //
    // Check whether, for multiple inheritance, any of the bases define
    // the same operations.
    //
    if(bases.size() > 1)
    {
	//
	// We have multiple inheritance. Build a list of paths through the
	// inheritance graph, such that multiple inheritance is legal if
	// the union of the names defined in classes on each path are disjoint.
	//
	GraphPartitionList gpl;
	for(ClassList::const_iterator p = bases.begin(); p != bases.end(); ++p)
	{
	    ClassList cl;
	    gpl.push_back(cl);
	    addPartition(gpl, gpl.rbegin(), *p);
	}

	//
	// We now have a list of partitions, with each partition containing
	// a list of class definitions. Turn the list of partitions of class
	// definitions into a list of sets of strings, with each
	// set containing the names of operations and data members defined in
	// the classes in each partition.
	//
	StringSetList spl = toStringSetList(gpl);

	//
	// Multiple inheritance is legal if no two partitions contain a common
	// name (that is, if the union of the intersections of all possible pairs
	// of partitions is empty.
	//
	StringList clashes = unionOfAllPairIntersections(spl);
	
	//
	// The strings in the clashes list are those operations or data members
	// that prevent the multiple inheritance hierarchies about to the
	// current class to be joined.
	//
	for(StringList::const_iterator q = clashes.begin(); q != clashes.end(); ++q)
	{
	    string msg = "ambiguous multiple inheritance: `";
	    msg += *q + "' is defined in two or more base classes";
	    _unit->error(msg);
	}
	//
	// We do *not* return 0 here; otherwise, we get lots of spurious
	// "Parse error" messsages and end up with lots of follow-up errors
	// about this class not being defined.
	//
    }

    ClassDefPtr def = new ClassDef(this, name, intf, bases, local);
    _contents.push_back(def);

    for(ContainedList::const_iterator q = matches.begin(); q != matches.end(); ++q)
    {
	ClassDeclPtr decl = ClassDeclPtr::dynamicCast(*q);
	decl->_definition = def;
    }

    //
    // Implicitly create a class declaration for each class
    // definition. This way the code generator can rely on always
    // having a class declaration available for lookup.
    //
    ClassDeclPtr decl = createClassDecl(name, intf, local);
    def->_declaration = decl;

    return def;
}

ClassDeclPtr
Slice::Container::createClassDecl(const string& name, bool intf, bool local)
{
    ClassDefPtr def;

    ContainedList matches = _unit->findContents(thisScope() + name);
    for(ContainedList::const_iterator p = matches.begin(); p != matches.end(); ++p)
    {
	ClassDefPtr clDef = ClassDefPtr::dynamicCast(*p);
	if(clDef)
	{
	    if(checkInterfaceAndLocal(name, true, intf, clDef->isInterface(), local, clDef->isLocal()))
	    {
		assert(!def);
		def = clDef;
		continue;
	    }

	    return 0;
	}
	
	ClassDeclPtr clDecl = ClassDeclPtr::dynamicCast(*p);
	if(clDecl)
	{
	    if(checkInterfaceAndLocal(name, false, intf, clDecl->isInterface(), local, clDecl->isLocal()))
	    {
		continue;
	    }

	    return 0;
	}
	
	string msg;
	bool differsOnlyInCase = matches.front()->name() != name;
	if (differsOnlyInCase)
	{
	    msg = "class declaration `" + name + "' differs only in capitalization from ";
	    msg += matches.front()->kindOf() + " name `" + matches.front()->name() + "'";
	    _unit->warning(msg);		// TODO: Change to error in stable_39
	}
	else
    	{
	    msg = "declaration of already defined `";
	    msg += name;
	    msg += "' as ";
	    msg += intf ? "interface" : "class";
	    _unit->error(msg);
	    return 0;
	}
    }

    //
    // Multiple declarations are permissible. But if we do already
    // have a declaration for the class in this container, we don't
    // create another one.
    //
    for(ContainedList::const_iterator q = _contents.begin(); q != _contents.end(); ++q)
    {
	if((*q)->name() == name)
	{
	    ClassDeclPtr decl = ClassDeclPtr::dynamicCast(*q);
	    if(decl)
	    {
		return decl;
	    }

	    assert(ClassDefPtr::dynamicCast(*q));
	}
    }

    ClassDeclPtr decl = new ClassDecl(this, name, intf, local);
    _contents.push_back(decl);

    if(def)
    {
	decl->_definition = def;
    }

    return decl;
}

ExceptionPtr
Slice::Container::createException(const string& name, const ExceptionPtr& base, bool local)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    if(!matches.empty())
    {
	ExceptionPtr p = ExceptionPtr::dynamicCast(matches.front());
	if(p)
	{
	    if(_unit->ignRedefs())
	    {
		p->updateIncludeLevel();
		return p;
	    }
	}
	string msg;
	if (matches.front()->name() == name)
	{
	    msg = "redefinition of " + matches.front()->kindOf() + " `" + matches.front()->name();
	    msg += "' as exception";
	    _unit->error(msg);
	    return 0;
	}
	msg = "exception `" + name + "' differs only in capitalization from ";
	msg += matches.front()->kindOf() + " `" + matches.front()->name() + "'";
	_unit->warning(msg);	// TODO: Change to error in stable_39
    }

    ExceptionPtr p = new Exception(this, name, base, local);
    _contents.push_back(p);
    return p;
}

StructPtr
Slice::Container::createStruct(const string& name, bool local)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    if(!matches.empty())
    {
	StructPtr p = StructPtr::dynamicCast(matches.front());
	if(p)
	{
	    if(_unit->ignRedefs())
	    {
		p->updateIncludeLevel();
		return p;
	    }
	}
	string msg;
	if (matches.front()->name() == name)
	{
	    msg = "redefinition of " + matches.front()->kindOf() + " `" + matches.front()->name();
	    msg += "' as struct";
	    _unit->error(msg);
	    return 0;
	}
	msg = "struct `" + name + "' differs only in capitalization from ";
	msg += matches.front()->kindOf() + " `" + matches.front()->name() + "'";
	_unit->warning(msg);	// TODO: Change to error in stable_39
    }

    StructPtr p = new Struct(this, name, local);
    _contents.push_back(p);
    return p;
}

SequencePtr
Slice::Container::createSequence(const string& name, const TypePtr& type, bool local)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    if(!matches.empty())
    {
	SequencePtr p = SequencePtr::dynamicCast(matches.front());
	if(p)
	{
	    if(_unit->ignRedefs())
	    {
		return p;
	    }
	}
	string msg;
	if (matches.front()->name() == name)
	{
	    msg = "redefinition of " + matches.front()->kindOf() + " `" + matches.front()->name();
	    msg += "' as sequence";
	    _unit->error(msg);
	    return 0;
	}
	msg = "sequence `" + name + "' differs only in capitalization from ";
	msg += matches.front()->kindOf() + " `" + matches.front()->name() + "'";
	_unit->warning(msg);	// TODO: change to error in stable_39
    }
    
    SequencePtr p = new Sequence(this, name, type, local);
    _contents.push_back(p);
    return p;
}

DictionaryPtr
Slice::Container::createDictionary(const string& name, const TypePtr& keyType, const TypePtr& valueType, bool local)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    if(!matches.empty())
    {
	DictionaryPtr p = DictionaryPtr::dynamicCast(matches.front());
	if(p)
	{
	    if(_unit->ignRedefs())
	    {
		return p;
	    }
	}
	string msg;
	if (matches.front()->name() == name)
	{
	    msg = "redefinition of " + matches.front()->kindOf() + " `" + matches.front()->name();
	    msg += "' as dictionary";
	    _unit->error(msg);
	    return 0;
	}
	msg = "dictionary `" + name + "' differs only in capitalization from ";
	msg += matches.front()->kindOf() + " `" + matches.front()->name() + "'";
	_unit->warning(msg);	// TODO: change to error in stable_39
    }

    DictionaryPtr p = new Dictionary(this, name, keyType, valueType, local);
    _contents.push_back(p);
    return p;
}

EnumPtr
Slice::Container::createEnum(const string& name, bool local)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    if(!matches.empty())
    {
	EnumPtr p = EnumPtr::dynamicCast(matches.front());
	if(p)
	{
	    if(_unit->ignRedefs())
	    {
		return p;
	    }
	}
	string msg;
	if (matches.front()->name() == name)
	{
	    msg = "redefinition of " + matches.front()->kindOf() + " `" + matches.front()->name();
	    msg += "' as enumeration";
	    _unit->error(msg);
	    return 0;
	}
	msg = "enumeration `" + name + "' differs only in capitalization from ";
	msg += matches.front()->kindOf() + " `" + matches.front()->name() + "'";
	_unit->warning(msg);	// TODO: change to error in stable_39
    }

    EnumPtr p = new Enum(this, name, local);
    _contents.push_back(p);
    return p;
}

EnumeratorPtr
Slice::Container::createEnumerator(const string& name)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    if(!matches.empty())
    {
	EnumeratorPtr p = EnumeratorPtr::dynamicCast(matches.front());
	if(p)
	{
	    if(_unit->ignRedefs())
	    {
		return p;
	    }
	}
	string msg;
	if (matches.front()->name() == name)
	{
	    msg = "redefinition of " + matches.front()->kindOf() + " `" + matches.front()->name();
	    msg += "' as enumerator";
	    _unit->error(msg);
	    return 0;
	}
	msg = "enumerator `" + name + "' differs only in capitalization from ";
	msg += matches.front()->kindOf() + " `" + matches.front()->name() + "'";
	_unit->warning(msg);	// TODO: change to error in stable_39
    }

    EnumeratorPtr p = new Enumerator(this, name);
    _contents.push_back(p);
    return p;
}

TypeList
Slice::Container::lookupType(const string& scoped, bool printError)
{
    //
    // Remove whitespace.
    //
    string sc = scoped;
    string::size_type pos;
    while((pos = sc.find_first_of(" \t\r\n")) != string::npos)
    {
	sc.erase(pos, 1);
    }

    //
    // Check for builtin type.
    //
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
	"Object",
	"Object*",
	"LocalObject"
    };

    for(unsigned int i = 0; i < sizeof(builtinTable) / sizeof(const char*); ++i)
    {
	if(sc == builtinTable[i])
	{
	    TypeList result;
	    result.push_back(_unit->builtin(static_cast<Builtin::Kind>(i)));
	    return result;
	}
    }

    //
    // Not a builtin type, try to look up a constructed type.
    //
    return lookupTypeNoBuiltin(scoped, printError);
}

TypeList
Slice::Container::lookupTypeNoBuiltin(const string& scoped, bool printError)
{
    //
    // Remove whitespace.
    //
    string sc = scoped;
    string::size_type pos;
    while((pos = sc.find_first_of(" \t\r\n")) != string::npos)
    {
	sc.erase(pos, 1);
    }

    //
    // Absolute scoped name?
    //
    if(sc.size() >= 2 && sc[0] == ':')
    {
	return _unit->lookupTypeNoBuiltin(sc.substr(2), printError);
    }

    TypeList results;
    if(sc.rfind('*') == sc.length() - 1)
    {
	//
	// Proxies.
	//
	ContainedList matches = _unit->findContents(thisScope() + sc.substr(0, sc.length() - 1));
	for(ContainedList::const_iterator p = matches.begin(); p != matches.end(); ++p)
	{
	    ClassDefPtr def = ClassDefPtr::dynamicCast(*p);
	    if(def)
	    {
		continue; // Ignore class definitions
	    }

	    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(*p);
	    if(!cl)
	    {
		if(printError)
		{
		    string msg = "`";
		    msg += sc;
		    msg += "' must be class or interface";
		    _unit->error(msg);
		}
		return TypeList();
	    }
	    results.push_back(new Proxy(cl));
	}
    }
    else
    {
	//
	// Non-Proxies.
	//
	ContainedList matches = _unit->findContents(thisScope() + sc);
	for(ContainedList::const_iterator p = matches.begin(); p != matches.end(); ++p)
	{
	    ClassDefPtr def = ClassDefPtr::dynamicCast(*p);
	    if(def)
	    {
		continue; // Ignore class definitions
	    }

	    ExceptionPtr ex = ExceptionPtr::dynamicCast(*p);
	    if(ex)
	    {
		if(printError)
		{
		    string msg = "`";
		    msg += sc;
		    msg += "' is an exception, which cannot be used as a type";
		    _unit->error(msg);
		}
		return TypeList();
	    }

	    TypePtr type = TypePtr::dynamicCast(*p);
	    if(!type)
	    {
		if(printError)
		{
		    string msg = "`";
		    msg += sc;
		    msg += "' is not a type";
		    _unit->error(msg);
		}
		return TypeList();
	    }
	    results.push_back(type);
	}
    }

    if(results.empty())
    {
	ContainedPtr contained = ContainedPtr::dynamicCast(this);
	if(!contained)
	{
	    if(printError)
	    {
		string msg = "`";
		msg += sc;
		msg += "' is not defined";
		_unit->error(msg);
	    }
	    return TypeList();
	}
	return contained->container()->lookupTypeNoBuiltin(sc, printError);
    }
    else
    {
	return results;
    }
}

ContainedList
Slice::Container::lookupContained(const string& scoped, bool printError)
{
    //
    // Remove whitespace.
    //
    string sc = scoped;
    string::size_type pos;
    while((pos = sc.find_first_of(" \t\r\n")) != string::npos)
    {
	sc.erase(pos, 1);
    }

    //
    // Absolute scoped name?
    //
    if(sc.size() >= 2 && sc[0] == ':')
    {
	return _unit->lookupContained(sc.substr(2), printError);
    }

    ContainedList matches = _unit->findContents(thisScope() + sc);
    ContainedList results;
    for(ContainedList::const_iterator p = matches.begin(); p != matches.end(); ++p)
    {
	if(!ClassDefPtr::dynamicCast(*p)) // Ignore class definitions
	{
	    results.push_back(*p);
	}
    }

    if(results.empty())
    {
	ContainedPtr contained = ContainedPtr::dynamicCast(this);
	if(!contained)
	{
	    if(printError)
	    {
		string msg = "`";
		msg += sc;
		msg += "' is not defined";
		_unit->error(msg);
	    }
	    return ContainedList();
	}
	return contained->container()->lookupContained(sc, printError);
    }
    else
    {
	return results;
    }
}

ExceptionPtr
Slice::Container::lookupException(const string& scoped, bool printError)
{
    ContainedList contained = lookupContained(scoped, printError);
    if(contained.empty())
    {
	return 0;
    }

    ExceptionList exceptions;
    for(ContainedList::iterator p = contained.begin(); p != contained.end(); ++p)
    {
	ExceptionPtr ex = ExceptionPtr::dynamicCast(*p);
	if(!ex)
	{
	    if(printError)
	    {
		string msg = "`";
		msg += scoped;
		msg += "' is not an exception";
		_unit->error(msg);
	    }
	    return 0;
	}
	exceptions.push_back(ex);
    }
    assert(exceptions.size() == 1);
    return exceptions.front();
}

ModuleList
Slice::Container::modules() const
{
    ModuleList result;
    for(ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	ModulePtr q = ModulePtr::dynamicCast(*p);
	if(q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

ClassList
Slice::Container::classes() const
{
    ClassList result;
    for(ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	ClassDefPtr q = ClassDefPtr::dynamicCast(*p);
	if(q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

ExceptionList
Slice::Container::exceptions() const
{
    ExceptionList result;
    for(ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	ExceptionPtr q = ExceptionPtr::dynamicCast(*p);
	if(q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

StructList
Slice::Container::structs() const
{
    StructList result;
    for(ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	StructPtr q = StructPtr::dynamicCast(*p);
	if(q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

SequenceList
Slice::Container::sequences() const
{
    SequenceList result;
    for(ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	SequencePtr q = SequencePtr::dynamicCast(*p);
	if(q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

DictionaryList
Slice::Container::dictionaries() const
{
    DictionaryList result;
    for(ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	DictionaryPtr q = DictionaryPtr::dynamicCast(*p);
	if(q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

EnumList
Slice::Container::enums() const
{
    EnumList result;
    for(ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	EnumPtr q = EnumPtr::dynamicCast(*p);
	if(q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

bool
Slice::Container::hasNonLocalClassDecls() const
{
    for(ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	ClassDeclPtr cl = ClassDeclPtr::dynamicCast(*p);
	if(cl && !cl->isLocal())
	{
	    return true;
	}

	ContainerPtr container = ContainerPtr::dynamicCast(*p);
	if(container && container->hasNonLocalClassDecls())
	{
	    return true;
	}
    }

    return false;
}

bool
Slice::Container::hasClassDecls() const
{
    for(ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	if(ClassDeclPtr::dynamicCast(*p))
	{
	    return true;
	}

	ContainerPtr container = ContainerPtr::dynamicCast(*p);
	if(container && container->hasClassDecls())
	{
	    return true;
	}
    }

    return false;
}

bool
Slice::Container::hasClassDefs() const
{
    for(ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	if(ClassDefPtr::dynamicCast(*p))
	{
	    return true;
	}

	ContainerPtr container = ContainerPtr::dynamicCast(*p);
	if(container && container->hasClassDefs())
	{
	    return true;
	}
    }

    return false;
}

bool
Slice::Container::hasOtherConstructedOrExceptions() const
{
    for(ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	if(ConstructedPtr::dynamicCast(*p) && !ClassDeclPtr::dynamicCast(*p) && !ClassDefPtr::dynamicCast(*p))
	{
	    return true;
	}

	if(ExceptionPtr::dynamicCast(*p))
	{
	    return true;
	}

	ContainerPtr container = ContainerPtr::dynamicCast(*p);
	if(container && container->hasOtherConstructedOrExceptions())
	{
	    return true;
	}
    }

    return false;
}

string
Slice::Container::thisScope()
{
    string s;
    ContainedPtr contained = ContainedPtr::dynamicCast(this);
    if(contained)
    {
	s = contained->scoped();
    }
    s += "::";
    return s;
}

void
Slice::Container::mergeModules()
{
    for(ContainedList::iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	ModulePtr mod1 = ModulePtr::dynamicCast(*p);
	if(!mod1)
	{
	    continue;
	}
	
	ContainedList::iterator q = p;
	++q;
	while(q != _contents.end())
	{
	    ModulePtr mod2 = ModulePtr::dynamicCast(*q);
	    if(!mod2)
	    {
		++q;
		continue;
	    }

	    if(mod1->name() != mod2->name())
	    {
		++q;
		continue;
	    }

	    mod1->_contents.splice(mod1->_contents.end(), mod2->_contents);

	    if(mod1->_comment.length() < mod2->_comment.length())
	    {
		mod1->_comment.swap(mod2->_comment);
	    }

	    mod1->_includeLevel = min(mod1->_includeLevel, mod2->_includeLevel);

	    _unit->removeContent(*q);
	    q = _contents.erase(q);
	}
	
	mod1->mergeModules();
    }
}

void
Slice::Container::sort()
{
    _contents.sort();
}

void
Slice::Container::sortContents()
{
    for(ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	ContainerPtr container = ContainerPtr::dynamicCast(*p);
	if(container)
	{
	    container->sort();
	    container->sortContents();
	}
    }
}

void
Slice::Container::visit(ParserVisitor* visitor)
{
    for(ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	(*p)->visit(visitor);
    }
}

void
Slice::Container::containerRecDependencies(set<ConstructedPtr>& dependencies)
{
    ContainedList::iterator p;
    for(p = _contents.begin(); p != _contents.end(); ++p)
    {
	ConstructedPtr constructed = ConstructedPtr::dynamicCast(*p);
	if(constructed && dependencies.find(constructed) != dependencies.end())
	{
	    dependencies.insert(constructed);
	    constructed->recDependencies(dependencies);
	}
    }
}

Slice::Container::Container(const UnitPtr& unit) :
    SyntaxTreeBase(unit)
{
}

bool
Slice::Container::checkInterfaceAndLocal(const string& name, bool defined,
					 bool intf, bool intfOther,
					 bool local, bool localOther)
{
    string definedOrDeclared;
    if(defined)
    {
	definedOrDeclared = "defined";
    }
    else
    {
	definedOrDeclared = "declared";
    }

    if(!intf && intfOther)
    {
	string msg = "class `";
	msg += name;
	msg += "' was ";
	msg += definedOrDeclared;
	msg += " as interface";
	_unit->error(msg);
	return false;
    }
    
    if(intf && !intfOther)
    {
	string msg = "interface `";
	msg += name;
	msg += "' was ";
	msg += definedOrDeclared;
	msg += " as class";
	_unit->error(msg);
	return false;
    }
    
    if(!local && localOther)
    {
	string msg = "non-local `";
	msg += name;
	msg += "' was ";
	msg += definedOrDeclared;
	msg += " local";
	_unit->error(msg);
	return false;
    }
    
    if(local && !localOther)
    {
	string msg = "local `";
	msg += name;
	msg += "' was ";
	msg += definedOrDeclared;
	msg += " non-local";
	_unit->error(msg);
	return false;
    }
    
    return true;
}

//
// Return true if the class definition cdp is on one of the class lists in gpl, false otherwise.
//
bool
Slice::Container::isInList(const GraphPartitionList& gpl, const ClassDefPtr cdp) const
{
    for(GraphPartitionList::const_iterator i = gpl.begin(); i != gpl.end(); ++i)
    {
	if(find(i->begin(), i->end(), cdp) != i->end())
	{
	    return true;
	}
    }
    return false;
}

void
Slice::Container::addPartition(GraphPartitionList& gpl,
	                       GraphPartitionList::reverse_iterator tail,
			       const ClassDefPtr base) const
{
    //
    // If this base is on one of the partition lists already, do nothing.
    //
    if(isInList(gpl, base))
    {
	return;
    }
    //
    // Put the current base at the end of the current partition.
    //
    tail->push_back(base);
    //
    // If the base has bases in turn, recurse, adding the first base
    // of base (the left-most "grandbase") to the current partition.
    //
    if(base->_bases.size())
    {
	addPartition(gpl, tail, *(base->_bases.begin()));
    }
    //
    // If the base has multiple bases, each of the "grandbases"
    // except for the left-most (which we just dealt with)
    // adds a new partition.
    //
    if(base->_bases.size() > 1)
    {
	ClassList::const_iterator i = base->_bases.begin();
	while(++i != base->_bases.end())
	{
	    ClassList cl;
	    gpl.push_back(cl);
	    addPartition(gpl, gpl.rbegin(), *i);
	}
    }
}

//
// Convert the list of partitions of class definitions into a
// list of sets, with each set containing the operation and member
// names defined by the classes in each partition.
// 
StringSetList
Slice::Container::toStringSetList(const GraphPartitionList& gpl) const
{
    StringSetList spl;
    for(GraphPartitionList::const_iterator i = gpl.begin(); i != gpl.end(); ++i)
    {
	StringSet ss;
	spl.push_back(ss);
	for(ClassList::const_iterator j = i->begin(); j != i->end(); ++j)
	{
	    OperationList operations = (*j)->operations();
	    DataMemberList members = (*j)->dataMembers();
	    for(OperationList::const_iterator l = operations.begin(); l != operations.end(); ++l)
	    {
		spl.rbegin()->insert((*l)->name());
	    }
	    for(DataMemberList::const_iterator m = members.begin(); m != members.end(); ++m)
	    {
		spl.rbegin()->insert((*m)->name());
	    }
	}
    }
    return spl;
}

//
// Return the union of the intersections of all possible pairs of sets of strings.
//
StringList
Slice::Container::unionOfAllPairIntersections(const StringSetList& l) const
{
    StringList result;
    for(StringSetList::const_iterator i = l.begin(); i != l.end(); ++i)
    {
	StringSetList::const_iterator cursor = i;
	++cursor;
	for(StringSetList::const_iterator j = cursor; j != l.end(); ++j)
	{
	    set_intersection(i->begin(), i->end(), j->begin(), j->end(), back_inserter(result));
	}
    }
    result.sort();
    result.unique();
    return result;
}

// ----------------------------------------------------------------------
// Module
// ----------------------------------------------------------------------

Contained::ContainedType
Slice::Module::containedType() const
{
    return ContainedTypeModule;
}

bool
Slice::Module::uses(const ContainedPtr&) const
{
    return false;
}

std::string
Slice::Module::kindOf() const
{
    return "module";
}

void
Slice::Module::visit(ParserVisitor* visitor)
{
    if(_includeLevel > 0)
    {
	return;
    }
   
    if(visitor->visitModuleStart(this))
    {
	Container::visit(visitor);
	visitor->visitModuleEnd(this);
    }
}

Slice::Module::Module(const ContainerPtr& container, const string& name) :
    Contained(container, name),
    Container(container->unit()),
    SyntaxTreeBase(container->unit())
{
}

// ----------------------------------------------------------------------
// Constructed
// ----------------------------------------------------------------------

bool
Slice::Constructed::isLocal() const
{
    return _local;
}

ConstructedList
Slice::Constructed::dependencies()
{
    set<ConstructedPtr> result;
    recDependencies(result);
    return ConstructedList(result.begin(), result.end());
}

Slice::Constructed::Constructed(const ContainerPtr& container, const string& name, bool local) :
    Type(container->unit()),
    Contained(container, name),
    SyntaxTreeBase(container->unit()),
    _local(local)
{
}

// ----------------------------------------------------------------------
// ClassDecl
// ----------------------------------------------------------------------

void
Slice::ClassDecl::destroy()
{
    _definition = 0;
    SyntaxTreeBase::destroy();
}

ClassDefPtr
Slice::ClassDecl::definition()
{
    return _definition;
}

bool
Slice::ClassDecl::isInterface()
{
    return _interface;
}

Contained::ContainedType
Slice::ClassDecl::containedType() const
{
    return ContainedTypeClass;
}

bool
Slice::ClassDecl::uses(const ContainedPtr&) const
{
    return false;
}

std::string
Slice::ClassDecl::kindOf() const
{
    string s = _interface ? "interface" : "class";
    s += " declaration";
    return s;
}

void
Slice::ClassDecl::visit(ParserVisitor* visitor)
{
    visitor->visitClassDecl(this);
}

void
Slice::ClassDecl::recDependencies(set<ConstructedPtr>& dependencies)
{
    if(_definition)
    {
	_definition->containerRecDependencies(dependencies);
	ClassList bases = _definition->bases();
	ClassList::iterator p;
	for(p = bases.begin(); p != bases.end(); ++p)
	{
	    (*p)->declaration()->recDependencies(dependencies);
	}
    }
}

Slice::ClassDecl::ClassDecl(const ContainerPtr& container, const string& name, bool intf, bool local) :
    Constructed(container, name, local),
    Type(container->unit()),
    Contained(container, name),
    SyntaxTreeBase(container->unit()),
    _interface(intf)
{
}

// ----------------------------------------------------------------------
// ClassDef
// ----------------------------------------------------------------------

void
Slice::ClassDef::destroy()
{
    _declaration = 0;
    _bases.empty();
    Container::destroy();
}

OperationPtr
Slice::ClassDef::createOperation(const string& name,
				 const TypePtr& returnType,
				 const TypeStringList& inParams,
				 const TypeStringList& outParams,
				 const ExceptionList& throws)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    if(!matches.empty())
    {
	OperationPtr p = OperationPtr::dynamicCast(matches.front());
	if(p)
	{
	    if(_unit->ignRedefs())
	    {
		return p;
	    }
	}
	string msg;
	if(matches.front()->name() != name)
	{
	    msg = "operation `" + name + "' differs only in capitalization from ";
	    msg += matches.front()->kindOf() + " `" + matches.front()->name() + "'";
	    _unit->warning(msg);	// TODO: change to error in stable_39
	}
	msg = "redefinition of " + matches.front()->kindOf() + " `" + matches.front()->name();
	msg += "' as operation `" + name + "'";
	_unit->error(msg);
	return 0;
    }

    {
	TypeStringList allParams = inParams;
	allParams.insert(allParams.end(), outParams.begin(), outParams.end());
	
	TypeStringList::const_iterator p = allParams.begin();
	while(p != allParams.end())
	{
	    TypeStringList::const_iterator q = p;
	    ++q;
	    while(q != allParams.end())
	    {
		//
		// Complain about duplicate parameters only if they are
		// non-empty; otherwise, we get a complaint about duplicates
		// if two or more parameter names are missing in an operation
		// signature (but the missing parameters have been reported
		// already).
		//
		string plc = p->second;
		to_lower(plc);
		string qlc = q->second;
		to_lower(qlc);
		if(p->second == q->second && p->second != "")
		{
		    string msg = "duplicate parameter `";
		    msg += p->second;
		    msg += "'";
		    _unit->error(msg);
		    return 0;
		}
		if(plc == qlc && plc != "")
		{
		    string msg = "parameter `" + p->second;
		    msg += "' differs only in capitalization from parameter `" + q->second + "'";
		    _unit->warning(msg);	// TODO: change to error in stable_39
		}

		++q;
	    }
	    ++p;
	}
    }

    //
    // Check whether enclosing interface/class has the same name.
    //
    if(name == this->name())
    {
	string msg = isInterface() ? "interface" : "class";
	msg += "name `" + name + "' cannot be used as operation name";
	_unit->error(msg);
	return 0;
    }
    string newName = name;
    to_lower(newName);
    string thisName = this->name();
    to_lower(thisName);
    if(newName == thisName)
    {
	string msg = "operation `" + name + "' differs only in capitalization from enclosing ";
	msg += isInterface() ? "interface" : "class";
	msg += " name `" + this->name() + "'";
	_unit->warning(msg);	// TODO: change to error in stable_39
    }

    //
    // Check whether any bases have defined something with the same name already
    //
    for(ClassList::const_iterator p = _bases.begin(); p != _bases.end(); ++p)
    {
	ContainedList cl;
	OperationList ol = (*p)->allOperations();
	copy(ol.begin(), ol.end(), back_inserter(cl));
	DataMemberList dml = (*p)->allDataMembers();
	copy(dml.begin(), dml.end(), back_inserter(cl));
	// TODO: once we have constants, append constants to this list
	// for checking. 
	for(ContainedList::const_iterator q = cl.begin(); q != cl.end(); ++q)
	{
	    if((*q)->name() == name)
	    {
		string msg = "operation `" + name;
		msg += "' is already defined as ";
		string vowels = "aeiou";
		msg += find(vowels.begin(), vowels.end(), *((*q)->kindOf().begin())) != vowels.end() ? "an " : "a ";
		msg += (*q)->kindOf() + " in a base interface or class";
		_unit->error(msg);
		return 0;
	    }
	    string baseName = (*q)->name();
	    to_lower(baseName);
	    string newName = name;
	    to_lower(newName);
	    if(baseName == newName)
	    {
		string msg = "operation `" + name + "' differs only in capitalization from " + (*q)->kindOf();
		msg += " `" + (*q)->name() + "', which is defined in a base interface or class";
		_unit->warning(msg);	// TODO: change to error in stable_39
	    }
	}
    }

    OperationPtr op = new Operation(this, name, returnType, inParams, outParams, throws);
    _contents.push_back(op);
    return op;
}

DataMemberPtr
Slice::ClassDef::createDataMember(const string& name, const TypePtr& type)
{
    assert(!isInterface()); 
    ContainedList matches = _unit->findContents(thisScope() + name);
    if(!matches.empty())
    {
	DataMemberPtr p = DataMemberPtr::dynamicCast(matches.front());
	if(p)
	{
	    if(_unit->ignRedefs())
	    {
		return p;
	    }
	}
	string msg;
	if(matches.front()->name() != name)
	{
	    msg = "data member `" + name + "' differs only in capitalization from ";
	    msg += matches.front()->kindOf() + " `" + matches.front()->name() + "'";
	    _unit->warning(msg);	// TODO: change to error in stable_39
	}
	else
	{
	    msg = "redefinition of " + matches.front()->kindOf() + " `" + matches.front()->name();
	    msg += "' as data member `" + name + "'";
	    _unit->error(msg);
	    return 0;
	}
    }

    //
    // Check whether enclosing class has the same name
    //
    if(name == this->name())
    {
	string msg = "class name `";
	msg += name;
	msg += "' cannot be used as data member name";
	_unit->error(msg);
	return 0;
    }
    string newName = name;
    to_lower(newName);
    string thisName = this->name();
    to_lower(thisName);
    if(newName == thisName)
    {
	string msg = "data member `" + name + "' differs only in capitalization from enclosing class name `";
	msg += this->name() + "'";
	_unit->warning(msg);	// TODO: change to error in stable_39
    }

    //
    // Check whether any bases have defined something with the same name already
    //
    for(ClassList::const_iterator p = _bases.begin(); p != _bases.end(); ++p)
    {
	ContainedList cl;
	OperationList ol = (*p)->allOperations();
	copy(ol.begin(), ol.end(), back_inserter(cl));
	DataMemberList dml = (*p)->allDataMembers();
	copy(dml.begin(), dml.end(), back_inserter(cl));
	// TODO: once we have constants, append constants to this list for checking.
	for(ContainedList::const_iterator q = cl.begin(); q != cl.end(); ++q)
	{
	    if((*q)->name() == name)
	    {
		string msg = "data member `" + name;
		msg += "' is already defined as ";
		string vowels = "aeiou"; // TODO: ML: Could be static const.
		msg += find(vowels.begin(), vowels.end(), *((*q)->kindOf().begin())) != vowels.end() ? "an " : "a ";
		msg += (*q)->kindOf() + " in a base interface or class";
		_unit->error(msg);
		return 0;
	    }
	    string baseName = (*q)->name();
	    to_lower(baseName);
	    string newName = name;
	    to_lower(newName);
	    if(baseName == newName)
	    {
		string msg = "data member `" + name + "' differs only in capitalization from " + (*q)->kindOf();
		msg += " `" + (*q)->name() + "', which is defined in a base interface or class";
		_unit->warning(msg);	// TODO: change to error in stable_39
	    }
	}
    }

    _hasDataMembers = true;
    DataMemberPtr member = new DataMember(this, name, type);
    _contents.push_back(member);
    return member;
}

ClassDeclPtr
Slice::ClassDef::declaration() const
{
    return _declaration;
}

ClassList
Slice::ClassDef::bases() const
{
    return _bases;
}

ClassList
Slice::ClassDef::allBases() const
{
    ClassList result = _bases;
    result.sort();
    result.unique();
    for(ClassList::const_iterator p = _bases.begin(); p != _bases.end(); ++p)
    {
	ClassList li = (*p)->allBases();
	result.merge(li);
	result.unique();
    }
    return result;
}

OperationList
Slice::ClassDef::operations() const
{
    OperationList result;
    for(ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	OperationPtr q = OperationPtr::dynamicCast(*p);
	if(q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

OperationList
Slice::ClassDef::allOperations() const
{
    OperationList result = operations();
    result.sort();
    result.unique();
    for(ClassList::const_iterator p = _bases.begin(); p != _bases.end(); ++p)
    {
	OperationList li = (*p)->allOperations();
	result.merge(li);
	result.unique();
    }
    return result;
}

DataMemberList
Slice::ClassDef::dataMembers() const
{
    DataMemberList result;
    for(ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	DataMemberPtr q = DataMemberPtr::dynamicCast(*p);
	if(q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

DataMemberList
Slice::ClassDef::allDataMembers() const
{
    DataMemberList result = dataMembers();
    result.sort();
    result.unique();
    for(ClassList::const_iterator p = _bases.begin(); p != _bases.end(); ++p)
    {
	DataMemberList li = (*p)->allDataMembers();
	result.merge(li);
	result.unique();
    }
    return result;
}

bool
Slice::ClassDef::isAbstract()
{
    if(isInterface() || _bases.size() > 1) // Is this an interface, or does it derive from interfaces?
    {
	return true;
    }

    if(!_bases.empty() && _bases.front()->isAbstract())
    {
	return true;
    }

    for(ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	if(OperationPtr::dynamicCast(*p))
	{
	    return true;
	}
    }

    return false;
}

bool
Slice::ClassDef::isInterface()
{
    return _interface;
}

bool
Slice::ClassDef::isLocal() const
{
    return _local;
}

bool
Slice::ClassDef::hasDataMembers() const
{
    return _hasDataMembers;
}

Contained::ContainedType
Slice::ClassDef::containedType() const
{
    return ContainedTypeClass;
}

bool
Slice::ClassDef::uses(const ContainedPtr&) const
{
    // No uses() implementation here. DataMember and Operation have
    // their own uses().
    return false;
}

std::string
Slice::ClassDef::kindOf() const
{
    string s;
    if(isLocal()) // TODO: ML: { } also for one-line statements.
	s += "local ";
    s += "isInterface()" ? "interface" : "class"; // TODO: ML: Guess the "" around isInterface() are a mistake? :-)
    return s;
}

void
Slice::ClassDef::visit(ParserVisitor* visitor)
{
    if(_includeLevel > 0)
    {
	return;
    }

    if(visitor->visitClassDefStart(this))
    {
	Container::visit(visitor);
	visitor->visitClassDefEnd(this);
    }
}

Slice::ClassDef::ClassDef(const ContainerPtr& container, const string& name, bool intf, const ClassList& bases,
			  bool local) :
    Contained(container, name),
    Container(container->unit()),
    SyntaxTreeBase(container->unit()),
    _interface(intf),
    _hasDataMembers(false),
    _bases(bases),
    _local(local)
{
    //
    // First element of bases may be a class, all others must be
    // interfaces
    //
#ifndef NDEBUG
    for(ClassList::const_iterator p = _bases.begin(); p != _bases.end(); ++p)
    {
	assert(p == _bases.begin() || (*p)->isInterface());
    }
#endif
}

// ----------------------------------------------------------------------
// Proxy
// ----------------------------------------------------------------------

ClassDeclPtr
Slice::Proxy::_class()
{
    return __class;
}

Slice::Proxy::Proxy(const ClassDeclPtr& cl) :
    Type(cl->unit()),
    SyntaxTreeBase(cl->unit()),
    __class(cl)
{
}

// ----------------------------------------------------------------------
// Exception
// ----------------------------------------------------------------------

void
Slice::Exception::destroy()
{
    _base = 0;
    Container::destroy();
}

DataMemberPtr
Slice::Exception::createDataMember(const string& name, const TypePtr& type)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    if(!matches.empty())
    {
	DataMemberPtr p = DataMemberPtr::dynamicCast(matches.front());
	if(p)
	{
	    if(_unit->ignRedefs())
	    {
		return p;
	    }
	}
	string msg;
	if(matches.front()->name() != name)
	{
	    msg = "exception member `" + name + "' differs only in capitalization from ";
	    msg += "exception member `" + matches.front()->name() + "'";
	    _unit->warning(msg);	// TODO: change to error in stable_39
	}
	else
	{
	    msg = "redefinition of exception member `" + name + "'";
	    _unit->error(msg);
	    return 0;
	}
    }

    //
    // Check whether enclosing exception has the same name
    //
    if(name == this->name())
    {
	string msg = "exception name `";
	msg += name;
	msg += "' cannot be used as exception member name";
	_unit->error(msg);
	return 0;
    }
    string newName = name;
    to_lower(newName);
    string thisName = this->name();
    to_lower(thisName);
    if(newName == thisName)
    {
	string msg = "exception member `" + name + "' differs only in capitalization from enclosing exception name `";
	msg += this->name() + "'";
	_unit->warning(msg);	// TODO: change to error in stable_39
    }

    //
    // Check whether any bases have defined a member with the same name already
    //
    ExceptionList bl = allBases();
    // TODO: ML: Always check your code also with Visual C++. The line
    // below gives a "redefinition of p" error with Visual C++,
    // because it uses the old scoping rules.
    //for(ExceptionList::const_iterator p = bl.begin(); p != bl.end(); ++p)
    for(ExceptionList::const_iterator q = bl.begin(); q != bl.end(); ++q)
    {
	ContainedList cl;
	DataMemberList dml = (*q)->dataMembers();
	copy(dml.begin(), dml.end(), back_inserter(cl));
	for(ContainedList::const_iterator r = cl.begin(); r != cl.end(); ++r)
	{
	    if((*r)->name() == name)
	    {
		string msg = "exception member `" + name + "' is already defined in a base exception";
		_unit->error(msg);
		return 0;
	    }
	    string baseName = (*r)->name();
	    to_lower(baseName);
	    string newName = name;
	    to_lower(newName);
	    if(baseName == newName)
	    {
		string msg = "exception member `" + name + "' differs only in capitalization from exception member `";
		msg += (*r)->name() + "', which is defined in a base exception";
		_unit->warning(msg);	// TODO: change to error in stable_39
	    }
	}
    }

    DataMemberPtr p = new DataMember(this, name, type);
    _contents.push_back(p);
    return p;
}

DataMemberList
Slice::Exception::dataMembers() const
{
    DataMemberList result;
    for(ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	DataMemberPtr q = DataMemberPtr::dynamicCast(*p);
	if(q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

ExceptionPtr
Slice::Exception::base() const
{
    return _base;
}

ExceptionList
Slice::Exception::allBases() const
{
    ExceptionList result;
    if(_base)
    {
	result = _base->allBases();
	result.push_front(_base);
    }
    return result;
}

bool
Slice::Exception::isLocal() const
{
    return _local;
}

Contained::ContainedType
Slice::Exception::containedType() const
{
    return ContainedTypeException;
}

bool
Slice::Exception::uses(const ContainedPtr&) const
{
    // No uses() implementation here. DataMember has its own uses().
    return false;
}

std::string
Slice::Exception::kindOf() const
{
    return "exception";
}

void
Slice::Exception::visit(ParserVisitor* visitor)
{
    if(_includeLevel > 0)
    {
	return;
    }

    if(visitor->visitExceptionStart(this))
    {
	Container::visit(visitor);
	visitor->visitExceptionEnd(this);
    }
}

Slice::Exception::Exception(const ContainerPtr& container, const string& name, const ExceptionPtr& base, bool local) :
    Container(container->unit()),
    Contained(container, name),
    SyntaxTreeBase(container->unit()),
    _base(base),
    _local(local)
{
}

// ----------------------------------------------------------------------
// Struct
// ----------------------------------------------------------------------

DataMemberPtr
Slice::Struct::createDataMember(const string& name, const TypePtr& type)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    if(!matches.empty())
    {
	DataMemberPtr p = DataMemberPtr::dynamicCast(matches.front());
	if(p)
	{
	    if(_unit->ignRedefs())
	    {
		return p;
	    }
	}
	string msg;
	if(matches.front()->name() != name)
	{
	    msg = "member `" + name + "' differs only in capitalization from ";
	    msg += "member `" + matches.front()->name() + "'";
	    _unit->warning(msg);	// TODO: change to error in stable_39
	}
	else
	{
	    msg = "redefinition of struct member `" + name + "'";
	    _unit->error(msg);
	    return 0;
	}
    }

    //
    // Check whether enclosing struct has the same name
    //
    if(name == this->name())
    {
	string msg = "struct name `";
	msg += name;
	msg += "' cannot be used as member name";
	_unit->error(msg);
	return 0;
    }
    string newName = name;
    to_lower(newName);
    string thisName = this->name();
    to_lower(thisName);
    if(newName == thisName)
    {
	string msg = "struct member `" + name + "' differs only in capitalization from enclosing struct name `";
	msg += this->name() + "'";
	_unit->warning(msg);	// TODO: change to error in stable_39
    }

    if(type.get() == this)
    {
	string msg = "struct `";
	msg += name;
	msg += "' cannot contain itself";
	_unit->error(msg);
	return 0;
    }

    DataMemberPtr p = new DataMember(this, name, type);
    _contents.push_back(p);
    return p;
}

DataMemberList
Slice::Struct::dataMembers()
{
    DataMemberList result;
    for(ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	DataMemberPtr q = DataMemberPtr::dynamicCast(*p);
	if(q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

Contained::ContainedType
Slice::Struct::containedType() const
{
    return ContainedTypeStruct;
}

bool
Slice::Struct::uses(const ContainedPtr&) const
{
    return false;
}

std::string
Slice::Struct::kindOf() const
{
    return "struct";
}

void
Slice::Struct::visit(ParserVisitor* visitor)
{
    if(_includeLevel > 0)
    {
	return;
    }

    if(visitor->visitStructStart(this))
    {
	Container::visit(visitor);
	visitor->visitStructEnd(this);
    }
}

void
Slice::Struct::recDependencies(set<ConstructedPtr>& dependencies)
{
    containerRecDependencies(dependencies);
}

Slice::Struct::Struct(const ContainerPtr& container, const string& name, bool local) :
    Container(container->unit()),
    Constructed(container, name, local),
    Type(container->unit()),
    Contained(container, name),
    SyntaxTreeBase(container->unit())
{
}

// ----------------------------------------------------------------------
// Sequence
// ----------------------------------------------------------------------

TypePtr
Slice::Sequence::type()
{
    return _type;
}

Contained::ContainedType
Slice::Sequence::containedType() const
{
    return ContainedTypeSequence;
}

bool
Slice::Sequence::uses(const ContainedPtr& contained) const
{
    ContainedPtr contained2 = ContainedPtr::dynamicCast(_type);
    if(contained2 && contained2 == contained)
    {
	return true;
    }

    return false;
}

std::string
Slice::Sequence::kindOf() const
{
    return "sequence";
}

void
Slice::Sequence::visit(ParserVisitor* visitor)
{
    visitor->visitSequence(this);
}

void
Slice::Sequence::recDependencies(set<ConstructedPtr>& dependencies)
{
    ConstructedPtr constructed = ConstructedPtr::dynamicCast(_type);
    if(constructed && dependencies.find(constructed) != dependencies.end())
    {
	dependencies.insert(constructed);
	constructed->recDependencies(dependencies);
    }
}

Slice::Sequence::Sequence(const ContainerPtr& container, const string& name, const TypePtr& type, bool local) :
    Constructed(container, name, local),
    Type(container->unit()),
    Contained(container, name),
    SyntaxTreeBase(container->unit()),
    _type(type)
{
}

// ----------------------------------------------------------------------
// Dictionary
// ----------------------------------------------------------------------

TypePtr
Slice::Dictionary::keyType()
{
    return _keyType;
}

TypePtr
Slice::Dictionary::valueType()
{
    return _valueType;
}

Contained::ContainedType
Slice::Dictionary::containedType() const
{
    return ContainedTypeDictionary;
}

bool
Slice::Dictionary::uses(const ContainedPtr& contained) const
{
    {
	ContainedPtr contained2 = ContainedPtr::dynamicCast(_keyType);
	if(contained2 && contained2 == contained)
	{
	    return true;
	}
    }

    {
	ContainedPtr contained2 = ContainedPtr::dynamicCast(_valueType);
	if(contained2 && contained2 == contained)
	{
	    return true;
	}
    }

    return false;
}

std::string
Slice::Dictionary::kindOf() const
{
    return "dictionary";
}

void
Slice::Dictionary::visit(ParserVisitor* visitor)
{
    visitor->visitDictionary(this);
}

void
Slice::Dictionary::recDependencies(set<ConstructedPtr>& dependencies)
{
    {
	ConstructedPtr constructed = ConstructedPtr::dynamicCast(_keyType);
	if(constructed && dependencies.find(constructed) != dependencies.end())
	{
	    dependencies.insert(constructed);
	    constructed->recDependencies(dependencies);
	}
    }

    {
	ConstructedPtr constructed = ConstructedPtr::dynamicCast(_valueType);
	if(constructed && dependencies.find(constructed) != dependencies.end())
	{
	    dependencies.insert(constructed);
	    constructed->recDependencies(dependencies);
	}
    }
}

Slice::Dictionary::Dictionary(const ContainerPtr& container, const string& name, const TypePtr& keyType,
			      const TypePtr& valueType, bool local) :
    Constructed(container, name, local),
    Type(container->unit()),
    Contained(container, name),
    SyntaxTreeBase(container->unit()),
    _keyType(keyType),
    _valueType(valueType)
{
}

// ----------------------------------------------------------------------
// Enum
// ----------------------------------------------------------------------

EnumeratorList
Slice::Enum::getEnumerators()
{
    return _enumerators;
}

void
Slice::Enum::setEnumerators(const EnumeratorList& ens)
{
    _enumerators = ens;
}

Contained::ContainedType
Slice::Enum::containedType() const
{
    return ContainedTypeEnum;
}

bool
Slice::Enum::uses(const ContainedPtr&) const
{
    return false;
}

std::string
Slice::Enum::kindOf() const
{
    return "enumeration";
}

void
Slice::Enum::visit(ParserVisitor* visitor)
{
    visitor->visitEnum(this);
}

void
Slice::Enum::recDependencies(set<ConstructedPtr>&)
{
    // An Enum does not have any dependencies.
}

Slice::Enum::Enum(const ContainerPtr& container, const string& name, bool local) :
    Constructed(container, name, local),
    Type(container->unit()),
    Contained(container, name),
    SyntaxTreeBase(container->unit())
{
}

// ----------------------------------------------------------------------
// Enumerator
// ----------------------------------------------------------------------

Contained::ContainedType
Slice::Enumerator::containedType() const
{
    return ContainedTypeEnumerator;
}

bool
Slice::Enumerator::uses(const ContainedPtr&) const
{
    return false;
}

std::string
Slice::Enumerator::kindOf() const
{
    return "enumerator";
}

Slice::Enumerator::Enumerator(const ContainerPtr& container, const string& name) :
    Contained(container, name),
    SyntaxTreeBase(container->unit())
{
}

// ----------------------------------------------------------------------
// Operation
// ----------------------------------------------------------------------

TypePtr
Slice::Operation::returnType() const
{
    return _returnType;
}

TypeStringList
Slice::Operation::inputParameters() const
{
    return _inParams;
}

TypeStringList
Slice::Operation::outputParameters() const
{
    return _outParams;
}

ExceptionList
Slice::Operation::throws() const
{
    return _throws;
}

Contained::ContainedType
Slice::Operation::containedType() const
{
    return ContainedTypeOperation;
}

bool
Slice::Operation::uses(const ContainedPtr& contained) const
{
    {
	ContainedPtr contained2 = ContainedPtr::dynamicCast(_returnType);
	if(contained2 && contained2 == contained)
	{
	    return true;
	}
    }

    TypeStringList::const_iterator p;

    for(p = _inParams.begin(); p != _inParams.end(); ++p)
    {
	ContainedPtr contained2 = ContainedPtr::dynamicCast(p->first);
	if(contained2 && contained2 == contained)
	{
	    return true;
	}
    }

    for(p = _outParams.begin(); p != _outParams.end(); ++p)
    {
	ContainedPtr contained2 = ContainedPtr::dynamicCast(p->first);
	if(contained2 && contained2 == contained)
	{
	    return true;
	}
    }

    ExceptionList::const_iterator q;

    for(q = _throws.begin(); q != _throws.end(); ++q)
    {
	ContainedPtr contained2 = ContainedPtr::dynamicCast(*q);
	if(contained2 && contained2 == contained)
	{
	    return true;
	}
    }

    return false;
}

std::string
Slice::Operation::kindOf() const
{
    return "operation";
}

void
Slice::Operation::visit(ParserVisitor* visitor)
{
    visitor->visitOperation(this);
}

Slice::Operation::Operation(const ContainerPtr& container, const string& name, const TypePtr& returnType,
			    const TypeStringList& inParams, const TypeStringList& outParams,
			    const ExceptionList& throws) :
    Contained(container, name),
    SyntaxTreeBase(container->unit()),
    _returnType(returnType),
    _inParams(inParams),
    _outParams(outParams),
    _throws(throws)
{
}

// ----------------------------------------------------------------------
// DataMember
// ----------------------------------------------------------------------

TypePtr
Slice::DataMember::type()
{
    return _type;
}

Contained::ContainedType
Slice::DataMember::containedType() const
{
    return ContainedTypeDataMember;
}

bool
Slice::DataMember::uses(const ContainedPtr& contained) const
{
    ContainedPtr contained2 = ContainedPtr::dynamicCast(_type);
    if(contained2 && contained2 == contained)
    {
	return true;
    }

    return false;
}

std::string
Slice::DataMember::kindOf() const
{
    return "data member";
}

void
Slice::DataMember::visit(ParserVisitor* visitor)
{
    visitor->visitDataMember(this);
}

Slice::DataMember::DataMember(const ContainerPtr& container, const string& name, const TypePtr& type) :
    Contained(container, name),
    SyntaxTreeBase(container->unit()),
    _type(type)
{
}

// ----------------------------------------------------------------------
// Unit
// ----------------------------------------------------------------------

UnitPtr
Slice::Unit::createUnit(bool ignRedefs, bool all)
{
    return new Unit(ignRedefs, all);
}

bool
Slice::Unit::ignRedefs() const
{
    return _ignRedefs;
}

void
Slice::Unit::setComment(const string& comment)
{
    _currentComment = "";

    string::size_type end = 0;
    while(true)
    {
	string::size_type begin = comment.find_first_not_of(" \t\r\n*", end);
	if(begin == string::npos)
	{
	    break;
	}

	end = comment.find('\n', begin);
	if(end != string::npos)
	{
	    if(end + 1 > begin)
	    {
		_currentComment += comment.substr(begin, end + 1 - begin);
	    }
	}
	else
	{
	    end = comment.find_last_not_of(" \t\r\n*");
	    if(end != string::npos)
	    {
		if(end + 1 > begin)
		{
		    _currentComment += comment.substr(begin, end + 1 - begin);
		}
	    }
	    break;
	}
    }
}

string
Slice::Unit::currentComment()
{
    string comment;
    comment.swap(_currentComment);
    return comment;
}

string
Slice::Unit::currentFile() const
{
    return _currentFile;
}

int
Slice::Unit::currentLine() const
{
    return _currentLine;
}

void
Slice::Unit::nextLine()
{
    _currentLine++;
}

void
Slice::Unit::scanPosition(const char* s)
{
    string line(s);
    string::size_type idx;

    idx = line.find("line");
    if(idx != string::npos)
    {
	line.erase(0, idx + 4);
    }

    idx = line.find_first_not_of(" \t\r#");
    if(idx != string::npos)
    {
	line.erase(0, idx);
    }

    _currentLine = atoi(line.c_str()) - 1;

    idx = line.find_first_of(" \t\r");
    if(idx != string::npos)
    {
	line.erase(0, idx);
    }

    idx = line.find_first_not_of(" \t\r\"");
    if(idx != string::npos)
    {
	line.erase(0, idx);

	idx = line.find_first_of(" \t\r\"");
	if(idx != string::npos)
	{
	    _currentFile = line.substr(0, idx);
	    line.erase(0, idx + 1);
	}
	else
	{
	    _currentFile = line;
	}

	idx = line.find_first_not_of(" \t\r");
	if(idx != string::npos)
	{
	    line.erase(0, idx);
	    int val = atoi(line.c_str());
	    if(val == 1)
	    {
		if(++_currentIncludeLevel == 1)
		{
		    if(find(_includeFiles.begin(), _includeFiles.end(), _currentFile) == _includeFiles.end())
		    {
			_includeFiles.push_back(_currentFile);
		    }
		}
	    }
	    else if(val == 2)
	    {
		--_currentIncludeLevel;
	    }
	    _currentComment.erase();
	}
	else
	{
	    if(_currentIncludeLevel == 0)
	    {
		_topLevelFile = _currentFile;
	    }
	}
    }
}

int
Slice::Unit::currentIncludeLevel() const
{
    if(_all)
    {
	return 0;
    }
    else
    {
	return _currentIncludeLevel;
    }
}

void
Slice::Unit::error(const char* s)
{
    cerr << _currentFile << ':' << _currentLine << ": " << s << endl;
    _errors++;
}

void
Slice::Unit::error(const string& s)
{
    error(s.c_str());
}

void
Slice::Unit::warning(const char* s)
{
    cerr << _currentFile << ':' << _currentLine << ": warning: " << s << endl;
}

void
Slice::Unit::warning(const string& s)
{
    warning(s.c_str());
}

ContainerPtr
Slice::Unit::currentContainer() const
{
    assert(!_containerStack.empty());
    return _containerStack.top();
}

void
Slice::Unit::pushContainer(const ContainerPtr& cont)
{
    _containerStack.push(cont);
}

void
Slice::Unit::popContainer()
{
    assert(!_containerStack.empty());
    _containerStack.pop();
}

void
Slice::Unit::addContent(const ContainedPtr& contained)
{
    string scopedLowerCase = contained->scoped();
    to_lower(scopedLowerCase);
    _contentMap[scopedLowerCase].push_back(contained);
}

void
Slice::Unit::removeContent(const ContainedPtr& contained)
{
    string scopedLowerCase = contained->scoped();
    to_lower(scopedLowerCase);
    map<string, ContainedList>::iterator p = _contentMap.find(scopedLowerCase);
    assert(p != _contentMap.end());
    ContainedList::iterator q;
    for(q = p->second.begin(); q != p->second.end(); ++q)
    {
	if(q->get() == contained.get())
	{
	    p->second.erase(q);
	    return;
	}
    }
    assert(false);
}

ContainedList
Slice::Unit::findContents(const string& scoped) const
{
    assert(!scoped.empty());
    assert(scoped[0] == ':');

    string scopedLowerCase = scoped;
    to_lower(scopedLowerCase);

    map<string, ContainedList>::const_iterator p = _contentMap.find(scopedLowerCase);
    if(p != _contentMap.end())
    {
	return p->second;
    }
    else
    {
	return ContainedList();
    }
}

ClassList
Slice::Unit::findDerivedClasses(const ClassDefPtr& cl) const
{
    ClassList derived;
    for(map<string, ContainedList>::const_iterator p = _contentMap.begin(); p != _contentMap.end(); ++p)
    {
	for(ContainedList::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
	{
	    ClassDefPtr r = ClassDefPtr::dynamicCast(*q);
	    if(r)
	    {
		ClassList bases = r->bases();
		if(find(bases.begin(), bases.end(), cl) != bases.end())
		{
		    derived.push_back(r);
		}
	    }
	}
    }
    derived.sort();
    derived.unique();
    return derived;
}

ExceptionList
Slice::Unit::findDerivedExceptions(const ExceptionPtr& ex) const
{
    ExceptionList derived;
    for(map<string, ContainedList>::const_iterator p = _contentMap.begin(); p != _contentMap.end(); ++p)
    {
	for(ContainedList::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
	{
	    ExceptionPtr r = ExceptionPtr::dynamicCast(*q);
	    if(r)
	    {
		ExceptionPtr base = r->base();
		if(base && base == ex)
		{
		    derived.push_back(r);
		}
	    }
	}
    }
    derived.sort();
    derived.unique();
    return derived;
}

ContainedList
Slice::Unit::findUsedBy(const ContainedPtr& contained) const
{
    ContainedList usedBy;
    for(map<string, ContainedList>::const_iterator p = _contentMap.begin(); p != _contentMap.end(); ++p)
    {
	for(ContainedList::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
	{
	    if((*q)->uses(contained))
	    {
		usedBy.push_back(*q);
	    }
	}
    }
    usedBy.sort();
    usedBy.unique();
    return usedBy;
}

bool
Slice::Unit::usesProxies() const
{
    for(map<string, ContainedList>::const_iterator p = _contentMap.begin(); p != _contentMap.end(); ++p)
    {
	for(ContainedList::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
	{
	    ClassDeclPtr decl = ClassDeclPtr::dynamicCast(*q);
	    if(decl && !decl->isLocal())
	    {
		return true;
	    }
	}
    }

    if(_builtins.find(Builtin::KindObjectProxy) != _builtins.end())
    {
	return true;
    }

    return false;
}

bool
Slice::Unit::usesNonLocals() const
{
    for(map<string, ContainedList>::const_iterator p = _contentMap.begin(); p != _contentMap.end(); ++p)
    {
	for(ContainedList::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
	{
	    ConstructedPtr constr = ConstructedPtr::dynamicCast(*q);
	    if(constr && !constr->isLocal())
	    {
		return true;
	    }

	    ExceptionPtr exc = ExceptionPtr::dynamicCast(*q);
	    if(exc && !exc->isLocal())
	    {
		return true;
	    }
	}
    }

    if(_builtins.find(Builtin::KindObject) != _builtins.end())
    {
	return true;
    }

    if(_builtins.find(Builtin::KindObjectProxy) != _builtins.end())
    {
	return true;
    }

    return false;
}

StringList
Slice::Unit::includeFiles() const
{
    return _includeFiles;
}

int
Slice::Unit::parse(FILE* file, bool debug)
{
    yydebug = debug ? 1 : 0;

    assert(!Slice::unit);
    Slice::unit = this;

    _currentComment = "";
    _currentLine = 1;
    _currentIncludeLevel = 0;
    _currentFile = "";
    _topLevelFile = "";
    pushContainer(this);

    yyin = file;
    int status = yyparse();
    if(_errors)
    {
	status = EXIT_FAILURE;
    }

    if(status == EXIT_FAILURE)
    {
	while(!_containerStack.empty())
	{
	    popContainer();
	}
    }
    else
    {
	assert(_containerStack.size() == 1);
	popContainer();
    }

    Slice::unit = 0;
    return status;
}

void
Slice::Unit::destroy()
{
    _contentMap.clear();
    _builtins.clear();
    Container::destroy();
}

void
Slice::Unit::visit(ParserVisitor* visitor)
{
    if(visitor->visitUnitStart(this))
    {
	Container::visit(visitor);
	visitor->visitUnitEnd(this);
    }
}

BuiltinPtr
Slice::Unit::builtin(Builtin::Kind kind)
{
    map<Builtin::Kind, BuiltinPtr>::const_iterator p = _builtins.find(kind);
    if(p != _builtins.end())
    {
	return p->second;
    }
    BuiltinPtr builtin = new Builtin(this, kind);
    _builtins.insert(make_pair(kind, builtin));
    return builtin;
}

Slice::Unit::Unit(bool ignRedefs, bool all) :
    SyntaxTreeBase(0),
    Container(0),
    _ignRedefs(ignRedefs),
    _all(all),
    _errors(0)
{
    _unit = this;
}
