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
// SyntaxTreeBase
// ----------------------------------------------------------------------

void
Slice::SyntaxTreeBase::destroy()
{
    _unit = 0;
}

UnitPtr
Slice::SyntaxTreeBase::unit()
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
Slice::Contained::container()
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
Slice::Contained::scope()
{
    string::size_type idx = _scoped.rfind("::");
    assert(idx != string::npos);
    return string(_scoped, 0, idx + 2);
}

string
Slice::Contained::file()
{
    return _file;
}

string
Slice::Contained::comment()
{
    return _comment;
}

list<string>
Slice::Contained::getMetaData()
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
    if (cont)
    {
	_scoped = cont->scoped();
    }
    _scoped += "::" + _name;				       
    assert(_unit);
    _unit->addContent(this);
    _file = _unit->currentFile();
    _comment = _unit->currentComment();
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
    for (ContainedList::const_iterator p = matches.begin(); p != matches.end(); ++p)
    {
	ModulePtr module = ModulePtr::dynamicCast(*p);
	if (module)
	{
	    continue; // Reopening modules is permissible
	}
	
	string msg = "redefinition of `";
	msg += name;
	msg += "' as module";
	_unit->error(msg);
	return 0;
    }

    ModulePtr q = new Module(this, name);
    _contents.push_back(q);
    return q;
}

ClassDefPtr
Slice::Container::createClassDef(const string& name, bool intf, const ClassList& bases, bool local)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    for (ContainedList::const_iterator p = matches.begin(); p != matches.end(); ++p)
    {
	ClassDeclPtr decl = ClassDeclPtr::dynamicCast(*p);
	if (decl)
	{
	    if (checkInterfaceAndLocal(name, false, intf, decl->isInterface(), local, decl->isLocal()))
	    {
		continue;
	    }
	    
	    return 0;
	}

	ClassDefPtr def = ClassDefPtr::dynamicCast(*p);
	if (def)
	{
	    if (_unit->ignRedefs())
	    {
		def->updateIncludeLevel();
		return def;
	    }

	    string msg = "redefinition of ";
	    if (intf)
	    {
		msg += "interface";
	    }
	    else
	    {
		msg += "class";
	    }
	    msg += " `";
	    msg += name;
	    msg += "'";
	    _unit->error(msg);
	    return 0;
	}
	
	string msg = "redefinition of `";
	msg += name;
	msg += "' as ";
	if (intf)
	{
	    msg += "interface";
	}
	else
	{
	    msg += "class";
	}
	_unit->error(msg);
	return 0;
    }
    
    ClassDefPtr def = new ClassDef(this, name, intf, bases, local);
    _contents.push_back(def);
    
    for (ContainedList::const_iterator q = matches.begin(); q != matches.end(); ++q)
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
    for (ContainedList::const_iterator p = matches.begin(); p != matches.end(); ++p)
    {
	ClassDefPtr clDef = ClassDefPtr::dynamicCast(*p);
	if (clDef)
	{
	    if (checkInterfaceAndLocal(name, true, intf, clDef->isInterface(), local, clDef->isLocal()))
	    {
		assert(!def);
		def = clDef;
		continue;
	    }

	    return 0;
	}
	
	ClassDeclPtr clDecl = ClassDeclPtr::dynamicCast(*p);
	if (clDecl)
	{
	    if (checkInterfaceAndLocal(name, false, intf, clDecl->isInterface(), local, clDecl->isLocal()))
	    {
		continue;
	    }
	    
	    return 0;
	}
	
	string msg = "declaration of already defined `";
	msg += name;
	msg += "' as ";
	if (intf)
	{
	    msg += "interface";
	}
	else
	{
	    msg += "class";
	}
	_unit->error(msg);
	return 0;
    }

    //
    // Multiple declarations are permissible. But if we do already
    // have a declaration for the class in this container, we don't
    // create another one.
    //
    for (ContainedList::const_iterator q = _contents.begin(); q != _contents.end(); ++q)
    {
	if ((*q)->name() == name)
	{
	    ClassDeclPtr decl = ClassDeclPtr::dynamicCast(*q);
	    if (decl)
	    {
		return decl;
	    }

	    assert(ClassDefPtr::dynamicCast(*q));
	}
    }

    ClassDeclPtr decl = new ClassDecl(this, name, intf, local);
    _contents.push_back(decl);

    if (def)
    {
	decl->_definition = def;
    }

    return decl;
}

ExceptionPtr
Slice::Container::createException(const string& name, const ExceptionPtr& base, bool local)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    if (!matches.empty())
    {
	ExceptionPtr p = ExceptionPtr::dynamicCast(matches.front());
	if (p)
	{
	    if (_unit->ignRedefs())
	    {
		p->updateIncludeLevel();
		return p;
	    }

	    string msg = "redefinition of exception `";
	    msg += name;
	    msg += "'";
	    _unit->error(msg);
	    return 0;
	}
	
	string msg = "redefinition of `";
	msg += name;
	msg += "' as exception";
	_unit->error(msg);
	return 0;
    }

    ExceptionPtr p = new Exception(this, name, base, local);
    _contents.push_back(p);
    return p;
}

StructPtr
Slice::Container::createStruct(const string& name, bool local)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    if (!matches.empty())
    {
	StructPtr p = StructPtr::dynamicCast(matches.front());
	if (p)
	{
	    if (_unit->ignRedefs())
	    {
		p->updateIncludeLevel();
		return p;
	    }

	    string msg = "redefinition of struct `";
	    msg += name;
	    msg += "'";
	    _unit->error(msg);
	    return 0;
	}
	
	string msg = "redefinition of `";
	msg += name;
	msg += "' as struct";
	_unit->error(msg);
	return 0;
    }

    StructPtr p = new Struct(this, name, local);
    _contents.push_back(p);
    return p;
}

SequencePtr
Slice::Container::createSequence(const string& name, const TypePtr& type, bool local)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    if (!matches.empty())
    {
	SequencePtr p = SequencePtr::dynamicCast(matches.front());
	if (p)
	{
	    if (_unit->ignRedefs())
	    {
		return p;
	    }

	    string msg = "redefinition of sequence `";
	    msg += name;
	    msg += "'";
	    _unit->error(msg);
	    return 0;
	}
	
	string msg = "redefinition of `";
	msg += name;
	msg += "' as sequence";
	_unit->error(msg);
	return 0;
    }

    SequencePtr p = new Sequence(this, name, type, local);
    _contents.push_back(p);
    return p;
}

DictionaryPtr
Slice::Container::createDictionary(const string& name, const TypePtr& keyType, const TypePtr& valueType, bool local)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    if (!matches.empty())
    {
	DictionaryPtr p = DictionaryPtr::dynamicCast(matches.front());
	if (p)
	{
	    if (_unit->ignRedefs())
	    {
		return p;
	    }

	    string msg = "redefinition of dictionary `";
	    msg += name;
	    msg += "'";
	    _unit->error(msg);
	    return 0;
	}
	
	string msg = "redefinition of `";
	msg += name;
	msg += "' as dictionary";
	_unit->error(msg);
	return 0;
    }

    DictionaryPtr p = new Dictionary(this, name, keyType, valueType, local);
    _contents.push_back(p);
    return p;
}

EnumPtr
Slice::Container::createEnum(const string& name, bool local)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    if (!matches.empty())
    {
	EnumPtr p = EnumPtr::dynamicCast(matches.front());
	if (p)
	{
	    if (_unit->ignRedefs())
	    {
		return p;
	    }

	    string msg = "redefinition of enum `";
	    msg += name;
	    msg += "'";
	    _unit->error(msg);
	    return 0;
	}
	
	string msg = "redefinition of `";
	msg += name;
	msg += "' as enum";
	_unit->error(msg);
	return 0;
    }

    EnumPtr p = new Enum(this, name, local);
    _contents.push_back(p);
    return p;
}

EnumeratorPtr
Slice::Container::createEnumerator(const string& name)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    if (!matches.empty())
    {
	EnumeratorPtr p = EnumeratorPtr::dynamicCast(matches.front());
	if (p)
	{
	    if (_unit->ignRedefs())
	    {
		return p;
	    }

	    string msg = "redefinition of enumerator `";
	    msg += name;
	    msg += "'";
	    _unit->error(msg);
	    return 0;
	}
	
	string msg = "redefinition of `";
	msg += name;
	msg += "' as enumerator";
	_unit->error(msg);
	return 0;
    }

    EnumeratorPtr p = new Enumerator(this, name);
    _contents.push_back(p);
    return p;
}

TypeList
Slice::Container::lookupType(const string& scoped, bool printError)
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
	"Object",
	"Object*",
	"LocalObject"
    };

    for (unsigned int i = 0; i < sizeof(builtinTable) / sizeof(const char*); ++i)
    {
	if (scoped == builtinTable[i])
	{
	    TypeList result;
	    result.push_back(_unit->builtin(static_cast<Builtin::Kind>(i)));
	    return result;
	}
    }

    return lookupTypeNoBuiltin(scoped, printError);
}

TypeList
Slice::Container::lookupTypeNoBuiltin(const string& scoped, bool printError)
{
    if (scoped.size() >= 2 && scoped[0] == ':')
    {
	return _unit->lookupTypeNoBuiltin(scoped.substr(2), printError);
    }
    
    ContainedList matches = _unit->findContents(thisScope() + scoped);
    if (matches.empty())
    {
	ContainedPtr contained = ContainedPtr::dynamicCast(this);
	if (!contained)
	{
	    if (printError)
	    {
		string msg = "`";
		msg += scoped;
		msg += "' is not defined";
		_unit->error(msg);
	    }
	    return TypeList();
	}
	return contained->container()->lookupTypeNoBuiltin(scoped, printError);
    }
    else
    {
	TypeList results;
	for (ContainedList::const_iterator p = matches.begin(); p != matches.end(); ++p)
	{
	    ClassDefPtr def = ClassDefPtr::dynamicCast(*p);
	    if (def)
	    {
		continue; // Ignore class definitions
	    }

	    ExceptionPtr ex = ExceptionPtr::dynamicCast(*p);
	    if (ex)
	    {
		if (printError)
		{
		    string msg = "`";
		    msg += scoped;
		    msg += "' is an exception, which cannot be used as a type";
		    _unit->error(msg);
		}
		return TypeList();
	    }

	    TypePtr type = TypePtr::dynamicCast(*p);
	    if (!type)
	    {
		if (printError)
		{
		    string msg = "`";
		    msg += scoped;
		    msg += "' is not a type";
		    _unit->error(msg);
		}
		return TypeList();
	    }
	    results.push_back(type);
	}
	return results;
    }
}

ContainedList
Slice::Container::lookupContained(const string& scoped, bool printError)
{
    if (scoped.size() >= 2 && scoped[0] == ':')
    {
	return _unit->lookupContained(scoped.substr(2), printError);
    }
    
    ContainedList matches = _unit->findContents(thisScope() + scoped);
    if (matches.empty())
    {
	ContainedPtr contained = ContainedPtr::dynamicCast(this);
	if (!contained)
	{
	    if (printError)
	    {
		string msg = "`";
		msg += scoped;
		msg += "' is not defined";
		_unit->error(msg);
	    }
	    return ContainedList();
	}
	return contained->container()->lookupContained(scoped, printError);
    }
    else
    {
	ContainedList results;
	for (ContainedList::const_iterator p = matches.begin(); p != matches.end(); ++p)
	{
	    if (!ClassDefPtr::dynamicCast(*p)) // Ignore class definitions
	    {
		results.push_back(*p);
	    }
	}
	return results;
    }
}

ExceptionPtr
Slice::Container::lookupException(const string& scoped, bool printError)
{
    ContainedList contained = lookupContained(scoped, printError);
    if (contained.empty())
    {
	return 0;
    }

    ExceptionList exceptions;
    for (ContainedList::iterator p = contained.begin(); p != contained.end(); ++p)
    {
	ExceptionPtr ex = ExceptionPtr::dynamicCast(*p);
	if (!ex)
	{
	    if (printError)
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
Slice::Container::modules()
{
    ModuleList result;
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	ModulePtr q = ModulePtr::dynamicCast(*p);
	if (q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

ClassList
Slice::Container::classes()
{
    ClassList result;
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	ClassDefPtr q = ClassDefPtr::dynamicCast(*p);
	if (q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

ExceptionList
Slice::Container::exceptions()
{
    ExceptionList result;
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	ExceptionPtr q = ExceptionPtr::dynamicCast(*p);
	if (q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

StructList
Slice::Container::structs()
{
    StructList result;
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	StructPtr q = StructPtr::dynamicCast(*p);
	if (q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

SequenceList
Slice::Container::sequences()
{
    SequenceList result;
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	SequencePtr q = SequencePtr::dynamicCast(*p);
	if (q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

DictionaryList
Slice::Container::dictionaries()
{
    DictionaryList result;
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	DictionaryPtr q = DictionaryPtr::dynamicCast(*p);
	if (q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

EnumList
Slice::Container::enums()
{
    EnumList result;
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	EnumPtr q = EnumPtr::dynamicCast(*p);
	if (q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

int
Slice::Container::includeLevel()
{
    return _includeLevel;
}

void
Slice::Container::updateIncludeLevel()
{
    _includeLevel = min(_includeLevel, _unit->currentIncludeLevel());
}

bool
Slice::Container::hasProxies()
{
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	ClassDeclPtr cl = ClassDeclPtr::dynamicCast(*p);
	if (cl && !cl->isLocal())
	{
	    return true;
	}

	ContainerPtr container = ContainerPtr::dynamicCast(*p);
	if (container && container->hasProxies())
	{
	    return true;
	}
    }

    return false;
}

bool
Slice::Container::hasClassDecls()
{
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	if (ClassDeclPtr::dynamicCast(*p))
	{
	    return true;
	}

	ContainerPtr container = ContainerPtr::dynamicCast(*p);
	if (container && container->hasClassDecls())
	{
	    return true;
	}
    }

    return false;
}

bool
Slice::Container::hasClassDefs()
{
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	if (ClassDefPtr::dynamicCast(*p))
	{
	    return true;
	}

	ContainerPtr container = ContainerPtr::dynamicCast(*p);
	if (container && container->hasClassDefs())
	{
	    return true;
	}
    }

    return false;
}

bool
Slice::Container::hasOtherConstructedOrExceptions()
{
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	if (ConstructedPtr::dynamicCast(*p) && !ClassDeclPtr::dynamicCast(*p) && !ClassDefPtr::dynamicCast(*p))
	{
	    return true;
	}

	if (ExceptionPtr::dynamicCast(*p))
	{
	    return true;
	}

	ContainerPtr container = ContainerPtr::dynamicCast(*p);
	if (container && container->hasOtherConstructedOrExceptions())
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
    if (contained)
    {
	s = contained->scoped();
    }
    s += "::";
    return s;
}

void
Slice::Container::mergeModules()
{
    for (ContainedList::iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	ModulePtr mod1 = ModulePtr::dynamicCast(*p);
	if (!mod1)
	{
	    continue;
	}
	
	ContainedList::iterator q = p;
	++q;
	while (q != _contents.end())
	{
	    ModulePtr mod2 = ModulePtr::dynamicCast(*q);
	    if (!mod2)
	    {
		++q;
		continue;
	    }
	    
	    if (mod1->name() != mod2->name())
	    {
		++q;
		continue;
	    }
	    
	    mod1->_contents.splice(mod1->_contents.end(), mod2->_contents);

	    if (mod1->_comment.length() < mod2->_comment.length())
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
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	ContainerPtr container = ContainerPtr::dynamicCast(*p);
	if (container)
	{
	    container->sort();
	    container->sortContents();
	}
    }
}

void
Slice::Container::visit(ParserVisitor* visitor)
{
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	(*p)->visit(visitor);
    }
}

Slice::Container::Container(const UnitPtr& unit) :
    SyntaxTreeBase(unit)
{
    if (_unit)
    {
	_includeLevel = _unit->currentIncludeLevel();
    }
    else
    {
	_includeLevel = 0;
    }
}

bool
Slice::Container::checkInterfaceAndLocal(const string& name, bool defined,
					 bool intf, bool intfOther,
					 bool local, bool localOther)
{
    string definedOrDeclared;
    if (defined)
    {
	definedOrDeclared = "defined";
    }
    else
    {
	definedOrDeclared = "declared";
    }

    if (!intf && intfOther)
    {
	string msg = "class `";
	msg += name;
	msg += "' was ";
	msg += definedOrDeclared;
	msg += " as interface";
	_unit->error(msg);
	return false;
    }
    
    if (intf && !intfOther)
    {
	string msg = "interface `";
	msg += name;
	msg += "' was ";
	msg += definedOrDeclared;
	msg += " as class";
	_unit->error(msg);
	return false;
    }
    
    if (!local && localOther)
    {
	string msg = "non-local `";
	msg += name;
	msg += "' was ";
	msg += definedOrDeclared;
	msg += " local";
	_unit->error(msg);
	return false;
    }
    
    if (local && !localOther)
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

// ----------------------------------------------------------------------
// Module
// ----------------------------------------------------------------------

Contained::ContainedType
Slice::Module::containedType()
{
    return ContainedTypeModule;
}

bool
Slice::Module::uses(const ConstructedPtr&)
{
    return false;
}

void
Slice::Module::visit(ParserVisitor* visitor)
{
    if (_includeLevel > 0)
    {
	return;
    }
   
    if (visitor->visitModuleStart(this))
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
Slice::Constructed::isLocal()
{
    return _local;
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
Slice::ClassDecl::containedType()
{
    return ContainedTypeClass;
}

bool
Slice::ClassDecl::uses(const ConstructedPtr& constructed)
{
    return false;
}

void
Slice::ClassDecl::visit(ParserVisitor* visitor)
{
    visitor->visitClassDecl(this);
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
    if (!matches.empty())
    {
	OperationPtr p = OperationPtr::dynamicCast(matches.front());
	if (p)
	{
	    if (_unit->ignRedefs())
	    {
		return p;
	    }

	    string msg = "redefinition of operation `";
	    msg += name;
	    msg += "'";
	    _unit->error(msg);
	    return 0;
	}
	
	string msg = "redefinition of `";
	msg += name;
	msg += "' as operation";
	_unit->error(msg);
	return 0;
    }

    {
	TypeStringList allParams = inParams;
	allParams.insert(allParams.end(), outParams.begin(), outParams.end());
	
	TypeStringList::const_iterator p = allParams.begin();
	while (p != allParams.end())
	{
	    TypeStringList::const_iterator q = p;
	    ++q;
	    while (q != allParams.end())
	    {
		if (p->second == q->second)
		{
		    string msg = "duplicate parameter `";
		    msg += p->second;
		    msg += "'";
		    _unit->error(msg);
		    return 0;
		}
		++q;
	    }
	    ++p;
	}
    }

    if (name == this->name())
    {
	string msg;
	if (isInterface())
	{
	    msg = "interface name `";
	}
	else
	{
	    msg = "class name `";
	}
	msg += name;
	msg += "' cannot be used as operation";
	_unit->error(msg);
	return 0;
    }

    OperationPtr p = new Operation(this, name, returnType, inParams, outParams, throws);
    _contents.push_back(p);
    return p;
}

DataMemberPtr
Slice::ClassDef::createDataMember(const string& name, const TypePtr& type)
{
    assert(!isInterface());

    ContainedList matches = _unit->findContents(thisScope() + name);
    if (!matches.empty())
    {
	DataMemberPtr p = DataMemberPtr::dynamicCast(matches.front());
	if (p)
	{
	    if (_unit->ignRedefs())
	    {
		return p;
	    }

	    string msg = "redefinition of data member `";
	    msg += name;
	    msg += "'";
	    _unit->error(msg);
	    return 0;
	}
	
	string msg = "redefinition of `";
	msg += name;
	msg += "' as data member";
	_unit->error(msg);
	return 0;
    }

    if (name == this->name())
    {
	string msg;
	if (isInterface())
	{
	    msg = "interface name `";
	}
	else
	{
	    msg = "class name `";
	}
	msg += name;
	msg += "' cannot be used as data member";
	_unit->error(msg);
	return 0;
    }

    _hasDataMembers = true;

    DataMemberPtr p = new DataMember(this, name, type);
    _contents.push_back(p);
    return p;
}

ClassDeclPtr
Slice::ClassDef::declaration()
{
    return _declaration;
}

ClassList
Slice::ClassDef::bases()
{
    return _bases;
}

ClassList
Slice::ClassDef::allBases()
{
    ClassList result = _bases;
    result.sort();
    result.unique();
    for (ClassList::const_iterator p = _bases.begin(); p != _bases.end(); ++p)
    {
	ClassList li = (*p)->allBases();
	result.merge(li);
	result.unique();
    }
    return result;
}

OperationList
Slice::ClassDef::operations()
{
    OperationList result;
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	OperationPtr q = OperationPtr::dynamicCast(*p);
	if (q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

OperationList
Slice::ClassDef::allOperations()
{
    OperationList result = operations();
    result.sort();
    result.unique();
    for (ClassList::const_iterator p = _bases.begin(); p != _bases.end(); ++p)
    {
	OperationList li = (*p)->allOperations();
	result.merge(li);
	result.unique();
    }
    return result;
}

DataMemberList
Slice::ClassDef::dataMembers()
{
    DataMemberList result;
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	DataMemberPtr q = DataMemberPtr::dynamicCast(*p);
	if (q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

bool
Slice::ClassDef::isAbstract()
{
    if (isInterface())
    {
	return true;
    }

    if (!_bases.empty() && _bases.front()->isAbstract())
    {
	return true;
    }

    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	if (OperationPtr::dynamicCast(*p))
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
Slice::ClassDef::isLocal()
{
    return _local;
}

bool
Slice::ClassDef::hasDataMembers()
{
    return _hasDataMembers;
}

Contained::ContainedType
Slice::ClassDef::containedType()
{
    return ContainedTypeClass;
}

bool
Slice::ClassDef::uses(const ConstructedPtr&)
{
    return false;
}

void
Slice::ClassDef::visit(ParserVisitor* visitor)
{
    if (_includeLevel > 0)
    {
	return;
    }
   
    if (visitor->visitClassDefStart(this))
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
    for (ClassList::const_iterator p = _bases.begin(); p != _bases.end(); ++p)
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
    if (!matches.empty())
    {
	DataMemberPtr p = DataMemberPtr::dynamicCast(matches.front());
	if (p)
	{
	    if (_unit->ignRedefs())
	    {
		return p;
	    }

	    string msg = "redefinition of data member `";
	    msg += name;
	    msg += "'";
	    _unit->error(msg);
	    return 0;
	}
	
	string msg = "redefinition of `";
	msg += name;
	msg += "' as data member";
	_unit->error(msg);
	return 0;
    }

    if (name == this->name())
    {
	string msg = "exception name `";
	msg += name;
	msg += "' cannot be used as data member";
	_unit->error(msg);
	return 0;
    }

    DataMemberPtr p = new DataMember(this, name, type);
    _contents.push_back(p);
    return p;
}

DataMemberList
Slice::Exception::dataMembers()
{
    DataMemberList result;
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	DataMemberPtr q = DataMemberPtr::dynamicCast(*p);
	if (q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

ExceptionPtr
Slice::Exception::base()
{
    return _base;
}

ExceptionList
Slice::Exception::allBases()
{
    ExceptionList result;
    if (_base)
    {
	result = _base->allBases();
	result.push_front(_base);
    }
    return result;
}

bool
Slice::Exception::isLocal()
{
    return _local;
}

Contained::ContainedType
Slice::Exception::containedType()
{
    return ContainedTypeException;
}

bool
Slice::Exception::uses(const ConstructedPtr&)
{
    return false;
}

void
Slice::Exception::visit(ParserVisitor* visitor)
{
    if (_includeLevel > 0)
    {
	return;
    }

    if (visitor->visitExceptionStart(this))
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
    if (!matches.empty())
    {
	DataMemberPtr p = DataMemberPtr::dynamicCast(matches.front());
	if (p)
	{
	    if (_unit->ignRedefs())
	    {
		return p;
	    }

	    string msg = "redefinition of data member `";
	    msg += name;
	    msg += "'";
	    _unit->error(msg);
	    return 0;
	}
	
	string msg = "redefinition of `";
	msg += name;
	msg += "' as data member";
	_unit->error(msg);
	return 0;
    }

    if (name == this->name())
    {
	string msg = "struct name `";
	msg += name;
	msg += "' cannot be used as data member";
	_unit->error(msg);
	return 0;
    }

    if (type.get() == this)
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
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	DataMemberPtr q = DataMemberPtr::dynamicCast(*p);
	if (q)
	{
	    result.push_back(q);
	}
    }
    return result;
}

Contained::ContainedType
Slice::Struct::containedType()
{
    return ContainedTypeStruct;
}

bool
Slice::Struct::uses(const ConstructedPtr&)
{
    return false;
}

void
Slice::Struct::visit(ParserVisitor* visitor)
{
    if (_includeLevel > 0)
    {
	return;
    }

    if (visitor->visitStructStart(this))
    {
	Container::visit(visitor);
	visitor->visitStructEnd(this);
    }
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
Slice::Sequence::containedType()
{
    return ContainedTypeSequence;
}

bool
Slice::Sequence::uses(const ConstructedPtr& constructed)
{
    ContainedPtr contained = ContainedPtr::dynamicCast(_type);
    if (contained && contained == constructed)
    {
	return true;
    }

    return false;
}

void
Slice::Sequence::visit(ParserVisitor* visitor)
{
    visitor->visitSequence(this);
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
Slice::Dictionary::containedType()
{
    return ContainedTypeDictionary;
}

bool
Slice::Dictionary::uses(const ConstructedPtr& constructed)
{
    {
	ContainedPtr contained = ContainedPtr::dynamicCast(_keyType);
	if (contained && contained == constructed)
	{
	    return true;
	}
    }

    {
	ContainedPtr contained = ContainedPtr::dynamicCast(_valueType);
	if (contained && contained == constructed)
	{
	    return true;
	}
    }

    return false;
}

void
Slice::Dictionary::visit(ParserVisitor* visitor)
{
    visitor->visitDictionary(this);
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
Slice::Enum::containedType()
{
    return ContainedTypeEnum;
}

bool
Slice::Enum::uses(const ConstructedPtr& constructed)
{
    return false;
}

void
Slice::Enum::visit(ParserVisitor* visitor)
{
    visitor->visitEnum(this);
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
Slice::Enumerator::containedType()
{
    return ContainedTypeEnumerator;
}

bool
Slice::Enumerator::uses(const ConstructedPtr& constructed)
{
    return false;
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
Slice::Operation::returnType()
{
    return _returnType;
}

TypeStringList
Slice::Operation::inputParameters()
{
    return _inParams;
}

TypeStringList
Slice::Operation::outputParameters()
{
    return _outParams;
}

ExceptionList
Slice::Operation::throws()
{
    return _throws;
}

Contained::ContainedType
Slice::Operation::containedType()
{
    return ContainedTypeOperation;
}

bool
Slice::Operation::uses(const ConstructedPtr& constructed)
{
    {
	ContainedPtr contained = ContainedPtr::dynamicCast(_returnType);
	if (contained && contained == constructed)
	{
	    return true;
	}
    }

    TypeStringList::const_iterator p;

    for (p = _inParams.begin(); p != _inParams.end(); ++p)
    {
	ContainedPtr contained = ContainedPtr::dynamicCast(p->first);
	if (contained && contained == constructed)
	{
	    return true;
	}
    }

    for (p = _outParams.begin(); p != _outParams.end(); ++p)
    {
	ContainedPtr contained = ContainedPtr::dynamicCast(p->first);
	if (contained && contained == constructed)
	{
	    return true;
	}
    }

    ExceptionList::const_iterator q;

    for (q = _throws.begin(); q != _throws.end(); ++q)
    {
	ContainedPtr contained = ContainedPtr::dynamicCast(*q);
	if (contained && contained == constructed)
	{
	    return true;
	}
    }

    return false;
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
Slice::DataMember::containedType()
{
    return ContainedTypeDataMember;
}

bool
Slice::DataMember::uses(const ConstructedPtr& constructed)
{
    ContainedPtr contained = ContainedPtr::dynamicCast(_type);
    if (contained && contained == constructed)
    {
	return true;
    }

    return false;
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
Slice::Unit::ignRedefs()
{
    return _ignRedefs;
}

void
Slice::Unit::setComment(const string& comment)
{
    _currentComment = "";

    string::size_type end = 0;
    while (true)
    {
	string::size_type begin = comment.find_first_not_of(" \t\r\n*", end);
	if (begin == string::npos)
	{
	    break;
	}

	end = comment.find('\n', begin);
	if (end != string::npos)
	{
	    if (end + 1 > begin)
	    {
		_currentComment += comment.substr(begin, end + 1 - begin);
	    }
	}
	else
	{
	    end = comment.find_last_not_of(" \t\r\n*");
	    if (end != string::npos)
	    {
		if (end + 1 > begin)
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
Slice::Unit::currentFile()
{
    return _currentFile;
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
    if (idx != string::npos)
    {
	line.erase(0, idx + 4);
    }

    idx = line.find_first_not_of(" \t\r#");
    if (idx != string::npos)
    {
	line.erase(0, idx);
    }

    _currentLine = atoi(line.c_str()) - 1;

    idx = line.find_first_of(" \t\r");
    if (idx != string::npos)
    {
	line.erase(0, idx);
    }

    idx = line.find_first_not_of(" \t\r\"");
    if (idx != string::npos)
    {
	line.erase(0, idx);

	idx = line.find_first_of(" \t\r\"");
	if (idx != string::npos)
	{
	    _currentFile = line.substr(0, idx);
	    line.erase(0, idx + 1);
	}
	else
	{
	    _currentFile = line;
	}

	idx = line.find_first_not_of(" \t\r");
	if (idx != string::npos)
	{
	    line.erase(0, idx);
	    int val = atoi(line.c_str());
	    if (val == 1)
	    {
		if (++_currentIncludeLevel == 1)
		{
		    if (find(_includeFiles.begin(), _includeFiles.end(), _currentFile) == _includeFiles.end())
		    {
			_includeFiles.push_back(_currentFile);
		    }
		}
	    }
	    else if (val == 2)
	    {
		--_currentIncludeLevel;
	    }
	    _currentComment.erase();
	}
	else
	{
	    if (_currentIncludeLevel == 0)
	    {
		_topLevelFile = _currentFile;
	    }
	}
    }
}

int
Slice::Unit::currentIncludeLevel()
{
    if (_all)
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
Slice::Unit::currentContainer()
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
    _contentMap[contained->scoped()].push_back(contained);
}

void
Slice::Unit::removeContent(const ContainedPtr& contained)
{
    string scoped = contained->scoped();
    map<string, ContainedList>::iterator p = _contentMap.find(scoped);
    assert(p != _contentMap.end());
    ContainedList::iterator q;
    for (q = p->second.begin(); q != p->second.end(); ++q)
    {
	if (q->get() == contained.get())
	{
	    p->second.erase(q);
	    return;
	}
    }
    assert(false);
}

ContainedList
Slice::Unit::findContents(const string& scoped)
{
    assert(!scoped.empty());
    assert(scoped[0] == ':');

    map<string, ContainedList>::const_iterator p = _contentMap.find(scoped);

    if (p != _contentMap.end())
    {
	return p->second;
    }
    else
    {
	return ContainedList();
    }
}

ClassList
Slice::Unit::findDerivedClasses(const ClassDefPtr& cl)
{
    ClassList derived;
    for(map<string, ContainedList>::const_iterator p = _contentMap.begin(); p != _contentMap.end(); ++p)
    {
	for(ContainedList::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
	{
	    ClassDefPtr r = ClassDefPtr::dynamicCast(*q);
	    if (r)
	    {
		ClassList bases = r->bases();
		if (find(bases.begin(), bases.end(), cl) != bases.end())
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
Slice::Unit::findDerivedExceptions(const ExceptionPtr& ex)
{
    ExceptionList derived;
    for(map<string, ContainedList>::const_iterator p = _contentMap.begin(); p != _contentMap.end(); ++p)
    {
	for(ContainedList::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
	{
	    ExceptionPtr r = ExceptionPtr::dynamicCast(*q);
	    if (r)
	    {
		ExceptionPtr base = r->base();
		if (base && base == ex)
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
Slice::Unit::findUsedBy(const ConstructedPtr& constructed)
{
    ContainedList usedBy;
    for(map<string, ContainedList>::const_iterator p = _contentMap.begin(); p != _contentMap.end(); ++p)
    {
	for(ContainedList::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
	{
	    if ((*q)->uses(constructed))
	    {
		usedBy.push_back(*q);
	    }
	}
    }
    usedBy.sort();
    usedBy.unique();
    return usedBy;
}

StringList
Slice::Unit::includeFiles()
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
    if (_errors)
    {
	status = EXIT_FAILURE;
    }

    if (status == EXIT_FAILURE)
    {
	while (!_containerStack.empty())
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
    if (visitor->visitUnitStart(this))
    {
	Container::visit(visitor);
	visitor->visitUnitEnd(this);
    }
}

BuiltinPtr
Slice::Unit::builtin(Builtin::Kind kind)
{
    map<Builtin::Kind, BuiltinPtr>::const_iterator p = _builtins.find(kind);
    if (p != _builtins.end())
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
