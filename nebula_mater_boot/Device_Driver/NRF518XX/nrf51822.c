#include "nrf51822.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "string.h"
#include "config.h"
#include "server.h"
#include "ble_client.h"


extern SPI_HandleTypeDef hspi3;
#define  SPI_CONTROLER    &hspi3
#define  RESPONSE_BUFF_LEN    64


//RESPONSE_CB   response_box;

PACKET_ST  packet;
#define BLE_BUF_NO          32

uint32_t ble_cmd_buffer_input_index = 0;
uint32_t ble_cmd_buffer_output_index = 0;
static	uint8_t buf[32];
struct BLEDataFormat ble_data_buffer[BLE_BUF_NO] = {0};
uint8_t   Ble_Response=0;
uint8_t   Host_Request_Flag=0;


BLE518XX_OPS  ble_ops={
	
    0,
    0,
    Open_ble,
    Close_ble,
    Ioctl_ble,
    Read_ble,
    Write_ble,
    manager_ble,
    ble_clear_buff,
    ble_direct_write ,	
};


//�������豸

void Open_ble(void)
{
		//��������Դ
    OPEN_BLE_POWER();	//��������Դ	
		
}

void  Close_ble(void)
{
	
    CLOSE_BLE_POWER() ;
		
}

//���������豸

uint8_t Ioctl_ble(uint8_t cmd,uint8_t *para)
{
	uint8_t res;
	switch(cmd)
	{
		case INIT://������¼� �����Ǿ�����Ӧ ���������	
		Create_Packet(QUERY_READY,0,0);
		Ble_Response=0;
		ble_rw(buf, NULL);//�����������ble 
		res=Wait_Response(10);  //�ȴ���Ӧ 
		if(res==0)
		{
			if(packet.CMD==RESPONSE_READY)
			{	
				ble_ops.status=INITIALIZATION;
			    return 0;
				
			}
			else
            {
			    return 1;	
            }                
		}			
		break;		
        case START_WORK:

        if(para)
        {

            Create_Packet(SET_MODE_PARING,0,0);
            Ble_Response=0;
            ble_rw(buf, NULL);//�����������ble 
            res=Wait_Response(10);  //�ȴ���Ӧ 
            if(res==0)
            { 
                if(packet.CMD==RESPONSE_OK)		
                {		
                    ble_ops.status=BLE_PAIRING;
                    return 0;

                }
                else
                {
                    return 1;
                }


            }

        }
        else
        {
            Create_Packet(SET_MODE_CONNECT,0,0);
            Ble_Response=0;
            ble_rw(buf, NULL);//�����������ble 
            res=Wait_Response(10);  //�ȴ���Ӧ 
            if(res==0)
            {
                if(packet.CMD==RESPONSE_OK)		
                {			
                    ble_ops.status= BLE_RECONNECTING;				
                    return 0;

                }
                else
                {
                    return 1;
                }


            }

        }

        break;
        case STOP_WORK:
        break;			 

        default: 
        break;

        }

        return 1;

}

void manager_ble(void)
{
	
    //�����host ������	//����ͻ�������
    if(Host_Request_Flag)
    {	
        host_client((uint8_t *)&packet);
        Host_Request_Flag=0;
    }
    else if(Ble_Response)	//�����֪ͨ״̬�¼�//�����л�״̬��
    {
        if(packet.CMD==RESPONSE_STA)
        {

            ble_ops.status=packet.message[0];
            server_status_call_back(packet.message[0]);  //״̬�ص�

        }
        Ble_Response=0;

    }
	

    if(	ble_ops.status== BLE_CONNECTED)
    {
        
        send_ble_data_buffer_to_host();  //�������������ȫ������ȥ ������
        
    }


}
uint8_t Read_ble(uint8_t *msg,uint8_t len)
{
	
	return 0;
}


uint8_t SD_SPI_ReadWriteByte(uint8_t TxData)
{
    uint8_t Rxdata;

    //HAL_SPI_TransmitReceive(&hspi3,&TxData,&Rxdata,1, 1000);  

    while((hspi3.Instance->SR&SPI_FLAG_TXE)==0);
        
    hspi3.Instance->DR=TxData;
        
    while((hspi3.Instance->SR&SPI_FLAG_RXNE)==0);

    Rxdata=hspi3.Instance->DR;

    return Rxdata;    
}	  

uint8_t SD_SPI_Read_N_BYTE(uint8_t *recive_buf,uint16_t len)
{
    uint16_t Rx_count;

    for(Rx_count=0;Rx_count<len;Rx_count++)
    {

        while((hspi3.Instance->SR&SPI_FLAG_TXE)==0);
            
        hspi3.Instance->DR=0xf0;
            
        while((hspi3.Instance->SR&SPI_FLAG_RXNE)==0);

        *recive_buf=hspi3.Instance->DR;
        recive_buf++;

    }
            
    return 0;    
}	 

uint8_t SD_SPI_Write_N_BYTE(uint8_t *send_buf,uint16_t len)
{
    uint16_t Tx_count;
    uint8_t temp;
    for(Tx_count=0;Tx_count<len;Tx_count++)
    {

        while((hspi3.Instance->SR&SPI_FLAG_TXE)==0);

        hspi3.Instance->DR=*send_buf;
        send_buf++;
        while((hspi3.Instance->SR&SPI_FLAG_RXNE)==0)
        {

            temp++;
            if(temp>100) 
            {
                break;
            }

        }

        temp=hspi3.Instance->DR;	

    }

    return 0;    
}




//���ط���������  ��Ҫд������ݷ��뻺�����
uint8_t Write_ble(uint8_t opcode, uint8_t * data, uint16_t len)
{
	
    uint32_t i = 0;
    uint8_t buf_index = 0;
    if(ble_ops.status != BLE_CONNECTED)
    {

        return 1;  //��̫����

    }
    if(len > 18)
    {
        return 1; //������
    }

    i = ble_cmd_buffer_input_index - ble_cmd_buffer_output_index;
    if(i > BLE_BUF_NO)
    {   

        ble_cmd_buffer_output_index = ble_cmd_buffer_input_index;
        return 1;
    }
    else
    {
        i = BLE_BUF_NO - i;//buffer left counts
        if((i*18) < len) //no much buffer
        {   

            return 1;
        }
    }


    buf_index = ble_cmd_buffer_input_index%BLE_BUF_NO;
    ble_data_buffer[buf_index].identifier = 0xAA;
    ble_data_buffer[buf_index].opcode = opcode;
    ble_data_buffer[buf_index].length = len;
    memcpy(ble_data_buffer[buf_index].payload, data, len);
    ble_cmd_buffer_input_index ++;

    return 0;
	
	
}


uint8_t ble_direct_write (uint8_t *data ,uint8_t len)
{
	
    return send_msg_to_ble(data , len);
	
}


void ble_clear_buff(void)
{
		
	ble_cmd_buffer_input_index = 0;
    ble_cmd_buffer_output_index = 0;	
		
}

void Ble_Response_callback(void)
{
	
    uint8_t   buff[64];
    PACKET_ST *packet_buff;
    uint8_t i;

    redo:
    if(IF_HAS_DATA() )	//��ȡ�ⲿ���� ����ǵ͵�ƽ
    {


        ble_rw(NULL,(uint8_t *)&buff);	

        for(i=0;i<32;i++)
        {
            if(buff[i]==0xaa)
            {	
                packet_buff=(PACKET_ST *)&buff[i];
                break;

            }

        }


        if(packet_buff->HEAD==0xaa)
        {
            if(packet_buff->CMD>=0x80)   //������host
            {

                Host_Request_Flag=1;	//֪ͨ��̨������host ������


            }
            else  //����ble����Ӧ
            {

                
                Ble_Response=1;	


            }

            memcpy(&packet,packet_buff,sizeof(PACKET_ST));

        }
        else
        {
            goto redo;
        }

    }

		
}


//�̶�ÿ�η���32�ֽ� ��д32�ֽ�
static void ble_rw(uint8_t * p_tx_data, uint8_t * p_rx_data)
{

	BLE_SELECT() ; 
    if(p_rx_data != NULL)
    {
       
        //HAL_SPI_Receive(SPI_CONTROLER,p_rx_data, 32, 5);
        SD_SPI_Read_N_BYTE(p_rx_data,32);
    }
    else
    {
        SD_SPI_Write_N_BYTE(p_tx_data,32)	;
        //	HAL_SPI_Transmit(SPI_CONTROLER, p_tx_data,32,5);
    }
  
	BLE_RELEASE();
}

//��������� 
static void Create_Packet(uint8_t cmd,uint8_t* pyload,uint8_t len)
{
	
    memset(buf,0,32);	
	buf[0]=0xaa;
	buf[1]=cmd;
	buf[2]=len;
	if(len>0)
    {
        memcpy(&buf[3], pyload,len);
    }
	
	
}
//����ģʽ�ȴ���Ӧ  ��ʱ���ش���
uint8_t Wait_Response(uint32_t  time)
{
    uint32_t count=0;

    do{
        HAL_Delay(1); 
        count++;
    }while((count<time)&&(Ble_Response==0));
    if(count>=time)
    {
        return 1;
    }
    else
    {
        return 0;
    }
	
}

uint32_t  send_msg_to_ble(uint8_t *data ,uint8_t len)
{
	uint8_t res;

	  Ble_Response=0;
		ble_rw(data, NULL);//�����������ble 
		res=Wait_Response(10);  //�ȴ���Ӧ 
		if(res==0)
		{
			if(packet.CMD==RESPONSE_OK)		
			{					
                return 0;	
			}
			else if(packet.CMD==RESPONSE_ERROR)
			{
				
				if(packet.message[0]==STATE_ERR) 
                {
					return 0;
                }
				else
                {
					return 1;
                }
				
			}
			return 1;
		
					
		}
		else
        {
			return 1;
        }
	
	
}

uint8_t send_ble_data_buffer_to_host(void)
{
    uint8_t buf_index = 0;

    do
    {


        buf_index = ble_cmd_buffer_output_index%BLE_BUF_NO;

        if(ble_cmd_buffer_input_index > ble_cmd_buffer_output_index) //
        {

            if(0== send_msg_to_ble( (uint8_t *)(&ble_data_buffer[buf_index].identifier), (ble_data_buffer[buf_index].length + 3)))  //���͸�����ģ��
            {
                ble_cmd_buffer_output_index ++;
            }
                    

        }
    }
    while(ble_cmd_buffer_input_index > ble_cmd_buffer_output_index);

    return 1;
}

