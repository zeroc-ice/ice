#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

DOTNETARGS = $(if $(filter $(OPTIMIZE),no),/p:Configuration=Debug)

all:
	dotnet msbuild msbuild/ice.proj $(DOTNETARGS) /m

tests:
	dotnet msbuild msbuild/ice.proj $(DOTNETARGS) /m

srcs:
	dotnet msbuild msbuild/ice.proj $(DOTNETARGS) /t:BuildDist /m

distclean clean:
	dotnet msbuild msbuild/ice.proj /t:Clean /m

install::
	@echo nothing to install

test::
	@python ./allTests.py
