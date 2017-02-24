
#include <Core.ice>

module User
{

class UserInfo
{
};

interface Registry
{
	UserInfo getUserInfo(string id) throws Core::ArgumentException;
};


};