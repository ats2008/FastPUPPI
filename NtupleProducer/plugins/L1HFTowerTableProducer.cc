// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/global/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/View.h"


#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "CalibFormats/CaloTPG/interface/CaloTPGTranscoder.h"
#include "CalibFormats/CaloTPG/interface/CaloTPGRecord.h"
#include "L1Trigger/L1TCalorimeter/interface/CaloTools.h"

#include "DataFormats/Candidate/interface/Candidate.h"

#include "DataFormats/Math/interface/deltaR.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/NanoAOD/interface/FlatTable.h"

#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "CommonTools/Utils/interface/StringObjectFunction.h"

#include <algorithm>

class L1HFTowerTableProducer : public edm::global::EDProducer<>  {
    public:
        explicit L1HFTowerTableProducer(const edm::ParameterSet&);
        ~L1HFTowerTableProducer();

    private:
  edm::EDGetTokenT<HcalTrigPrimDigiCollection> hfTowersSrc_;
  edm::ESGetToken<CaloTPGTranscoder, CaloTPGRecord> decoderTag_;
  virtual void produce(edm::StreamID id, edm::Event& iEvent, const edm::EventSetup& iSetup) const override;
    
};

L1HFTowerTableProducer::L1HFTowerTableProducer(const edm::ParameterSet& iConfig) :
  hfTowersSrc_(consumes<HcalTrigPrimDigiCollection>(iConfig.getParameter<edm::InputTag>("hcalDigis"))),
  decoderTag_(esConsumes<CaloTPGTranscoder, CaloTPGRecord>(edm::ESInputTag("", "")))
{

        produces<nanoaod::FlatTable>("HFTower");

 }

L1HFTowerTableProducer::~L1HFTowerTableProducer() { }

// ------------ method called for each event  ------------
    void
L1HFTowerTableProducer::produce(edm::StreamID id, edm::Event& iEvent, const edm::EventSetup& iSetup) const
{

  std::vector<float> hfTower_et;
  std::vector<float> hfTower_eta , hfTower_ieta;
  std::vector<float> hfTower_phi , hfTower_iphi;

  const auto& decoder = iSetup.getData(decoderTag_);
  edm::Handle<HcalTrigPrimDigiCollection> hfHandle;
  int hf_count=0;
  if (iEvent.getByToken(hfTowersSrc_, hfHandle)){
    for (const auto& hit : *hfHandle.product()) {
      if (abs(hit.id().ieta()) < l1t::CaloTools::kHFBegin) continue;
      if (abs(hit.id().ieta()) > l1t::CaloTools::kHFEnd) continue;
      float et = decoder.hcaletValue(hit.id(), hit.t0());
      float eta = l1t::CaloTools::towerEta(hit.id().ieta());
      float phi = l1t::CaloTools::towerPhi(hit.id().ieta(), hit.id().iphi());
      // std::cout<<" hf TPDigi id "<<hf_count++
      //         <<" | ieta : "<<hit.id().ieta()<<" , iphi : "<<hit.id().iphi()
      //         <<" | eta  : "<<eta<<" , phi : "<<phi
      //         <<" | et : "<<et<<"\n";
        hfTower_et.push_back(et);
        hfTower_eta.push_back(eta); 
        hfTower_phi.push_back(phi);
        hfTower_ieta.push_back(hit.id().ieta());
        hfTower_iphi.push_back(hit.id().iphi());

    }
  }
   else
    std::cout<<"Failed to get HcalTrigPrimDigi for HF!"<<std::endl;


   // create the table
   unsigned int nHFTower = hfTower_et.size();
   auto out = std::make_unique<nanoaod::FlatTable>(nHFTower,"HFTower", false);

   out->addColumn<float>("pt"  , hfTower_et  , "et of hftower");
   out->addColumn<float>("eta" , hfTower_eta , "eta of hftower");
   out->addColumn<float>("phi" , hfTower_phi , "phi of hftower");
   out->addColumn<float>("ieta", hfTower_ieta, "IEta of hftower");
   out->addColumn<float>("iphi", hfTower_iphi, "IPhi of hftower");

   iEvent.put(std::move(out), "HFTower");
}

//define this as a plug-in
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(L1HFTowerTableProducer);
