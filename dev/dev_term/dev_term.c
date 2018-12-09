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

#include "dev_term.h"
#include "drv_i2c.h"
#include "tda19988.h"
#include "stage.h"
#include "sm.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include <string.h>

#define STDOUT_FILENO 	1
#define STDERR_FILENO 	2

#define IPSR_THREADED_MODE      0x01

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
    CMD_MAX
} cmd_t;

static char *g_cmd_list_ap[CMD_MAX+1] = {"i2c_cec_read", "i2c_cec_write", "i2c_hdmi_read", "i2c_hdmi_write", "mem_read", "mem_write", NULL};

static char *g_term_help_p =   "[i2c_cec_read <reg>], read tda19988 register\r" \
                                "[i2c_cec_write <reg> <val>], write tda19988 register\r" \
                                "[i2c_hdmi_write <reg>], read tda19988 register\r" \
                                "[i2c_hdmi_write <reg> <val>], write tda19988 register\r" \
                                "[mem_read <addr>], read memory address\r" \
                                "[mem_write <addr> <val>], write to memory address\n";
static uint8_t g_cmd_input_str_a[128] = "";
static uint8_t g_cmd_input_cnt = 0;
static char g_delimiter_a[2] = " ";

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

static void interpret(uint8_t *buf, uint32_t len)
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
            uint8_t reg = 0xFF;
            uint8_t val = 0xFF;
            if(argsv_pp[1] == NULL)
            {
                printf("need one argument!\n");
                break;
            }
            reg = term_atoi(argsv_pp[1]);
            if(cmd == CMD_I2C_CEC_READ)
            {
                val = drv_i2c_rd_reg(TDA19988_ADDR_CEC, reg);
            }
            else
            {
                val = drv_i2c_rd_reg(TDA19988_ADDR_HDMI, reg);
            }
            printf("reading 0x%02X from register 0x%02X\n", val, reg);
        }
        break;
        case CMD_I2C_HDMI_WRITE:
        case CMD_I2C_CEC_WRITE:
        {
            uint8_t reg = 0xFF;
            uint8_t val = 0xFF;
            if(argsv_pp[1] == NULL || argsv_pp[2] == NULL)
            {
                printf("need two arguments!\n");
                break;
            }
            reg = term_atoi(argsv_pp[1]);
            val = term_atoi(argsv_pp[2]);
            if(cmd == CMD_I2C_CEC_WRITE)
            {
                drv_i2c_wr_reg(TDA19988_ADDR_CEC, reg, val);
            }
            else
            {
                drv_i2c_wr_reg(TDA19988_ADDR_HDMI, reg, val);
            }
            printf("writing 0x%02X to register 0x%02X\n", val, reg);
        }
        break;
        case CMD_MEM_READ:
        {
            uint32_t addr;
            uint8_t val;
            if(argsv_pp[1] == NULL)
            {
                printf("need one argument!\n");
                break;
            }

            addr = term_atoi(argsv_pp[1]);
            val = *(uint8_t *)addr;
            printf("reading 0x%02X from address 0x%02X\n", val, (unsigned int)addr);
        }
        break;
        case CMD_MEM_WRITE:
        {
            uint32_t addr;
            uint8_t val;
            if(argsv_pp[1] == NULL || argsv_pp[2] == NULL)
            {
                printf("need two arguments!\n");
                break;
            }

            addr = term_atoi(argsv_pp[1]);
            val = term_atoi(argsv_pp[2]);
            *(uint8_t *)addr = val;
            printf("writing 0x%02X to address 0x%02X\n", val, (unsigned int)addr);
        }
        break;
    default:
        printf("%s", g_term_help_p);
    }
}

static void receive(uint8_t *buf, uint32_t len)
{
    uint8_t i;

    if((g_cmd_input_cnt + len) >= 128)
    {
        printf("command too long, buffer reset!\n");
        memset(g_cmd_input_str_a, 0x00, 128);
        g_cmd_input_cnt = 0;
        return;
    }

    memcpy(g_cmd_input_str_a + g_cmd_input_cnt, buf, len);

    g_cmd_input_cnt += len;

    for(i = 0; i < g_cmd_input_cnt; i++)
    {
        if(g_cmd_input_str_a[i] == '\r')
        {
            /* Send new line */
            CDC_Itf_Send((uint8_t *)"\n", 1);

            /* Remove CR and NL */
            while(g_cmd_input_str_a[g_cmd_input_cnt - 1] == '\n' ||
                  g_cmd_input_str_a[g_cmd_input_cnt - 1] == '\r')
            {
                g_cmd_input_str_a[g_cmd_input_cnt - 1] = '\0';
                g_cmd_input_cnt--;
            }

            /* Interpret the command */
            interpret(g_cmd_input_str_a, g_cmd_input_cnt);
            memset(g_cmd_input_str_a, 0x00, 128);
            g_cmd_input_cnt = 0;
            return;
        }
    }

    /* Remote echo */
    CDC_Itf_Send((uint8_t *)buf, len);
}

void dev_term_init()
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

  memset(g_cmd_input_str_a, 0x00, 128);
}

/* So that printf() will output on this device */
int _write(int file, char *ptr, int len)
{
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
