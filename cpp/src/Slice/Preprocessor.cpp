// Copyright (c) ZeroC, Inc.

#include "Preprocessor.h"
#include "../Ice/ConsoleUtil.h"
#include "../Ice/FileUtil.h"
#include "Ice/StringConverter.h"
#include "Ice/StringUtil.h"
#include "Ice/UUID.h"
#include "Util.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iterator>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#ifndef _WIN32
#    include <sys/wait.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceInternal;

//
// mcpp defines
//
namespace Slice
{
    enum Outdest
    {
        Out = 0,
        Err = 1,
        Dbg = 2,
        Num_Outdest = 3
    };
}

extern "C" int mcpp_lib_main(int argc, char** argv);
extern "C" void mcpp_use_mem_buffers(int tf);
extern "C" char* mcpp_get_mem_buffer(Outdest od);

Slice::PreprocessorPtr
Slice::Preprocessor::create(const string& path, const string& fileName, const vector<string>& args)
{
    return make_shared<Preprocessor>(path, fileName, args);
}

Slice::Preprocessor::Preprocessor(string path, const string& fileName, const vector<string>& args)
    : _path(std::move(path)),
      _fileName(fullPath(fileName)),
      _shortFileName(fileName),
      _args(args)
{
}

Slice::Preprocessor::~Preprocessor() { close(); }

string
Slice::Preprocessor::getFileName()
{
    return _fileName;
}

string
Slice::Preprocessor::getBaseName()
{
    string base(_fileName);
    string suffix;
    string::size_type pos = base.rfind('.');
    if (pos != string::npos)
    {
        base.erase(pos);
    }
    return base;
}

string
Slice::Preprocessor::normalizeIncludePath(const string& path)
{
    string result = path;

#ifdef _WIN32
    //
    // MCPP does not handle "-IC:/" well as an include path.
    //
    if (path.size() != 3 || !(path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z') ||
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
    if (result.find("//") == 0)
    {
        startReplace = 2;
    }
#endif
    string::size_type pos;
    while ((pos = result.find("//", startReplace)) != string::npos)
    {
        result.replace(pos, 2, "/");
    }

    if (result == "/" ||
        (result.size() == 3 && IceInternal::isAlpha(result[0]) && result[1] == ':' && result[2] == '/'))
    {
        return result;
    }

    if (result.size() > 1 && result[result.size() - 1] == '/')
    {
        result.erase(result.size() - 1);
    }

    return result;
}

namespace
{
    vector<string> baseArgs(vector<string> args, const string& languageArg, const string& fileName)
    {
        // -C keep comments
        args.emplace_back("-C");

        // UTF-8 encoding
        args.emplace_back("-e");
        args.emplace_back("en_us.utf8");

        // Define version macros __ICE_VERSION__ is preferred. We keep ICE_VERSION for backward compatibility
        // with 3.5.0.
        const string version[2] = {"ICE_VERSION", "__ICE_VERSION__"};
        for (const auto& i : version)
        {
            ostringstream os;
            os << "-D" << i << "=" << ICE_INT_VERSION;
            args.push_back(os.str());
        }

        args.push_back(languageArg);
        args.push_back(fileName);
        return args;
    }
}

FILE*
Slice::Preprocessor::preprocess(const string& languageArg)
{
    checkInputFile();

    // Build arguments list.
    vector<string> args = baseArgs(_args, languageArg, _fileName);
    const char** argv = new const char*[args.size() + 1];
    argv[0] = "mcpp";
    for (unsigned int i = 0; i < args.size(); ++i)
    {
        argv[i + 1] = args[i].c_str();
    }

    // Call mcpp using memory buffer.
    mcpp_use_mem_buffers(1);
    int status = mcpp_lib_main(static_cast<int>(args.size()) + 1, const_cast<char**>(argv));
    delete[] argv;

    // Display any errors.
    char* err = mcpp_get_mem_buffer(Err);
    if (err)
    {
        vector<string> messages = filterMcppWarnings(err);
        for (const auto& message : messages)
        {
            emitRaw(message.c_str());

            //
            // MCPP FIX: mcpp does not always return non-zero exit status when there is an error.
            //
            if (message.find("error:") != string::npos)
            {
                status = 1;
            }
        }
    }

    if (status == 0)
    {
        //
        // Write output to temporary file.
        //
        char* buf = mcpp_get_mem_buffer(Out);

        //
        // First try to open temporay file in tmp directory.
        //
#ifdef _WIN32
        //
        // We use an unique id as the tmp file name prefix to avoid
        // problems with this code being called concurrently from
        // several processes, otherwise there is a change that two
        // process call _tempnam before any of them call fopen and
        // they will end up using the same tmp file.
        //
        wchar_t* name = _wtempnam(0, Ice::stringToWstring("slice-" + Ice::generateUUID()).c_str());
        if (name)
        {
            _cppFile = Ice::wstringToString(name);
            free(name);
            _cppHandle = IceInternal::fopen(_cppFile, "w+");
        }
#else
        _cppHandle = tmpfile();
#endif

        // If that fails try to open file in current directory.
        if (_cppHandle == nullptr)
        {
#ifdef _WIN32
            _cppFile = "slice-" + Ice::generateUUID();
#else
            _cppFile = ".slice-" + Ice::generateUUID();
#endif
            _cppHandle = IceInternal::fopen(_cppFile, "w+");
        }

        if (_cppHandle != nullptr)
        {
            if (buf)
            {
                [[maybe_unused]] auto _ = ::fwrite(buf, strlen(buf), 1, _cppHandle);
            }
            ::rewind(_cppHandle); // NOLINT(cert-msc24-c,cert-msc33-c)
        }
        else
        {
            // Calling this again causes the memory buffers to be freed.
            mcpp_use_mem_buffers(1);
            throw runtime_error(_path + ": error: could not open temporary file: " + _cppFile);
        }
    }

    // Calling this again causes the memory buffers to be freed.
    mcpp_use_mem_buffers(1);

    return _cppHandle;
}

void
Slice::Preprocessor::close()
{
    if (_cppHandle != nullptr)
    {
        int status = fclose(_cppHandle);
        _cppHandle = nullptr;

        if (_cppFile.size() != 0)
        {
            IceInternal::unlink(_cppFile);
        }

        if (status != 0)
        {
            throw runtime_error("failed to close preprocessor file: " + IceInternal::lastErrorToString());
        }
    }
}

void
Slice::Preprocessor::checkInputFile()
{
    string base(_fileName);
    string suffix;
    string::size_type pos = base.rfind('.');
    if (pos != string::npos)
    {
        suffix = IceInternal::toLower(base.substr(pos));
    }
    if (suffix != ".ice")
    {
        throw runtime_error(_path + ": error: input files must end with `.ice'");
    }

    ifstream test(IceInternal::streamFilename(_fileName).c_str());
    if (!test)
    {
        throw runtime_error(_path + ": error: cannot open '" + _fileName + "' for reading");
    }
    test.close();
}
