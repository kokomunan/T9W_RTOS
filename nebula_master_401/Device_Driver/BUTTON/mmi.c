#include "mmi.h"
#include "stdlib.h"
extern ADC_HandleTypeDef hadc1;

#define  ADC_CONTROLER   &hadc1
#define RAGE_VALUE     200


#define BTN_CHK_VAL_POWER 	            (1<<0)
#define BUTTON_PRESSED                  1
#define BUTTON_RELEASED                 0
#define SHORT_PRESS_TIME				5
#define DOUBLE_PRESS_INTERVAL           50
#define LONG_PRESS_TIME				    200
#define VERY_LONG_PRESS_TIME		    300
#define BUTTON_PRESSED                  1
#define SETBIT(BYTE,BIT)                ( (BYTE) |= (uint16_t)((uint16_t)1<<(uint16_t)(BIT)) )
#define CLRBIT(BYTE,BIT)                ( (BYTE) &= (uint16_t)((uint16_t)((uint16_t)1<<(uint16_t)(BIT))^(uint16_t)0xffff) )

const uint32_t ButtonArray_polar[BTN_TOTAL_NO]={1,1,1,1,1,1,1,1,1,1,1}; //按键极性
const uint32_t ButtonArray_double_interval[BTN_TOTAL_NO]={DOUBLE_PRESS_INTERVAL,DOUBLE_PRESS_INTERVAL,DOUBLE_PRESS_INTERVAL,DOUBLE_PRESS_INTERVAL,DOUBLE_PRESS_INTERVAL,DOUBLE_PRESS_INTERVAL,DOUBLE_PRESS_INTERVAL
                                                          ,DOUBLE_PRESS_INTERVAL,DOUBLE_PRESS_INTERVAL,DOUBLE_PRESS_INTERVAL,DOUBLE_PRESS_INTERVAL};

static PinStatus ButtonStatusArray[BTN_TOTAL_NO] = {0};
static uint16_t Threashold_value[3]={983,2048,3358};
static uint16_t button_status = 0;

uint8_t GetButtonStatus(ButtonsNumber buttonNO)//return a false when button pressed
{
	return (((1<<buttonNO) & button_status)!=0);
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


//读取按键状态
uint8_t read_button(uint8_t num)
{
	uint16_t value;
	if(ButtonStatusArray[num].type==DIGITAL_IO)
	{	
        
        if(num==BTN_DET)
        {
           return HAL_GPIO_ReadPin(BUTTON_DET_GPIO_Port, BUTTON_DET_Pin);  
        }           
        else
        {
            return 0;
        }
        
		     
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

void ButtonProcessLoop(void)  //要测量整个过程的时间
{
    uint8_t BIndex = 0;
    
    for(BIndex = 0;BIndex<BTN_TOTAL_NO;BIndex++)
    {
        if(ButtonArray_polar[BIndex] == read_button(BIndex))
        {
            SETBIT(button_status,BIndex);  //如果按下了 设置位
       
        }	
        else
        {
            CLRBIT(button_status,BIndex);   //如果没有按下就清除位
   				
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
    ButtonStatusArray[0].LongPressThreshold = LONG_PRESS_TIME;  //长按的时间
    ButtonStatusArray[0].short_press_counts = 0;
    ButtonStatusArray[0].short_press_interval = 0;
    CLRBIT(button_status,i);       
    ButtonStatusArray[0].type=DIGITAL_IO;
    ButtonStatusArray[0].pinStatus = GetButtonStatus((ButtonsNumber)i); //记录状态引脚
    ButtonStatusArray[0].pinLastStatus = ButtonStatusArray[i].pinStatus; //记录上次引脚状态
    ButtonStatusArray[0].StartEventDetect = 0;
	

    for(i = BTN_1;i< BTN_4;i++)
    {	
        ButtonStatusArray[i].press_callback = &NULLFunction;
        ButtonStatusArray[i].short_release_callback = &NULLFunction;
        ButtonStatusArray[i].long_release_callback = &NULLFunction;
        ButtonStatusArray[i].keep_press_callback = &NULLFunction;
        ButtonStatusArray[i].double_press_callback=&NULLFunction;
        ButtonStatusArray[i].LongPressThreshold = LONG_PRESS_TIME;  //长按的时间
        ButtonStatusArray[i].short_press_counts = 0;
        ButtonStatusArray[i].short_press_interval = 0;
        CLRBIT(button_status,i);     
        ButtonStatusArray[i].type=ANOLOG_IO;
        ButtonStatusArray[i].anolog_channal=ADC_CHANNEL_4;
        ButtonStatusArray[i].threshold=Threashold_value[i-BTN_1];
        ButtonStatusArray[i].pinStatus = GetButtonStatus((ButtonsNumber)i); //记录状态引脚
        ButtonStatusArray[i].pinLastStatus = ButtonStatusArray[i].pinStatus; //记录上次引脚状态
        ButtonStatusArray[i].StartEventDetect = 0;
    }
    for(i = BTN_4;i< BTN_6;i++)
    {	
        ButtonStatusArray[i].press_callback = &NULLFunction;
        ButtonStatusArray[i].short_release_callback = &NULLFunction;
        ButtonStatusArray[i].long_release_callback = &NULLFunction;
        ButtonStatusArray[i].keep_press_callback = &NULLFunction;
        ButtonStatusArray[i].double_press_callback=&NULLFunction;
        ButtonStatusArray[i].LongPressThreshold = LONG_PRESS_TIME;  //长按的时间
        ButtonStatusArray[i].short_press_counts = 0;
        ButtonStatusArray[i].short_press_interval = 0;
        CLRBIT(button_status,i);     
        ButtonStatusArray[i].type=ANOLOG_IO;
        ButtonStatusArray[i].anolog_channal=ADC_CHANNEL_5;
        ButtonStatusArray[i].threshold=Threashold_value[i-BTN_4];
        ButtonStatusArray[i].pinStatus = GetButtonStatus((ButtonsNumber)i); //记录状态引脚
        ButtonStatusArray[i].pinLastStatus = ButtonStatusArray[i].pinStatus; //记录上次引脚状态
        ButtonStatusArray[i].StartEventDetect = 0;
    }
    for(i = BTN_6;i< BTN_9;i++)
    {	
        ButtonStatusArray[i].press_callback = &NULLFunction;
        ButtonStatusArray[i].short_release_callback = &NULLFunction;
        ButtonStatusArray[i].long_release_callback = &NULLFunction;
        ButtonStatusArray[i].keep_press_callback = &NULLFunction;
        ButtonStatusArray[i].double_press_callback=&NULLFunction;
        ButtonStatusArray[i].LongPressThreshold = LONG_PRESS_TIME;  //长按的时间
        ButtonStatusArray[i].short_press_counts = 0;
        ButtonStatusArray[i].short_press_interval = 0;
        CLRBIT(button_status,i);     
        ButtonStatusArray[i].type=ANOLOG_IO;
        ButtonStatusArray[i].anolog_channal=ADC_CHANNEL_6;
        ButtonStatusArray[i].threshold=Threashold_value[i-BTN_6];
        ButtonStatusArray[i].pinStatus = GetButtonStatus((ButtonsNumber)i); //记录状态引脚
        ButtonStatusArray[i].pinLastStatus = ButtonStatusArray[i].pinStatus; //记录上次引脚状态
        ButtonStatusArray[i].StartEventDetect = 0;
    }
    for(i = BTN_9;i< BTN_TOTAL_NO;i++)
    {	
        ButtonStatusArray[i].press_callback = &NULLFunction;
        ButtonStatusArray[i].short_release_callback = &NULLFunction;
        ButtonStatusArray[i].long_release_callback = &NULLFunction;
        ButtonStatusArray[i].keep_press_callback = &NULLFunction;
        ButtonStatusArray[i].double_press_callback=&NULLFunction;
        ButtonStatusArray[i].LongPressThreshold = LONG_PRESS_TIME;  //长按的时间
        ButtonStatusArray[i].short_press_counts = 0;
        ButtonStatusArray[i].short_press_interval = 0;
        CLRBIT(button_status,i);     
        ButtonStatusArray[i].type=ANOLOG_IO;
        ButtonStatusArray[i].anolog_channal=ADC_CHANNEL_7;
        ButtonStatusArray[i].threshold=Threashold_value[i-BTN_9];
        ButtonStatusArray[i].pinStatus = GetButtonStatus((ButtonsNumber)i); //记录状态引脚
        ButtonStatusArray[i].pinLastStatus = ButtonStatusArray[i].pinStatus; //记录上次引脚状态
        ButtonStatusArray[i].StartEventDetect = 0;
    }
		

//		//重新指定回调函数	

    ButtonStatusArray[BTN_DET].short_release_callback = &button_det_short_release;
    ButtonStatusArray[BTN_DET].double_press_callback = &button_det_double_press;		
    ButtonStatusArray[BTN_DET].long_release_callback=&button_det_long_press;
    ButtonStatusArray[BTN_1].short_release_callback= &button1_short_press;
    ButtonStatusArray[BTN_2].short_release_callback= &button2_short_press;
    ButtonStatusArray[BTN_3].short_release_callback= &button3_short_press;
    ButtonStatusArray[BTN_4].short_release_callback= &button4_short_press;
	ButtonStatusArray[BTN_5].short_release_callback= &button5_short_press;
    ButtonStatusArray[BTN_6].short_release_callback= &button6_short_press;
    ButtonStatusArray[BTN_7].short_release_callback= &button7_short_press;
    ButtonStatusArray[BTN_8].short_release_callback= &button8_short_press;
    ButtonStatusArray[BTN_9].short_release_callback= &button9_short_press;
    ButtonStatusArray[BTN_10].short_release_callback= &button10_short_press;

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
    ButtonStatusArray[buttonNO].pinStatus = GetButtonStatus(buttonNO); //得到当前引脚状态
    if((0 == ButtonStatusArray[buttonNO].StartEventDetect)
    &&(BUTTON_PRESSED == ButtonStatusArray[buttonNO].pinStatus))//如果按下了 并且没有检测开始
    {
        ButtonStatusArray[buttonNO].pinLastStatus = ButtonStatusArray[buttonNO].pinStatus ;  //更新状态
        return NO_BUTTON_EVENT;  //无事件
    }

    if(BUTTON_PRESSED == ButtonStatusArray[buttonNO].pinStatus) //如果按键按下
    {
        if(ButtonStatusArray[buttonNO].pinStatus != ButtonStatusArray[buttonNO].pinLastStatus) //按键状态发生变化
        {                 
            ButtonStatusArray[buttonNO].buttonPressedTimeslots = 0;  //按下的时间清零

        }
        else //如果之前也是按下 现在也是按下
        {

            ButtonStatusArray[buttonNO].buttonPressedTimeslots ++; //持续时间累积
            if(ButtonStatusArray[buttonNO].LongPressThreshold < ButtonStatusArray[buttonNO].buttonPressedTimeslots) //如果按下的累计时间超过200tick
            {

                ButtonStatusArray[buttonNO].long_release_callback();     //回调长按   
                ButtonStatusArray[buttonNO].StartEventDetect = 0;
   
            }         

        }
    }
    else // 如果按键释放了
    {
        if(ButtonStatusArray[buttonNO].pinStatus != ButtonStatusArray[buttonNO].pinLastStatus) //按键状态变化  之前是按下的
        {
            if((ButtonStatusArray[buttonNO].buttonPressedTimeslots < ButtonStatusArray[buttonNO].LongPressThreshold)  
            &&(ButtonStatusArray[buttonNO].buttonPressedTimeslots > SHORT_PRESS_TIME))// 如果案件按下的时间大于短按的时间 但小于长按的
            {
                ButtonStatusArray[buttonNO].buttonPressedTimeslots = 0;  //按下的时间清零
                ButtonStatusArray[buttonNO].short_press_counts ++;  //按下事件累加
                ButtonStatusArray[buttonNO].short_press_interval = 0; //短按时间清零
                if(ButtonStatusArray[buttonNO].short_press_counts > 1)  //如果大于1 说明上次发生过一次
                {
                    ButtonStatusArray[buttonNO].short_press_counts = 0; //单机累计书清零
                    ButtonStatusArray[buttonNO].double_press_callback();  //回调双击事件       
                
                }

            } 

        }
        else  //按键没有变化 之前也是释放的
        {
            //start next button event detecting
            ButtonStatusArray[buttonNO].StartEventDetect = 1;  //开始下一次的检测

            if(1 == ButtonStatusArray[buttonNO].short_press_counts)  //如果之前是一次单机
            {
                ButtonStatusArray[buttonNO].short_press_interval ++ ;//时间累积
                if(ButtonStatusArray[buttonNO].short_press_interval >ButtonArray_double_interval[buttonNO])  //如果超过窗口期 当做单纯的单击
                {
                    ButtonStatusArray[buttonNO].short_press_counts = 0;
                    ButtonStatusArray[buttonNO].short_press_interval = 0;
                    ButtonStatusArray[buttonNO].short_release_callback();          
                    //return SHORT_PRESS;
                }
            }
        }
    }
    ButtonStatusArray[buttonNO].pinLastStatus = ButtonStatusArray[buttonNO].pinStatus ;
    return NO_BUTTON_EVENT;
}
