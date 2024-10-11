#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>
#include <EVENT/TrackerHit.h>
#include <EVENT/CalorimeterHit.h>
#include "marlin/VerbosityLevels.h"
#include "DDMarlinCED.h"

#include "Calo5D.h"

#include <iostream>

using namespace lcio;
using namespace marlin;


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

    DDMarlinCED::newEvent(this);

    dd4hep::Detector& detector = dd4hep::Detector::getInstance();
    DDMarlinCED::drawDD4hepDetector( detector, false, std::vector<std::string>{""} );

    DDCEDPickingHandler& pHandler = DDCEDPickingHandler::getInstance();
    pHandler.update(evt);


    LCCollection* trackerHits = evt->getCollection("TrackerHitCollection");
    for (size_t i = 0; i < trackerHits->getNumberOfElements(); i++) {
        auto* hit = static_cast<TrackerHit*> ( trackerHits->getElementAt(i) );
        auto pos = hit->getPosition();
        int type = 0; // point
        int size = 4; // larger point
        unsigned long color = 0x000000;
        ced_hit_ID(pos[0], pos[1], pos[2], type, 1, size, color, hit->id() );
        ced_hit_ID_animate(pos[0], pos[1], pos[2], hit->getTime(), type, 2, size, color, hit->id() );
    }
    LCCollection* calorimeterHits = evt->getCollection("CalorimeterHitCollection");
    for (size_t i = 0; i < calorimeterHits->getNumberOfElements(); i++) {
        auto* hit = static_cast<CalorimeterHit*> ( calorimeterHits->getElementAt(i) );
        auto pos = hit->getPosition();
        int type = 0; // point
        int size = 4; // larger point
        unsigned long color = 0x000000;
        ced_hit_ID(pos[0], pos[1], pos[2], type, 1, size, color, hit->id() );
        ced_hit_ID_animate(pos[0], pos[1], pos[2], hit->getTime(), type, 2, size, color, hit->id() );
    }

    DDMarlinCED::draw(this, 1);

}





void Calo5D::end(){ 
}
