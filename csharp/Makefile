#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

#
# We use /p:UseSharedCompilation=false /nr:false to ensure msbuild doesn't leave processes behind
# after the build (this causes make to hang).
#
DOTNETARGS = /p:UseSharedCompilation=false /nr:false /m
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
