ARCH_FLAGS=-mcpu=cortex-m7
CC = arm-none-eabi-gcc
AR = arm-none-eabi-gcc-ar

BL_CFLAGS = $(ARCH_FLAGS) $(BL_INCLUDES) -mthumb -Wall -static -ffunction-sections -Ofast -c -DSTM32H743xx
BL_LFLAGS = $(ARCH_FLAGS) --specs=nosys.specs -mthumb -static -Ofast -Wl,-Map=./out_bl/bl.map,--gc-section,-T ./link.ld

# NOTE: If building bootloader then change to ORIGIN = 0x08000000 in link.ld
# and also change the vector tables in system_stm32h7xx.c to 0x00000 (VECT_TAB_OFFSET).
TARGET_CFLAGS = $(ARCH_FLAGS) $(TARGET_INCLUDES) -mthumb -Wall -static -ffunction-sections -g -c -DSTM32H743xx
TARGET_LFLAGS = $(ARCH_FLAGS) --specs=nosys.specs -mthumb -static -g -Wl,-Map=./out_target/target.map,--gc-section,-T ./link.ld

BL_INCLUDES := \
	-I./if \
	-I./cmsis \
	-I./cmsis_boot \
	-I./hal \
	-I./irq \
	-I./hal/Legacy \
	-I./diag \
	-I./drv/drv_sidbus \
	-I./drv/drv_timer \
	-I./drv/drv_ltdc \
	-I./drv/drv_sdcard \
	-I./drv/drv_crc \
	-I./drv/drv_rng \
	-I./drv/drv_sdram \
	-I./dev/dev_tda19988 \
	-I./dev/dev_is42s16400j \
	-I./dev/dev_mos658x \
	-I./drv/drv_usbd \
	-I./drv/drv_usbh \
	-I./drv/drv_joyst \
	-I./drv/drv_led \
	-I./hal_setup \
	-I./mware/fatfs \
	-I./mware/usb_host/core \
	-I./mware/usb_host/hid \
	-I./mware/usb_host/cust \
	-I./mware/usb_device/core \
	-I./mware/usb_device/cdc \
	-I./mware/usb_device/cust \
	-I./app \
	-I./rom \
	-I./hostif


BL_LINK_FILES := \
	./out_bl/startup_stm32h743xx.o \
	./out_bl/bl.o \
	./out_bl/sdcard.o \
	./out_bl/diskio.o \
	./out_bl/system_stm32h7xx.o \
	./out_bl/stm32h7xx_hal_flash.o \
	./out_bl/stm32h7xx_hal_flash_ex.o \
	./out_bl/stm32h7xx_hal_sd.o \
	./out_bl/stm32h7xx_hal.o \
	./out_bl/stm32h7xx_hal_gpio.o \
	./out_bl/stm32h7xx_hal_cortex.o \
	./out_bl/stm32h7xx_hal_pwr.o \
	./out_bl/stm32h7xx_hal_pwr_ex.o \
	./out_bl/stm32h7xx_hal_rcc.o \
	./out_bl/stm32h7xx_hal_rcc_ex.o \
	./out_bl/stm32h7xx_ll_sdmmc.o \
	./out_bl/stm32h7xx_ll_delayblock.o \
	./out_bl/ff.o \
	./out_bl/ccsbcs.o

TARGET_INCLUDES := \
	-I./serv/serv_video \
	-I./serv/serv_mem \
	-I./serv/serv_storage \
	-I./serv/serv_audio \
	-I./serv/serv_keybd \
	-I./serv/serv_misc \
	-I./if \
	-I./cmsis \
	-I./cmsis_boot \
	-I./hal \
	-I./irq \
	-I./hal/Legacy \
	-I./diag \
	-I./drv/drv_sidbus \
	-I./drv/drv_timer \
	-I./drv/drv_ltdc \
	-I./drv/drv_sdcard \
	-I./drv/drv_crc \
	-I./drv/drv_rng \
	-I./drv/drv_sdram \
	-I./dev/dev_tda19988 \
	-I./dev/dev_is42s16400j \
	-I./dev/dev_mos658x \
	-I./drv/drv_usbd \
	-I./drv/drv_usbh \
	-I./drv/drv_joyst \
	-I./drv/drv_led \
	-I./hal_setup \
	-I./drv/drv_i2c \
	-I./mware/fatfs \
	-I./mware/usb_host/core \
	-I./mware/usb_host/hid \
	-I./mware/usb_host/cust \
	-I./mware/usb_device/core \
	-I./mware/usb_device/cdc \
	-I./mware/usb_device/cust \
	-I./serv/serv_term \
	-I./app \
	-I./rom \
	-I./hostif

TARGET_LINK_FILES := \
	./out_target/serv_video.o \
	./out_target/serv_mem.o \
	./out_target/serv_storage.o \
	./out_target/serv_audio.o \
	./out_target/serv_keybd.o \
	./out_target/serv_misc.o \
	./out_target/startup_stm32h743xx.o \
	./out_target/drv_sidbus.o \
	./out_target/drv_timer.o \
	./out_target/drv_sdcard.o \
	./out_target/drv_crc.o \
	./out_target/drv_rng.o \
	./out_target/drv_joyst.o \
	./out_target/drv_led.o \
	./out_target/diskio.o \
	./out_target/drv_sdram.o \
	./out_target/dev_tda19988.o \
	./out_target/dev_is42s16400j.o \
	./out_target/dev_mos658x.o \
	./out_target/drv_ltdc.o \
	./out_target/drv_i2c.o \
	./out_target/hal_conf.o \
	./out_target/hal_msp.o \
	./out_target/serv_term.o \
	./out_target/diag.o \
	./out_target/usbh_conf.o \
	./out_target/usbd_cdc_if.o \
	./out_target/usbd_desc.o \
	./out_target/usbd_conf.o \
	./out_target/system_stm32h7xx.o \
	./out_target/stm32h7xx_hal_sd.o \
	./out_target/stm32h7xx_hal_sd_ex.o \
	./out_target/stm32h7xx_hal.o \
	./out_target/stm32h7xx_hal_ltdc.o \
	./out_target/stm32h7xx_hal_sdram.o \
	./out_target/stm32h7xx_hal_pcd.o \
	./out_target/stm32h7xx_hal_pcd_ex.o \
	./out_target/stm32h7xx_hal_gpio.o \
	./out_target/stm32h7xx_hal_cortex.o \
	./out_target/stm32h7xx_hal_pwr.o \
	./out_target/stm32h7xx_hal_pwr_ex.o \
	./out_target/stm32h7xx_hal_hcd.o \
	./out_target/stm32h7xx_hal_rcc.o \
	./out_target/stm32h7xx_hal_i2c.o \
	./out_target/stm32h7xx_hal_rcc_ex.o \
	./out_target/stm32h7xx_hal_i2c_ex.o \
	./out_target/stm32h7xx_hal_crc.o \
	./out_target/stm32h7xx_hal_crc_ex.o \
	./out_target/stm32h7xx_hal_rng.o \
	./out_target/stm32h7xx_ll_fmc.o \
	./out_target/stm32h7xx_ll_sdmmc.o \
	./out_target/stm32h7xx_ll_usb.o \
	./out_target/stm32h7xx_hal_tim.o \
	./out_target/stm32h7xx_hal_tim_ex.o \
	./out_target/stm32h7xx_hal_dma.o \
	./out_target/stm32h7xx_hal_dma_ex.o \
	./out_target/stm32h7xx_hal_flash.o \
	./out_target/stm32h7xx_hal_flash_ex.o \
	./out_target/stm32h7xx_ll_delayblock.o \
	./out_target/irq.o \
	./out_target/usbh_core.o \
	./out_target/usbh_ctlreq.o \
	./out_target/usbh_ioreq.o \
	./out_target/usbh_pipes.o \
	./out_target/usbh_hid.o \
	./out_target/usbh_hid_keybd.o \
	./out_target/usbh_hid_parser.o \
	./out_target/usbd_core.o \
	./out_target/usbd_ctlreq.o \
	./out_target/usbd_ioreq.o \
	./out_target/usbd_cdc.o \
	./out_target/ff.o \
	./out_target/ccsbcs.o \
	./out_target/hostif.o \
	./out_target/main.o \
	./out_target/fsm.o \
	./out_target/romcc.o \
	./out_target/romdd.o

BL = bootloader
TARGET = target

all: $(TARGET)

$(BL):
	@mkdir ./out_bl 2>/dev/null; true
	@echo Compiling bootloader...
	$(CC) $(BL_CFLAGS) -o out_bl/sdcard.o ./drv/drv_sdcard/drv_sdcard.c
	$(CC) $(BL_CFLAGS) -o out_bl/diskio.o ./drv/drv_sdcard/diskio.c
	$(CC) $(BL_CFLAGS) -o out_bl/ltdc.o ./drv/drv_ltdc/drv_ltdc.c
	$(CC) $(BL_CFLAGS) -o out_bl/system_stm32h7xx.o ./cmsis_boot/system_stm32h7xx.c
	$(CC) $(BL_CFLAGS) -o out_bl/stm32h7xx_hal_sd.o ./hal/stm32h7xx_hal_sd.c
	$(CC) $(BL_CFLAGS) -o out_bl/stm32h7xx_hal.o ./hal/stm32h7xx_hal.c
	$(CC) $(BL_CFLAGS) -o out_bl/stm32h7xx_hal_flash.o ./hal/stm32h7xx_hal_flash.c
	$(CC) $(BL_CFLAGS) -o out_bl/stm32h7xx_hal_flash_ex.o ./hal/stm32h7xx_hal_flash_ex.c
	$(CC) $(BL_CFLAGS) -o out_bl/stm32h7xx_hal_ltdc.o ./hal/stm32h7xx_hal_ltdc.c
	$(CC) $(BL_CFLAGS) -o out_bl/stm32h7xx_hal_gpio.o ./hal/stm32h7xx_hal_gpio.c
	$(CC) $(BL_CFLAGS) -o out_bl/stm32h7xx_hal_cortex.o ./hal/stm32h7xx_hal_cortex.c
	$(CC) $(BL_CFLAGS) -o out_bl/stm32h7xx_hal_pwr.o ./hal/stm32h7xx_hal_pwr.c
	$(CC) $(BL_CFLAGS) -o out_bl/stm32h7xx_hal_pwr_ex.o ./hal/stm32h7xx_hal_pwr_ex.c
	$(CC) $(BL_CFLAGS) -o out_bl/stm32h7xx_hal_rcc.o ./hal/stm32h7xx_hal_rcc.c
	$(CC) $(BL_CFLAGS) -o out_bl/stm32h7xx_hal_rcc_ex.o ./hal/stm32h7xx_hal_rcc_ex.c
	$(CC) $(BL_CFLAGS) -o out_bl/stm32h7xx_ll_sdmmc.o ./hal/stm32h7xx_ll_sdmmc.c
	$(CC) $(BL_CFLAGS) -o out_bl/stm32h7xx_ll_delayblock.o ./hal/stm32h7xx_ll_delayblock.c
	$(CC) $(BL_CFLAGS) -o out_bl/irq.o ./irq/irq.c
	$(CC) $(BL_CFLAGS) -o out_bl/ff.o ./mware/fatfs/ff.c
	$(CC) $(BL_CFLAGS) -o out_bl/ccsbcs.o ./mware/fatfs/ccsbcs.c
	$(CC) $(BL_CFLAGS) -o out_bl/bl.o ./bl/bl.c
	$(CC) $(BL_CFLAGS) -o out_bl/startup_stm32h743xx.o ./cmsis_boot/startup/startup_stm32h743xx.s

	@echo Linking...
	$(CC) $(BL_LFLAGS) -o out_bl/bl.elf $(BL_LINK_FILES)


$(TARGET):
	@mkdir ./out_target 2>/dev/null; true
	@echo Compiling target...
	$(CC) $(TARGET_CFLAGS) -o out_target/serv_video.o ./serv/serv_video/serv_video.c
	$(CC) $(TARGET_CFLAGS) -o out_target/serv_mem.o ./serv/serv_mem/serv_mem.c
	$(CC) $(TARGET_CFLAGS) -o out_target/serv_storage.o ./serv/serv_storage/serv_storage.c
	$(CC) $(TARGET_CFLAGS) -o out_target/serv_audio.o ./serv/serv_audio/serv_audio.c
	$(CC) $(TARGET_CFLAGS) -o out_target/serv_keybd.o ./serv/serv_keybd/serv_keybd.c
	$(CC) $(TARGET_CFLAGS) -o out_target/serv_misc.o ./serv/serv_misc/serv_misc.c
	$(CC) $(TARGET_CFLAGS) -o out_target/drv_sidbus.o ./drv/drv_sidbus/drv_sidbus.c
	$(CC) $(TARGET_CFLAGS) -o out_target/drv_timer.o ./drv/drv_timer/drv_timer.c
	$(CC) $(TARGET_CFLAGS) -o out_target/drv_sdcard.o ./drv/drv_sdcard/drv_sdcard.c
	$(CC) $(TARGET_CFLAGS) -o out_target/diskio.o ./drv/drv_sdcard/diskio.c
	$(CC) $(TARGET_CFLAGS) -o out_target/drv_crc.o ./drv/drv_crc/drv_crc.c
	$(CC) $(TARGET_CFLAGS) -o out_target/drv_rng.o ./drv/drv_rng/drv_rng.c
	$(CC) $(TARGET_CFLAGS) -o out_target/drv_joyst.o ./drv/drv_joyst/drv_joyst.c
	$(CC) $(TARGET_CFLAGS) -o out_target/drv_led.o ./drv/drv_led/drv_led.c
	$(CC) $(TARGET_CFLAGS) -o out_target/drv_sdram.o ./drv/drv_sdram/drv_sdram.c
	$(CC) $(TARGET_CFLAGS) -o out_target/dev_tda19988.o ./dev/dev_tda19988/dev_tda19988.c
	$(CC) $(TARGET_CFLAGS) -o out_target/dev_is42s16400j.o ./dev/dev_is42s16400j/dev_is42s16400j.c
	$(CC) $(TARGET_CFLAGS) -o out_target/dev_mos658x.o ./dev/dev_mos658x/dev_mos658x.c
	$(CC) $(TARGET_CFLAGS) -o out_target/drv_ltdc.o ./drv/drv_ltdc/drv_ltdc.c
	$(CC) $(TARGET_CFLAGS) -o out_target/drv_i2c.o ./drv/drv_i2c/drv_i2c.c
	$(CC) $(TARGET_CFLAGS) -o out_target/hal_conf.o ./hal_setup/hal_conf.c
	$(CC) $(TARGET_CFLAGS) -o out_target/hal_msp.o ./hal_setup/hal_msp.c
	$(CC) $(TARGET_CFLAGS) -o out_target/serv_term.o ./serv/serv_term/serv_term.c
	$(CC) $(TARGET_CFLAGS) -o out_target/diag.o ./diag/diag.c
	$(CC) $(TARGET_CFLAGS) -o out_target/system_stm32h7xx.o ./cmsis_boot/system_stm32h7xx.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_sd.o ./hal/stm32h7xx_hal_sd.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_sd_ex.o ./hal/stm32h7xx_hal_sd_ex.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal.o ./hal/stm32h7xx_hal.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_ltdc.o ./hal/stm32h7xx_hal_ltdc.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_sdram.o ./hal/stm32h7xx_hal_sdram.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_pcd.o ./hal/stm32h7xx_hal_pcd.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_pcd_ex.o ./hal/stm32h7xx_hal_pcd_ex.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_gpio.o ./hal/stm32h7xx_hal_gpio.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_cortex.o ./hal/stm32h7xx_hal_cortex.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_pwr.o ./hal/stm32h7xx_hal_pwr.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_crc.o ./hal/stm32h7xx_hal_crc.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_crc_ex.o ./hal/stm32h7xx_hal_crc_ex.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_pwr_ex.o ./hal/stm32h7xx_hal_pwr_ex.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_hcd.o ./hal/stm32h7xx_hal_hcd.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_rcc.o ./hal/stm32h7xx_hal_rcc.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_i2c.o ./hal/stm32h7xx_hal_i2c.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_rcc_ex.o ./hal/stm32h7xx_hal_rcc_ex.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_i2c_ex.o ./hal/stm32h7xx_hal_i2c_ex.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_rng.o ./hal/stm32h7xx_hal_rng.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_ll_fmc.o ./hal/stm32h7xx_ll_fmc.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_ll_sdmmc.o ./hal/stm32h7xx_ll_sdmmc.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_ll_usb.o ./hal/stm32h7xx_ll_usb.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_tim.o ./hal/stm32h7xx_hal_tim.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_tim_ex.o ./hal/stm32h7xx_hal_tim_ex.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_dma.o ./hal/stm32h7xx_hal_dma.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_dma_ex.o ./hal/stm32h7xx_hal_dma_ex.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_flash.o ./hal/stm32h7xx_hal_flash.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_hal_flash_ex.o ./hal/stm32h7xx_hal_flash_ex.c
	$(CC) $(TARGET_CFLAGS) -o out_target/stm32h7xx_ll_delayblock.o ./hal/stm32h7xx_ll_delayblock.c
	$(CC) $(TARGET_CFLAGS) -o out_target/irq.o ./irq/irq.c
	$(CC) $(TARGET_CFLAGS) -o out_target/usbh_core.o ./mware/usb_host/core/usbh_core.c
	$(CC) $(TARGET_CFLAGS) -o out_target/usbh_ctlreq.o ./mware/usb_host/core/usbh_ctlreq.c
	$(CC) $(TARGET_CFLAGS) -o out_target/usbh_ioreq.o ./mware/usb_host/core/usbh_ioreq.c
	$(CC) $(TARGET_CFLAGS) -o out_target/usbh_pipes.o ./mware/usb_host/core/usbh_pipes.c
	$(CC) $(TARGET_CFLAGS) -o out_target/usbh_hid.o ./mware/usb_host/hid/usbh_hid.c
	$(CC) $(TARGET_CFLAGS) -o out_target/usbh_hid_keybd.o ./mware/usb_host/hid/usbh_hid_keybd.c
	$(CC) $(TARGET_CFLAGS) -o out_target/usbh_hid_parser.o ./mware/usb_host/hid/usbh_hid_parser.c
	$(CC) $(TARGET_CFLAGS) -DUSE_USB_HS -o out_target/usbh_conf.o ./drv/drv_usbh/usbh_conf.c
	$(CC) $(TARGET_CFLAGS) -DUSE_USB_FS -o out_target/usbd_conf.o ./drv/drv_usbd/usbd_conf.c
	$(CC) $(TARGET_CFLAGS) -o out_target/usbd_core.o ./mware/usb_device/core/usbd_core.c
	$(CC) $(TARGET_CFLAGS) -o out_target/usbd_ctlreq.o ./mware/usb_device/core/usbd_ctlreq.c
	$(CC) $(TARGET_CFLAGS) -o out_target/usbd_ioreq.o ./mware/usb_device/core/usbd_ioreq.c
	$(CC) $(TARGET_CFLAGS) -o out_target/usbd_cdc.o ./mware/usb_device/cdc/usbd_cdc.c
	$(CC) $(TARGET_CFLAGS) -o out_target/usbd_cdc_if.o ./mware/usb_device/cust/usbd_cdc_if.c
	$(CC) $(TARGET_CFLAGS) -o out_target/usbd_desc.o ./mware/usb_device/cust/usbd_desc.c
	$(CC) $(TARGET_CFLAGS) -o out_target/ff.o ./mware/fatfs/ff.c
	$(CC) $(TARGET_CFLAGS) -o out_target/ccsbcs.o ./mware/fatfs/ccsbcs.c
	$(CC) $(TARGET_CFLAGS) -DUSE_CRC -o out_target/hostif.o ./hostif/hostif.c
	$(CC) $(TARGET_CFLAGS) -o out_target/main.o ./app/main.c
	$(CC) $(TARGET_CFLAGS) -o out_target/fsm.o ./app/fsm.c
	$(CC) $(TARGET_CFLAGS) -o out_target/romcc.o ./rom/romcc.c
	$(CC) $(TARGET_CFLAGS) -o out_target/romdd.o ./rom/romdd.c
	$(CC) $(TARGET_CFLAGS) -o out_target/startup_stm32h743xx.o ./cmsis_boot/startup/startup_stm32h743xx.s

	@echo Linking...
	$(CC) $(TARGET_LFLAGS) -o out_target/target.elf $(TARGET_LINK_FILES) -L./libemucc -lemucc -L./libemudd -lemudd

	@echo Create binary...
	arm-none-eabi-objcopy -O binary out_target/target.elf out_target/target.bin

clean:
	rm -rf ./out_target ./out_bl

