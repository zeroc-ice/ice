// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <IceUtil/Config.h>

namespace Slice
{

class Preprocessor
{
public:

    Preprocessor(const std::string&, const std::string&, const std::string&);
    ~Preprocessor();
    
    FILE* preprocess(bool);
    bool close();

    void printMakefileDependencies();
    
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
