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

Parser* parser;

}

void __Ice::incRef(Token* p) { p -> __incRef(); }
void __Ice::decRef(Token* p) { p -> __decRef(); }
void __Ice::incRef(String* p) { p -> __incRef(); }
void __Ice::decRef(String* p) { p -> __decRef(); }
void __Ice::incRef(Parameters* p) { p -> __incRef(); }
void __Ice::decRef(Parameters* p) { p -> __decRef(); }
void __Ice::incRef(Throws* p) { p -> __incRef(); }
void __Ice::decRef(Throws* p) { p -> __decRef(); }
void __Ice::incRef(DataMember* p) { p -> __incRef(); }
void __Ice::decRef(DataMember* p) { p -> __decRef(); }
void __Ice::incRef(SyntaxTreeBase* p) { p -> __incRef(); }
void __Ice::decRef(SyntaxTreeBase* p) { p -> __decRef(); }
void __Ice::incRef(Type* p) { p -> __incRef(); }
void __Ice::decRef(Type* p) { p -> __decRef(); }
void __Ice::incRef(Builtin* p) { p -> __incRef(); }
void __Ice::decRef(Builtin* p) { p -> __decRef(); }
void __Ice::incRef(Contained* p) { p -> __incRef(); }
void __Ice::decRef(Contained* p) { p -> __decRef(); }
void __Ice::incRef(Container* p) { p -> __incRef(); }
void __Ice::decRef(Container* p) { p -> __decRef(); }
void __Ice::incRef(Module* p) { p -> __incRef(); }
void __Ice::decRef(Module* p) { p -> __decRef(); }
void __Ice::incRef(Constructed* p) { p -> __incRef(); }
void __Ice::decRef(Constructed* p) { p -> __decRef(); }
void __Ice::incRef(ClassDecl* p) { p -> __incRef(); }
void __Ice::decRef(ClassDecl* p) { p -> __decRef(); }
void __Ice::incRef(ClassDef* p) { p -> __incRef(); }
void __Ice::decRef(ClassDef* p) { p -> __decRef(); }
void __Ice::incRef(Proxy* p) { p -> __incRef(); }
void __Ice::decRef(Proxy* p) { p -> __decRef(); }
void __Ice::incRef(Operation* p) { p -> __incRef(); }
void __Ice::decRef(Operation* p) { p -> __decRef(); }
void __Ice::incRef(Vector* p) { p -> __incRef(); }
void __Ice::decRef(Vector* p) { p -> __decRef(); }
void __Ice::incRef(Parser* p) { p -> __incRef(); }
void __Ice::decRef(Parser* p) { p -> __decRef(); }

// ----------------------------------------------------------------------
// SyntaxTreeBase
// ----------------------------------------------------------------------

void
Slice::SyntaxTreeBase::destroy()
{
    parser_ = 0;
}

Parser_ptr
Slice::SyntaxTreeBase::parser()
{
    return parser_;
}

void
Slice::SyntaxTreeBase::visit(ParserVisitor*)
{
}

Slice::SyntaxTreeBase::SyntaxTreeBase(const Parser_ptr& parser)
    : parser_(parser)
{
}

// ----------------------------------------------------------------------
// Type
// ----------------------------------------------------------------------

Slice::Type::Type(const Parser_ptr& parser)
    : SyntaxTreeBase(parser)
{
}

// ----------------------------------------------------------------------
// Builtin
// ----------------------------------------------------------------------

Slice::Builtin::Kind
Slice::Builtin::kind()
{
    return kind_;
}

Slice::Builtin::Builtin(const Parser_ptr& parser, Kind kind)
    : Type(parser),
      SyntaxTreeBase(parser),
      kind_(kind)
{
}

// ----------------------------------------------------------------------
// Contained
// ----------------------------------------------------------------------

Container_ptr
Slice::Contained::container()
{
    return container_;
}

string
Slice::Contained::name()
{
    return name_;
}

string
Slice::Contained::scoped()
{
    return scoped_;
}

string
Slice::Contained::scope()
{
    string::size_type idx = scoped_.rfind("::");
    assert(idx != string::npos);
    return string(scoped_, 0, idx);
}

Slice::Contained::Contained(const Container_ptr& container,
			      const string& name)
    : SyntaxTreeBase(container -> parser()),
      container_(container),
      name_(name)
{
    Contained_ptr cont = Contained_ptr::dynamicCast(container_);
    if(cont)
	scoped_ = cont -> scoped();
    scoped_ += "::" + name_;				       
    if(parser_)
	parser_ -> addContent(this);
}

bool
Slice::operator<(Contained& l, Contained& r)
{
    return l.name() < r.name();
}

bool
Slice::operator==(Contained& l, Contained& r)
{
    return l.name() == r.name();
}

// ----------------------------------------------------------------------
// Container
// ----------------------------------------------------------------------

void
Slice::Container::destroy()
{
    for(vector<Contained_ptr>::iterator p = contents_.begin();
	p != contents_.end();
	++p)
    {
	(*p) -> destroy();
    }

    contents_.clear();
    SyntaxTreeBase::destroy();
}

Module_ptr
Slice::Container::createModule(const string& name)
{
    vector<Contained_ptr> matches =
	parser_ -> findContents(thisScope() + name);
    for(vector<Contained_ptr>::iterator p = matches.begin();
	p != matches.end();
	++p)
    {
	Module_ptr module = Module_ptr::dynamicCast(*p);
	if(module)
	    continue; // Reopening modules is permissible
	
	assert(false); // TODO: Already exits
    }

    Module_ptr q = new Module(this, name);
    contents_.push_back(q);
    return q;
}

ClassDef_ptr
Slice::Container::createClassDef(const string& name,
				   const ClassDef_ptr& base,
				   bool local)
{
    vector<Contained_ptr> matches =
	parser_ -> findContents(thisScope() + name);
    for(vector<Contained_ptr>::iterator p = matches.begin();
	p != matches.end();
	++p)
    {
	ClassDecl_ptr cl = ClassDecl_ptr::dynamicCast(*p);
	assert(cl); // TODO: Already exits

	// TODO: Check whether locality matches
    }
    
    ClassDef_ptr def = new ClassDef(this, name, base, local);
    contents_.push_back(def);
    
    for(vector<Contained_ptr>::iterator q = matches.begin();
	q != matches.end();
	++q)
    {
	ClassDecl_ptr cl = ClassDecl_ptr::dynamicCast(*q);
	cl -> definition_ = def;
    }

    //
    // Implicitly create a class declaration for each class
    // definition. This way the code generator can rely on always
    // having a class declaration available for lookup.
    //
    createClassDecl(name, local);

    return def;
}

ClassDecl_ptr
Slice::Container::createClassDecl(const string& name, bool local)
{
    ClassDef_ptr def;

    vector<Contained_ptr> matches =
	parser_ -> findContents(thisScope() + name);
    for(vector<Contained_ptr>::iterator p = matches.begin();
	p != matches.end();
	++p)
    {
	ClassDef_ptr clDef = ClassDef_ptr::dynamicCast(*p);
	if(clDef)
	{
	    assert(!def);
	    def = clDef;
	    continue; // TODO: Check whether locality matches
	}
	
	ClassDecl_ptr clDecl = ClassDecl_ptr::dynamicCast(*p);
	if(clDecl)
	    continue; // TODO: Check whether locality matches
    
	assert(false); // TODO: Not a class
    }

    //
    // Multiple declarations are permissible. But if we do already
    // have a declaration for the class in this container, we don't
    // create another one.
    //
    for(vector<Contained_ptr>::iterator q = contents_.begin();
	q != contents_.end();
	++q)
    {
	if((*q) -> name() == name)
	{
	    ClassDecl_ptr cl = ClassDecl_ptr::dynamicCast(*q);
	    if(cl)
		return cl;

	    assert(ClassDef_ptr::dynamicCast(*q));
	}
    }

    ClassDecl_ptr cl = new ClassDecl(this, name, local);
    contents_.push_back(cl);

    if(def)
	cl -> definition_ = def;

    return cl;
}

Vector_ptr
Slice::Container::createVector(const string& name, const Type_ptr& type)
{
    vector<Contained_ptr> matches =
	parser_ -> findContents(thisScope() + name);
    assert(matches.empty()); // TODO: Already exits

    Vector_ptr p = new Vector(this, name, type);
    contents_.push_back(p);
    return p;
}

vector<Type_ptr>
Slice::Container::lookupType(const string& scoped)
{
    assert(!scoped.empty());

    if(scoped[0] == ':')
	return parser_ -> lookupType(scoped.substr(2));
    
    vector<Contained_ptr> matches =
	parser_ -> findContents(thisScope() + scoped);
    if(matches.empty())
    {
	Contained_ptr contained = Contained_ptr::dynamicCast(this);
	assert(contained); // TODO: Not found error
	return contained -> container() -> lookupType(scoped);
    }
    else
    {
	vector<Type_ptr> results;
	results.reserve(matches.size());
	for(vector<Contained_ptr>::iterator p = matches.begin();
	    p != matches.end();
	    ++p)
	{
	    ClassDef_ptr cl = ClassDef_ptr::dynamicCast(*p);
	    if(cl)
		continue; // Ignore class definitions

	    Type_ptr type = Type_ptr::dynamicCast(*p);
	    assert(type); // TODO: Not a type error
	    results.push_back(type);
	}
	return results;
    }
}

int
Slice::Container::includeLevel()
{
    return includeLevel_;
}

bool
Slice::Container::hasProxies()
{
    for(vector<Contained_ptr>::const_iterator p = contents_.begin();
	p != contents_.end();
	++p)
    {
	ClassDecl_ptr cl = ClassDecl_ptr::dynamicCast(*p);
	if(cl && !cl -> local())
	    return true;

	Container_ptr container = Container_ptr::dynamicCast(*p);
	if(container && container -> hasProxies())
	    return true;
    }

    return false;
}

bool
Slice::Container::hasClassDecls()
{
    for(vector<Contained_ptr>::const_iterator p = contents_.begin();
	p != contents_.end();
	++p)
    {
	if(ClassDecl_ptr::dynamicCast(*p))
	    return true;

	Container_ptr container = Container_ptr::dynamicCast(*p);
	if(container && container -> hasClassDecls())
	    return true;
    }

    return false;
}

bool
Slice::Container::hasClassDefs()
{
    for(vector<Contained_ptr>::const_iterator p = contents_.begin();
	p != contents_.end();
	++p)
    {
	if(ClassDef_ptr::dynamicCast(*p))
	    return true;

	Container_ptr container = Container_ptr::dynamicCast(*p);
	if(container && container -> hasClassDefs())
	    return true;
    }

    return false;
}

bool
Slice::Container::hasOtherConstructedTypes()
{
    for(vector<Contained_ptr>::const_iterator p = contents_.begin();
	p != contents_.end();
	++p)
    {
	if(Constructed_ptr::dynamicCast(*p) &&
	   !ClassDecl_ptr::dynamicCast(*p) &&
	   !ClassDef_ptr::dynamicCast(*p))
	    return true;

	Container_ptr container = Container_ptr::dynamicCast(*p);
	if(container && container -> hasOtherConstructedTypes())
	    return true;
    }

    return false;
}

string
Slice::Container::thisScope()
{
    string s;
    Contained_ptr contained = Contained_ptr::dynamicCast(this);
    if(contained)
	s = contained -> scoped();
    s += "::";
    return s;
}

void
Slice::Container::visit(ParserVisitor* visitor)
{
    for(vector<Contained_ptr>::const_iterator p = contents_.begin();
	p != contents_.end();
	++p)
    {
	(*p) -> visit(visitor);
    }
}

Slice::Container::Container(const Parser_ptr& parser)
    : SyntaxTreeBase(parser)
{
    if(parser_)
	includeLevel_ = parser -> currentIncludeLevel();
    else
	includeLevel_ = 0;
}

// ----------------------------------------------------------------------
// Module
// ----------------------------------------------------------------------

void
Slice::Module::visit(ParserVisitor* visitor)
{
    if(includeLevel_ > 0)
	return;
   
    visitor -> visitModuleStart(this);
    Container::visit(visitor);
    visitor -> visitModuleEnd(this);
}

Slice::Module::Module(const Container_ptr& container,
			const string& name)
    : Contained(container, name),
      Container(container -> parser()),
      SyntaxTreeBase(container -> parser())
{
}

// ----------------------------------------------------------------------
// Constructed
// ----------------------------------------------------------------------

Slice::Constructed::Constructed(const Container_ptr& container,
				  const string& name)
    : Type(container -> parser()),
      Contained(container, name),
      SyntaxTreeBase(container -> parser())
{
}

// ----------------------------------------------------------------------
// ClassDecl
// ----------------------------------------------------------------------

ClassDef_ptr
Slice::ClassDecl::definition()
{
    return definition_;
}

bool
Slice::ClassDecl::local()
{
    return local_;
}

void
Slice::ClassDecl::visit(ParserVisitor* visitor)
{
    visitor -> visitClassDecl(this);
}

Slice::ClassDecl::ClassDecl(const Container_ptr& container,
			      const string& name,
			      bool local)
    : Constructed(container, name),
      Type(container -> parser()),
      Contained(container, name),
      SyntaxTreeBase(container -> parser()),
      local_(local)
{
}

// ----------------------------------------------------------------------
// ClassDef
// ----------------------------------------------------------------------

void
Slice::ClassDef::destroy()
{
    base_ = 0;
    Container::destroy();
}

Operation_ptr
Slice::ClassDef::createOperation(const string& name,
				   const Type_ptr& returnType,
				   const TypeNameList& inParams,
				   const TypeNameList& outParams,
				   const TypeList& throws)
{
    vector<Contained_ptr> matches =
	parser_ -> findContents(thisScope() + name);
    assert(matches.empty()); // TODO: Already exits

    Operation_ptr p = new Operation(this, name, returnType,
				    inParams, outParams, throws);
    contents_.push_back(p);
    return p;
}

DataMember_ptr
Slice::ClassDef::createDataMember(const string& name, const Type_ptr& type)
{
    vector<Contained_ptr> matches =
	parser_ -> findContents(thisScope() + name);
    assert(matches.empty()); // TODO: Already exits

    DataMember_ptr p = new DataMember(this, name, type);
    contents_.push_back(p);
    return p;
}

ClassDef_ptr
Slice::ClassDef::base()
{
    return base_;
}

void
Slice::ClassDef::base(const ClassDef_ptr& cl)
{
    base_ = cl;
}

vector<Operation_ptr>
Slice::ClassDef::operations()
{
    vector<Operation_ptr> result;
    for(vector<Contained_ptr>::const_iterator p = contents_.begin();
	p != contents_.end();
	++p)
    {
	Operation_ptr derived = Operation_ptr::dynamicCast(*p);
	if(derived)
	    result.push_back(derived);
    }
    return result;
}

vector<DataMember_ptr>
Slice::ClassDef::dataMembers()
{
    vector<DataMember_ptr> result;
    for(vector<Contained_ptr>::const_iterator p = contents_.begin();
	p != contents_.end();
	++p)
    {
	DataMember_ptr derived = DataMember_ptr::dynamicCast(*p);
	if(derived)
	    result.push_back(derived);
    }
    return result;
}

bool
Slice::ClassDef::abstract()
{
    if(base_ && base_ -> abstract())
	return true;

    for(vector<Contained_ptr>::const_iterator p = contents_.begin();
	p != contents_.end();
	++p)
    {
	if(Operation_ptr::dynamicCast(*p))
	    return true;
    }

    return false;
}

bool
Slice::ClassDef::local()
{
    return local_;
}

void
Slice::ClassDef::visit(ParserVisitor* visitor)
{
    if(includeLevel_ > 0)
	return;
   
    visitor -> visitClassDefStart(this);
    Container::visit(visitor);
    visitor -> visitClassDefEnd(this);
}

Slice::ClassDef::ClassDef(const Container_ptr& container,
			    const string& name,
			    const ClassDef_ptr& base,
			    bool local)
    : Contained(container, name),
      Container(container -> parser()),
      SyntaxTreeBase(container -> parser()),
      base_(base),
      local_(local)
{
}

// ----------------------------------------------------------------------
// Proxy
// ----------------------------------------------------------------------

ClassDecl_ptr
Slice::Proxy::_class()
{
    return class_;
}

Slice::Proxy::Proxy(const ClassDecl_ptr& cl)
    : Type(cl -> parser()),
      SyntaxTreeBase(cl -> parser()),
      class_(cl)
{
}

// ----------------------------------------------------------------------
// Operation
// ----------------------------------------------------------------------

Type_ptr
Slice::Operation::returnType()
{
    return returnType_;
}

TypeNameList
Slice::Operation::inputParameters()
{
    return inParams_;
}

TypeNameList
Slice::Operation::outputParameters()
{
    return outParams_;
}

TypeList
Slice::Operation::throws()
{
    return throws_;
}

void
Slice::Operation::visit(ParserVisitor* visitor)
{
    visitor -> visitOperation(this);
}

Slice::Operation::Operation(const Container_ptr& container,
			      const string& name,
			      const Type_ptr& returnType,
			      const TypeNameList& inParams,
			      const TypeNameList& outParams,
			      const TypeList& throws)
    : Contained(container, name),
      SyntaxTreeBase(container -> parser()),
      returnType_(returnType),
      inParams_(inParams),
      outParams_(outParams),
      throws_(throws)
{
}

// ----------------------------------------------------------------------
// DataMember
// ----------------------------------------------------------------------

Type_ptr
Slice::DataMember::type()
{
    return type_;
}
void
Slice::DataMember::visit(ParserVisitor* visitor)
{
    visitor -> visitDataMember(this);
}

Slice::DataMember::DataMember(const Container_ptr& container,
				const string& name,
				const Type_ptr& type)
    : Contained(container, name),
      SyntaxTreeBase(container -> parser()),
      type_(type)
{
}

// ----------------------------------------------------------------------
// Vector
// ----------------------------------------------------------------------

Type_ptr
Slice::Vector::type()
{
    return type_;
}

void
Slice::Vector::visit(ParserVisitor* visitor)
{
    visitor -> visitVector(this);
}

Slice::Vector::Vector(const Container_ptr& container,
			const string& name,
			const Type_ptr& type)
    : Constructed(container, name),
      Type(container -> parser()),
      Contained(container,  name),
      SyntaxTreeBase(container -> parser()),
      type_(type)
{
}

// ----------------------------------------------------------------------
// Parser
// ----------------------------------------------------------------------

Parser_ptr
Slice::Parser::createParser()
{
    return new Parser;
}

void
Slice::Parser::nextLine()
{
    currentLine_++;
}

void
Slice::Parser::scanPosition(const char* s)
{
    string line(s);
    string::size_type idx;

    idx = line.find("line");
    if(idx != string::npos)
	line.erase(0, idx + 4);

    idx = line.find_first_not_of(" \t\r#");
    if(idx != string::npos)
	line.erase(0, idx);

    currentLine_ = atoi(line.c_str()) - 1;

    idx = line.find_first_of(" \t\r");
    if(idx != string::npos)
	line.erase(0, idx);

    idx = line.find_first_not_of(" \t\r\"");
    if(idx != string::npos)
    {
	line.erase(0, idx);

	idx = line.find_first_of(" \t\r\"");
	if(idx != string::npos)
	{
	    currentFile_ = line.substr(0, idx);
	    line.erase(0, idx + 1);
	}
	else
	    currentFile_ = line;

	idx = line.find_first_not_of(" \t\r");
	if(idx != string::npos)
	{
	    line.erase(0, idx);
	    int val = atoi(line.c_str());
	    if(val == 1)
	    {
		if(++currentIncludeLevel_ == 1)
		{
		    if(find(includeFiles_.begin(), includeFiles_.end(),
			    currentFile_) == includeFiles_.end())
		    {
			includeFiles_.push_back(currentFile_);
		    }
		}
	    }
	    else if(val == 2)
	    {
		--currentIncludeLevel_;
	    }
	}
	else
	{
	    if(currentIncludeLevel_ == 0)
		topLevelFile_ = currentFile_;
	}
    }
}

int
Slice::Parser::currentIncludeLevel()
{
    return currentIncludeLevel_;
}

void
Slice::Parser::error(const char* s)
{
    cerr << currentFile_ << ':' << currentLine_ << " error: " << s << endl;
    yynerrs++;
}

void
Slice::Parser::warning(const char* s)
{
    cerr << currentFile_ << ':' << currentLine_ << " warning: " << s << endl;
}

Container_ptr
Slice::Parser::currentContainer()
{
    assert(!containerStack_.empty());
    return containerStack_.top();
}

void
Slice::Parser::pushContainer(const Container_ptr& cont)
{
    containerStack_.push(cont);    
}

void
Slice::Parser::popContainer()
{
    assert(!containerStack_.empty());
    containerStack_.pop();
}

void
Slice::Parser::addContent(const Contained_ptr& contained)
{
    contentMap_[contained -> scoped()].push_back(contained);
}

vector<Contained_ptr>
Slice::Parser::findContents(const string& scoped)
{
    assert(!scoped.empty());
    assert(scoped[0] == ':');

    map<string, vector<Contained_ptr> >::iterator p =
	contentMap_.find(scoped);

    if(p != contentMap_.end())
	return p -> second;
    else
	return vector<Contained_ptr>();
}

vector<string>
Slice::Parser::includeFiles()
{
    return includeFiles_;
}

int
Slice::Parser::parse(FILE* file)
{
    assert(!Slice::parser);
    Slice::parser = this;

    currentLine_ = 1;
    currentIncludeLevel_ = 0;
    currentFile_ = "<standard input>";
    topLevelFile_ = currentFile_;
    includeFiles_.clear();
    pushContainer(this);

    extern FILE* yyin;
    yyin = file;
    int status = yyparse();

    assert(containerStack_.size() == 1);
    popContainer();

    Slice::parser = 0;
    return status;
}

void
Slice::Parser::destroy()
{
    builtins_.clear();
    Container::destroy();
}

void
Slice::Parser::visit(ParserVisitor* visitor)
{
    visitor -> visitUnitStart(this);
    Container::visit(visitor);
    visitor -> visitUnitEnd(this);
}

Builtin_ptr
Slice::Parser::builtin(Builtin::Kind kind)
{
    map<Builtin::Kind, Builtin_ptr>::iterator p = builtins_.find(kind);
    if(p != builtins_.end())
	return p -> second;
    Builtin_ptr builtin = new Builtin(this, kind);
    builtins_.insert(make_pair(kind, builtin));
    return builtin;
}

Slice::Parser::Parser()
    : SyntaxTreeBase(0),
      Container(0)
{
    parser_ = this;
}
