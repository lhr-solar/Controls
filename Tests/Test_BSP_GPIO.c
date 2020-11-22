#include "common.h"
#include "config.h"
#include <bsp.h>

int readFromPIO(char* pn){
    int data = -2;
    if(!strcmp(pn, "PA0")){
        data = BSP_GPIO_Read(PA0);
    }
    else if(!strcmp(pn, "PA2")){
        data = BSP_GPIO_Read(PA2);
    }
    else if(!strcmp(pn, "PA3")){
        data = BSP_GPIO_Read(PA3);
    }
    else if(!strcmp(pn, "PB0")){
        data = BSP_GPIO_Read(PB0);
    }
    else if(!strcmp(pn, "PB1")){
        data = BSP_GPIO_Read(PB1);
    }
    else if(!strcmp(pn, "PB2")){
        data = BSP_GPIO_Read(PB2);
    }
    else if(!strcmp(pn, "PB4")){
        data = BSP_GPIO_Read(PB4);
    }
    else if(!strcmp(pn, "PB5")){
        data = BSP_GPIO_Read(PB5);
    }
    else if(!strcmp(pn, "PB6")){
        data = BSP_GPIO_Read(PB6);
    }
    else if(!strcmp(pn, "PB7")){
        data = BSP_GPIO_Read(PB7);
    }
    else if(!strcmp(pn, "PB8")){
        data = BSP_GPIO_Read(PB8);
    }
    else if(!strcmp(pn, "PB14")){
        data = BSP_GPIO_Read(PB14);
    }
    else if(!strcmp(pn, "PB15")){
        data = BSP_GPIO_Read(PB15);
    }
    else{
        printf("pin not found\n");
    } 
    if(data != -2){
        printf("Data: %d\n", data);
    }

}

void writeToGPIO(char* pn){
    int message;
    printf("Enter Message: ");
    scanf("%d", &message);

    if(!strcmp(pn, "PA0")){
        BSP_GPIO_Write(PA0, message);
    }
    else if(!strcmp(pn, "PA2")){
        BSP_GPIO_Write(PA2, message);
    }
    else if(!strcmp(pn, "PA3")){
        BSP_GPIO_Write(PA3, message);
    }
    else if (!strcmp(pn, "PB0")){
        BSP_GPIO_Write(PB0, message);
    }
    else if (!strcmp(pn, "PB1")){
        BSP_GPIO_Write(PB1, message);
    }
    else if (!strcmp(pn, "PB2")){
        BSP_GPIO_Write(PB2, message);
    }
    else if (!strcmp(pn, "PB4")){
        BSP_GPIO_Write(PB4, message);
    }
    else if (!strcmp(pn, "PB5")){
        BSP_GPIO_Write(PB5, message);
    }
    else if (!strcmp(pn, "PB6")){
        BSP_GPIO_Write(PB6, message);
    }
    else if (!strcmp(pn, "PB7")){
        BSP_GPIO_Write(PB7, message);
    }
    else if (!strcmp(pn, "PB8")){
        BSP_GPIO_Write(PB8, message);
    }
    else if (!strcmp(pn, "PB14")){
        BSP_GPIO_Write(PB14, message);
    }
    else if (!strcmp(pn, "PB15")){
        BSP_GPIO_Write(PB15, message);
    }
    else{
        printf("pin not found \n");
    } 
}


int main(){

    BSP_GPIO_Init();

    BSP_GPIO_Write(PA0, 69);

    BSP_GPIO_Read(PA0);

    char pin[5] = {0};
    char rorw[6] = {0};
    int status = 0;

    while(0){
        printf("Choose Pin:");
        gets(pin);

        printf("Choose read or write:");
        gets(rorw);

        if(!strcmp("READ", rorw))
        {
            readFromPIO(pin);
        }
        else if(!strcmp("WRITE", rorw))
        {
             writeToGPIO(pin);
        }
        else
        {
            printf("Read or write not recognized\n");
        }
        

    }

}