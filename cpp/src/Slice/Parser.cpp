// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Functional.h>
#include <Parser.h>

using namespace std;
using namespace Slice;

namespace Slice
{

Unit* unit;

}

void IceInternal::incRef(GrammerBase* p) { p->__incRef(); }
void IceInternal::decRef(GrammerBase* p) { p->__decRef(); }
void IceInternal::incRef(SyntaxTreeBase* p) { p->__incRef(); }
void IceInternal::decRef(SyntaxTreeBase* p) { p->__decRef(); }
void IceInternal::incRef(Type* p) { p->__incRef(); }
void IceInternal::decRef(Type* p) { p->__decRef(); }
void IceInternal::incRef(Builtin* p) { p->__incRef(); }
void IceInternal::decRef(Builtin* p) { p->__decRef(); }
void IceInternal::incRef(Contained* p) { p->__incRef(); }
void IceInternal::decRef(Contained* p) { p->__decRef(); }
void IceInternal::incRef(Container* p) { p->__incRef(); }
void IceInternal::decRef(Container* p) { p->__decRef(); }
void IceInternal::incRef(Module* p) { p->__incRef(); }
void IceInternal::decRef(Module* p) { p->__decRef(); }
void IceInternal::incRef(Constructed* p) { p->__incRef(); }
void IceInternal::decRef(Constructed* p) { p->__decRef(); }
void IceInternal::incRef(ClassDecl* p) { p->__incRef(); }
void IceInternal::decRef(ClassDecl* p) { p->__decRef(); }
void IceInternal::incRef(ClassDef* p) { p->__incRef(); }
void IceInternal::decRef(ClassDef* p) { p->__decRef(); }
void IceInternal::incRef(Proxy* p) { p->__incRef(); }
void IceInternal::decRef(Proxy* p) { p->__decRef(); }
void IceInternal::incRef(Operation* p) { p->__incRef(); }
void IceInternal::decRef(Operation* p) { p->__decRef(); }
void IceInternal::incRef(DataMember* p) { p->__incRef(); }
void IceInternal::decRef(DataMember* p) { p->__decRef(); }
void IceInternal::incRef(Vector* p) { p->__incRef(); }
void IceInternal::decRef(Vector* p) { p->__decRef(); }
void IceInternal::incRef(Enum* p) { p->__incRef(); }
void IceInternal::decRef(Enum* p) { p->__decRef(); }
void IceInternal::incRef(Enumerator* p) { p->__incRef(); }
void IceInternal::decRef(Enumerator* p) { p->__decRef(); }
void IceInternal::incRef(Native* p) { p->__incRef(); }
void IceInternal::decRef(Native* p) { p->__decRef(); }
void IceInternal::incRef(Unit* p) { p->__incRef(); }
void IceInternal::decRef(Unit* p) { p->__decRef(); }

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

Slice::Builtin::Kind
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
    return string(_scoped, 0, idx);
}

string
Slice::Contained::comment()
{
    return _comment;
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
    if (_unit)
    {
	_unit->addContent(this);
	_comment = _unit->currentComment();
    }
}

bool
Slice::operator<(Contained& l, Contained& r)
{
    if (l.containedType() != r.containedType())
    {
	return static_cast<int>(l.containedType()) < static_cast<int>(r.containedType());
    }

    return l.scoped() < r.scoped();
}

bool
Slice::operator==(Contained& l, Contained& r)
{
    return l.scoped() == r.scoped();
}

// ----------------------------------------------------------------------
// Container
// ----------------------------------------------------------------------

void
Slice::Container::destroy()
{
    for_each(_contents.begin(), _contents.end(), ::Ice::voidMemFun(&Contained::destroy));
    _contents.clear();
    SyntaxTreeBase::destroy();
}

ModulePtr
Slice::Container::createModule(const string& name)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    for (ContainedList::iterator p = matches.begin(); p != matches.end(); ++p)
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
Slice::Container::createClassDef(const string& name, bool local, bool intf, const ClassList& bases)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    for (ContainedList::iterator p = matches.begin(); p != matches.end(); ++p)
    {
	ClassDeclPtr cl = ClassDeclPtr::dynamicCast(*p);
	if (cl)
	{
	    if (checkInterfaceAndLocal(name, false, intf, cl->isInterface(), local, cl->isLocal()))
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
    
    ClassDefPtr def = new ClassDef(this, name, local, intf, bases);
    _contents.push_back(def);
    
    for (ContainedList::iterator q = matches.begin(); q != matches.end(); ++q)
    {
	ClassDeclPtr cl = ClassDeclPtr::dynamicCast(*q);
	cl->_definition = def;
    }

    //
    // Implicitly create a class declaration for each class
    // definition. This way the code generator can rely on always
    // having a class declaration available for lookup.
    //
    createClassDecl(name, local, intf);

    return def;
}

ClassDeclPtr
Slice::Container::createClassDecl(const string& name, bool local, bool intf)
{
    ClassDefPtr def;

    ContainedList matches = _unit->findContents(thisScope() + name);
    for (ContainedList::iterator p = matches.begin(); p != matches.end(); ++p)
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
    for (ContainedList::iterator q = _contents.begin(); q != _contents.end(); ++q)
    {
	if ((*q)->name() == name)
	{
	    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(*q);
	    if (cl)
	    {
		return cl;
	    }

	    assert(ClassDefPtr::dynamicCast(*q));
	}
    }

    ClassDeclPtr cl = new ClassDecl(this, name, local, intf);
    _contents.push_back(cl);

    if (def)
    {
	cl->_definition = def;
    }

    return cl;
}

VectorPtr
Slice::Container::createVector(const string& name, const TypePtr& type)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    if (!matches.empty())
    {
	VectorPtr p = VectorPtr::dynamicCast(matches.front());
	if (p)
	{
	    if (_unit->ignRedefs())
	    {
		return p;
	    }

	    string msg = "redefinition of vector `";
	    msg += name;
	    msg += "'";
	    _unit->error(msg);
	    return 0;
	}
	
	string msg = "redefinition of `";
	msg += name;
	msg += "' as vector";
	_unit->error(msg);
	return 0;
    }

    VectorPtr p = new Vector(this, name, type);
    _contents.push_back(p);
    return p;
}

EnumPtr
Slice::Container::createEnum(const string& name, const StringList& enumerators)
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

    EnumPtr p = new Enum(this, name, enumerators);
    _contents.push_back(p);
    return p;
}

EnumeratorPtr
Slice::Container::createEnumerator(const std::string& name)
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

NativePtr
Slice::Container::createNative(const string& name)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    if (!matches.empty())
    {
	NativePtr p = NativePtr::dynamicCast(matches.front());
	if (p)
	{
	    if (_unit->ignRedefs())
	    {
		return p;
	    }

	    string msg = "redefinition of native `";
	    msg += name;
	    msg += "'";
	    _unit->error(msg);
	    return 0;
	}
	
	string msg = "redefinition of `";
	msg += name;
	msg += "' as native";
	_unit->error(msg);
	return 0;
    }

    NativePtr p = new Native(this, name);
    _contents.push_back(p);
    return p;
}

TypeList
Slice::Container::lookupType(const string& scoped, bool printError)
{
    assert(!scoped.empty());

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
    assert(!scoped.empty());

    if (scoped[0] == ':')
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
	return contained->container()->lookupTypeNoBuiltin(scoped,
							       printError);
    }
    else
    {
	TypeList results;
	for (ContainedList::iterator p = matches.begin(); p != matches.end(); ++p)
	{
	    ClassDefPtr cl = ClassDefPtr::dynamicCast(*p);
	    if (cl)
		continue; // Ignore class definitions

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
    assert(!scoped.empty());

    if (scoped[0] == ':')
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
	return contained->container()->lookupContained(scoped,
							   printError);
    }
    else
    {
	ContainedList results;
	for (ContainedList::iterator p = matches.begin(); p != matches.end(); ++p)
	{
	    if (!ClassDefPtr::dynamicCast(*p)) // Ignore class definitions
	    {
		results.push_back(*p);
	    }
	}
	return results;
    }
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

VectorList
Slice::Container::vectors()
{
    VectorList result;
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	VectorPtr q = VectorPtr::dynamicCast(*p);
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

NativeList
Slice::Container::natives()
{
    NativeList result;
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	NativePtr q = NativePtr::dynamicCast(*p);
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
Slice::Container::hasOtherConstructedTypes()
{
    for (ContainedList::const_iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	if (ConstructedPtr::dynamicCast(*p) && !ClassDeclPtr::dynamicCast(*p) && !ClassDefPtr::dynamicCast(*p))
	{
	    return true;
	}

	ContainerPtr container = ContainerPtr::dynamicCast(*p);
	if (container && container->hasOtherConstructedTypes())
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

	    _unit->removeContent(*q);
	    q = _contents.erase(q);
	}
	
	mod1->mergeModules();
    }
}

void
Slice::Container::sort()
{
    for (ContainedList::iterator p = _contents.begin(); p != _contents.end(); ++p)
    {
	ContainerPtr container = ContainerPtr::dynamicCast(*p);
	if (container)
	{
	    container->sort();
	}
    }

    _contents.sort();
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

Slice::Contained::ContainedType
Slice::Module::containedType()
{
    return ContainedTypeModule;
}

void
Slice::Module::visit(ParserVisitor* visitor)
{
    if (_includeLevel > 0)
    {
	return;
    }
   
    visitor->visitModuleStart(this);
    Container::visit(visitor);
    visitor->visitModuleEnd(this);
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

Slice::Constructed::Constructed(const ContainerPtr& container, const string& name) :
    Type(container->unit()),
    Contained(container, name),
    SyntaxTreeBase(container->unit())
{
}

// ----------------------------------------------------------------------
// ClassDecl
// ----------------------------------------------------------------------

ClassDefPtr
Slice::ClassDecl::definition()
{
    return _definition;
}

bool
Slice::ClassDecl::isLocal()
{
    return _local;
}

bool
Slice::ClassDecl::isInterface()
{
    return _interface;
}

Slice::Contained::ContainedType
Slice::ClassDecl::containedType()
{
    return ContainedTypeClass;
}

void
Slice::ClassDecl::visit(ParserVisitor* visitor)
{
    visitor->visitClassDecl(this);
}

Slice::ClassDecl::ClassDecl(const ContainerPtr& container, const string& name, bool local, bool intf) :
    Constructed(container, name),
    Type(container->unit()),
    Contained(container, name),
    SyntaxTreeBase(container->unit()),
    _local(local),
    _interface(intf)
{
}

// ----------------------------------------------------------------------
// ClassDef
// ----------------------------------------------------------------------

void
Slice::ClassDef::destroy()
{
    _bases.clear();
    Container::destroy();
}

OperationPtr
Slice::ClassDef::createOperation(const string& name,
				 const TypePtr& returnType,
				 const TypeStringList& inParams,
				 const TypeStringList& outParams,
				 const TypeList& throws)
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
	
	TypeStringList::iterator p = allParams.begin();
	while (p != allParams.end())
	{
	    TypeStringList::iterator q = p;
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
	msg += "' can not be used as operation";
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
	msg += "' can not be used as data member";
	_unit->error(msg);
	return 0;
    }

    DataMemberPtr p = new DataMember(this, name, type);
    _contents.push_back(p);
    return p;
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
    for (ClassList::iterator p = _bases.begin(); p != _bases.end(); ++p)
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
    for (ClassList::iterator p = _bases.begin(); p != _bases.end(); ++p)
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
Slice::ClassDef::isLocal()
{
    return _local;
}

bool
Slice::ClassDef::isInterface()
{
    return _interface;
}

Slice::Contained::ContainedType
Slice::ClassDef::containedType()
{
    return ContainedTypeClass;
}

void
Slice::ClassDef::visit(ParserVisitor* visitor)
{
    if (_includeLevel > 0)
    {
	return;
    }
   
    visitor->visitClassDefStart(this);
    Container::visit(visitor);
    visitor->visitClassDefEnd(this);
}

Slice::ClassDef::ClassDef(const ContainerPtr& container, const string& name, bool local, bool intf,
			  const ClassList& bases) :
    Contained(container, name),
    Container(container->unit()),
    SyntaxTreeBase(container->unit()),
    _local(local),
    _interface(intf),
    _bases(bases)
{
    //
    // First element of bases may be a class, all others must be
    // interfaces
    //
#ifndef NDEBUG
    for (ClassList::iterator p = _bases.begin(); p != _bases.end(); ++p)
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

TypeList
Slice::Operation::throws()
{
    return _throws;
}

Slice::Contained::ContainedType
Slice::Operation::containedType()
{
    return ContainedTypeOperation;
}

void
Slice::Operation::visit(ParserVisitor* visitor)
{
    visitor->visitOperation(this);
}

Slice::Operation::Operation(const ContainerPtr& container, const string& name, const TypePtr& returnType,
			    const TypeStringList& inParams, const TypeStringList& outParams, const TypeList& throws) :
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

Slice::Contained::ContainedType
Slice::DataMember::containedType()
{
    return ContainedTypeDataMember;
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
// Native
// ----------------------------------------------------------------------

Slice::Contained::ContainedType
Slice::Native::containedType()
{
    return ContainedTypeNative;
}

void
Slice::Native::visit(ParserVisitor* visitor)
{
    visitor->visitNative(this);
}

Slice::Native::Native(const ContainerPtr& container, const string& name) :
    Constructed(container, name),
    Type(container->unit()),
    Contained(container,  name),
    SyntaxTreeBase(container->unit())
{
}

// ----------------------------------------------------------------------
// Vector
// ----------------------------------------------------------------------

TypePtr
Slice::Vector::type()
{
    return _type;
}

Slice::Contained::ContainedType
Slice::Vector::containedType()
{
    return ContainedTypeVector;
}

void
Slice::Vector::visit(ParserVisitor* visitor)
{
    visitor->visitVector(this);
}

Slice::Vector::Vector(const ContainerPtr& container, const string& name, const TypePtr& type) :
    Constructed(container, name),
    Type(container->unit()),
    Contained(container,  name),
    SyntaxTreeBase(container->unit()),
    _type(type)
{
}

// ----------------------------------------------------------------------
// Enum
// ----------------------------------------------------------------------

Slice::StringList
Slice::Enum::enumerators()
{
    return _enumerators;
}

Slice::Contained::ContainedType
Slice::Enum::containedType()
{
    return ContainedTypeEnum;
}

void
Slice::Enum::visit(ParserVisitor* visitor)
{
    visitor->visitEnum(this);
}

Slice::Enum::Enum(const ContainerPtr& container, const string& name, const StringList& enumerators) :
    Constructed(container, name),
    Type(container->unit()),
    Contained(container,  name),
    SyntaxTreeBase(container->unit()),
    _enumerators(enumerators)
{
}

// ----------------------------------------------------------------------
// Enumerator
// ----------------------------------------------------------------------

Slice::Contained::ContainedType
Slice::Enumerator::containedType()
{
    return ContainedTypeEnumerator;
}

Slice::Enumerator::Enumerator(const ContainerPtr& container, const string& name) :
    Contained(container, name),
    SyntaxTreeBase(container->unit())
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
Slice::Unit::setComment(const std::string& comment)
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
	_currentComment += comment.substr(begin, end - begin + 1);
    }
}

std::string
Slice::Unit::currentComment()
{
    string comment;
    comment.swap(_currentComment);
    return comment;
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
    cerr << _currentFile << ':' << _currentLine << " error: " << s << endl;
    yynerrs++;
}

void
Slice::Unit::error(const string& s)
{
    error(s.c_str());
}

void
Slice::Unit::warning(const char* s)
{
    cerr << _currentFile << ':' << _currentLine << " warning: " << s << endl;
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
    map<string, ContainedList >::iterator p = _contentMap.find(scoped);
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

    map<string, ContainedList >::iterator p = _contentMap.find(scoped);

    if (p != _contentMap.end())
    {
	return p->second;
    }
    else
    {
	return ContainedList();
    }
}

StringList
Slice::Unit::includeFiles()
{
    return _includeFiles;
}

int
Slice::Unit::parse(FILE* file, bool debug)
{
    extern int yydebug;
    yydebug = debug ? 1 : 0;

    assert(!Slice::unit);
    Slice::unit = this;

    _currentComment = "";
    _currentLine = 1;
    _currentIncludeLevel = 0;
    _currentFile = "<standard input>";
    _topLevelFile = _currentFile;
    _includeFiles.clear();
    pushContainer(this);

    extern FILE* yyin;
    yyin = file;
    int status = yyparse();
    if (yynerrs)
    {
	status = EXIT_FAILURE;
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
    _builtins.clear();
    Container::destroy();
}

void
Slice::Unit::visit(ParserVisitor* visitor)
{
    visitor->visitUnitStart(this);
    Container::visit(visitor);
    visitor->visitUnitEnd(this);
}

BuiltinPtr
Slice::Unit::builtin(Builtin::Kind kind)
{
    map<Builtin::Kind, BuiltinPtr>::iterator p = _builtins.find(kind);
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
    _all(all)
{
    _unit = this;
}
