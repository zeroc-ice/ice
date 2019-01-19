#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(test)_programs        = client1 client2 client3 client4 client5
$(test)_libs            = iconv

$(test)_client1_sources = Client1.cpp
$(test)_client2_sources = Client2.cpp
$(test)_client3_sources = Client3.cpp
$(test)_client4_sources = Client4.cpp
$(test)_client5_sources = Client5.cpp

tests += $(test)
