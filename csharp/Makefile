# Copyright (c) ZeroC, Inc.

top_srcdir := ..

include $(top_srcdir)/config/Make.rules

DOTNETBUILDARGS = $(if $(filter yes,$(OPTIMIZE)),,/p:Configuration=Debug)

all::
	dotnet msbuild msbuild/ice.proj $(DOTNETBUILDARGS)

tests::
	dotnet msbuild msbuild/ice.proj $(DOTNETBUILDARGS)

srcs::
	dotnet msbuild msbuild/ice.proj $(DOTNETBUILDARGS) /t:BuildDist

distclean:: clean
clean::
	dotnet msbuild msbuild/ice.proj /t:Clean

install::
	@echo nothing to install
