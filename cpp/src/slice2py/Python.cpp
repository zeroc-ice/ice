// Copyright (c) ZeroC, Inc.

#include "../Ice/ConsoleUtil.h"
#include "../Ice/FileUtil.h"
#include "../Ice/Options.h"
#include "../Slice/FileTracker.h"
#include "../Slice/Preprocessor.h"
#include "../Slice/Util.h"
#include "Ice/CtrlCHandler.h"
#include "Ice/StringUtil.h"
#include "PythonUtil.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <mutex>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#    include <direct.h>
#else
#    include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace Slice::Python;
using namespace IceInternal;

namespace
{
    mutex globalMutex;
    bool interrupted = false;

    void interruptedCallback(int /*signal*/)
    {
        lock_guard lock(globalMutex);

        interrupted = true;
    }

    void createDirectory(const string& dir)
    {
        IceInternal::structstat st;
        if (!IceInternal::stat(dir, &st))
        {
            if (!(st.st_mode & S_IFDIR))
            {
                ostringstream os;
                os << "failed to create directory '" << dir << "': file already exists and is not a directory";
                throw FileException(os.str());
            }
            return;
        }

        if (IceInternal::mkdir(dir, 0777) != 0)
        {
            ostringstream os;
            os << "cannot create directory '" << dir << "': " << IceInternal::errorToString(errno);
            throw FileException(os.str());
        }
    }

    //
    // Starting in the directory given by output (can be empty for the CWD), create all necessary subdirectories
    // in the path given by pkgdir.
    //
    void createPackageDirectory(const string& output, const string& pkgdir)
    {
        assert(output.empty() || IceInternal::directoryExists(output));
        assert(!pkgdir.empty());

        vector<string> elements;
        if (!IceInternal::splitString(pkgdir, "/", elements))
        {
            throw FileException("invalid path in '" + pkgdir + "'");
        }

        assert(!elements.empty());

        //
        // Create all necessary subdirectories.
        //
        string path = output;
        for (const auto& element : elements)
        {
            if (!path.empty())
            {
                path += "/";
            }
            path += element;

            IceInternal::structstat st;
            if (IceInternal::stat(path, &st) < 0)
            {
                int err = IceInternal::mkdir(path, 0777);
                // If slice2py is run concurrently, it's possible that another instance of slice2py has already created
                // the directory.
                if (err == 0 || (errno == EEXIST && IceInternal::directoryExists(path)))
                {
                    // Directory successfully created or already exists.
                }
                else
                {
                    ostringstream os;
                    os << "cannot create directory '" << path << "': " << IceInternal::errorToString(errno);
                    throw FileException(os.str());
                }
                FileTracker::instance()->addDirectory(path);
            }
            else if (!(st.st_mode & S_IFDIR))
            {
                ostringstream os;
                os << "failed to create directory '" << path << "': file already exists and is not a directory";
                throw FileException(os.str());
            }

            //
            // It's possible that the pkgdir metadata specified a directory that won't be visited by our
            // PackageVisitor. We need every intermediate subdirectory to have an __init__.py file, which
            // can be empty.
            //
            const string initFile = path + "/__init__.py";
            if (!IceInternal::fileExists(initFile))
            {
                //
                // Create an empty file.
                //
                IceInternal::Output out;
                out.open(initFile.c_str());
                if (!out)
                {
                    ostringstream os;
                    os << "cannot open '" << initFile << "': " << IceInternal::errorToString(errno);
                    throw FileException(os.str());
                }
                FileTracker::instance()->addFile(initFile);
            }
        }
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
    class PackageVisitor final : public ParserVisitor
    {
    public:
        static void createModules(const UnitPtr&, const string&, const string&);

        void visitModuleEnd(const ModulePtr&) final;

    private:
        PackageVisitor(StringList&);

        enum ReadState
        {
            PreModules,
            InModules,
            InSubmodules
        };

        static void addModule(const string&, const string&, const string&);
        static void addSubmodule(const string&, const string&, const string&);

        static void readInit(const string&, StringList&, StringList&);
        static void writeInit(const string&, const string&, const StringList&, const StringList&);

        StringList& _modules;
    };

    const string moduleTag = "# Modules:";       // NOLINT(cert-err58-cpp)
    const string submoduleTag = "# Submodules:"; // NOLINT(cert-err58-cpp)

    PackageVisitor::PackageVisitor(StringList& modules) : _modules(modules) {}

    void PackageVisitor::createModules(const UnitPtr& unt, const string& module, const string& dir)
    {
        StringList modules;
        PackageVisitor v(modules);
        unt->visit(&v);

        for (const auto& p : modules)
        {
            vector<string> vs;
            if (!IceInternal::splitString(p, ".", vs))
            {
                assert(false);
            }
            string currentModule;
            string path = dir.empty() ? "." : dir;
            for (auto q = vs.begin(); q != vs.end(); ++q)
            {
                if (q != vs.begin())
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

    void PackageVisitor::visitModuleEnd(const ModulePtr& p)
    {
        //
        // Collect the most deeply-nested modules. For example, if we have a
        // module named M.N.O, then we don't need to keep M or M.N in the list.
        //
        string abs = getAbsolute(p);
        if (find(_modules.begin(), _modules.end(), abs) == _modules.end())
        {
            _modules.push_back(abs);
        }
        string::size_type pos = abs.rfind('.');
        if (pos != string::npos)
        {
            string parent = abs.substr(0, pos);
            _modules.remove(parent);
        }
    }

    void PackageVisitor::addModule(const string& dir, const string& module, const string& name)
    {
        //
        // Add a module to the set of imported modules in __init__.py.
        //
        StringList modules, submodules;
        readInit(dir, modules, submodules);
        auto p = find(modules.begin(), modules.end(), name);
        if (p == modules.end())
        {
            modules.push_back(name);
            writeInit(dir, module, modules, submodules);
        }
    }

    void PackageVisitor::addSubmodule(const string& dir, const string& module, const string& name)
    {
        //
        // Add a submodule to the set of imported modules in __init__.py.
        //
        StringList modules, submodules;
        readInit(dir, modules, submodules);
        auto p = find(submodules.begin(), submodules.end(), name);
        if (p == submodules.end())
        {
            submodules.push_back(name);
            writeInit(dir, module, modules, submodules);
        }
    }

    void PackageVisitor::readInit(const string& dir, StringList& modules, StringList& submodules)
    {
        string initPath = dir + "/__init__.py";

        IceInternal::structstat st;
        if (!IceInternal::stat(initPath, &st))
        {
            ifstream in(IceInternal::streamFilename(initPath).c_str());
            if (!in)
            {
                ostringstream os;
                os << "cannot open file '" << initPath << "': " << IceInternal::errorToString(errno);
                throw FileException(os.str());
            }

            ReadState state = PreModules;
            char line[1024];
            while (in.getline(line, 1024))
            {
                string s = line;
                if (s.find(moduleTag) == 0)
                {
                    if (state != PreModules)
                    {
                        break;
                    }
                    state = InModules;
                }
                else if (s.find(submoduleTag) == 0)
                {
                    if (state != InModules)
                    {
                        break;
                    }
                    state = InSubmodules;
                }
                else if (s.find("import") == 0)
                {
                    if (state == PreModules)
                    {
                        continue;
                    }

                    if (s.size() < 8)
                    {
                        throw runtime_error("invalid line '" + s + "' in '" + initPath + "'");
                    }

                    string name = s.substr(7);
                    if (state == InModules)
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
                else if (s.find("from . import") == 0)
                {
                    if (state != InSubmodules)
                    {
                        throw runtime_error("invalid line '" + s + "' in '" + initPath + "'");
                    }

                    if (s.size() < 15)
                    {
                        throw runtime_error("invalid line '" + s + "' in '" + initPath + "'");
                    }

                    submodules.push_back(s.substr(14));
                }
            }

            if (state == InModules)
            {
                throw runtime_error("invalid format in '" + initPath + "'\n");
            }
        }
    }

    void PackageVisitor::writeInit(
        const string& dir,
        const string& name,
        const StringList& modules,
        const StringList& submodules)
    {
        string initPath = dir + "/__init__.py";

        ofstream os(IceInternal::streamFilename(initPath).c_str());
        if (!os)
        {
            ostringstream oss;
            oss << "cannot open file '" << initPath << "': " << IceInternal::errorToString(errno);
            throw FileException(oss.str());
        }
        FileTracker::instance()->addFile(initPath);

        os << "# Generated by slice2py - DO NOT EDIT!" << endl << "#" << endl;
        os << endl << "import Ice" << endl << "Ice.updateModule(\"" << name << "\")" << endl << endl;
        os << moduleTag << endl;
        for (const auto& module : modules)
        {
            os << "import " << module << endl;
        }

        os << endl;
        os << submoduleTag << endl;
        for (const auto& submodule : submodules)
        {
            os << "from . import " << submodule << endl;
        }
    }

    void usage(const string& n)
    {
        consoleErr << "Usage: " << n << " [options] slice-files...\n";
        consoleErr << "Options:\n"
                      "-h, --help               Show this message.\n"
                      "-v, --version            Display the Ice version.\n"
                      "-DNAME                   Define NAME as 1.\n"
                      "-DNAME=DEF               Define NAME as DEF.\n"
                      "-UNAME                   Remove any definition for NAME.\n"
                      "-IDIR                    Put DIR in the include file search path.\n"
                      "-E                       Print preprocessor output on stdout.\n"
                      "--output-dir DIR         Create files in the directory DIR.\n"
                      "-d, --debug              Print debug messages.\n"
                      "--depend                 Generate Makefile dependencies.\n"
                      "--depend-xml             Generate dependencies in XML format.\n"
                      "--depend-file FILE       Write dependencies to FILE instead of standard output.\n"
                      "--all                    Generate code for Slice definitions in included files.\n"
                      "--prefix PREFIX          Prepend filenames of Python modules with PREFIX.\n"
                      "--no-package             Do not generate Python package hierarchy.\n"
                      "--build-package          Only generate Python package hierarchy.\n";
    }
}

int
Slice::Python::compile(const vector<string>& argv)
{
    IceInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("D", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("U", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("I", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("E");
    opts.addOpt("", "output-dir", IceInternal::Options::NeedArg);
    opts.addOpt("", "depend");
    opts.addOpt("", "depend-xml");
    opts.addOpt("", "depend-file", IceInternal::Options::NeedArg, "");
    opts.addOpt("d", "debug");
    opts.addOpt("", "all");
    opts.addOpt("", "no-package");
    opts.addOpt("", "build-package");
    opts.addOpt("", "prefix", IceInternal::Options::NeedArg);

    vector<string> args;
    try
    {
        args = opts.parse(argv);
    }
    catch (const IceInternal::BadOptException& e)
    {
        consoleErr << argv[0] << ": error: " << e.what() << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (opts.isSet("help"))
    {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if (opts.isSet("version"))
    {
        consoleErr << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    vector<string> cppArgs;
    vector<string> optargs = opts.argVec("D");
    cppArgs.reserve(optargs.size()); // keep clang-tidy happy
    for (const auto& arg : optargs)
    {
        cppArgs.push_back("-D" + arg);
    }

    optargs = opts.argVec("U");
    for (const auto& arg : optargs)
    {
        cppArgs.push_back("-U" + arg);
    }

    vector<string> includePaths = opts.argVec("I");
    for (const auto& includePath : includePaths)
    {
        cppArgs.push_back("-I" + Preprocessor::normalizeIncludePath(includePath));
    }

    bool preprocess = opts.isSet("E");

    string output = opts.optArg("output-dir");

    bool depend = opts.isSet("depend");

    bool dependxml = opts.isSet("depend-xml");

    string dependFile = opts.optArg("depend-file");

    bool debug = opts.isSet("debug");

    bool all = opts.isSet("all");

    bool noPackage = opts.isSet("no-package");

    bool buildPackage = opts.isSet("build-package");

    string prefix = opts.optArg("prefix");

    if (args.empty())
    {
        consoleErr << argv[0] << ": error: no input file" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (depend && dependxml)
    {
        consoleErr << argv[0] << ": error: cannot specify both --depend and --dependxml" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (noPackage && buildPackage)
    {
        consoleErr << argv[0] << ": error: cannot specify both --no-package and --build-package" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (!output.empty() && !IceInternal::directoryExists(output))
    {
        consoleErr << argv[0] << ": error: argument for --output-dir does not exist or is not a directory" << endl;
        return EXIT_FAILURE;
    }

    int status = EXIT_SUCCESS;

    Ice::CtrlCHandler ctrlCHandler;
    ctrlCHandler.setCallback(interruptedCallback);

    ostringstream os;
    if (dependxml)
    {
        os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dependencies>" << endl;
    }

    for (auto i = args.begin(); i != args.end(); ++i)
    {
        //
        // Ignore duplicates.
        //
        auto p = find(args.begin(), args.end(), *i);
        if (p != i)
        {
            continue;
        }

        if (depend || dependxml)
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2PY__");

            if (cppHandle == nullptr)
            {
                return EXIT_FAILURE;
            }

            UnitPtr u = Unit::createUnit("python", false);
            int parseStatus = u->parse(*i, cppHandle, debug);
            u->destroy();

            if (parseStatus == EXIT_FAILURE)
            {
                return EXIT_FAILURE;
            }

            if (!icecpp->printMakefileDependencies(
                    os,
                    depend ? Preprocessor::Python : Preprocessor::SliceXML,
                    includePaths,
                    "-D__SLICE2PY__",
                    prefix))
            {
                return EXIT_FAILURE;
            }

            if (!icecpp->close())
            {
                return EXIT_FAILURE;
            }
        }
        else
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(true, "-D__SLICE2PY__");

            if (cppHandle == nullptr)
            {
                return EXIT_FAILURE;
            }

            if (preprocess)
            {
                char buf[4096];
                while (fgets(buf, static_cast<int>(sizeof(buf)), cppHandle) != nullptr)
                {
                    if (fputs(buf, stdout) == EOF)
                    {
                        return EXIT_FAILURE;
                    }
                }
                if (!icecpp->close())
                {
                    return EXIT_FAILURE;
                }
            }
            else
            {
                UnitPtr u = Unit::createUnit("python", all);
                int parseStatus = u->parse(*i, cppHandle, debug);

                if (!icecpp->close())
                {
                    u->destroy();
                    return EXIT_FAILURE;
                }

                if (parseStatus == EXIT_FAILURE)
                {
                    status = EXIT_FAILURE;
                }
                else
                {
                    try
                    {
                        string base = icecpp->getBaseName();
                        string::size_type pos = base.find_last_of("/\\");
                        if (pos != string::npos)
                        {
                            base.erase(0, pos + 1);
                        }

                        //
                        // Check if the file contains the python:pkgdir file metadata.
                        //
                        const string pkgdir = getPackageDirectory(icecpp->getFileName(), u);

                        //
                        // If --build-package is specified, we don't generate any code and simply
                        // update the __init__.py files.
                        //
                        if (!buildPackage)
                        {
                            string path;
                            if (!output.empty())
                            {
                                path = output + '/'; // The output directory must already exist.
                            }

                            if (!pkgdir.empty())
                            {
                                //
                                // The metadata is present. It should have the form
                                //
                                // python:pkgdir:A/B/C
                                //
                                // We open the output file in the specified directory, prefixed by the
                                // output directory (if any).
                                //
                                createPackageDirectory(output, pkgdir);
                                path += pkgdir;
                                if (path[path.size() - 1] != '/')
                                {
                                    path += "/"; // Append a separator if necessary.
                                }
                            }
                            else
                            {
                                //
                                // The file doesn't contain the python:pkgdir metadata, so we use the
                                // value of the --prefix option (if any).
                                //
                                path += prefix;
                            }

                            //
                            // Add the file name (without the .ice extension).
                            //
                            path += base;

                            //
                            // Append the suffix "_ice" to the filename in order to avoid any conflicts
                            // with Slice module or type names. For example, if the file Test.ice defines a
                            // Slice module named "Test", then we couldn't create a Python package named
                            // "Test" and also call the generated file "Test.py".
                            //
                            path += "_ice.py";

                            IceInternal::Output out;
                            out.open(path.c_str());
                            if (!out)
                            {
                                ostringstream oss;
                                oss << "cannot open '" << path << "': " << IceInternal::errorToString(errno);
                                throw FileException(oss.str());
                            }
                            FileTracker::instance()->addFile(path);

                            printHeader(out);
                            printGeneratedHeader(out, base + ".ice", "#");
                            out << sp;

                            //
                            // Generate Python code.
                            //
                            generate(u, all, includePaths, out);

                            out.close();
                        }

                        //
                        // Create or update the Python package hierarchy.
                        //
                        if (!noPackage)
                        {
                            string name;
                            if (!pkgdir.empty())
                            {
                                name = getImportFileName(icecpp->getFileName(), u, vector<string>());
                            }
                            else
                            {
                                name = prefix + base + "_ice";
                            }
                            PackageVisitor::createModules(u, name, output);
                        }
                    }
                    catch (const Slice::FileException& ex)
                    {
                        //
                        // If a file could not be created, then clean up any created files.
                        //
                        FileTracker::instance()->cleanup();
                        u->destroy();
                        consoleErr << argv[0] << ": error: " << ex.what() << endl;
                        return EXIT_FAILURE;
                    }
                    catch (const exception& ex)
                    {
                        FileTracker::instance()->cleanup();
                        consoleErr << argv[0] << ": error: " << ex.what() << endl;
                        status = EXIT_FAILURE;
                    }
                }

                status |= u->getStatus();
                u->destroy();
            }
        }

        {
            lock_guard lock(globalMutex);
            if (interrupted)
            {
                FileTracker::instance()->cleanup();
                return EXIT_FAILURE;
            }
        }
    }

    if (dependxml)
    {
        os << "</dependencies>\n";
    }

    if (depend || dependxml)
    {
        writeDependencies(os.str(), dependFile);
    }

    return status;
}
