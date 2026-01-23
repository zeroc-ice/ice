# Copyright (c) ZeroC, Inc.

DOTNETBUILDARGS = $(if $(filter $(OPTIMIZE),no),/p:Configuration=Debug)

all:
	dotnet msbuild msbuild/ice.proj $(DOTNETBUILDARGS)

tests:
	dotnet msbuild msbuild/ice.proj $(DOTNETBUILDARGS)

srcs:
	dotnet msbuild msbuild/ice.proj $(DOTNETBUILDARGS) /t:BuildDist

distclean clean:
	dotnet msbuild msbuild/ice.proj /t:Clean

install::
	@echo nothing to install
