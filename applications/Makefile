TOPTARGETS := all clean install clean_all

app-y 	+= tools
app-y 	+= dashcam/resource
app-y	+= manager
app-y	+= dashcam

SUBDIRS += $(app-y)

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

# 依赖关系
dashcam: manager

.PHONY: $(TOPTARGETS) $(SUBDIRS)
