/*
 *  Keewifi Kisslink NB1210 board support
 *
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"


#define KISSLINK_NB1210_GPIO_I2C_SDA		15
#define KISSLINK_NB1210_GPIO_I2C_SLC		14

#define KISSLINK_NB1210_GPIO_LED_GPIO_LED_WAN	18

#define KISSLINK_NB1210_GPIO_BTN_RESET	1

#define KISSLINK_NB1210_KEYS_POLL_INTERVAL	20	/* msecs */
#define KISSLINK_NB1210_KEYS_DEBOUNCE_INTERVAL (3 * KISSLINK_NB1210_KEYS_POLL_INTERVAL)

#define KISSLINK_NB1210_MAC0_OFFSET   0
#define KISSLINK_NB1210_WMAC_CALDATA_OFFSET   0x1000

static const char *kisslink_nb1210_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data kisslink_nb1210_flash_data = {
	.part_probes	= kisslink_nb1210_part_probes,
};

static struct i2c_gpio_platform_data kisslink_nb1210_i2c_gpio_data = {
	.sda_pin	= KISSLINK_NB1210_GPIO_I2C_SDA,
	.scl_pin	= KISSLINK_NB1210_GPIO_I2C_SLC,
};

static struct platform_device kisslink_nb1210_i2c_gpio_device = {
	.name		= "i2c-gpio",
	.id		= 0,
	.dev = {
		.platform_data  = &kisslink_nb1210_i2c_gpio_data,
	}
};

static struct gpio_led kisslink_nb1210_leds_gpio[] __initdata = {
	{
		.name		= "nb1210:green:wan",
		.gpio		= KISSLINK_NB1210_GPIO_LED_GPIO_LED_WAN,
		.active_low	= 0,
	}, 
};

static struct gpio_keys_button kisslink_nb1210_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = KISSLINK_NB1210_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= KISSLINK_NB1210_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static void __init tl_ap123_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	/* Disable JTAG, enabling GPIOs 0-3 */
	/* Configure OBS4 line, for GPIO 4*/
	ath79_gpio_function_setup(AR934X_GPIO_FUNC_JTAG_DISABLE,
				 AR934X_GPIO_FUNC_CLK_OBS4_EN);

	ath79_register_m25p80(&kisslink_nb1210_flash_data);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_PHY_SWAP);

	ath79_register_mdio(1, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + KISSLINK_NB1210_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, art + KISSLINK_NB1210_MAC0_OFFSET, 1);

	/* GMAC0 is connected to the PHY0 of the internal switch */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = BIT(0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_register_eth(0);

	/* GMAC1 is connected to the internal switch */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_register_eth(1);

	ath79_register_wmac(art + KISSLINK_NB1210_WMAC_CALDATA_OFFSET, art);

	platform_device_register(&kisslink_nb1210_i2c_gpio_device);

	ath79_register_usb();
}

static void __init kisslink_nb1210_setup(void)
{
	tl_ap123_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(kisslink_nb1210_leds_gpio) ,
				 kisslink_nb1210_leds_gpio);

	ath79_register_gpio_keys_polled(1, KISSLINK_NB1210_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(kisslink_nb1210_gpio_keys),
					kisslink_nb1210_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_KISSLINK_NB1210, "KISSLINK-NB1210", "Keewifi Kisslink NB1210",
	     kisslink_nb1210_setup);
