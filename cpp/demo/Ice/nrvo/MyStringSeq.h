#ifndef MY_STRING_SEQ
#define MY_STRING_SEQ

#include <vector>
#include <string>
#include <iostream>

class MyStringSeq : public std::vector<std::string>
{
public:

    MyStringSeq()
    {}

    MyStringSeq(size_t n) : 
        std::vector<std::string>(n)
    {
    }

    MyStringSeq(size_t n, const std::string& str) : 
        std::vector<std::string>(n, str)
    {
    }

    MyStringSeq(const MyStringSeq& seq) : 
        std::vector<std::string>(seq)
    {
        std::cout << "MyStringSeq copy ctor" << std::endl;
    }
};

#endif
