## Process this file with automake to produce Makefile.in

SUBDIRS= recent-files

INCLUDES = \
        -I$(top_srcdir)                                 \
        -I$(top_builddir)                               \
        $(GBONDS_CFLAGS)                                \
        -DPREFIX=\""$(prefix)"\"                        \
        -DSYSCONFDIR=\""$(sysconfdir)"\"                \
        -DDATADIR=\""$(datadir)"\"                      \
        -DLIBDIR=\""$(libdir)"\"                        \
        -DGBONDS_LOCALEDIR=\""$(datadir)/locale"\"      \
        -DGBONDS_UI_DIR=\""$(datadir)/gbonds/ui/"\"     \
	$(HAVE_FILE_CHOOSER)



bin_PROGRAMS = gbonds

BUILT_SOURCES =                         \
        marshal.c                       \
        marshal.h

gbonds_SOURCES = \
	gbonds.c			\
	splash.c			\
	splash.h			\
	window.c			\
	window.h			\
	ui.c				\
	ui.h				\
	commands.c			\
	commands.h			\
	recent.c			\
	recent.h			\
	file.c				\
	file.h				\
	print-dialog.c			\
	print-dialog.h			\
	prefs-dialog.c			\
	prefs-dialog.h			\
	view.c				\
	view.h				\
	edit.c				\
	edit.h				\
	doc.c				\
	doc.h				\
	doc-xml.c			\
	doc-xml.h			\
	doc-sbw.c			\
	doc-sbw.h			\
	sbw4.h				\
	table.c				\
	table.h				\
	table-model.c			\
	table-model.h			\
	rules.c				\
	rules.h				\
	types.c				\
	types.h				\
	print.c				\
	print.h				\
	prefs.c				\
	prefs.h				\
	update.c			\
	update.h			\
	util.c				\
	util.h				\
	hig.c				\
	hig.h				\
	debug.c				\
	debug.h				\
	$(BUILT_SOURCES)

gbonds_LDFLAGS = 

gbonds_LDADD =  \
	$(GBONDS_LIBS)			\
	recent-files/librecent.la


uidir = $(datadir)/gbonds/ui/

ui_DATA = gbonds-ui.xml

EXTRA_DIST = \
	$(ui_DATA)			\
	marshal.list

CLEANFILES = $(BUILT_SOURCES)

marshal.h: marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=gb_marshal > $@

marshal.c: marshal.list $(GLIB_GENMARSHAL)
	echo "#include \"marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=gb_marshal >> $@

