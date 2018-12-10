#include "page_detect.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "Soft_Timer.h"
#include "debug_log.h"
#include "server.h"
#include "Status_Machin.h"
#include "note_manager.h"
#include "display.h"
#include "ble_client.h"
#define DEBUG  debug_log
UART_HandleTypeDef huart2;
extern st_device_data_storage g_device_info;

#define UART_CONTROLER    &huart2
#define DATA_SIZE     4
uint8_t rev_flag=0;
typedef struct{
 uint8_t tx_buff[DATA_SIZE];
 uint8_t rx_buff[DATA_SIZE];
 uint16_t value;  
 uint16_t last_value;   
 uint8_t  value_change_flag;    
       
}PAGE_DET_ST;

PAGE_DET_ST page_det;
uint16_t old_page=0;
uint8_t repeat_count=0;
uint8_t valid_flag=0;
uint8_t uart2_temp=0;
uint8_t uart2_rcv_step =0;
uint8_t uart2_cmd_count=0;
uint8_t uart2_cmd_flag=0;
uint8_t out_really_value=0;



osThreadId  read_page_thread_handle=NULL;
osSemaphoreId  valid_page_handle=NULL;


void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    DEBUG("init uart2 failed");
  }
    
  
	__HAL_UART_ENABLE(&huart2);



}


void page_det_moudule_init(void)
{
   MX_USART2_UART_Init();
   SET_BIT(huart2.Instance->CR1, USART_CR1_RXNEIE);
   memset((uint8_t *)&page_det,0,sizeof(PAGE_DET_ST));
   old_page=0;

}

void page_det_moudule_reset(void)
{
//   memset((uint8_t *)&page_det,0,sizeof(PAGE_DET_ST));
//   old_page=0;
//   valid_flag=0;
    
}

void page_det_start(void)
{
    DEBUG("start page detect");
    PAGE_DET_POWER_ON() ;//打开电源
    HAL_Delay(50);   
    page_det_moudule_init();
  
    osSemaphoreDef(valid_page);
    valid_page_handle=osSemaphoreCreate (osSemaphore(valid_page), 1);
    if(valid_page_handle==NULL)
    {
        DEBUG("read_note page semaphore error");
    }

    osThreadDef(read_page_thread , read_page_thread, osPriorityNormal, 0, 256); 
    read_page_thread_handle=osThreadCreate(osThread(read_page_thread), NULL); 
    if(read_page_thread_handle==NULL)
    {
        DEBUG("read_note page thread create_new_note_head ERROR");
    }
        
    


    
}
void page_det_stop(void)
{
  DEBUG("stop page detect");
  HAL_UART_DeInit(&huart2);  
  
  
  if(valid_page_handle)
  {
      osSemaphoreDelete (valid_page_handle);
      valid_page_handle=NULL;
  }
  
  
  if(read_page_thread_handle)
  {  
      osThreadTerminate (read_page_thread_handle);
      read_page_thread_handle=NULL; 
  }
   
    
   PAGE_DET_POWER_OFF();//关闭电源
  
    
}

uint8_t IS_page_det_change(void)
{
    
    return page_det.value_change_flag;
    
}

void Clear_page_det_change_flag(void)
{
    page_det.value_change_flag=0;
}

uint16_t read_page_value()
{
    
   return   page_det.value;
    
}


void read_page_thread(const void *argument)
{

  uint8_t device_sta;
  while(1)
  {
             
      request_pages();      
      if(osSemaphoreWait (valid_page_handle, 200 )==0)
      {
          device_sta=Get_Device_Status();                               
          if(page_det.last_value!=page_det.value)
          {
              page_det.value_change_flag=1;
              page_det.last_value= page_det.value;      


              if(DEVICE_OFFLINE==device_sta)
              {

                  HAL_NVIC_DisableIRQ(EXTI0_IRQn);  //关闭模组中断 //关闭i2c中断
                  close_note(1, 0); //关闭创建的临时笔记                                     
                  HAL_NVIC_EnableIRQ(EXTI0_IRQn);  //打开模组中断//打开中断
              }

              if(DEVICE_ACTIVE ==device_sta)               
              {
                  if((get_server_mode()==NRF51822)&&(IS_ENABLE_NOTIFY==0))
                  {

                      goto exit;
                  }
                  else
                  {
                      server_notify_host(CMD_REPORT_PAGE_AUTO, (uint8_t *)&page_det.value, 2);    
                  }


              }
              
              exit:
              update_dis_online_page((page_det.value&0x00ff),page_det.value>>8);          
              DEBUG("page %x",page_det.value);  
          }
          
          
      

      }

    

  }
            

       
}

uint8_t request_pages(void)
{
    page_det.tx_buff[0]=0x55;
    memset(page_det.rx_buff,0,DATA_SIZE);    
   // HAL_UART_Transmit_IT(UART_CONTROLER, page_det.tx_buff, 1);  
  
    HAL_UART_Transmit(UART_CONTROLER, page_det.tx_buff, 1,10);  

return 0;
        
       
}

void UART2_Callback(void)  
{
     uart2_temp= (uint8_t)(huart2.Instance->DR & (uint8_t)0x00FFU);
     
    if(uart2_rcv_step == 0)
    {
        if(uart2_temp== 0x55)
        {
            uart2_cmd_count = 0;
            page_det.rx_buff[uart2_cmd_count] =uart2_temp;
            uart2_cmd_count++ ;
            uart2_rcv_step = 1;
            
        }
        else
        {
            uart2_rcv_step = 0;
        }
        
    }
    else if(uart2_rcv_step == 1)
    {
        page_det.rx_buff[uart2_cmd_count] = uart2_temp;
        uart2_cmd_count++;
        if(uart2_cmd_count >= 4)
        {
            uart2_cmd_count = 0;
            uart2_rcv_step = 0;
            uart2_cmd_flag = 1;
        }
    }
    else
    {
        
    }
    
   if( uart2_cmd_flag==1)
   { 
         uart2_cmd_flag=0;
        if((page_det.rx_buff[0]==0x55)&&(page_det.rx_buff[1]==0xaa))
        {
            uint16_t temp=0;
            temp=page_det.rx_buff[2]<<8|page_det.rx_buff[3];       
            temp=~(temp);       
          //  DEBUG("page temp %x",temp);  
                     
            if((temp&0x8001))
            {
                page_det.value=0; 
            }
            else
            {
                if( out_really_value)
                {
                     page_det.value=temp/2; 
                }
                else
                {
                    
                    if(temp&0x4000)  
                    {

                        temp&=0xbfff;
                        page_det.value=temp/2; 

                    }
                    else
                    {
                        //  DEBUG("page error");  
                        page_det.value=0; 
                    }               
                    
                }

            }
            
            if(old_page!=page_det.value)
            {
               repeat_count=0; 
               old_page=page_det.value;
            }
            else
            {
                repeat_count++;
            }
            
            if(repeat_count==3)
            {
                if(valid_page_handle)
                {
                  osSemaphoreRelease (valid_page_handle);
                }
                repeat_count=0; 
            }
        }

 
       
   }
    
    
    
}


