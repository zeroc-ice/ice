# Copyright (c) ZeroC, Inc.

$(project)_programs        = client1 client2 client3 client4 client5
$(project)_libs            = iconv

$(project)_client1_sources = Client1.cpp
$(project)_client2_sources = Client2.cpp
$(project)_client3_sources = Client3.cpp
$(project)_client4_sources = Client4.cpp
$(project)_client5_sources = Client5.cpp

tests += $(project)
