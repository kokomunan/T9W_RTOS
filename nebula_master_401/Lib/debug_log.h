#ifndef _debug_log_H
#define _debug_log_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "cmsis_os.h"



extern osMutexId  log_mutex_handle;
extern QueueHandle_t Log_Queue;
extern int inHandlerMode (void);

#define  UserLog(...)       {char log_temp[50],data[100];\
                            sprintf(data,"->:");\
                            sprintf(log_temp,__VA_ARGS__);\
                            strcat(data,log_temp);\
                            strcat(data,"\r\n");\
                            xQueueSendToBack( Log_Queue, &data, portMAX_DELAY);} 

//#define debug_log(...)      if(osMutexWait (log_mutex_handle, 50)==osOK)\
//                            {    printf("DEBUG->[%s]:",__FUNCTION__);\
//                                 printf(__VA_ARGS__);\
//                                 printf("\r\n");\
//                                osMutexRelease (log_mutex_handle);}
														
#define debug_log(...)      {char log_temp[50],data[100];\
                            portBASE_TYPE taskWoken;\
                            sprintf(data,"DEBUG->[%s]:",__FUNCTION__);\
                            sprintf(log_temp,__VA_ARGS__);\
                            strcat(data,log_temp);\
                            strcat(data,"\r\n");\
                            if(inHandlerMode()){\
                                xQueueSendFromISR(Log_Queue, &data, &taskWoken);\
                                portEND_SWITCHING_ISR(taskWoken);\
                            }\
                            else{\
                                xQueueSendToBack( Log_Queue, &data, portMAX_DELAY);\
                            }\
                            }
                                                                               
                                                        
                                                        
                                                        
#define err_log(...)        {char log_temp[50],data[100];\
                            sprintf(data,"ERROR->:");\
                            sprintf(log_temp,__VA_ARGS__);\
                            strcat(data,log_temp);\
                            strcat(data,"\r\n");\
                            xQueueSendToBack( Log_Queue, &data, portMAX_DELAY);} 
 

 void init_log(void );
 
 

#endif

