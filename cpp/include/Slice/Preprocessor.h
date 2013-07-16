// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <vector>

#ifndef SLICE_API
#   ifdef SLICE_API_EXPORTS
#       define SLICE_API ICE_DECLSPEC_EXPORT
#   else
#       define SLICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace Slice
{

class Preprocessor;
typedef IceUtil::Handle<Preprocessor> PreprocessorPtr;

class SLICE_API Preprocessor : public IceUtil::SimpleShared
{
public:

    static PreprocessorPtr create(const std::string&, const std::string&, const std::vector<std::string>&);

    ~Preprocessor();

    FILE* preprocess(bool, const std::string& = "");
    bool close();

    enum Language { CPlusPlus, Java, JavaXML, CSharp, Python, Ruby, PHP };

    bool printMakefileDependencies(Language, const std::vector<std::string>&, const std::string& = "", const std::string& = "cpp",
                                   const std::string& = "");

    std::string getBaseName();

    static std::string addQuotes(const std::string&);
    static std::string normalizeIncludePath(const std::string&);

private:

    Preprocessor(const std::string&, const std::string&, const std::vector<std::string>&);

    bool checkInputFile();

    const std::string _path;
    const std::string _fileName;
    const std::string _shortFileName;
    const std::vector<std::string> _args;
    std::string _cppFile;
    FILE* _cppHandle;
};

}

#endif
