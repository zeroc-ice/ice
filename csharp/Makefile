#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

DOTNETARGS = /nr:false /m
DOTNETBUILDARGS = $(if $(filter $(OPTIMIZE),no),/p:Configuration=Debug)

all:
	dotnet msbuild msbuild/ice.proj $(DOTNETARGS) $(DOTNETBUILDARGS)

tests:
	dotnet msbuild msbuild/ice.proj $(DOTNETARGS) $(DOTNETBUILDARGS)

srcs:
	dotnet msbuild msbuild/ice.proj $(DOTNETARGS) $(DOTNETBUILDARGS) /t:BuildDist

distclean clean:
	dotnet msbuild msbuild/ice.proj $(DOTNETARGS) /t:Clean

install::
	@echo nothing to install

test::
	@python ./allTests.py
