/*
 *  RT-Thread Wi-Fi Device
 *
 * COPYRIGHT (C) 2014 - 2018, Shanghai Real-Thread Technology Co., Ltd
 *
 *  This file is part of RT-Thread (http://www.rt-thread.org)
 *
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-03     RT-Thread    the first verion
 */
#include <rthw.h>
#include <rtthread.h>
#include <wlan_dev.h>
#include <wlan_prot.h>

#define DBG_ENABLE
#define DBG_LEVEL DBG_INFO
#define DBG_SECTION_NAME  "WLAN.dev"
#define DBG_COLOR
#include <rtdbg.h>

#ifndef RT_DEVICE
#define RT_DEVICE(__device) ((rt_device_t)__device)
#endif

#define WLAN_DEV_LOCK(_wlan)      (rt_mutex_take(&(_wlan)->lock, RT_WAITING_FOREVER))
#define WLAN_DEV_UNLOCK(_wlan)    (rt_mutex_release(&(_wlan)->lock))

rt_err_t rt_wlan_dev_init(struct rt_wlan_device *device, rt_wlan_mode_t mode)
{
    rt_err_t result = RT_EOK;

    /* init wlan device */
    LOG_D("F:%s L:%d is run device:0x%08x mode:%d", __FUNCTION__, __LINE__, device, mode);
    if ((device == RT_NULL) || (mode >= RT_WLAN_MODE_MAX))
    {
        LOG_E("F:%s L:%d Parameter Wrongful device:0x%08x mode:%d", __FUNCTION__, __LINE__, device, mode);
        return -RT_ERROR;
    }

    result = rt_device_init(RT_DEVICE(device));
    if (result != RT_EOK)
    {
        LOG_E("L:%d wlan init failed", __LINE__);
        return -RT_ERROR;
    }
    result = rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_MODE, (void *)&mode);
    if (result != RT_EOK)
    {
        LOG_E("L:%d wlan config mode failed", __LINE__);
        return -RT_ERROR;
    }
    device->mode = mode;
    return result;
}

rt_err_t rt_wlan_dev_connect(struct rt_wlan_device *device, struct rt_wlan_info *info, const char *password, int password_len)
{
    rt_err_t result = RT_EOK;
    struct rt_sta_info sta_info;

    if (device == RT_NULL) return -RT_EIO;
    if (info == RT_NULL) return -RT_ERROR;
    if ((password_len >= KEY_LENGTH_MAX_SIZE) || 
        (info->ssid.len >= SSID_LENGTH_MAX_SIZE))
    {
        LOG_E("L:%d password or ssid is to long", __LINE__);
        return -RT_ERROR;
    }
    rt_memset(&sta_info, 0, sizeof(struct rt_sta_info));
    rt_memcpy(&sta_info.ssid, &info->ssid, sizeof(rt_wlan_ssid_t));
    rt_memcpy(sta_info.bssid, info->bssid, BSSID_LENGTH_MAX_SIZE);
    if (password != RT_NULL)
        rt_memcpy(sta_info.key.val, password, password_len);
    sta_info.key.len = password_len;
    sta_info.channel = info->channel;
    sta_info.security = info->security;

    result = rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_JOIN, &sta_info);
    return result;
}

rt_err_t rt_wlan_dev_disconnect(struct rt_wlan_device *device)
{
    rt_err_t result = RT_EOK;

    if (device == RT_NULL) return -RT_EIO;
    result = rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_DISCONNECT, RT_NULL);
    return result;
}

rt_err_t rt_wlan_dev_ap_start(struct rt_wlan_device *device, struct rt_wlan_info *info, const char *password, int password_len)
{
    rt_err_t result = RT_EOK;
    struct rt_ap_info ap_info;

    if (device == RT_NULL) return -RT_EIO;
    if (info == RT_NULL) return -RT_ERROR;
    if ((password_len >= KEY_LENGTH_MAX_SIZE) || 
        (info->ssid.len >= SSID_LENGTH_MAX_SIZE))
    {
        LOG_E("L:%d password or ssid is to long", __LINE__);
        return -RT_ERROR;
    }

    rt_memset(&ap_info, 0, sizeof(struct rt_ap_info));
    rt_memcpy(&ap_info.ssid, &info->ssid, sizeof(rt_wlan_ssid_t));
    rt_memcpy(ap_info.key.val, password, password_len);
    ap_info.key.len = password_len;
    ap_info.hidden = info->hidden;
    ap_info.channel = info->channel;
    ap_info.security = info->security;

    result = rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_SOFTAP, &ap_info);
    return result;
}

rt_err_t rt_wlan_dev_ap_stop(struct rt_wlan_device *device)
{
    rt_err_t result = 0;

    if (device == RT_NULL) return -RT_EIO;
    result = rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_AP_STOP, RT_NULL);
    return result;
}

int rt_wlan_dev_ap_deauth(struct rt_wlan_device *device, rt_uint8_t mac[6])
{
    int result = 0;

    if (device == RT_NULL) return -RT_EIO;
    result = rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_AP_DEAUTH, mac);
    return result;
}

int rt_wlan_dev_get_rssi(struct rt_wlan_device *device)
{
    int rssi = 0;

    if (device == RT_NULL) return -RT_EIO;
    rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_GET_RSSI, &rssi);
    return rssi;
}

rt_err_t rt_wlan_dev_get_mac(struct rt_wlan_device *device, rt_uint8_t mac[6])
{
    rt_err_t result = 0;

    if (device == RT_NULL) return -RT_EIO;
    result = rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_GET_MAC, &mac[0]);
    return result;
}

rt_err_t rt_wlan_dev_set_mac(struct rt_wlan_device *device, rt_uint8_t mac[6])
{
    rt_err_t result = RT_EOK;

    if (device == RT_NULL) return -RT_EIO;
    result = rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_SET_MAC, &mac[0]);
    return result;
}

rt_err_t rt_wlan_dev_enable_powersave(struct rt_wlan_device *device)
{
    rt_err_t result = RT_EOK;
    int enable = 1;

    if (device == RT_NULL) return -RT_EIO;
    result = rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_POWERSAVE, &enable);
    return result;
}

rt_err_t rt_wlan_dev_disable_powersave(struct rt_wlan_device *device)
{
    rt_err_t result = RT_EOK;
    int enable = 0;

    if (device == RT_NULL) return -RT_EIO;
    result = rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_POWERSAVE, &enable);
    return result;
}

rt_err_t rt_wlan_dev_register_event_handler(struct rt_wlan_device *device, rt_wlan_dev_event_t event, rt_wlan_dev_event_handler handler, void *parameter)
{
    int i = 0;

    if (device == RT_NULL) return -RT_EIO;
    if (event >= RT_WLAN_DEV_EVT_MAX) return -RT_EINVAL;

    rt_enter_critical();
    for (i = 0; i < RT_WLAN_DEV_EVENT_NUM; i++)
    {
        if (device->handler_table[event][i].handler == RT_NULL)
        {
            device->handler_table[event][i].handler = handler;
            device->handler_table[event][i].parameter = parameter;
            rt_exit_critical();
            return RT_EOK;
        }
    }
    rt_exit_critical();

    /* No space found */
    return -RT_ERROR;
}

int rt_wlan_dev_unregister_event_handler(struct rt_wlan_device *device, rt_wlan_dev_event_t event, rt_wlan_dev_event_handler handler)
{
    int i = 0;
    if (device == RT_NULL) return -RT_EIO;
    if (event >= RT_WLAN_DEV_EVT_MAX) return -RT_EINVAL;

    rt_enter_critical();
    for (i = 0; i < RT_WLAN_DEV_EVENT_NUM; i++)
    {
        if (device->handler_table[event][i].handler == handler)
        {
            rt_memset(&device->handler_table[event][i], 0, sizeof(struct rt_wlan_dev_event_desc));
            rt_exit_critical();
            return RT_EOK;
        }
    }
    rt_exit_critical();
    /* not find iteam */
    return -RT_ERROR;
}

int rt_wlan_dev_indicate_event_handle(struct rt_wlan_device *device, rt_wlan_dev_event_t event, struct rt_wlan_buff *buff)
{
    void *parameter[RT_WLAN_DEV_EVENT_NUM];
    rt_wlan_dev_event_handler handler[RT_WLAN_DEV_EVENT_NUM];
    int i;

    if (device == RT_NULL) return -RT_EIO;
    if (event >= RT_WLAN_DEV_EVT_MAX) return -RT_EINVAL;

    /* get callback handle */
    rt_enter_critical();
    for (i = 0; i < RT_WLAN_DEV_EVENT_NUM; i++)
    {
        handler[i] = device->handler_table[event][i].handler;
        parameter[i] = device->handler_table[event][i].parameter;
    }
    rt_exit_critical();

    /* run callback */
    for (i = 0; i < RT_WLAN_DEV_EVENT_NUM; i++)
    {
        if (handler[i] != RT_NULL)
        {
            handler[i](device, event, buff, parameter[i]);
        }
    }
    return RT_EOK;
}

int rt_wlan_dev_enter_pormisc(struct rt_wlan_device *device)
{
    int result = 0;
    int enable = 1;

    if (device == RT_NULL) return -RT_EIO;
    result = rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_CFG_PROMISC, &enable);
    return result;
}

int rt_wlan_dev_exit_pormisc(struct rt_wlan_device *device)
{
    int result = 0;
    int enable = 0;

    if (device == RT_NULL) return -RT_EIO;
    result = rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_CFG_PROMISC, &enable);
    return result;
}

int rt_wlan_dev_cfg_filter(struct rt_wlan_device *device, struct rt_wlan_filter *filter)
{
    int result = 0;

    if (device == RT_NULL) return -RT_EIO;
    if (filter == RT_NULL) return -RT_ERROR;
    result = rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_CFG_FILTER, filter);
    return result;
}

int rt_wlan_dev_set_channel(struct rt_wlan_device *device, int channel)
{    
    int result = 0;

    if (device == RT_NULL) return -RT_EIO;
    if (channel < 0) return -RT_ERROR;
    result = rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_CFG_FILTER, &channel);
    return result;
}

int rt_wlan_dev_set_country(struct rt_wlan_device *device, rt_country_code_t country_code)
{
    int result = 0;

    if (device == RT_NULL) return -RT_EIO;
    result = rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_SET_COUNTRY, &country_code);
    return result;
}

rt_country_code_t rt_wlan_dev_get_country(struct rt_wlan_device *device)
{
    int result = 0;
    rt_country_code_t country_code = RT_COUNTRY_UNKNOWN;

    if (device == RT_NULL) return -RT_EIO;
    result = rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_GET_COUNTRY, &country_code);
    if (result != RT_EOK) return RT_COUNTRY_UNKNOWN;
    return country_code;
}

rt_err_t rt_wlan_dev_scan(struct rt_wlan_device *device, struct rt_wlan_info *info)
{
    struct rt_scan_info scan_info = { 0 };
    struct rt_scan_info *p_scan_info = RT_NULL;
    rt_err_t result = 0;

    if (device == RT_NULL) return -RT_EIO;
    if (info != RT_NULL)
    {
        if (info->ssid.len >= SSID_LENGTH_MAX_SIZE)
        {
            LOG_E("L:%d ssid is to long", __LINE__);
            return -RT_EINVAL;
        }
        rt_memcpy(&scan_info.ssid, &info->ssid, sizeof(rt_wlan_ssid_t));
        rt_memcpy(scan_info.bssid, info->bssid, BSSID_LENGTH_MAX_SIZE);
        scan_info.channel_min = -1;
        scan_info.channel_max = -1;
        p_scan_info = &scan_info;
    }
    result = rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_SCAN, p_scan_info);
    return result;
}

rt_err_t rt_wlan_dev_scan_stop(struct rt_wlan_device *device)
{
    rt_err_t result = 0;

    if (device == RT_NULL) return -RT_EIO;
    result = rt_device_control(RT_DEVICE(device), RT_WLAN_CMD_SCAN_STOP, RT_NULL);
    return result;
}

int rt_wlan_dev_report_data(struct rt_wlan_device *device, void *buff, int len)
{
    return rt_wlan_dev_transfer_prot(device, buff, len);
}

static rt_err_t _rt_wlan_dev_init(rt_device_t dev)
{
    struct rt_wlan_device *wlan = (struct rt_wlan_device *)dev;
    rt_err_t result = RT_EOK;

    rt_mutex_init(&wlan->lock, "wlan_dev", RT_IPC_FLAG_FIFO);

    if (wlan->ops->wlan_init) 
        result = wlan->ops->wlan_init(wlan);

    if (result == RT_EOK)
    {
        LOG_I("wlan init success");
    }
    else
    {
        LOG_I("wlan init failed");
    }

    return result;
}

static rt_err_t _rt_wlan_dev_control(rt_device_t dev, int cmd, void *args)
{
    struct rt_wlan_device *wlan = (struct rt_wlan_device *)dev;
    rt_err_t err = RT_EOK;

    RT_ASSERT(dev != RT_NULL);

    WLAN_DEV_LOCK(wlan);

    switch(cmd)
    {
    case RT_WLAN_CMD_MODE:
    {
        rt_wlan_mode_t mode = *((rt_wlan_mode_t *)args);

        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, RT_WLAN_CMD_MODE, "RT_WLAN_CMD_MODE");
        if (wlan->ops->wlan_mode)
            err = wlan->ops->wlan_mode(wlan, mode);
        break;
    }
    case RT_WLAN_CMD_SCAN:
    {
        struct rt_scan_info *scan_info = args;

        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, RT_WLAN_CMD_SCAN, "RT_WLAN_CMD_SCAN");
        if (wlan->ops->wlan_scan)
            err = wlan->ops->wlan_scan(wlan, scan_info);
        break;
    }
    case RT_WLAN_CMD_JOIN:
    {
        struct rt_sta_info *sta_info = args;

        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, RT_WLAN_CMD_JOIN, "RT_WLAN_CMD_JOIN");
        if (wlan->ops->wlan_join)
            err = wlan->ops->wlan_join(wlan, sta_info);
        break;
    }
    case RT_WLAN_CMD_SOFTAP:
    {
        struct rt_ap_info *ap_info = args;

        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, RT_WLAN_CMD_SOFTAP, "RT_WLAN_CMD_SOFTAP");
        if (wlan->ops->wlan_softap)
            err = wlan->ops->wlan_softap(wlan, ap_info);
        break;
    }
    case RT_WLAN_CMD_DISCONNECT:
    {
        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, RT_WLAN_CMD_DISCONNECT, "RT_WLAN_CMD_DISCONNECT");
        if (wlan->ops->wlan_disconnect)
            err = wlan->ops->wlan_disconnect(wlan);   
        break;
    }
    case RT_WLAN_CMD_AP_STOP:
    {
        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, RT_WLAN_CMD_AP_STOP, "RT_WLAN_CMD_AP_STOP");
        if (wlan->ops->wlan_ap_stop)
            err = wlan->ops->wlan_ap_stop(wlan);
        break;
    }
    case RT_WLAN_CMD_AP_DEAUTH:
    {
        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, RT_WLAN_CMD_AP_DEAUTH, "RT_WLAN_CMD_AP_DEAUTH");
        if (wlan->ops->wlan_ap_deauth)
            err = wlan->ops->wlan_ap_deauth(wlan, args);
        break;
    }
    case RT_WLAN_CMD_SCAN_STOP:
    {
        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, RT_WLAN_CMD_SCAN_STOP, "RT_WLAN_CMD_SCAN_STOP");
        if (wlan->ops->wlan_scan_stop)
            err = wlan->ops->wlan_scan_stop(wlan);
        break;
    }
    case RT_WLAN_CMD_GET_RSSI:
    {
        int *rssi = args;

        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, RT_WLAN_CMD_GET_RSSI, "RT_WLAN_CMD_GET_RSSI");
        if (wlan->ops->wlan_get_rssi)
            *rssi = wlan->ops->wlan_get_rssi(wlan);
        break;
    }
    case RT_WLAN_CMD_POWERSAVE:
    {
        rt_bool_t enable = *((rt_bool_t *)args);

        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, RT_WLAN_CMD_POWERSAVE, "RT_WLAN_CMD_POWERSAVE");
        if (wlan->ops->wlan_powersave)
            wlan->ops->wlan_powersave(wlan, enable);
        break;
    }
    case RT_WLAN_CMD_CFG_PROMISC:
    {
        rt_bool_t start = *((rt_bool_t *)args);

        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, RT_WLAN_CMD_CFG_PROMISC, "RT_WLAN_CMD_CFG_PROMISC");
        if (wlan->ops->wlan_cfg_promisc)
            wlan->ops->wlan_cfg_promisc(wlan, start);
        break;
    }
    case RT_WLAN_CMD_CFG_FILTER:
    {
        struct rt_wlan_filter *filter = args;

        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, RT_WLAN_CMD_CFG_FILTER, "RT_WLAN_CMD_CFG_FILTER");
        if (wlan->ops->wlan_cfg_filter)
            wlan->ops->wlan_cfg_filter(wlan, filter);
        break;
    }
    case RT_WLAN_CMD_SET_CHANNEL:
    {
        int channel = *(int *)args;
        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, RT_WLAN_CMD_SET_CHANNEL, "RT_WLAN_CMD_SET_CHANNEL");
        if (wlan->ops->wlan_set_channel)
            wlan->ops->wlan_set_channel(wlan, channel);
        break;
    }
    case RT_WLAN_CMD_GET_CHANNEL:
    {
        int *channel = args;

        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, RT_WLAN_CMD_GET_CHANNEL, "RT_WLAN_CMD_GET_CHANNEL");
        if (wlan->ops->wlan_get_channel)
            *channel = wlan->ops->wlan_get_channel(wlan);
        break;
    }
    case RT_WLAN_CMD_SET_COUNTRY:
    {
        rt_country_code_t country = *(rt_country_code_t *)args;

        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, RT_WLAN_CMD_SET_COUNTRY, "RT_WLAN_CMD_SET_COUNTRY");
        if (wlan->ops->wlan_set_country)
            wlan->ops->wlan_set_country(wlan, country);
        break;
    }
    case RT_WLAN_CMD_GET_COUNTRY:
    {
        rt_country_code_t *country = args;
        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, RT_WLAN_CMD_GET_COUNTRY, "RT_WLAN_CMD_GET_COUNTRY");
        if (wlan->ops->wlan_get_country)
            *country = wlan->ops->wlan_get_country(wlan);
        break;
    }
    case RT_WLAN_CMD_SET_MAC:
    {
        rt_uint8_t *mac = args;

        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, RT_WLAN_CMD_SET_MAC, "RT_WLAN_CMD_SET_MAC");
        if (wlan->ops->wlan_set_mac)
            wlan->ops->wlan_set_mac(wlan, mac);
        break;
    }
    case RT_WLAN_CMD_GET_MAC:
    {
        rt_uint8_t *mac = args;

        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, RT_WLAN_CMD_GET_MAC, "RT_WLAN_CMD_GET_MAC");
        if (wlan->ops->wlan_get_mac)
            wlan->ops->wlan_get_mac(wlan, mac);
        break;
    }
    default:
        LOG_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, -1, "UNKUOWN");
        break;
    }

    WLAN_DEV_UNLOCK(wlan);

    return err;
}

struct rt_wlan_device *rt_wlan_dev_register(const char *name, const struct rt_wlan_dev_ops *ops, void *user_data)
{
    struct rt_wlan_device *wlan;

    if (name == RT_NULL || ops == RT_NULL)
    {
        LOG_E("F:%s L:%d parameter Wrongful", __FUNCTION__, __LINE__);
        return RT_NULL;
    }

    wlan = rt_malloc(sizeof(struct rt_wlan_device));
    if (wlan == RT_NULL)
    {
        LOG_E("F:%s L:%d", __FUNCTION__, __LINE__);
        return RT_NULL;
    }
    rt_memset(wlan, 0, sizeof(struct rt_wlan_device));

    wlan->device.init       = _rt_wlan_dev_init;
    wlan->device.open       = RT_NULL;
    wlan->device.close      = RT_NULL;
    wlan->device.read       = RT_NULL;
    wlan->device.write      = RT_NULL;
    wlan->device.control    = _rt_wlan_dev_control;
    wlan->device.user_data  = RT_NULL;

    wlan->device.type = RT_Device_Class_NetIf;

    wlan->ops = ops;
    wlan->user_data  = user_data;
    rt_device_register(&wlan->device, name, RT_DEVICE_FLAG_RDWR);

    LOG_D("F:%s L:%d run", __FUNCTION__, __LINE__);

    return wlan;
}
