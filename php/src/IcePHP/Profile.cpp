// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Profile.h>
#include <Util.h>

#include <Slice/Preprocessor.h>
#include <IceUtil/Options.h>
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
    "define(\"ICE_STRING_VERSION\", \"3.2.0\");\n"
    "define(\"ICE_INT_VERSION\", 30200);\n"
    "\n"
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
    "interface Ice_ObjectFactory extends Ice_LocalObject\n"
    "{\n"
    "    function create($id);\n"
    "    function destroy();\n"
    "}\n"
;

//
// Parse the Slice files that define the types and operations available to a PHP script.
//
static bool
parseSlice(const string& argStr, Slice::UnitPtr& unit, bool& suppressWarnings TSRMLS_DC)
{
    vector<string> args;
    try
    {
        args = IceUtil::Options::split(argStr);
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

    IceUtil::Options opts;
    opts.addOpt("D", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("U", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("I", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
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

    string cppArgs;
    bool debug = false;
    bool ice = true; // This must be true so that we can create Ice::Identity when necessary.
    bool caseSensitive = false;
    if(opts.isSet("D"))
    {
        vector<string> optargs = opts.argVec("D");
        for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
        {
            cppArgs += " -D" + *i;
        }
    }
    if(opts.isSet("U"))
    {
        vector<string> optargs = opts.argVec("U");
        for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
        {
            cppArgs += " -U" + *i;
        }
    }
    if(opts.isSet("I"))
    {
        vector<string> optargs = opts.argVec("I");
        for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
        {
            cppArgs += " -I" + *i;
        }
    }
    debug = opts.isSet("d") || opts.isSet("debug");
    caseSensitive = opts.isSet("case-sensitive");
    suppressWarnings = opts.isSet("w");

    bool ignoreRedefs = false;
    bool all = true;
    unit = Slice::Unit::createUnit(ignoreRedefs, all, ice, caseSensitive);
    bool status = true;

    for(vector<string>::iterator p = files.begin(); p != files.end(); ++p)
    {
        Slice::Preprocessor icecpp("icecpp", *p, cppArgs);
        FILE* cppHandle = icecpp.preprocess(false);

        if(cppHandle == 0)
        {
            status = false;
            break;
        }

        int parseStatus = unit->parse(cppHandle, debug);

        if(!icecpp.close())
        {
            status = false;
            break;
        }

        if(parseStatus == EXIT_FAILURE)
        {
            status = false;
            break;
        }
    }

    return status;
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
            args = IceUtil::Options::split(options);
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

    Slice::UnitPtr unit;
    bool suppressWarnings = false;
    if(!slice.empty())
    {
        if(!parseSlice(slice, unit, suppressWarnings TSRMLS_CC))
        {
            return false;
        }
    }
    else
    {
        //
        // We must be allowed to obtain builtin types, as well as create Ice::Identity if necessary.
        //
        unit = Slice::Unit::createUnit(false, false, true, false);
    }

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
                php_error_docref(0 TSRMLS_CC, E_ERROR, "the symbol `::Ice' is defined in Slice but is not a module");
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
                php_error_docref(0 TSRMLS_CC, E_ERROR, "the symbol `::Ice' is defined in Slice but is not a module");
                return false;
            }
        }
        Slice::EnumPtr en = module->createEnum("EndpointSelectionType", false);
        Slice::EnumeratorList el;
        el.push_back(module->createEnumerator("Random"));
        el.push_back(module->createEnumerator("Ordered"));
        en->setEnumerators(el);
    }

    //
    // Descend the parse tree to create PHP code.
    //
    ostringstream out;
    Profile::ClassMap classes;
    CodeVisitor visitor(out, classes, suppressWarnings TSRMLS_CC);
    unit->visit(&visitor, false);

    Profile* profile = new Profile;
    profile->name = name;
    profile->unit = unit;
    profile->code = out.str();
    profile->classes = classes;
    profile->properties = properties;

    _profiles[name] = profile;

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
        try
        {
            p->second->unit->destroy();
        }
        catch(const IceUtil::Exception& ex)
        {
            ostringstream ostr;
            ex.ice_print(ostr);
            php_error_docref(0 TSRMLS_CC, E_ERROR, "error while destroying Slice parse tree:\n%s\n",
                             ostr.str().c_str());
        }

        delete p->second;
    }

    _profiles.clear();

    return true;
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
    if(zend_eval_string(const_cast<char*>(profile->code.c_str()), 0, "__slice" TSRMLS_CC) == FAILURE)
    {
        php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to create Slice types:\n%s\n", profile->code.c_str());
        return false;
    }

    //
    // Make a copy of the profile's properties, and include any command-line arguments.
    //
    Ice::PropertiesPtr properties = Ice::createProperties();
    properties->parseCommandLineOptions("", profile->properties->getCommandLineOptions());
    properties->parseCommandLineOptions("", args);
    ICE_G(properties) = new Ice::PropertiesPtr(properties);

    ICE_G(profile) = profile;
    return true;
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
    HashPosition pos;
    zval** val;
    zend_hash_internal_pointer_reset_ex(arr, &pos);
    while(zend_hash_get_current_data_ex(arr, reinterpret_cast<void**>(&val), &pos) != FAILURE)
    {
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
    out << "Ice profile: " << profile->name << endl;

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

    if(!profile->code.empty())
    {
        out << endl << "PHP code for Slice types:" << endl << endl;
        out << profile->code;
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

    if(p->isInterface())
    {
        _out << "interface " << flat << " extends ";
        if(!bases.empty())
        {
            for(Slice::ClassList::iterator q = bases.begin(); q != bases.end(); ++q)
            {
                if(q != bases.begin())
                {
                    _out << ",";
                }
                _out << flatten((*q)->scoped());
            }
        }
        else if(p->isLocal())
        {
            _out << "Ice_LocalObject";
        }
        else
        {
            _out << "Ice_Object";
        }
    }
    else
    {
        if(p->isAbstract())
        {
            _out << "abstract ";
        }
        _out << "class " << flat << " extends ";
        if(!bases.empty() && !bases.front()->isInterface())
        {
            _out << flatten(bases.front()->scoped());
            bases.pop_front();
        }
        else if(p->isLocal())
        {
            _out << "Ice_LocalObjectImpl";
        }
        else
        {
            _out << "Ice_ObjectImpl";
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

    return true;
}

void
IcePHP::CodeVisitor::visitClassDefEnd(const Slice::ClassDefPtr& p)
{
    _out << '}' << endl;
}

bool
IcePHP::CodeVisitor::visitExceptionStart(const Slice::ExceptionPtr& p)
{
    string flat = flatten(p->scoped());
    Slice::ExceptionPtr base = p->base();

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

    _out << "function __construct($message = '')" << endl;
    _out << "{" << endl;
    _out << "    " << baseName << "::__construct($message);" << endl;
    _out << "}" << endl;

    return true;
}

void
IcePHP::CodeVisitor::visitExceptionEnd(const Slice::ExceptionPtr& p)
{
    _out << '}' << endl;
}

bool
IcePHP::CodeVisitor::visitStructStart(const Slice::StructPtr& p)
{
    string flat = flatten(p->scoped());

    _out << "class " << flatten(p->scoped()) << endl;
    _out << '{' << endl;

    return true;
}

void
IcePHP::CodeVisitor::visitStructEnd(const Slice::StructPtr& p)
{
    _out << '}' << endl;
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
    _out << "public $" << fixIdent(p->name()) << ';' << endl;
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
        _out << "const " << fixIdent((*q)->name()) << " = " << i << ';' << endl;
    }

    _out << '}' << endl;
}

void
IcePHP::CodeVisitor::visitConst(const Slice::ConstPtr& p)
{
    string flat = flatten(p->scoped());
    Slice::TypePtr type = p->type();
    string value = p->value();

    _out << "define(\"" << flat << "\", ";

    Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(type);
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
            IceUtil::stringToInt64(value, l);
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

        _out << ");" << endl;
        return;
    }

    Slice::EnumPtr en = Slice::EnumPtr::dynamicCast(type);
    if(en)
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
                _out << flatten(en->scoped()) << "::" << fixIdent(value) << ");" << endl;
                return;
            }
        }
        assert(false); // No match found.
    }
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
