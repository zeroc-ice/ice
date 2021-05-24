// This file is used to check if the C++ compiler version is greater than 199711L (c++98)
// in which case we don't have to set -std=c++11 when building the C++11 mapping
int main()
{
#if !defined(__cplusplus) || (__cplusplus == 199711L)
#    error "c++98 mode"
#endif
    return 0;
}
