/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 * @FileName: supermonitor.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年2月9日 下午3:33:23
 *  
 */

#ifndef SUPERMONITOR_H_
#define SUPERMONITOR_H_

#include <pcap.h>
#include <map>
#include <iostream>
#include <vector>

#include "monitor_define.h"
#include "capturedata.h"
#include "parsedata.h"
#include "mutexlock.h"
#include "thread.h"
#include "define.h"

class MonitorManager;
class SuperMonitor :public Thread{
public:
	SuperMonitor(MonitorManager* mm);
	virtual ~SuperMonitor();
	void set_stop();
	static void monitorData(u_char *arg, const struct pcap_pkthdr *pkthdr, const u_char *packet);
	static thread_start_func(start);
private:
	TaskT* get_taskFromList();
	void handle_task(TaskT* task);
private:
	TaskList taskList;
	MutexLock taskLock;
	declare_class_member(bool, stop)
	declare_class_member(IntBoolMap, desPortMap)//this map is not empty, only handle this port.
	declare_class_member(CaptureData*, captureData)
	MonitorManager* m_monitorManager;
};

#endif /* SUPERMONITOR_H_ */
