#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

top_srcdir := ..

include $(top_srcdir)/config/Make.rules

CONFIG ?= $(if $(filter $(OPTIMIZE),no),Debug,Release)

SWIFT_BUILD_FLAGS ?= $(if $(V),,--quiet) --configuration  $(shell echo $(CONFIG) | tr '[:upper:]' '[:lower:]')

# $(call make-xcodebuild-rule,$1=rule,$2=platform,$3=scheme,$4=target)
define make-xcodebuild-rule
$1:: $(PROJECT)
	xcodebuild $(if $(V),,-quiet) \
                   -project $(PROJECT) \
                   -target "$3 $(if $(filter $2,macosx),macOS,iOS)" \
                   -configuration $(CONFIG) \
                   -sdk $2 $4
endef

all:: tests

srcs:
	$(Q)swift build $(SWIFT_BUILD_FLAGS)

tests: spm-tests xcode-tests

spm-tests:
	$(Q)swift build --package-path test $(SWIFT_BUILD_FLAGS)

xcode-tests:
	$(Q) xcodebuild $(if $(V),,-quiet) \
        -project test/ios/TestDriverApp.xcodeproj \
        -configuration $(CONFIG) \
        -sdk iphonesimulator

install::
	@echo nothing to install

clean::
	$(Q)rm -rf .build
	$(Q)rm -rf test/.build

distclean:: clean

# $(foreach p,$(platforms),$(eval $(call make-xcodebuild-rule,srcs,$p,Ice,build)))

# $(foreach p,$(platforms),$(eval $(call make-xcodebuild-rule,tests,$p,TestDriver,build)))

# $(foreach p,$(platforms),$(eval $(call make-xcodebuild-rule,clean,$p,Ice,clean)))
# $(foreach p,$(platforms),$(eval $(call make-xcodebuild-rule,clean,$p,TestDriver,clean)))

# $(foreach p,$(supported-platforms),$(eval $(call make-xcodebuild-rule,distclean,$p,Ice,clean)))
# $(foreach p,$(supported-platforms),$(eval $(call make-xcodebuild-rule,distclean,$p,TestDriver,clean)))
