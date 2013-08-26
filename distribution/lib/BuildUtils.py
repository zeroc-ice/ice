
import os, sys

def getThirdpartyHome(version):
    if os.environ.get("THIRDPARTY_HOME"):
        return os.environ.get("THIRDPARTY_HOME")
    else:
        import winreg
        try:
            key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\ZeroC\\Ice %s Third Party Packages" % \
                                 version, 0, winreg.KEY_READ | winreg.KEY_WOW64_64KEY)
            installDir = os.path.abspath(winreg.QueryValueEx(key, "InstallDir")[0])

            if os.path.exists(installDir):
                return installDir
            else:
                return None
        except WindowsError as error:
            print(error)
            return None
            
def getIceHome(version):
    if os.environ.get("ICE_HOME"):
        return os.environ.get("ICE_HOME")
    else:
        import winreg
        try:
            key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\ZeroC\\Ice %s" % \
                                 version, 0, winreg.KEY_READ | winreg.KEY_WOW64_64KEY)
            installDir = os.path.abspath(winreg.QueryValueEx(key, "InstallDir")[0])

            if os.path.exists(installDir):
                return installDir
            else:
                return None
        except WindowsError as error:
            print(error)
            return None

def getAdvancedInstallerHome():
    import winreg
    try:
        key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\Caphyon\\Advanced Installer", \
                             0, winreg.KEY_READ | winreg.KEY_WOW64_32KEY)
        installDir = os.path.abspath(winreg.QueryValueEx(key, "Advanced Installer Path")[0])

        if os.path.exists(installDir):
            return installDir
        else:
            return None
    except WindowsError as error:
        print(error)
        return None

def getVcVarsAll(compiler):

    version = None

    if compiler == "VC90":
        version = "9.0"
    elif compiler == "VC100":
        version = "10.0"
    elif compiler == "VC110":
        version = "11.0"
    else:
        return None # Unsuported VC compiler 

    import winreg
    try:
        vcKey = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\VisualStudio\\%s" % version, 0, \
                               winreg.KEY_READ | winreg.KEY_WOW64_32KEY)
        vcInstallDir = os.path.abspath(winreg.QueryValueEx(vcKey, "InstallDir")[0])
        vcInstallDir = os.path.abspath(os.path.join(vcInstallDir, "..", "..", "VC"))
        vcVarsAll = os.path.join(vcInstallDir, "vcvarsall.bat")
        if os.path.exists(vcVarsAll):
            return vcVarsAll
        else:
            return None
    except WindowsError as error:
        return None

def getJavaHome(arch, configuration):

    if configuration == "java1.7":
        version = "1.7"
    else:
        version = "1.6"

    import winreg
    flags = winreg.KEY_READ
    if arch == "amd64":
        flags = flags | winreg.KEY_WOW64_64KEY
    else:
        flags = flags | winreg.KEY_WOW64_32KEY

    try:
        jdkKey = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\JavaSoft\\Java Development Kit\\%s" % \
                                version, 0, flags)
        javaHome = os.path.abspath(winreg.QueryValueEx(jdkKey, "JavaHome")[0])
        if os.path.exists(javaHome):
            return javaHome
        else:
            return None
    except WindowsError as error:
        return None

def getPythonHome(arch):

    import winreg
    flags = winreg.KEY_READ
    if arch == "amd64":
        flags = flags | winreg.KEY_WOW64_64KEY
    else:
        flags = flags | winreg.KEY_WOW64_32KEY

    try:
        key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\Python\\PythonCore\\3.3\\InstallPath", 0, flags)
        home = os.path.abspath(winreg.QueryValueEx(key, "")[0])
        if os.path.exists(home):
            return home
        else:
            return None
    except WindowsError as error:
        return None
