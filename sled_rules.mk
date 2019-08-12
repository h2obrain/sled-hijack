# create a custom c-compile recipe which depends on SLED_MODS_H and has some warnings disabled
$(eval $(call c_recipe,$(SLED_MK_DIR)/%,$(SLED_MODS_H) $(SLED_MK_DIR)/sled.mk,,\
-Wno-all -Wno-extra \
-Wno-strict-prototypes -Wno-missing-prototypes \
-Wno-undef -Wno-shadow -Wno-redundant-decls))

# dummy rule
$(SLED_MK_DIR)/modules/../sled/src/modules/%.c:

# create modules folder
$(SLED_MK_DIR)/modules:
	@mkdir $(if $(Q),,-v) -p $(@)

# create basic glue files (module list, etc)
$(SLED_MODS_H) $(SLED_MODS_C) $(SLED_MODS_H_PATHS): $(SLED_MK_DIR)/modules $(SLED_MODS_ORIG) # $(SLED_MODS_LOCAL)
	@echo "  CREATING $(subst $(LOCM3EX_DIR)/,,$(realpath $(@)))";
	@# h;
	@
	$(file  >$(SLED_MODS_H),/* Autogenerated */)
	$(file >>$(SLED_MODS_H),#pragma once)
	$(file >>$(SLED_MODS_H),#include <sled.h>)
	$(file >>$(SLED_MODS_H),#define SLED_MODULE_COUNT $(words $(SLED_MODS_NAMES)))
	$(file >>$(SLED_MODS_H),extern module_t *sled_modules[];)
	$(file >>$(SLED_MODS_H),)
	@# c
	$(file  >$(SLED_MODS_C),/* Autogenerated */)
	$(file >>$(SLED_MODS_C),#include "sled_modules.h")
	$(foreach NAME,$(SLED_MODS_NAMES),$(file >>$(SLED_MODS_C),extern module_t $(NAME);))
	$(file >>$(SLED_MODS_C),module_t *sled_modules[] = {)
	$(foreach NAME,&$(subst $(SPACE),$(COMMA) &,$(strip $(SLED_MODS_NAMES))),$(file >>$(SLED_MODS_C),		$(NAME)))
	$(file >>$(SLED_MODS_C),	};)
	$(file >>$(SLED_MODS_C),)

define SLED_MODULE_BODY =
static int init_(uint32_t modno_) {
	return init(modno_, NULL);
}
static void reset_(uint32_t modno_) {
	reset(modno_);
}
static int draw_(uint32_t modno_) {
	return draw(modno_, 0, NULL);
}
static void deinit_(uint32_t modno_) {
	deinit(modno_);
}
endef
$(SLED_MK_DIR)/modules/%.c : $(SLED_MK_DIR)/sled/src/modules/%.c $(SLED_MK_DIR)/modules $(SLED_MODS_H)
	@echo "  CREATING $@ << $<";
	$(file  >$@,#include <sled_gfx_private.h>)
	$(file >>$@,#include "../sled/src/modules/$(*).c")
	$(file >>$@,$(SLED_MODULE_BODY))
	$(file >>$@,module_t $(*) = { .name = "$(subst gfx_,,$(*))", .init = init_, .reset = reset_, .draw = draw_, .deinit = deinit_ };)
