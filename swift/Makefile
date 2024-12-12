#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

top_srcdir := ..

include $(top_srcdir)/config/Make.rules

CONFIG ?= $(if $(filter $(OPTIMIZE),no),Debug,Release)

SWIFT_BUILD_FLAGS ?= $(if $(V),,--quiet) --configuration  $(shell echo $(CONFIG) | tr '[:upper:]' '[:lower:]')

# $(call make-xcodebuild-rule,$1=rule,$2=platform,$3=action)
define xcode-test-app-rule
$1:: test/ios/TestDriverApp.xcodeproj
	xcodebuild $(if $(V),,-quiet) \
		-project test/ios/TestDriverApp.xcodeproj \
		-scheme TestDriverApp \
		-configuration $(CONFIG) \
		-arch arm64 \
		-sdk $2 $3

endef

all:: tests

srcs::
	$(Q)swift build $(SWIFT_BUILD_FLAGS)

tests::
	$(Q)swift build -Xswiftc -warnings-as-errors --package-path test $(SWIFT_BUILD_FLAGS)

install::
	@echo nothing to install

clean::
	$(Q)rm -rf ../.build
	$(Q)rm -rf test/.build

distclean:: clean

# We just build the test app for the iphonesimulator platform by default
test-app-platforms ?= $(filter iphonesimulator, $(platforms))

$(foreach p,$(test-app-platforms),$(eval $(call xcode-test-app-rule,tests,$p,build)))
$(foreach p,$(test-app-platforms),$(eval $(call xcode-test-app-rule,clean,$p,clean)))
$(foreach p,$(test-app-platforms),$(eval $(call xcode-test-app-rule,distclean,$p,clean)))
