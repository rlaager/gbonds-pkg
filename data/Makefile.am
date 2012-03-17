## Process this file with automake to produce Makefile.in

@INTLTOOL_DESKTOP_RULE@

DESKTOP_IN_FILES = gbonds.desktop.in
DESKTOP_FILES =$(DESKTOP_IN_FILES:.desktop.in=.desktop)

desktopdir = $(datadir)/applications
desktop_DATA = $(DESKTOP_FILES)

@INTLTOOL_KEYS_RULE@

KEYS_IN_FILES=gbonds.keys.in
KEYS_FILES = $(KEYS_IN_FILES:.keys.in=.keys)

mimedir = $(datadir)/mime-info
mime_DATA = $(KEYS_FILES) gbonds.mime

appdir = $(datadir)/application-registry
app_DATA = gbonds.applications

@INTLTOOL_XML_RULE@

xml_in_files=gbonds.xml.in
xml_files = $(xml_in_files:.xml.in=.xml)

newmimedir = $(datadir)/mime/packages
newmime_DATA = $(xml_files)

redemptiondir = $(datadir)/gbonds
redemption_DATA = \
	download-sites.xml \
	sb199205.asc \
	sb199211.asc \
	sb199305.asc \
	sb199309.asc \
	sb199405.asc \
	sb199411.asc \
	sb199505.asc \
	sb199511.asc \
	sb199605.asc \
	sb199611.asc \
	sb199705.asc \
	sb199711.asc \
	sb199803.asc \
	sb199809.asc \
	sb199903.asc \
	sb199909.asc \
	sb200003.asc \
	sb200009.asc \
	sb200103.asc \
	sb200203.asc \
	sb200206.asc \
	sb200212.asc \
	sb200306.asc \
	sb200311.asc \
	sb200406.asc \
	sb200412.asc

EXTRA_DIST = \
	$(DESKTOP_IN_FILES)	\
	$(KEYS_IN_FILES)	\
	$(mime_DATA)            \
	$(app_DATA)		\
	$(xml_in_files)		\
	$(redemption_DATA)

install-data-hook:
if HAVE_NEW_MIME
	$(UPDATE_MIME_DATABASE) "$(DESTDIR)$(datadir)/mime"
endif
if HAVE_NEWER_MIME
	$(UPDATE_DESKTOP_DATABASE) 
endif

uninstall-hook:
if HAVE_NEW_MIME
	$(UPDATE_MIME_DATABASE) "$(DESTDIR)$(datadir)/mime"
endif
if HAVE_NEWER_MIME
	$(UPDATE_DESKTOP_DATABASE) 
endif

CLEANFILES = $(KEY_FILES) $(DESKTOP_FILES) $(xml_files)

