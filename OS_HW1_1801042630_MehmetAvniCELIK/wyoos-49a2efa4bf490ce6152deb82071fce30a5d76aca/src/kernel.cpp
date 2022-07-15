
#include <common/types.h>
#include <gdt.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <gui/desktop.h>
#include <gui/window.h>
#include <multitasking.h>


// #define GRAPHICSMODE


using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;

#define FALSE 0         //for petersons algorithm
#define TRUE 1          //for petersons algorithm
#define N 2             //for petersons algorithm
int turn;               //for petersons algorithm
int interested[N];      //for petersons algorithm

int total=0;            //for prıducer-consumer problem
int productBuffer[1024]; //for producer-consumer problem

//-----------------PETERSON ALGORITHM FOR SCHEDULING-----------------------------------//
void enter_region(int process){
    int other = 1-process;
    interested[process] = TRUE;
    turn = process;
    while(turn == process && interested[other] == TRUE );
}

void leave_region(int process){
    interested[process] = FALSE;
}
//--------------------------------------------------------------------------//


void printf(char* str)
{
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    static uint8_t x=0,y=0;

    for(int i = 0; str[i] != '\0'; ++i)
    {
        switch(str[i])
        {
            case '\n':
                x = 0;
                y++;
                break;
            default:
                VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | str[i];
                x++;
                break;
        }

        if(x >= 80)
        {
            x = 0;
            y++;
        }

        if(y >= 25)
        {
            for(y = 0; y < 25; y++)
                for(x = 0; x < 80; x++)
                    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';
            x = 0;
            y = 0;
        }
    }
}

void printfHex(uint8_t key)
{
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf(foo);
}

class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
public:
    void OnKeyDown(char c)
    {
        char* foo = " ";
        foo[0] = c;
        printf(foo);
    }
};

class MouseToConsole : public MouseEventHandler
{
    int8_t x, y;
public:
    
    MouseToConsole()
    {
        uint16_t* VideoMemory = (uint16_t*)0xb8000;
        x = 40;
        y = 12;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);        
    }
    
    virtual void OnMouseMove(int xoffset, int yoffset)
    {
        static uint16_t* VideoMemory = (uint16_t*)0xb8000;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);

        x += xoffset;
        if(x >= 80) x = 79;
        if(x < 0) x = 0;
        y += yoffset;
        if(y >= 25) y = 24;
        if(y < 0) y = 0;

        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);
    }
    
};

void producer(){
    while(true){
        if(total<1024){
            enter_region(0);
            productBuffer[total++]=1;
            printfHex(productBuffer[total]);
            printf("\n");
            leave_region(0);
        }
    }
}
void consumer(){
    while(true){
        if(total>0){
            enter_region(1);
            productBuffer[total--]=0;
            printfHex(productBuffer[total]);
            printf("\n");
            leave_region(1);
        }
    }
}


//I have more than 2 threads to tests easily.
//________________________________________________________
void taskA()
{    
    while(true){
        enter_region(0);
        printf("A");
        leave_region(0);
    }

}
void taskB()
{
    while(true){
        enter_region(1);
        printf("B");
        leave_region(1);
    }
}
void taskC()
{
    while(true)
        printf("C");
}
void taskD()
{
    while(true)
        printf("Z");
}
void taskPetersonA()
{
    while(true){
        enter_region(0);
        printf("PetersonA");
        leave_region(0);
    }
}
void taskPetersonB()
{
    while(true){
        enter_region(1);
        printf("PetersonB");
        leave_region(1);
    }
}






typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}



extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{
    printf("Hello World! --- http://www.AlgorithMan.de\n");

    GlobalDescriptorTable gdt;
    
    TaskManager taskManager;
    Task thread1(&gdt, taskA);
    Task thread2(&gdt, taskB);
    Task thread3(&gdt, taskC);
    Task thread4(&gdt, taskD);
    Task threadP1(&gdt, taskPetersonA);
    Task threadP2(&gdt, taskPetersonB);
    Task thread_producer(&gdt, producer);
    Task thread_consumer(&gdt, consumer);


//-----------------UNIT TESTING FOR JOIN THREAD-----------------------------------//
//    taskManager.CreateThread(&thread1);          //thread A will be created.
//    taskManager.CreateThread(&thread2);          //thread B will be created.
//    taskManager.CreateThread(&thread3);          //thread C will be created. if you also comment this out only in this unit test, you can see peterson algorithm is working as well.
//    taskManager.JoinThread(&thread1,0);          //thread1.join will be called.
//    taskManager.CreateThread(&thread4);          //thread D will be called but not created due to joinThread(&thread1)
//--------------------------------------------------------------------------//

//-----------------UNIT TESTING FOR YIELD THREAD-----------------------------------//
//    taskManager.CreateThread(&thread1);          //thread A will be created.
//    taskManager.CreateThread(&thread2);          //thread B will be created.
//    taskManager.CreateThread(&thread3);          //thread C will be created.
//    taskManager.CreateThread(&thread4);          //thread D will be created.
//    taskManager.YieldThread(&thread3);          //thread C will be sent to the end.
//--------------------------------------------------------------------------//

//-----------------UNIT TESTING FOR PETERSON ALGORITHM-----------------------------------//
//    taskManager.CreateThread(&threadP1);          //thread A will be created.
//    taskManager.CreateThread(&threadP2);          //thread B will be created.
//--------------------------------------------------------------------------//


//-----------------UNIT TESTING FOR PRODUCER-CONSUMER-----------------------------------//
    taskManager.CreateThread(&thread_producer);         //producing
    taskManager.CreateThread(&thread_consumer);         //consuming    
//--------------------------------------------------------------------------//

    InterruptManager interrupts(0x20, &gdt, &taskManager);
    
    printf("Initializing Hardware, Stage 1\n");
    
    #ifdef GRAPHICSMODE
        Desktop desktop(320,200, 0x00,0x00,0xA8);
    #endif
    
    DriverManager drvManager;
    
        #ifdef GRAPHICSMODE
            KeyboardDriver keyboard(&interrupts, &desktop);
        #else
            PrintfKeyboardEventHandler kbhandler;
            KeyboardDriver keyboard(&interrupts, &kbhandler);
        #endif
        drvManager.AddDriver(&keyboard);
        
    
        #ifdef GRAPHICSMODE
            MouseDriver mouse(&interrupts, &desktop);
        #else
            MouseToConsole mousehandler;
            MouseDriver mouse(&interrupts, &mousehandler);
        #endif
        drvManager.AddDriver(&mouse);
        
        PeripheralComponentInterconnectController PCIController;
        PCIController.SelectDrivers(&drvManager, &interrupts);

        VideoGraphicsArray vga;
        
    printf("Initializing Hardware, Stage 2\n");
        drvManager.ActivateAll();
        
    printf("Initializing Hardware, Stage 3\n");

    #ifdef GRAPHICSMODE
        vga.SetMode(320,200,8);
        Window win1(&desktop, 10,10,20,20, 0xA8,0x00,0x00);
        desktop.AddChild(&win1);
        Window win2(&desktop, 40,15,30,30, 0x00,0xA8,0x00);
        desktop.AddChild(&win2);
    #endif


    interrupts.Activate();
    
    while(1)
    {
        #ifdef GRAPHICSMODE
            desktop.Draw(&vga);
        #endif
    }
}
