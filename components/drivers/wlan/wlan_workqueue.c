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
 * 2018-08-19     RT-Thread    the first verion
 */

#include <rthw.h>
#include <rtthread.h>
#include <wlan_workqueue.h>

#define DBG_ENABLE
#define DBG_LEVEL DBG_INFO
#define DBG_SECTION_NAME  "WLAN.work"
#define DBG_COLOR
#include <rtdbg.h>

struct rt_wlan_work
{
    struct rt_work work;
    void (*fun)(void* parameter);
    void *parameter;
};

static struct rt_workqueue *wlan_workqueue;

static void rt_wlan_workqueue_fun(struct rt_work* work, void* work_data)
{
    struct rt_wlan_work *wlan_work = work_data;

    wlan_work->fun(wlan_work->parameter);
    rt_free(wlan_work);
}

rt_err_t rt_wlan_workqueue_dowork(void (*func)(void *parameter), void *parameter)
{
    struct rt_wlan_work *wlan_work;
    rt_err_t err = RT_EOK;

    LOG_D("F:%s is run", __FUNCTION__);
    if (func == RT_NULL)
    {
        LOG_E("F:%s L:%d func is null", __FUNCTION__, __LINE__);
        return -RT_EINVAL;
    }

    if (wlan_workqueue == RT_NULL)
    {
        LOG_E("F:%s L:%d not init wlan work queue", __FUNCTION__, __LINE__);
        return -RT_ERROR;
    }

    wlan_work = rt_malloc(sizeof(struct rt_wlan_work));
    if (wlan_work == RT_NULL)
    {
        LOG_E("F:%s L:%d create work failed", __FUNCTION__, __LINE__);
        return -RT_ENOMEM;
    }
    wlan_work->fun = func;
    wlan_work->parameter = parameter;
    rt_work_init(&wlan_work->work, rt_wlan_workqueue_fun, wlan_work);
    err = rt_workqueue_dowork(wlan_workqueue, &wlan_work->work);
    if (err != RT_EOK)
    {
        LOG_E("F:%s L:%d do work failed", __FUNCTION__, __LINE__);
        rt_free(wlan_work);
        return err;
    }
    return err;
}

struct rt_workqueue *rt_wlan_get_workqueue(void)
{
    return wlan_workqueue;
}

int rt_wlan_workqueue_init(void)
{
    static rt_int8_t _init_flag = 0;

    if (_init_flag == 0)
    {
        wlan_workqueue = rt_workqueue_create(RT_WLAN_WORKQUEUE_THREAD_NAME, RT_WLAN_WORKQUEUE_THREAD_SIZE,
            RT_WLAN_WORKQUEUE_THREAD_PRIO);
        if (wlan_workqueue == RT_NULL)
        {
            LOG_E("F:%s L:%d wlan work queue create failed", __FUNCTION__, __LINE__);
            return -1;
        }
        _init_flag = 1;
        return 0;
    }
    return 0;
}
INIT_PREV_EXPORT(rt_wlan_workqueue_init);
