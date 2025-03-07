    
#include "Core.ice"
    
module User
{
    class UserInfo
    {
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface Registry
    {
        UserInfo getUserInfo(string id) throws Core::ArgumentException;
    }
}
