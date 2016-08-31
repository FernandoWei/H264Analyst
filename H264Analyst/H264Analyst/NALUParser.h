//
//  NALUParser.h
//  H264Analyst
//
//  Created by fernando on 16/6/15.
//  Copyright © 2016年 Fernando Wei. All rights reserved.
//

#ifndef NALUParser_h
#define NALUParser_h

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
}

#include "NALUnit.h"
#include <vector>
#include <array>
#include <map>
#include <iostream>

namespace H264Analyst {
    
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
    
    void parsePkt(AVPacket* pkt, std::vector<unsigned long>& results);
    unsigned long getNalSize(uint8_t* data, unsigned long index);
    NalType getNalType(uint8_t* data, unsigned long index);
    
    class h264data{
    public:
        h264data();
        virtual ~h264data(){}
        virtual void parse(uint8_t* dataPtr, uint32_t length) = 0;
        virtual std::string&& toString() = 0;
    public:
        void dumpInfo();
        unsigned long getValueForKey(std::string&& key);
    public:
        uint8_t* mData;
        uint32_t mSize;
    protected:
        std::vector<std::pair<std::string, unsigned long> > informations;
    };
    
    
    class sps : public h264data{
    public:
        virtual ~sps(){}
        
        virtual void parse(uint8_t* dataPtr, uint32_t length);
        virtual std::string&& toString();
        
    };

    class pps : public h264data{
    public:
        virtual ~pps(){}
        
        virtual void parse(uint8_t* dataPtr, uint32_t length);
        virtual std::string&& toString();
    };
    
    class vcl_nalu : public h264data{
    public:
        virtual ~vcl_nalu(){}
        virtual void parse(uint8_t* dataPtr, uint32_t length);
        virtual std::string&& toString();

    public:
        std::shared_ptr<H264Analyst::sps> sps;
        std::shared_ptr<H264Analyst::pps> pps;
    };
}




#endif /* NALUParser_h */
