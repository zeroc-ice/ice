
#include <Core.ice>

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
