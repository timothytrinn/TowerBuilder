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


#include <calobase/TowerInfoContainer.h>
#include <calobase/TowerInfo.h>


//____________________________________________________________________________..
CaloTowerBuilder::CaloTowerBuilder(const std::string &name):
 SubsysReco(name)
 , m_dettype(CaloTowerBuilder::CEMC)
 , m_TowerInfoContainer(0)
 , m_detector("CEMC")
 , m_packet_low(6017)
 , m_packet_high(6032)
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

  if (m_dettype == CaloTowerBuilder::CEMC)
    {
      m_detector = "CEMC";
      m_packet_low = 6001;
      m_packet_high = 6128;
      WaveformProcessing->set_template_file("testbeam_cemc_template.root");
    }
  else if (m_dettype == CaloTowerBuilder::HCALIN)
    {
      m_detector = "HCALIN";
      m_packet_low = 7001;
      m_packet_high = 7032;
      WaveformProcessing->set_template_file("testbeam_ihcal_template.root");
   }
  else if (m_dettype == CaloTowerBuilder::HCALOUT)
    {
      m_detector = "HCALOUT";
      m_packet_low = 8001;
      m_packet_high = 8032;
      WaveformProcessing->set_template_file("testbeam_ohcal_template.root");
   }
  else if (m_dettype == CaloTowerBuilder::EPD)
    {
      m_detector = "EPD";
      m_packet_low = 9001;
      m_packet_high = 9002;  
      WaveformProcessing->set_template_file("testbeam_cemc_template.root"); // place holder until we have EPD templates
  }

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
  if ( _event->getEvtType() >= 8)/// special events where we do not read out the calorimeters
    {
      return Fun4AllReturnCodes::DISCARDEVENT;
    }
  std::vector<std::vector<float>> waveforms;

  for ( int pid = m_packet_low; pid <= m_packet_high; pid++)
    { 
      Packet *packet = _event->getPacket(pid);
      if (!packet)
	{// TODO: consider replacing DISCARDEVENT with filling empty towers... low need
	  return Fun4AllReturnCodes::DISCARDEVENT;
	}      
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
  for (int i = 0 ; i < n_channels;i++)
    {
      TowerInfo *towerinfo = new TowerInfo();
      if (waveforms.at(i).size() < nsamples;i++) // zero suppressed channels
	{
	  towerinfo->setTime(-1); // Flag tower as zero suppressed
	  towerinfo->setAmplitude(waveforms.at(i).at(0)-waveforms.at(i).at(1)); // zero suppression returns val at fixed time, and pedestal
	}
      else
	{
	  towerinfo->setTime(processed_waveforms.at(i).at(0));
	  towerinfo->setAmplitude(processed_waveforms.at(i).at(1));
	}
      m_TowerInfoContainer->add(towerinfo,i);
    }



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

  // towers
  m_TowerInfoContainer = new ClonesContainer();

  PHIODataNode<PHObject> *emcal_towerNode = new PHIODataNode<PHObject>(m_TowerInfoContainer, Form("TOWERS_%s",m_detector.c_str()), "PHObject");
  dst_node->addNode(emcal_towerNode);
}






