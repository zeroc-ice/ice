#ifndef ICE_TRAITS_H
#define ICE_TRAITS_H

#ifdef ICE_CPP11_MAPPING

#include <type_traits>

namespace Ice
{

class ObjectPrx;
class Value;

template<class T> using IsProxy = ::std::is_base_of<::Ice::ObjectPrx, T>;
template<class T> using IsValue = ::std::is_base_of<::Ice::Value, T>;

enum class Kind { Value, Proxy };

template<typename T>
struct TypeTraits
{
    constexpr static bool isKind(Kind kind)
    {
        switch(kind)
        {
            case Kind::Value:
            {
                return std::is_base_of<Value, T>::value;
            }
            case Kind::Proxy:
            {
                return std::is_base_of<ObjectPrx, T>::value;
            }
            default:
            {
                return false;
            }
        }
    }
};

}
#endif

#endif
