#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries := IcePHP

IcePHP_target           := php-module
IcePHP_targetname       := ice
IcePHP_targetdir        := lib
IcePHP_installdir       := $(install_phplibdir)
IcePHP_cppflags         := -I$(project) -I$(top_srcdir)/cpp/src $(ice_cpp_cppflags) $(php_cppflags)
IcePHP_ldflags          := $(php_ldflags)
IcePHP_dependencies     := IceDiscovery++11 IceLocatorDiscovery++11 IceSSL++11 Ice++11
# TODO temporary ++11 dependencies are not linked
IcePHP_system_libs      := -L$(top_srcdir)/cpp/lib/x86_64-linux-gnu \
    -L$(top_srcdir)/cpp/lib \
    -lIce++11 -lIceSSL++11 -lIceDiscovery++11 -lIceLocatorDiscovery++11
IcePHP_extra_sources    := $(filter-out %Util.cpp %Ruby.cpp,\
                           $(wildcard $(top_srcdir)/cpp/src/Slice/*.cpp)) \
                           $(top_srcdir)/cpp/src/Slice/SliceUtil.cpp \
                           $(top_srcdir)/cpp/src/Slice/PHPUtil.cpp \
                           $(top_srcdir)/cpp/src/Slice/StringLiteralUtil.cpp \

projects += $(project)
srcs:: $(project)
