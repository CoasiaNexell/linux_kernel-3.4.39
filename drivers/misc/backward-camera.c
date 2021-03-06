#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/i2c.h>

#include <mach/platform.h>
#include <mach/nxp4330.h>
#include <mach/soc.h>
#include <mach/nxp-backward-camera.h>

#ifndef MLC_LAYER_RGB_OVERLAY
#define MLC_LAYER_RGB_OVERLAY 0
#endif

static struct nxp_backward_camera_context {
    struct nxp_backward_camera_platform_data *plat_data;
    int irq;
    bool running;
    bool backgear_on;
    struct work_struct work;
#ifdef CONFIG_PM
    struct i2c_client *client;
    struct delayed_work resume_work;
#endif
} _context;

static void vip_run(int module)
{
    NX_VIP_SetVIPEnable(module, CTRUE, CTRUE, CTRUE, CFALSE);
}

static void vip_stop(int module)
{
    NX_VIP_SetVIPEnable(module, CFALSE, CFALSE, CFALSE, CFALSE);
}

static void mlc_run(int module)
{
    NX_MLC_SetTopDirtyFlag(module);
    NX_MLC_SetLayerEnable(module, MLC_LAYER_VIDEO, CTRUE);
    NX_MLC_SetDirtyFlag(module, MLC_LAYER_VIDEO);
}

static void mlc_stop(int module)
{
    NX_MLC_SetTopDirtyFlag(module);
    NX_MLC_SetLayerEnable(module, MLC_LAYER_VIDEO, CFALSE);
    NX_MLC_SetDirtyFlag(module, MLC_LAYER_VIDEO);
}

static void mlc_overlay_run(int module)
{
    u32 layer = MLC_LAYER_RGB_OVERLAY;
    NX_MLC_SetLayerEnable(module, layer, CTRUE);
    NX_MLC_SetDirtyFlag(module, layer);
}

static void mlc_overlay_stop(int module)
{
    u32 layer = MLC_LAYER_RGB_OVERLAY;
    NX_MLC_SetLayerEnable(module, layer, CFALSE);
    NX_MLC_SetDirtyFlag(module, layer);
}

static void _turn_on(struct nxp_backward_camera_context *me)
{
    printk("%s\n", __func__);
    vip_run(me->plat_data->vip_module_num);
    mlc_overlay_run(me->plat_data->mlc_module_num);
    mlc_run(me->plat_data->mlc_module_num);
}

static void _turn_off(struct nxp_backward_camera_context *me)
{
    printk("%s\n", __func__);
    mlc_overlay_stop(me->plat_data->mlc_module_num);
    mlc_stop(me->plat_data->mlc_module_num);
    vip_stop(me->plat_data->vip_module_num);
}

static inline bool _is_backgear_on(struct nxp_backward_camera_platform_data *pdata)
{
    bool is_on = nxp_soc_gpio_get_in_value(pdata->backgear_gpio_num);
    printk("%s: is_on %d\n", __func__, is_on);
    if (!pdata->active_high)
        is_on ^= 1;
    return is_on;
}

static inline bool _is_running(struct nxp_backward_camera_context *me)
{
    CBOOL vipenb, sepenb, clipenb, decenb;
    bool mlcenb;

    NX_VIP_GetVIPEnable(me->plat_data->vip_module_num, &vipenb, &sepenb, &clipenb, &decenb);
    mlcenb = NX_MLC_GetLayerEnable(me->plat_data->mlc_module_num, 3);

    return mlcenb && vipenb && sepenb && clipenb;
}

static void _decide(struct nxp_backward_camera_context *me)
{
    /*me->running = NX_MLC_GetLayerEnable(me->plat_data->mlc_module_num, 3); // video layer*/
    me->running = _is_running(me);
    me->backgear_on = _is_backgear_on(me->plat_data);
    printk("%s: running %d, backgear on %d\n", __func__, me->running, me->backgear_on);
    if (me->backgear_on && !me->running)
        _turn_on(me);
    else if (me->running && !me->backgear_on)
        _turn_off(me);
}

static irqreturn_t _irq_handler(int irq, void *devdata)
{
    struct nxp_backward_camera_context *me = devdata;
    printk("%s\n", __func__);
    schedule_work(&me->work);
    return IRQ_HANDLED;
}

static void _work_handler(struct work_struct *work)
{
    printk("%s\n", __func__);
    _decide(&_context);
}

#ifdef CONFIG_PM
static void _resume_work(struct work_struct *work);
#endif

static int nxp_backward_camera_probe(struct platform_device *pdev)
{
    int ret;
    struct nxp_backward_camera_platform_data *pdata = pdev->dev.platform_data;
    struct nxp_backward_camera_context *me = &_context;

    printk("%s\n", __func__);
    me->plat_data = pdata;
    me->irq = IRQ_GPIO_START + pdata->backgear_gpio_num;

    NX_MLC_SetBaseAddress(pdata->mlc_module_num, IO_ADDRESS(NX_MLC_GetPhysicalAddress(pdata->mlc_module_num)));
    NX_VIP_SetBaseAddress(pdata->vip_module_num, IO_ADDRESS(NX_VIP_GetPhysicalAddress(pdata->vip_module_num)));

    INIT_WORK(&me->work, _work_handler);
    ret = request_irq(me->irq, _irq_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "backward-camera", me);
    if (ret) {
        pr_err("%s: failed to request_irq (irqnum %d)\n", __func__, me->irq);
        return -1;
    }

    _decide(me);
    /*INIT_WORK(&me->work, _work_handler);*/

#ifdef CONFIG_PM
    INIT_DELAYED_WORK(&me->resume_work, _resume_work);
#endif

    return 0;
}

static int nxp_backward_camera_remove(struct platform_device *pdev)
{
    printk("%s\n", __func__);
    free_irq(_context.irq, &_context);
    return 0;
}

#ifdef CONFIG_PM

#define RESUME_CAMERA_ON_DELAY_MS   300

static int _get_i2c_client(struct nxp_backward_camera_context *me)
{
    struct i2c_client *client;
    struct i2c_adapter *adapter = i2c_get_adapter(me->plat_data->i2c_bus);
    if (!adapter) {
        pr_err("%s: unable to get i2c adapter %d\n", __func__, me->plat_data->i2c_bus);
        return -EINVAL;
    }

    client = kzalloc(sizeof *client, GFP_KERNEL);
    if (!client) {
        pr_err("%s: can't allocate i2c_client\n", __func__);
        return -ENOMEM;
    }

    client->adapter = adapter;
    client->addr = me->plat_data->chip_addr;
    client->flags = 0;

    me->client = client;
    return 0;
}

static int _camera_sensor_run(struct nxp_backward_camera_context *me)
{
    struct reg_val *reg_val;

    if (me->plat_data->power_enable)
        me->plat_data->power_enable(true);

    if (me->plat_data->setup_io)
        me->plat_data->setup_io();

    if (me->plat_data->set_clock)
        me->plat_data->set_clock(me->plat_data->clk_rate);

    reg_val = me->plat_data->reg_val;
    while (reg_val->reg != 0xff) {
        i2c_smbus_write_byte_data(me->client, reg_val->reg, reg_val->val);
        reg_val++;
    }

    return 0;
}

static void _vip_hw_set_clock(int module, struct nxp_backward_camera_platform_data *param, bool on)
{
    if (on) {
        volatile u32 *clkgen_base = (volatile u32 *)IO_ADDRESS(NX_CLKGEN_GetPhysicalAddress(NX_VIP_GetClockNumber(module)));
        NX_CLKGEN_SetBaseAddress(NX_VIP_GetClockNumber(module), (U32)clkgen_base);
        NX_CLKGEN_SetClockDivisorEnable(NX_VIP_GetClockNumber(module), CTRUE);
        NX_CLKGEN_SetClockBClkMode(NX_VIP_GetClockNumber(module), NX_BCLKMODE_DYNAMIC);
        /* printk("RSTCON Base: 0x%x\n", NX_RSTCON_GetBaseAddress()); */
        /* printk("RSTCON VIP %d Reset Number %d, REG Number %d\n", module, NX_VIP_GetResetNumber(module), NX_VIP_GetResetNumber(module)>>5); */
        /* printk("CLKGEN Base: 0x%x\n", clkgen_base); */
        NX_RSTCON_SetnRST(NX_VIP_GetResetNumber(module), RSTCON_nDISABLE);
        NX_RSTCON_SetnRST(NX_VIP_GetResetNumber(module), RSTCON_nENABLE);

        if (param->is_mipi) {
            printk("%s: apply mipi csi clock!!!\n", __func__);
            NX_CLKGEN_SetClockSource(NX_VIP_GetClockNumber(module), 0, 2); /* external PCLK */
            NX_CLKGEN_SetClockDivisor(NX_VIP_GetClockNumber(module), 0, 2);
            NX_CLKGEN_SetClockDivisorEnable(NX_VIP_GetClockNumber(module), CTRUE);
        } else {
            NX_CLKGEN_SetClockSource(NX_VIP_GetClockNumber(module), 0, 4 + param->port); /* external PCLK */
            NX_CLKGEN_SetClockDivisor(NX_VIP_GetClockNumber(module), 0, 1);
            NX_CLKGEN_SetClockDivisorEnable(NX_VIP_GetClockNumber(module), CTRUE);
        }

        printk("VIP CLK GEN VAL: 0x%x\n", *clkgen_base);
        NX_VIP_SetBaseAddress(module, IO_ADDRESS(NX_VIP_GetPhysicalAddress(module)));
    }
}

static void _vip_hw_set_sensor_param(int module, struct nxp_backward_camera_platform_data *param)
{
    if (param->is_mipi) {
        NX_VIP_SetInputPort(module, NX_VIP_INPUTPORT_B);
        NX_VIP_SetDataMode(module, NX_VIP_DATAORDER_CBY0CRY1, 16);
        NX_VIP_SetFieldMode(module, CFALSE, NX_VIP_FIELDSEL_BYPASS, CFALSE, CFALSE);
        NX_VIP_SetDValidMode(module, CTRUE, CTRUE, CTRUE);
        NX_VIP_SetFIFOResetMode(module, NX_VIP_FIFORESET_ALL);

        NX_VIP_SetHVSyncForMIPI(module,
                param->h_active * 2,
                param->v_active,
                param->h_syncwidth,
                param->h_frontporch,
                param->h_backporch,
                param->v_syncwidth,
                param->v_frontporch,
                param->v_backporch);
    } else {
        NX_VIP_SetDataMode(module, param->data_order, 8);
        NX_VIP_SetFieldMode(module,
                CFALSE,
                0,
                param->interlace,
                CFALSE);
        NX_VIP_SetDValidMode(module,
                CFALSE,
                CFALSE,
                CFALSE);
        NX_VIP_SetFIFOResetMode(module, NX_VIP_FIFORESET_ALL);
        NX_VIP_SetInputPort(module, (NX_VIP_INPUTPORT)param->port);

        NX_VIP_SetHVSync(module,
                param->external_sync,
                param->h_active*2,
                param->interlace ? param->v_active >> 1 : param->v_active,
                param->h_syncwidth,
                param->h_frontporch,
                param->h_backporch,
                param->v_syncwidth,
                param->v_frontporch,
                param->v_backporch);
    }

    NX_VIP_SetClipperFormat(module, NX_VIP_FORMAT_420, 0, 0, 0);
    NX_VIP_SetClipRegion(module,
            0,
            0,
            param->h_active,
            param->interlace ? param->v_active >> 1 : param->v_active);
}

static void _vip_hw_set_addr(int module, struct nxp_backward_camera_platform_data *param, u32 lu_addr, u32 cb_addr, u32 cr_addr)
{
    NX_VIP_SetClipperAddr(module, NX_VIP_FORMAT_420, param->h_active, param->v_active,
            lu_addr, cb_addr, cr_addr,
            param->interlace ? ALIGN(param->h_active, 64)   : param->h_active,
            param->interlace ? ALIGN(param->h_active/2, 64) : param->h_active/2);
}

/*static void _vip_hw_run(int module)*/
/*{*/
    /*NX_VIP_SetVIPEnable(module, CTRUE, CTRUE, CTRUE, CFALSE);*/
/*}*/

#ifndef MLC_LAYER_VIDEO
#define MLC_LAYER_VIDEO     3
#endif

static void _mlc_video_set_param(int module, struct nxp_backward_camera_platform_data *param)
{
    int srcw = param->h_active;
    int srch = param->v_active;
    int dstw, dsth, hf, vf;

    NX_MLC_GetScreenSize(module, &dstw, &dsth);

    hf = 1, vf = 1;

    if (srcw == dstw && srch == dsth)
        hf = 0, vf = 0;

    NX_MLC_SetFormatYUV(module, NX_MLC_YUVFMT_420);
    NX_MLC_SetVideoLayerScale(module, srcw, srch, dstw, dsth,
            (CBOOL)hf, (CBOOL)hf, (CBOOL)vf, (CBOOL)vf);
    NX_MLC_SetPosition(module, MLC_LAYER_VIDEO,
            0, 0, dstw - 1, dsth - 1);
    /*NX_MLC_SetLayerPriority(module, 0);*/
    NX_MLC_SetLayerPriority(module, 1);
    NX_MLC_SetDirtyFlag(module, MLC_LAYER_VIDEO);
    printk("%s exit\n", __func__);
}

static void _mlc_video_set_addr(int module, u32 lu_a, u32 cb_a, u32 cr_a, u32 lu_s, u32 cb_s, u32 cr_s)
{
    NX_MLC_SetVideoLayerStride (module, lu_s, cb_s, cr_s);
    NX_MLC_SetVideoLayerAddress(module, lu_a, cb_a, cr_a);
    NX_MLC_SetVideoLayerLineBufferPowerMode(module, CTRUE);
    NX_MLC_SetVideoLayerLineBufferSleepMode(module, CFALSE);
    NX_MLC_SetDirtyFlag(module, MLC_LAYER_VIDEO);
    printk("%s exit\n", __func__);
}

static inline u32 _get_pixel_byte(u32 nxp_rgb_format)
{
    switch (nxp_rgb_format) {
        case NX_MLC_RGBFMT_R5G6B5:
        case NX_MLC_RGBFMT_B5G6R5:
        case NX_MLC_RGBFMT_X1R5G5B5:
        case NX_MLC_RGBFMT_X1B5G5R5:
        case NX_MLC_RGBFMT_X4R4G4B4:
        case NX_MLC_RGBFMT_X4B4G4R4:
        case NX_MLC_RGBFMT_X8R3G3B2:
        case NX_MLC_RGBFMT_X8B3G3R2:
        case NX_MLC_RGBFMT_A1R5G5B5:
        case NX_MLC_RGBFMT_A1B5G5R5:
        case NX_MLC_RGBFMT_A4R4G4B4:
        case NX_MLC_RGBFMT_A4B4G4R4:
        case NX_MLC_RGBFMT_A8R3G3B2:
        case NX_MLC_RGBFMT_A8B3G3R2:
            return 2;

        case NX_MLC_RGBFMT_R8G8B8:
        case NX_MLC_RGBFMT_B8G8R8:
            return 3;

        case NX_MLC_RGBFMT_A8R8G8B8:
        case NX_MLC_RGBFMT_A8B8G8R8:
            return 4;

        default:
            pr_err("%s: invalid nxp_rgb_format(0x%x)\n", __func__, nxp_rgb_format);
            return 0;
    }
}

static void _mlc_rgb_overlay_set_param(int module, struct nxp_backward_camera_platform_data *param)
{
    u32 format = param->rgb_format;
    u32 pixelbyte = _get_pixel_byte(format);
    u32 stride = param->width * pixelbyte;
    u32 layer = MLC_LAYER_RGB_OVERLAY;
    CBOOL EnAlpha = CFALSE;

    if (format == MLC_RGBFMT_A1R5G5B5 ||
        format == MLC_RGBFMT_A1B5G5R5 ||
        format == MLC_RGBFMT_A4R4G4B4 ||
        format == MLC_RGBFMT_A4B4G4R4 ||
        format == MLC_RGBFMT_A8R3G3B2 ||
        format == MLC_RGBFMT_A8B3G3R2 ||
        format == MLC_RGBFMT_A8R8G8B8 ||
        format == MLC_RGBFMT_A8B8G8R8)
        EnAlpha = CTRUE;

    NX_MLC_SetColorInversion(module, layer, CFALSE, 0);
    NX_MLC_SetAlphaBlending(module, layer, EnAlpha, 0);
    NX_MLC_SetFormatRGB(module, layer, (NX_MLC_RGBFMT)format);
    NX_MLC_SetRGBLayerInvalidPosition(module, layer, 0, 0, 0, 0, 0, CFALSE);
    NX_MLC_SetRGBLayerInvalidPosition(module, layer, 1, 0, 0, 0, 0, CFALSE);
    NX_MLC_SetPosition(module, layer, 0, 0, param->width-1, param->height-1);

    NX_MLC_SetRGBLayerStride (module, layer, pixelbyte, stride);
    NX_MLC_SetRGBLayerAddress(module, layer, param->rgb_addr);
    NX_MLC_SetDirtyFlag(module, layer);
}

static void _mlc_rgb_overlay_draw(int module, struct nxp_backward_camera_platform_data *param)
{
    if (param->draw_rgb_overlay)
        param->draw_rgb_overlay(param);
}

static void _resume_work(struct work_struct *work)
{
    struct nxp_backward_camera_context *me = &_context;
    int vip_module_num = me->plat_data->vip_module_num;
    int mlc_module_num = me->plat_data->mlc_module_num;
    u32 lu_addr = me->plat_data->lu_addr;
    u32 cb_addr = me->plat_data->cb_addr;
    u32 cr_addr = me->plat_data->cr_addr;
    u32 lu_stride = me->plat_data->lu_stride;
    u32 cb_stride = me->plat_data->cb_stride;
    u32 cr_stride = me->plat_data->cr_stride;

    _camera_sensor_run(me);
    _vip_hw_set_clock(vip_module_num, me->plat_data, true);
    _vip_hw_set_sensor_param(vip_module_num, me->plat_data);
    _vip_hw_set_addr(vip_module_num, me->plat_data, lu_addr, cb_addr, cr_addr);

    _mlc_video_set_param(mlc_module_num, me->plat_data);
    _mlc_video_set_addr(mlc_module_num, lu_addr, cb_addr, cr_addr, lu_stride, cb_stride, cr_stride);

    _mlc_rgb_overlay_set_param(mlc_module_num, me->plat_data);
    _mlc_rgb_overlay_draw(mlc_module_num, me->plat_data);

    _decide(me);
    printk("%s: exit\n", __func__);
}

static int nxp_backward_camera_suspend(struct device *dev)
{
    struct nxp_backward_camera_context *me = &_context;
    PM_DBGOUT("+%s\n", __func__);
    me->running = false;
    PM_DBGOUT("-%s\n", __func__);
    return 0;
}

static int nxp_backward_camera_resume(struct device *dev)
{
    struct nxp_backward_camera_context *me = &_context;
    PM_DBGOUT("+%s\n", __func__);
    if (!me->client)
        _get_i2c_client(me);

    queue_delayed_work(system_nrt_wq, &me->resume_work, msecs_to_jiffies(RESUME_CAMERA_ON_DELAY_MS));

    PM_DBGOUT("-%s\n", __func__);
    return 0;
}
#endif

#ifdef CONFIG_PM
static const struct dev_pm_ops nxp_backward_camera_pm_ops = {
    SET_SYSTEM_SLEEP_PM_OPS(nxp_backward_camera_suspend, nxp_backward_camera_resume)
};
#define NXP_BACKWARD_CAMERA_PMOPS       (&nxp_backward_camera_pm_ops)
#else
#define NXP_BACKWARD_CAMERA_PMOPS       NULL
#endif

static struct platform_driver backward_camera_driver = {
    .probe  = nxp_backward_camera_probe,
    .remove = nxp_backward_camera_remove,
    .driver = {
        .name  = "nxp-backward-camera",
        .owner = THIS_MODULE,
        .pm    = NXP_BACKWARD_CAMERA_PMOPS,
    },
};

static int __init backward_camera_init(void)
{
    printk("%s\n", __func__);
    return platform_driver_register(&backward_camera_driver);
}

subsys_initcall(backward_camera_init);

MODULE_AUTHOR("swpark <swpark@nexell.co.kr>");
MODULE_DESCRIPTION("Backward Camera Driver for Nexell");
MODULE_LICENSE("GPL");
