#ifndef PDS_SCREEN_RECORDING_PROCESS_RING_H
#define PDS_SCREEN_RECORDING_PROCESS_RING_H

#include <optional>
#include "../error.h"

extern "C"{
#include <libavformat/avformat.h>
};

template<typename NextRingExecuteInputType, typename ExecuteInputType>
class ProcessChainRing {
    ProcessChainRing<NextRingExecuteInputType> *next;

public:
    ProcessChainRing *getNext() { return this->next; };

    void setNext(ProcessChainRing *ring) { this->next = ring; };

    virtual void execute(ExecuteInputType input) {};
};


#endif //PDS_SCREEN_RECORDING_PROCESS_RING_H
