/*
 * terminal
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
 * This utility is resposible for the console that is available
 * from usb cable. It handles incoming commands and also outgoing
 * prints from whole software. This file implements the printf
 * function.
 */

#include "serv_term.h"
#include "fsm.h"
#include "drv_i2c.h"
#include "dev_tda19988.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "diag.h"
#include <string.h>

#define STDOUT_FILENO 	1
#define STDERR_FILENO 	2

#define KEY_DEL             0x7F
#define IPSR_THREADED_MODE  0x01
#define TERM_ROW_MAX        4000
#define TERM_RAW_MAX        1024
#define TERM_CMD_MAX        128

USBD_HandleTypeDef g_usbd_device;
extern USBD_DescriptorsTypeDef VCP_Desc;
extern USBD_CDC_ItfTypeDef  USBD_CDC_fops;

typedef enum
{
    CMD_I2C_CEC_READ,
    CMD_I2C_CEC_WRITE,
    CMD_I2C_HDMI_READ,
    CMD_I2C_HDMI_WRITE,
    CMD_MEM_READ,
    CMD_MEM_WRITE,
    CMD_DIAG_VIDEO,
    CMD_DIAG_SDCARD,
    CMD_DIAG_SDRAM,
    CMD_MAX
} cmd_t;

static char *g_cmd_list_ap[CMD_MAX+1] =
{
    "tcr",
    "tcw",
    "thr",
    "thw",
    "mr",
    "mw",
    "dv",
    "dsc",
    "dsr",
    NULL
};

static char *g_term_help_p = "[tcr <reg>], read tda19988 cec register\r" \
                             "[tcw <reg> <val>], write tda19988 cec register\r" \
                             "[thr <reg>], read tda19988 hdmi register\r" \
                             "[thw <reg> <val>], write tda19988 hdmi register\r" \
                             "[mr <addr>], read memory address\r" \
                             "[mw <addr> <val>], write to memory address\r" \
                             "[dv], video diag\r" \
                             "[dsc], sdcard diag\r" \
                             "[dsr], sdram diag (reset needed!)\r";

static char g_cmd_input_str_p[TERM_CMD_MAX];
static uint32_t g_cmd_input_cnt = 0;
static char g_delimiter_a[2] = " ";
static char *g_term_text_rows_pp[TERM_ROW_MAX];
static uint32_t g_term_text_row_cnt;
static char *g_raw_text_p;
static uint32_t g_raw_text_cnt;

static uint32_t term_atoi(char *str_p)
{
    if(str_p[1] == 'x')
    {
        return strtoul(str_p, NULL, 16);
    }
    else
    {
        return atoi(str_p);
    }
}

static void interpret(char *buf, uint32_t len)
{
    char *argsv_pp[16];
    uint8_t argsc = 0;
    uint8_t i;
    cmd_t cmd = CMD_MAX;

    memset(argsv_pp, 0x00, 16 * sizeof(char *));

    argsv_pp[argsc] = strtok((char *)buf, g_delimiter_a);
    argsc++;
    while(argsv_pp[argsc-1] != NULL)
    {
        argsv_pp[argsc] = strtok(NULL, g_delimiter_a);
        argsc++;
    }

    for(i = 0; i < CMD_MAX; i++)
    {
        if(strcmp(argsv_pp[0], g_cmd_list_ap[i]) == 0)
        {
            cmd = (cmd_t)i;
            break;
        }
    }

    switch(cmd)
    {
        case CMD_I2C_CEC_READ:
        case CMD_I2C_HDMI_READ:
        {
            drv_i2c_status_t res = DRV_I2C_STATUS_OK;
            uint8_t reg = 0xFF;
            uint8_t val = 0xFF;
            if(argsv_pp[1] == NULL)
            {
                printf("need one argument!");
                break;
            }
            reg = term_atoi(argsv_pp[1]);
            if(cmd == CMD_I2C_CEC_READ)
            {
                res = drv_i2c_rd_reg_8(TDA19988_ADDR_CEC, reg, &val);
            }
            else
            {
                res = drv_i2c_rd_reg_8(TDA19988_ADDR_HDMI, reg, &val);
            }

            if(res == DRV_I2C_STATUS_OK)
            {
                printf("reading 0x%02X from tda19988 register 0x%02X", val, reg);
            }
            else
            {
                printf("failed to read tda19988 register!");
            }
        }
        break;
        case CMD_I2C_HDMI_WRITE:
        case CMD_I2C_CEC_WRITE:
        {
            drv_i2c_status_t res = DRV_I2C_STATUS_OK;
            uint8_t reg = 0xFF;
            uint8_t val = 0xFF;
            if(argsv_pp[1] == NULL || argsv_pp[2] == NULL)
            {
                printf("need two arguments!");
                break;
            }
            reg = term_atoi(argsv_pp[1]);
            val = term_atoi(argsv_pp[2]);
            if(cmd == CMD_I2C_CEC_WRITE)
            {
                res = drv_i2c_wr_reg_8(TDA19988_ADDR_CEC, reg, val);
            }
            else
            {
                res = drv_i2c_wr_reg_8(TDA19988_ADDR_HDMI, reg, val);
            }

            if(res == DRV_I2C_STATUS_OK)
            {
                printf("writing 0x%02X to tda19988 register 0x%02X", val, reg);
            }
            else
            {
                printf("failed to write tda19988 register!");   
            }
        }
        break;
        case CMD_MEM_READ:
        {
            uint32_t addr;
            uint8_t val;
            if(argsv_pp[1] == NULL)
            {
                printf("need one argument!");
                break;
            }

            addr = term_atoi(argsv_pp[1]);
            val = *(uint8_t *)addr;
            printf("reading 0x%02X from mem address 0x%02X", val, (unsigned int)addr);
        }
        break;
        case CMD_MEM_WRITE:
        {
            uint32_t addr;
            uint8_t val;
            if(argsv_pp[1] == NULL || argsv_pp[2] == NULL)
            {
                printf("need two arguments!");
                break;
            }

            addr = term_atoi(argsv_pp[1]);
            val = term_atoi(argsv_pp[2]);
            *(uint8_t *)addr = val;
            printf("writing 0x%02X to mem address 0x%02X", val, (unsigned int)addr);
        }
        break;
        case CMD_DIAG_VIDEO:
            diag_video();
        break;
        case CMD_DIAG_SDCARD:
            diag_sdcard();
        break;
        case CMD_DIAG_SDRAM:
            diag_sdram();
        break;
    default:
        printf("%s", g_term_help_p);
    }

    printf("\n");
    CDC_Itf_Flush();
}

/* Remember to only use one flush */
static void receive(uint8_t *buf_p, uint32_t len)
{
    uint8_t i;

    /* Ignore head backspace char */
    while(buf_p[0] == '\177' && g_cmd_input_cnt == 0)
    {
        buf_p++;
        len--;
        
        if(len == 0)
        {
            return;
        }
    }

    if((g_cmd_input_cnt + len) >= TERM_CMD_MAX)
    {
        printf("command too long, buffer reset!");
        memset(g_cmd_input_str_p, 0x00, TERM_CMD_MAX);
        g_cmd_input_cnt = 0;
        return;
    }

    memcpy(g_cmd_input_str_p + g_cmd_input_cnt, buf_p, len);

    g_cmd_input_cnt += len;

    for(i = 0; i < g_cmd_input_cnt; i++)
    {
        if(g_cmd_input_str_p[i] == '\r')
        {
            uint32_t j;

            /* Remove CR and NL */
            while(g_cmd_input_str_p[g_cmd_input_cnt - 1] == '\n' ||
                  g_cmd_input_str_p[g_cmd_input_cnt - 1] == '\r')
            {
                g_cmd_input_str_p[g_cmd_input_cnt - 1] = '\0';
                g_cmd_input_cnt--;
            }

            /* Fix backspaces if any */
            for(j = 0; j < g_cmd_input_cnt; j++)
            {
                if(g_cmd_input_str_p[j] == KEY_DEL)
                {
                    uint32_t k;

                    for(k = j; k < g_cmd_input_cnt; k++)
                    {
                        g_cmd_input_str_p[k-1] = g_cmd_input_str_p[k+1];
                    }
                    g_cmd_input_cnt -= 2; /* both backspace and the char it deleted must go */
                    j -= 2;
                }
            }

            printf("\r");

            /* Interpret the command */
            interpret(g_cmd_input_str_p, g_cmd_input_cnt);
            memset(g_cmd_input_str_p, 0x00, TERM_CMD_MAX);
            g_cmd_input_cnt = 0;
            return;
        }
    }

    /* Remote echo */
    CDC_Itf_Send((uint8_t *)buf_p, len);
    CDC_Itf_Flush();
}

static void clear_all_rows()
{
    uint32_t i;

    for(i = 0; i < g_term_text_row_cnt; i++)
    {
        free(g_term_text_rows_pp[i]);
        g_term_text_rows_pp[i] = NULL;
    }

    g_term_text_row_cnt = 0;
}

static void record_row(char *text_p, uint32_t len)
{
    if(g_term_text_row_cnt < TERM_ROW_MAX)
    {
        g_term_text_rows_pp[g_term_text_row_cnt] = calloc(1, len + 1);
        memcpy(g_term_text_rows_pp[g_term_text_row_cnt], text_p, len);
        g_term_text_row_cnt++;
        fsm_event(FSM_EVENT_TEXT_ROW, (uint32_t)text_p, len);
    }
    else
    {
        /* TODO: perhaps just delete oldest half and then shift to beginning ? */
        clear_all_rows();
    }
}

void serv_term_init()
{
  /* Init Device Library */
  USBD_Init(&g_usbd_device, &VCP_Desc, 0);
  
  /* Add Supported Class */
  USBD_RegisterClass(&g_usbd_device, USBD_CDC_CLASS);
  
  /* Add CDC Interface Class */
  USBD_CDC_RegisterInterface(&g_usbd_device, &USBD_CDC_fops);
  
  /* Start Device Process */
  USBD_Start(&g_usbd_device);

  /* Enable the USB voltage level detector */
  HAL_PWREx_EnableUSBVoltageDetector();

  /* Register receive function */
  CDC_Iif_RegisterReceiveCb(receive);

  memset(g_cmd_input_str_p, 0x00, TERM_CMD_MAX);

  g_raw_text_p = calloc(1, TERM_RAW_MAX);
}

char *serv_term_get_row(uint32_t row)
{
    if(row < TERM_ROW_MAX)
    {
        return g_term_text_rows_pp[row];
    }

    return NULL;
}

uint32_t serv_term_get_rows()
{
    return g_term_text_row_cnt;
}

void serv_term_clear_rows()
{
    clear_all_rows();
}

void serv_term_receive(uint8_t *buf_p, uint32_t len)
{
    receive(buf_p, len);
}

/* So that printf() will output on this device */
int _write(int file, char *ptr, int len)
{
    uint32_t i;

    if(len == 0)
    {
        return len;
    }

    switch (file)
    {
    case STDOUT_FILENO: /*stdout*/
        CDC_Itf_Send((uint8_t *)ptr, (uint32_t)len);
        break;
    case STDERR_FILENO: /* stderr */
        CDC_Itf_Send((uint8_t *)ptr, (uint32_t)len);
        break;
    default:
        return -1;
    }

    for(i = 0; i < len; i++)
    {
        g_raw_text_p[g_raw_text_cnt] = ptr[i];

        if(g_raw_text_p[g_raw_text_cnt] == '\n' || g_raw_text_p[g_raw_text_cnt] == '\r')
        {
            g_raw_text_p[g_raw_text_cnt] = '\0';
            record_row(g_raw_text_p, g_raw_text_cnt);
            g_raw_text_cnt = 0;
        }
        else
        {
            if(g_raw_text_cnt < TERM_RAW_MAX - 1)
            {
                g_raw_text_cnt++;
            }
        }
    }

    /*
     * If thread mode the buffer can be flushed at any time.
     * This is not the case when in interrupt context, since
     * here only one flush can be done per IRQ.
     */
    if(__get_IPSR() & IPSR_THREADED_MODE)
    {
        CDC_Itf_Flush();
    }

    return len;
}
