// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <IceUtil/Config.h>
#include <vector>
#ifdef __BCPLUSPLUS__
#  include <stdio.h>
#endif


#ifndef SLICE_API
#   ifdef SLICE_API_EXPORTS
#       define SLICE_API ICE_DECLSPEC_EXPORT
#   else
#       define SLICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace Slice
{

class SLICE_API Preprocessor
{
public:

    Preprocessor(const std::string&, const std::string&, const std::vector<std::string>&);
    ~Preprocessor();
    
    FILE* preprocess(bool);
    bool close();

    enum Language { CPlusPlus, Java, CSharp, VisualBasic };

    void printMakefileDependencies(Language, const std::vector<std::string>&);
    
    std::string getBaseName();

    static std::string addQuotes(const std::string&);
    static std::string normalizeIncludePath(const std::string&);

private:
    
    bool checkInputFile();

    const std::string _path;
    const std::string _fileName;
    const std::vector<std::string> _args;
#ifdef _WIN32
    std::wstring _cppFile;
#else
    std::string _cppFile;
#endif
    FILE* _cppHandle;
};

}

#endif
