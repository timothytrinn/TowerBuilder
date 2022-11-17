#pragma once
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4allraw/Fun4AllPrdfInputManager.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <calotowerbuilder/CaloTowerBuilder.h>
#include <fun4all/Fun4AllDstOutputManager.h>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libfun4allraw.so)
R__LOAD_LIBRARY(libCaloTowerBuilder.so)

#endif

void Fun4All_CaloWaveFormSim(const char *fname = "/sphenix/data/data02/sphenix/cemc/combinedEvents/EmCalSEB01-000000222-0000_x.prdf", const char *outfile = "trees/0.root")
{
  gSystem->Load("libg4dst");
  Fun4AllServer *se = Fun4AllServer::instance();

  // CaloWaveFormSim *ca = new CaloWaveFormSim("CALOWAVEFORMSIM",outfile);
  // ca->Detector("CEMC");
  // se->registerSubsystem(ca);

  Fun4AllInputManager *in = new Fun4AllPrdfInputManager("in");
  in->fileopen(fname);


  CaloTowerBuilder *ca = new CaloTowerBuilder();
  // ca->Detector("CEMC");
  se->registerSubsystem(ca);


  // in->AddListFile("g4hits.list");

// Fun4All
  se->registerInputManager(in);

  Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", "testoutput.root");

  se->registerOutputManager(out);



  se->run(10);
  se->End();
}
