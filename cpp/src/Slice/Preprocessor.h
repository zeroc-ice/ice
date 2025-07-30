// Copyright (c) ZeroC, Inc.

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <memory>
#include <string>
#include <vector>

namespace Slice
{
    class Preprocessor;
    using PreprocessorPtr = std::shared_ptr<Preprocessor>;

    class Preprocessor final
    {
    public:
        static PreprocessorPtr
        create(const std::string& path, const std::string& fileName, const std::vector<std::string>& args);

        Preprocessor(std::string path, const std::string& fileName, const std::vector<std::string>& args);
        ~Preprocessor();

        FILE* preprocess(const std::string& languageArg = "");
        void close();

        std::string getFileName();
        std::string getBaseName();

        static std::string normalizeIncludePath(const std::string& path);

    private:
        void checkInputFile();

        const std::string _path;
        const std::string _fileName;
        const std::string _shortFileName;
        const std::vector<std::string> _args;
        std::string _cppFile;
        FILE* _cppHandle{nullptr};
    };
}

#endif
