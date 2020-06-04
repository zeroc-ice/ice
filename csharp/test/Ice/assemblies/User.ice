
#include <Core.ice>

[[3.7]]

[[suppress-warning:reserved-identifier]]

module ZeroC::Ice::Test::Assemblies::User
{
    class UserInfo
    {
    }

    interface Registry
    {
        UserInfo getUserInfo(string id, ZeroC::Ice::Test::Assemblies::Core::Data data);
    }
}
