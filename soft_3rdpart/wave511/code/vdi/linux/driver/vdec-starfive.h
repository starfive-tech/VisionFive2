// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#ifndef __VDEC_STARFIVE_H__
#define __VDEC_STARFIVE_H__

#include <linux/io.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/reset-controller.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/clk-provider.h>

#define VDEC_ID_NUM 5

struct starfive_vdec_data {
	struct device *dev;

	struct clk *clk_vdec_id[VDEC_ID_NUM];
	struct reset_control *rst_vdec_id[VDEC_ID_NUM];

};

static struct starfive_vdec_data *sf_vdec = NULL;

const char vdec_data_id[VDEC_ID_NUM][15] = {
	"vdecbrg_main",
	"vdec_axi",
	"vdec_bclk",
	"vdec_cclk",
	"vdec_apb",
};

void starfive_vdec_rst_status(void)
{
	int i;
	int rst_state;
	for (i = 0; i < VDEC_ID_NUM; i++) {		
		rst_state = reset_control_status(sf_vdec->rst_vdec_id[i]);
		dev_dbg(sf_vdec->dev, "status_rst%d:%d", i, rst_state);
	}
	return;
}

void starfive_vdec_clk_status(void)
{
	int i;
	int clk_state;
	for (i = 0; i < VDEC_ID_NUM; i++) {		
		clk_state = __clk_is_enabled(sf_vdec->clk_vdec_id[i]);
		dev_dbg(sf_vdec->dev, "status_clk%d:%d", i, clk_state);
	}
	return;
}

void starfive_vdec_rst_exit(void)
{
	int i;
	int ret;
	for (i = 1; i < VDEC_ID_NUM; i++) {
		/* Assert the reset of "vdecbrg_main" could crash*/
		ret = reset_control_assert(sf_vdec->rst_vdec_id[i]);
		if (ret) {
			dev_err(sf_vdec->dev, "VDEC reset assert failed:\n");
			dev_err(sf_vdec->dev, vdec_data_id[i]);
		}		
	}
	return;
}

void starfive_vdec_clk_exit(void)
{
	int i;
	for (i = 1; i < VDEC_ID_NUM; i++) {
		clk_disable_unprepare(sf_vdec->clk_vdec_id[i]);
	}
	return;
}

static int starfive_vdec_clk_init(void)
{
	int ret = 0;
	int i;
	for (i = 0; i < VDEC_ID_NUM; i++) {
		ret = clk_prepare_enable(sf_vdec->clk_vdec_id[i]);
		if (ret) {
			dev_err(sf_vdec->dev, "VDEC enable clock failed:\n");
			dev_err(sf_vdec->dev, vdec_data_id[i]);
			goto init_clk_failed;
		}
	}

	return 0;

init_clk_failed:
	for(; i > 1 ; i--) {
		clk_disable_unprepare(sf_vdec->clk_vdec_id[i-1]);
	}

	return ret;

}

static int starfive_vdec_get_clk(void)
{
	int ret = 0;
	int i;
	for ( i = 0; i < VDEC_ID_NUM ; i++) {
		sf_vdec->clk_vdec_id[i] = devm_clk_get(sf_vdec->dev, vdec_data_id[i]);
		if (IS_ERR(sf_vdec->clk_vdec_id[i])) {
			dev_err(sf_vdec->dev, "VDEC get clock failed:\n");
			dev_err(sf_vdec->dev,  vdec_data_id[i]);
			ret = PTR_ERR(sf_vdec->clk_vdec_id[i]);
			goto get_clk_failed;
		}
	}

	return 0;

get_clk_failed:
	for( ; i > 0 ; i--) {
		devm_clk_put(sf_vdec->dev, sf_vdec->clk_vdec_id[i-1]);
	}

	return ret;
}

static int starfive_vdec_reset_init(void)
{
    int ret = 0;
	int i;
	for (i = 0; i < VDEC_ID_NUM ; i++) {
		ret = reset_control_deassert(sf_vdec->rst_vdec_id[i]);
    	if (ret) {
			dev_err(sf_vdec->dev, "VDEC deassert reset failed:\n");
			dev_err(sf_vdec->dev, vdec_data_id[i]);
       	 	goto init_reset_failed;
		}
	}

	return 0;

init_reset_failed:
	for (; i > 1 ; i--) {
		reset_control_assert(sf_vdec->rst_vdec_id[i-1]);
	}

	return ret;
}

static int starfive_vdec_get_resets(void)
{	
    int ret = 0;
	int i;
	for (i = 0; i < VDEC_ID_NUM ; i++) {
		sf_vdec->rst_vdec_id[i] = devm_reset_control_get_exclusive(sf_vdec->dev, vdec_data_id[i]);
		if (IS_ERR(sf_vdec->rst_vdec_id[i])) {
			dev_err(sf_vdec->dev, "VDEC get reset failed:\n");
			dev_err(sf_vdec->dev,  vdec_data_id[i]);
			ret = PTR_ERR(sf_vdec->rst_vdec_id[i]);
			goto get_resets_failed;
		}
	}

	return 0;

get_resets_failed:
	for (; i > 0; i--) {
		reset_control_put(sf_vdec->rst_vdec_id[i-1]);
	}

	return ret;
}


int starfive_vdec_data_init(struct device *dev)
{
	int ret = 0;

	if (sf_vdec == NULL){
		sf_vdec = devm_kzalloc(dev, sizeof(*sf_vdec), GFP_KERNEL);
		if (!sf_vdec)
			return -ENOMEM;
		sf_vdec->dev = dev;

		ret = starfive_vdec_get_clk();
		if (ret) {
			dev_err(sf_vdec->dev, "failed to get vdec clock\n");
			return ret;
		}

		ret = starfive_vdec_get_resets();
		if (ret) {
			dev_err(sf_vdec->dev, "failed to get vdec resets\n");
			return ret;
		}		
	}

	return 0;
}

int starfive_vdec_clk_enable(struct device *dev)
{
    int ret = 0;

	ret = starfive_vdec_data_init(dev);
	if (ret)
		return ret;

	ret = starfive_vdec_clk_init();
	if (ret) {
		dev_err(sf_vdec->dev, "failed to enable vdec clock\n");
		return ret;
	}

	return 0;
}

int starfive_vdec_clk_disable(struct device *dev)
{
    int ret = 0;

	ret = starfive_vdec_data_init(dev);
	if (ret)
		return ret;

	starfive_vdec_clk_exit();

	return 0;
	
}

int starfive_vdec_rst_deassert(struct device *dev)
{
    int ret = 0;

	ret = starfive_vdec_data_init(dev);
	if (ret)
		return ret;

	ret = starfive_vdec_reset_init();
	if (ret) {
		dev_err(sf_vdec->dev, "failed to deassert vdec resets\n");
		return ret;
	}

	return 0;
}

int starfive_vdec_rst_assert(struct device *dev)
{
    int ret = 0;
	ret = starfive_vdec_data_init(dev);
	if (ret)
		return ret;
    
	starfive_vdec_rst_exit();

	return 0;
	
}


int starfive_vdec_clk_rst_init(struct platform_device *pdev)
{
    int ret = 0;

	ret = starfive_vdec_data_init(&pdev->dev);
	if (ret)
		return ret;

	ret = starfive_vdec_clk_init();
	if (ret) {
		dev_err(sf_vdec->dev, "failed to enable vdec clock\n");
		return ret;
	} 

	starfive_vdec_rst_exit();
	mdelay(1);
	ret = starfive_vdec_reset_init();
	if (ret) {
		dev_err(sf_vdec->dev, "failed to set vdec reset\n");
		goto init_failed;
	}

	dev_info(sf_vdec->dev, "success to init VDEC clock & reset.");
    return 0;

init_failed:
    starfive_vdec_clk_exit();
    return ret;

}

#endif
