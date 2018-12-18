# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

$(project)_libraries                            += Glacier2CryptPermissionsVerifier

Glacier2CryptPermissionsVerifier_targetdir      := $(libdir)
Glacier2CryptPermissionsVerifier_dependencies   := Glacier2 Ice
Glacier2CryptPermissionsVerifier_cppflags       := -DCRYPT_PERMISSIONS_VERIFIER_API_EXPORTS
Glacier2CryptPermissionsVerifier_devinstall     := no

projects += $(project)
