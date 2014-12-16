
import os, sys

def getThirdpartyHome(version):
    if os.environ.get("THIRDPARTY_HOME"):
        return os.environ.get("THIRDPARTY_HOME")
    elif sys.platform == "darwin":
        if os.path.exists("/Library/Developer/Ice-%s-ThirdParty/lib/db.jar" % version):
            return "/Library/Developer/Ice-%s-ThirdParty/lib/" % version
    elif sys.platform == "win32":
        import winreg
        try:
            key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\ZeroC\\Ice %s Third Party Packages" % \
                                 version, 0, winreg.KEY_READ | winreg.KEY_WOW64_64KEY)
            installDir = os.path.abspath(winreg.QueryValueEx(key, "InstallDir")[0])

            if os.path.exists(installDir):
                return installDir
        except WindowsError as error:
            print(error)
    return None
            
def getIceHome(version):
    if os.environ.get("ICE_HOME"):
        return os.environ.get("ICE_HOME")
    elif sys.platform == "darwin":
        if os.path.exists("/Library/Developer/Ice-%s/bin/slice2cpp" % version):
            return "/Library/Developer/Ice-%s" % version
        elif os.path.exists("/opt/Ice-%s/bin/slice2cpp" % version):
            return "/opt/Ice-%s/bin/slice2cpp" % version
    elif sys.platform.startswith("linux"):
        if os.path.exists("/usr/bin/slice2cpp"):
            return "/usr"  
        elif os.path.exists("/opt/Ice-%s/bin/slice2cpp" % version):
            return "/opt/Ice-%s" % version
    elif sys.platform == "win32":
        import winreg
        try:
            key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\ZeroC\\Ice %s" % \
                                 version, 0, winreg.KEY_READ | winreg.KEY_WOW64_64KEY)
            installDir = os.path.abspath(winreg.QueryValueEx(key, "InstallDir")[0])

            if os.path.exists(os.path.join(installDir, "bin", "slice2cpp.exe")):
                return installDir
        except WindowsError as error:
            print(error)
            return None
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
    elif compiler == "VC120":
        version = "12.0"
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

def getJavaHome(arch, version):

    import winreg
    flags = winreg.KEY_READ
    if arch == "amd64" or arch == "x64":
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
    if arch == "amd64" or arch == "x64":
        flags = flags | winreg.KEY_WOW64_64KEY
    else:
        flags = flags | winreg.KEY_WOW64_32KEY

    try:
        key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\Python\\PythonCore\\3.4\\InstallPath", 0, flags)
        home = os.path.abspath(winreg.QueryValueEx(key, "")[0])
        if os.path.exists(home):
            return home
        else:
            return None
    except WindowsError as error:
        return None

def getRubyHome(arch):
    import winreg
    flags = winreg.KEY_READ
    if arch == "amd64" or arch == "x64":
        flags = flags | winreg.KEY_WOW64_64KEY
    else:
        flags = flags | winreg.KEY_WOW64_32KEY

    #
    # Unfortunately, the RubyInstaller creates a single for both the
    # x86 and x64 installer.
    #
    for parent in [winreg.HKEY_LOCAL_MACHINE, winreg.HKEY_CURRENT_USER]:
        try:
            key = winreg.OpenKey(parent, "SOFTWARE\\RubyInstaller\\MRI\\2.1.5", 0, flags)
            buildPlatform = winreg.QueryValueEx(key, "BuildPlatform")[0]

            key = winreg.OpenKey(parent, "SOFTWARE\\RubyInstaller\\MRI\\2.1.5", 0, flags)
            rubyHome = os.path.abspath(winreg.QueryValueEx(key, "InstallLocation")[0])
            if not os.path.exists(rubyHome):
                continue

            if (arch == "amd64" or arch == "x64") == (buildPaltform.find("x64-") == 0):
                return rubyHome
            elif (arch == "amd64" or arch == "x64"):
                rubyHome += "x64"
            elif(rubyHome.find("-x64") > 0):
                rubyHome = rubyHome[0:(len(rubyHome) - 4)]

            if os.path.exists(rubyHome):
                return rubyHome
        except:
            pass

    #
    # Check default installation directory
    #
    if (arch == "amd64" or arch == "x64"):
        if os.path.exists("C:\Ruby21-x64"):
            return "C:\Ruby21-x64"
    else:
        if os.path.exists("C:\Ruby21"):
            return "C:\Ruby21"

    return None

def getNodeHome(arch):
    import winreg
    flags = winreg.KEY_READ
    if arch == "amd64" or arch == "x64":
        flags = flags | winreg.KEY_WOW64_64KEY
    else:
        flags = flags | winreg.KEY_WOW64_32KEY

    for parent in [winreg.HKEY_LOCAL_MACHINE, winreg.HKEY_CURRENT_USER]:
        try:
            key = winreg.OpenKey(parent, "SOFTWARE\\Node.js", 0, flags)
            nodeHome = os.path.abspath(winreg.QueryValueEx(key, "InstallPath")[0])
            if os.path.exists(nodeHome):
                return nodeHome
        except:
            pass
    return None

