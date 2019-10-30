//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <vector>

namespace Slice
{

class Preprocessor;
typedef IceUtil::Handle<Preprocessor> PreprocessorPtr;

class Preprocessor : public IceUtil::SimpleShared
{
public:

    static PreprocessorPtr create(const std::string&, const std::string&, const std::vector<std::string>&);

    ~Preprocessor();

    FILE* preprocess(bool, const std::string& = "");
    FILE* preprocess(bool, const std::vector<std::string>&);
    bool close();

    enum Language { CPlusPlus, Java, CSharp, Python, JavaScript, JavaScriptJSON, SliceXML, MATLAB, Swift };

    bool printMakefileDependencies(std::ostream&, Language, const std::vector<std::string>&, const std::string& = "",
                                   const std::string& = "cpp", const std::string& = "");
    bool printMakefileDependencies(std::ostream&, Language, const std::vector<std::string>&,
                                   const std::vector<std::string>&, const std::string& = "cpp",
                                   const std::string& = "");

    std::string getFileName();
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
