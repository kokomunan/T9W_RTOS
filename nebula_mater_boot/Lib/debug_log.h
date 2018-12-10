#ifndef _DEBUG_LOG_H
#define _DEBUG_LOG_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#define  UserLog(...)       printf(__VA_ARGS__);
                           

#define debug_log(...)      printf("DEBUG->") ;\
                            printf(__VA_ARGS__);\
                            printf("\r\n");
														
#define err_log(...)        printf("ERROR->") ;\
                            printf(__VA_ARGS__);\
                            printf("\r\n");
 


#endif

