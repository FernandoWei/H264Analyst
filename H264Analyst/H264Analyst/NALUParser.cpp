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
   
    
    h264data::h264data():mData(nullptr), mSize(0){
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
        if (!mAllInfoItems.empty()){
            for (auto& item : mAllInfoItems){
                unsigned long value = mCurrentInfoItemMap[item];
                std::cout << item << ": " << value;
                if (item.compare(std::string("slice_type")) == 0){
                    std::string sliceType(std::move(sliceTypeToString(getSliceType(value))));
                    std::cout << "(" << sliceType << ")";
                }
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;
    }
    
    void h264data::resetCurrentInfoItemMap(){
        for (auto& it : mCurrentInfoItemMap){
            it.second = 0;
        }
    }
    
    void h264data::prepareCurrentInfoItemMap(){
        for (auto& it : mAllInfoItems){
            mCurrentInfoItemMap.insert(std::move(std::make_pair(it, 0)));
        }
    }
    
    long h264data::getValueForKey(std::string&& key){
        std::string destString(key);
        for (auto& item : mCurrentInfoItemMap){
            if (item.first.compare(destString) == 0){
                return item.second;
            }
        }
        return INVALID_VALUE;
    }
    
    sps::sps(){
        prepareAllInfoItems();
        prepareCurrentInfoItemMap();
    }
    
    void sps::prepareAllInfoItems(){
        mAllInfoItems.emplace_back("profile_idc");
        mAllInfoItems.emplace_back("constraint_set0_flag");
        mAllInfoItems.emplace_back("constraint_set1_flag");
        mAllInfoItems.emplace_back("constraint_set2_flag");
        mAllInfoItems.emplace_back("constraint_set3_flag");
        mAllInfoItems.emplace_back("reserved_zero_4bits");
        mAllInfoItems.emplace_back("level_idc");
        mAllInfoItems.emplace_back("seq_parameter_set_id");
        mAllInfoItems.emplace_back("chroma_format_idc");
        mAllInfoItems.emplace_back("residual_colour_transform_flag");
        mAllInfoItems.emplace_back("bit_depth_luma_minus8");
        mAllInfoItems.emplace_back("bit_depth_chroma_minus8");
        mAllInfoItems.emplace_back("qpprime_y_zero_transform_bypass_flag");
        mAllInfoItems.emplace_back("seq_scaling_matrix_present_flag");
        mAllInfoItems.emplace_back("log2_max_frame_num_minus4");
        mAllInfoItems.emplace_back("pic_order_cnt_type");
        mAllInfoItems.emplace_back("log2_max_pic_order_cnt_lsb_minus4");
        mAllInfoItems.emplace_back("delta_pic_order_always_zero_flag");
        mAllInfoItems.emplace_back("offset_for_non_ref_pic");
        mAllInfoItems.emplace_back("offset_for_top_to_bottom_field");
        mAllInfoItems.emplace_back("num_ref_frames_in_pic_order_cnt_cycle");
        mAllInfoItems.emplace_back("num_ref_frames");
        mAllInfoItems.emplace_back("gaps_in_frames_num_value_allowed_flag");
        mAllInfoItems.emplace_back("pic_width_in_mbs_minus1");
        mAllInfoItems.emplace_back("pic_height_in_map_units_minus1");
        mAllInfoItems.emplace_back("frame_mbs_only_flag");
        mAllInfoItems.emplace_back("mb_adaptive_frame_field_flag");
        mAllInfoItems.emplace_back("direct_8x8_inference_flag");
        mAllInfoItems.emplace_back("frame_cropping_flag");
        mAllInfoItems.emplace_back("frame_crop_left_offset");
        mAllInfoItems.emplace_back("frame_crop_right_offset");
        mAllInfoItems.emplace_back("frame_crop_top_offset");
        mAllInfoItems.emplace_back("frame_crop_bottom_offset");
        mAllInfoItems.emplace_back("vui_parameters_present_flag");
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
        resetCurrentInfoItemMap();
        
        unsigned long profile_idc = mCurrentInfoItemMap["profile_idc"] = nal.GetWord(8);
        mCurrentInfoItemMap["constraint_set0_flag"] = nal.GetBit();
        mCurrentInfoItemMap["constraint_set1_flag"] = nal.GetBit();
        mCurrentInfoItemMap["constraint_set2_flag"] = nal.GetBit();
        mCurrentInfoItemMap["constraint_set3_flag"] = nal.GetBit();
        mCurrentInfoItemMap["reserved_zero_4bits"] = nal.GetWord(4);
        mCurrentInfoItemMap["level_idc"] = nal.GetWord(8);
        mCurrentInfoItemMap["seq_parameter_set_id"] = nal.GetUE();
        if (profile_idc == 100 || profile_idc == 110 || profile_idc == 122 || profile_idc == 144){
            mCurrentInfoItemMap["chroma_format_idc"] = nal.GetUE();
            if (mCurrentInfoItemMap["chroma_format_idc"] == 3){
                mCurrentInfoItemMap["residual_colour_transform_flag"] = nal.GetBit();
            }
            mCurrentInfoItemMap["bit_depth_luma_minus8"] = nal.GetUE();
            mCurrentInfoItemMap["bit_depth_chroma_minus8"] = nal.GetUE();
            mCurrentInfoItemMap["qpprime_y_zero_transform_bypass_flag"] = nal.GetBit();
            mCurrentInfoItemMap["seq_scaling_matrix_present_flag"] = nal.GetBit();
            if (mCurrentInfoItemMap["seq_scaling_matrix_present_flag"]){
                for (int i = 0; i < 8; i++){
                    std::string str = std::string("seq_scaling_list_present_flag_").append(std::to_string(i));
                    if (mCurrentInfoItemMap.end() != mCurrentInfoItemMap.find(str)){
                        mAllInfoItems.insert(++std::find(mAllInfoItems.begin(), mAllInfoItems.end(), "seq_scaling_matrix_present_flag"), str);
                    }
                    mCurrentInfoItemMap[str] = nal.GetBit();
                }
            }
        }
        mCurrentInfoItemMap["log2_max_frame_num_minus4"] = nal.GetUE();
        unsigned long pic_order_cnt_type = mCurrentInfoItemMap["pic_order_cnt_type"] = nal.GetUE();
        if (pic_order_cnt_type == 0){
            mCurrentInfoItemMap["log2_max_pic_order_cnt_lsb_minus4"] = nal.GetUE();
        }else if (pic_order_cnt_type == 1){
            mCurrentInfoItemMap["delta_pic_order_always_zero_flag"] = nal.GetBit();
            mCurrentInfoItemMap["offset_for_non_ref_pic"] = std::abs(nal.GetSE());
            mCurrentInfoItemMap["offset_for_top_to_bottom_field"] = nal.GetSE();
            unsigned long num_ref_frames_in_pic_order_cnt_cycle = mCurrentInfoItemMap["num_ref_frames_in_pic_order_cnt_cycle"] = nal.GetUE();
            for (int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++){
                nal.GetSE(); //这个值暂不存储
            }
        }
        mCurrentInfoItemMap["num_ref_frames"] = nal.GetUE();
        mCurrentInfoItemMap["gaps_in_frames_num_value_allowed_flag"] = nal.GetBit();
        mCurrentInfoItemMap["pic_width_in_mbs_minus1"] = nal.GetUE();
        mCurrentInfoItemMap["pic_height_in_map_units_minus1"] = nal.GetUE();
        mCurrentInfoItemMap["frame_mbs_only_flag"] = nal.GetBit();
        if (!mCurrentInfoItemMap["frame_mbs_only_flag"]){
            mCurrentInfoItemMap["mb_adaptive_frame_field_flag"] = nal.GetBit();
        }
        mCurrentInfoItemMap["direct_8x8_inference_flag"] = nal.GetBit();
        mCurrentInfoItemMap["frame_cropping_flag"] = nal.GetBit();
        if (mCurrentInfoItemMap["frame_cropping_flag"]){
            mCurrentInfoItemMap["frame_crop_left_offset"] = nal.GetUE();
            mCurrentInfoItemMap["frame_crop_right_offset"] = nal.GetUE();
            mCurrentInfoItemMap["frame_crop_top_offset"] = nal.GetUE();
            mCurrentInfoItemMap["frame_crop_bottom_offset"] = nal.GetUE();
        }
        mCurrentInfoItemMap["vui_parameters_present_flag"] = nal.GetBit();
    }
    
    std::string sps::toString(){
        return std::string("Sequence Parameter Set");
    }
    
    pps::pps(){
        prepareAllInfoItems();
        prepareCurrentInfoItemMap();
    }
    void pps::prepareAllInfoItems(){
        mAllInfoItems.emplace_back("pic_parameter_set_id");
        mAllInfoItems.emplace_back("seq_parameter_set_id");
        mAllInfoItems.emplace_back("entropy_coding_mode_flag");
        mAllInfoItems.emplace_back("pic_order_present_flag");
        mAllInfoItems.emplace_back("num_slice_groups_mimus1");
        mAllInfoItems.emplace_back("slice_group_map_type");
        mAllInfoItems.emplace_back("slice_group_change_direction_flag");
        mAllInfoItems.emplace_back("slice_group_change_rate_minus1");
        mAllInfoItems.emplace_back("pic_size_in_map_units_minus1");
        mAllInfoItems.emplace_back("num_ref_idx_l0_active_minus1");
        mAllInfoItems.emplace_back("num_ref_idx_l1_active_minus1");
        mAllInfoItems.emplace_back("weighted_pred_flag");
        mAllInfoItems.emplace_back("weighted_bipred_idc");
        mAllInfoItems.emplace_back("pic_init_qp_minus26");
        mAllInfoItems.emplace_back("pic_init_qs_minus26");
        mAllInfoItems.emplace_back("chroma_qp_index_offset");
        mAllInfoItems.emplace_back("deblocking_filter_control_present_flag");
        mAllInfoItems.emplace_back("constrained_intra_pred_flag");
        mAllInfoItems.emplace_back("redundant_pic_cnt_present_flag");
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
        resetCurrentInfoItemMap();
        
        mCurrentInfoItemMap["pic_parameter_set_id"] = nal.GetUE();
        mCurrentInfoItemMap["seq_parameter_set_id"] = nal.GetUE();
        mCurrentInfoItemMap["entropy_coding_mode_flag"] = nal.GetBit();
        mCurrentInfoItemMap["pic_order_present_flag"] = nal.GetBit();
        unsigned long num_slice_groups_mimus1 = mCurrentInfoItemMap["num_slice_groups_mimus1"] = nal.GetUE();
        if (num_slice_groups_mimus1 > 0){
            unsigned long slice_group_map_type = mCurrentInfoItemMap["slice_group_map_type"] = nal.GetUE();
            if (slice_group_map_type == 0){
                for (int i = 0; i < num_slice_groups_mimus1; i++){
                    std::string str = std::string("run_length_minus1_").append(std::to_string(i));
                    if (mCurrentInfoItemMap.find(str) != mCurrentInfoItemMap.end()){
                        mAllInfoItems.insert(++std::find(mAllInfoItems.begin(), mAllInfoItems.end(), "slice_group_map_type"), str);
                    }
                    mCurrentInfoItemMap[str] = nal.GetUE();
                }
            }else if (slice_group_map_type == 2){
                for (int i = 0; i < num_slice_groups_mimus1; i++){
                    std::string str1 = std::string("top_left_").append(std::to_string(i));
                    if (mCurrentInfoItemMap.find(str1) != mCurrentInfoItemMap.end()){
                        mAllInfoItems.insert(++std::find(mAllInfoItems.begin(), mAllInfoItems.end(), "slice_group_map_type"), str1);
                    }
                    mCurrentInfoItemMap[str1] = nal.GetUE();
                    
                    std::string str2 = std::string("bottom_right_").append(std::to_string(i));
                    if (mCurrentInfoItemMap.find(str2) != mCurrentInfoItemMap.end()){
                        mAllInfoItems.insert(++std::find(mAllInfoItems.begin(), mAllInfoItems.end(), "slice_group_map_type"), str2);
                    }
                    mCurrentInfoItemMap[str2] = nal.GetUE();
                }
            }else if (slice_group_map_type == 3 || slice_group_map_type == 4 || slice_group_map_type == 5){
                mCurrentInfoItemMap["slice_group_change_direction_flag"] = nal.GetBit();
                mCurrentInfoItemMap["slice_group_change_rate_minus1"] = nal.GetUE();
            }else if (slice_group_map_type == 6){
                unsigned long pic_size_in_map_units_minus1 = mCurrentInfoItemMap["pic_size_in_map_units_minus1"] = nal.GetUE();
                for (int i = 0; i < pic_size_in_map_units_minus1; i++){
                    std::string str = std::string("slice_group_id_").append(std::to_string(i));
                    if (mCurrentInfoItemMap.end() != mCurrentInfoItemMap.find(str)){
                        mAllInfoItems.insert(++std::find(mAllInfoItems.begin(), mAllInfoItems.end(), "pic_size_in_map_units_minus1"), str);
                    }
                    mCurrentInfoItemMap[str] = nal.GetWord(num_slice_groups_mimus1 + 1);
                }
            }
        }
        mCurrentInfoItemMap["num_ref_idx_l0_active_minus1"] = nal.GetUE();
        mCurrentInfoItemMap["num_ref_idx_l1_active_minus1"] = nal.GetUE();
        mCurrentInfoItemMap["weighted_pred_flag"] = nal.GetBit();
        mCurrentInfoItemMap["weighted_bipred_idc"] = nal.GetWord(2);
        mCurrentInfoItemMap["pic_init_qp_minus26"] = std::abs(nal.GetSE());
        mCurrentInfoItemMap["pic_init_qs_minus26"] = std::abs(nal.GetSE());
        mCurrentInfoItemMap["chroma_qp_index_offset"] = std::abs(nal.GetSE());
        mCurrentInfoItemMap["deblocking_filter_control_present_flag"] = nal.GetBit();
        mCurrentInfoItemMap["constrained_intra_pred_flag"] = nal.GetBit();
        mCurrentInfoItemMap["redundant_pic_cnt_present_flag"] = nal.GetBit();
    }
    
    std::string pps::toString(){
        return std::string("Picture Parameter Set");
    }
    
    vcl_nalu::vcl_nalu(){
        prepareAllInfoItems();
        prepareCurrentInfoItemMap();
    }
    
    void vcl_nalu::prepareAllInfoItems(){
        mAllInfoItems.emplace_back("first_mb_in_slice");
        mAllInfoItems.emplace_back("slice_type");
        mAllInfoItems.emplace_back("pic_parameter_set_id");
        mAllInfoItems.emplace_back("frame_num");
        mAllInfoItems.emplace_back("field_pic_flag");
        mAllInfoItems.emplace_back("bottom_field_flag");
        mAllInfoItems.emplace_back("idr_pic_id");
        mAllInfoItems.emplace_back("pic_order_cnt_lsb");
        mAllInfoItems.emplace_back("delta_pic_order_cnt_bottom");
        mAllInfoItems.emplace_back("delta_pic_order_cnt_0");
        mAllInfoItems.emplace_back("delta_pic_order_cnt_1");
        mAllInfoItems.emplace_back("redundant_pic_cnt");
        mAllInfoItems.emplace_back("direct_spatial_mv_pred_flag");
        mAllInfoItems.emplace_back("num_ref_idx_active_override_flag");
        mAllInfoItems.emplace_back("num_ref_idx_l0_active_minus1");
        mAllInfoItems.emplace_back("num_ref_idx_l1_active_minus1");
        mAllInfoItems.emplace_back("cabac_init_idc");
        mAllInfoItems.emplace_back("slice_qp_delta");
        mAllInfoItems.emplace_back("sp_for_switch_flag");
        mAllInfoItems.emplace_back("slice_qs_delta");
        mAllInfoItems.emplace_back("disable_deblocking_filter_idc");
        mAllInfoItems.emplace_back("slice_alpha_c0_offset_div2");
        mAllInfoItems.emplace_back("slice_beta_offset_div2");
        mAllInfoItems.emplace_back("slice_group_change_cycle");
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
        resetCurrentInfoItemMap();
        
        mCurrentInfoItemMap["first_mb_in_slice"] = nal.GetUE();
        mCurrentInfoItemMap["slice_type"] = nal.GetUE();
        mCurrentInfoItemMap["pic_parameter_set_id"] = nal.GetUE();
        mCurrentInfoItemMap["frame_num"] = nal.GetWord(sps->getValueForKey(std::move(std::string("log2_max_frame_num_minus4"))) + 4);
        unsigned long field_pic_flag = 0;
        if (!sps->getValueForKey(std::move(std::string("frame_mbs_only_flag")))){
            field_pic_flag = mCurrentInfoItemMap["field_pic_flag"] = nal.GetBit();
            if (field_pic_flag){
                mCurrentInfoItemMap["bottom_field_flag"] =  nal.GetBit();
            }
        }
        if (nal.Type() == NALUnit::NAL_IDR_Slice){
            mCurrentInfoItemMap["idr_pic_id"] = nal.GetUE();
        }
        
        unsigned long pic_order_cnt_type = sps->getValueForKey(std::move(std::string("pic_order_cnt_type")));
        unsigned long pic_order_present_flag = pps->getValueForKey(std::move(std::string("pic_order_present_flag")));
        if (pic_order_cnt_type == 0){
            mCurrentInfoItemMap["pic_order_cnt_lsb"] = nal.GetWord(sps->getValueForKey(std::move(std::string("log2_max_pic_order_cnt_lsb_minus4"))) + 4);
            if (pic_order_present_flag && !field_pic_flag){
                mCurrentInfoItemMap["delta_pic_order_cnt_bottom"] = std::abs(nal.GetSE());
            }
        }
        unsigned long delta_pic_order_always_zero_flag = sps->getValueForKey(std::move(std::string("delta_pic_order_always_zero_flag")));
        if (pic_order_cnt_type == 1 && !delta_pic_order_always_zero_flag){
            mCurrentInfoItemMap["delta_pic_order_cnt_0"] = std::abs(nal.GetSE());
            if (pic_order_present_flag && !field_pic_flag){
                mCurrentInfoItemMap["delta_pic_order_cnt_1"] = std::abs(nal.GetSE());
            }
        }
        
        if (pps->getValueForKey(std::move(std::string("redundant_pic_cnt_present_flag")))){
            mCurrentInfoItemMap["redundant_pic_cnt"] = nal.GetUE();
        }
        
        H264Analyst::SliceType sliceType = H264Analyst::getSliceType(mCurrentInfoItemMap["slice_type"]);
        if (sliceType == H264Analyst::SliceType::B_Slice){
            mCurrentInfoItemMap["direct_spatial_mv_pred_flag"] = nal.GetBit();
        }
        if (sliceType == H264Analyst::SliceType::P_Slice ||
            sliceType == H264Analyst::SliceType::SP_Slice ||
            sliceType == H264Analyst::SliceType::B_Slice){
            unsigned long num_ref_idx_active_override_flag = mCurrentInfoItemMap["num_ref_idx_active_override_flag"] = nal.GetBit();
            if (num_ref_idx_active_override_flag){
                mCurrentInfoItemMap["num_ref_idx_l0_active_minus1"] = nal.GetUE();
                if (sliceType == H264Analyst::SliceType::B_Slice){
                    mCurrentInfoItemMap["num_ref_idx_l1_active_minus1"] = nal.GetUE();
                }
            }
        }
        
        if (pps->getValueForKey(std::move(std::string("entropy_coding_mode_flag"))) && sliceType != H264Analyst::SliceType::I_Slice && sliceType != H264Analyst::SliceType::SI_Slice){
            mCurrentInfoItemMap["cabac_init_idc"] = nal.GetUE();
        }
        mCurrentInfoItemMap["slice_qp_delta"] = std::abs(nal.GetSE());
        if (sliceType == H264Analyst::SliceType::SP_Slice || sliceType == H264Analyst::SliceType::SI_Slice){
            if (sliceType == H264Analyst::SliceType::SP_Slice){
                mCurrentInfoItemMap["sp_for_switch_flag"] = nal.GetBit();
                mCurrentInfoItemMap["slice_qs_delta"] = std::abs(nal.GetSE());
            }
            
            if (pps->getValueForKey(std::move(std::string("deblocking_filter_control_present_flag")))){
                unsigned long disable_deblocking_filter_idc = mCurrentInfoItemMap["disable_deblocking_filter_idc"] = nal.GetUE();
                if (disable_deblocking_filter_idc != 1){
                    mCurrentInfoItemMap["slice_alpha_c0_offset_div2"] = std::abs(nal.GetSE());
                    mCurrentInfoItemMap["slice_beta_offset_div2"] = std::abs(nal.GetSE());
                }
            }
            
            unsigned long slice_group_map_type = pps->getValueForKey(std::move(std::string("slice_group_map_type")));
            if (pps->getValueForKey(std::move(std::string("num_slice_groups_mimus1"))) > 0 && slice_group_map_type >= 3 && slice_group_map_type <= 5){
                unsigned long pic_width_in_mbs_minus1 = sps->getValueForKey(std::move(std::string("pic_width_in_mbs_minus1")));
                unsigned long pic_height_in_map_units_minus1 = sps->getValueForKey(std::move(std::string("pic_height_in_map_units_minus1")));
                unsigned long picSizeInMapUnits = (pic_width_in_mbs_minus1 + 1) * (pic_height_in_map_units_minus1 + 1);
                unsigned long slice_group_change_rate_minus1 = pps->getValueForKey(std::move(std::string("slice_group_change_rate_minus1")));
                unsigned long sliceGroupChangeRate = slice_group_change_rate_minus1 + 1;
                mCurrentInfoItemMap["slice_group_change_cycle"] = nal.GetWord(std::ceil(std::log2(picSizeInMapUnits/sliceGroupChangeRate + 1)));
            }
        }
    }
    
    std::string vcl_nalu::toString(){
        return std::string("VCL_NALU");
    }
}



