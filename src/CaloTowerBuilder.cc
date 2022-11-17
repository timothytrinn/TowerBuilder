#include "CaloTowerBuilder.h"

#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/SubsysReco.h>  // for SubsysReco

#include <calobase/RawTower.h>  // for RawTower
#include <calobase/RawTowerContainer.h>
#include <calobase/RawTowerDefs.h>  // for HCALIN, HCALOUT, CEMC

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>    // for PHIODataNode
#include <phool/PHNodeIterator.h>  // for PHNodeIterator
#include <phool/PHObject.h>        // for PHObject
#include <phool/getClass.h>
#include <phool/phool.h>

#include <phool/PHCompositeNode.h>
#include <Event/Event.h>
#include <Event/EventTypes.h>
#include <Event/packet.h>

#include <savecatonode/ClonesContainer.h>
#include <savecatonode/CaloInfo.h>

//____________________________________________________________________________..
CaloTowerBuilder::CaloTowerBuilder(const std::string &name):
 SubsysReco(name)
 , m_dettype(CaloTowerBuilder::CEMC)
 , m_CaloInfoContainer(0)
{
  std::cout << "CaloTowerBuilder::CaloTowerBuilder(const std::string &name) Calling ctor" << std::endl;
}

//____________________________________________________________________________..
CaloTowerBuilder::~CaloTowerBuilder()
{
  std::cout << "CaloTowerBuilder::~CaloTowerBuilder() Calling dtor" << std::endl;
}

//____________________________________________________________________________..
int CaloTowerBuilder::Init(PHCompositeNode *topNode)
{
  std::cout << "CaloTowerBuilder::Init(PHCompositeNode *topNode) Initializing" << std::endl;



  WaveformProcessing = new CaloWaveformProcessing();
  WaveformProcessing->set_processing_type(CaloWaveformProcessing::TEMPLATE);
  WaveformProcessing->set_template_file("testbeam_cemc_template.root");
  WaveformProcessing->initialize_processing();



  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int CaloTowerBuilder::InitRun(PHCompositeNode *topNode)
{
  std::cout << "CaloTowerBuilder::InitRun(PHCompositeNode *topNode) Initializing for Run XXX" << std::endl;
  CreateNodeTree(topNode);
  topNode->print();
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int CaloTowerBuilder::process_event(PHCompositeNode *topNode)
{
  _nevents++;
  int nsamples = 16;
  _event = findNode::getClass<Event>(topNode, "PRDF");
  if (_event == 0)
  {
    std::cout << "CaloUnpackPRDF::Process_Event - Event not found" << std::endl;
    return -1;
  }
  // std::cout << _event->getEvtType() << std::endl;
  if ( _event->getEvtType() >= 7)/// special event where we do not read out the calorimeters
    {
      return Fun4AllReturnCodes::DISCARDEVENT;
    }

  std::vector<std::vector<float>> waveforms;


  // for ( int pid = 6001; pid <= 6128; pid++)
  for ( int pid = 6017; pid <= 6032; pid++)
    { 
      // std::cout << 1 << std::endl;
      Packet *packet = _event->getPacket(pid);
      // packet->identify();
      if (!packet)
	{
	  return Fun4AllReturnCodes::DISCARDEVENT;
	}      
      // std::cout << 2 << std::endl;

      for ( int channel = 0; channel <  packet->iValue(0,"CHANNELS"); channel++)
	{
	  std::vector<float> waveform;
	  for (int samp = 0; samp < nsamples;samp++)
	    {
	      waveform.push_back(packet->iValue(samp,channel));	      
	    }
	  waveforms.push_back(waveform);
	  waveform.clear();
	}
    }

  std::vector<std::vector<float>> processed_waveforms =  WaveformProcessing->process_waveform(waveforms);

  int n_channels = processed_waveforms.size();
  std::cout << n_channels << std::endl;
  for (int i = 0 ; i < n_channels;i++)
    {
      CaloInfo *caloinfo = new CaloInfo();
      caloinfo->setTime(processed_waveforms.at(i).at(0));
      caloinfo->setAmplitude(processed_waveforms.at(i).at(1));
      m_CaloInfoContainer->add(caloinfo,i);
    }



  //Test Script to see if i can read things back:


    int csize = m_CaloInfoContainer->size();


    for(int j = 0; j < csize; j++)
    {
      CaloInfo *ci = m_CaloInfoContainer->at(j);
      float v1 = ci->getTime();
      float v2 = ci->getAmplitude();
      std::cout << "time: " << v1 << ", " << "Amplitude: " << v2 << std::endl;
    }










  std::cout << "CaloTowerBuilder::process_event(PHCompositeNode *topNode) Processing Event" << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int CaloTowerBuilder::ResetEvent(PHCompositeNode *topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int CaloTowerBuilder::EndRun(const int runnumber)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int CaloTowerBuilder::End(PHCompositeNode *topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int CaloTowerBuilder::Reset(PHCompositeNode *topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
void CaloTowerBuilder::Print(const std::string &what) const
{
}




void CaloTowerBuilder::CreateNodeTree(PHCompositeNode *topNode)
{
  PHNodeIterator nodeItr(topNode);
  // DST node
  dst_node = static_cast<PHCompositeNode *>(
      nodeItr.findFirst("PHCompositeNode", "DST"));
  if (!dst_node)
  {
    std::cout << "PHComposite node created: DST" << std::endl;
    dst_node = new PHCompositeNode("DST");
    topNode->addNode(dst_node);
  }

  // DATA nodes
  // data_node = static_cast<PHCompositeNode *>(
  //     nodeItr.findFirst("PHCompositeNode", "RAW_DATA"));
  // if (!data_node)
  // {
  //   if (Verbosity())
  //     std::cout << "PHComposite node created: RAW_DATA" << std::endl;
  //   data_node = new PHCompositeNode("RAW_DATA");
  //   dst_node->addNode(data_node);
  // }

  // HCAL Towers
  // hcalin_towers = new RawTowerContainer(RawTowerDefs::HCALIN);
  // PHIODataNode<PHObject> *tower_node = new PHIODataNode<PHObject>(
  //     hcalin_towers, "TOWER_RAW_HCALIN", "PHObject");
  // data_node->addNode(tower_node);

  // hcalout_towers = new RawTowerContainer(RawTowerDefs::HCALOUT);
  // tower_node = new PHIODataNode<PHObject>(hcalout_towers,
  //                                         "TOWER_RAW_HCALOUT", "PHObject");
  // data_node->addNode(tower_node);

  // EMCAL towers
  m_CaloInfoContainer = new ClonesContainer();

  PHIODataNode<PHObject> *emcal_towerNode =
      new PHIODataNode<PHObject>(m_CaloInfoContainer, "TOWER_CEMC", "PHObject");
  dst_node->addNode(emcal_towerNode);
}






