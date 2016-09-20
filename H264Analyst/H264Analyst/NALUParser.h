//
//  NALUParser.h
//  H264Analyst
//
//  Created by fernando on 16/6/15.
//  Copyright © 2016年 Fernando Wei. All rights reserved.
//

#ifndef NALUParser_h
#define NALUParser_h

#include "NALUnit.h"
#include "Utinity.hpp"
#include <vector>
#include <array>
#include <map>
#include <iostream>

namespace H264Analyst {
    
    class h264data{
    public:
        h264data();
        virtual ~h264data(){}
        virtual void parse(uint8_t* dataPtr, uint32_t length) = 0;
        virtual std::string toString() = 0;
        virtual void prepareAllInfoItems() = 0;
    public:
        void dumpInfo();
        long getValueForKey(std::string&& key);
        void resetCurrentInfoItemMap();
        void prepareCurrentInfoItemMap();
    public:
        uint8_t* mData;
        uint32_t mSize;
    protected:
        std::vector<std::string> mAllInfoItems;
        std::map<std::string, unsigned long> mCurrentInfoItemMap;
    };
    
    
    class sps : public h264data{
    public:
        sps();
        virtual ~sps(){}
        
        virtual void parse(uint8_t* dataPtr, uint32_t length);
        virtual std::string toString();
        virtual void prepareAllInfoItems();
        
    };

    class pps : public h264data{
    public:
        pps();
        virtual ~pps(){}
        
        virtual void parse(uint8_t* dataPtr, uint32_t length);
        virtual std::string toString();
        virtual void prepareAllInfoItems();
    };
    
    class vcl_nalu : public h264data{
    public:
        vcl_nalu();
        virtual ~vcl_nalu(){}
        virtual void parse(uint8_t* dataPtr, uint32_t length);
        virtual std::string toString();
        virtual void prepareAllInfoItems();

    public:
        std::shared_ptr<H264Analyst::sps> sps;
        std::shared_ptr<H264Analyst::pps> pps;
    };
}




#endif /* NALUParser_h */
