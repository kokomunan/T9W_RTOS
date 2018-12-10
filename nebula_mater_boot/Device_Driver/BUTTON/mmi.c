#include "mmi.h"
#include "stdlib.h"
extern ADC_HandleTypeDef hadc1;

#define  ADC_CONTROLER   &hadc1
#define RAGE_VALUE     100


#define BTN_CHK_VAL_POWER 	(1<<0)
#define BUTTON_PRESSED      1
#define BUTTON_RELEASED     0
#define SHORT_PRESS_TIME				5
#define DOUBLE_PRESS_INTERVAL   50
#define LONG_PRESS_TIME				  200
#define VERY_LONG_PRESS_TIME		300
#define BUTTON_PRESSED          1
#define SETBIT(BYTE,BIT)         ( (BYTE) |= (uint16_t)((uint16_t)1<<(uint16_t)(BIT)) )
#define CLRBIT(BYTE,BIT)         ( (BYTE) &= (uint16_t)((uint16_t)((uint16_t)1<<(uint16_t)(BIT))^(uint16_t)0xffff) )

const uint32_t ButtonArray_polar[BTN_TOTAL_NO]={0,1,1,1,1,1,1}; //��������
const uint32_t ButtonArray_double_interval[BTN_TOTAL_NO]={DOUBLE_PRESS_INTERVAL,DOUBLE_PRESS_INTERVAL,DOUBLE_PRESS_INTERVAL,DOUBLE_PRESS_INTERVAL
                                                          ,DOUBLE_PRESS_INTERVAL,DOUBLE_PRESS_INTERVAL,DOUBLE_PRESS_INTERVAL};

static PinStatus ButtonStatusArray[BTN_TOTAL_NO] = {0};
static uint16_t Threashold_value[3]={1200,2700,3700};
static uint16_t button_status = 0;

uint8_t GetButtonStatus(ButtonsNumber buttonNO)//return a false when button pressed
{
	return (((1<<buttonNO) & button_status)==1);
}

uint16_t get_button_ad_value(uint8_t channal)
{
	ADC_ChannelConfTypeDef ADC1_ChanConf;
	ADC1_ChanConf.Channel=channal;                                   
	ADC1_ChanConf.Rank=1 ;
	ADC1_ChanConf.SamplingTime=ADC_SAMPLETIME_480CYCLES;
	ADC1_ChanConf.Offset=0;                 
	HAL_ADC_ConfigChannel(ADC_CONTROLER,&ADC1_ChanConf);        

	HAL_ADC_Start(ADC_CONTROLER);

	HAL_ADC_PollForConversion(ADC_CONTROLER,10);               

	return 	HAL_ADC_GetValue(ADC_CONTROLER);
}


//��ȡ����״̬
uint8_t read_button(uint8_t num)
{
	uint16_t value;
	if(ButtonStatusArray[num].type==DIGITAL_IO)
	{	
		return HAL_GPIO_ReadPin(BUTTON_DET_GPIO_Port, BUTTON_DET_Pin);  //ֻ�е�Դ��������������		
	}
	else
	{
		
		value=get_button_ad_value(ButtonStatusArray[num].anolog_channal);
		if(abs(value-ButtonStatusArray[num].threshold)<RAGE_VALUE)
        {
			return 1;
        }
		else
        {
			return 0;	
        }
	}
	
}

void ButtonProcessLoop(void)  //Ҫ�����������̵�ʱ��
{
    uint8_t BIndex = 0;
    
    for(BIndex = 0;BIndex<BTN_TOTAL_NO;BIndex++)
    {
        if(ButtonArray_polar[BIndex] == read_button(BIndex))
        {
            SETBIT(button_status,BIndex);  //��������� ����λ
       
        }	
        else
        {
            CLRBIT(button_status,BIndex);   //���û�а��¾����λ
   				
        }
    }
  
    ButtonEventProcess();	
}
static void NULLFunction(void)
{
    //button_status = 0;
	return ;
}
void mmi_init(void)
{
    uint8_t i = 0;
	

    ButtonStatusArray[0].press_callback = &NULLFunction;
    ButtonStatusArray[0].short_release_callback = &NULLFunction;
    ButtonStatusArray[0].long_release_callback = &NULLFunction;
    ButtonStatusArray[0].keep_press_callback = &NULLFunction;
    ButtonStatusArray[0].double_press_callback=&NULLFunction;
    ButtonStatusArray[0].LongPressThreshold = LONG_PRESS_TIME;  //������ʱ��
    ButtonStatusArray[0].short_press_counts = 0;
    ButtonStatusArray[0].short_press_interval = 0;
    CLRBIT(button_status,i);       
    ButtonStatusArray[0].type=DIGITAL_IO;
    ButtonStatusArray[0].pinStatus = GetButtonStatus((ButtonsNumber)i); //��¼״̬����
    ButtonStatusArray[0].pinLastStatus = ButtonStatusArray[i].pinStatus; //��¼�ϴ�����״̬
    ButtonStatusArray[0].StartEventDetect = 0;
	
	
    for(i = BTN_UP;i<BTN_DWN;i++)
    {	
        ButtonStatusArray[i].press_callback = &NULLFunction;
        ButtonStatusArray[i].short_release_callback = &NULLFunction;
        ButtonStatusArray[i].long_release_callback = &NULLFunction;
        ButtonStatusArray[i].keep_press_callback = &NULLFunction;
        ButtonStatusArray[i].double_press_callback=&NULLFunction;
        ButtonStatusArray[i].LongPressThreshold = LONG_PRESS_TIME;  //������ʱ��
        ButtonStatusArray[i].short_press_counts = 0;
        ButtonStatusArray[i].short_press_interval = 0;
        CLRBIT(button_status,i);     
        ButtonStatusArray[i].type=ANOLOG_IO;
        ButtonStatusArray[i].anolog_channal= ADC_CHANNEL_9;
        ButtonStatusArray[i].threshold=Threashold_value[i-BTN_UP];
        ButtonStatusArray[i].pinStatus = GetButtonStatus((ButtonsNumber)i); //��¼״̬����
        ButtonStatusArray[i].pinLastStatus = ButtonStatusArray[i].pinStatus; //��¼�ϴ�����״̬
        ButtonStatusArray[i].StartEventDetect = 0;
    }	
		
			
    for(i = BTN_DWN;i< BTN_TOTAL_NO;i++)
    {	
        ButtonStatusArray[i].press_callback = &NULLFunction;
        ButtonStatusArray[i].short_release_callback = &NULLFunction;
        ButtonStatusArray[i].long_release_callback = &NULLFunction;
        ButtonStatusArray[i].keep_press_callback = &NULLFunction;
        ButtonStatusArray[i].double_press_callback=&NULLFunction;
        ButtonStatusArray[i].LongPressThreshold = LONG_PRESS_TIME;  //������ʱ��
        ButtonStatusArray[i].short_press_counts = 0;
        ButtonStatusArray[i].short_press_interval = 0;
        CLRBIT(button_status,i);     
        ButtonStatusArray[i].type=ANOLOG_IO;
        ButtonStatusArray[i].anolog_channal=ADC_CHANNEL_8;
        ButtonStatusArray[i].threshold=Threashold_value[i-BTN_DWN];
        ButtonStatusArray[i].pinStatus = GetButtonStatus((ButtonsNumber)i); //��¼״̬����
        ButtonStatusArray[i].pinLastStatus = ButtonStatusArray[i].pinStatus; //��¼�ϴ�����״̬
        ButtonStatusArray[i].StartEventDetect = 0;
    }	
		

		//����ָ���ص�����	
    ButtonStatusArray[BTN_PWR].long_release_callback = &power_button_long_press;
    ButtonStatusArray[BTN_PWR].short_release_callback = &power_button_short_release;
    ButtonStatusArray[BTN_PWR].double_press_callback = &power_button_double_press;		
    ButtonStatusArray[BTN_UP].short_release_callback = &up_page_button_short_press;   //���·�ҳ�Ļص�����
    ButtonStatusArray[BTN_DWN].short_release_callback = &down_page_button_short_press;
		

}



static void ButtonEventProcess(void)
{
    uint8_t i = 0;
    for(i = 0;i<BTN_TOTAL_NO;i++)
    {
        CheckButtonEvent((ButtonsNumber)i);
    }			
}

static Buttonevents CheckButtonEvent(ButtonsNumber buttonNO)
{
    ButtonStatusArray[buttonNO].pinStatus = GetButtonStatus(buttonNO); //�õ���ǰ����״̬
    if((0 == ButtonStatusArray[buttonNO].StartEventDetect)
    &&(BUTTON_PRESSED == ButtonStatusArray[buttonNO].pinStatus))//��������� ����û�м�⿪ʼ
    {
        ButtonStatusArray[buttonNO].pinLastStatus = ButtonStatusArray[buttonNO].pinStatus ;  //����״̬
        return NO_BUTTON_EVENT;  //���¼�
    }

    if(BUTTON_PRESSED == ButtonStatusArray[buttonNO].pinStatus) //�����������
    {
        if(ButtonStatusArray[buttonNO].pinStatus != ButtonStatusArray[buttonNO].pinLastStatus) //����״̬�����仯
        {
            //  ButtonStatusArray[buttonNO].press_callback();    //�ص����»ص�����                    
            ButtonStatusArray[buttonNO].buttonPressedTimeslots = 0;  //���µ�ʱ������

        }
        else //���֮ǰҲ�ǰ��� ����Ҳ�ǰ���
        {

            ButtonStatusArray[buttonNO].buttonPressedTimeslots ++; //����ʱ���ۻ�
            if(ButtonStatusArray[buttonNO].LongPressThreshold < ButtonStatusArray[buttonNO].buttonPressedTimeslots) //������µ��ۼ�ʱ�䳬��200tick
            {
                // ButtonStatusArray[buttonNO].StartEventDetect = 0;
                //  ButtonStatusArray[buttonNO].pinLastStatus = ButtonStatusArray[buttonNO].pinStatus ;  //����֮ǰ�İ���״̬
                ButtonStatusArray[buttonNO].long_release_callback();     //�ص�����   
                ButtonStatusArray[buttonNO].StartEventDetect = 0;

                //   return LONG_PRESS;
            }         
            //         else
            //         {
            //           // ButtonStatusArray[buttonNO].keep_press_callback();      //���û�г���һ����ʱ�� �ص��������»ص�    �������ȡ����                
            //            return KEEP_PRESS;
            //         }
        }
    }
    else // ��������ͷ���
    {
        if(ButtonStatusArray[buttonNO].pinStatus != ButtonStatusArray[buttonNO].pinLastStatus) //����״̬�仯  ֮ǰ�ǰ��µ�
        {
            if((ButtonStatusArray[buttonNO].buttonPressedTimeslots < ButtonStatusArray[buttonNO].LongPressThreshold)  
            &&(ButtonStatusArray[buttonNO].buttonPressedTimeslots > SHORT_PRESS_TIME))// ����������µ�ʱ����ڶ̰���ʱ�� ��С�ڳ�����
            {
                ButtonStatusArray[buttonNO].buttonPressedTimeslots = 0;  //���µ�ʱ������
                // ButtonStatusArray[buttonNO].StartEventDetect = 0;        //�ر��¼����     
                // ButtonStatusArray[buttonNO].pinLastStatus = ButtonStatusArray[buttonNO].pinStatus ;  //�����ϴε�����״̬
                ButtonStatusArray[buttonNO].short_press_counts ++;  //�����¼��ۼ�
                ButtonStatusArray[buttonNO].short_press_interval = 0; //�̰�ʱ������
                if(ButtonStatusArray[buttonNO].short_press_counts > 1)  //�������1 ˵���ϴη�����һ��
                {
                    ButtonStatusArray[buttonNO].short_press_counts = 0; //�����ۼ�������
                    ButtonStatusArray[buttonNO].double_press_callback();  //�ص�˫���¼�       
                    // return DOUBLE_PRESS;
                }

                //ButtonStatusArray[buttonNO].short_release_callback();          
                //return SHORT_PRESS;
            } 

        }
        else  //����û�б仯 ֮ǰҲ���ͷŵ�
        {
            //start next button event detecting
            ButtonStatusArray[buttonNO].StartEventDetect = 1;  //��ʼ��һ�εļ��

            if(1 == ButtonStatusArray[buttonNO].short_press_counts)  //���֮ǰ��һ�ε���
            {
                ButtonStatusArray[buttonNO].short_press_interval ++ ;//ʱ���ۻ�
                if(ButtonStatusArray[buttonNO].short_press_interval >ButtonArray_double_interval[buttonNO])  //������������� ���������ĵ���
                {
                    ButtonStatusArray[buttonNO].short_press_counts = 0;
                    ButtonStatusArray[buttonNO].short_press_interval = 0;
                    ButtonStatusArray[buttonNO].short_release_callback();          
                    //return SHORT_PRESS;
                }
            }
        }
    }
    //  ButtonStatusArray[buttonNO].StartEventDetect = 1;  //��ʼ��һ�εļ��
    ButtonStatusArray[buttonNO].pinLastStatus = ButtonStatusArray[buttonNO].pinStatus ;
    return NO_BUTTON_EVENT;
}
