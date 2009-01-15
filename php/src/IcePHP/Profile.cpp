// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Profile.h>
#include <Util.h>

#include <Slice/Preprocessor.h>
#include <IceUtil/Options.h>
#include <IceUtil/InputUtil.h>
#include <fstream>

using namespace std;
using namespace IcePHP;

ZEND_EXTERN_MODULE_GLOBALS(ice)

//
// The name we give to the default profile.
//
static const char* _defaultProfileName = "__default__";

//
// The table of profiles.
//
static map<string, Profile*> _profiles;

namespace IcePHP
{

//
// CodeVisitor descends the Slice parse tree and generates PHP code for certain Slice types.
//
class CodeVisitor : public Slice::ParserVisitor
{
public:
    CodeVisitor(ostream&, Profile::ClassMap&, bool TSRMLS_DC);

    virtual void visitClassDecl(const Slice::ClassDeclPtr&);
    virtual bool visitClassDefStart(const Slice::ClassDefPtr&);
    virtual void visitClassDefEnd(const Slice::ClassDefPtr&);
    virtual bool visitExceptionStart(const Slice::ExceptionPtr&);
    virtual void visitExceptionEnd(const Slice::ExceptionPtr&);
    virtual bool visitStructStart(const Slice::StructPtr&);
    virtual void visitStructEnd(const Slice::StructPtr&);
    virtual void visitOperation(const Slice::OperationPtr&);
    virtual void visitDataMember(const Slice::DataMemberPtr&);
    virtual void visitDictionary(const Slice::DictionaryPtr&);
    virtual void visitEnum(const Slice::EnumPtr&);
    virtual void visitConst(const Slice::ConstPtr&);

private:

    string getTypeHint(const Slice::TypePtr&);
    string getDefaultValue(const Slice::TypePtr&);
    void writeConstructorParameter(const Slice::DataMemberPtr&);
    void writeConstructorAssignment(const Slice::DataMemberPtr&);

    ostream& _out;
    Profile::ClassMap& _classes;
    bool _suppressWarnings;
#ifdef ZTS
    TSRMLS_D;
#endif
};

} // End of namespace IcePHP

//
// This PHP code defines the core types we need. We supply a few of the common
// local exception subclasses; all other local exceptions are mapped to
// UnknownLocalException. We don't define Ice::Identity here because it's
// possible the user will have included its definition (see createProfile).
//
// NOTE: If a local exception is added or removed here, then changes are also
// necessary to IcePHP::throwException.
//
static const char* _coreTypes =
    "abstract class Ice_Exception extends Exception\n"
    "{\n"
    "    function __construct($message = '')\n"
    "    {\n"
    "        Exception::__construct($message);\n"
    "    }\n"
    "}\n"
    "\n"
    "abstract class Ice_LocalException extends Ice_Exception\n"
    "{\n"
    "    function __construct($message = '')\n"
    "    {\n"
    "        Ice_Exception::__construct($message);\n"
    "    }\n"
    "}\n"
    "\n"
    "class Ice_TwowayOnlyException extends Ice_LocalException\n"
    "{\n"
    "    function __construct($message = '')\n"
    "    {\n"
    "        Ice_LocalException::__construct($message);\n"
    "    }\n"
    "\n"
    "    function __toString()\n"
    "    {\n"
    "        return get_class($this) . \"\\n\" .\n"
    "               \"  operation: \" . $this->operation;\n"
    "    }\n"
    "\n"
    "    public $operation;\n"
    "}\n"
    "\n"
    "class Ice_UnknownException extends Ice_LocalException\n"
    "{\n"
    "    function __construct($message = '')\n"
    "    {\n"
    "        Ice_LocalException::__construct($message);\n"
    "    }\n"
    "\n"
    "    function __toString()\n"
    "    {\n"
    "        return get_class($this) . \"\\n\" .\n"
    "               $this->unknown;\n"
    "    }\n"
    "\n"
    "    public $unknown;\n"
    "}\n"
    "\n"
    "class Ice_UnknownLocalException extends Ice_UnknownException\n"
    "{\n"
    "    function __construct($message = '')\n"
    "    {\n"
    "        Ice_UnknownException::__construct($message);\n"
    "    }\n"
    "}\n"
    "\n"
    "class Ice_UnknownUserException extends Ice_UnknownException\n"
    "{\n"
    "    function __construct($message = '')\n"
    "    {\n"
    "        Ice_UnknownException::__construct($message);\n"
    "    }\n"
    "}\n"
    "\n"
    "class Ice_RequestFailedException extends Ice_LocalException\n"
    "{\n"
    "    function __construct($message = '')\n"
    "    {\n"
    "        Ice_LocalException::__construct($message);\n"
    "    }\n"
    "\n"
    "    function __toString()\n"
    "    {\n"
    "        return get_class($this) . \"\\n\" .\n"
    "               \"   identity: \" . Ice_identityToString($this->id) . \"\\n\" .\n"
    "               \"      facet: \" . $this->facet . \"\\n\" .\n"
    "               \"  operation: \" . $this->operation;\n"
    "    }\n"
    "\n"
    "    public $id;\n"
    "    public $facet;\n"
    "    public $operation;\n"
    "}\n"
    "\n"
    "class Ice_ObjectNotExistException extends Ice_RequestFailedException\n"
    "{\n"
    "    function __construct($message = '')\n"
    "    {\n"
    "        Ice_RequestFailedException::__construct($message);\n"
    "    }\n"
    "}\n"
    "\n"
    "class Ice_FacetNotExistException extends Ice_RequestFailedException\n"
    "{\n"
    "    function __construct($message = '')\n"
    "    {\n"
    "        Ice_RequestFailedException::__construct($message);\n"
    "    }\n"
    "}\n"
    "\n"
    "class Ice_OperationNotExistException extends Ice_RequestFailedException\n"
    "{\n"
    "    function __construct($message = '')\n"
    "    {\n"
    "        Ice_RequestFailedException::__construct($message);\n"
    "    }\n"
    "}\n"
    "\n"
    "class Ice_ProtocolException extends Ice_LocalException\n"
    "{\n"
    "    function __construct($message = '')\n"
    "    {\n"
    "        Ice_LocalException::__construct($message);\n"
    "    }\n"
    "\n"
    "    function __toString()\n"
    "    {\n"
    "        return get_class($this) . \"\\n\" .\n"
    "               \"  reason: \" . $this->reason;\n"
    "    }\n"
    "\n"
    "    public $reason;\n"
    "}\n"
    "\n"
    "class Ice_MarshalException extends Ice_ProtocolException\n"
    "{\n"
    "    function __construct($message = '')\n"
    "    {\n"
    "        Ice_ProtocolException::__construct($message);\n"
    "    }\n"
    "}\n"
    "\n"
    "class Ice_NoObjectFactoryException extends Ice_MarshalException\n"
    "{\n"
    "    function __construct($message = '')\n"
    "    {\n"
    "        Ice_MarshalException::__construct($message);\n"
    "    }\n"
    "\n"
    "    function __toString()\n"
    "    {\n"
    "        return get_class($this) . \"\\n\" .\n"
    "               \"  reason: \" . $this->reason . \"\\n\" .\n"
    "               \"    type: \" . $this->type;\n"
    "    }\n"
    "\n"
    "    public $type;\n"
    "}\n"
    "\n"
    "class Ice_UnexpectedObjectException extends Ice_MarshalException\n"
    "{\n"
    "    function __construct($message = '')\n"
    "    {\n"
    "        Ice_MarshalException::__construct($message);\n"
    "    }\n"
    "\n"
    "    function __toString()\n"
    "    {\n"
    "        return get_class($this) . \"\\n\" .\n"
    "               \"        reason: \" . $this->reason . \"\\n\" .\n"
    "               \"          type: \" . $this->type . \"\\n\" .\n"
    "               \"  expectedType: \" . $this->expectedType;\n"
    "    }\n"
    "\n"
    "    public $type;\n"
    "}\n"
    "\n"
    "class Ice_ProfileAlreadyLoadedException extends Ice_LocalException\n"
    "{\n"
    "    function __construct($message = '')\n"
    "    {\n"
    "        Ice_LocalException::__construct($message);\n"
    "    }\n"
    "}\n"
    "\n"
    "class Ice_ProfileNotFoundException extends Ice_LocalException\n"
    "{\n"
    "    function __construct($message = '')\n"
    "    {\n"
    "        Ice_LocalException::__construct($message);\n"
    "    }\n"
    "\n"
    "    function __toString()\n"
    "    {\n"
    "        return get_class($this) . \"\\n\" .\n"
    "               \"  name: \" . $this->name;\n"
    "    }\n"
    "\n"
    "    public $name;\n"
    "}\n"
    "\n"
    "abstract class Ice_UserException extends Ice_Exception\n"
    "{\n"
    "    function __construct($message = '')\n"
    "    {\n"
    "        Ice_Exception::__construct($message);\n"
    "    }\n"
    "}\n"
    "\n"
    "interface Ice_LocalObject\n"
    "{\n"
    "}\n"
    "\n"
    "class Ice_LocalObjectImpl implements Ice_LocalObject\n"
    "{\n"
    "}\n"
    "\n"
    "interface Ice_Object\n"
    "{\n"
    "    function ice_preMarshal();\n"
    "    function ice_postUnmarshal();\n"
    "}\n"
    "\n"
    "abstract class Ice_ObjectImpl implements Ice_Object\n"
    "{\n"
    "    function ice_preMarshal()\n"
    "    {\n"
    "    }\n"
    "\n"
    "    function ice_postUnmarshal()\n"
    "    {\n"
    "    }\n"
    "}\n"
    "\n"
    "interface Ice_ObjectFactory\n"
    "{\n"
    "    function create($id);\n"
    "    function destroy();\n"
    "}\n"
;

//
// Parse the Slice files that define the types and operations available to a PHP script.
//
static bool
parseSlice(const string& argStr, vector<Slice::UnitPtr>& units, bool& suppressWarnings TSRMLS_DC)
{
    vector<string> args;
    try
    {
        args = IceUtilInternal::Options::split(argStr);
    }
    catch(const IceUtil::Exception& ex)
    {
        ostringstream ostr;
        ex.ice_print(ostr);
        string msg = ostr.str();
        php_error_docref(0 TSRMLS_CC, E_ERROR, "error occurred while parsing Slice options in `%s':\n%s",
                         argStr.c_str(), msg.c_str());
        return false;
    }

    IceUtilInternal::Options opts;
    opts.addOpt("D", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("U", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("I", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("d", "debug");
    opts.addOpt("", "ice");
    opts.addOpt("", "case-sensitive");
    opts.addOpt("w");

    vector<string> files;
    try
    {
        args.insert(args.begin(), ""); // dummy argv[0]
        files = opts.parse(args);
        if(files.empty() && !argStr.empty())
        {
            php_error_docref(0 TSRMLS_CC, E_ERROR, "no Slice files specified in `%s'", argStr.c_str());
            return false;
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ostringstream ostr;
        ex.ice_print(ostr);
        string msg = ostr.str();
        php_error_docref(0 TSRMLS_CC, E_ERROR, "error occurred while parsing Slice options in `%s':\n%s",
                         argStr.c_str(), msg.c_str());
        return false;
    }

    vector<string> cppArgs;
    bool debug = false;
    bool ice = true; // This must be true so that we can create Ice::Identity when necessary.
    bool caseSensitive = false;
    if(opts.isSet("D"))
    {
        vector<string> optargs = opts.argVec("D");
        for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
        {
            cppArgs.push_back("-D" + *i);
        }
    }
    if(opts.isSet("U"))
    {
        vector<string> optargs = opts.argVec("U");
        for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
        {
            cppArgs.push_back("-U" + *i);
        }
    }
    if(opts.isSet("I"))
    {
        vector<string> optargs = opts.argVec("I");
        for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
        {
            cppArgs.push_back("-I" + *i);
        }
    }
    debug = opts.isSet("d") || opts.isSet("debug");
    caseSensitive = opts.isSet("case-sensitive");
    suppressWarnings = opts.isSet("w");

    bool ignoreRedefs = false;
    bool all = true;

    for(vector<string>::iterator p = files.begin(); p != files.end(); ++p)
    {
        Slice::UnitPtr unit = Slice::Unit::createUnit(ignoreRedefs, all, ice, caseSensitive);
        Slice::Preprocessor icecpp("icecpp", *p, cppArgs);
        FILE* cppHandle = icecpp.preprocess(false);

        if(cppHandle == 0)
        {
            return false;
        }

        int parseStatus = unit->parse(*p, cppHandle, debug);

        if(!icecpp.close())
        {
            return false;
        }

        if(parseStatus == EXIT_FAILURE)
        {
            return false;
        }

        units.push_back(unit);
    }

    return true;
}

static bool
createProfile(const string& name, const string& config, const string& options, const string& slice TSRMLS_DC)
{
    map<string, Profile*>::iterator p = _profiles.find(name);
    if(p != _profiles.end())
    {
        php_error_docref(0 TSRMLS_CC, E_ERROR, "profile `%s' already exists", name.c_str());
        return false;
    }

    Ice::PropertiesPtr properties = Ice::createProperties();

    if(!config.empty())
    {
        try
        {
            properties->load(config);
        }
        catch(const IceUtil::Exception& ex)
        {
            ostringstream ostr;
            ex.ice_print(ostr);
            php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to load Ice configuration file %s:\n%s", config.c_str(),
                             ostr.str().c_str());
            return false;
        }
    }

    if(!options.empty())
    {
        vector<string> args;
        try
        {
            args = IceUtilInternal::Options::split(options);
        }
        catch(const IceUtil::Exception& ex)
        {
            ostringstream ostr;
            ex.ice_print(ostr);
            string msg = ostr.str();
            php_error_docref(0 TSRMLS_CC, E_ERROR, "error occurred while parsing the options `%s':\n%s",
                             options.c_str(), msg.c_str());
            return false;
        }
        properties->parseCommandLineOptions("", args);
    }

    //
    // We create a Unit for each Slice file.
    //
    vector<Slice::UnitPtr> units;

    //
    // Even if the profile specifies no Slice files, we still need to obtain builtin
    // types as well as create types such as Ice::Identity.
    //
    {
        Slice::UnitPtr unit = Slice::Unit::createUnit(false, false, true, false);

        //
        // Create the Slice definition for Ice::Identity if it doesn't exist. The PHP class will
        // be created automatically by CodeVisitor.
        //
        string scoped = "::Ice::Identity";
        Slice::TypeList l = unit->lookupTypeNoBuiltin(scoped, false);
        if(l.empty())
        {
            Slice::ContainedList c = unit->lookupContained("Ice", false);
            Slice::ModulePtr module;
            if(c.empty())
            {
                module = unit->createModule("Ice");
            }
            else
            {
                module = Slice::ModulePtr::dynamicCast(c.front());
                if(!module)
                {
                    php_error_docref(0 TSRMLS_CC, E_ERROR,
                                     "the symbol `::Ice' is defined in Slice but is not a module");
                    return false;
                }
            }
            Slice::StructPtr identity = module->createStruct("Identity", false);
            Slice::TypePtr str = unit->builtin(Slice::Builtin::KindString);
            identity->createDataMember("category", str);
            identity->createDataMember("name", str);
        }

        //
        // Create the Slice definition for Ice::EndpointSelectionType if it doesn't exist. The PHP class will
        // be created automatically by CodeVisitor.
        //
        scoped = "::Ice::EndpointSelectionType";
        l = unit->lookupTypeNoBuiltin(scoped, false);
        if(l.empty())
        {
            Slice::ContainedList c = unit->lookupContained("Ice", false);
            Slice::ModulePtr module;
            if(c.empty())
            {
                module = unit->createModule("Ice");
            }
            else
            {
                module = Slice::ModulePtr::dynamicCast(c.front());
                if(!module)
                {
                    php_error_docref(0 TSRMLS_CC, E_ERROR,
                                     "the symbol `::Ice' is defined in Slice but is not a module");
                    return false;
                }
            }
            Slice::EnumPtr en = module->createEnum("EndpointSelectionType", false);
            Slice::EnumeratorList el;
            el.push_back(module->createEnumerator("Random"));
            el.push_back(module->createEnumerator("Ordered"));
            en->setEnumerators(el);
        }

        units.push_back(unit);
    }

    bool suppressWarnings = false;
    if(!slice.empty() && !parseSlice(slice, units, suppressWarnings TSRMLS_CC))
    {
        return false;
    }

    //
    // Descend the parse trees to generate PHP code.
    //
    ostringstream out;
    Profile::ClassMap classes;
    for(vector<Slice::UnitPtr>::const_iterator q = units.begin(); q != units.end(); ++ q)
    {
        CodeVisitor visitor(out, classes, suppressWarnings TSRMLS_CC);
        (*q)->visit(&visitor, false);
    }

    _profiles[name] = new Profile(name, units, out.str(), classes, properties);

    return true;
}

bool
IcePHP::profileInit(TSRMLS_D)
{
    //
    // The default profile is configured using ice.config, ice.options and ice.slice. Named profiles
    // are contained in a separate INI file, whose name is defined by ice.profiles.
    //
    const char* config = INI_STR("ice.config");
    const char* options = INI_STR("ice.options");
    const char* profiles = INI_STR("ice.profiles");
    const char* slice = INI_STR("ice.slice");

    if(!createProfile(_defaultProfileName, config, options, slice TSRMLS_CC))
    {
        return false;
    }

    if(strlen(profiles) > 0)
    {
        //
        // The Zend engine doesn't export a function for loading an INI file, so we
        // have to do it ourselves. The format is:
        //
        // [profile-name]
        // ice.config = config-file
        // ice.options = args
        // ice.slice = slice-args
        //
        ifstream in(profiles);
        if(!in)
        {
            php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to open Ice profiles in %s", profiles);
            return false;
        }

        string currentName, currentConfig, currentOptions, currentSlice;
        char line[1024];
        while(in.getline(line, 1024))
        {
            const string delim = " \t\r\n";
            string s = line;

            string::size_type idx = s.find(';');
            if(idx != string::npos)
            {
                s.erase(idx);
            }

            idx = s.find_last_not_of(delim);
            if(idx != string::npos && idx + 1 < s.length())
            {
                s.erase(idx + 1);
            }

            string::size_type beg = s.find_first_not_of(delim);
            if(beg == string::npos)
            {
                continue;
            }

            if(s[beg] == '[')
            {
                beg++;
                string::size_type end = s.find_first_of(" \t]", beg);
                if(end == string::npos || s[s.length() - 1] != ']')
                {
                    php_error_docref(0 TSRMLS_CC, E_ERROR, "invalid profile section in file %s:\n%s\n", profiles,
                                     line);
                    return false;
                }

                if(!currentName.empty())
                {
                    if(!createProfile(currentName, currentConfig, currentOptions, currentSlice TSRMLS_CC))
                    {
                        return false;
                    }
                    currentConfig.clear();
                    currentOptions.clear();
                    currentSlice.clear();
                }

                currentName = s.substr(beg, end - beg);
            }
            else
            {
                string::size_type end = s.find_first_of(delim + "=", beg);
                assert(end != string::npos);

                string key = s.substr(beg, end - beg);

                end = s.find('=', end);
                if(end == string::npos)
                {
                    php_error_docref(0 TSRMLS_CC, E_ERROR, "invalid profile entry in file %s:\n%s\n", profiles,
                                     line);
                    return false;
                }
                ++end;

                string value;
                beg = s.find_first_not_of(delim, end);
                if(beg != string::npos)
                {
                    end = s.length();
                    value = s.substr(beg, end - beg);
                }

                if(key == "config" || key == "ice.config")
                {
                    currentConfig = value;
                }
                else if(key == "options" || key == "ice.options")
                {
                    currentOptions = value;
                }
                else if(key == "slice" || key == "ice.slice")
                {
                    currentSlice = value;
                }
                else
                {
                    php_error_docref(0 TSRMLS_CC, E_ERROR, "unknown profile entry in file %s:\n%s\n", profiles,
                                     line);
                    return false;
                }

                if(currentName.empty())
                {
                    php_error_docref(0 TSRMLS_CC, E_ERROR, "no section for profile entry in file %s:\n%s\n",
                                     profiles, line);
                    return false;
                }
            }
        }

        if(!currentName.empty() && !createProfile(currentName, currentConfig, currentOptions, currentSlice TSRMLS_CC))
        {
            return false;
        }
    }

    return true;
}

bool
IcePHP::profileShutdown(TSRMLS_D)
{
    for(map<string, Profile*>::iterator p = _profiles.begin(); p != _profiles.end(); ++p)
    {
        p->second->destroy(TSRMLS_C);
        delete p->second;
    }

    _profiles.clear();

    return true;
}

IcePHP::Profile::Profile(const string& name, const vector<Slice::UnitPtr>& units, const string& code,
                         const ClassMap& classes, const Ice::PropertiesPtr& properties) :
    _name(name), _units(units), _code(code), _classes(classes), _properties(properties)
{
}

string
IcePHP::Profile::name() const
{
    return _name;
}

string
IcePHP::Profile::code() const
{
    return _code;
}

const IcePHP::Profile::ClassMap&
IcePHP::Profile::classes() const
{
    return _classes;
}

Ice::PropertiesPtr
IcePHP::Profile::properties() const
{
    return _properties;
}

Slice::TypePtr
IcePHP::Profile::lookupType(const string& id) const
{
    for(vector<Slice::UnitPtr>::const_iterator p = _units.begin(); p != _units.end(); ++p)
    {
        Slice::TypeList l = (*p)->lookupType(id, false);
        if(!l.empty())
        {
            return l.front();
        }
    }

    return 0;
}

Slice::ExceptionPtr
IcePHP::Profile::lookupException(const string& id) const
{
    for(vector<Slice::UnitPtr>::const_iterator p = _units.begin(); p != _units.end(); ++p)
    {
        Slice::ExceptionPtr ex = (*p)->lookupException(id, false);
        if(ex)
        {
            return ex;
        }
    }

    return 0;
}

void
IcePHP::Profile::destroy(TSRMLS_D)
{
    for(vector<Slice::UnitPtr>::iterator p = _units.begin(); p != _units.end(); ++p)
    {
        try
        {
            (*p)->destroy();
        }
        catch(const IceUtil::Exception& ex)
        {
            ostringstream ostr;
            ex.ice_print(ostr);
            php_error_docref(0 TSRMLS_CC, E_ERROR, "error while destroying Slice parse tree:\n%s\n",
                             ostr.str().c_str());
        }
    }
}

static bool
do_load(const string& name, const Ice::StringSeq& args TSRMLS_DC)
{
    Profile* profile = static_cast<Profile*>(ICE_G(profile));

    if(profile)
    {
        //
        // A profile has already been loaded; raise Ice_ProfileAlreadyLoadedException.
        //
        zend_class_entry* cls = findClass("Ice_ProfileAlreadyLoadedException" TSRMLS_CC);
        assert(cls);

        zval* zex;
        MAKE_STD_ZVAL(zex);
        if(object_init_ex(zex, cls) != SUCCESS)
        {
            php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to create exception %s", cls->name);
            return false;
        }

        zend_throw_exception_object(zex TSRMLS_CC);
        return false;
    }

    string profileName = name;
    if(profileName.empty())
    {
        profileName = _defaultProfileName;
    }

    //
    // Compile the core types if necessary. We do this now so that the exceptions
    // are available.
    //
    if(!findClass("Ice_Exception" TSRMLS_CC))
    {
        if(zend_eval_string(const_cast<char*>(_coreTypes), 0, "__core" TSRMLS_CC) == FAILURE)
        {
            php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to create core types:\n%s\n", _coreTypes);
            return false;
        }
    }

    map<string, Profile*>::iterator p = _profiles.find(profileName);
    if(p == _profiles.end())
    {
        zend_class_entry* cls = findClass("Ice_ProfileNotFoundException" TSRMLS_CC);
        assert(cls);

        zval* zex;
        MAKE_STD_ZVAL(zex);
        if(object_init_ex(zex, cls) != SUCCESS)
        {
            php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to create exception %s", cls->name);
            return false;
        }

        //
        // Set the name member.
        //
        zend_update_property_string(cls, zex, "name", sizeof("name") - 1,
                                    const_cast<char*>(profileName.c_str()) TSRMLS_CC);

        zend_throw_exception_object(zex TSRMLS_CC);
        return false;
    }
    profile = p->second;

    //
    // Compile the user-defined types.
    //
    if(zend_eval_string(const_cast<char*>(profile->code().c_str()), 0, "__slice" TSRMLS_CC) == FAILURE)
    {
        php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to create Slice types:\n%s\n", profile->code().c_str());
        return false;
    }

    //
    // Make a copy of the profile's properties, and include any command-line arguments.
    //
    Ice::PropertiesPtr properties = Ice::createProperties();
    properties->parseCommandLineOptions("", profile->properties()->getCommandLineOptions());
    properties->parseCommandLineOptions("", args);
    ICE_G(properties) = new Ice::PropertiesPtr(properties);

    ICE_G(profile) = profile;
    return true;
}

ZEND_FUNCTION(Ice_stringVersion)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    string s = ICE_STRING_VERSION;
    RETURN_STRINGL(const_cast<char*>(s.c_str()), s.length(), 1);
}

ZEND_FUNCTION(Ice_intVersion)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    RETURN_LONG(ICE_INT_VERSION);
}

ZEND_FUNCTION(Ice_loadProfile)
{
    if(ZEND_NUM_ARGS() > 1)
    {
        WRONG_PARAM_COUNT;
    }

    char* name = "";
    int len;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &name, &len) == FAILURE)
    {
        return;
    }

    Ice::StringSeq args;
    do_load(name, args TSRMLS_CC);
}

ZEND_FUNCTION(Ice_loadProfileWithArgs)
{
    if(ZEND_NUM_ARGS() > 2)
    {
        WRONG_PARAM_COUNT;
    }

    zval* zv;
    char* name = "";
    int len;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|s", &zv, &name, &len) == FAILURE)
    {
        return;
    }

    //
    // Extract the command-line arguments from the array.
    //
    Ice::StringSeq args;
    HashTable* arr = Z_ARRVAL_P(zv);
    void* data;
    HashPosition pos;
    zend_hash_internal_pointer_reset_ex(arr, &pos);
    while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
    {
        zval** val = reinterpret_cast<zval**>(data);
        if(Z_TYPE_PP(val) != IS_STRING)
        {
            php_error_docref(0 TSRMLS_CC, E_ERROR, "argument array must contain strings");
            return;
        }
        args.push_back(Z_STRVAL_PP(val));
        zend_hash_move_forward_ex(arr, &pos);
    }

    do_load(name, args TSRMLS_CC);
}

ZEND_FUNCTION(Ice_dumpProfile)
{
    Profile* profile = static_cast<Profile*>(ICE_G(profile));
    Ice::PropertiesPtr* properties = static_cast<Ice::PropertiesPtr*>(ICE_G(properties));

    if(!profile)
    {
        php_error_docref(0 TSRMLS_CC, E_ERROR, "no profile has been loaded");
        return;
    }

    ostringstream out;
    out << "Ice profile: " << profile->name() << endl;

    Ice::PropertyDict props = (*properties)->getPropertiesForPrefix("");
    if(!props.empty())
    {
        out << endl << "Ice configuration properties:" << endl << endl;
        for(Ice::PropertyDict::iterator p = props.begin(); p != props.end(); ++p)
        {
            out << p->first << "=" << p->second << endl;
        }
    }
    else
    {
        out << endl << "Ice configuration properties: <none>" << endl;
    }

    string code = profile->code();
    if(!code.empty())
    {
        out << endl << "PHP code for Slice types:" << endl << endl;
        out << code;
    }
    else
    {
        out << endl << "PHP code for Slice types: <none>" << endl;
    }

    string s = out.str();
    PUTS(s.c_str());
}

IcePHP::CodeVisitor::CodeVisitor(ostream& out, map<string, Slice::ClassDefPtr>& classes, bool suppressWarnings
                                 TSRMLS_DC) :
    _out(out), _classes(classes), _suppressWarnings(suppressWarnings)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif
}

void
IcePHP::CodeVisitor::visitClassDecl(const Slice::ClassDeclPtr& p)
{
    Slice::ClassDefPtr def = p->definition();
    if(!def && !_suppressWarnings)
    {
        string scoped = p->scoped();
        php_error_docref(0 TSRMLS_CC, E_WARNING, "%s %s declared but not defined",
                         p->isInterface() ? "interface" : "class", scoped.c_str());
    }
}

bool
IcePHP::CodeVisitor::visitClassDefStart(const Slice::ClassDefPtr& p)
{
    string flat = flatten(p->scoped());

    _classes[flat] = p;

    Slice::ClassList bases = p->bases();
    Slice::ClassDefPtr base;

    if(p->isInterface())
    {
        _out << "if(!interface_exists(\"" << flat << "\"))" << endl;
        _out << "{" << endl;
        _out << "interface " << flat;
        if(!bases.empty())
        {
            _out << " extends ";
            for(Slice::ClassList::iterator q = bases.begin(); q != bases.end(); ++q)
            {
                if(q != bases.begin())
                {
                    _out << ",";
                }
                _out << flatten((*q)->scoped());
            }
        }
        else if(!p->isLocal())
        {
            _out << " extends Ice_Object";
        }
    }
    else
    {
        _out << "if(!class_exists(\"" << flat << "\"))" << endl;
        _out << "{" << endl;
        if(p->isAbstract())
        {
            _out << "abstract ";
        }
        _out << "class " << flat;
        if(!bases.empty() && !bases.front()->isInterface())
        {
            _out << " extends " << flatten(bases.front()->scoped());
            base = bases.front();
            bases.pop_front();
        }
        else if(!p->isLocal())
        {
            _out << " extends Ice_ObjectImpl";
        }
        if(!bases.empty())
        {
            _out << " implements ";
            for(Slice::ClassList::iterator q = bases.begin(); q != bases.end(); ++q)
            {
                if(q != bases.begin())
                {
                    _out << ",";
                }
                _out << flatten((*q)->scoped());
            }
        }
    }

    _out << endl << '{' << endl;

    if(!p->isInterface())
    {
        Slice::DataMemberList baseMembers;
        if(base)
        {
            baseMembers = base->allDataMembers();
        }

        Slice::DataMemberList members = p->dataMembers();
        Slice::DataMemberList::const_iterator q;

        //
        // Generate a constructor.
        //
        _out << "function __construct(";
        for(q = baseMembers.begin(); q != baseMembers.end(); ++q)
        {
            if(q != baseMembers.begin())
            {
                _out << ", ";
            }
            writeConstructorParameter(*q);
        }
        for(q = members.begin(); q != members.end(); ++q)
        {
            if(!baseMembers.empty() || q != members.begin())
            {
                _out << ", ";
            }
            writeConstructorParameter(*q);
        }
        _out << ')' << endl;
        _out << '{' << endl;
        if(base)
        {
            _out << "    parent::__construct(";
            for(q = baseMembers.begin(); q != baseMembers.end(); ++q)
            {
                if(q != baseMembers.begin())
                {
                    _out << ", ";
                }
                _out << '$' << fixIdent((*q)->name());
            }
            _out << ");" << endl;
        }
        for(q = members.begin(); q != members.end(); ++q)
        {
            writeConstructorAssignment(*q);
        }
        _out << "}" << endl;
    }

    return true;
}

void
IcePHP::CodeVisitor::visitClassDefEnd(const Slice::ClassDefPtr& p)
{
    _out << '}' << endl;
    _out << '}' << endl; // interface_exists/class_exists
}

bool
IcePHP::CodeVisitor::visitExceptionStart(const Slice::ExceptionPtr& p)
{
    string flat = flatten(p->scoped());
    Slice::ExceptionPtr base = p->base();

    _out << "if(!class_exists(\"" << flat << "\"))" << endl;
    _out << "{" << endl;
    _out << "class " << flat << " extends ";
    string baseName;
    if(!base)
    {
        if(p->isLocal())
        {
            baseName = "Ice_LocalException";
        }
        else
        {
            baseName = "Ice_UserException";
        }
    }
    else
    {
        baseName = flatten(base->scoped());
    }

    _out << baseName << endl << '{' << endl;

    Slice::DataMemberList baseMembers;
    if(base)
    {
        baseMembers = base->allDataMembers();
    }

    Slice::DataMemberList members = p->dataMembers();
    Slice::DataMemberList::const_iterator q;

    //
    // Generate a constructor.
    //
    _out << "function __construct($_message=''";
    for(q = baseMembers.begin(); q != baseMembers.end(); ++q)
    {
        _out << ", ";
        writeConstructorParameter(*q);
    }
    for(q = members.begin(); q != members.end(); ++q)
    {
        _out << ", ";
        writeConstructorParameter(*q);
    }
    _out << ')' << endl;
    _out << '{' << endl;
    _out << "    " << baseName << "::__construct($_message";
    for(q = baseMembers.begin(); q != baseMembers.end(); ++q)
    {
        _out << ", $" << fixIdent((*q)->name());
    }
    _out << ");" << endl;
    for(q = members.begin(); q != members.end(); ++q)
    {
        writeConstructorAssignment(*q);
    }
    _out << "}" << endl;

    return true;
}

void
IcePHP::CodeVisitor::visitExceptionEnd(const Slice::ExceptionPtr& p)
{
    _out << '}' << endl;
    _out << '}' << endl; // class_exists
}

bool
IcePHP::CodeVisitor::visitStructStart(const Slice::StructPtr& p)
{
    string flat = flatten(p->scoped());

    _out << "if(!class_exists(\"" << flat << "\"))" << endl;
    _out << "{" << endl;
    _out << "class " << flatten(p->scoped()) << endl;
    _out << '{' << endl;

    //
    // Generate a constructor.
    //
    Slice::DataMemberList members = p->dataMembers();
    Slice::DataMemberList::const_iterator q;
    _out << "function __construct(";
    for(q = members.begin(); q != members.end(); ++q)
    {
        if(q != members.begin())
        {
            _out << ", ";
        }
        writeConstructorParameter(*q);
    }
    _out << ')' << endl;
    _out << '{' << endl;
    for(q = members.begin(); q != members.end(); ++q)
    {
        writeConstructorAssignment(*q);
    }
    _out << '}' << endl;

    return true;
}

void
IcePHP::CodeVisitor::visitStructEnd(const Slice::StructPtr& p)
{
    _out << '}' << endl;
    _out << '}' << endl; // class_exists
}

void
IcePHP::CodeVisitor::visitOperation(const Slice::OperationPtr& p)
{
    string name = fixIdent(p->name());

    Slice::ParamDeclList params = p->parameters();

    Slice::ClassDefPtr cl = Slice::ClassDefPtr::dynamicCast(p->container());
    assert(cl);

    if(!cl->isInterface())
    {
        _out << "abstract ";
    }
    _out << "function " << name << '(';
    for(Slice::ParamDeclList::const_iterator q = params.begin(); q != params.end(); ++q)
    {
        Slice::ParamDeclPtr param = *q;
        if(q != params.begin())
        {
            _out << ", ";
        }
        if(param->isOutParam())
        {
            _out << '&';
        }
        else
        {
            string hint = getTypeHint(param->type());
            if(!hint.empty())
            {
                _out << hint << ' ';
            }
        }
        _out << '$' << fixIdent(param->name());
    }
    _out << ");" << endl;
}

void
IcePHP::CodeVisitor::visitDataMember(const Slice::DataMemberPtr& p)
{
    Slice::ContainedPtr cont = Slice::ContainedPtr::dynamicCast(p->container());
    assert(cont);
    if(Slice::ClassDefPtr::dynamicCast(cont) && (cont->hasMetaData("protected") || p->hasMetaData("protected")))
    {
        _out << "protected $" << fixIdent(p->name()) << ';' << endl;
    }
    else
    {
        _out << "public $" << fixIdent(p->name()) << ';' << endl;
    }
}

void
IcePHP::CodeVisitor::visitDictionary(const Slice::DictionaryPtr& p)
{
    Slice::TypePtr keyType = p->keyType();
    if(!isNativeKey(keyType) && !_suppressWarnings)
    {
        //
        // TODO: Generate class.
        //
        string scoped = p->scoped();
        php_error_docref(0 TSRMLS_CC, E_WARNING, "skipping dictionary %s - unsupported key type", scoped.c_str());
    }
}

void
IcePHP::CodeVisitor::visitEnum(const Slice::EnumPtr& p)
{
    string flat = flatten(p->scoped());

    _out << "if(!class_exists(\"" << flat << "\"))" << endl;
    _out << "{" << endl;
    _out << "class " << flat << endl;
    _out << '{' << endl;

    //
    // Create a class constant for each enumerator.
    //
    Slice::EnumeratorList l = p->getEnumerators();
    Slice::EnumeratorList::const_iterator q;
    long i;
    for(q = l.begin(), i = 0; q != l.end(); ++q, ++i)
    {
        string name = fixIdent((*q)->name());
        _out << "    const " << fixIdent((*q)->name()) << " = " << i << ';' << endl;
    }

    _out << '}' << endl;
    _out << '}' << endl; // class_exists
}

void
IcePHP::CodeVisitor::visitConst(const Slice::ConstPtr& p)
{
    string flat = flatten(p->scoped());
    Slice::TypePtr type = p->type();
    string value = p->value();

    _out << "if(!defined(\"" << flat << "\"))" << endl;
    _out << "{" << endl;
    _out << "define(\"" << flat << "\", ";

    Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(type);
    Slice::EnumPtr en = Slice::EnumPtr::dynamicCast(type);
    if(b)
    {
        switch(b->kind())
        {
        case Slice::Builtin::KindBool:
        case Slice::Builtin::KindByte:
        case Slice::Builtin::KindShort:
        case Slice::Builtin::KindInt:
        case Slice::Builtin::KindFloat:
        case Slice::Builtin::KindDouble:
            _out << value;
            break;

        case Slice::Builtin::KindLong:
        {
            IceUtil::Int64 l;
            IceUtilInternal::stringToInt64(value, l);
            //
            // The platform's 'long' type may not be 64 bits, so we store 64-bit
            // values as a string.
            //
            if(sizeof(IceUtil::Int64) > sizeof(long) && (l < LONG_MIN || l > LONG_MAX))
            {
                _out << "\"" << value << "\";";
            }
            else
            {
                _out << value;
            }
            break;
        }

        case Slice::Builtin::KindString:
        {
            //
            // Expand strings into the basic source character set. We can't use isalpha() and the like
            // here because they are sensitive to the current locale.
            //
            static const string basicSourceChars = "abcdefghijklmnopqrstuvwxyz"
                                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                   "0123456789"
                                                   "_{}[]#()<>%:;,?*+=/^&|~!=,\\' \t";
            static const set<char> charSet(basicSourceChars.begin(), basicSourceChars.end());

            _out << "\"";                                        // Opening "

            ios_base::fmtflags originalFlags = _out.flags();        // Save stream state
            streamsize originalWidth = _out.width();
            ostream::char_type originalFill = _out.fill();

            for(string::const_iterator c = value.begin(); c != value.end(); ++c)
            {
                if(*c == '$')
                {
                    _out << "\\$";
                }
                else if(*c == '"')
                {
                    _out << "\\\"";
                }
                else if(charSet.find(*c) == charSet.end())
                {
                    unsigned char uc = *c;                    // char may be signed, so make it positive
                    _out << "\\";                                // Print as octal if not in basic source character set
                    _out.flags(ios_base::oct);
                    _out.width(3);
                    _out.fill('0');
                    _out << static_cast<unsigned>(uc);
                }
                else
                {
                    _out << *c;                                  // Print normally if in basic source character set
                }
            }

            _out.fill(originalFill);                             // Restore stream state
            _out.width(originalWidth);
            _out.flags(originalFlags);

            _out << "\"";                                        // Closing "

            break;
        }

        case Slice::Builtin::KindObject:
        case Slice::Builtin::KindObjectProxy:
        case Slice::Builtin::KindLocalObject:
            assert(false);
        }
    }
    else if(en)
    {
        string::size_type colon = value.rfind(':');
        if(colon != string::npos)
        {
            value = value.substr(colon + 1);
        }
        Slice::EnumeratorList l = en->getEnumerators();
        Slice::EnumeratorList::iterator q;
        for(q = l.begin(); q != l.end(); ++q)
        {
            if((*q)->name() == value)
            {
                _out << flatten(en->scoped()) << "::" << fixIdent(value);
                break;
            }
        }
    }

    _out << ");" << endl;
    _out << "}" << endl; // defined
}

string
IcePHP::CodeVisitor::getTypeHint(const Slice::TypePtr& type)
{
    //
    // Currently, the Zend engine does not allow an argument with a type hint to have
    // a value of null, therefore we can only use type hints for structs.
    //
    Slice::StructPtr st = Slice::StructPtr::dynamicCast(type);
    if(st)
    {
        return flatten(st->scoped());
    }

    return string();
}

string
IcePHP::CodeVisitor::getDefaultValue(const Slice::TypePtr& type)
{
    Slice::BuiltinPtr builtin = Slice::BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
        case Slice::Builtin::KindByte:
        case Slice::Builtin::KindShort:
        case Slice::Builtin::KindInt:
        case Slice::Builtin::KindLong:
            return "0";

        case Slice::Builtin::KindBool:
            return "false";

        case Slice::Builtin::KindFloat:
        case Slice::Builtin::KindDouble:
            return "0.0";

        case Slice::Builtin::KindString:
            return "''";

        case Slice::Builtin::KindObject:
        case Slice::Builtin::KindObjectProxy:
        case Slice::Builtin::KindLocalObject:
            return "null";
        }
    }

    Slice::EnumPtr en = Slice::EnumPtr::dynamicCast(type);
    if(en)
    {
        //
        // Use the first enumerator.
        //
        string flat = flatten(en->scoped());
        Slice::EnumeratorList l = en->getEnumerators();
        string name = fixIdent(l.front()->name());
        return flat + "::" + name;
    }

    Slice::StructPtr str = Slice::StructPtr::dynamicCast(type);
    if(str)
    {
        return "new " + flatten(str->scoped()) + "()";
    }

    return "null";
}

void
IcePHP::CodeVisitor::writeConstructorParameter(const Slice::DataMemberPtr& member)
{
    _out << '$' << fixIdent(member->name()) << '=';
    //
    // Structure types must be handled specially.
    //
    if(Slice::StructPtr::dynamicCast(member->type()))
    {
        //
        // If a data member is a structure, we want to initialize it to a new instance of the
        // structure type. However, PHP does not allow a call to "new" in the default value of
        // a function argument, so we assign a marker value now and create the instance in the
        // constructor body.
        //
        _out << "-1";
    }
    else
    {
        _out << getDefaultValue(member->type());
    }
}

void
IcePHP::CodeVisitor::writeConstructorAssignment(const Slice::DataMemberPtr& member)
{
    //
    // Structure types are instantiated in the constructor body.
    //
    string name = fixIdent(member->name());
    if(Slice::StructPtr::dynamicCast(member->type()))
    {
        _out << "    $this->" << name << " = $" << name << " == -1 ? " << getDefaultValue(member->type())
             << " : $" << name << ';' << endl;
    }
    else
    {
        _out << "    $this->" << name << " = $" << name << ';' << endl;
    }
}
