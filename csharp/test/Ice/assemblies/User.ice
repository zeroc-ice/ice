
#include <Core.ice>

[[3.7]]

module User
{
    class UserInfo
    {
    }

    interface Registry
    {
        UserInfo getUserInfo(string id, Core::Data data);
    }
}
