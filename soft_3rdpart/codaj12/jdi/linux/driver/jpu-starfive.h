// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#ifndef __JPU_STARFIVE_H__
#define __JPU_STARFIVE_H__

#include <linux/io.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/reset-controller.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/clk-provider.h>

#define JPU_ID_NUM 5

struct starfive_jpu_data {
	struct device *dev;

	struct clk *clk_jpu_id[JPU_ID_NUM];
	struct reset_control *rst_jpu_id[JPU_ID_NUM];

};

static struct starfive_jpu_data *sf_jpu = NULL;

const char jpu_data_id[JPU_ID_NUM][15] = {
	"jpcgc300_main",
	"vdecbrg_main",
	"jpeg_axi",
	"jpeg_cclk",
	"jpeg_apb",
};


void starfive_jpu_rst_status(void)
{
	int i;
	int rst_state;
	for (i = 0; i < JPU_ID_NUM; i++) {
		rst_state = reset_control_status(sf_jpu->rst_jpu_id[i]);
		dev_dbg(sf_jpu->dev, "status_rst%d:%d", i, rst_state);
	}

	return;
}

void starfive_jpu_clk_status(void)
{
	int i;
	int clk_state;
	for (i = 0; i < JPU_ID_NUM; i++) {
		clk_state = __clk_is_enabled(sf_jpu->clk_jpu_id[i]);
		dev_dbg(sf_jpu->dev, "status_clk%d:%d", i, clk_state);
	}

	return;
}

/*Do not assert the reset: jpeg_axi, jpcgc300_main, vdecbrg_main. */
void starfive_jpu_rst_exit(void)
{
    int i;
	int ret;
	for (i = 3; i < JPU_ID_NUM; i++) {
		ret = reset_control_assert(sf_jpu->rst_jpu_id[i]);
		if (ret) {
			dev_err(sf_jpu->dev, "JPU reset assert failed:\n");
			dev_err(sf_jpu->dev, jpu_data_id[i]);
		}
	}
	return;
}

/*Do not disable the clock: jpeg_axi, jpcgc300_main, vdecbrg_main. */
void starfive_jpu_clk_exit(void)
{
	int i;
	for (i = 2; i < JPU_ID_NUM; i++) {
		clk_disable_unprepare(sf_jpu->clk_jpu_id[i]);
	}
	return;
}

static int starfive_jpu_clk_init(void)
{
	int ret = 0;
	int i;
	for (i = 0; i < JPU_ID_NUM; i++) {
		ret = clk_prepare_enable(sf_jpu->clk_jpu_id[i]);
		if (ret) {
			dev_err(sf_jpu->dev, "JPU enable clock failed:\n");
			dev_err(sf_jpu->dev, jpu_data_id[i]);
			goto init_clk_failed;
		}
	}

	return 0;

init_clk_failed:
	for(; i > 2 ; i--) {
		clk_disable_unprepare(sf_jpu->clk_jpu_id[i-1]);
	}

	return ret;
}

static int starfive_jpu_get_clk(void)
{
	int ret = 0;
	int i;
	for ( i = 0; i < JPU_ID_NUM ; i++) {
		sf_jpu->clk_jpu_id[i] = devm_clk_get(sf_jpu->dev, jpu_data_id[i]);
		if (IS_ERR(sf_jpu->clk_jpu_id[i])) {
			dev_err(sf_jpu->dev, "JPU get clock failed: \n");
			dev_err(sf_jpu->dev,  jpu_data_id[i]);
			ret = PTR_ERR(sf_jpu->clk_jpu_id[i]);
			goto get_clk_failed;
		}
	}

	return 0;

get_clk_failed:
	for( ; i > 0 ; i--) {
		devm_clk_put(sf_jpu->dev, sf_jpu->clk_jpu_id[i-1]);
	}

	return ret;
}

static int starfive_jpu_reset_init(void)
{
	int ret = 0;
	int i;
	for (i = 2; i < JPU_ID_NUM ; i++) {
		ret = reset_control_deassert(sf_jpu->rst_jpu_id[i]);
		if (ret) {
			dev_err(sf_jpu->dev, "JPU reset deassert failed: \n");
			dev_err(sf_jpu->dev, jpu_data_id[i]);
			goto init_reset_failed;
		}
	}

	return 0;

init_reset_failed:
	for (; i > 3 ; i--) {
		reset_control_assert(sf_jpu->rst_jpu_id[i-1]);
	}

	return ret;
}

/*Do not manipulate the reset: jpcgc300_main, vdecbrg_main. */
static int starfive_jpu_get_resets(void)
{
	int ret = 0;
	int i;
	for (i = 2; i < JPU_ID_NUM ; i++) {
		sf_jpu->rst_jpu_id[i] = devm_reset_control_get_exclusive(sf_jpu->dev, jpu_data_id[i]);
		if (IS_ERR(sf_jpu->rst_jpu_id[i])) {
			dev_err(sf_jpu->dev, "failed to get JPU reset: \n");
			dev_err(sf_jpu->dev,  jpu_data_id[i]);
			ret = PTR_ERR(sf_jpu->rst_jpu_id[i]);
			goto get_resets_failed;
		}
	}

	return 0;

get_resets_failed:
	for (; i > 0; i--) {
		reset_control_put(sf_jpu->rst_jpu_id[i-1]);
	}

	return ret;
}

int starfive_jpu_data_init(struct device *dev)
{
	int ret = 0;

	if (sf_jpu == NULL){
		sf_jpu = devm_kzalloc(dev, sizeof(*sf_jpu), GFP_KERNEL);
		if (!sf_jpu)
			return -ENOMEM;
		sf_jpu->dev = dev;

		ret = starfive_jpu_get_clk();
		if (ret) {
			dev_err(sf_jpu->dev, "failed to get jpu clock\n");
			return ret;
		}

		ret = starfive_jpu_get_resets();
		if (ret) {
			dev_err(sf_jpu->dev, "failed to get jpu resets\n");
			return ret;
		}
	}

	return 0;
}

int starfive_jpu_clk_enable(struct device *dev)
{
    int ret = 0;

	ret = starfive_jpu_data_init(dev);
	if (ret)
		return ret;

	ret = starfive_jpu_clk_init();
	if (ret) {
		dev_err(sf_jpu->dev, "failed to enable jpu clock\n");
		return ret;
	}

	return 0;
}

int starfive_jpu_clk_disable(struct device *dev)
{
    int ret = 0;

	ret = starfive_jpu_data_init(dev);
	if (ret)
		return ret;

	starfive_jpu_clk_exit();

	return 0;
}

int starfive_jpu_rst_deassert(struct device *dev)
{
    int ret = 0;

	ret = starfive_jpu_data_init(dev);
	if (ret)
		return ret;

	ret = starfive_jpu_reset_init();
	if (ret) {
		dev_err(sf_jpu->dev, "failed to deassert jpu resets\n");
		return ret;
	}

	return 0;
}

int starfive_jpu_rst_assert(struct device *dev)
{
    int ret = 0;

	ret = starfive_jpu_data_init(dev);
	if (ret)
		return ret;

	starfive_jpu_rst_exit();

	return 0;
}


int starfive_jpu_clk_rst_init(struct platform_device *pdev)
{
    int ret = 0;

	ret = starfive_jpu_data_init(&pdev->dev);
	if (ret)
		return ret;

	ret = starfive_jpu_clk_init();
	if (ret) {
		dev_err(sf_jpu->dev, "failed to enable jpu clock\n");
		return ret;
	}

	starfive_jpu_rst_exit();
	mdelay(1);
	ret = starfive_jpu_reset_init();
	if (ret) {
		dev_err(sf_jpu->dev, "failed to set jpu reset\n");
		goto init_failed;
	}

	dev_info(sf_jpu->dev,"success to init JPU clock & reset.");
    return 0;

init_failed:
    starfive_jpu_clk_exit();
    return ret;

}

#endif

