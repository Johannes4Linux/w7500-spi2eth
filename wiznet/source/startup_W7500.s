# 1 "wiznet/source/startup_W7500.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "wiznet/source/startup_W7500.S"
# 49 "wiznet/source/startup_W7500.S"
    .syntax unified
    .arch armv6-m

    .section .stack
    .align 3







    .section .stack
    .align 3



    .equ Stack_Size, 0x200

    .globl __StackTop
    .globl __StackLimit
__StackLimit:
    .space Stack_Size
    .size __StackLimit, . - __StackLimit
__StackTop:
    .size __StackTop, . - __StackTop
# 83 "wiznet/source/startup_W7500.S"
    .section .heap
    .align 3



    .equ Heap_Size, 0

    .globl __HeapBase
    .globl __HeapLimit
__HeapBase:
    .if Heap_Size
    .space Heap_Size
    .endif
    .size __HeapBase, . - __HeapBase
__HeapLimit:
    .size __HeapLimit, . - __HeapLimit




    .section .isr_vector
    .align 2
    .globl __isr_vector
__isr_vector:
    .long __StackTop
    .long Reset_Handler
    .long NMI_Handler
    .long HardFault_Handler
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long SVC_Handler
    .long 0
    .long 0
    .long PendSV_Handler
    .long SysTick_Handler


    .long SSP0_Handler
    .long SSP1_Handler
    .long UART0_Handler
    .long UART1_Handler
    .long UART2_Handler
    .long I2C0_Handler
    .long I2C1_Handler
    .long PORT0_Handler
    .long PORT1_Handler
    .long PORT2_Handler
    .long PORT3_Handler
    .long DMA_Handler
    .long DUALTIMER0_Handler
    .long DUALTIMER1_Handler
    .long PWM0_Handler
    .long PWM1_Handler
    .long PWM2_Handler
    .long PWM3_Handler
    .long PWM4_Handler
    .long PWM5_Handler
    .long PWM6_Handler
    .long PWM7_Handler
    .long RTC_Handler
    .long ADC_Handler
    .long WZTOE_Handler
    .long EXTI_Handler

    .size __isr_vector, . - __isr_vector

    .text
    .thumb
    .thumb_func
    .align 2
    .globl Reset_Handler
    .type Reset_Handler, %function
Reset_Handler:







    ldr r1, =__etext
    ldr r2, =__data_start__
    ldr r3, =__data_end__

    subs r3, r2
    ble .LC1
.LC0:
    subs r3, #4
    ldr r0, [r1, r3]
    str r0, [r2, r3]
    bgt .LC0
.LC1:
# 205 "wiznet/source/startup_W7500.S"
    bl main

    .pool
    .size Reset_Handler, . - Reset_Handler




    .macro def_default_handler handler_name
    .align 1
    .thumb_func
    .weak \handler_name
    .type \handler_name, %function
\handler_name :
    b .
    .size \handler_name, . - \handler_name
    .endm



    def_default_handler NMI_Handler
    def_default_handler HardFault_Handler
    def_default_handler MemManage_Handler
    def_default_handler BusFault_Handler
    def_default_handler UsageFault_Handler
    def_default_handler SVC_Handler
    def_default_handler DebugMon_Handler
    def_default_handler PendSV_Handler
    def_default_handler SysTick_Handler



    def_default_handler SSP0_Handler
    def_default_handler SSP1_Handler
    def_default_handler UART0_Handler
    def_default_handler UART1_Handler
    def_default_handler UART2_Handler
    def_default_handler I2C0_Handler
    def_default_handler I2C1_Handler
    def_default_handler PORT0_Handler
    def_default_handler PORT1_Handler
    def_default_handler PORT2_Handler
    def_default_handler PORT3_Handler

    def_default_handler DMA_Handler
    def_default_handler DUALTIMER0_Handler
    def_default_handler DUALTIMER1_Handler
    def_default_handler PWM0_Handler
    def_default_handler PWM1_Handler
    def_default_handler PWM2_Handler
    def_default_handler PWM3_Handler
    def_default_handler PWM4_Handler
    def_default_handler PWM5_Handler
    def_default_handler PWM6_Handler
    def_default_handler PWM7_Handler
    def_default_handler RTC_Handler
    def_default_handler ADC_Handler
    def_default_handler WZTOE_Handler
    def_default_handler EXTI_Handler







    .end
