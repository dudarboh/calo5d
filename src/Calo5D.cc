#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>
#include <EVENT/TrackerHit.h>
#include <EVENT/SimTrackerHit.h>
#include <EVENT/CalorimeterHit.h>
#include <UTIL/LCRelationNavigator.h>
#include "marlin/VerbosityLevels.h"
#include "DDMarlinCED.h"

#include "Calo5D.h"

#include <iostream>

using namespace lcio;
using namespace marlin;

float getTrackerHitTime(EVENT::TrackerHit* hit, const UTIL::LCRelationNavigator& nav){
    // Some reco hits stored *without* time. Need to manually get SimHit timing.
    if ( hit->getTime() != 0.f ) return hit->getTime(); // GOOD: just read recoHit time
    if ( nav.getRelatedToObjects(hit).empty() ) return 0.f; // BAD: No simHit found!?
    
    auto& weights = nav.getRelatedToWeights(hit);
    int max_i = std::max_element(weights.begin(), weights.end()) - weights.begin(); // to be safe, but most of the time should be one to one match
    auto simHit = static_cast<EVENT::SimTrackerHit*> (nav.getRelatedToObjects(hit)[max_i]);
    return simHit->getTime();
}

EVENT::SimCalorimeterHit* getSimCalorimeterHit(EVENT::CalorimeterHit* hit, const UTIL::LCRelationNavigator& nav){
    if ( nav.getRelatedToObjects(hit).empty() ) return nullptr; // BAD: No simHit found!?
    auto& weights = nav.getRelatedToWeights(hit);
    int max_i = std::max_element(weights.begin(), weights.end()) - weights.begin(); // to be safe, but most of the time should be one to one match
    return static_cast<EVENT::SimCalorimeterHit*> (nav.getRelatedToObjects(hit)[max_i]);
}

float getCalorimeterHitTime(EVENT::CalorimeterHit* hit, const UTIL::LCRelationNavigator& nav){
    // Some reco hits stored *without* time. Need to manually get *earliest contribution* in the SimHit timing.
    if ( hit->getTime() != 0.f ) return hit->getTime(); // GOOD: just read recoHit time

    auto simHit = getSimCalorimeterHit(hit, nav);
    if ( simHit == nullptr ) return 0.f;

    std::vector<float> contTimes;
    for(int i=0; i < simHit->getNMCContributions(); i++) contTimes.push_back( simHit->getTimeCont(i) );
    
    if ( contTimes.empty() ) return 0.f; // BAD: SimHit without MC contributions !?
    return *std::min_element( contTimes.begin(), contTimes.end() );
}


Calo5D aCalo5D ;


Calo5D::Calo5D() : Processor("Calo5D") {

    // modify processor description
    _description = "Description ...";
}


void Calo5D::init(){ 
    DDMarlinCED::init(this);
}


void Calo5D::processEvent( LCEvent * evt ) {
    ++_nEvent;
    streamlog_out(MESSAGE)<<"Event: "<<_nEvent<<std::endl;
    
    //find all reconstructed charged and neutral hits
    std::vector<CalorimeterHit*> hitsRecoCharged;
    std::vector<CalorimeterHit*> hitsRecoNeutral;
    auto pfos = evt->getCollection("PandoraPFOs");

    for (int i = 0; i < pfos->getNumberOfElements(); i++) {
        auto* pfo = static_cast<ReconstructedParticle*> ( pfos->getElementAt(i) );
        bool isCharged = std::abs( pfo->getCharge() ) > 0.1;
        auto clusters = pfo->getClusters();
        for (const auto& cluster : clusters){
            auto hits = cluster->getCalorimeterHits();
            if (isCharged) hitsRecoCharged.insert( hitsRecoCharged.end(), hits.begin(), hits.end() );
            else  hitsRecoNeutral.insert( hitsRecoNeutral.end(), hits.begin(), hits.end() );
        }
    }


    //drawing
    DDMarlinCED::newEvent(this);

    dd4hep::Detector& detector = dd4hep::Detector::getInstance();
    DDMarlinCED::drawDD4hepDetector( detector, false, std::vector<std::string>{""} );

    DDCEDPickingHandler& pHandler = DDCEDPickingHandler::getInstance();
    pHandler.update(evt);

    int type = 0; // point
    int size = 4; // larger point
    unsigned long black = 0x000000;
    unsigned long red = 0xff0000;
    unsigned long blue = 0x0000ff;
    unsigned long gray = 0x808080;

    auto trackerHits = evt->getCollection("TrackerHitCollection");
    auto navToSimTrackerHits = LCRelationNavigator ( evt->getCollection("TrackerHitRelations") );
    for (int i = 0; i < trackerHits->getNumberOfElements(); i++) {
        auto* hit = static_cast<TrackerHit*> ( trackerHits->getElementAt(i) );
        auto pos = hit->getPosition();
        float time = getTrackerHitTime(hit, navToSimTrackerHits);

        ced_hit_ID( pos[0], pos[1], pos[2], type, 1, size, black, hit->id() );
        ced_hit_ID_animate( pos[0], pos[1], pos[2], time, type, 2, size, black, hit->id() );
        ced_hit_ID( pos[0], pos[1], pos[2], type, 3, size, red, hit->id() );
        ced_hit_ID_animate( pos[0], pos[1], pos[2], time, type, 4, size, red, hit->id() );
        ced_hit_ID( pos[0], pos[1], pos[2], type, 5, size, red, hit->id() );
        ced_hit_ID_animate( pos[0], pos[1], pos[2], time, type, 6, size, red, hit->id() );
        ced_hit_ID( pos[0], pos[1], pos[2], type, 7, size, gray, hit->id() );
        ced_hit_ID_animate( pos[0], pos[1], pos[2], time, type, 8, size, gray, hit->id() );
    }
    auto calorimeterHits = evt->getCollection("CalorimeterHitCollection");
    auto navToSimCalorimeterHits = LCRelationNavigator ( evt->getCollection("CalorimeterHitRelations") );
    for (int i = 0; i < calorimeterHits->getNumberOfElements(); i++) {
        auto* hit = static_cast<CalorimeterHit*> ( calorimeterHits->getElementAt(i) );
        auto pos = hit->getPosition();
        float time = getCalorimeterHitTime(hit, navToSimCalorimeterHits);

        // draw/animate just in black
        ced_hit_ID( pos[0], pos[1], pos[2], type, 1, size, black, hit->id() );
        ced_hit_ID_animate( pos[0], pos[1], pos[2], time, type, 2, size, black, hit->id() );

        // Use Pandora charged/neutral hit association
        unsigned long recoColor = black;
        if ( std::find(hitsRecoCharged.begin(), hitsRecoCharged.end(), hit) != hitsRecoCharged.end() ) recoColor = red;
        else if ( std::find(hitsRecoNeutral.begin(), hitsRecoNeutral.end(), hit) != hitsRecoNeutral.end() ) recoColor = blue;

        ced_hit_ID( pos[0], pos[1], pos[2], type, 3, size, recoColor, hit->id() );
        ced_hit_ID_animate( pos[0], pos[1], pos[2], time, type, 4, size, recoColor, hit->id() );

        // Use true charged/neutral hit association
        unsigned long trueColor = black;
        auto simHit = getSimCalorimeterHit(hit, navToSimCalorimeterHits);
        std::map<EVENT::MCParticle*, float> mc2edep;
        float energySum = 0.f;
        for(int j=0; j < simHit->getNMCContributions(); j++){
            mc2edep[ simHit->getParticleCont(j) ] += simHit->getEnergyCont(j);
            energySum += simHit->getEnergyCont(j); 
        }
        MCParticle* mcHighestEnergy = std::max_element( mc2edep.begin(), mc2edep.end(), [](const auto& l, const auto& r){ return l.second < r.second; } )->first;
        float energyFraction =  std::max_element( mc2edep.begin(), mc2edep.end(), [](const auto& l, const auto& r){ return l.second < r.second; } )->second / energySum;
        if ( std::abs(mcHighestEnergy->getCharge()) > 0.1 ) trueColor = red;
        else trueColor = blue;

        ced_hit_ID( pos[0], pos[1], pos[2], type, 5, size, trueColor, hit->id() );
        ced_hit_ID_animate( pos[0], pos[1], pos[2], time, type, 6, size, trueColor, hit->id() );
        unsigned long diffColor = recoColor == trueColor ? gray : recoColor;
        ced_hit_ID( pos[0], pos[1], pos[2], type, 7, size, diffColor, hit->id() );
        ced_hit_ID_animate( pos[0], pos[1], pos[2], time, type, 8, size, diffColor, hit->id() );
    }

    DDMarlinCED::draw(this, 1);

}





void Calo5D::end(){ 
}
