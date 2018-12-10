

#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "oled.h"
#include "string.h"
#include "oled_font.h"
//#include "oled_icon.h"
#include "string.h"
#include "stdio.h"
#include "debug_log.h"


extern SPI_HandleTypeDef hspi3;
#define  SPI_CONTROLER    &hspi3
#define   OLED_DEBUG            debug_log
#define   OLED_ERROR            err_log

 
uint8_t OLED_GRAM[128][4];	   //定义这么大是应为要把oled 当成两部分来看



void SPI_Write_Command(uint8_t cmd)
{


    HAL_GPIO_WritePin(OLED_D_C_GPIO_Port, OLED_D_C_Pin, GPIO_PIN_RESET);
    OLED_SELECT();		
    HAL_SPI_Transmit(SPI_CONTROLER, &cmd, 1, 10);
    OLED_RELEASE();
    HAL_GPIO_WritePin(OLED_D_C_GPIO_Port, OLED_D_C_Pin,  GPIO_PIN_SET);
	
}
void SPI_Write_Data(uint8_t *data,uint16_t lenth)
{

    HAL_GPIO_WritePin(OLED_D_C_GPIO_Port, OLED_D_C_Pin,  GPIO_PIN_SET);
    OLED_SELECT();  
    HAL_SPI_Transmit(SPI_CONTROLER, data, lenth, 10);
    OLED_RELEASE();
    HAL_GPIO_WritePin(OLED_D_C_GPIO_Port, OLED_D_C_Pin, GPIO_PIN_RESET);
	
}


void OLED_Off_Init(void) //关闭oled
{
	
    OLED_Display_Off();//g关闭显示
    HAL_Delay(20);
    OLED_VDD_OFF();
    HAL_Delay(10);
    OLED_VBAT_OFF(); //关闭vbat 电源 
    HAL_Delay(200);

	
}

void OLED_On_Test_Mode(void)
{
	
    OLED_VBAT_ON() ;
    HAL_Delay(200);	
    OLED_DEBUG("clear oled");
    CLR_GDDRAM();	
    OLED_Display_On();
	
}



void CLR_GDDRAM(void)  //清屏 
{
                       
    OLED_CLEAR_BUFF();
    SPI_Write_Command(SET_MEM_ADDR_MODE_CMD);  //设置地址模式
    SPI_Write_Command(VERTICAL_ADDR_MODE_CMD);  //纵向
    SPI_Write_Command(SET_COLUMN_ADDR_CMD); //设置纵向起始和终止地址
    SPI_Write_Command(2);  //起始地址
    SPI_Write_Command(129);  //结束地址
    SPI_Write_Command(SET_PAGE_ADDR_CMD); //设置页地址
    SPI_Write_Command(0); //起始                        
    SPI_Write_Command(7); //结束
    SPI_Write_Data(&OLED_GRAM[0][0],512);   //一共是1K字节ram 需要写两边
    SPI_Write_Data(&OLED_GRAM[0][0],512);  	
		
}


void OLED_Display_ON_PUMP(void)
{

    SPI_Write_Command(0x8d); 
    SPI_Write_Command(0x14);
}

void OLED_Display_OFF_PUMP(void)
{

    SPI_Write_Command(0x8d);
    SPI_Write_Command(0x10);
}


void OLED_Display_On(void)
{
	SPI_Write_Command(DISPLAY_ON_CMD);  //打开显示
}
void OLED_Display_Off(void)
{
	SPI_Write_Command(DISPLAY_OFF_CMD);  //关闭显示
}

void OLED_CLEAR_BUFF(void)
{
	for(uint16_t i=0;i<128;i++)
    {
        for(uint16_t j=0;j<4;j++)
        OLED_GRAM[i][j]=0x00;
    }
}

void OLED_Refresh_bank(uint8_t bank)    //将缓存刷新到oled上
{
	uint8_t H16_GRAM[128][2],i,j;
	
	SPI_Write_Command(SET_MEM_ADDR_MODE_CMD);  //设置地址模式
	SPI_Write_Command(VERTICAL_ADDR_MODE_CMD);  //纵向
	SPI_Write_Command(SET_COLUMN_ADDR_CMD); //设置纵向起始和终止地址
	SPI_Write_Command(2);  //起始地址
	SPI_Write_Command(129);  //结束地址
	SPI_Write_Command(SET_PAGE_ADDR_CMD); //设置页地址
	if(bank==0)
	{
        SPI_Write_Command(6); //起始                        
        SPI_Write_Command(7); //结束
        for(i=0;i<128;i++)
        {
            for(j=0;j<2;j++)
            H16_GRAM[i][j]=OLED_GRAM[i][j];
        }			
        SPI_Write_Data(&H16_GRAM[0][0],256);  	

	}
	else if(bank==1)
	{
		SPI_Write_Command(2); //起始                        
		SPI_Write_Command(5); //结束
		SPI_Write_Data(&OLED_GRAM[0][0],512);  
	}
	else
	{
		SPI_Write_Command(0); //起始                        
		SPI_Write_Command(1); //结束
		
		for(i=0;i<128;i++)
		{
            for(j=0;j<2;j++)
            H16_GRAM[i][j]=OLED_GRAM[i][j];
		}
			
		SPI_Write_Data(&H16_GRAM[0][0],256);  
		
	}
	
}
//将图片载入到显存中
void OLED_Draw_BMP(uint8_t *Matrix_Buffer,uint8_t Start_Adr,uint8_t Use_Col_Num)
{
	uint16_t i ,j;
	i=127-(Start_Adr+Use_Col_Num);
	for(j=0;j<Use_Col_Num;j++)
	{
		OLED_GRAM[i][0]=*Matrix_Buffer;
		OLED_GRAM[i][1]=*(Matrix_Buffer+1);
		Matrix_Buffer+=2;
        i++;
	}
	
	
}
		   
void OLED_DrawPoint(uint8_t x,uint8_t y)
{
	uint8_t pos,bx,temp=0;
	if(x>127||y>31)
    {
        return;//超出范围了.
    }
	pos=y/8;	
	bx=y%8;
	temp=1<<(bx);
	OLED_GRAM[127-x][pos]|=temp;
    
}
void OLED_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)
    {
        incx=1; //设置单步方向 
    }
	else if(delta_x==0)
    {
        incx=0;//垂直线 
    }
	else
    {
        incx=-1;
        delta_x=-delta_x;
    } 
    
    if(delta_y>0)
    {
        incy=1; 
    }
    else if(delta_y==0)
    {
        incy=0;//水平线 
    }
    else
    {
        incy=-1;
        delta_y=-delta_y;
    } 
    
    if( delta_x>delta_y)
    {
        distance=delta_x; //选取基本增量坐标轴 
    }
    else 
    {
        distance=delta_y; 
    }
    for(t=0;t<=distance+1;t++ )//画线输出 
    {  
        OLED_DrawPoint(uRow,uCol);//画点 
        xerr+=delta_x ; 
        yerr+=delta_y ; 
        if(xerr>distance) 
        { 
            xerr-=distance; 
            uRow+=incx; 
        } 
        if(yerr>distance) 
        { 
            yerr-=distance; 
            uCol+=incy; 
        } 
    }  
}    

void  OLED_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	OLED_DrawLine(x1,y1,x2,y1);
	OLED_DrawLine(x1,y1,x1,y2);
    OLED_DrawLine(x1,y2,x2,y2);
	OLED_DrawLine(x2,y1,x2,y2);
}
 
void OLED_Fill(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2)  
{  
	uint8_t x,y;  
	for(x=x1;x<=x2;x++)
	{
		for(y=y1;y<=y2;y++)OLED_DrawPoint(x,y);
	}													    

}

void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size)
{      			    
    uint8_t temp,t,t1;
    uint8_t y0=y;
    uint8_t csize=(size/8+((size%8)?1:0))*(size/2);		//得到字体一个字符对应点阵集所占的字节数
    chr=chr-' ';//得到偏移后的值		 
    for(t=0;t<csize;t++)
    {   
        if(size==16)
        {
            temp=ASCII_Matrix[chr][t];	//调用1608字体
        }
        else 
        {
            return;								//没有的字库
        }
        for(t1=0;t1<8;t1++)
        {
            if(temp&0x01)
            {
                OLED_DrawPoint(x,y);
            }

            temp>>=1;
            y++;
            if((y-y0)==size)
            {
                y=y0;
                x++;
                break;
            }
        }  	 
    }          
}


void OLED_ShowString(uint8_t x,uint8_t y,const uint8_t *p,uint8_t size)
{	
    while((*p<='~')&&(*p>=' '))//判断是不是非法字符!
    {       
        if(x>(128-8))
        {
            return;
        }
        OLED_ShowChar(x,y,*p,16);	 
        x+=8;
        p++;
    }  
	
}	

void DRAW_1_Area(uint8_t *Matrix_Buffer,uint8_t Start_Adr,uint8_t Use_Col_Num)  
{
	SPI_Write_Command(SET_MEM_ADDR_MODE_CMD);
	SPI_Write_Command(VERTICAL_ADDR_MODE_CMD);	
	SPI_Write_Command(SET_COLUMN_ADDR_CMD);
	SPI_Write_Command(129-(Start_Adr+Use_Col_Num));
	SPI_Write_Command(129-Start_Adr);
	SPI_Write_Command(SET_PAGE_ADDR_CMD);  //设置页地址  前两页
	SPI_Write_Command(6);
	SPI_Write_Command(7);
	SPI_Write_Data(Matrix_Buffer,Use_Col_Num*2); // use 2 page to display ,so it need to *2
}
void DRAW_2_Area(uint8_t *Matrix_Buffer,uint8_t Start_Adr,uint8_t Use_Col_Num)  
{
	SPI_Write_Command(SET_MEM_ADDR_MODE_CMD);
	SPI_Write_Command(VERTICAL_ADDR_MODE_CMD);	
	SPI_Write_Command(SET_COLUMN_ADDR_CMD);
	SPI_Write_Command(129-(Start_Adr+Use_Col_Num));
	SPI_Write_Command(129-Start_Adr);
	SPI_Write_Command(SET_PAGE_ADDR_CMD);  //设置页地址  前两页
	SPI_Write_Command(2);
	SPI_Write_Command(5);
	SPI_Write_Data(Matrix_Buffer,Use_Col_Num*4); // use 2 page to display ,so it need to *2
}

void OLED_On_Init(void)  //打开oled
{

	OLED_VBAT_ON() ;
	HAL_Delay(200);	
	OLED_DEBUG("clear oled");
	CLR_GDDRAM();
	OLED_DEBUG("draw main menu");	
	OLED_CLEAR_BUFF(); //清除缓存
	
}



