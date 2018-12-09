/*
 * memwa3 tda19988 driver
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

#include "tda19988.h"
#include "drv_i2c.h"

#define PAGE(reg)                       (((reg) >> 8) & 0xff)
#define REG(reg)                        ((reg) & 0xff)

#define TDA_VERSION                     0x0000
#define TDA_MAIN_CNTRL0                 0x0001
#define TDA_VERSION_MSB                 0x0002
#define TDA_SOFTRESET                   0x000a
#define TDA_DDC_CTRL                    0x000b
#define TDA_CCLK                        0x000c
#define TDA_INT_FLAGS_2                 0x0011
#define TDA_VIP_CNTRL_0                 0x0020
#define TDA_VIP_CNTRL_1                 0x0021
#define TDA_VIP_CNTRL_2                 0x0022
#define TDA_VIP_CNTRL_3                 0x0023
#define TDA_VIP_CNTRL_4                 0x0024
#define TDA_VIP_CNTRL_5                 0x0025
#define TDA_MUX_VP_VIP_OUT              0x0027
#define TDA_MAT_CONTRL                  0x0080
#define TDA_VIDFORMAT                   0x00a0
#define TDA_REFPIX_MSB                  0x00a1
#define TDA_REFPIX_LSB                  0x00a2
#define TDA_REFLINE_MSB                 0x00a3
#define TDA_REFLINE_LSB                 0x00a4
#define TDA_NPIX_MSB                    0x00a5
#define TDA_NPIX_LSB                    0x00a6
#define TDA_NLINE_MSB                   0x00a7
#define TDA_NLINE_LSB                   0x00a8
#define TDA_VS_LINE_STRT_1_MSB          0x00a9
#define TDA_VS_LINE_STRT_1_LSB          0x00aa
#define TDA_VS_PIX_STRT_1_MSB           0x00ab
#define TDA_VS_PIX_STRT_1_LSB           0x00ac
#define TDA_VS_LINE_END_1_MSB           0x00ad
#define TDA_VS_LINE_END_1_LSB           0x00ae
#define TDA_VS_PIX_END_1_MSB            0x00af
#define TDA_VS_PIX_END_1_LSB            0x00b0
#define TDA_VS_LINE_STRT_2_MSB          0x00b1
#define TDA_VS_LINE_STRT_2_LSB          0x00b2
#define TDA_VS_PIX_STRT_2_MSB           0x00b3
#define TDA_VS_PIX_STRT_2_LSB           0x00b4
#define TDA_VS_LINE_END_2_MSB           0x00b5
#define TDA_VS_LINE_END_2_LSB           0x00b6
#define TDA_VS_PIX_END_2_MSB            0x00b7
#define TDA_VS_PIX_END_2_LSB            0x00b8
#define TDA_HS_PIX_START_MSB            0x00b9
#define TDA_HS_PIX_START_LSB            0x00ba
#define TDA_HS_PIX_STOP_MSB             0x00bb
#define TDA_HS_PIX_STOP_LSB             0x00bc
#define TDA_VWIN_START_1_MSB            0x00bd
#define TDA_VWIN_START_1_LSB            0x00be
#define TDA_VWIN_END_1_MSB              0x00bf
#define TDA_VWIN_END_1_LSB              0x00c0
#define TDA_VWIN_START_2_MSB            0x00c1
#define TDA_VWIN_START_2_LSB            0x00c2
#define TDA_VWIN_END_2_MSB              0x00c3
#define TDA_VWIN_END_2_LSB              0x00c4
#define TDA_DE_START_MSB                0x00c5
#define TDA_DE_START_LSB                0x00c6
#define TDA_DE_STOP_MSB                 0x00c7
#define TDA_DE_STOP_LSB                 0x00c8
#define TDA_TBG_CNTRL_0                 0x00ca
#define TDA_TBG_CNTRL_1                 0x00cb
#define TDA_HVF_CNTRL_0                 0x00e4
#define TDA_HVF_CNTRL_1                 0x00e5
#define TDA_ENABLE_SPACE                0x00d6
#define TDA_RPT_CNTRL                   0x00f0
#define TDA_PLL_SERIAL_1                0x0200
#define TDA_PLL_SERIAL_2                0x0201
#define TDA_PLL_SERIAL_3                0x0202
#define TDA_SERIALIZER                  0x0203
#define TDA_BUFFER_OUT                  0x0204
#define TDA_PLL_SCG1                    0x0205
#define TDA_PLL_SCG2                    0x0206
#define TDA_PLL_SCGN1                   0x0207
#define TDA_PLL_SCGN2                   0x0208
#define TDA_PLL_SCGR1                   0x0209
#define TDA_PLL_SCGR2                   0x020a
#define TDA_SEL_CLK                     0x0211
#define TDA_ANA_GENERAL                 0x0212
#define TDA_EDID_DATA0                  0x0900
#define TDA_EDID_CTRL                   0x09fa
#define TDA_DDC_ADDR                    0x09fb
#define TDA_DDC_OFFS                    0x09fc
#define TDA_DDC_SEGM_ADDR               0x09fd
#define TDA_DDC_SEGM                    0x09fe
#define TDA_IF_VSP                      0x1020
#define TDA_IF_AVI                      0x1040
#define TDA_IF_SPD                      0x1060
#define TDA_IF_AUD                      0x1080
#define TDA_IF_MPS                      0x10a0
#define TDA_ENC_CNTRL                   0x110d
#define TDA_DIP_IF_FLAGS                0x110f
#define TDA_TX3                         0x129a
#define TDA_TX4                         0x129b
#define TDA_HDCP_TX33                   0x12b8
#define TDA_CURPAGE_ADDR                0xff
#define TDA_CEC_ENAMODS                 0xff
#define TDA_CEC_FRO_IM_CLK_CTRL         0xfb

#define DDC_ENABLE                      0
#define CCLK_ENABLE                     1
#define MAIN_CNTRL0_SR                  (1 << 0)
#define SOFTRESET_I2C                   (1 << 1)
#define SOFTRESET_AUDIO                 (1 << 0)
#define INT_FLAGS_2_EDID_BLK_RD         (1 << 1)
#define VIP_CNTRL_3_SYNC_HS             (2 << 4)
#define VIP_CNTRL_3_H_TGL               (1 << 1)
#define VIP_CNTRL_3_V_TGL               (1 << 2)
#define VIP_CNTRL_4_BLANKIT_NDE         (0 << 2)
#define VIP_CNTRL_4_BLANKIT_HS_VS       (1 << 2)
#define VIP_CNTRL_4_BLANKIT_NHS_VS      (2 << 2)
#define VIP_CNTRL_4_BLANKIT_HE_VE       (3 << 2)
#define VIP_CNTRL_4_BLC_NONE            (0 << 0)
#define VIP_CNTRL_4_BLC_RGB444          (1 << 0)
#define VIP_CNTRL_4_BLC_YUV444          (2 << 0)
#define VIP_CNTRL_4_BLC_YUV422          (3 << 0)
#define VIP_CNTRL_5_SP_CNT(n)           (((n) & 3) << 1)
#define MAT_CONTRL_MAT_BP               (1 << 2)
#define TBG_CNTRL_0_SYNC_ONCE           (1 << 7)
#define TBG_CNTRL_0_SYNC_MTHD           (1 << 6)
#define TBG_CNTRL_1_DWIN_DIS            (1 << 6)
#define TBG_CNTRL_1_TGL_EN              (1 << 2)
#define TBG_CNTRL_1_V_TGL               (1 << 1)
#define TBG_CNTRL_1_H_TGL               (1 << 0)
#define HVF_CNTRL_0_PREFIL_NONE         (0 << 2)
#define HVF_CNTRL_0_INTPOL_BYPASS       (0 << 0)
#define HVF_CNTRL_1_VQR(x)              (((x) & 3) << 2)
#define HVF_CNTRL_1_VQR_FULL            HVF_CNTRL_1_VQR(0)
#define PLL_SERIAL_1_SRL_MAN_IP         (1 << 6)
#define PLL_SERIAL_2_SRL_PR(x)          (((x) & 0xf) << 4)
#define PLL_SERIAL_2_SRL_NOSC(x)        (((x) & 0x3) << 0)
#define PLL_SERIAL_3_SRL_PXIN_SEL       (1 << 4)
#define PLL_SERIAL_3_SRL_DE             (1 << 2)
#define PLL_SERIAL_3_SRL_CCIR           (1 << 0)
#define SEL_CLK_ENA_SC_CLK              (1 << 3)
#define SEL_CLK_SEL_VRF_CLK(x)          (((x) & 3) << 1)
#define SEL_CLK_SEL_CLK1                (1 << 0)
#define ENC_CNTRL_DVI_MODE              (0 << 2)
#define ENC_CNTRL_HDMI_MODE             (1 << 2)
#define DIP_IF_FLAGS_IF5                (1 << 5)
#define DIP_IF_FLAGS_IF4                (1 << 4)
#define DIP_IF_FLAGS_IF3                (1 << 3)
#define DIP_IF_FLAGS_IF2                (1 << 2) /* AVI IF on page 10h */
#define DIP_IF_FLAGS_IF1                (1 << 1)
#define TX4_PD_RAM                      (1 << 1)
#define HDCP_TX33_HDMI                  (1 << 1)
#define ENAMODS_RXSENS                  (1 << 2)
#define ENAMODS_HDMI                    (1 << 1)
#define CEC_FRO_IM_CLK_CTRL_GHOST_DIS   (1 << 7)
#define CEC_FRO_IM_CLK_CTRL_IMCLK_SEL   (1 << 1)

/* EDID reading */ 
#define EDID_LENGTH                     0x80
#define MAX_READ_ATTEMPTS               100

/* EDID fields */
#define EDID_MODES0                     35
#define EDID_MODES1                     36
#define EDID_TIMING_START               38
#define EDID_TIMING_END                 54
#define EDID_TIMING_X(v)                (((v) + 31) * 8)
#define EDID_FREQ(v)                    (((v) & 0x3f) + 60)
#define EDID_RATIO(v)                   (((v) >> 6) & 0x3)
#define EDID_RATIO_10x16                0
#define EDID_RATIO_3x4                  1   
#define EDID_RATIO_4x5                  2   
#define EDID_RATIO_9x16                 3

/* Version */
#define TDA19988                        0x0301

static uint16_t version;
static uint8_t sc_current_page;

static const int videomode_count = 46;
tda19988_vm_t videomode_list[] =
{
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

static void handle_paging(uint8_t i2c_addr, uint16_t mem_addr)
{
    uint8_t page = PAGE(mem_addr);

    if(i2c_addr == TDA19988_ADDR_HDMI &&
       sc_current_page != page)
    {
        if(drv_i2c_wr_reg_8(i2c_addr, TDA_CURPAGE_ADDR, page) != I2C_STATUS_OK)
        {
            dev_term_printf(DEV_TERM_PRINT_TYPE_ERROR, "Failed to write page!", __FILE__, __LINE__);
            return;
        }
        else
        {
            sc_current_page = page;
        }
    }
}

static void clear_reg_8(uint8_t i2c_addr, uint16_t mem_addr, uint8_t mask)
{
    uint8_t reg = REG(mem_addr);
    handle_paging(i2c_addr, mem_addr);
    drv_i2c_mod_reg_8(i2c_addr, reg, mask, I2C_OP_CLEAR);
}

static void set_reg_8(uint8_t i2c_addr, uint16_t mem_addr, uint8_t mask)
{
    uint8_t reg = REG(mem_addr);
    handle_paging(i2c_addr, mem_addr);
    drv_i2c_mod_reg_8(i2c_addr, reg, mask, I2C_OP_SET);
}

static void write_reg_8(uint8_t i2c_addr, uint16_t mem_addr, uint8_t val)
{
    uint8_t reg = REG(mem_addr);
    handle_paging(i2c_addr, mem_addr);
    if(drv_i2c_wr_reg_8(i2c_addr, reg, val) != I2C_STATUS_OK)
    {
        dev_term_printf(DEV_TERM_PRINT_TYPE_ERROR, "Failed to write reg!", __FILE__, __LINE__);
        return;
    }
}

static void write_reg_16(uint8_t i2c_addr, uint16_t mem_addr, uint16_t val)
{
    uint8_t reg = REG(mem_addr);
    handle_paging(i2c_addr, mem_addr);
    if(drv_i2c_wr_reg_16(i2c_addr, reg, val) != I2C_STATUS_OK)
    {
        dev_term_printf(DEV_TERM_PRINT_TYPE_ERROR, "Failed to write reg!", __FILE__, __LINE__);
        return;
    }
}

static void read_reg_8(uint8_t i2c_addr, uint16_t mem_addr, uint8_t *val_p)
{
    uint8_t reg = REG(mem_addr);
    handle_paging(i2c_addr, mem_addr);
    if(drv_i2c_rd_reg_8(i2c_addr, reg, val_p) != I2C_STATUS_OK)
    {
        dev_term_printf(DEV_TERM_PRINT_TYPE_ERROR, "Failed to read reg!", __FILE__, __LINE__);
        return;
    }
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

static int tda19988_read_edid_block(struct tda19988_softc *sc, uint8_t *buf, int block)
{
    int32_t attempt;
    int32_t err;
    uint8_t data;

    err = 0;

    tda19988_reg_set(TDA19988_ADDR_HDMI, TDA_INT_FLAGS_2, INT_FLAGS_2_EDID_BLK_RD);

    /* Block 0 */
    write_reg_8(TDA19988_ADDR_HDMI, TDA_DDC_ADDR, 0xa0);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_DDC_OFFS, (block % 2) ? 128 : 0);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_DDC_SEGM_ADDR, 0x60);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_DDC_SEGM, block / 2);

    write_reg_8(TDA19988_ADDR_HDMI, TDA_EDID_CTRL, 1);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_EDID_CTRL, 0);

    data = 0;
    for(attempt = 0; attempt < MAX_READ_ATTEMPTS; attempt++)
    {
        tda19988_reg_read(TDA19988_ADDR_HDMI, TDA_INT_FLAGS_2, &data);
        if(data & INT_FLAGS_2_EDID_BLK_RD)
        {
            break;
        }

        pause("EDID", 1);
    }

    if(attempt == MAX_READ_ATTEMPTS) {
        err = -1;
        goto done;
    }
/*
    if (tda19988_block_read(TDA19988_ADDR_HDMI, TDA_EDID_DATA0, buf, EDID_LENGTH) != 0) {
        err = -1;
        goto done;
    }*/

done:
    clear_reg_8(TDA19988_ADDR_HDMI, TDA_INT_FLAGS_2, INT_FLAGS_2_EDID_BLK_RD);

    return err;
}

void tda19988_init()
{
    drv_i2c_init();
}

void tda19988_init_encoder(tda19988_vm_t *mode)
{
    uint16_t ref_pix;
    uint16_t ref_line;
    uint16_t n_pix;
    uint16_t n_line;
    uint16_t hs_pix_start;
    uint16_t hs_pix_stop;
    uint16_t vs1_pix_start;
    uint16_t vs1_pix_stop;
    uint16_t vs1_line_start;
    uint16_t vs1_line_end;
    uint16_t vs2_pix_start;
    uint16_t vs2_pix_stop;
    uint16_t vs2_line_start;
    uint16_t vs2_line_end;
    uint16_t vwin1_line_start;
    uint16_t vwin1_line_end;
    uint16_t vwin2_line_start;
    uint16_t vwin2_line_end;
    uint16_t de_start;
    uint16_t de_stop;
    uint8_t reg;
    uint8_t div;

    n_pix = mode->htotal;
    n_line = mode->vtotal;

    hs_pix_stop = mode->hsync_end - mode->hdisplay;
    hs_pix_start = mode->hsync_start - mode->hdisplay;

    de_stop = mode->htotal;
    de_start = mode->htotal - mode->hdisplay;
    ref_pix = hs_pix_start + 3;

    if (mode->flags & VID_HSKEW)
    {
        ref_pix += mode->hskew;
    }

    if ((mode->flags & VID_INTERLACE) == 0)
    {
        ref_line = 1 + mode->vsync_start - mode->vdisplay;
        vwin1_line_start = mode->vtotal - mode->vdisplay - 1;
        vwin1_line_end = vwin1_line_start + mode->vdisplay;

        vs1_pix_start = vs1_pix_stop = hs_pix_start;
        vs1_line_start = mode->vsync_start - mode->vdisplay;
        vs1_line_end = vs1_line_start + mode->vsync_end - mode->vsync_start;

        vwin2_line_start = vwin2_line_end = 0;
        vs2_pix_start = vs2_pix_stop = 0;
        vs2_line_start = vs2_line_end = 0;
    }
    else
    {
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
    if (div != 0)
    {
        div--;
        if (div > 3)
        {
            div = 3;
        }
    }

    /* set HDMI HDCP mode off */
    set_reg_8(TDA19988_ADDR_HDMI, TDA_TBG_CNTRL_1, TBG_CNTRL_1_DWIN_DIS);
    clear_reg_8(TDA19988_ADDR_HDMI, TDA_HDCP_TX33, HDCP_TX33_HDMI);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_ENC_CNTRL, ENC_CNTRL_DVI_MODE);

    /* no pre-filter or interpolator */
    write_reg_8(TDA19988_ADDR_HDMI, TDA_HVF_CNTRL_0, HVF_CNTRL_0_INTPOL_BYPASS | HVF_CNTRL_0_PREFIL_NONE);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_VIP_CNTRL_5, VIP_CNTRL_5_SP_CNT(0));
    write_reg_8(TDA19988_ADDR_HDMI, TDA_VIP_CNTRL_4, VIP_CNTRL_4_BLANKIT_NDE | VIP_CNTRL_4_BLC_NONE);
    clear_reg_8(TDA19988_ADDR_HDMI, TDA_PLL_SERIAL_3, PLL_SERIAL_3_SRL_CCIR);
    clear_reg_8(TDA19988_ADDR_HDMI, TDA_PLL_SERIAL_1, PLL_SERIAL_1_SRL_MAN_IP);
    clear_reg_8(TDA19988_ADDR_HDMI, TDA_PLL_SERIAL_3, PLL_SERIAL_3_SRL_DE);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_SERIALIZER, 0);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_HVF_CNTRL_1, HVF_CNTRL_1_VQR_FULL);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_RPT_CNTRL, 0);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_SEL_CLK, SEL_CLK_SEL_VRF_CLK(0) | SEL_CLK_SEL_CLK1 | SEL_CLK_ENA_SC_CLK);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_PLL_SERIAL_2, PLL_SERIAL_2_SRL_NOSC(div) | PLL_SERIAL_2_SRL_PR(0));
    set_reg_8(TDA19988_ADDR_HDMI, TDA_MAT_CONTRL, MAT_CONTRL_MAT_BP);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_ANA_GENERAL, 0x09);
    clear_reg_8(TDA19988_ADDR_HDMI, TDA_TBG_CNTRL_0, TBG_CNTRL_0_SYNC_MTHD);

    /*
     * Sync on rising HSYNC/VSYNC
     */
    reg = VIP_CNTRL_3_SYNC_HS;
    if (mode->flags & VID_NHSYNC)
    {
        reg |= VIP_CNTRL_3_H_TGL;
    }
    if (mode->flags & VID_NVSYNC)
    {
        reg |= VIP_CNTRL_3_V_TGL;
    }
    write_reg_8(TDA19988_ADDR_HDMI, TDA_VIP_CNTRL_3, reg);

    reg = TBG_CNTRL_1_TGL_EN;
    if (mode->flags & VID_NHSYNC)
    {
        reg |= TBG_CNTRL_1_H_TGL;
    }
    if (mode->flags & VID_NVSYNC)
    {
        reg |= TBG_CNTRL_1_V_TGL;
    }
    write_reg_8(TDA19988_ADDR_HDMI, TDA_TBG_CNTRL_1, reg);

    /* Program timing */
    write_reg_8(TDA19988_ADDR_HDMI, TDA_VIDFORMAT, 0x00);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_REFPIX_MSB, ref_pix);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_REFLINE_MSB, ref_line);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_NPIX_MSB, n_pix);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_NLINE_MSB, n_line);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_VS_LINE_STRT_1_MSB, vs1_line_start);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_VS_PIX_STRT_1_MSB, vs1_pix_start);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_VS_LINE_END_1_MSB, vs1_line_end);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_VS_PIX_END_1_MSB, vs1_pix_stop);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_VS_LINE_STRT_2_MSB, vs2_line_start);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_VS_PIX_STRT_2_MSB, vs2_pix_start);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_VS_LINE_END_2_MSB, vs2_line_end);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_VS_PIX_END_2_MSB, vs2_pix_stop);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_HS_PIX_START_MSB, hs_pix_start);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_HS_PIX_STOP_MSB, hs_pix_stop);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_VWIN_START_1_MSB, vwin1_line_start);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_VWIN_END_1_MSB, vwin1_line_end);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_VWIN_START_2_MSB, vwin2_line_start);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_VWIN_END_2_MSB, vwin2_line_end);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_DE_START_MSB, de_start);
    write_reg_16(TDA19988_ADDR_HDMI, TDA_DE_STOP_MSB, de_stop);

    if (version == TDA19988)
        write_reg_8(TDA19988_ADDR_HDMI, TDA_ENABLE_SPACE, 0x00);

    /* must be last register set */
    clear_reg_8(TDA19988_ADDR_HDMI, TDA_TBG_CNTRL_0, TBG_CNTRL_0_SYNC_ONCE);
}

void tda19988_configure()
{
    uint8_t data;

    /* Turn on HDMI interface */
    write_reg_8(TDA19988_ADDR_CEC, TDA_CEC_ENAMODS, ENAMODS_RXSENS | ENAMODS_HDMI);
    HAL_Delay(200);
    read_reg_8(TDA19988_ADDR_CEC, 0xfe, &data);

    /* Reset core */
    set_reg_8(TDA19988_ADDR_HDMI, TDA_SOFTRESET, 3);
    HAL_Delay(200);
    clear_reg_8(TDA19988_ADDR_HDMI, TDA_SOFTRESET, 3);
    HAL_Delay(200);

    /* reset transmitter: */
    set_reg_8(TDA19988_ADDR_HDMI, TDA_MAIN_CNTRL0, MAIN_CNTRL0_SR);
    clear_reg_8(TDA19988_ADDR_HDMI, TDA_MAIN_CNTRL0, MAIN_CNTRL0_SR);

    /* PLL registers common configuration */
    write_reg_8(TDA19988_ADDR_HDMI, TDA_PLL_SERIAL_1, 0x00);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_PLL_SERIAL_2, PLL_SERIAL_2_SRL_NOSC(1));
    write_reg_8(TDA19988_ADDR_HDMI, TDA_PLL_SERIAL_3, 0x00);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_SERIALIZER, 0x00);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_BUFFER_OUT, 0x00);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_PLL_SCG1, 0x00);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_SEL_CLK, SEL_CLK_SEL_CLK1 | SEL_CLK_ENA_SC_CLK);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_PLL_SCGN1, 0xfa);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_PLL_SCGN2, 0x00);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_PLL_SCGR1, 0x5b);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_PLL_SCGR2, 0x00);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_PLL_SCG2, 0x10);

    /* Write the default value MUX register */
    write_reg_8(TDA19988_ADDR_HDMI, TDA_MUX_VP_VIP_OUT, 0x24);

    version = 0;
    read_reg_8(TDA19988_ADDR_HDMI, TDA_VERSION, &data);
    version |= data;
    read_reg_8(TDA19988_ADDR_HDMI, TDA_VERSION_MSB, &data);
    version |= (data << 8);

    /* Clear feature bits */
    version = version & ~0x30;
    switch (version)
    {
        case TDA19988:
            printf("TDA19988\n");
            break;
        default:
            printf("Unknown device: %04x\n", version);
            goto done;
    }

    write_reg_8(TDA19988_ADDR_HDMI, TDA_DDC_CTRL, DDC_ENABLE);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_TX3, 39);
    write_reg_8(TDA19988_ADDR_CEC, TDA_CEC_FRO_IM_CLK_CTRL, CEC_FRO_IM_CLK_CTRL_GHOST_DIS | CEC_FRO_IM_CLK_CTRL_IMCLK_SEL);

    /*if (tda19988_read_edid(sc) < 0) {
        printf("Failed to read EDID\n");
        goto done;
    }*/

    /* Default values for RGB 4:4:4 mapping */
    write_reg_8(TDA19988_ADDR_HDMI, TDA_VIP_CNTRL_0, 0x23);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_VIP_CNTRL_1, 0x01);
    write_reg_8(TDA19988_ADDR_HDMI, TDA_VIP_CNTRL_2, 0x45);
done:
    ;  
}



void tda19988_irq()
{
    uint8_t val;

}
