AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = \
  -I$(includedir) \
  -I$(OFFLINE_MAIN)/include \
  -I$(ROOTSYS)/include

AM_LDFLAGS = \
  -L$(libdir) \
  -L/gpfs/mnt/gpfs02/sphenix/user/trinn/calo_reco_software_dev/install/ \
  -L$(OFFLINE_MAIN)/lib \
  -L$(OFFLINE_MAIN)/lib64 \
  -lcalo_io \
  -lfun4all \
  -lsavecatonode \
  -lcalo_reco


pkginclude_HEADERS = \
  CaloTowerBuilder.h

lib_LTLIBRARIES = \
  libCaloTowerBuilder.la

libCaloTowerBuilder_la_SOURCES = \
  CaloTowerBuilder.cc

libCaloTowerBuilder_la_LIBADD = \
  -lphool \
  -lSubsysReco

BUILT_SOURCES = testexternals.cc

noinst_PROGRAMS = \
  testexternals

testexternals_SOURCES = testexternals.cc
testexternals_LDADD   = libCaloTowerBuilder.la

testexternals.cc:
	echo "//*** this is a generated file. Do not commit, do not edit" > $@
	echo "int main()" >> $@
	echo "{" >> $@
	echo "  return 0;" >> $@
	echo "}" >> $@

clean-local:
	rm -f $(BUILT_SOURCES)
