//
//  main.cpp
//  H264Analyst
//
//  Created by fernando on 16/6/13.
//  Copyright © 2016年 Fernando Wei. All rights reserved.
//

#include "MediaSource.hpp"

int main(int argc, const char * argv[]) {
    
//    std::string url = "/Users/fernando/Library/Application Support/RTXC/accounts/weihongqiang/sessionPhoto/test.flv";
//    std::string url = "http://10.11.10.45/ttkankan/video/test3.flv";
    std::string url = "/Users/fernando/Documents/iOS/testVedio/test_22.flv";
    
    std::unique_ptr<H264Analyst::MediaSource, std::function<void(H264Analyst::MediaSource* source)> > mediaSource(new H264Analyst::MediaSource(std::move(url)), [](H264Analyst::MediaSource* source){
        return source->close();
    });
    
    if (mediaSource->prepareSource() != 0){
        std::cout << "failed to prepare media souce.\n";
        return -1;
    }
    
    mediaSource->dumpParameterSetInfo();
    mediaSource->dumpVclNaluHeaderInfo();
    return 0;
}