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
* @ClassName: networksocket.h
* @Description: TODO
* All rights Reserved, Designed By huih
* @Company: onexsoft
* @Author: hui
* @Version: V1.0
* @Date: 2016年7月29日
*
*/

#ifndef NETWORKSOCKET_H_
#define NETWORKSOCKET_H_

#include <iostream>
#include <string>
#include <string.h>
#include <errno.h>
#include <map>

#include "define.h"
#include "logger.h"
#include "stringbuf.h"
#include "systemapi.h"
#include "conf/config.h"
#include "openssl/ssl.h"

typedef union _addr {
	struct sockaddr sa;
	struct sockaddr_in sin;
	struct sockaddr_in6 sin6;
}Addr;

typedef enum _connection_type_t{
	connection_stream = 0,
	connection_dgram
}ConnectionType;

typedef struct socket_record{
	int request_conn_server_failNum; //请求连接服务端的失败次数
	socket_record() {
		this->request_conn_server_failNum = 0;
	}
	void connServerFailed() {
		this->request_conn_server_failNum++;
	}
	int get_connServerFailed() {
		return this->request_conn_server_failNum;
	}
}SocketRecord;

typedef struct socket_attach_data_t{
	declare_class_member(unsigned int, listenPort) //客户端的socket是从哪个监听端口来的。
	declare_class_member(std::string, listenAddr) //server address.
	declare_class_member(unsigned int, connHashCode)
	declare_class_member(u_uint64, connTime)
	void* pointer; //用于各个协议成自行保存数据
	FreeFunc pointer_desFunc; //pointer的内存释放函数
	socket_attach_data_t(){
		this->m_listenPort = 0;
		this->pointer = NULL;
		this->pointer_desFunc = NULL;
		this->m_connHashCode = 0;
		this->m_connTime = 0;
	}
	~socket_attach_data_t() {
		if (pointer != NULL && pointer_desFunc != NULL) {
			(*pointer_desFunc)(pointer);
		}
	}
}SocketAttachData;

typedef enum _socket_status_t{
	SOCKET_STATUS_CLEANUP_T,
	SOCKET_STATUS_WORKING_T,
} SocketStatusT;

class NetworkSocket{
public:
	NetworkSocket() {
		this->m_port = 0;
		this->m_fd = 0;
		this->m_connectionType = connection_stream;
		this->m_addressHashCode = 0;
		this->m_bufPointer = NULL;
		this->m_dataBase = NULL;
		this->m_status = SOCKET_STATUS_WORKING_T;
		this->m_address.clear();
		this->m_useSSLRead = false;
		this->m_useSSLWrite =false;
		this->m_ssl = NULL;
	}
	NetworkSocket(std::string address, int port) {
		this->m_address = address;
		this->m_port = port;
		this->m_fd = 0;
		this->m_connectionType = connection_stream;
		this->m_addressHashCode = 0;
		this->m_bufPointer = NULL;
		this->m_dataBase = NULL;
		this->m_status = SOCKET_STATUS_WORKING_T;
		this->m_useSSLRead = false;
		this->m_useSSLWrite = false;
		this->m_ssl = NULL;
	}
	~NetworkSocket() {
		if (this->m_attachData.pointer != NULL && this->m_attachData.pointer_desFunc != NULL) {
			(*this->m_attachData.pointer_desFunc)(this->m_attachData.pointer);
			this->m_attachData.pointer = NULL;
			this->m_attachData.pointer_desFunc = NULL;
		}
		if (m_fd) {
			closeSocket(m_fd);
			m_fd = 0;
		}
		if (this->m_dataBase) {
			this->m_dataBase->dec_connectionNum();
		}
	}

	static void destroy_networkSocket(void* ns) {
		NetworkSocket* tns = (NetworkSocket*)ns;
		if (tns != NULL) {
			delete tns;
		}
	}

	int addr_pton();
	int addr_ntop();
	int addr_assign(const sockaddr *addr);
	int is_validAddress();
	int set_sockReUseAddr(unsigned int sfd);
	int set_sockReUsePort(unsigned int sfd);
	int set_sockCommonOpt(unsigned int sfd, int is_unix);
	int parse_address();

	void set_addr(int af, unsigned int port);
	void set_portAndAddr(unsigned int port, std::string address);

	std::string ssl_error();
	int read_data();
	int read_nosslData();
	int get_sslErrorCode(SSL* ssl);
	int read_sslData();
	int read_dataonBlock();
	int write_data(StringBuf& buf);
	int write_nosslData(StringBuf& buf);
	int write_sslData(StringBuf& buf);

	void save_data(StringBuf& buf);
	void closeSocket(unsigned int fd);
	void clear_dataBuf();
	void clear_sendData();

	//add database connect count
	std::string connArgsMap2String();
	void add_connArgs(const std::string& key, const std::string& value);
	void add_backendArgs(const std::string& key, const std::string& value);
private:
	declare_class_member(unsigned int, fd)
	declare_class_member(unsigned int, port)
//	declare_class_member(std::string, name)//socket name:listen socket, client socket, server socket
	declare_class_member(std::string, address)
	declare_class_member(unsigned int, addressHashCode)
	declare_class_member(ConnectionType, connectionType)
	declare_class_member_co(StringBuf, recvData)
	declare_class_member_co(StringBuf, sendData)//需要重新尝试发送的数据
	declare_class_member_co(SocketRecord, socketRecord)
	//指向需要经过协议处理的数据， 有可能是本socket接收到的数据包，也有可能是新生成的数据包
	//比如sqlserver 中合并后的数据包
	declare_class_member(StringBuf*, bufPointer)
	declare_class_member_co(Addr, addr)
	declare_class_member_co(SocketAttachData, attachData)
	//当前socket所属的数据库
	declare_class_member(DataBase*, dataBase);

	//add by huih@20161220, for server socket.
	declare_class_member_co(KVStringMap, connArgsMap)
	declare_class_member_co(KVStringMap, backendArgsMap)
	declare_class_member(SocketStatusT, status)

	//add by huih@20170323, for ssl
	declare_class_member(SSL*, ssl)
	declare_class_member(bool, useSSLRead)
	declare_class_member(bool, useSSLWrite)
};
#endif /* NETWORKSOCKET_H_ */
