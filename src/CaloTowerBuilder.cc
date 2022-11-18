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
      // m_packet_low = 6001;
      // m_packet_high = 6128;
      m_packet_low = 6017;   // temporary to run over the subset file i have
      m_packet_high = 6032; // temporary to run over the subset file i have
      WaveformProcessing->set_template_file("testbeam_cemc_template.root");
    }
  else if (m_dettype == CaloTowerBuilder::HCALIN)
    {
      m_detector = "HCALIN";
      WaveformProcessing->set_template_file("testbeam_ihcal_template.root");
   }
  else if (m_dettype == CaloTowerBuilder::HCALOUT)
    {
      m_detector = "HCALOUT";
      m_packet_low = 8001;
      m_packet_high = 6032;
      WaveformProcessing->set_template_file("testbeam_ohcal_template.root");
   }
  else if (m_dettype == CaloTowerBuilder::EPD)
    {
      m_detector = "EPD";
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
  if ( _event->getEvtType() >= 7)/// special event where we do not read out the calorimeters
    {
      return Fun4AllReturnCodes::DISCARDEVENT;
    }
  std::vector<std::vector<float>> waveforms;

  for ( int pid = m_packet_low; pid <= m_packet_high; pid++)
    { 
      Packet *packet = _event->getPacket(pid);
      if (!packet)
	{
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
      CaloInfo *caloinfo = new CaloInfo();
      caloinfo->setTime(processed_waveforms.at(i).at(0));
      caloinfo->setAmplitude(processed_waveforms.at(i).at(1));
      m_CaloInfoContainer->add(caloinfo,i);
    }



  //Quick test script to see if i can read things back:
  //-----------------------------------------------------------------------
    // int csize = m_CaloInfoContainer->size();
    // for(int j = 0; j < csize; j++)
    // {
    //   CaloInfo *ci = m_CaloInfoContainer->at(j);
    //   float v1 = ci->getTime();
    //   float v2 = ci->getAmplitude();
    //   std::cout << "time: " << v1 << ", " << "Amplitude: " << v2 << std::endl;
    // }
  //-----------------------------------------------------------------------


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
  m_CaloInfoContainer = new ClonesContainer();

  PHIODataNode<PHObject> *emcal_towerNode = new PHIODataNode<PHObject>(m_CaloInfoContainer, Form("TOWERS_%s",m_detector.c_str()), "PHObject");
  dst_node->addNode(emcal_towerNode);
}






