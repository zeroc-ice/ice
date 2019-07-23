#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

top_srcdir := ..

include $(top_srcdir)/config/Make.rules

CONFIG ?= $(if $(filter $(OPTIMIZE),yes),Release,Debug)

# $(call make-xcodebuild-rule,$1=rule,$2=platform,$3=scheme,$4=target)
define make-xcodebuild-rule
$1::
	xcodebuild -project ice.xcodeproj \
                   -scheme "$3 $(if $(filter $2,macosx),macOS,iOS)" \
                   -configuration $(CONFIG) \
                   -sdk $2 $4
endef

all:: tests

carthage:
	cd .. && carthage update --cache-builds

tests srcs:: carthage

install::
	@echo nothing to install

$(foreach p,$(platforms),$(eval $(call make-xcodebuild-rule,srcs,$p,Ice,build)))
$(foreach p,$(platforms),$(eval $(call make-xcodebuild-rule,tests,$p,TestDriver,build)))

$(foreach p,$(platforms),$(eval $(call make-xcodebuild-rule,clean,$p,Ice,clean)))
$(foreach p,$(platforms),$(eval $(call make-xcodebuild-rule,clean,$p,TestDriver,clean)))

$(foreach p,$(supported-platforms),$(eval $(call make-xcodebuild-rule,distclean,$p,Ice,clean)))
$(foreach p,$(supported-platforms),$(eval $(call make-xcodebuild-rule,distclean,$p,TestDriver,clean)))

test::
	@python ./allTests.py --config $(CONFIG)
