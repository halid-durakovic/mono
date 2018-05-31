
EXTRA_DIST=SUBMODULES.json build.mk

if INTERNAL_LLVM
.PHONY: all-local
all-local: configure-llvm build-llvm install-llvm

.PHONY: clean-local
clean-local: clean-llvm
endif

include build.mk
