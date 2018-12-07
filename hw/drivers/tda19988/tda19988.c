/*
 * memwa2 tda19988 driver
 *
 * Copyright (c) 2016 Mathias Edman <mail@dicetec.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 * USA
 *
 */


/**
 * Handles all communication with the HDMI transmitter.
 */

#include "tda19988.h"
#include "config.h"
#include "stm32h7xx_hal_i2c.h"

#define I2C_CEC_ADDRESS    0x68
#define I2C_HDMI_ADDRESS   0xE0
//#define I2C_ADDRESS      0x70
#define I2C_OWN_ADDRESS    0x00
//#define I2C_TIMING       0x40912732
#define I2C_TIMING       0x10C0ECFF

#define MKREG(page, addr)   (((page) << 8) | (addr))

#define REGPAGE(reg)        (((reg) >> 8) & 0xff)
#define REGADDR(reg)        ((reg) & 0xff)

#define TDA_VERSION     MKREG(0x00, 0x00)
#define TDA_MAIN_CNTRL0     MKREG(0x00, 0x01)
#define     MAIN_CNTRL0_SR      (1 << 0)
#define TDA_VERSION_MSB     MKREG(0x00, 0x02)
#define TDA_SOFTRESET       MKREG(0x00, 0x0a)
#define     SOFTRESET_I2C       (1 << 1)
#define     SOFTRESET_AUDIO     (1 << 0)
#define TDA_DDC_CTRL        MKREG(0x00, 0x0b)
#define     DDC_ENABLE      0
#define TDA_CCLK        MKREG(0x00, 0x0c)
#define     CCLK_ENABLE     1
#define TDA_INT_FLAGS_2     MKREG(0x00, 0x11)
#define     INT_FLAGS_2_EDID_BLK_RD (1 << 1)

#define TDA_VIP_CNTRL_0     MKREG(0x00, 0x20)
#define TDA_VIP_CNTRL_1     MKREG(0x00, 0x21)
#define TDA_VIP_CNTRL_2     MKREG(0x00, 0x22)
#define TDA_VIP_CNTRL_3     MKREG(0x00, 0x23)
#define     VIP_CNTRL_3_SYNC_HS (2 << 4)
#define     VIP_CNTRL_3_V_TGL   (1 << 2)
#define     VIP_CNTRL_3_H_TGL   (1 << 1)

#define TDA_VIP_CNTRL_4     MKREG(0x00, 0x24)
#define     VIP_CNTRL_4_BLANKIT_NDE     (0 << 2)
#define     VIP_CNTRL_4_BLANKIT_HS_VS   (1 << 2)
#define     VIP_CNTRL_4_BLANKIT_NHS_VS  (2 << 2)
#define     VIP_CNTRL_4_BLANKIT_HE_VE   (3 << 2)
#define     VIP_CNTRL_4_BLC_NONE        (0 << 0)
#define     VIP_CNTRL_4_BLC_RGB444      (1 << 0)
#define     VIP_CNTRL_4_BLC_YUV444      (2 << 0)
#define     VIP_CNTRL_4_BLC_YUV422      (3 << 0)
#define TDA_VIP_CNTRL_5     MKREG(0x00, 0x25)
#define     VIP_CNTRL_5_SP_CNT(n)   (((n) & 3) << 1)
#define TDA_MUX_VP_VIP_OUT  MKREG(0x00, 0x27)
#define TDA_MAT_CONTRL      MKREG(0x00, 0x80)
#define     MAT_CONTRL_MAT_BP   (1 << 2)
#define TDA_VIDFORMAT       MKREG(0x00, 0xa0)
#define TDA_REFPIX_MSB      MKREG(0x00, 0xa1)
#define TDA_REFPIX_LSB      MKREG(0x00, 0xa2)
#define TDA_REFLINE_MSB     MKREG(0x00, 0xa3)
#define TDA_REFLINE_LSB     MKREG(0x00, 0xa4)
#define TDA_NPIX_MSB        MKREG(0x00, 0xa5)
#define TDA_NPIX_LSB        MKREG(0x00, 0xa6)
#define TDA_NLINE_MSB       MKREG(0x00, 0xa7)
#define TDA_NLINE_LSB       MKREG(0x00, 0xa8)
#define TDA_VS_LINE_STRT_1_MSB  MKREG(0x00, 0xa9)
#define TDA_VS_LINE_STRT_1_LSB  MKREG(0x00, 0xaa)
#define TDA_VS_PIX_STRT_1_MSB   MKREG(0x00, 0xab)
#define TDA_VS_PIX_STRT_1_LSB   MKREG(0x00, 0xac)
#define TDA_VS_LINE_END_1_MSB   MKREG(0x00, 0xad)
#define TDA_VS_LINE_END_1_LSB   MKREG(0x00, 0xae)
#define TDA_VS_PIX_END_1_MSB    MKREG(0x00, 0xaf)
#define TDA_VS_PIX_END_1_LSB    MKREG(0x00, 0xb0)
#define TDA_VS_LINE_STRT_2_MSB  MKREG(0x00, 0xb1)
#define TDA_VS_LINE_STRT_2_LSB  MKREG(0x00, 0xb2)
#define TDA_VS_PIX_STRT_2_MSB   MKREG(0x00, 0xb3)
#define TDA_VS_PIX_STRT_2_LSB   MKREG(0x00, 0xb4)
#define TDA_VS_LINE_END_2_MSB   MKREG(0x00, 0xb5)
#define TDA_VS_LINE_END_2_LSB   MKREG(0x00, 0xb6)
#define TDA_VS_PIX_END_2_MSB    MKREG(0x00, 0xb7)
#define TDA_VS_PIX_END_2_LSB    MKREG(0x00, 0xb8)
#define TDA_HS_PIX_START_MSB    MKREG(0x00, 0xb9)
#define TDA_HS_PIX_START_LSB    MKREG(0x00, 0xba)
#define TDA_HS_PIX_STOP_MSB MKREG(0x00, 0xbb)
#define TDA_HS_PIX_STOP_LSB MKREG(0x00, 0xbc)
#define TDA_VWIN_START_1_MSB    MKREG(0x00, 0xbd)
#define TDA_VWIN_START_1_LSB    MKREG(0x00, 0xbe)
#define TDA_VWIN_END_1_MSB  MKREG(0x00, 0xbf)
#define TDA_VWIN_END_1_LSB  MKREG(0x00, 0xc0)
#define TDA_VWIN_START_2_MSB    MKREG(0x00, 0xc1)
#define TDA_VWIN_START_2_LSB    MKREG(0x00, 0xc2)
#define TDA_VWIN_END_2_MSB  MKREG(0x00, 0xc3)
#define TDA_VWIN_END_2_LSB  MKREG(0x00, 0xc4)
#define TDA_DE_START_MSB    MKREG(0x00, 0xc5)
#define TDA_DE_START_LSB    MKREG(0x00, 0xc6)
#define TDA_DE_STOP_MSB     MKREG(0x00, 0xc7)
#define TDA_DE_STOP_LSB     MKREG(0x00, 0xc8)

#define TDA_TBG_CNTRL_0     MKREG(0x00, 0xca)
#define     TBG_CNTRL_0_SYNC_ONCE   (1 << 7)
#define     TBG_CNTRL_0_SYNC_MTHD   (1 << 6)

#define TDA_TBG_CNTRL_1     MKREG(0x00, 0xcb)
#define     TBG_CNTRL_1_DWIN_DIS    (1 << 6)
#define     TBG_CNTRL_1_TGL_EN  (1 << 2)
#define     TBG_CNTRL_1_V_TGL   (1 << 1)
#define     TBG_CNTRL_1_H_TGL   (1 << 0)

#define TDA_HVF_CNTRL_0     MKREG(0x00, 0xe4)
#define     HVF_CNTRL_0_PREFIL_NONE     (0 << 2)
#define     HVF_CNTRL_0_INTPOL_BYPASS   (0 << 0)
#define TDA_HVF_CNTRL_1     MKREG(0x00, 0xe5)
#define     HVF_CNTRL_1_VQR(x)  (((x) & 3) << 2)
#define     HVF_CNTRL_1_VQR_FULL    HVF_CNTRL_1_VQR(0)
#define TDA_ENABLE_SPACE    MKREG(0x00, 0xd6)
#define TDA_RPT_CNTRL       MKREG(0x00, 0xf0)

#define TDA_PLL_SERIAL_1    MKREG(0x02, 0x00)
#define     PLL_SERIAL_1_SRL_MAN_IP (1 << 6)
#define TDA_PLL_SERIAL_2    MKREG(0x02, 0x01)
#define     PLL_SERIAL_2_SRL_PR(x)      (((x) & 0xf) << 4)
#define     PLL_SERIAL_2_SRL_NOSC(x)    (((x) & 0x3) << 0)
#define TDA_PLL_SERIAL_3    MKREG(0x02, 0x02)
#define     PLL_SERIAL_3_SRL_PXIN_SEL   (1 << 4)
#define     PLL_SERIAL_3_SRL_DE     (1 << 2)
#define     PLL_SERIAL_3_SRL_CCIR       (1 << 0)
#define TDA_SERIALIZER      MKREG(0x02, 0x03)
#define TDA_BUFFER_OUT      MKREG(0x02, 0x04)
#define TDA_PLL_SCG1        MKREG(0x02, 0x05)
#define TDA_PLL_SCG2        MKREG(0x02, 0x06)
#define TDA_PLL_SCGN1       MKREG(0x02, 0x07)
#define TDA_PLL_SCGN2       MKREG(0x02, 0x08)
#define TDA_PLL_SCGR1       MKREG(0x02, 0x09)
#define TDA_PLL_SCGR2       MKREG(0x02, 0x0a)

#define TDA_SEL_CLK     MKREG(0x02, 0x11)
#define     SEL_CLK_ENA_SC_CLK  (1 << 3)
#define     SEL_CLK_SEL_VRF_CLK(x)  (((x) & 3) << 1)
#define     SEL_CLK_SEL_CLK1    (1 << 0)
#define TDA_ANA_GENERAL     MKREG(0x02, 0x12)

#define TDA_EDID_DATA0      MKREG(0x09, 0x00)
#define TDA_EDID_CTRL       MKREG(0x09, 0xfa)
#define TDA_DDC_ADDR        MKREG(0x09, 0xfb)
#define TDA_DDC_OFFS        MKREG(0x09, 0xfc)
#define TDA_DDC_SEGM_ADDR   MKREG(0x09, 0xfd)
#define TDA_DDC_SEGM        MKREG(0x09, 0xfe)

#define TDA_IF_VSP      MKREG(0x10, 0x20)
#define TDA_IF_AVI      MKREG(0x10, 0x40)
#define TDA_IF_SPD      MKREG(0x10, 0x60)
#define TDA_IF_AUD      MKREG(0x10, 0x80)
#define TDA_IF_MPS      MKREG(0x10, 0xa0)

#define TDA_ENC_CNTRL       MKREG(0x11, 0x0d)
#define     ENC_CNTRL_DVI_MODE  (0 << 2)
#define     ENC_CNTRL_HDMI_MODE (1 << 2)
#define TDA_DIP_IF_FLAGS    MKREG(0x11, 0x0f)
#define     DIP_IF_FLAGS_IF5    (1 << 5)
#define     DIP_IF_FLAGS_IF4    (1 << 4)
#define     DIP_IF_FLAGS_IF3    (1 << 3)
#define     DIP_IF_FLAGS_IF2    (1 << 2) /* AVI IF on page 10h */
#define     DIP_IF_FLAGS_IF1    (1 << 1)

#define TDA_TX3         MKREG(0x12, 0x9a)
#define TDA_TX4         MKREG(0x12, 0x9b)
#define     TX4_PD_RAM      (1 << 1)
#define TDA_HDCP_TX33       MKREG(0x12, 0xb8)
#define     HDCP_TX33_HDMI      (1 << 1)

#define TDA_CURPAGE_ADDR    0xff

#define TDA_CEC_ENAMODS     0xff
#define     ENAMODS_RXSENS      (1 << 2)
#define     ENAMODS_HDMI        (1 << 1)
#define TDA_CEC_FRO_IM_CLK_CTRL 0xfb
#define     CEC_FRO_IM_CLK_CTRL_GHOST_DIS   (1 << 7)
#define     CEC_FRO_IM_CLK_CTRL_IMCLK_SEL   (1 << 1)

/* EDID reading */ 
#define EDID_LENGTH     0x80
#define MAX_READ_ATTEMPTS   100

/* EDID fields */
#define EDID_MODES0     35
#define EDID_MODES1     36
#define EDID_TIMING_START   38
#define EDID_TIMING_END     54
#define EDID_TIMING_X(v)    (((v) + 31) * 8)
#define EDID_FREQ(v)        (((v) & 0x3f) + 60)
#define EDID_RATIO(v)       (((v) >> 6) & 0x3)
#define EDID_RATIO_10x16    0
#define EDID_RATIO_3x4      1   
#define EDID_RATIO_4x5      2   
#define EDID_RATIO_9x16     3

#define TDA19988        0x0301



static I2C_HandleTypeDef g_i2c_handle;
static uint16_t version;
static uint8_t sc_current_page;

const int videomode_count = 46;
tda19988_vm_t videomode_list[] = {
M("640x350x85",640,350,31500,672,736,832,382,385,445,HP|VN),
M("640x400x85",640,400,31500,672,736,832,401,404,445,HN|VP),
M("720x400x70",720,400,28320,738,846,900,412,414,449,HN|VP),
M("720x400x85",720,400,35500,756,828,936,401,404,446,HN|VP),
M("720x400x87",720,400,35500,738,846,900,421,423,449,HN|VN),
M("640x480x60",640,480,25175,656,752,800,490,492,525,HN|VN),
M("640x480x72",640,480,31500,664,704,832,489,492,520,HN|VN),
M("640x480x75",640,480,31500,656,720,840,481,484,500,HN|VN),
M("640x480x85",640,480,36000,696,752,832,481,484,509,HN|VN),
M("800x600x56",800,600,36000,824,896,1024,601,603,625,HP|VP),
M("800x600x60",800,600,40000,840,968,1056,601,605,628,HP|VP),   
M("800x600x72",800,600,50000,856,976,1040,637,643,666,HP|VP),
M("800x600x75",800,600,49500,816,896,1056,601,604,625,HP|VP),
M("800x600x85",800,600,56250,832,896,1048,601,604,631,HP|VP),
M("1024x768x87i",1024,768,44900,1032,1208,1264,768,776,817,HP|VP|I),
M("1024x768x60",1024,768,65000,1048,1184,1344,771,777,806,HN|VN),
M("1024x768x70",1024,768,75000,1048,1184,1328,771,777,806,HN|VN),
M("1024x768x75",1024,768,78750,1040,1136,1312,769,772,800,HP|VP),
M("1024x768x85",1024,768,94500,1072,1168,1376,769,772,808,HP|VP),
M("1024x768x89",1024,768,100000,1108,1280,1408,768,780,796,HP|VP),
M("1152x864x75",1152,864,108000,1216,1344,1600,865,868,900,HP|VP),
M("1280x768x75",1280,768,105640,1312,1712,1744,782,792,807,HN|VP),
M("1280x960x60",1280,960,108000,1376,1488,1800,961,964,1000,HP|VP),
M("1280x960x85",1280,960,148500,1344,1504,1728,961,964,1011,HP|VP),
M("1280x1024x60",1280,1024,108000,1328,1440,1688,1025,1028,1066,HP|VP),
M("1280x1024x70",1280,1024,126000,1328,1440,1688,1025,1028,1066,HP|VP),
M("1280x1024x75",1280,1024,135000,1296,1440,1688,1025,1028,1066,HP|VP),
M("1280x1024x85",1280,1024,157500,1344,1504,1728,1025,1028,1072,HP|VP),
M("1600x1200x60",1600,1200,162000,1664,1856,2160,1201,1204,1250,HP|VP),
M("1600x1200x65",1600,1200,175500,1664,1856,2160,1201,1204,1250,HP|VP),
M("1600x1200x70",1600,1200,189000,1664,1856,2160,1201,1204,1250,HP|VP),
M("1600x1200x75",1600,1200,202500,1664,1856,2160,1201,1204,1250,HP|VP),
M("1600x1200x85",1600,1200,229500,1664,1856,2160,1201,1204,1250,HP|VP),
M("1680x1050x60",1680,1050,147140,1784,1968,2256,1051,1054,1087,HP|VP),
M("1792x1344x60",1792,1344,204800,1920,2120,2448,1345,1348,1394,HN|VP),
M("1792x1344x75",1792,1344,261000,1888,2104,2456,1345,1348,1417,HN|VP),
M("1856x1392x60",1856,1392,218300,1952,2176,2528,1393,1396,1439,HN|VP),
M("1856x1392x75",1856,1392,288000,1984,2208,2560,1393,1396,1500,HN|VP),
M("1920x1440x60",1920,1440,234000,2048,2256,2600,1441,1444,1500,HN|VP),
M("1920x1440x75",1920,1440,297000,2064,2288,2640,1441,1444,1500,HN|VP),
M("832x624x74",832,624,57284,864,928,1152,625,628,667,HN|VN),
M("1152x768x54",1152,768,64995,1178,1314,1472,771,777,806,HP|VP),
M("1400x1050x60",1400,1050,122000,1488,1640,1880,1052,1064,1082,HP|VP),
M("1400x1050x74",1400,1050,155800,1464,1784,1912,1052,1064,1090,HP|VP),
M("1152x900x66",1152,900,94500,1192,1320,1528,902,906,937,HN|VN),
M("1152x900x76",1152,900,105560,1168,1280,1472,902,906,943,HN|VN),

/* Derived Double Scan Modes */

M("320x175x85",320,175,15750,336,368,416,191,192,222,HP|VN|DS),
M("320x200x85",320,200,15750,336,368,416,200,202,222,HN|VP|DS),
M("360x200x70",360,200,14160,369,423,450,206,207,224,HN|VP|DS),
M("360x200x85",360,200,17750,378,414,468,200,202,223,HN|VP|DS),
M("360x200x87",360,200,17750,369,423,450,210,211,224,HN|VN|DS),
M("320x240x60",320,240,12587,328,376,400,245,246,262,HN|VN|DS),
M("320x240x72",320,240,15750,332,352,416,244,246,260,HN|VN|DS),
M("320x240x75",320,240,15750,328,360,420,240,242,250,HN|VN|DS),
M("320x240x85",320,240,18000,348,376,416,240,242,254,HN|VN|DS),
M("400x300x56",400,300,18000,412,448,512,300,301,312,HP|VP|DS),
M("400x300x60",400,300,20000,420,484,528,300,302,314,HP|VP|DS),
M("400x300x72",400,300,25000,428,488,520,318,321,333,HP|VP|DS),
M("400x300x75",400,300,24750,408,448,528,300,302,312,HP|VP|DS),
M("400x300x85",400,300,28125,416,448,524,300,302,315,HP|VP|DS),
M("512x384x87i",512,384,22450,516,604,632,384,388,408,HP|VP|DS|I),
M("512x384x60",512,384,32500,524,592,672,385,388,403,HN|VN|DS),
M("512x384x70",512,384,37500,524,592,664,385,388,403,HN|VN|DS),
M("512x384x75",512,384,39375,520,568,656,384,386,400,HP|VP|DS),
M("512x384x85",512,384,47250,536,584,688,384,386,404,HP|VP|DS),
M("512x384x89",512,384,50000,554,640,704,384,390,398,HP|VP|DS),
M("576x432x75",576,432,54000,608,672,800,432,434,450,HP|VP|DS),
M("640x384x75",640,384,52820,656,856,872,391,396,403,HN|VP|DS),
M("640x480x60",640,480,54000,688,744,900,480,482,500,HP|VP|DS),
M("640x480x85",640,480,74250,672,752,864,480,482,505,HP|VP|DS),
M("640x512x60",640,512,54000,664,720,844,512,514,533,HP|VP|DS),
M("640x512x70",640,512,63000,664,720,844,512,514,533,HP|VP|DS),
M("640x512x75",640,512,67500,648,720,844,512,514,533,HP|VP|DS),
M("640x512x85",640,512,78750,672,752,864,512,514,536,HP|VP|DS),
M("800x600x60",800,600,81000,832,928,1080,600,602,625,HP|VP|DS),
M("800x600x65",800,600,87750,832,928,1080,600,602,625,HP|VP|DS),
M("800x600x70",800,600,94500,832,928,1080,600,602,625,HP|VP|DS),
M("800x600x75",800,600,101250,832,928,1080,600,602,625,HP|VP|DS),
M("800x600x85",800,600,114750,832,928,1080,600,602,625,HP|VP|DS),
M("840x525x60",840,525,73570,892,984,1128,525,527,543,HP|VP|DS),
M("896x672x60",896,672,102400,960,1060,1224,672,674,697,HN|VP|DS),
M("896x672x75",896,672,130500,944,1052,1228,672,674,708,HN|VP|DS),
M("928x696x60",928,696,109150,976,1088,1264,696,698,719,HN|VP|DS),
M("928x696x75",928,696,144000,992,1104,1280,696,698,750,HN|VP|DS),
M("960x720x60",960,720,117000,1024,1128,1300,720,722,750,HN|VP|DS),
M("960x720x75",960,720,148500,1032,1144,1320,720,722,750,HN|VP|DS),
M("416x312x74",416,312,28642,432,464,576,312,314,333,HN|VN|DS),
M("576x384x54",576,384,32497,589,657,736,385,388,403,HP|VP|DS),
M("700x525x60",700,525,61000,744,820,940,526,532,541,HP|VP|DS),
M("700x525x74",700,525,77900,732,892,956,526,532,545,HP|VP|DS),
M("576x450x66",576,450,47250,596,660,764,451,453,468,HN|VN|DS),
M("576x450x76",576,450,52780,584,640,736,451,453,471,HN|VN|DS),
};

__weak void HAL_TDA19988_MspInit()
{
    ;
}

void tda19988_init()
{
    HAL_StatusTypeDef ret = HAL_OK;

    g_i2c_handle.Instance = I2C4;
    g_i2c_handle.Init.Timing = I2C_TIMING;
    g_i2c_handle.Init.OwnAddress1 = I2C_OWN_ADDRESS;
    g_i2c_handle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    g_i2c_handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    g_i2c_handle.Init.OwnAddress2 = I2C_OWN_ADDRESS;
    g_i2c_handle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    g_i2c_handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    g_i2c_handle.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    ret = HAL_I2C_Init(&g_i2c_handle);
    if(ret != HAL_OK)
    {
        main_error("Failed to initialize I2C!", __FILE__, __LINE__, ret);
    }

    HAL_I2CEx_ConfigAnalogFilter(&g_i2c_handle, I2C_ANALOGFILTER_ENABLE);

    HAL_TDA19988_MspInit(); /* Configure IRQ */
}
/*
static int
tda19988_block_read(struct tda19988_softc *sc, uint16_t addr, uint8_t *data, int len)
{
    uint8_t reg;
    int result;
    struct iic_msg msg[] = {
        { sc->sc_addr, IIC_M_WR, 1, &reg },
        { sc->sc_addr, IIC_M_RD, len, data },
    };

    reg = REGADDR(addr);

    if (sc_current_page != REGPAGE(addr))
        tda19988_set_page(sc, REGPAGE(addr));

    result = (iicbus_transfer(sc->sc_dev, msg, 2));
    if (result)
        device_printf(sc->sc_dev, "tda19988_block_read failed: %d\n", result);
    return (result);
}*/

static int
tda19988_read_edid_block(struct tda19988_softc *sc, uint8_t *buf, int block)
{
    int attempt, err;
    uint8_t data;

    err = 0;

    tda19988_reg_set(I2C_HDMI_ADDRESS, TDA_INT_FLAGS_2, INT_FLAGS_2_EDID_BLK_RD);

    /* Block 0 */
    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_DDC_ADDR, 0xa0);
    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_DDC_OFFS, (block % 2) ? 128 : 0);
    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_DDC_SEGM_ADDR, 0x60);
    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_DDC_SEGM, block / 2);

    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_EDID_CTRL, 1);
    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_EDID_CTRL, 0);

    data = 0;
    for (attempt = 0; attempt < MAX_READ_ATTEMPTS; attempt++) {
        tda19988_reg_read(I2C_HDMI_ADDRESS, TDA_INT_FLAGS_2, &data);
        if (data & INT_FLAGS_2_EDID_BLK_RD)
            break;
        pause("EDID", 1);
    }

    if (attempt == MAX_READ_ATTEMPTS) {
        err = -1;
        goto done;
    }
/*
    if (tda19988_block_read(I2C_HDMI_ADDRESS, TDA_EDID_DATA0, buf, EDID_LENGTH) != 0) {
        err = -1;
        goto done;
    }*/

done:
    tda19988_clear_reg(I2C_HDMI_ADDRESS, TDA_INT_FLAGS_2, INT_FLAGS_2_EDID_BLK_RD);

    return (err);
}

static int tda19988_set_page(uint8_t page)
{
    uint8_t addr = TDA_CURPAGE_ADDR;
    uint8_t cmd[2];
    int result;

    cmd[0] = addr;
    cmd[1] = page;

    result = HAL_I2C_Mem_Write(&g_i2c_handle, I2C_HDMI_ADDRESS, addr, 1, &page, 1, 2000);
    if (result)
        printf("tda19988_set_page failed: %d\n", result);
    else
        sc_current_page = page;

    return (result);
}

static int tda19988_wr_reg2(tda19988_addr_t addr, uint16_t address, uint16_t data)
{
    uint8_t cmd[3];
    int result;

    cmd[0] = REGADDR(address);
    cmd[1] = (data >> 8);
    cmd[2] = (data & 0xff);

    if (sc_current_page != REGPAGE(address))
        tda19988_set_page(REGPAGE(address));

    result = HAL_I2C_Mem_Write(&g_i2c_handle, addr, cmd[0], 1, &cmd[1], 2, 2000);
    if (result)
        printf("tda19988_reg_write2 failed: %d\n", result);

    return (result);
}

void tda19988_init_encoder(tda19988_vm_t *mode)
{
    uint16_t ref_pix, ref_line, n_pix, n_line;
    uint16_t hs_pix_start, hs_pix_stop;
    uint16_t vs1_pix_start, vs1_pix_stop;
    uint16_t vs1_line_start, vs1_line_end;
    uint16_t vs2_pix_start, vs2_pix_stop;
    uint16_t vs2_line_start, vs2_line_end;
    uint16_t vwin1_line_start, vwin1_line_end;
    uint16_t vwin2_line_start, vwin2_line_end;
    uint16_t de_start, de_stop;
    uint8_t reg, div;

    n_pix = mode->htotal;
    n_line = mode->vtotal;

    hs_pix_stop = mode->hsync_end - mode->hdisplay;
    hs_pix_start = mode->hsync_start - mode->hdisplay;

    de_stop = mode->htotal;
    de_start = mode->htotal - mode->hdisplay;
    ref_pix = hs_pix_start + 3;

    if (mode->flags & VID_HSKEW)
        ref_pix += mode->hskew;

    if ((mode->flags & VID_INTERLACE) == 0) {
        ref_line = 1 + mode->vsync_start - mode->vdisplay;
        vwin1_line_start = mode->vtotal - mode->vdisplay - 1;
        vwin1_line_end = vwin1_line_start + mode->vdisplay;

        vs1_pix_start = vs1_pix_stop = hs_pix_start;
        vs1_line_start = mode->vsync_start - mode->vdisplay;
        vs1_line_end = vs1_line_start + mode->vsync_end - mode->vsync_start;

        vwin2_line_start = vwin2_line_end = 0;
        vs2_pix_start = vs2_pix_stop = 0;
        vs2_line_start = vs2_line_end = 0;
    } else {
        ref_line = 1 + (mode->vsync_start - mode->vdisplay)/2;
        vwin1_line_start = (mode->vtotal - mode->vdisplay)/2;
        vwin1_line_end = vwin1_line_start + mode->vdisplay/2;

        vs1_pix_start = vs1_pix_stop = hs_pix_start;
        vs1_line_start = (mode->vsync_start - mode->vdisplay)/2;
        vs1_line_end = vs1_line_start + (mode->vsync_end - mode->vsync_start)/2;

        vwin2_line_start = vwin1_line_start + mode->vtotal/2;
        vwin2_line_end = vwin2_line_start + mode->vdisplay/2;

        vs2_pix_start = vs2_pix_stop = hs_pix_start + mode->htotal/2;
        vs2_line_start = vs1_line_start + mode->vtotal/2 ;
        vs2_line_end = vs2_line_start + (mode->vsync_end - mode->vsync_start)/2;
    }

    div = 148500 / mode->dot_clock;
    if (div != 0) {
        div--;
        if (div > 3)
            div = 3;
    }

    /* set HDMI HDCP mode off */
    tda19988_set_reg(I2C_HDMI_ADDRESS, TDA_TBG_CNTRL_1, TBG_CNTRL_1_DWIN_DIS);
    tda19988_clear_reg(I2C_HDMI_ADDRESS, TDA_HDCP_TX33, HDCP_TX33_HDMI);
    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_ENC_CNTRL, ENC_CNTRL_DVI_MODE);

    /* no pre-filter or interpolator */
    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_HVF_CNTRL_0, HVF_CNTRL_0_INTPOL_BYPASS | HVF_CNTRL_0_PREFIL_NONE);
    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_VIP_CNTRL_5, VIP_CNTRL_5_SP_CNT(0));
    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_VIP_CNTRL_4, VIP_CNTRL_4_BLANKIT_NDE | VIP_CNTRL_4_BLC_NONE);
    tda19988_clear_reg(I2C_HDMI_ADDRESS, TDA_PLL_SERIAL_3, PLL_SERIAL_3_SRL_CCIR);
    tda19988_clear_reg(I2C_HDMI_ADDRESS, TDA_PLL_SERIAL_1, PLL_SERIAL_1_SRL_MAN_IP);
    tda19988_clear_reg(I2C_HDMI_ADDRESS, TDA_PLL_SERIAL_3, PLL_SERIAL_3_SRL_DE);
    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_SERIALIZER, 0);
    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_HVF_CNTRL_1, HVF_CNTRL_1_VQR_FULL);
    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_RPT_CNTRL, 0);
    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_SEL_CLK, SEL_CLK_SEL_VRF_CLK(0) | SEL_CLK_SEL_CLK1 | SEL_CLK_ENA_SC_CLK);
    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_PLL_SERIAL_2, PLL_SERIAL_2_SRL_NOSC(div) | PLL_SERIAL_2_SRL_PR(0));
    tda19988_set_reg(I2C_HDMI_ADDRESS, TDA_MAT_CONTRL, MAT_CONTRL_MAT_BP);
    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_ANA_GENERAL, 0x09);
    tda19988_clear_reg(I2C_HDMI_ADDRESS, TDA_TBG_CNTRL_0, TBG_CNTRL_0_SYNC_MTHD);

    /*
     * Sync on rising HSYNC/VSYNC
     */
    reg = VIP_CNTRL_3_SYNC_HS;
    if (mode->flags & VID_NHSYNC)
        reg |= VIP_CNTRL_3_H_TGL;
    if (mode->flags & VID_NVSYNC)
        reg |= VIP_CNTRL_3_V_TGL;
    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_VIP_CNTRL_3, reg);

    reg = TBG_CNTRL_1_TGL_EN;
    if (mode->flags & VID_NHSYNC)
        reg |= TBG_CNTRL_1_H_TGL;
    if (mode->flags & VID_NVSYNC)
        reg |= TBG_CNTRL_1_V_TGL;
    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_TBG_CNTRL_1, reg);

    /* Program timing */
    tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_VIDFORMAT, 0x00);

    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_REFPIX_MSB, ref_pix);
    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_REFLINE_MSB, ref_line);
    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_NPIX_MSB, n_pix);
    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_NLINE_MSB, n_line);

    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_VS_LINE_STRT_1_MSB, vs1_line_start);
    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_VS_PIX_STRT_1_MSB, vs1_pix_start);
    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_VS_LINE_END_1_MSB, vs1_line_end);
    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_VS_PIX_END_1_MSB, vs1_pix_stop);
    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_VS_LINE_STRT_2_MSB, vs2_line_start);
    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_VS_PIX_STRT_2_MSB, vs2_pix_start);
    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_VS_LINE_END_2_MSB, vs2_line_end);
    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_VS_PIX_END_2_MSB, vs2_pix_stop);
    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_HS_PIX_START_MSB, hs_pix_start);
    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_HS_PIX_STOP_MSB, hs_pix_stop);
    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_VWIN_START_1_MSB, vwin1_line_start);
    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_VWIN_END_1_MSB, vwin1_line_end);
    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_VWIN_START_2_MSB, vwin2_line_start);
    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_VWIN_END_2_MSB, vwin2_line_end);
    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_DE_START_MSB, de_start);
    tda19988_wr_reg2(I2C_HDMI_ADDRESS, TDA_DE_STOP_MSB, de_stop);

    if (version == TDA19988)
        tda19988_wr_reg(I2C_HDMI_ADDRESS, TDA_ENABLE_SPACE, 0x00);

    /* must be last register set */
    tda19988_clear_reg(I2C_HDMI_ADDRESS, TDA_TBG_CNTRL_0, TBG_CNTRL_0_SYNC_ONCE);
}

void tda19988_configure()
{
    uint8_t data;

    tda19988_wr_reg(TDA19988_ADDR_CEC, TDA_CEC_ENAMODS, ENAMODS_RXSENS | ENAMODS_HDMI);

    HAL_Delay(200);
    data = tda19988_rd_reg(TDA19988_ADDR_CEC, 0xfe);

    /* Reset core */
    tda19988_set_reg(I2C_HDMI_ADDRESS,  TDA_SOFTRESET, 3);
    HAL_Delay(200);
    tda19988_clear_reg(I2C_HDMI_ADDRESS,  TDA_SOFTRESET, 3);
    HAL_Delay(200);

    /* reset transmitter: */
    tda19988_set_reg(I2C_HDMI_ADDRESS,  TDA_MAIN_CNTRL0, MAIN_CNTRL0_SR);
    tda19988_clear_reg(I2C_HDMI_ADDRESS,  TDA_MAIN_CNTRL0, MAIN_CNTRL0_SR);

    /* PLL registers common configuration */
    tda19988_wr_reg(I2C_HDMI_ADDRESS,  TDA_PLL_SERIAL_1, 0x00);
    tda19988_wr_reg(I2C_HDMI_ADDRESS,  TDA_PLL_SERIAL_2, PLL_SERIAL_2_SRL_NOSC(1));
    tda19988_wr_reg(I2C_HDMI_ADDRESS,  TDA_PLL_SERIAL_3, 0x00);
    tda19988_wr_reg(I2C_HDMI_ADDRESS,  TDA_SERIALIZER, 0x00);
    tda19988_wr_reg(I2C_HDMI_ADDRESS,  TDA_BUFFER_OUT, 0x00);
    tda19988_wr_reg(I2C_HDMI_ADDRESS,  TDA_PLL_SCG1, 0x00);
    tda19988_wr_reg(I2C_HDMI_ADDRESS,  TDA_SEL_CLK, SEL_CLK_SEL_CLK1 | SEL_CLK_ENA_SC_CLK);
    tda19988_wr_reg(I2C_HDMI_ADDRESS,  TDA_PLL_SCGN1, 0xfa);
    tda19988_wr_reg(I2C_HDMI_ADDRESS,  TDA_PLL_SCGN2, 0x00);
    tda19988_wr_reg(I2C_HDMI_ADDRESS,  TDA_PLL_SCGR1, 0x5b);
    tda19988_wr_reg(I2C_HDMI_ADDRESS,  TDA_PLL_SCGR2, 0x00);
    tda19988_wr_reg(I2C_HDMI_ADDRESS,  TDA_PLL_SCG2, 0x10);

    /* Write the default value MUX register */
    tda19988_wr_reg(I2C_HDMI_ADDRESS,  TDA_MUX_VP_VIP_OUT, 0x24);

    version = 0;
    data = tda19988_rd_reg(I2C_HDMI_ADDRESS,  TDA_VERSION);
    version |= data;
    data = tda19988_rd_reg(I2C_HDMI_ADDRESS,  TDA_VERSION_MSB);
    version |= (data << 8);

    /* Clear feature bits */
    version = version & ~0x30;
    switch (version) {
        case TDA19988:
            printf("TDA19988\n");
            break;
        default:
            printf("Unknown device: %04x\n", version);
            goto done;
    }

    tda19988_wr_reg(I2C_HDMI_ADDRESS,  TDA_DDC_CTRL, DDC_ENABLE);
    tda19988_wr_reg(I2C_HDMI_ADDRESS,  TDA_TX3, 39);
    tda19988_wr_reg(I2C_CEC_ADDRESS,  TDA_CEC_FRO_IM_CLK_CTRL,
        CEC_FRO_IM_CLK_CTRL_GHOST_DIS | CEC_FRO_IM_CLK_CTRL_IMCLK_SEL);

    /*if (tda19988_read_edid(sc) < 0) {
        printf("Failed to read EDID\n");
        goto done;
    }*/

    /* Default values for RGB 4:4:4 mapping */
    tda19988_wr_reg(I2C_HDMI_ADDRESS,  TDA_VIP_CNTRL_0, 0x23);
    tda19988_wr_reg(I2C_HDMI_ADDRESS,  TDA_VIP_CNTRL_1, 0x01);
    tda19988_wr_reg(I2C_HDMI_ADDRESS,  TDA_VIP_CNTRL_2, 0x45);
done:
    ;  
}

void tda19988_wr_reg(tda19988_addr_t addr, uint8_t reg, uint8_t val)
{
    uint16_t i2c_addr;

    if(addr == TDA19988_ADDR_CEC)
    {
        i2c_addr = (uint16_t)I2C_CEC_ADDRESS;
    }
    else
    {
        i2c_addr = (uint16_t)I2C_HDMI_ADDRESS;
    }

    if(HAL_I2C_Mem_Write(&g_i2c_handle, i2c_addr, reg, 1, &val, 1, 2000) != HAL_OK)
    {
        main_error("Failed to write I2C register!", __FILE__, __LINE__, reg);
    }
}

uint8_t tda19988_rd_reg(tda19988_addr_t addr, uint8_t reg)
{
    uint8_t val;
    uint16_t i2c_addr;

    if(addr == TDA19988_ADDR_CEC)
    {
        i2c_addr = (uint16_t)I2C_CEC_ADDRESS;
    }
    else
    {
        i2c_addr = (uint16_t)I2C_HDMI_ADDRESS;
    }

    //if(HAL_I2C_Master_ReadReg(&g_i2c_handle, (uint16_t)I2C_ADDRESS, &reg, &val, 2000) != HAL_OK)
    if(HAL_I2C_Mem_Read(&g_i2c_handle, i2c_addr, reg, 1, &val, 1, 2000) != HAL_OK)
    {
        main_error("Failed to read I2C register!", __FILE__, __LINE__, reg);
    }

    return val;
}

void tda19988_set_reg(tda19988_addr_t addr, uint8_t reg, uint8_t bits_to_set)
{
    tda19988_wr_reg(addr, reg, tda19988_rd_reg(addr, reg) | bits_to_set);
}

void tda19988_clear_reg(tda19988_addr_t addr, uint8_t reg, uint8_t bits_to_clear)
{
    tda19988_wr_reg(addr, reg, tda19988_rd_reg(addr, reg) & ~bits_to_clear);
}

void tda19988_irq()
{
    uint8_t val;

}
