// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <IceUtil/Config.h>

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

    Preprocessor(const std::string&, const std::string&, const std::string&);
    ~Preprocessor();
    
    FILE* preprocess(bool);
    bool close();

    enum Language { CPlusPlus, Java, CSharp, VisualBasic };

    void printMakefileDependencies(Language);
    
    std::string getBaseName();

private:
    
    bool checkInputFile();
    std::string searchIceCpp();

    const std::string _path;
    const std::string _fileName;
    const std::string _args;
    FILE* _cppHandle;
};

}

#endif
