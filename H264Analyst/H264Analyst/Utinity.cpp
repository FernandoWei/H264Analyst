//
//  Utinity.cpp
//  H264Analyst
//
//  Created by fernando on 16/9/20.
//  Copyright © 2016年 Fernando Wei. All rights reserved.
//

#include "Utinity.hpp"


namespace H264Analyst {
    
    SliceType getSliceType(int slice_type){
        slice_type %= 5;
        if (slice_type == 0){
            return SliceType::P_Slice;
        }else if (slice_type == 1){
            return SliceType::B_Slice;
        }else if (slice_type == 2){
            return SliceType::I_Slice;
        }else if (slice_type == 3){
            return SliceType::SP_Slice;
        }else{
            return SliceType::SI_Slice;
        }
    }
    
    std::string sliceTypeToString(SliceType type){
        std::string result;
        switch (type) {
            case SliceType::P_Slice:
                result = "P_Slice";
                break;
            case SliceType::B_Slice:
                result = "B_Slice";
                break;
            case SliceType::I_Slice:
                result = "I_Slice";
                break;
            case SliceType::SI_Slice:
                result = "SI_Slice";
                break;
            case SliceType::SP_Slice:
                result = "SP_Slice";
                break;
            default:
                break;
        }
        return result;
    }
    
    NalType getNalType(uint8_t* data, unsigned long index){
        NalType type;
        switch(data[index] & 0x1F){
            case 0x01:{
                type = NalType::NAL_Slice;
                break;
            }case 0x02:{
                type = NalType::NAL_PA;
                break;
            }case 0x03:{
                type = NalType::NAL_PB;
                break;
            }case 0x04:{
                type = NalType::NAL_PC;
                break;
            }case 0x05:{
                type = NalType::NAL_IDR_Slice;
                break;
            }case 0x06:{
                type = NalType::NAL_SEI;
                break;
            }case 0x07:{
                type = NalType::NAL_SPS;
                break;
            }case 0x08:{
                type = NalType::NAL_PPS;
                break;
            }case 0x09:{
                type = NalType::NAL_AUD;
                break;
            }default:{
                type = NalType::NAL_ERR;
                break;
            }
        }
        return type;
    }
    
    unsigned long getNalSize(uint8_t* data, unsigned long index){
        return data[index - 1]
        + (data[index - 2] << 8)
        + (data[index - 3] << 16)
        + (data[index - 4] << 24);
    }
    
    void parsePkt(AVPacket* pkt, std::vector<unsigned long>& result){
        unsigned long keyIndex = 4;
        unsigned long currentSegmentLength = 0;
        unsigned long size = pkt->size;
        uint8_t* data = pkt->data;
        if (getNalSize(data, keyIndex) + 4 == size){
            NalType type = getNalType(data, keyIndex);
            if (type == H264Analyst::NalType::NAL_IDR_Slice || type == H264Analyst::NalType::NAL_Slice){
                result.push_back(keyIndex);
            }
            return;
        }
        while (keyIndex < size){
            auto type = getNalType(data, keyIndex);
            if (NalType::NAL_IDR_Slice == type || NalType::NAL_Slice == type){
                result.push_back(keyIndex);
            }
            currentSegmentLength = getNalSize(data, keyIndex);
            keyIndex += currentSegmentLength + 4;
            continue;
        }
    }
}