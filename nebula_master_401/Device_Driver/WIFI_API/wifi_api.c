#include "wifi_api.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "string.h"
#include "config.h"
#include "server.h"
#include "device_info.h"
#include "usb_host.h"
#include "usb_controler.h"
#include "Status_Machin.h"
#include "debug_log.h"
#include "Soft_Timer.h"
#include "display.h"
#include "misc.h"
#include "wifi_uart.h"
#include "ewm3080b.h"

#define  WAIT_TIME    1000

char to_wifi_buf[TRNASFER_MTU];
char from_wifi_buf[TRNASFER_MTU];


uint8_t	 Response_Status=0;
uint8_t	 Wifi_Status=0;
uint8_t	 wifi_response=0;

uint8_t	 wait_tcp_send_data_flag=0;
uint8_t	 wait_return_flag=0;
uint8_t wifi_send_block(char *data,uint8_t len,uint8_t target_result)
{
    uint8_t res;
    wifi_response=0;
    wifi_transmit(data,len);					
    res=Wait_Wifi_Response(WAIT_TIME);  //等待响应					
    if(res==0)
    {
        if(Response_Status==target_result)  	//wifi回复内容正确
        {         
            return 0;
        }

    }
        
    return 1;
        
}


uint8_t wifi_AT_commond_set(uint8_t cmd,char *para)  //设置类指令or带参查询类指令  (参数字符串 结尾需带换行符 '\r')
{
		uint8_t res;
        uint8_t target_result=AT_RESPONSE_OK;
		char end[]="\r";
		memset(to_wifi_buf,'\0',TRNASFER_MTU);   //清零to_wifi_buf
       	Response_Status=AT_RESPONSE_NULL;
		switch(cmd)
		{
				case UART_SET:
				{
											
						strcpy(to_wifi_buf,uart_set);                    
						
				}
				break;
				case UARTE_SET:
				{
											
						strcpy(to_wifi_buf,uarte_set);
                                        
						
				}
				break;
				case WJAPIP_SET:
				{
												
						strcpy(to_wifi_buf,wjapip_set);
                 
				}
				break;
				case WJAP_SET:
				{
						
						strcpy(to_wifi_buf,wjap_set);
	
				}
                break;
				case WDHCP_SET:
				{
						
						strcpy(to_wifi_buf,wdhcp_set);
				}
				break;
				case CIPAUTOCONN_SET:
				{
					
						
						strcpy(to_wifi_buf,cipautoconn_set);
                     
				}
				break;
				case CIPSTART_SET:
				{
						
						strcpy(to_wifi_buf,cipstart_set);

				}
				break;
				case CIPSTOP_SET:
				{
						
						strcpy(to_wifi_buf,cipstop_set);

				}
				break;
				case CIPSEND_SET:
				{
						
						wait_tcp_send_data_flag=1;
                        target_result=AT_TCP_START_SEND_DATA;
						strcpy(to_wifi_buf,cipsend_set);

				}
				break;
				case CIPRECV_SET:
				{
						
						strcpy(to_wifi_buf,ciprecv_set);

				}
				break;

				case CIPAUTOCONN_INQUIRE:
				{
												
						strcpy(to_wifi_buf,cipautoconn_inquire);
				  	                       
				}
				break;
				case CIPSTATUS_INQUIRE:
				{									
						strcpy(to_wifi_buf,cipstatus_inquire);
				}
				break;

				default:
                    
                        return 0;

		}

    strcat(to_wifi_buf,para);
    strcat(to_wifi_buf,end);       
    res=wifi_send_block(to_wifi_buf,strlen(to_wifi_buf),target_result);        
	return res;
		
}


uint8_t wifi_AT_commond_inquire_sys(uint8_t cmd)  //查询系统类指令
{
		uint8_t res;
         
		switch(cmd)
		{
			case FWVER_INQUIRE:
			{
					Response_Status=AT_RESPONSE_NULL;
					
					strcpy(to_wifi_buf,fwver_inquire);
				
					wifi_response=0;
				    wait_return_flag=1;
					wifi_transmit(to_wifi_buf,strlen(to_wifi_buf));
					
					res=Wait_Wifi_Response(WAIT_TIME);  //等待响应
				
					if(res==0)
					{
						if(Response_Status==AT_RESPONSE_OK)  	//wifi回复内容正确
						{
							
							return 0;
						}
					}
			}
			break;
			case UART_INQUIRE:
			{
					Response_Status=AT_RESPONSE_NULL;
					
					strcpy(to_wifi_buf,uart_inquire);
					wifi_transmit(to_wifi_buf,strlen(to_wifi_buf));

					res=Wait_Wifi_Response(WAIT_TIME);  //等待响应
					wifi_response=0;
					if(res==0)
						if(Response_Status==AT_RESPONSE_OK)  	//wifi回复内容正确
						{
							
							return 0;
						}
			}
			break;
			case WJAPIP_INQUIRE:
			{
					Response_Status=AT_RESPONSE_NULL;
					
					strcpy(to_wifi_buf,wjapip_inquire);
					wifi_transmit(to_wifi_buf,strlen(to_wifi_buf));

					res=Wait_Wifi_Response(WAIT_TIME);  //等待响应
					wifi_response=0;
					if(res==0)
						if(Response_Status==AT_RESPONSE_OK)  	//wifi回复内容正确
						{
							
							return 0;
						}
			}
			break;
			case CIPRECVCFG_INQUIRE:
			{
					Response_Status=AT_RESPONSE_NULL;
					
					strcpy(to_wifi_buf,ciprecvcfg_inquire);
					wifi_transmit(to_wifi_buf,strlen(to_wifi_buf));

					res=Wait_Wifi_Response(WAIT_TIME);  //等待响应
					wifi_response=0;
					if(res==0)
						if(Response_Status==AT_RESPONSE_OK)  	//wifi回复内容正确
						{
							
							return 0;
						}
			}
			break;

			default:
			break;

		}

		
		return 1;
} 

uint8_t wifi_AT_commond_excute(uint8_t cmd)  //执行类指令
{
		uint8_t res;

		switch(cmd)
		{
				case REBOOT:
				{
						Response_Status=AT_RESPONSE_NULL;
						
						strcpy(to_wifi_buf,reboot);
						wifi_transmit(to_wifi_buf,strlen(to_wifi_buf));

						res=Wait_Wifi_Response(WAIT_TIME);  //等待响应
						wifi_response=0;
						if(res==0)
							if(Response_Status==AT_RESPONSE_OK)  	//wifi回复内容正确
							{
								
								return 0;
							}
				}
				break;
				case FACTORY:
				{
						Response_Status=AT_RESPONSE_NULL;
						
						strcpy(to_wifi_buf,factory);
						wifi_transmit(to_wifi_buf,strlen(to_wifi_buf));

						res=Wait_Wifi_Response(WAIT_TIME);  //等待响应
						wifi_response=0;
						if(res==0)
							if(Response_Status==AT_RESPONSE_OK)  	//wifi回复内容正确
							{
								
								return 0;
							}
				}
				break;
				case CIPSENDRAW:
				{
						Response_Status=AT_RESPONSE_NULL;
						
						strcpy(to_wifi_buf,cipsendraw);
						wifi_transmit(to_wifi_buf,strlen(to_wifi_buf));

						res=Wait_Wifi_Response(WAIT_TIME);  //等待响应
						wifi_response=0;
						if(res==0)
							if(Response_Status==AT_RESPONSE_OK)  	//wifi回复内容正确
							{
								
								return 0;
							}
				}
				break;
				case WMAC:
				{
						Response_Status=AT_RESPONSE_NULL;
						
						strcpy(to_wifi_buf,wmac);
						wifi_transmit(to_wifi_buf,strlen(to_wifi_buf));

						res=Wait_Wifi_Response(WAIT_TIME);  //等待响应
						wifi_response=0;
						if(res==0)
							if(Response_Status==AT_RESPONSE_OK)  	//wifi回复内容正确
							{
								
								return 0;
							}
				}
				break;
				case WSCAN:
				{
						Response_Status=AT_RESPONSE_NULL;
						
						strcpy(to_wifi_buf,wscan);
						wifi_transmit(to_wifi_buf,strlen(to_wifi_buf));

						res=Wait_Wifi_Response(WAIT_TIME);  //等待响应
						wifi_response=0;
						if(res==0)
							if(Response_Status==AT_RESPONSE_OK)  	//wifi回复内容正确
							{
								
								return 0;
							}
				}
				break;
				case WJAPQ:
				{
						Response_Status=AT_RESPONSE_NULL;
						
                    
                    
						strcpy(to_wifi_buf,wjapq);
                    	wifi_response=0;
						wifi_transmit(to_wifi_buf,strlen(to_wifi_buf));

						res=Wait_Wifi_Response(WAIT_TIME);  //等待响应
					
						if(res==0)
                        {
							if(Response_Status==AT_RESPONSE_OK)  	//wifi回复内容正确
							{
								
								return 0;
							}
                        }
				}
				break;
				case WJAPS:
				{
						Response_Status=AT_RESPONSE_NULL;
						
						strcpy(to_wifi_buf,wjaps);
						wifi_transmit(to_wifi_buf,strlen(to_wifi_buf));

						res=Wait_Wifi_Response(WAIT_TIME);  //等待响应
						wifi_response=0;
						if(res==0)
							if(Response_Status==AT_RESPONSE_OK)  	//wifi回复内容正确
							{
								
								return 0;
							}
				}
				break;

				default:
				break;
		}

		
		return 1;
}

uint8_t wifi_AT_commond_inquire_user(uint8_t cmd)  //查询用户类指令
{
		uint8_t res;

		switch(cmd)
		{
				case WJAP_INQUIRE:
				{
						Response_Status=AT_RESPONSE_NULL;
						
						strcpy(to_wifi_buf,wjap_inquire);
						wifi_transmit(to_wifi_buf,strlen(to_wifi_buf));

						res=Wait_Wifi_Response(WAIT_TIME);  //等待响应
						wifi_response=0;
						if(res==0)
							if(Response_Status==AT_RESPONSE_OK)  	//wifi回复内容正确
							{
								
								return 0;
							}
				}
				break;

				default:
				break;
		}

		
		return 1;
}
uint8_t wifi_data_transmit(uint8_t *data, uint8_t len)   //
{
		uint8_t res;
		char data_len[20];
		char para[20];

	
		memset(para,'\0',20);
		strcpy(para,TCP_ID);
	    sprintf(data_len,",%d",len);
		strcat(para,data_len);
	
		res = wifi_AT_commond_set(CIPSEND_SET,para);
		if(!res)
		{
				memset(to_wifi_buf,'\0',TRNASFER_MTU);   //清零to_wifi_buf
				Response_Status=AT_RESPONSE_NULL;
			    wifi_response=0;
               	LOW;
				wifi_transmit((char *)data,len);
			   
                HIGH;
        
				res=Wait_Wifi_Response(WAIT_TIME);  //等待响应
				LOW;
				if(res==0)
                {
					if(Response_Status==AT_RESPONSE_OK)  	//wifi回复内容正确
					{
						
						return 0;
					}
                }
                else
                {
                    debug_log("wait ok over time");
                }
		}
		return 1;
}

//阻塞模式等待响应，超时返回错误
uint8_t Wait_Wifi_Response(uint32_t  time)
{
    uint32_t count=0;

    do{
        HAL_Delay(1); 
        count++;
    }while((count<time)&&(wifi_response==0));
    if(count>=time)
    {
        return 1;
    }
    else
    {
        return 0;
    }
	
}

void wifi_Response_callback(void)
{
			char ok[]="OK",error[]="ERROR";
			char wevent[]="+WEVENT",cipevent[]="+CIPEVENT",mqttevent[]="+MQTTEVENT";
			char station_up[]="STATION_UP",station_down[]="STATION_DOWN";
			char serverconnected[]="SERVER,CONNECTED",serverclosed[]="SERVER,CLOSED",serverdisconnected[]="SERVER,DISCONNECTED";
			char cipsocket[]="SOCKET";
			char*temp;
			uint8_t data_len=0;
			
			if(strstr(from_wifi_buf,cipevent))
			{
					if(strstr(from_wifi_buf,cipsocket))	//TCP接收数据命令 //+CIPEVENT:SOCKET,0,9,123456789
					{
									temp = strtok(from_wifi_buf, ",");
									temp = strtok(NULL, ",");
									temp = strtok(NULL, ",");      //data_len 数据指针
									data_len=*temp;
									temp=&from_wifi_buf[21];

						HAL_WIFI_data_recive((uint8_t *)temp, data_len);
					}
					else if(strstr(from_wifi_buf,serverconnected))
					{
							TCP_SERVER_CONNECTED_callback();
					}
					else if(strstr(from_wifi_buf,serverdisconnected))
					{
							TCP_SERVER_DISCONNECTED_callback();
						
						
					}
					else if(strstr(from_wifi_buf,serverclosed))
					{
							TCP_SERVER_CLOSED_callback();
					}
			}
			else if(strstr(from_wifi_buf,wevent))
			{
					if(strstr(from_wifi_buf,station_up))
					{
							STATION_UP_callback();
					}
					else if(strstr(from_wifi_buf,station_down))	
					{
							STATION_DOWN_callback();
					}
			}
			else if(strstr(from_wifi_buf,mqttevent))
			{
				//暂不处理
			}
			else if(strstr(from_wifi_buf,ok))  //字符串中找'OK'的指针，找到返回指针，没有返回空指针
			{
					char ch=':';
					char *ret;
					char response_para[strlen(from_wifi_buf)];
						
					memset(response_para,'\0',strlen(from_wifi_buf));  //清零response_para字符串
					ret=strchr(from_wifi_buf, ch);
					if(ret!=NULL)
					{
						strcpy(response_para,ret+1);    //from_wifi_buf内容放入response_para
						HAL_WIFI_return_result((uint8_t *)response_para, strlen(response_para));   //':'后面内容
					}
					
				    Response_Status=AT_RESPONSE_OK;	
					wifi_response=1;
				
			}
			else if(strstr(from_wifi_buf,error))
			{
					Response_Status=AT_RESPONSE_ERROR;
					wifi_response=1;
			}

			
			memset(from_wifi_buf,'\0',TRNASFER_MTU);   //清零from_ble_buf
}

