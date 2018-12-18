# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_programs        = client1 client2 client3 client4 client5
$(test)_libs            = iconv

$(test)_client1_sources = Client1.cpp
$(test)_client2_sources = Client2.cpp
$(test)_client3_sources = Client3.cpp
$(test)_client4_sources = Client4.cpp
$(test)_client5_sources = Client5.cpp

tests += $(test)
