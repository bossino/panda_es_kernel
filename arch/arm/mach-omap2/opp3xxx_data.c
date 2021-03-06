/*
 * OMAP3 OPP table definitions.
 *
 * Copyright (C) 2009-2010 Texas Instruments Incorporated - http://www.ti.com/
 *	Nishanth Menon
 *	Kevin Hilman
 * Copyright (C) 2010-2011 Nokia Corporation.
 *      Eduardo Valentin
 *      Paul Walmsley
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/module.h>

#include <plat/cpu.h>

#include "control.h"
#include "omap_opp_data.h"
#include "pm.h"
#include "abb.h"

/* 34xx */
#define OMAP3_ON_VOLTAGE_UV		1200000
#define OMAP3_ONLP_VOLTAGE_UV		1000000
#define OMAP3_RET_VOLTAGE_UV		975000
#define OMAP3_OFF_VOLTAGE_UV		600000

/* VDD1 */

#define OMAP3430_VDD_MPU_OPP1_UV		975000
#define OMAP3430_VDD_MPU_OPP2_UV		1075000
#define OMAP3430_VDD_MPU_OPP3_UV		1200000
#define OMAP3430_VDD_MPU_OPP4_UV		1270000
#define OMAP3430_VDD_MPU_OPP5_UV		1350000

struct omap_volt_data omap34xx_vddmpu_volt_data[] = {
	VOLT_DATA_DEFINE(OMAP3430_VDD_MPU_OPP1_UV, 0, OMAP343X_CONTROL_FUSE_OPP1_VDD1, 0xf4, 0x0c, OMAP_ABB_NO_LDO),
	VOLT_DATA_DEFINE(OMAP3430_VDD_MPU_OPP2_UV, 0, OMAP343X_CONTROL_FUSE_OPP2_VDD1, 0xf4, 0x0c, OMAP_ABB_NO_LDO),
	VOLT_DATA_DEFINE(OMAP3430_VDD_MPU_OPP3_UV, 0, OMAP343X_CONTROL_FUSE_OPP3_VDD1, 0xf9, 0x18, OMAP_ABB_NO_LDO),
	VOLT_DATA_DEFINE(OMAP3430_VDD_MPU_OPP4_UV, 0, OMAP343X_CONTROL_FUSE_OPP4_VDD1, 0xf9, 0x18, OMAP_ABB_NO_LDO),
	VOLT_DATA_DEFINE(OMAP3430_VDD_MPU_OPP5_UV, 0, OMAP343X_CONTROL_FUSE_OPP5_VDD1, 0xf9, 0x18, OMAP_ABB_NO_LDO),
	VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0),
};

struct omap_vp_param omap34xx_mpu_vp_data = {
	.vddmin			= OMAP3430_VP1_VLIMITTO_VDDMIN,
	.vddmax			= OMAP3430_VP1_VLIMITTO_VDDMAX,
};

struct omap_vc_param omap34xx_mpu_vc_data = {
	.on		= OMAP3_ON_VOLTAGE_UV,
	.onlp		= OMAP3_ONLP_VOLTAGE_UV,
	.ret		= OMAP3_RET_VOLTAGE_UV,
	.off		= OMAP3_OFF_VOLTAGE_UV,
};

/* VDD2 */

#define OMAP3430_VDD_CORE_OPP1_UV		975000
#define OMAP3430_VDD_CORE_OPP2_UV		1050000
#define OMAP3430_VDD_CORE_OPP3_UV		1150000

struct omap_volt_data omap34xx_vddcore_volt_data[] = {
	VOLT_DATA_DEFINE(OMAP3430_VDD_CORE_OPP1_UV, 0, OMAP343X_CONTROL_FUSE_OPP1_VDD2, 0xf4, 0x0c, OMAP_ABB_NO_LDO),
	VOLT_DATA_DEFINE(OMAP3430_VDD_CORE_OPP2_UV, 0, OMAP343X_CONTROL_FUSE_OPP2_VDD2, 0xf4, 0x0c, OMAP_ABB_NO_LDO),
	VOLT_DATA_DEFINE(OMAP3430_VDD_CORE_OPP3_UV, 0, OMAP343X_CONTROL_FUSE_OPP3_VDD2, 0xf9, 0x18, OMAP_ABB_NO_LDO),
	VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0),
};

/* OMAP 3430 MPU Core VDD dependency table */
static struct omap_vdd_dep_volt omap34xx_vdd_mpu_core_dep_data[] = {
	{.main_vdd_volt = OMAP3430_VDD_MPU_OPP1_UV, .dep_vdd_volt = OMAP3430_VDD_CORE_OPP2_UV},
	{.main_vdd_volt = OMAP3430_VDD_MPU_OPP2_UV, .dep_vdd_volt = OMAP3430_VDD_CORE_OPP2_UV},
	{.main_vdd_volt = OMAP3430_VDD_MPU_OPP3_UV, .dep_vdd_volt = OMAP3430_VDD_CORE_OPP3_UV},
	{.main_vdd_volt = OMAP3430_VDD_MPU_OPP4_UV, .dep_vdd_volt = OMAP3430_VDD_CORE_OPP3_UV},
	{.main_vdd_volt = OMAP3430_VDD_MPU_OPP5_UV, .dep_vdd_volt = OMAP3430_VDD_CORE_OPP3_UV},
};

struct omap_vdd_dep_info omap34xx_vddmpu_dep_info[] = {
	{
		.name	= "core",
		.dep_table = omap34xx_vdd_mpu_core_dep_data,
		.nr_dep_entries = ARRAY_SIZE(omap34xx_vdd_mpu_core_dep_data),
	},
	{.name = NULL, .dep_table = NULL, .nr_dep_entries = 0},
};

struct omap_vp_param omap34xx_core_vp_data = {
	.vddmin			= OMAP3430_VP2_VLIMITTO_VDDMIN,
	.vddmax			= OMAP3430_VP2_VLIMITTO_VDDMAX,
};

struct omap_vc_param omap34xx_core_vc_data = {
	.on		= OMAP3_ON_VOLTAGE_UV,
	.onlp		= OMAP3_ONLP_VOLTAGE_UV,
	.ret		= OMAP3_RET_VOLTAGE_UV,
	.off		= OMAP3_OFF_VOLTAGE_UV,
};

/* 36xx */

/* VDD1 */

#define OMAP3630_VDD_MPU_OPP50_UV		1012500
#define OMAP3630_VDD_MPU_OPP100_UV		1200000
#define OMAP3630_VDD_MPU_OPP120_UV		1325000
#define OMAP3630_VDD_MPU_OPP1G_UV		1375000

struct omap_volt_data omap36xx_vddmpu_volt_data[] = {
	VOLT_DATA_DEFINE(OMAP3630_VDD_MPU_OPP50_UV, 0, OMAP3630_CONTROL_FUSE_OPP50_VDD1, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP3630_VDD_MPU_OPP100_UV, 0, OMAP3630_CONTROL_FUSE_OPP100_VDD1, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP3630_VDD_MPU_OPP120_UV, 0, OMAP3630_CONTROL_FUSE_OPP120_VDD1, 0xfa, 0x23, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP3630_VDD_MPU_OPP1G_UV, 0, OMAP3630_CONTROL_FUSE_OPP1G_VDD1, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0),
};

struct omap_vp_param omap36xx_mpu_vp_data = {
	.vddmin			= OMAP3630_VP1_VLIMITTO_VDDMIN,
	.vddmax			= OMAP3630_VP1_VLIMITTO_VDDMAX,
};

struct omap_vc_param omap36xx_mpu_vc_data = {
	.on		= OMAP3_ON_VOLTAGE_UV,
	.onlp		= OMAP3_ONLP_VOLTAGE_UV,
	.ret		= OMAP3_RET_VOLTAGE_UV,
	.off		= OMAP3_OFF_VOLTAGE_UV,
};

/* VDD2 */

#define OMAP3630_VDD_CORE_OPP50_UV		1000000
#define OMAP3630_VDD_CORE_OPP100_UV		1200000

struct omap_volt_data omap36xx_vddcore_volt_data[] = {
	VOLT_DATA_DEFINE(OMAP3630_VDD_CORE_OPP50_UV, 0, OMAP3630_CONTROL_FUSE_OPP50_VDD2, 0xf4, 0x0c, OMAP_ABB_NO_LDO),
	VOLT_DATA_DEFINE(OMAP3630_VDD_CORE_OPP100_UV, 0, OMAP3630_CONTROL_FUSE_OPP100_VDD2, 0xf9, 0x16, OMAP_ABB_NO_LDO),
	VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0),
};

struct omap_vp_param omap36xx_core_vp_data = {
	.vddmin			= OMAP3630_VP2_VLIMITTO_VDDMIN,
	.vddmax			= OMAP3630_VP2_VLIMITTO_VDDMAX,
};

struct omap_vc_param omap36xx_core_vc_data = {
	.on		= OMAP3_ON_VOLTAGE_UV,
	.onlp		= OMAP3_ONLP_VOLTAGE_UV,
	.ret		= OMAP3_RET_VOLTAGE_UV,
	.off		= OMAP3_OFF_VOLTAGE_UV,
};

/* OPP data */

static struct omap_opp_def __initdata omap34xx_opp_def_list[] = {
	/* MPU OPP1 */
	OPP_INITIALIZER("mpu", "dpll1_ck", "mpu_iva", true, 125000000, OMAP3430_VDD_MPU_OPP1_UV),
	/* MPU OPP2 */
	OPP_INITIALIZER("mpu", "dpll1_ck", "mpu_iva", true, 250000000, OMAP3430_VDD_MPU_OPP2_UV),
	/* MPU OPP3 */
	OPP_INITIALIZER("mpu", "dpll1_ck", "mpu_iva", true, 500000000, OMAP3430_VDD_MPU_OPP3_UV),
	/* MPU OPP4 */
	OPP_INITIALIZER("mpu", "dpll1_ck", "mpu_iva", true, 550000000, OMAP3430_VDD_MPU_OPP4_UV),
	/* MPU OPP5 */
	OPP_INITIALIZER("mpu", "dpll1_ck", "mpu_iva", true, 600000000, OMAP3430_VDD_MPU_OPP5_UV),
	/* MPU OPP6 : omap3530 high speed grade only */
	OPP_INITIALIZER("mpu", "dpll1_ck", "mpu_iva", false, 720000000, OMAP3430_VDD_MPU_OPP5_UV),

	/*
	 * L3 OPP1 - 41.5 MHz is disabled because: The voltage for that OPP is
	 * almost the same than the one at 83MHz thus providing very little
	 * gain for the power point of view. In term of energy it will even
	 * increase the consumption due to the very negative performance
	 * impact that frequency will do to the MPU and the whole system in
	 * general.
	 */
	OPP_INITIALIZER("l3_main", "dpll3_ck", "core", false, 41500000, OMAP3430_VDD_CORE_OPP1_UV),
	/* L3 OPP2 */
	OPP_INITIALIZER("l3_main", "dpll3_ck", "core", true, 83000000, OMAP3430_VDD_CORE_OPP2_UV),
	/* L3 OPP3 */
	OPP_INITIALIZER("l3_main", "dpll3_ck", "core", true, 166000000, OMAP3430_VDD_CORE_OPP3_UV),

	/* DSP OPP1 */
	OPP_INITIALIZER("iva", "dpll2_ck", "mpu_iva", true, 90000000, OMAP3430_VDD_MPU_OPP1_UV),
	/* DSP OPP2 */
	OPP_INITIALIZER("iva", "dpll2_ck", "mpu_iva", true, 180000000, OMAP3430_VDD_MPU_OPP2_UV),
	/* DSP OPP3 */
	OPP_INITIALIZER("iva", "dpll2_ck", "mpu_iva", true, 360000000, OMAP3430_VDD_MPU_OPP3_UV),
	/* DSP OPP4 */
	OPP_INITIALIZER("iva", "dpll2_ck", "mpu_iva", true, 400000000, OMAP3430_VDD_MPU_OPP4_UV),
	/* DSP OPP5 */
	OPP_INITIALIZER("iva", "dpll2_ck", "mpu_iva", true, 430000000, OMAP3430_VDD_MPU_OPP5_UV),
	/* DSP OPP6 : omap3530 high speed grade only */
	OPP_INITIALIZER("iva", "dpll2_ck", "mpu_iva", false, 520000000, OMAP3430_VDD_MPU_OPP5_UV),
};

static struct omap_opp_def __initdata omap36xx_opp_def_list[] = {
	/* MPU OPP1 - OPP50 */
	OPP_INITIALIZER("mpu", "dpll1_ck", "mpu_iva", true,  300000000, OMAP3630_VDD_MPU_OPP50_UV),
	/* MPU OPP2 - OPP100 */
	OPP_INITIALIZER("mpu", "dpll1_ck", "mpu_iva", true,  600000000, OMAP3630_VDD_MPU_OPP100_UV),
	/* MPU OPP3 - OPP-Turbo */
	OPP_INITIALIZER("mpu", "dpll1_ck", "mpu_iva", false, 800000000, OMAP3630_VDD_MPU_OPP120_UV),
	/* MPU OPP4 - OPP-SB */
	OPP_INITIALIZER("mpu", "dpll1_ck", "mpu_iva", false, 1000000000, OMAP3630_VDD_MPU_OPP1G_UV),

	/* L3 OPP1 - OPP50 */
	OPP_INITIALIZER("l3_main", "dpll3_ck", "core", true, 100000000, OMAP3630_VDD_CORE_OPP50_UV),
	/* L3 OPP2 - OPP100, OPP-Turbo, OPP-SB */
	OPP_INITIALIZER("l3_main", "dpll3_ck", "core", true, 200000000, OMAP3630_VDD_CORE_OPP100_UV),

	/* DSP OPP1 - OPP50 */
	OPP_INITIALIZER("iva", "dpll2_ck", "mpu_iva", true,  260000000, OMAP3630_VDD_MPU_OPP50_UV),
	/* DSP OPP2 - OPP100 */
	OPP_INITIALIZER("iva", "dpll2_ck", "mpu_iva", true,  520000000, OMAP3630_VDD_MPU_OPP100_UV),
	/* DSP OPP3 - OPP-Turbo */
	OPP_INITIALIZER("iva", "dpll2_ck", "mpu_iva", false, 660000000, OMAP3630_VDD_MPU_OPP120_UV),
	/* DSP OPP4 - OPP-SB */
	OPP_INITIALIZER("iva", "dpll2_ck", "mpu_iva", false, 800000000, OMAP3630_VDD_MPU_OPP1G_UV),
};

/* OMAP 3630 MPU Core VDD dependency table */
static struct omap_vdd_dep_volt omap36xx_vdd_mpu_core_dep_data[] = {
	{.main_vdd_volt = OMAP3630_VDD_MPU_OPP50_UV, .dep_vdd_volt = OMAP3630_VDD_CORE_OPP50_UV},
	{.main_vdd_volt = OMAP3630_VDD_MPU_OPP100_UV, .dep_vdd_volt = OMAP3630_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP3630_VDD_MPU_OPP120_UV, .dep_vdd_volt = OMAP3630_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP3630_VDD_MPU_OPP1G_UV, .dep_vdd_volt = OMAP3630_VDD_CORE_OPP100_UV},
};

struct omap_vdd_dep_info omap36xx_vddmpu_dep_info[] = {
	{
		.name	= "core",
		.dep_table = omap36xx_vdd_mpu_core_dep_data,
		.nr_dep_entries = ARRAY_SIZE(omap36xx_vdd_mpu_core_dep_data),
	},
	{.name = NULL, .dep_table = NULL, .nr_dep_entries = 0},
};

/**
 * omap3_opp_init() - initialize omap3 opp table
 */
int __init omap3_opp_init(void)
{
	int r = -ENODEV;

	if (!cpu_is_omap34xx())
		return r;

	if (cpu_is_omap3630())
		r = omap_init_opp_table(omap36xx_opp_def_list,
			ARRAY_SIZE(omap36xx_opp_def_list));
	else
		r = omap_init_opp_table(omap34xx_opp_def_list,
			ARRAY_SIZE(omap34xx_opp_def_list));

	return r;
}
device_initcall(omap3_opp_init);
