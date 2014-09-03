
#include <IceSSL/IceSSL.h>

#ifdef ICE_USE_SECURE_TRANSPORT

#include <Security/Security.h>

namespace Test
{

int
getcwd(std::string& cwd)
{
    char cwdbuf[PATH_MAX];
    if(::getcwd(cwdbuf, PATH_MAX) == NULL)
    {
        return -1;
    }
    cwd = cwdbuf;
    return 0;
}

}

void
removeKeychain(const std::string& keychainPath, const std::string& password)
{
    //
    // KeyChain path is relative to the current working directory.
    //
    std::string path = keychainPath;
    if(path.find("/") != 0)
    {
        std::string cwd;
        if(Test::getcwd(cwd) == 0)
        {
            path = std::string(cwd) + '/' + path;
        }
    }
    
    SecKeychainRef keychain;
    OSStatus err = SecKeychainOpen(path.c_str(),  &keychain);
    if(err == noErr)
    {
        err = SecKeychainUnlock(keychain, password.size(), password.c_str(), true);
        if(err == noErr)
        {
            err = SecKeychainDelete(keychain);
        }
        CFRelease(keychain);
    }
}
#endif