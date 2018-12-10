
	EXPORT  OS_CPU_SR_Save                                     
	EXPORT  OS_CPU_SR_Restore     

	PRESERVE8 
		
	AREA    |.text|, CODE, READONLY
	THUMB 
    

OS_CPU_SR_Save
    MRS     R0, PRIMASK  	
    CPSID   I				
    BX      LR			    

OS_CPU_SR_Restore
    MSR     PRIMASK, R0	   	
    BX      LR				
	end 