# Copyright (c) ZeroC, Inc.

top_srcdir := ..

include $(top_srcdir)/config/Make.rules

# We use /p:UseSharedCompilation=false /nr:false to ensure msbuild doesn't leave processes behind
# after the build (this causes make to hang).
DOTNETARGS = /p:UseSharedCompilation=false /nr:false /m $(if $(filter yes,$(OPTIMIZE)),,/p:Configuration=Debug)

all::
	dotnet msbuild msbuild/ice.proj $(DOTNETARGS)

tests::
	dotnet msbuild msbuild/ice.proj $(DOTNETARGS)

srcs::
	dotnet msbuild msbuild/ice.proj $(DOTNETARGS) /t:BuildDist

distclean:: clean
clean::
	dotnet msbuild msbuild/ice.proj $(DOTNETARGS) /t:Clean

install::
	@echo nothing to install
