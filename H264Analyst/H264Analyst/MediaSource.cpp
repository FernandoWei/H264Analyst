//
//  MediaSource.cpp
//  H264Analyst
//
//  Created by fernando on 16/8/31.
//  Copyright © 2016年 Fernando Wei. All rights reserved.
//

#include "MediaSource.hpp"

#define START_CODE_LENGTH 4
#define INVALID_VIDEO_STREAM_INDEX (-1)

namespace H264Analyst {
    MediaSource::MediaSource(std::string&& url):
    mURL(url),
    mCodecCtx(nullptr),
    mFormatCtx(nullptr){
        mSPS = std::make_shared<H264Analyst::sps>();
        mPPS = std::make_shared<H264Analyst::pps>();
        mVclNalu = std::make_shared<H264Analyst::vcl_nalu>();
    }
    
    void MediaSource::init(){
        av_register_all();
        avformat_network_init();
        mFormatCtx = avformat_alloc_context();
        av_init_packet(&mPacket);
    }
    
    ResultType MediaSource::prepareSource(){
        init();
        ResultType result = ResultType::ERROR;
        if (ResultType::OK == open()){
            result = extractParameterSet();
        }
        return result;
    }
    
    ResultType MediaSource::open(){
        if (avformat_open_input(&mFormatCtx, mURL.c_str(), nullptr, nullptr) == 0){
            if (avformat_find_stream_info(mFormatCtx, nullptr) >= 0){
                for (int i = 0; i < mFormatCtx->nb_streams; i++){
                    if (mFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
                        mVideoStreamIndex = i;
                        break;
                    }
                }
                
                if (INVALID_VIDEO_STREAM_INDEX == mVideoStreamIndex){
                    std::cout << "failed to find video stream!\n";
                    return ResultType::ERROR;
                }
                
                mCodecCtx = mFormatCtx->streams[mVideoStreamIndex]->codec;
                if (mCodecCtx->codec_id != AV_CODEC_ID_H264){
                    std::cout << "It's not H264 coded video.\n";
                    return ResultType::ERROR;
                }
            }
        }
        return ResultType::OK;
    }
    
    ResultType MediaSource::extractParameterSet(){
        ResultType result = ResultType::ERROR;
        if (mCodecCtx && mCodecCtx->extradata && mCodecCtx->extradata[0] == 0x01 && mCodecCtx->extradata_size > 8){
            uint8_t spsOffsetIndex = 8;
            uint8_t spsLength = mCodecCtx->extradata[spsOffsetIndex - 1];
            mSPSData.resize(spsLength);
            memcpy(mSPSData.data(), &mCodecCtx->extradata[spsOffsetIndex], spsLength);
            uint8_t ppsOffsetIndex = spsOffsetIndex + spsLength + 3;
            uint8_t ppsLength = mCodecCtx->extradata[ppsOffsetIndex - 1];
            mPPSData.resize(ppsLength);
            memcpy(mPPSData.data(), &mCodecCtx->extradata[ppsOffsetIndex], ppsLength);
            result = ResultType::OK;
        }else{
            std::cout << "invalid codecCtx or codecCtx->extradata.\n";
        }
        return result;
    }
    
    void MediaSource::parseParameterSet(){
        mSPS->parse(mSPSData.data(), mSPSData.size());
        mPPS->parse(mPPSData.data(), mPPSData.size());
        mVclNalu->sps = mSPS;
        mVclNalu->pps = mPPS;
    }
    
    void MediaSource::dumpParameterSetInfo(){
        parseParameterSet();
        mSPS->dumpInfo();
        mPPS->dumpInfo();
    }
    
    void MediaSource::dumpVclNaluHeaderInfo(){
        while (true){
            int result = av_read_frame(mFormatCtx, &mPacket);
            if (result == 0 && mPacket.stream_index == mVideoStreamIndex){
                extractNaluFromPkt();
                parseNaluAndDumpInfo();
            }
            av_packet_unref(&mPacket);
            
            if (result == AVERROR_EOF || result == AVERROR(EIO)){
                std::cout << "end of stream.\n";
                break;
            }
        }
        
    }
    
    void MediaSource::extractNaluFromPkt(){
            mExtractedNalus.clear();
            H264Analyst::parsePkt(&mPacket, mExtractedNalus);
    }
    
    void MediaSource::parseNaluAndDumpInfo(){
        if (!mExtractedNalus.empty()){
            for (auto& item : mExtractedNalus){
                switch (H264Analyst::getNalType(mPacket.data, item)){
                    case H264Analyst::NalType::NAL_Slice:
                    case H264Analyst::NalType::NAL_IDR_Slice:{
                        mVclNalu->parse(mPacket.data + item - START_CODE_LENGTH, H264Analyst::getNalSize(mPacket.data, item) + START_CODE_LENGTH);
                        mVclNalu->dumpInfo();
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }
    
    void MediaSource::close(){
        if (mCodecCtx){
           avcodec_close(mCodecCtx);
        }
        if (mFormatCtx){
            avformat_close_input(&mFormatCtx);
        }
    }
}