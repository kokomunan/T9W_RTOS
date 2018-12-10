#ifndef WIFI_API_H_
#define WIFI_API_H_

#include "stm32f4xx_hal.h"
#include "mxconstants.h"
#include "config.h"
#include "stdint.h"

#define TRNASFER_MTU        64
//设置类指令or带参查询类指令
//uart指令
static	char uart_set[]="AT+UART=";
static	char uarte_set[]="AT+UARTE=";
//wifi指令
static	char wjapip_set[]="AT+WJAPIP=";
static	char wjap_set[]="AT+WJAP=";
static	char wdhcp_set[]="AT+WDHCP=";
//tcp指令
static	char cipautoconn_set[]="AT+CIPAUTOCONN=";  //设置指定ip自动连接
static	char cipstart_set[]="AT+CIPSTART=";
static	char cipstop_set[]="AT+CIPSTOP=";
static	char ciprecv_set[]="AT+CIPRECV=";
//带参查询类指令
static	char cipautoconn_inquire[]="AT+CIPAUTOCONN=";
static	char cipstatus_inquire[]="AT+CIPSTATUS=";

//查询系统类指令
//基本指令
static	char fwver_inquire[]="AT+FWVER?\r";
//uart指令
static	char uart_inquire[]="AT+UART?\r";
//wifi指令
static	char wjapip_inquire[]="AT+WJAPIP?\r";
//tcp指令
static	char ciprecvcfg_inquire[]="AT+CIPRECVCFG?\r";  //查询wifi数据发送到用户串口是指令模式还是透传模式

//执行类指令
//基本指令`
static	char reboot[]="AT+REBOOT\r";
static	char factory[]="AT+FACTORY\r";
static	char cipsendraw[]="AT+CIPSENDRAW\r";  //AT模式-透传模式
//wifi指令
static	char wmac[]="AT+WMAC\r";
static	char wscan[]="AT+WSCAN\r";
static	char wjapq[]="AT+WJAPQ\r";  //关闭当前station模式
static	char wjaps[]="AT+WJAPS\r";  //查看当前station模式连接状态

//查询系统类指令
//wifi指令
static	char wjap_inquire[]="AT+WJAP=?\r";  //查询station模式要接入的ap的信息

//tcp 数据发送
static	char cipsend_set[]="AT+CIPSEND=";
//static	char cipsend_para[]=TCP_ID+LOCAL_PORT;

enum WIFI_AT_COMMOND{
	UART_SET,
	UARTE_SET,
	WJAPIP_SET,
	WJAP_SET,
	WDHCP_SET,
	CIPAUTOCONN_SET,
	CIPSTART_SET,
	CIPSTOP_SET,
	CIPSEND_SET,
	CIPRECV_SET,
	
	CIPAUTOCONN_INQUIRE,
	CIPSTATUS_INQUIRE,
	
	FWVER_INQUIRE,
	UART_INQUIRE,
	WJAPIP_INQUIRE,
	CIPRECVCFG_INQUIRE,
	
	REBOOT,
	FACTORY,
	CIPSENDRAW,
	WMAC,
	WSCAN,
	WJAPQ,
	WJAPS,
	
	WJAP_INQUIRE,
	
	
};

enum WIFI_AT_RESPONSE{

	AT_RESPONSE_NULL,
	AT_RESPONSE_OK,       //操作成功
	AT_RESPONSE_ERROR,    //操作失败
	AT_TCP_START_SEND_DATA,
	AT_READY_RECIVE,
	
};



uint8_t wifi_AT_commond_set(uint8_t cmd,char *para);
uint8_t wifi_AT_commond_inquire_sys(uint8_t cmd);
uint8_t wifi_AT_commond_excute(uint8_t cmd);
uint8_t wifi_AT_commond_inquire_user(uint8_t cmd);
uint8_t wifi_data_transmit(uint8_t *data, uint8_t len);

uint8_t Wait_Wifi_Response(uint32_t  time);
void wifi_Response_AT_Handle(void);



#endif

