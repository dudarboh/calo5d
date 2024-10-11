#ifndef Calo5D_h
#define Calo5D_h 1

#include "marlin/Processor.h"
#include "EVENT/LCEvent.h"

#include "TFile.h"
#include "TTree.h"


class Calo5D : public marlin::Processor {
    public:
        Calo5D(const Calo5D&) = delete;
        Calo5D& operator=(const Calo5D&) = delete;

        marlin::Processor* newProcessor() { return new Calo5D; }

        Calo5D();
        void init();
        void processEvent(EVENT::LCEvent* evt);
        void end();
        
    private:
        int _nEvent{};
        float _bField{};
};

#endif
