//
//  MediaSource.hpp
//  H264Analyst
//
//  Created by fernando on 16/8/31.
//  Copyright © 2016年 Fernando Wei. All rights reserved.
//

#ifndef MediaSource_hpp
#define MediaSource_hpp

#include "NALUParser.h"
#include <iostream>
#include <vector>
#include <array>

namespace H264Analyst {
    
    class MediaSource{
        
    public:
        MediaSource() = delete;
        MediaSource(std::string&& url);
        
    public:
        int prepareSource();
        void close();
        void dumpParameterSetInfo();
        void dumpVclNaluHeaderInfo();
        
    private:
        void init();
        int open();
        int extractParameterSet();
        void extractNaluFromPkt();
        void parseNaluAndDumpInfo();
        void parseParameterSet();
 
    private:
        std::string mURL;
        AVCodecContext* mCodecCtx;
        AVFormatContext* mFormatCtx;
        int mVideoStreamIndex;
        AVPacket* mPacket;
        std::vector<uint8_t> mSPSData;
        std::vector<uint8_t> mPPSData;
        std::shared_ptr<H264Analyst::sps> mSPS;
        std::shared_ptr<H264Analyst::pps> mPPS;
        std::shared_ptr<H264Analyst::vcl_nalu> mVclNalu;
        std::vector<unsigned long> mExtractedNalus;
    };
}

#endif /* MediaSource_hpp */
