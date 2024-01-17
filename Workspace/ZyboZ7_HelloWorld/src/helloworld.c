/*
 * Copyright (c) 2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#define MM2S_CONTROL_REGISTER       0x00
#define MM2S_STATUS_REGISTER        0x04
#define MM2S_SRC_ADDRESS_REGISTER   0x18
#define MM2S_TRNSFR_LENGTH_REGISTER 0x28

#define S2MM_CONTROL_REGISTER       0x30
#define S2MM_STATUS_REGISTER        0x34
#define S2MM_DST_ADDRESS_REGISTER   0x48
#define S2MM_BUFF_LENGTH_REGISTER   0x58

#define IOC_IRQ_FLAG                1<<12
#define IDLE_FLAG                   1<<1

#define STATUS_HALTED               0x00000001
#define STATUS_IDLE                 0x00000002
#define STATUS_SG_INCLDED           0x00000008
#define STATUS_DMA_INTERNAL_ERR     0x00000010
#define STATUS_DMA_SLAVE_ERR        0x00000020
#define STATUS_DMA_DECODE_ERR       0x00000040
#define STATUS_SG_INTERNAL_ERR      0x00000100
#define STATUS_SG_SLAVE_ERR         0x00000200
#define STATUS_SG_DECODE_ERR        0x00000400
#define STATUS_IOC_IRQ              0x00001000
#define STATUS_DELAY_IRQ            0x00002000
#define STATUS_ERR_IRQ              0x00004000

#define HALT_DMA                    0x00000000
#define RUN_DMA                     0x00000001
#define RESET_DMA                   0x00000004
#define ENABLE_IOC_IRQ              0x00001000
#define ENABLE_DELAY_IRQ            0x00002000
#define ENABLE_ERR_IRQ              0x00004000
#define ENABLE_ALL_IRQ              0x00007000

#define AXI_LITE_CTRL_IF_REG_ADDR	0x40400000
#define AXI_DMA_MM2S_SRC_ADDR		0x0E000000
#define AXI_DMA_S2MM_DST_ADDR		0x0F000000

#define DMA_MM2S_TRANSFER_SIZE		65536
#define DMA_S2MM_TRANSFER_SIZE		65536

unsigned int write_dma(unsigned int *virtual_addr, int offset, unsigned int value)
{
    virtual_addr[offset>>2] = value;

    return 0;
}

unsigned int read_dma(unsigned int *virtual_addr, int offset)
{
    return virtual_addr[offset>>2];
}

void dma_s2mm_status(unsigned int *virtual_addr)
{
    unsigned int status = read_dma(virtual_addr, S2MM_STATUS_REGISTER);

    printf("Stream to memory-mapped status (0x%08x@0x%02x):", status, S2MM_STATUS_REGISTER);

    if (status & STATUS_HALTED) {
		printf(" Halted.\n");
	} else {
		printf(" Running.\n");
	}

    if (status & STATUS_IDLE) {
		printf(" Idle.\n");
	}

    if (status & STATUS_SG_INCLDED) {
		printf(" SG is included.\n");
	}

    if (status & STATUS_DMA_INTERNAL_ERR) {
		printf(" DMA internal error.\n");
	}

    if (status & STATUS_DMA_SLAVE_ERR) {
		printf(" DMA slave error.\n");
	}

    if (status & STATUS_DMA_DECODE_ERR) {
		printf(" DMA decode error.\n");
	}

    if (status & STATUS_SG_INTERNAL_ERR) {
		printf(" SG internal error.\n");
	}

    if (status & STATUS_SG_SLAVE_ERR) {
		printf(" SG slave error.\n");
	}

    if (status & STATUS_SG_DECODE_ERR) {
		printf(" SG decode error.\n");
	}

    if (status & STATUS_IOC_IRQ) {
		printf(" IOC interrupt occurred.\n");
	}

    if (status & STATUS_DELAY_IRQ) {
		printf(" Interrupt on delay occurred.\n");
	}

    if (status & STATUS_ERR_IRQ) {
		printf(" Error interrupt occurred.\n");
	}
}

void dma_mm2s_status(unsigned int *virtual_addr)
{
    unsigned int status = read_dma(virtual_addr, MM2S_STATUS_REGISTER);

    printf("Memory-mapped to stream status (0x%08x@0x%02x):", status, MM2S_STATUS_REGISTER);

    if (status & STATUS_HALTED) {
		printf(" Halted.\n");
	} else {
		printf(" Running.\n");
	}

    if (status & STATUS_IDLE) {
		printf(" Idle.\n");
	}

    if (status & STATUS_SG_INCLDED) {
		printf(" SG is included.\n");
	}

    if (status & STATUS_DMA_INTERNAL_ERR) {
		printf(" DMA internal error.\n");
	}

    if (status & STATUS_DMA_SLAVE_ERR) {
		printf(" DMA slave error.\n");
	}

    if (status & STATUS_DMA_DECODE_ERR) {
		printf(" DMA decode error.\n");
	}

    if (status & STATUS_SG_INTERNAL_ERR) {
		printf(" SG internal error.\n");
	}

    if (status & STATUS_SG_SLAVE_ERR) {
		printf(" SG slave error.\n");
	}

    if (status & STATUS_SG_DECODE_ERR) {
		printf(" SG decode error.\n");
	}

    if (status & STATUS_IOC_IRQ) {
		printf(" IOC interrupt occurred.\n");
	}

    if (status & STATUS_DELAY_IRQ) {
		printf(" Interrupt on delay occurred.\n");
	}

    if (status & STATUS_ERR_IRQ) {
		printf(" Error interrupt occurred.\n");
	}
}

int dma_mm2s_sync(unsigned int *virtual_addr)
{
    unsigned int mm2s_status =  read_dma(virtual_addr, MM2S_STATUS_REGISTER);

	// sit in this while loop as long as the status does not read back 0x00001002 (4098)
	// 0x00001002 = IOC interrupt has occured and DMA is idle
	while(!(mm2s_status & IOC_IRQ_FLAG) || !(mm2s_status & IDLE_FLAG))
	{
        dma_s2mm_status(virtual_addr);
        dma_mm2s_status(virtual_addr);

        mm2s_status =  read_dma(virtual_addr, MM2S_STATUS_REGISTER);
    }

	return 0;
}

int dma_s2mm_sync(unsigned int *virtual_addr)
{
    unsigned int s2mm_status = read_dma(virtual_addr, S2MM_STATUS_REGISTER);

	// sit in this while loop as long as the status does not read back 0x00001002 (4098)
	// 0x00001002 = IOC interrupt has occured and DMA is idle
	while(!(s2mm_status & IOC_IRQ_FLAG) || !(s2mm_status & IDLE_FLAG))
	{
        dma_s2mm_status(virtual_addr);
        dma_mm2s_status(virtual_addr);

        s2mm_status = read_dma(virtual_addr, S2MM_STATUS_REGISTER);
    }

	return 0;
}

void print_mem(void *virtual_address, int byte_count)
{
	char *data_ptr = virtual_address;

	for(int i=0;i<byte_count;i++){
		printf("%02X", data_ptr[i]);

		// print a space every 4 bytes (0 indexed)
		if(i%4==3){
			printf(" ");
		}
	}

	printf("\n");
}

//void load_mem(void *virtual_address, int byte_count, unsigned int data)
//{
//	char *data_ptr = data;
//
//	for(int i=0;i<byte_count;i++){
//		data_ptr[i] = data;
//	}
//
//	memset(virtual_address, *data_ptr, byte_count);
//}

struct timespec diff_timespec(const struct timespec *time1,
    const struct timespec *time0) {
  assert(time1);
  assert(time0);
  struct timespec diff = {.tv_sec = time1->tv_sec - time0->tv_sec, //
      .tv_nsec = time1->tv_nsec - time0->tv_nsec};
  if (diff.tv_nsec < 0) {
    diff.tv_nsec += 1000000000; // nsec/sec
    diff.tv_sec--;
  }
  return diff;
}

int main()
{
    printf("Hello World! - Running DMA transfer test application.\n");

	printf("Opening a character device file of the Arty's DDR memeory...\n");
	int ddr_memory = open("/dev/mem", O_RDWR | O_SYNC);

	printf("Memory map the address of the DMA AXI IP via its AXI lite control interface register block.\n");
    unsigned int *dma_virtual_addr = mmap(NULL, 65535, PROT_READ | PROT_WRITE, MAP_SHARED, ddr_memory, AXI_LITE_CTRL_IF_REG_ADDR);

	printf("Memory map the MM2S source address register block.\n");
    unsigned int *virtual_src_addr  = mmap(NULL, 65535, PROT_READ | PROT_WRITE, MAP_SHARED, ddr_memory, AXI_DMA_MM2S_SRC_ADDR);

	printf("Memory map the S2MM destination address register block.\n");
    unsigned int *virtual_dst_addr = mmap(NULL, 65535, PROT_READ | PROT_WRITE, MAP_SHARED, ddr_memory, AXI_DMA_S2MM_DST_ADDR);

	printf("Writing random data to source register block...\n");
	virtual_src_addr[0]= 0xEFBEADDE;
	virtual_src_addr[1]= 0x11223344;
	virtual_src_addr[2]= 0xABABABAB;
	virtual_src_addr[3]= 0xCDCDCDCD;
	virtual_src_addr[4]= 0x00001111;
	virtual_src_addr[5]= 0x22223333;
	virtual_src_addr[6]= 0x44445555;
	virtual_src_addr[7]= 0x66667777;

	printf("Clearing the destination register block...\n");
    memset(virtual_dst_addr, 0, 32);

    printf("Source memory block data:      ");
	print_mem(virtual_src_addr, 32);

    printf("Destination memory block data: ");
	print_mem(virtual_dst_addr, 32);

    printf("Reset the DMA.\n");
    write_dma(dma_virtual_addr, S2MM_CONTROL_REGISTER, RESET_DMA);
    write_dma(dma_virtual_addr, MM2S_CONTROL_REGISTER, RESET_DMA);
    dma_s2mm_status(dma_virtual_addr);
    dma_mm2s_status(dma_virtual_addr);

	printf("Halt the DMA.\n");
    write_dma(dma_virtual_addr, S2MM_CONTROL_REGISTER, HALT_DMA);
    write_dma(dma_virtual_addr, MM2S_CONTROL_REGISTER, HALT_DMA);
    dma_s2mm_status(dma_virtual_addr);
    dma_mm2s_status(dma_virtual_addr);

	printf("Enable all interrupts.\n");
    write_dma(dma_virtual_addr, S2MM_CONTROL_REGISTER, ENABLE_ALL_IRQ);
    write_dma(dma_virtual_addr, MM2S_CONTROL_REGISTER, ENABLE_ALL_IRQ);
    dma_s2mm_status(dma_virtual_addr);
    dma_mm2s_status(dma_virtual_addr);

    printf("Writing source address of the data from MM2S in DDR...\n");
    write_dma(dma_virtual_addr, MM2S_SRC_ADDRESS_REGISTER, AXI_DMA_MM2S_SRC_ADDR);
    dma_mm2s_status(dma_virtual_addr);

    printf("Writing the destination address for the data from S2MM in DDR...\n");
    write_dma(dma_virtual_addr, S2MM_DST_ADDRESS_REGISTER, AXI_DMA_S2MM_DST_ADDR);
    dma_s2mm_status(dma_virtual_addr);

	printf("Run the MM2S channel.\n");
    write_dma(dma_virtual_addr, MM2S_CONTROL_REGISTER, RUN_DMA);
    dma_mm2s_status(dma_virtual_addr);

	printf("Run the S2MM channel.\n");
    write_dma(dma_virtual_addr, S2MM_CONTROL_REGISTER, RUN_DMA);
    dma_s2mm_status(dma_virtual_addr);

    printf("Writing MM2S transfer length of %d bytes...\n", DMA_MM2S_TRANSFER_SIZE);
    write_dma(dma_virtual_addr, MM2S_TRNSFR_LENGTH_REGISTER, DMA_MM2S_TRANSFER_SIZE);
    dma_mm2s_status(dma_virtual_addr);

    printf("Writing S2MM transfer length of %d bytes...\n", DMA_S2MM_TRANSFER_SIZE);
    write_dma(dma_virtual_addr, S2MM_BUFF_LENGTH_REGISTER, DMA_S2MM_TRANSFER_SIZE);
    dma_s2mm_status(dma_virtual_addr);

    printf("Waiting for MM2S synchronization...\n");
    dma_mm2s_sync(dma_virtual_addr);

    printf("Waiting for S2MM synchronization...\n");
    dma_s2mm_sync(dma_virtual_addr);

    dma_s2mm_status(dma_virtual_addr);
    dma_mm2s_status(dma_virtual_addr);

    printf("Destination memory block: ");
	print_mem(virtual_dst_addr, 32);

	{
		struct timespec ts_start, ts_end, ts_diff;
		long diff_sec, diff_nsec;
		timespec_get(&ts_start, TIME_UTC);
		sleep(1);
		timespec_get(&ts_end, TIME_UTC);
		printf("%ld %09ld\n", ts_start.tv_sec, ts_start.tv_nsec);
		printf("%ld %09ld\n", ts_end.tv_sec, ts_end.tv_nsec);
		ts_diff = diff_timespec(&ts_end, &ts_start);
		printf("%ld.%09ld", ts_diff.tv_sec, ts_diff.tv_nsec);
	}

	printf("\n");

    return 0;
}
