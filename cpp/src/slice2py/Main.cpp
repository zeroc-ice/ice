// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/Options.h>
#include <Slice/Preprocessor.h>
#include <Slice/PythonUtil.h>

#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif

#ifndef _WIN32
#include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace Slice::Python;

//
// For each Slice file Foo.ice we generate Foo_ice.py containing the Python
// mappings. Furthermore, for each Slice module M in Foo.ice, we create a
// Python package of the same name. This package is simply a subdirectory
// containing the special file "__init__.py" that is executed when a Python
// script executes the statement "import M".
//
// Inside __init__.py we add an import statement for Foo_ice, causing
// Foo_ice to be imported implicitly when M is imported.
// 
// Of course, another Slice file Bar.ice may contain definitions for the
// same Slice module M, in which case the __init__.py file for M is modified
// to contain an additional import statement for Bar_ice. Therefore a
// Python script executing "import M" implicitly imports the definitions
// from both Foo_ice and Bar_ice.
//
// The __init__.py file also contains import statements for submodules,
// so that importing the top-level module automatically imports all of
// its submodules.
//
// The PackageVisitor class creates the directory hierarchy to mirror the
// Slice module hierarchy, and updates the __init__.py files as necessary.
//
class PackageVisitor : public ParserVisitor
{
public:

    PackageVisitor(const string&, const string&, const string&);

    virtual bool visitModuleStart(const ModulePtr&);
    virtual void visitModuleEnd(const ModulePtr&);

private:

    enum ReadState { PreModules, InModules, InSubmodules };

    static const char* _moduleTag;
    static const char* _submoduleTag;

    bool createDirectory(const string&);

    bool addModule(const string&, const string&);
    bool addSubmodule(const string&, const string&);

    bool readInit(const string&, StringList&, StringList&);
    bool writeInit(const string&, const StringList&, const StringList&);

    string _name;
    string _module;
    StringList _pathStack;
};

const char* PackageVisitor::_moduleTag = "# Modules:";
const char* PackageVisitor::_submoduleTag = "# Submodules:";

PackageVisitor::PackageVisitor(const string& name, const string& module, const string& dir) :
    _name(name), _module(module)
{
    if(dir.empty())
    {
        _pathStack.push_front(".");
    }
    else
    {
        _pathStack.push_front(dir);
    }
}

bool
PackageVisitor::visitModuleStart(const ModulePtr& p)
{
    assert(!_pathStack.empty());
    string name = fixIdent(p->name());

    string path;
    if(_pathStack.size() == 1)
    {
        path = _pathStack.front();

        //
        // Check top-level modules for package metadata and create the package
        // directories.
        //
        string package = getPackageMetadata(p);
        if(!package.empty())
        {
            vector<string> v;
            if(!splitString(package, v, "."))
            {
                return false;
            }
            for(vector<string>::iterator q = v.begin(); q != v.end(); ++q)
            {
                if(q != v.begin() && !addSubmodule(path, fixIdent(*q)))
                {
                    return false;
                }
                    
                path += "/" + *q;
                if(!createDirectory(path))
                {
                    return false;
                }

                if(!addModule(path, _module))
                {
                    return false;
                }
            }

            if(!addSubmodule(path, name))
            {
                return false;
            }
        }

        path += "/" + name;
    }
    else
    {
        path = _pathStack.front() + "/" + name;
    }

    string parentPath = _pathStack.front();
    _pathStack.push_front(path);

    if(!createDirectory(path))
    {
        return false;
    }

    //
    // If necessary, add this module to the set of imported modules in __init__.py.
    //
    if(!addModule(path, _module))
    {
        return false;
    }

    //
    // If this is a submodule, then modify the parent's __init__.py to import us.
    //
    ModulePtr mod = ModulePtr::dynamicCast(p->container());
    if(mod && !addSubmodule(parentPath, name))
    {
        return false;
    }

    return true;
}

void
PackageVisitor::visitModuleEnd(const ModulePtr& p)
{
    assert(!_pathStack.empty());
    _pathStack.pop_front();
}

bool
PackageVisitor::createDirectory(const string& dir)
{
    struct stat st;
    int result;
    result = stat(dir.c_str(), &st);
    if(result != 0)
    {
#ifdef _WIN32
        result = _mkdir(dir.c_str());
#else       
        result = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
#endif

        if(result != 0)
        {
            cerr << _name << ": unable to create `" << dir << "': " << strerror(errno) << endl;
            return false;
        }
    }

    return true;
}

bool
PackageVisitor::addModule(const string& dir, const string& name)
{
    //
    // Add a module to the set of imported modules in __init__.py.
    //
    StringList modules, submodules;
    if(readInit(dir, modules, submodules))
    {
        StringList::iterator p = find(modules.begin(), modules.end(), name);
        if(p == modules.end())
        {
            modules.push_back(name);
            return writeInit(dir, modules, submodules);
        }

        return true;
    }

    return false;
}

bool
PackageVisitor::addSubmodule(const string& dir, const string& name)
{
    //
    // Add a submodule to the set of imported modules in __init__.py.
    //
    StringList modules, submodules;
    if(readInit(dir, modules, submodules))
    {
        StringList::iterator p = find(submodules.begin(), submodules.end(), name);
        if(p == submodules.end())
        {
            submodules.push_back(name);
            return writeInit(dir, modules, submodules);
        }

        return true;
    }

    return false;
}

bool
PackageVisitor::readInit(const string& dir, StringList& modules, StringList& submodules)
{
    string initPath = dir + "/__init__.py";

    struct stat st;
    if(stat(initPath.c_str(), &st) == 0)
    {
        ifstream in(initPath.c_str());
        if(!in)
        {
            cerr << _name << ": unable to open `" << initPath << "': " << strerror(errno) << endl;
            return false;
        }

        ReadState state = PreModules;
        char line[1024];
        while(in.getline(line, 1024))
        {
            string s = line;
            if(s.find(_moduleTag) == 0)
            {
                if(state != PreModules)
                {
                    break;
                }
                state = InModules;
            }
            else if(s.find(_submoduleTag) == 0)
            {
                if(state != InModules)
                {
                    break;
                }
                state = InSubmodules;
            }
            else if(s.find("import") == 0)
            {
                if(state == PreModules)
                {
                    break;
                }

                if(s.size() < 8)
                {
                    cerr << _name << ": invalid line `" << s << "' in `" << initPath << "'" << endl;
                    return false;
                }

                string name = s.substr(7);
                if(state == InModules)
                {
                    modules.push_back(name);
                }
                else
                {
                    submodules.push_back(name);
                }
            }
        }

        if(state != InSubmodules)
        {
            cerr << _name << ": invalid format in `" << initPath << "'" << endl;
            return false;
        }
    }

    return true;
}

bool
PackageVisitor::writeInit(const string& dir, const StringList& modules, const StringList& submodules)
{
    string initPath = dir + "/__init__.py";

    ofstream os(initPath.c_str());
    if(!os)
    {
        return false;
    }

    StringList::const_iterator p;

    os << "# Generated by slice2py - DO NOT EDIT!" << endl
       << "#" << endl
       << _moduleTag << endl;
    for(p = modules.begin(); p != modules.end(); ++p)
    {
        os << "import " << *p << endl;
    }

    os << endl;
    os << _submoduleTag << endl;
    for(p = submodules.begin(); p != submodules.end(); ++p)
    {
        os << "import " << *p << endl;
    }

    return true;
}

void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options] slice-files...\n";
    cerr <<        
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        "-DNAME               Define NAME as 1.\n"
        "-DNAME=DEF           Define NAME as DEF.\n"
        "-UNAME               Remove any definition for NAME.\n"
        "-IDIR                Put DIR in the include file search path.\n"
        "-E                   Print preprocessor output on stdout.\n"
        "--output-dir DIR     Create files in the directory DIR.\n"
        "-d, --debug          Print debug messages.\n"
        "--ice                Permit `Ice' prefix (for building Ice source code only)\n"
        "--all                Generate code for Slice definitions in included files.\n"
        "--no-package         Do not create Python packages.\n"
        "--checksum           Generate checksums for Slice definitions.\n"
        "--prefix PREFIX      Prepend filenames of Python modules with PREFIX.\n"
        ;
    // Note: --case-sensitive is intentionally not shown here!
}

int
main(int argc, char* argv[])
{
    IceUtil::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("D", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("U", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("I", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("E");
    opts.addOpt("", "output-dir", IceUtil::Options::NeedArg);
    opts.addOpt("d", "debug");
    opts.addOpt("", "ice");
    opts.addOpt("", "all");
    opts.addOpt("", "no-package");
    opts.addOpt("", "checksum");
    opts.addOpt("", "prefix", IceUtil::Options::NeedArg);
    opts.addOpt("", "case-sensitive");
     
    vector<string> args;
    try
    {
        args = opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtil::BadOptException& e)
    {
        cerr << argv[0] << ": " << e.reason << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if(opts.isSet("help"))
    {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if(opts.isSet("version"))
    {
        cout << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    string cppArgs;
    vector<string> optargs = opts.argVec("D");
    vector<string>::const_iterator i;
    for(i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs += " -D" + Preprocessor::addQuotes(*i);
    }

    optargs = opts.argVec("U");
    for(i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs += " -U" + Preprocessor::addQuotes(*i);
    }

    vector<string> includePaths = opts.argVec("I");
    for(i = includePaths.begin(); i != includePaths.end(); ++i)
    {
	cppArgs += " -I" + Preprocessor::normalizeIncludePath(*i);
    }

    bool preprocess = opts.isSet("E");

    string output = opts.optArg("output-dir");

    bool debug = opts.isSet("debug");

    bool ice = opts.isSet("ice");

    bool all = opts.isSet("all");

    bool noPackage = opts.isSet("no-package");

    bool checksum = opts.isSet("checksum");

    string prefix = opts.optArg("prefix");

    bool caseSensitive = opts.isSet("case-sensitive");

    if(args.empty())
    {
        cerr << argv[0] << ": no input file" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    int status = EXIT_SUCCESS;

    for(i = args.begin(); i != args.end(); ++i)
    {
        Preprocessor icecpp(argv[0], *i, cppArgs);
        FILE* cppHandle = icecpp.preprocess(false);

        if(cppHandle == 0)
        {
            return EXIT_FAILURE;
        }

        if(preprocess)
        {
            char buf[4096];
            while(fgets(buf, static_cast<int>(sizeof(buf)), cppHandle) != NULL)
            {
                if(fputs(buf, stdout) == EOF)
                {
                    return EXIT_FAILURE;
                }
            }
            if(!icecpp.close())
            {
                return EXIT_FAILURE;
            }
        }
        else
        {
            UnitPtr u = Unit::createUnit(false, all, ice, caseSensitive);
            int parseStatus = u->parse(cppHandle, debug);

            if(!icecpp.close())
            {
                u->destroy();
                return EXIT_FAILURE;
            }

            if(parseStatus == EXIT_FAILURE)
            {
                status = EXIT_FAILURE;
            }
            else
            {
                string base = icecpp.getBaseName();
                string::size_type pos = base.find_last_of("/\\");
                if(pos != string::npos)
                {
                    base.erase(0, pos + 1);
                }

                //
                // Append the suffix "_ice" to the filename in order to avoid any conflicts
                // with Slice module names. For example, if the file Test.ice defines a
                // Slice module named "Test", then we couldn't create a Python package named
                // "Test" and also call the generated file "Test.py".
                //
                string file = prefix + base + "_ice.py";
                if(!output.empty())
                {
                    file = output + '/' + file;
                }

                IceUtil::Output out;
                out.open(file.c_str());
                if(!out)
                {
                    cerr << argv[0] << ": can't open `" << file << "' for writing" << endl;
                    u->destroy();
                    return EXIT_FAILURE;
                }

                printHeader(out);
                out << "\n# Generated from file `" << base << ".ice'\n";

                //
                // Generate the Python mapping.
                //
                generate(u, all, checksum, includePaths, out);

                //
                // Create or update the Python package hierarchy.
                //
                if(!noPackage)
                {
                    PackageVisitor visitor(argv[0], prefix + base + "_ice", output);
                    u->visit(&visitor, false);
                }
            }

            u->destroy();
        }
    }

    return status;
}
