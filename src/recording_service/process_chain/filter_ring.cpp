//
// Created by Marco Bramini on 01/11/21.
//

#include "filter_ring.h"


void FilterChainRing::execute(AVFrame *inputFrame) {
    if (std::holds_alternative<FilterChainRing *>(next)) {
        std::get<FilterChainRing *>(next)->execute(inputFrame);
    } else {
        std::get<EncoderChainRing *>(next)->execute(inputFrame);
    }
}