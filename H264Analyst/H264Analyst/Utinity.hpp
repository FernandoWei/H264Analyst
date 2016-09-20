//
//  Utinity.hpp
//  H264Analyst
//
//  Created by fernando on 16/9/20.
//  Copyright © 2016年 Fernando Wei. All rights reserved.
//

#ifndef Utinity_hpp
#define Utinity_hpp

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

#include <stdio.h>
#include <iostream>
#include <vector>
#include <array>

namespace H264Analyst{
    enum class SliceType{
        P_Slice,
        B_Slice,
        I_Slice,
        SI_Slice,
        SP_Slice
    };
    
    enum class NalType{
        NAL_Slice,
        NAL_PA,
        NAL_PB,
        NAL_PC,
        NAL_IDR_Slice,
        NAL_SEI,
        NAL_SPS,
        NAL_PPS,
        NAL_AUD,
        NAL_ERR
    };
    
    enum class ResultType{
        OK,
        ERROR
    };
    
    void parsePkt(AVPacket* pkt, std::vector<unsigned long>& results);
    unsigned long getNalSize(uint8_t* data, unsigned long index);
    NalType getNalType(uint8_t* data, unsigned long index);
    SliceType getSliceType(int slice_type);
    std::string sliceTypeToString(SliceType type);
}

#endif /* Utinity_hpp */
