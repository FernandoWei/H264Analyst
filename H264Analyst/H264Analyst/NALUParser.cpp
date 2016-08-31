//
//  NALUParser.cpp
//  H264Analyst
//
//  Created by fernando on 16/6/15.
//  Copyright © 2016年 Fernando Wei. All rights reserved.
//

#include "NALUParser.h"
#include <cmath>
#include <algorithm>

namespace H264Analyst {
    
#define INVALID_VALUE (-1)
#define MAX_DUMP_BYTES_NUM 20
    
#define GET_VALUE_FOR_KEY(thiz, key, value){ \
long result = thiz->getValueForKey(key); \
if (result != -1){ \
*value = result;} \
} \


    
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
            if (getNalType(data, keyIndex) == H264Analyst::NalType::NAL_IDR_Slice || getNalType(data, keyIndex) == H264Analyst::NalType::NAL_Slice){
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
    
    h264data::h264data():mData(nullptr), mSize(0){
        informations.clear();
    }
    
    std::string&& sliceTypeToString(SliceType type){
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
        return std::move(result);
    }
    
    void h264data::dumpInfo(){
        std::string name(toString());
        std::cout << "==========" << name << "==========" << std::endl;
        if (mData && mSize){
            for (int i = 0; i < (mSize > MAX_DUMP_BYTES_NUM ? MAX_DUMP_BYTES_NUM : mSize); i++){
                printf("%02x ", *(mData + i));
            }
            if (mSize > MAX_DUMP_BYTES_NUM){
                std::cout << "......";
            }
        }
        std::cout << "\n---------informations---------\n";
        if (!informations.empty()){
            for (auto& item : informations){
                std::cout << item.first << ": " << item.second;
                if (item.first.compare(std::string("slice_type")) == 0){
                    std::string sliceType(sliceTypeToString(getSliceType(item.second)));
                    std::cout << "(" << sliceType << ")";
                }
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;
    }
    
    long h264data::getValueForKey(std::string&& key){
        std::string destString(key);
        for (auto& item : informations){
            if (item.first.compare(destString) == 0){
                return item.second;
            }
        }
        return INVALID_VALUE;
    }
    
    void sps::parse(uint8_t* dataPtr, uint32_t length){
        mData = dataPtr;
        mSize = length;
        
        NALUnit nal(mData, mSize);
        if (nal.Type() != NALUnit::NAL_Sequence_Params){
            std::cout << "It's not sps.\n";
            return;
        }
        nal.Skip(8);
        informations.clear();
        informations.push_back(std::make_pair(std::move(std::string("profile_idc")), nal.GetWord(8)));
        informations.push_back(std::make_pair(std::move(std::string("constraint_set0_flag")), nal.GetBit()));
        informations.push_back(std::make_pair(std::move(std::string("constraint_set1_flag")), nal.GetBit()));
        informations.push_back(std::make_pair(std::move(std::string("constraint_set2_flag")), nal.GetBit()));
        informations.push_back(std::make_pair(std::move(std::string("constraint_set3_flag")), nal.GetBit()));
        informations.push_back(std::make_pair(std::move(std::string("reserved_zero_4bits")), nal.GetWord(4)));
        informations.push_back(std::make_pair(std::move(std::string("level_idc")), nal.GetWord(8)));
        informations.push_back(std::make_pair(std::move(std::string("seq_parameter_set_id")), nal.GetUE()));
        unsigned long profile_idc = 0;
        GET_VALUE_FOR_KEY(this, std::move(std::string("profile_idc")), &profile_idc);
        if (profile_idc == 100 || profile_idc == 110 || profile_idc == 122 || profile_idc == 144){
            informations.push_back(std::make_pair(std::move(std::string("chroma_format_idc")), nal.GetUE()));
            if (informations.back().second == 3){
                informations.push_back(std::make_pair(std::move(std::string("residual_colour_transform_flag")), nal.GetBit()));
            }
            informations.push_back(std::make_pair(std::move(std::string("bit_depth_luma_minus8")), nal.GetUE()));
            informations.push_back(std::make_pair(std::move(std::string("bit_depth_chroma_minus8")), nal.GetUE()));
            informations.push_back(std::make_pair(std::move(std::string("qpprime_y_zero_transform_bypass_flag")), nal.GetBit()));
            informations.push_back(std::make_pair(std::move(std::string("seq_scaling_matrix_present_flag")), nal.GetBit()));
            if (informations.back().second){
                for (int i = 0; i < 8; i++){
                    informations.push_back(std::make_pair(std::move(std::string("seq_scaling_list_present_flag_").append(std::to_string(i))), nal.GetBit()));
                }
            }
        }
        informations.push_back(std::make_pair(std::move(std::string("log2_max_frame_num_minus4")), nal.GetUE()));
        informations.push_back(std::make_pair(std::move(std::string("pic_order_cnt_type")), nal.GetUE()));
        unsigned long pic_order_cnt_type = informations.back().second;
        if (pic_order_cnt_type == 0){
            informations.push_back(std::make_pair(std::move(std::string("log2_max_pic_order_cnt_lsb_minus4")), nal.GetUE()));
        }else if (pic_order_cnt_type == 1){
            informations.push_back(std::make_pair(std::move(std::string("delta_pic_order_always_zero_flag")), nal.GetBit()));
            informations.push_back(std::make_pair(std::move(std::string("offset_for_non_ref_pic")), std::abs(nal.GetSE())));
            informations.push_back(std::make_pair(std::move(std::string("offset_for_top_to_bottom_field")), nal.GetSE()));
            informations.push_back(std::make_pair(std::move(std::string("num_ref_frames_in_pic_order_cnt_cycle")), nal.GetUE()));
            unsigned long num_ref_frames_in_pic_order_cnt_cycle = informations.back().second;
            for (int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++){
                informations.push_back(std::make_pair(std::move(std::string("offset_for_ref_frames_").append(std::to_string(i))), std::abs(nal.GetSE())));
            }
        }
        informations.push_back(std::make_pair(std::move(std::string("num_ref_frames")), nal.GetUE()));
        informations.push_back(std::make_pair(std::move(std::string("gaps_in_frames_num_value_allowed_flag")), nal.GetBit()));
        informations.push_back(std::make_pair(std::move(std::string("pic_width_in_mbs_minus1")), nal.GetUE()));
        informations.push_back(std::make_pair(std::move(std::string("pic_height_in_map_units_minus1")), nal.GetUE()));
        informations.push_back(std::make_pair(std::move(std::string("frame_mbs_only_flag")), nal.GetBit()));
        if (!informations.back().second){
            informations.push_back(std::make_pair(std::move(std::string("mb_adaptive_frame_field_flag")), nal.GetBit()));
        }
        informations.push_back(std::make_pair(std::move(std::string("direct_8x8_inference_flag")), nal.GetBit()));
        informations.push_back(std::make_pair(std::move(std::string("frame_cropping_flag")), nal.GetBit()));
        if (informations.back().second){
            informations.push_back(std::make_pair(std::move(std::string("frame_crop_left_offset")), nal.GetUE()));
            informations.push_back(std::make_pair(std::move(std::string("frame_crop_right_offset")), nal.GetUE()));
            informations.push_back(std::make_pair(std::move(std::string("frame_crop_top_offset")), nal.GetUE()));
            informations.push_back(std::make_pair(std::move(std::string("frame_crop_bottom_offset")), nal.GetUE()));
        }
        informations.push_back(std::make_pair(std::move(std::string("vui_parameters_present_flag")), nal.GetBit()));
    }
    
    std::string&& sps::toString(){
        return std::move(std::string("Sequence Parameter Set"));
    }
    
    void pps::parse(uint8_t* dataPtr, uint32_t length){
        mData = dataPtr;
        mSize = length;
        
        NALUnit nal(mData, mSize);
        if (nal.Type() != NALUnit::NAL_Picture_Params){
            std::cout << "It's not pps.\n";
            return;
        }
        nal.Skip(8);
        informations.push_back(std::make_pair(std::move(std::string("pic_parameter_set_id")), nal.GetUE()));
        informations.push_back(std::make_pair(std::move(std::string("seq_parameter_set_id")), nal.GetUE()));
        informations.push_back(std::make_pair(std::move(std::string("entropy_coding_mode_flag")), nal.GetBit()));
        informations.push_back(std::make_pair(std::move(std::string("pic_order_present_flag")), nal.GetBit()));
        informations.push_back(std::make_pair(std::move(std::string("num_slice_groups_mimus1")), nal.GetUE()));
        unsigned long num_slice_groups_mimus1 = informations.back().second;
        if (num_slice_groups_mimus1 > 0){
            unsigned long slice_group_map_type = nal.GetUE();
            informations.push_back(std::make_pair(std::move(std::string("slice_group_map_type")), slice_group_map_type));
            if (slice_group_map_type == 0){
                for (int i = 0; i < num_slice_groups_mimus1; i++){
                    informations.push_back(std::make_pair(std::move(std::string("run_length_minus1_").append(std::to_string(i))), nal.GetUE()));
                }
            }else if (slice_group_map_type == 2){
                for (int i = 0; i < num_slice_groups_mimus1; i++){
                    informations.push_back(std::make_pair(std::move(std::string("top_left_").append(std::to_string(i))), nal.GetUE()));
                    informations.push_back(std::make_pair(std::move(std::string("bottom_right_").append(std::to_string(i))), nal.GetUE()));
                }
            }else if (slice_group_map_type == 3 || slice_group_map_type == 4 || slice_group_map_type == 5){
                informations.push_back(std::make_pair(std::move(std::string("slice_group_change_direction_flag")), nal.GetBit()));
                informations.push_back(std::make_pair(std::move(std::string("slice_group_change_rate_minus1")), nal.GetUE()));
            }else if (slice_group_map_type == 6){
                unsigned long pic_size_in_map_units_minus1 = nal.GetUE();
                informations.push_back(std::make_pair(std::move(std::string("pic_size_in_map_units_minus1")), pic_size_in_map_units_minus1));
                for (int i = 0; i < pic_size_in_map_units_minus1; i++){
                    informations.push_back(std::make_pair(std::move(std::string("slice_group_id_").append(std::to_string(i))), nal.GetWord(num_slice_groups_mimus1 + 1)));
                }
            }
        }
        informations.push_back(std::make_pair(std::move(std::string("num_ref_idx_l0_active_minus1")), nal.GetUE()));
        informations.push_back(std::make_pair(std::move(std::string("num_ref_idx_l1_active_minus1")), nal.GetUE()));
        informations.push_back(std::make_pair(std::move(std::string("weighted_pred_flag")), nal.GetBit()));
        informations.push_back(std::make_pair(std::move(std::string("weighted_bipred_idc")), nal.GetWord(2)));
        informations.push_back(std::make_pair(std::move(std::string("pic_init_qp_minus26")), std::abs(nal.GetSE())));
        informations.push_back(std::make_pair(std::move(std::string("pic_init_qs_minus26")), std::abs(nal.GetSE())));
        informations.push_back(std::make_pair(std::move(std::string("chroma_qp_index_offset")), std::abs(nal.GetSE())));
        informations.push_back(std::make_pair(std::move(std::string("deblocking_filter_control_present_flag")), nal.GetBit()));
        informations.push_back(std::make_pair(std::move(std::string("constrained_intra_pred_flag")), nal.GetBit()));
        informations.push_back(std::make_pair(std::move(std::string("redundant_pic_cnt_present_flag")), nal.GetBit()));
    }
    
    std::string&& pps::toString(){
        return std::move(std::string("Picture Parameter Set"));
    }
    
    void vcl_nalu::parse(uint8_t* dataPtr, uint32_t length){
        mData = dataPtr;
        mSize = length;

        NALUnit nal(mData + 4, mSize - 4);
        if (!(nal.Type() == NALUnit::NAL_IDR_Slice ||
            nal.Type() == NALUnit::NAL_Slice ||
            nal.Type() == NALUnit::NAL_PartitionA ||
            nal.Type() == NALUnit::NAL_PartitionB ||
            nal.Type() == NALUnit::NAL_PartitionC)){
            std::cout << "It's not vcl nalu.\n";
            return;
        }
        if (length != getNalSize(dataPtr, 4) + 4){
            std::cout << "It's not invalid vcl nalu.\n";
            return;
        }
        
        nal.Skip(8);
        informations.clear();
        informations.push_back(std::make_pair(std::move(std::string("first_mb_in_slice")), nal.GetUE()));
        informations.push_back(std::make_pair(std::move(std::string("slice_type")), nal.GetUE()));
        informations.push_back(std::make_pair(std::move(std::string("pic_parameter_set_id")), nal.GetUE()));
        unsigned long log2_max_frame_num_minus4 = 0;
        GET_VALUE_FOR_KEY(sps, std::move(std::string("log2_max_frame_num_minus4")), &log2_max_frame_num_minus4);
        informations.push_back(std::make_pair(std::move(std::string("frame_num")), nal.GetWord(log2_max_frame_num_minus4 + 4)));
        unsigned long field_pic_flag = 0;
        unsigned long frame_mbs_only_flag = 0;
        GET_VALUE_FOR_KEY(sps, std::move(std::string("frame_mbs_only_flag")), &frame_mbs_only_flag);
        if (!frame_mbs_only_flag){
            field_pic_flag = nal.GetBit();
            informations.push_back(std::make_pair(std::move(std::string("field_pic_flag")), field_pic_flag));
            if (informations.back().second){
                informations.push_back(std::make_pair(std::move(std::string("bottom_field_flag")), nal.GetBit()));
            }
        }
        if (nal.Type() == NALUnit::NAL_IDR_Slice){
            informations.push_back(std::make_pair(std::move(std::string("idr_pic_id")), nal.GetUE()));
        }
        
        unsigned long pic_order_cnt_type = 0;
        GET_VALUE_FOR_KEY(sps, std::move(std::string("pic_order_cnt_type")), &pic_order_cnt_type);
        unsigned long pic_order_present_flag = 0;
        GET_VALUE_FOR_KEY(pps, std::move(std::string("pic_order_present_flag")), &pic_order_present_flag);
        if (pic_order_cnt_type == 0){
            unsigned long log2_max_pic_order_cnt_lsb_minus4 = 0;
            GET_VALUE_FOR_KEY(sps, std::move(std::string("log2_max_pic_order_cnt_lsb_minus4")), &log2_max_pic_order_cnt_lsb_minus4);
            unsigned long maxPicOrderCntLsb = log2_max_pic_order_cnt_lsb_minus4 + 4;
            informations.push_back(std::make_pair(std::move(std::string("pic_order_cnt_lsb")), nal.GetWord(maxPicOrderCntLsb)));

            if (pic_order_present_flag && !field_pic_flag){
                informations.push_back(std::make_pair(std::move(std::string("delta_pic_order_cnt_bottom")), std::abs(nal.GetSE())));
            }
        }
        unsigned long delta_pic_order_always_zero_flag = 0;
        GET_VALUE_FOR_KEY(sps, std::move(std::string("delta_pic_order_always_zero_flag")), &delta_pic_order_always_zero_flag);
        if (pic_order_cnt_type == 1 && !delta_pic_order_always_zero_flag){
            informations.push_back(std::make_pair(std::move(std::string("delta_pic_order_cnt_0")), std::abs(nal.GetSE())));
            if (pic_order_present_flag && !field_pic_flag){
                informations.push_back(std::make_pair(std::move(std::string("delta_pic_order_cnt_1")), std::abs(nal.GetSE())));
            }
        }
        
        unsigned long redundant_pic_cnt_present_flag = 0;
        GET_VALUE_FOR_KEY(pps, std::move(std::string("redundant_pic_cnt_present_flag")), &redundant_pic_cnt_present_flag);
        if (redundant_pic_cnt_present_flag){
            informations.push_back(std::make_pair(std::move(std::string("redundant_pic_cnt")), nal.GetUE()));
        }
        
        unsigned long slice_type = 0;
        GET_VALUE_FOR_KEY(this, std::move(std::string("slice_type")), &slice_type);
        H264Analyst::SliceType sliceType = H264Analyst::getSliceType(slice_type);
        if (sliceType == H264Analyst::SliceType::B_Slice){
            informations.push_back(std::make_pair(std::move(std::string("direct_spatial_mv_pred_flag")), nal.GetBit()));
        }
        if (sliceType == H264Analyst::SliceType::P_Slice ||
            sliceType == H264Analyst::SliceType::SP_Slice ||
            sliceType == H264Analyst::SliceType::B_Slice){
            unsigned long num_ref_idx_active_override_flag = nal.GetBit();
            informations.push_back(std::make_pair(std::move(std::string("num_ref_idx_active_override_flag")), nal.GetBit()));
            if (num_ref_idx_active_override_flag){
                informations.push_back(std::make_pair(std::move(std::string("num_ref_idx_l0_active_minus1")), nal.GetUE()));
                if (sliceType == H264Analyst::SliceType::B_Slice){
                    informations.push_back(std::make_pair(std::move(std::string("num_ref_idx_l1_active_minus1")), nal.GetUE()));
                }
            }
        }
        
        unsigned long entropy_coding_mode_flag = 0;
        GET_VALUE_FOR_KEY(pps, std::move(std::string("entropy_coding_mode_flag")), &entropy_coding_mode_flag);
        if (entropy_coding_mode_flag && sliceType != H264Analyst::SliceType::I_Slice && sliceType != H264Analyst::SliceType::SI_Slice){
            informations.push_back(std::make_pair(std::move(std::string("cabac_init_idc")), nal.GetUE()));
        }
        informations.push_back(std::make_pair(std::move(std::string("slice_qp_delta")), std::abs(nal.GetSE())));
        if (sliceType == H264Analyst::SliceType::SP_Slice || sliceType == H264Analyst::SliceType::SI_Slice){
            if (sliceType == H264Analyst::SliceType::SP_Slice){
                informations.push_back(std::make_pair(std::move(std::string("sp_for_switch_flag")), nal.GetBit()));
                informations.push_back(std::make_pair(std::move(std::string("slice_qs_delta")), std::abs(nal.GetSE())));
            }
            
            unsigned long deblocking_filter_control_present_flag = 0;
            GET_VALUE_FOR_KEY(pps, std::move(std::string("deblocking_filter_control_present_flag")), &deblocking_filter_control_present_flag);
            if (deblocking_filter_control_present_flag){
                unsigned long disable_deblocking_filter_idc = nal.GetUE();
                informations.push_back(std::make_pair(std::move(std::string("disable_deblocking_filter_idc")), disable_deblocking_filter_idc));
                if (disable_deblocking_filter_idc != 1){
                    informations.push_back(std::make_pair(std::move(std::string("slice_alpha_c0_offset_div2")), std::abs(nal.GetSE())));
                    informations.push_back(std::make_pair(std::move(std::string("slice_beta_offset_div2")), std::abs(nal.GetSE())));
                }
            }
            
            unsigned long num_slice_groups_mimus1 = 0;
            unsigned long slice_group_map_type = 0;
            GET_VALUE_FOR_KEY(pps, std::move(std::string("num_slice_groups_mimus1")), &num_slice_groups_mimus1);
            GET_VALUE_FOR_KEY(pps, std::move(std::string("slice_group_map_type")), &slice_group_map_type);
            if (num_slice_groups_mimus1 > 0 && slice_group_map_type >= 3 && slice_group_map_type <= 5){
                unsigned long pic_width_in_mbs_minus1 = 0;
                unsigned long pic_height_in_map_units_minus1 = 0;
                GET_VALUE_FOR_KEY(sps, std::move(std::string("pic_width_in_mbs_minus1")), &pic_width_in_mbs_minus1);
                GET_VALUE_FOR_KEY(sps, std::move(std::string("pic_height_in_map_units_minus1")), &pic_height_in_map_units_minus1);
                unsigned long picSizeInMapUnits = (pic_width_in_mbs_minus1 + 1) * (pic_height_in_map_units_minus1 + 1);
                unsigned long slice_group_change_rate_minus1 = 0;
                GET_VALUE_FOR_KEY(pps, std::move(std::string("slice_group_change_rate_minus1")), &slice_group_change_rate_minus1);
                unsigned long sliceGroupChangeRate = slice_group_change_rate_minus1 + 1;
                informations.push_back(std::make_pair(std::move(std::string("slice_group_change_cycle")), nal.GetWord(std::ceil(std::log2(picSizeInMapUnits/sliceGroupChangeRate + 1)))));
            }
        }
    }
    
    std::string&& vcl_nalu::toString(){
        return std::move(std::string("VCL_NALU"));
    }
}



