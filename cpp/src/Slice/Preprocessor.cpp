// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Slice/Preprocessor.h>
#include <Slice/Util.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/FileUtil.h>
#include <IceUtil/UUID.h>
#include <IceUtil/Unicode.h>
#include <algorithm>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>

#ifndef _WIN32
#   include <sys/wait.h>
#endif

using namespace std;
using namespace Slice;

//
// mcpp defines
//
namespace Slice
{

enum Outdest
{
    Out=0, Err=1, Dbg=2, Num_Outdest=3
};

}

extern "C" int   mcpp_lib_main(int argc, char** argv);
extern "C" void  mcpp_use_mem_buffers(int tf);
extern "C" char* mcpp_get_mem_buffer(Outdest od);

Slice::PreprocessorPtr
Slice::Preprocessor::create(const string& path, const string& fileName, const vector<string>& args)
{
    return new Preprocessor(path, fileName, args);
}

Slice::Preprocessor::Preprocessor(const string& path, const string& fileName, const vector<string>& args) :
    _path(path),
    _fileName(fullPath(fileName)),
    _shortFileName(fileName),
    _args(args),
    _cppHandle(0)
{
}

Slice::Preprocessor::~Preprocessor()
{
    close();
}

string
Slice::Preprocessor::getBaseName()
{
    string base(_fileName);
    string suffix;
    string::size_type pos = base.rfind('.');
    if(pos != string::npos)
    {
        base.erase(pos);
    }
    return base;
}

string
Slice::Preprocessor::addQuotes(const string& arg)
{
    //
    // Add quotes around the given argument to ensure that arguments
    // with spaces will be preserved as a single argument. We also
    // escape the "\" character to ensure that we don't end up with a
    // \" at the end of the string.
    //
    return "\"" + IceUtilInternal::escapeString(arg, "\\") + "\"";
}

string
Slice::Preprocessor::normalizeIncludePath(const string& path)
{
    string result = path;

#ifdef _WIN32
    //
    // MCPP does not handle "-IC:/" well as an include path.
    //
    if(path.size() != 3 || !(path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z') ||
       path[1] != ':' || path[2] != '\\')
#endif
    {
        replace(result.begin(), result.end(), '\\', '/');
    }

    string::size_type startReplace = 0;
#ifdef _WIN32
    //
    // For UNC paths we need to ensure they are in the format that is
    // used by MCPP. IE. "//MACHINE/PATH"
    //
    if(result.find("//") == 0)
    {
        startReplace = 2;
    }
#endif
    string::size_type pos;
    while((pos = result.find("//", startReplace)) != string::npos)
    {
        result.replace(pos, 2, "/");
    }

    if(result == "/" || (result.size() == 3 && IceUtilInternal::isAlpha(result[0]) && result[1] == ':' && 
                         result[2] == '/'))
    {
        return result;
    }

    if(result.size() > 1 && result[result.size() - 1] == '/')
    {
        result.erase(result.size() - 1);
    }

    return result;
}

namespace
{

vector<string>
baseArgs(vector<string> args, bool keepComments, const string& extraArgs, const string& fileName)
{
    if(keepComments)
    {
        args.push_back("-C");
    }
    args.push_back("-e");
    args.push_back("en_us.utf8");
    
    //
    // Define version macros __ICE_VERSION__ is preferred. We keep
    // ICE_VERSION for backward compatibility with 3.5.0.
    //
    const string version[2] = {"ICE_VERSION", "__ICE_VERSION__"};
    for(int i = 0; i < 2; ++i)
    {
        ostringstream os;
        os << "-D" << version[i] << "=" << ICE_INT_VERSION;
        args.push_back(os.str());
    }
    
    if(!extraArgs.empty())
    {
        args.push_back(extraArgs);
    }
    args.push_back(fileName);
    return args;
}

}

FILE*
Slice::Preprocessor::preprocess(bool keepComments, const string& extraArgs)
{
    if(!checkInputFile())
    {
        return 0;
    }

    //
    // Build arguments list.
    //
    vector<string> args = baseArgs(_args, keepComments, extraArgs, _fileName);
    const char** argv = new const char*[args.size() + 1];
    argv[0] = "mcpp";
    for(unsigned int i = 0; i < args.size(); ++i)
    {
        argv[i + 1] = args[i].c_str();
    }

    //
    // Call mcpp using memory buffer.
    //
    mcpp_use_mem_buffers(1);
    int status = mcpp_lib_main(static_cast<int>(args.size()) + 1, const_cast<char**>(argv));
    delete[] argv;

    //
    // Display any errors.
    //
    char* err = mcpp_get_mem_buffer(Err);
    if(err)
    {
        vector<string> messages = filterMcppWarnings(err);
        for(vector<string>::const_iterator i = messages.begin(); i != messages.end(); ++i)
        {
            emitRaw(i->c_str());

            //
            // MCPP FIX: mcpp does not always return non-zero exit status when there is an error.
            //
            if(i->find("error:") != string::npos)
            {
                status = 1;
            }
        }
    }

    if(status == 0)
    {
        //
        // Write output to temporary file.
        //
        char* buf = mcpp_get_mem_buffer(Out);

        //
        // First try to open temporay file in tmp directory.
        //
#ifdef _WIN32
        wchar_t* name = _wtempnam(NULL, L".preprocess");
        if(name)
        {
            _cppFile = IceUtil::wstringToString(name);
            free(name);
            _cppHandle = IceUtilInternal::fopen(_cppFile, "w+");
        }
#else
        _cppHandle = tmpfile();
#endif

        //
        // If that fails try to open file in current directory.
        //
        if(_cppHandle == 0)
        {
            _cppFile = ".preprocess." + IceUtil::generateUUID();
            _cppHandle = IceUtilInternal::fopen(_cppFile, "w+");
        }

        if(_cppHandle != 0)
        {
            if(buf)
            {
                ::fwrite(buf, strlen(buf), 1, _cppHandle);
            }
            ::rewind(_cppHandle);
        }
        else
        {
            ostream& os = getErrorStream();
            os << _path << ": error: could not open temporary file: ";
            os << _cppFile;
            os << endl;
        }
    }

    //
    // Calling this again causes the memory buffers to be freed.
    //
    mcpp_use_mem_buffers(1);

    return _cppHandle;
}

bool
Slice::Preprocessor::printMakefileDependencies(Language lang, const vector<string>& includePaths,
                                               const std::string& extraArgs, const string& cppSourceExt,
                                               const string& optValue)
{
    if(!checkInputFile())
    {
        return false;
    }
    
    string cppHeaderExt;
    string pyPrefix;
    if(lang == CPlusPlus)
    {
        cppHeaderExt = optValue;
    }
    else if(lang == Python)
    {
        pyPrefix = optValue;
    }

    //
    // Build arguments list.
    //
    vector<string> args = _args;
    args.push_back("-M");
    args = baseArgs(args, false, extraArgs, _fileName);
   
    const char** argv = new const char*[args.size() + 1];
    for(unsigned int i = 0; i < args.size(); ++i)
    {
        argv[i + 1] = args[i].c_str();
    }

    //
    // Call mcpp using memory buffer.
    //
    mcpp_use_mem_buffers(1);
    int status = mcpp_lib_main(static_cast<int>(args.size() + 1), const_cast<char**>(argv));
    delete[] argv;

    //
    // Print errors to stderr.
    //
    char* err = mcpp_get_mem_buffer(Err);
    if(err)
    {
        vector<string> messages = filterMcppWarnings(err);
        for(vector<string>::const_iterator i = messages.begin(); i != messages.end(); ++i)
        {
            emitRaw(i->c_str());
        }
    }

    if(status != 0)
    {
        //
        // Calling this again causes the memory buffers to be freed.
        //
        mcpp_use_mem_buffers(1);
        return false;
    }

    //
    // Get mcpp output.
    //
    string unprocessed;
    char* buf = mcpp_get_mem_buffer(Out);
    if(buf)
    {
        unprocessed = string(buf);
    }

    //
    // Calling this again causes the memory buffers to be freed.
    //
    mcpp_use_mem_buffers(1);

    //
    // We now need to massage the result to get the desired output.
    // First make it a single line.
    //
    string::size_type pos;
    while((pos = unprocessed.find("\\\n")) != string::npos)
    {
        unprocessed.replace(pos, 2, "");
    }

    //
    // Get the main output file name.
    //
#ifdef _WIN32
     string suffix = ".obj:";
#else
     string suffix = ".o:";
#endif
    pos = unprocessed.find(suffix) + suffix.size();
    string result;
    if(lang != JavaXML)
    {
        result = unprocessed.substr(0, pos);
    }

    vector<string> fullIncludePaths;

    for(vector<string>::const_iterator p = includePaths.begin(); p != includePaths.end(); ++p)
    {
        fullIncludePaths.push_back(fullPath(*p));
    }

    //
    // Process each dependency.
    //
    string::size_type end;
    while((end = unprocessed.find(".ice", pos)) != string::npos)
    {
        end += 4;
        string file = IceUtilInternal::trim(unprocessed.substr(pos, end - pos));
        if(IceUtilInternal::isAbsolutePath(file))
        {
            if(file == _fileName)
            {
                file = _shortFileName;
            }
            else
            {
                //
                // Transform back full paths generated by mcpp to paths relative to the specified
                // include paths.
                //
                string newFile = file;
                for(vector<string>::const_iterator p = fullIncludePaths.begin(); p != fullIncludePaths.end(); ++p)
                {
                    if(file.compare(0, p->length(), *p) == 0)
                    {
                        string s = includePaths[p - fullIncludePaths.begin()] + file.substr(p->length());
                        if(IceUtilInternal::isAbsolutePath(newFile) || s.size() < newFile.size())
                        {
                            newFile = s;
                        }
                    }
                }
                file = newFile;
            }
        }

        if(lang == JavaXML)
        {
            if(result.size() == 0)
            {
                result = "  <source name=\"" + file + "\">";
            }
            else
            {
                result += "\n    <dependsOn name=\"" + file + "\"/>";
            }
        }
        else
        {
            //
            // Escape spaces in the file name.
            //
            string::size_type space = 0;
            while((space = file.find(" ", space)) != string::npos)
            {
                file.replace(space, 1, "\\ ");
                space += 2;
            }

            //
            // Add to result
            //
            result += " \\\n " + file;
        }
        pos = end;
    }
    if(lang == JavaXML)
    {
        result += "\n  </source>\n";
    }
    else
    {
        result += "\n";
    }

    /*
     * Emit dependencies in any of the following formats, depending on the
     * length of the filenames:
     *
     * x.o[bj]: /path/x.ice /path/y.ice
     *
     * x.o[bj]: /path/x.ice \
     *  /path/y.ice
     *
     * x.o[bj]: /path/x.ice /path/y.ice \
     *  /path/z.ice
     *
     * x.o[bj]: \
     *  /path/x.ice
     *
     * x.o[bj]: \
     *  /path/x.ice \
     *  /path/y.ice
     *
     * Spaces embedded within filenames are escaped with a backslash. Note that
     * Windows filenames may contain colons.
     *
     */
    switch(lang)
    {
        case CPlusPlus:
        {
            //
            // Change .o[bj] suffix to the cpp source extension suffix.
            //
            string::size_type pos = result.find(suffix);
            if(pos != string::npos)
            {
                string name = result.substr(0, pos);
                result.replace(0, pos + suffix.size() - 1, name + "." + cppHeaderExt + " " + name + "." + cppSourceExt);
            }
            break;
        }
        case JavaXML:
            break;
        case Java:
        {
            //
            // We want to shift the files left one position, so that
            // "x.cpp: x.ice y.ice" becomes "x.ice: y.ice".
            //

            //
            // Remove the first file.
            //
            string::size_type start = result.find(suffix);
            assert(start != string::npos);
            start = result.find_first_not_of(" \t\r\n\\", start + suffix.size()); // Skip to beginning of next file.
            assert(start != string::npos);
            result.erase(0, start);

            //
            // Find end of next file.
            //
            string::size_type pos = 0;
            while((pos = result.find_first_of(" :\t\r\n\\", pos + 1)) != string::npos)
            {
                if(result[pos] == ':')
                {
                    result.insert(pos, 1, '\\'); // Escape colons.
                    ++pos;
                }
                else if(result[pos] == '\\') // Ignore escaped characters.
                {
                    ++pos;
                }
                else
                {
                    break;
                }
            }

            if(pos == string::npos)
            {
                result.append(":");
            }
            else
            {
                result.insert(pos, 1, ':');
            }
            break;
        }
        case CSharp:
        {
            //
            // Change .o[bj] suffix to .cs suffix.
            //
            string::size_type pos;
            if((pos = result.find(suffix)) != string::npos)
            {
                result.replace(pos, suffix.size() - 1, ".cs");
            }
            break;
        }
        case JS:
        {
            //
            // Change .o[bj] suffix to .js suffix.
            //
            string::size_type pos;
            if((pos = result.find(suffix)) != string::npos)
            {
                result.replace(pos, suffix.size() - 1, ".js");
            }
            break;
        }
        case Python:
        {
            //
            // Change .o[bj] suffix to .py suffix.
            //
            if(pyPrefix.size() != 0)
            {
                result = pyPrefix + result;
            }
            string::size_type pos;
            if((pos = result.find(suffix)) != string::npos)
            {
                result.replace(pos, suffix.size() - 1, "_ice.py");
            }
            break;
        }
        case Ruby:
        {
            //
            // Change .o[bj] suffix to .rb suffix.
            //
            string::size_type pos;
            if((pos = result.find(suffix)) != string::npos)
            {
                result.replace(pos, suffix.size() - 1, ".rb");
            }
            break;
        }
        case PHP:
        {
            //
            // Change .o[bj] suffix to .php suffix.
            //
            string::size_type pos;
            if((pos = result.find(suffix)) != string::npos)
            {
                result.replace(pos, suffix.size() - 1, ".php");
            }
            break;
        }
        default:
        {
            abort();
            break;
        }
    }

    //
    // Output result
    //
    fputs(result.c_str(), stdout);
    return true;
}

bool
Slice::Preprocessor::close()
{
    if(_cppHandle != 0)
    {
        int status = fclose(_cppHandle);
        _cppHandle = 0;

        if(_cppFile.size() != 0)
        {
            IceUtilInternal::unlink(_cppFile);
        }

        if(status != 0)
        {
            return false;
        }
    }

    return true;
}

bool
Slice::Preprocessor::checkInputFile()
{
    string base(_fileName);
    string suffix;
    string::size_type pos = base.rfind('.');
    if(pos != string::npos)
    {
        suffix = IceUtilInternal::toLower(base.substr(pos));
    }
    if(suffix != ".ice")
    {
        getErrorStream() << _path << ": error: input files must end with `.ice'" << endl;
        return false;
    }

    ifstream test(_fileName.c_str());
    if(!test)
    {
        getErrorStream() << _path << ": error: cannot open `" << _fileName << "' for reading" << endl;
        return false;
    }
    test.close();

    return true;
}
