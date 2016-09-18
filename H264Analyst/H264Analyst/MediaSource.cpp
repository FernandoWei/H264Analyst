//
//  MediaSource.cpp
//  H264Analyst
//
//  Created by fernando on 16/8/31.
//  Copyright © 2016年 Fernando Wei. All rights reserved.
//

#include "MediaSource.hpp"

namespace H264Analyst {
    MediaSource::MediaSource(std::string&& url):
    mURL(url),
    mPacket(nullptr),
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
        mPacket = av_packet_alloc();
    }
    
    int MediaSource::prepareSource(){
        init();
        int result = open();
        if (result == 0){
            result = extractParameterSet();
        }
        return result;
    }
    
    int MediaSource::open(){
        if (avformat_open_input(&mFormatCtx, mURL.c_str(), nullptr, nullptr) == 0){
            if (avformat_find_stream_info(mFormatCtx, nullptr) >= 0){
                for (int i = 0; i < mFormatCtx->nb_streams; i++){
                    if (mFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
                        mVideoStreamIndex = i;
                        break;
                    }
                }
                
                if (-1 == mVideoStreamIndex){
                    std::cout << "failed to find video stream!\n";
                    return -1;
                }
                
                mCodecCtx = mFormatCtx->streams[mVideoStreamIndex]->codec;
                if (mCodecCtx->codec_id != AV_CODEC_ID_H264){
                    std::cout << "It's not H264 coded video.\n";
                    return -1;
                }
            }
        }
        return 0;
    }
    
    int MediaSource::extractParameterSet(){
        int result = -1;
        if (mCodecCtx && mCodecCtx->extradata){
            if (mCodecCtx->extradata[0] == 0x01){
                std::cout << "Extradata:";
                for (int i = 0; i < mCodecCtx->extradata_size; i++){
                    printf("%02x ", mCodecCtx->extradata[i]);
                }
                std::cout << "\n";
                AVBitStreamFilterContext* bsfc = av_bitstream_filter_init("h264_mp4toannexb");
                uint8_t *dummy = nullptr;
                int dummy_len = -1;
                av_bitstream_filter_filter(bsfc, mCodecCtx, nullptr, &dummy, &dummy_len, mCodecCtx->extradata, 0, 0);
                uint8_t* data_sps_pps = mCodecCtx->extradata;
                int size_sps_pps = mCodecCtx->extradata_size;
                const std::array<uint8_t, 4> KStartCode{0, 0, 0, 1};
                if(memcmp(data_sps_pps, KStartCode.data(), 4) == 0){
                    uint8_t* buffBegin = data_sps_pps + 4;
                    uint8_t* buffEnd = data_sps_pps + size_sps_pps - 1;
                    while (buffBegin != buffEnd){
                        if(*buffBegin == 0x01){
                            if(memcmp(buffBegin - 3, KStartCode.data(), 4) == 0){
                                mSPSData.resize(buffBegin - 3 - data_sps_pps - 4);
                                memcpy(mSPSData.data(), data_sps_pps + 4, buffBegin - 3 - data_sps_pps - 4);
                                break;
                            }
                        }
                        ++buffBegin;
                    }
                    mPPSData.resize(buffEnd - buffBegin);
                    memcpy(mPPSData.data(), buffBegin + 1, buffEnd - buffBegin);
                }
                free(dummy);
                av_bitstream_filter_close(bsfc);
                result = 0;
            }else {
                std::cout << "Not avcC sequence header contained in codec->extradata.\n";
            }
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
            int result = av_read_frame(mFormatCtx, mPacket);
            if (result >= 0 && mPacket->stream_index == mVideoStreamIndex){
                extractNaluFromPkt();
                parseNaluAndDumpInfo();
            }
            
            if (result == AVERROR_EOF || result == AVERROR(EIO)){
                std::cout << "end of stream.\n";
                break;
            }
        }
        
    }
    
    void MediaSource::extractNaluFromPkt(){
            mExtractedNalus.clear();
            H264Analyst::parsePkt(mPacket, mExtractedNalus);
    }
    
    void MediaSource::parseNaluAndDumpInfo(){
        if (!mExtractedNalus.empty()){
            for (auto& item : mExtractedNalus){
                switch (H264Analyst::getNalType(mPacket->data, item)){
                    case H264Analyst::NalType::NAL_Slice:
                    case H264Analyst::NalType::NAL_IDR_Slice:{
                        mVclNalu->parse(mPacket->data + item - 4, H264Analyst::getNalSize(mPacket->data, item) + 4);
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
        if (mPacket){
            av_packet_unref(mPacket);
        }
    }
}