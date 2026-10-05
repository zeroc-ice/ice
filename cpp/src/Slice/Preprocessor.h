// Copyright (c) ZeroC, Inc.

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <cstdio>
#include <string>
#include <vector>

namespace Slice
{
    class Preprocessor final
    {
    public:
        Preprocessor(const std::string& fileName, const std::vector<std::string>& args);
        ~Preprocessor();

        Preprocessor(const Preprocessor&) = delete;
        Preprocessor& operator=(const Preprocessor&) = delete;

        FILE* preprocess(const std::string& languageArg = "");

        std::string getBaseName();

        static std::string normalizeIncludePath(const std::string& path);

    private:
        void checkInputFile();

        const std::string _fileName;
        const std::vector<std::string> _args;
        FILE* _cppHandle{nullptr};
    };
}

#endif
