// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef CALOTOWERBUILDER_H
#define CALOTOWERBUILDER_H

#include <fun4all/SubsysReco.h>


#include <calobase/RawTower.h>  // for RawTower
#include <calobase/RawTowerContainer.h>
#include <calobase/RawTowerDefs.h>  // for HCALIN, HCALOUT, CEMC
#include <caloreco/CaloWaveformProcessing.h>


#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>    // for PHIODataNode
#include <phool/PHNodeIterator.h>  // for PHNodeIterator
#include <phool/PHObject.h>        // for PHObject
#include <phool/getClass.h>
#include <phool/phool.h>

#include <Event/Event.h>
#include <Event/EventTypes.h>
#include <Event/packet.h>

#include <cassert>
#include <cmath>  // for NAN
#include <iostream>
#include <map>      // for _Rb_tree_const_iterator
#include <utility>  // for pair


#include <calobase/TowerInfoContainer.h>
#include <calobase/TowerInfo.h>

#include <string>

class PHCompositeNode;

class CaloTowerBuilder : public SubsysReco
{
 public:

  CaloTowerBuilder(const std::string &name = "CaloTowerBuilder");

  ~CaloTowerBuilder() override;

  int Init(PHCompositeNode *topNode) override;
  int InitRun(PHCompositeNode *topNode) override;
  int process_event(PHCompositeNode *topNode) override;
  int ResetEvent(PHCompositeNode *topNode) override;
  int EndRun(const int runnumber) override;
  int End(PHCompositeNode *topNode) override;
  int Reset(PHCompositeNode * /*topNode*/) override;

  void Print(const std::string &what = "ALL") const override;



  void CreateNodeTree(PHCompositeNode *topNode);


  enum DetectorSystem
  {
    CEMC = 0,
    HCALIN = 1,
    HCALOUT = 2,
    EPD = 3
  };



  void set_detector_type(CaloTowerBuilder::DetectorSystem dettype)
  {
    m_dettype = dettype;
    return;
  }





 private:
  int _nevents;
  Event *_event;



  CaloWaveformProcessing* WaveformProcessing;

  PHCompositeNode *dst_node;
  PHCompositeNode *data_node;
  RawTowerContainer *hcalin_towers;
  RawTowerContainer *hcalout_towers;
  RawTowerContainer *emcal_towers;



  CaloTowerBuilder::DetectorSystem m_dettype; 
  TowerInfoContainer *m_TowerInfoContainer; //! Calo info
  std::string m_detector;
  int m_packet_low;
  int m_packet_high;



};

#endif // CALOTOWERBUILDER_H
