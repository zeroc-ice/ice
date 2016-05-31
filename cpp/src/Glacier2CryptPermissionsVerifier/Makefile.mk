# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries 				+= Glacier2CryptPermissionsVerifier

Glacier2CryptPermissionsVerifier_targetdir	:= $(libdir)
Glacier2CryptPermissionsVerifier_dependencies	:= Glacier2 Ice
Glacier2CryptPermissionsVerifier_cppflags	:= -DCRYPT_PERMISSIONS_VERIFIER_API_EXPORTS

projects += $(project)
