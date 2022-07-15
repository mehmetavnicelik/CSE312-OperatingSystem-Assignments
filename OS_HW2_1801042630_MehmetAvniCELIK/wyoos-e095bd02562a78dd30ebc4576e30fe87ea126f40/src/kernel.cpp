#include <common/types.h>
#include <gdt.h>
#include <memorymanagement.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <gui/desktop.h>
#include <gui/window.h>
#include <multitasking.h>


#define MY_VIRTUAL_MEMORY_SIZE 8
#define MY_ARRAY_SIZE 10
#define MY_DISK_SIZE 16


int virtualSize=0;
int diskSize=0;
int hit=0;
int miss=0;
int pagesLoaded=0;
int back2disk=0;
int diskIndex=0;
// #define GRAPHICSMODE


using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;

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

class Page{
    public:
    int data;
    int reference;
    bool modified;
    int size=0;
    void setData(int i){
        data=i;
    };
    int getData(){
        return data;
    }

};

int myArray[MY_ARRAY_SIZE]={5,9,6,0,3,2,1,7,4,8};
Page myVirtualMemory[MY_VIRTUAL_MEMORY_SIZE];
Page myDisk[MY_DISK_SIZE];

int virtualMemoryFiller(){      //fills virtual memory and then disk.
    diskIndex=MY_ARRAY_SIZE-MY_VIRTUAL_MEMORY_SIZE;
    for(int i=0;i<MY_VIRTUAL_MEMORY_SIZE;i++){
        myVirtualMemory[i].setData(myArray[i]);
        virtualSize++;
    }
    for(int i=0;i<diskIndex;i++){
        myDisk[i].setData(myArray[diskIndex+i]);
        diskSize++;
    }
}
bool check_is_in_virtual_mem(int x){
    for(int i=0;i<MY_VIRTUAL_MEMORY_SIZE;i++){
        if(myVirtualMemory[i].data==x){
            return true;
        }
        else{
            return false;
        }
    }
}

void FIFO(int x){
    int temp= myVirtualMemory[0].data;
    for(int j=1;j<MY_VIRTUAL_MEMORY_SIZE;j++){ 
        myVirtualMemory[j-1].setData(myVirtualMemory[j].data);
    }
    for(int i=0;i<MY_DISK_SIZE;i++){
        if(myDisk[i].data==x){
            myVirtualMemory[MY_VIRTUAL_MEMORY_SIZE-1].setData(myDisk[i].data);
            myDisk[diskIndex].setData(temp);
            diskIndex++;        
        }
    }
}

void swap(int* xp, int* yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}


// A function to implement bubble sort
void bubbleSort(int arr[], int n)
{
    int i, j;
    for (i = 0; i < n - 1; i++) 
        for (j = 0; j < n - i - 1; j++){
            if(check_is_in_virtual_mem(arr[j])==true){
                hit++;
            }
            else{
                FIFO(myArray[arr[j]]);
                miss++;
            }
            if(check_is_in_virtual_mem(arr[j+1])==true){
                hit++;
            }
            else{
                FIFO(myArray[arr[j+1]]);
                miss++;
            }
            if (arr[j] > arr[j + 1]){
                swap(&arr[j], &arr[j + 1]);
            }
        }
}
/* Function to sort an array using insertion sort*/
void insertionSort(int arr[], int n)
{
    int i, key, j;
    for (i = 1; i < n; i++) {
        if(check_is_in_virtual_mem(arr[i])==true){
            hit++;
        }
        else{
            FIFO(myArray[arr[i]]);
            miss++;
        }
        key = arr[i];
        j = i - 1;
        while (j >= 0 && arr[j] > key) {
            if(check_is_in_virtual_mem(arr[j])==true){
                hit++;
            }
            else{
                miss++;
            }
            if(check_is_in_virtual_mem(arr[j+1])==true){
                hit++;
            }
            else{
                miss++;
            }
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

int partition (int arr[], int low, int high)
{
    if(check_is_in_virtual_mem(arr[high])==true){
        hit++;
    }
    else{
        miss++;
    }
    int pivot = arr[high]; // pivot
    int i = (low - 1); // Index of smaller element and indicates the right position of pivot found so far
 
    for (int j = low; j <= high - 1; j++)
    {
        if(check_is_in_virtual_mem(arr[i])==true){
            hit++;
        }
        else{
            miss++;
        }
        if(check_is_in_virtual_mem(arr[j])==true){
            hit++;
        }
        else{
            miss++;
        }
        
        // If current element is smaller than the pivot
        if (arr[j] < pivot)
        {
            i++; // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    if(check_is_in_virtual_mem(arr[i])==true){
        hit++;
    }
    else{
        miss++;
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}
 
/* The main function that implements QuickSort
arr[] --> Array to be sorted,
low --> Starting index,
high --> Ending index */
void quickSort(int arr[], int low, int high)
{
    if (low < high)
    {
        /* pi is partitioning index, arr[p] is now
        at right place */
        int pi = partition(arr, low, high);
 
        // Separately sort elements before
        // partition and after partition
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
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
void taskA()
{
    while(true)
        printf("A");
}
void taskB()
{
    while(true)
        printf("B");
}
typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

void printArray(int myArray[],int size){
    for(int i=0;i<size;i++){
        printfHex(myArray[i]);
        printf("  ");
    }
    printf("\n");
}
extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{
    virtualMemoryFiller();
    printArray(myArray,MY_ARRAY_SIZE);
    printf("\n");
    //bubbleSort(myArray,MY_ARRAY_SIZE);
    //insertionSort(myArray,MY_ARRAY_SIZE);
    quickSort(myArray,0,MY_ARRAY_SIZE-1);
    printf("\n");
    printArray(myArray,MY_ARRAY_SIZE);
    printf("---");
    printf("Hit Number: ");
    printfHex(hit);
    printf("---");
    printf("Miss Number: ");
    printfHex(miss);
    printf("---");
    printf("Pages Loaded: ");
    printfHex(miss);
    printf("---");
    printf("Pages Written Back: ");
    printfHex(miss);
    printf("---\n");

    GlobalDescriptorTable gdt;
    uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
    size_t heap = 10*1024*1024;
    MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);
    
    printf("heap: 0x");
    printfHex((heap >> 24) & 0xFF);
    printfHex((heap >> 16) & 0xFF);
    printfHex((heap >> 8 ) & 0xFF);
    printfHex((heap      ) & 0xFF);
    
    void* allocated = memoryManager.malloc(1024);
    printf("\nallocated: 0x");
    printfHex(((size_t)allocated >> 24) & 0xFF);
    printfHex(((size_t)allocated >> 16) & 0xFF);
    printfHex(((size_t)allocated >> 8 ) & 0xFF);
    printfHex(((size_t)allocated      ) & 0xFF);
    printf("\n");
    
    TaskManager taskManager;
    /*
    Task task1(&gdt, taskA);
    Task task2(&gdt, taskB);
    taskManager.AddTask(&task1);
    taskManager.AddTask(&task2);
    */
    
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
