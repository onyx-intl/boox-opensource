if BUILD_SPLASH_OUTPUT

pdftoppm_SOURCES =				\
	pdftoppm.cc				\
	$(common)

pdftoppm_binary = pdftoppm

pdftoppm_manpage = pdftoppm.1

endif

INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/goo			\
	-I$(top_srcdir)/utils			\
	-I$(top_srcdir)/poppler			\
	$(UTILS_CFLAGS)				\
	$(FONTCONFIG_CFLAGS)			\
	$(CAIRO_CFLAGS)

LDADD =						\
	$(top_builddir)/poppler/libpoppler.la	\
	$(UTILS_LIBS)				\
	$(FONTCONFIG_LIBS)

if BUILD_CAIRO_OUTPUT

pdftocairo_SOURCES =				\
	pdftocairo.cc				\
	$(common)

pdftocairo_LDADD =				\
	$(top_builddir)/poppler/libpoppler-cairo.la \
	$(LDADD) $(CAIRO_LIBS)


pdftocairo_binary = pdftocairo

pdftocairo_manpage = pdftocairo.1

endif

AM_LDFLAGS = @auto_import_flags@

bin_PROGRAMS =					\
	pdffonts				\
	pdfimages				\
	pdfinfo					\
	pdftops					\
	pdftotext				\
	pdftohtml				\
	pdfextract				\
	pdfmerge				\
	$(pdftoppm_binary)			\
	$(pdftocairo_binary)

dist_man1_MANS =				\
	pdffonts.1				\
	pdfimages.1				\
	pdfinfo.1				\
	pdftops.1				\
	pdftotext.1				\
	pdftohtml.1				\
	$(pdftoppm_manpage)			\
	$(pdftocairo_manpage)

common = parseargs.cc parseargs.h

pdffonts_SOURCES =				\
	pdffonts.cc				\
	$(common)

pdfimages_SOURCES =				\
	pdfimages.cc				\
	ImageOutputDev.cc			\
	ImageOutputDev.h			\
	$(common)

pdfinfo_SOURCES =				\
	pdfinfo.cc				\
	printencodings.cc			\
	printencodings.h			\
	$(common)

pdftops_SOURCES =				\
	pdftops.cc				\
	$(common)

pdftotext_SOURCES =				\
	pdftotext.cc				\
	printencodings.cc			\
	printencodings.h			\
	$(common)

pdftohtml_SOURCES =				\
	pdftohtml.cc				\
	HtmlFonts.cc				\
	HtmlFonts.h				\
	HtmlLinks.cc				\
	HtmlLinks.h				\
	HtmlOutputDev.cc			\
	HtmlOutputDev.h				\
	HtmlUtils.h				\
	$(common)

pdfextract_SOURCES =				\
	pdfextract.cc				\
	$(common)

pdfmerge_SOURCES =				\
	pdfmerge.cc				\
	$(common)

# Yay, automake!  It should be able to figure out that it has to dist
# pdftoppm.1, but nooo.  So we just add it here.

EXTRA_DIST = pdf2xml.dtd pdftoppm.1
