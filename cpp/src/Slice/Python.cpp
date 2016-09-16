// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/IceUtil.h>
#include <IceUtil/Options.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/CtrlCHandler.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
// BUGFIX: With MSVC2013 if this isn't included you get strange linker errors.
#include <IceUtil/FileUtil.h>
#include <Slice/Preprocessor.h>
#include <Slice/FileTracker.h>
#include <Slice/PythonUtil.h>
#include <Slice/Util.h>
#include <cstring>

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

namespace
{

IceUtil::Mutex* globalMutex = 0;
bool interrupted = false;

class Init
{
public:

    Init()
    {
        globalMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete globalMutex;
        globalMutex = 0;
    }
};

Init init;

void
interruptedCallback(int /*signal*/)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(globalMutex);

    interrupted = true;
}


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

    static void createModules(const UnitPtr&, const string&, const string&);

    virtual void visitModuleEnd(const ModulePtr&);

private:

    PackageVisitor(StringList&);

    enum ReadState { PreModules, InModules, InSubmodules };

    static const char* _moduleTag;
    static const char* _submoduleTag;

    static void createDirectory(const string&);

    static void addModule(const string&, const string&, const string&);
    static void addSubmodule(const string&, const string&, const string&);

    static void readInit(const string&, StringList&, StringList&);
    static void writeInit(const string&, const string&, const StringList&, const StringList&);

    StringList& _modules;
};

const char* PackageVisitor::_moduleTag = "# Modules:";
const char* PackageVisitor::_submoduleTag = "# Submodules:";

PackageVisitor::PackageVisitor(StringList& modules) :
    _modules(modules)
{
}

void
PackageVisitor::createModules(const UnitPtr& unit, const string& module, const string& dir)
{
    StringList modules;
    PackageVisitor v(modules);
    unit->visit(&v, false);

    for(StringList::iterator p = modules.begin(); p != modules.end(); ++p)
    {
        vector<string> v;
        if(!IceUtilInternal::splitString(*p, ".", v))
        {
            assert(false);
        }
        string currentModule;
        string path = dir.empty() ? "." : dir;
        for(vector<string>::iterator q = v.begin(); q != v.end(); ++q)
        {
            if(q != v.begin())
            {
                addSubmodule(path, currentModule, *q);
                currentModule += ".";
            }

            currentModule += *q;
            path += "/" + *q;
            createDirectory(path);

            addModule(path, currentModule, module);
        }
    }
}

void
PackageVisitor::visitModuleEnd(const ModulePtr& p)
{
    //
    // Collect the most deeply-nested modules. For example, if we have a
    // module named M.N.O, then we don't need to keep M or M.N in the list.
    //
    string abs = getAbsolute(p);
    if(find(_modules.begin(), _modules.end(), abs) == _modules.end())
    {
        _modules.push_back(abs);
    }
    string::size_type pos = abs.rfind('.');
    if(pos != string::npos)
    {
        string parent = abs.substr(0, pos);
        _modules.remove(parent);
    }
}

void
PackageVisitor::createDirectory(const string& dir)
{
    IceUtilInternal::structstat st;
    if(!IceUtilInternal::stat(dir, &st))
    {
        if(!(st.st_mode & S_IFDIR))
        {
            ostringstream os;
            os << "failed to create package directory `" << dir
               << "': file already exists and is not a directory";
            throw FileException(__FILE__, __LINE__, os.str());
        }
        return;
    }

    if(IceUtilInternal::mkdir(dir, 0777) != 0)
    {
        ostringstream os;
        os << "cannot create directory `" << dir << "': " << strerror(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }

    FileTracker::instance()->addDirectory(dir);
}

void
PackageVisitor::addModule(const string& dir, const string& module, const string& name)
{
    //
    // Add a module to the set of imported modules in __init__.py.
    //
    StringList modules, submodules;
    readInit(dir, modules, submodules);
    StringList::iterator p = find(modules.begin(), modules.end(), name);
    if(p == modules.end())
    {
        modules.push_back(name);
        writeInit(dir, module, modules, submodules);
    }
}

void
PackageVisitor::addSubmodule(const string& dir, const string& module, const string& name)
{
    //
    // Add a submodule to the set of imported modules in __init__.py.
    //
    StringList modules, submodules;
    readInit(dir, modules, submodules);
    StringList::iterator p = find(submodules.begin(), submodules.end(), name);
    if(p == submodules.end())
    {
        submodules.push_back(name);
        writeInit(dir, module, modules, submodules);
    }
}

void
PackageVisitor::readInit(const string& dir, StringList& modules, StringList& submodules)
{
    string initPath = dir + "/__init__.py";

    IceUtilInternal::structstat st;
    if(!IceUtilInternal::stat(initPath, &st))
    {
        IceUtilInternal::ifstream in(initPath);
        if(!in)
        {
            ostringstream os;
            os << "cannot open file `" << initPath << "': " << strerror(errno);
            throw FileException(__FILE__, __LINE__, os.str());
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
                    continue;
                }

                if(s.size() < 8)
                {
                    ostringstream os;
                    os << "invalid line `" << s << "' in `" << initPath << "'";
                    throw os.str();
                }

                string name = s.substr(7);
                if(state == InModules)
                {
                    modules.push_back(name);
                }
                else
                {
                    //
                    // This case occurs in old (Ice <= 3.5.1) code that used implicit
                    // relative imports, such as:
                    //
                    // File: outer/__init__.py
                    //
                    // import inner
                    //
                    // These aren't supported in Python 3. We'll translate these into
                    // explicit relative imports:
                    //
                    // from . import inner
                    //
                    submodules.push_back(name);
                }
            }
            else if(s.find("from . import") == 0)
            {
                if(state != InSubmodules)
                {
                    ostringstream os;
                    os << "invalid line `" << s << "' in `" << initPath << "'";
                    throw os.str();
                }

                if(s.size() < 15)
                {
                    ostringstream os;
                    os << "invalid line `" << s << "' in `" << initPath << "'";
                    throw os.str();
                }

                submodules.push_back(s.substr(14));
            }
        }

        if(state != InSubmodules)
        {
            ostringstream os;
            os << "invalid format in `" << initPath << "'" << endl;
            throw os.str();
        }
    }
}

void
PackageVisitor::writeInit(const string& dir, const string& name, const StringList& modules,
                          const StringList& submodules)
{
    string initPath = dir + "/__init__.py";

    IceUtilInternal::ofstream os(initPath);
    if(!os)
    {
        ostringstream os;
        os << "cannot open file `" << initPath << "': " << strerror(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addFile(initPath);

    os << "# Generated by slice2py - DO NOT EDIT!" << endl
       << "#" << endl;
    os << endl
       << "import Ice" << endl
       << "Ice.updateModule(\"" << name << "\")" << endl
       << endl;
    os << _moduleTag << endl;
    for(StringList::const_iterator p = modules.begin(); p != modules.end(); ++p)
    {
        os << "import " << *p << endl;
    }

    os << endl;
    os << _submoduleTag << endl;
    for(StringList::const_iterator p = submodules.begin(); p != submodules.end(); ++p)
    {
        os << "from . import " << *p << endl;
    }
}

void
usage(const string& n)
{
    getErrorStream() << "Usage: " << n << " [options] slice-files...\n";
    getErrorStream() <<
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        "-DNAME               Define NAME as 1.\n"
        "-DNAME=DEF           Define NAME as DEF.\n"
        "-UNAME               Remove any definition for NAME.\n"
        "-IDIR                Put DIR in the include file search path.\n"
        "-E                   Print preprocessor output on stdout.\n"
        "--output-dir DIR     Create files in the directory DIR.\n"
        "--depend             Generate Makefile dependencies.\n"
        "--depend-xml         Generate dependencies in XML format.\n"
        "--depend-file FILE   Write dependencies to FILE instead of standard output.\n"
        "-d, --debug          Print debug messages.\n"
        "--ice                Permit `Ice' prefix (for building Ice source code only).\n"
        "--underscore         Permit underscores in Slice identifiers.\n"
        "--all                Generate code for Slice definitions in included files.\n"
        "--checksum           Generate checksums for Slice definitions.\n"
        "--prefix PREFIX      Prepend filenames of Python modules with PREFIX.\n"
        ;
}

}

int
Slice::Python::compile(const vector<string>& argv)
{
    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("D", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("U", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("I", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("E");
    opts.addOpt("", "output-dir", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "depend");
    opts.addOpt("", "depend-xml");
    opts.addOpt("", "depend-file", IceUtilInternal::Options::NeedArg, "");
    opts.addOpt("d", "debug");
    opts.addOpt("", "ice");
    opts.addOpt("", "underscore");
    opts.addOpt("", "all");
    opts.addOpt("", "no-package");
    opts.addOpt("", "build-package");
    opts.addOpt("", "checksum");
    opts.addOpt("", "prefix", IceUtilInternal::Options::NeedArg);

    vector<string> args;
    try
    {
        args = opts.parse(argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        getErrorStream() << argv[0] << ": error: " << e.reason << endl;
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
        getErrorStream() << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    vector<string> cppArgs;
    vector<string> optargs = opts.argVec("D");
    for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs.push_back("-D" + *i);
    }

    optargs = opts.argVec("U");
    for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs.push_back("-U" + *i);
    }

    vector<string> includePaths = opts.argVec("I");
    for(vector<string>::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
    {
        cppArgs.push_back("-I" + Preprocessor::normalizeIncludePath(*i));
    }

    bool preprocess = opts.isSet("E");

    string output = opts.optArg("output-dir");

    bool depend = opts.isSet("depend");

    bool dependxml = opts.isSet("depend-xml");

    string dependFile = opts.optArg("depend-file");

    bool debug = opts.isSet("debug");

    bool ice = opts.isSet("ice");

    bool underscore = opts.isSet("underscore");

    bool all = opts.isSet("all");

    bool noPackage = opts.isSet("no-package");

    bool buildPackage = opts.isSet("build-package");

    bool checksum = opts.isSet("checksum");

    string prefix = opts.optArg("prefix");

    if(args.empty())
    {
        getErrorStream() << argv[0] << ": error: no input file" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if(depend && dependxml)
    {
        getErrorStream() << argv[0] << ": error: cannot specify both --depend and --dependxml" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if(noPackage && buildPackage)
    {
        getErrorStream() << argv[0] << ": error: cannot specify both --no-package and --build-package" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    int status = EXIT_SUCCESS;

    IceUtil::CtrlCHandler ctrlCHandler;
    ctrlCHandler.setCallback(interruptedCallback);

    bool keepComments = true;

    DependOutputUtil out(dependFile);
    if(dependxml)
    {
        out.os() << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dependencies>" << endl;
    }

    for(vector<string>::const_iterator i = args.begin(); i != args.end(); ++i)
    {
        //
        // Ignore duplicates.
        //
        vector<string>::iterator p = find(args.begin(), args.end(), *i);
        if(p != i)
        {
            continue;
        }

        if(depend || dependxml)
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2PY__");

            if(cppHandle == 0)
            {
                out.cleanup();
                return EXIT_FAILURE;
            }

            UnitPtr u = Unit::createUnit(false, false, ice, underscore);
            int parseStatus = u->parse(*i, cppHandle, debug);
            u->destroy();

            if(parseStatus == EXIT_FAILURE)
            {
                out.cleanup();
                return EXIT_FAILURE;
            }

            if(!icecpp->printMakefileDependencies(out.os(), depend ? Preprocessor::Python : Preprocessor::SliceXML,
                                                  includePaths, "-D__SLICE2PY__", "", prefix))
            {
                out.cleanup();
                return EXIT_FAILURE;
            }

            if(!icecpp->close())
            {
                out.cleanup();
                return EXIT_FAILURE;
            }
        }
        else
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(keepComments, "-D__SLICE2PY__");

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
                if(!icecpp->close())
                {
                    return EXIT_FAILURE;
                }
            }
            else
            {
                UnitPtr u = Unit::createUnit(false, all, ice, underscore);
                int parseStatus = u->parse(*i, cppHandle, debug);

                if(!icecpp->close())
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
                    try
                    {
                        string base = icecpp->getBaseName();
                        string::size_type pos = base.find_last_of("/\\");
                        if(pos != string::npos)
                        {
                            base.erase(0, pos + 1);
                        }

                        //
                        // If --build-package is specified, we don't generate any code and simply
                        // update the __init__.py files.
                        //
                        if(!buildPackage)
                        {
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

                            IceUtilInternal::Output out;
                            out.open(file.c_str());
                            if(!out)
                            {
                                ostringstream os;
                                os << "cannot open`" << file << "': " << strerror(errno);
                                throw FileException(__FILE__, __LINE__, os.str());
                            }
                            FileTracker::instance()->addFile(file);

                            //
                            // Python magic comment to set the file encoding, it must be first or second line
                            //
                            out << "# -*- coding: utf-8 -*-\n";
                            printHeader(out);
                            printGeneratedHeader(out, base + ".ice", "#");
                            //
                            // Generate the Python mapping.
                            //
                            generate(u, all, checksum, includePaths, out);

                            out.close();
                        }

                        //
                        // Create or update the Python package hierarchy.
                        //
                        if(!noPackage)
                        {
                            PackageVisitor::createModules(u, prefix + base + "_ice", output);
                        }
                    }
                    catch(const Slice::FileException& ex)
                    {
                        //
                        // If a file could not be created, then clean up any created files.
                        //
                        FileTracker::instance()->cleanup();
                        u->destroy();
                        getErrorStream() << argv[0] << ": error: " << ex.reason() << endl;
                        return EXIT_FAILURE;
                    }
                    catch(const string& err)
                    {
                        FileTracker::instance()->cleanup();
                        getErrorStream() << argv[0] << ": error: " << err << endl;
                        status = EXIT_FAILURE;
                    }
                }

                u->destroy();
            }
        }

        {
            IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(globalMutex);

            if(interrupted)
            {
                out.cleanup();
                FileTracker::instance()->cleanup();
                return EXIT_FAILURE;
            }
        }
    }

    if(dependxml)
    {
        out.os() << "</dependencies>\n";
    }

    return status;
}
