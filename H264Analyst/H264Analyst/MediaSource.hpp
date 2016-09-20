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

namespace H264Analyst {
    
    class MediaSource{
        
    public:
        MediaSource() = delete;
        MediaSource(std::string&& url);
        
    public:
        ResultType prepareSource();
        void close();
        void dumpParameterSetInfo();
        void dumpVclNaluHeaderInfo();
        
    private:
        void init();
        ResultType open();
        ResultType extractParameterSet();
        void extractNaluFromPkt();
        void parseNaluAndDumpInfo();
        void parseParameterSet();
 
    private:
        std::string mURL;
        AVCodecContext* mCodecCtx;
        AVFormatContext* mFormatCtx;
        int mVideoStreamIndex;
        std::vector<uint8_t> mSPSData;
        std::vector<uint8_t> mPPSData;
        std::shared_ptr<H264Analyst::sps> mSPS;
        std::shared_ptr<H264Analyst::pps> mPPS;
        std::shared_ptr<H264Analyst::vcl_nalu> mVclNalu;
        std::vector<unsigned long> mExtractedNalus;
        AVPacket mPacket;
    };
}

#endif /* MediaSource_hpp */
