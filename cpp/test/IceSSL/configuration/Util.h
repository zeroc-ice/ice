
#include <IceSSL/IceSSL.h>

#ifdef ICE_USE_SECURE_TRANSPORT

#include <IceUtil/FileUtil.h>
#include <Security/Security.h>

void
removeKeychain(const std::string& keychainPath, const std::string& password)
{
    //
    // KeyChain path is relative to the current working directory.
    //
    std::string path = keychainPath;
    if(!IceUtilInternal::isAbsolutePath(keychainPath))
    {
        std::string cwd;
        if(IceUtilInternal::getcwd(cwd) == 0)
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