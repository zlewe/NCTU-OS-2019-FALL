#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "os_hw4_driver.h"
#include <math.h>
#define ENTRY(x,a,b) ((x&bitmask(a,b+1)) >> a)	//use with bitmask for bit manipulation

int fd;

//bitmask
static inline uint64_t bitmask(uint64_t a, uint64_t b){
	return ((1ull << (b - a)) - 1ull) << a;
}

//get entries from virtual address and return as array
void get_entries(uint64_t add, uint64_t arr[])
{
	arr[0] = ENTRY(add,0,11);
	arr[1] = ENTRY(add,12,20);
	arr[2] = ENTRY(add,21,29);
	arr[3] = ENTRY(add,30,38);
	arr[4] = ENTRY(add,39,47);

	return;	
}

// Obtain my cr3 value (a.k.a. PML4 table physical address)
uint64_t get_cr3_value()
{
	struct ioctl_arg cmd;
	int ret;
	cmd.request[0] = IO_CR3;
	ret = ioctl(fd, IOCTL_REQUEST, &cmd);
	return cmd.ret;
}

// Given a physical address, return the value
uint64_t read_physical_address(uint64_t physical_address)
{
	struct ioctl_arg cmd;
	int ret;
	cmd.request[0] = IO_READ;
	cmd.request[1] = physical_address;
	ret = ioctl(fd, IOCTL_REQUEST, &cmd);
	return cmd.ret;
}

// Write value to a physical address
void write_physical_address(uint64_t physical_address, uint64_t value)
{
	struct ioctl_arg cmd;
	int ret;
	cmd.request[0] = IO_WRITE;
	cmd.request[1] = physical_address;
	cmd.request[2] = value;
	ret = ioctl(fd, IOCTL_REQUEST, &cmd);
}

//given virtual address, translate to physical address
uint64_t addr_trans(uint64_t vadd){
	uint64_t arr[5] = {0};
	get_entries(vadd, arr);

	uint64_t base = get_cr3_value();
	base = (ENTRY(base, 12, 51)<<12) + arr[4]*8;	//addr of PML4E
    base = read_physical_address(base);				//content of PML4E
    base = (ENTRY(base, 12, 51)<<12) + arr[3]*8;	//addr of PDPE
    base = read_physical_address(base);				//content of PDPE
    base = (ENTRY(base, 12, 51)<<12) + arr[2]*8;	//addr of PDE
    base = read_physical_address(base);				//content of PDE
    base = (ENTRY(base, 12, 51)<<12) + arr[1]*8;	//addr of PTE
	
	return base;
}

int main()
{
	char *x = (char*)aligned_alloc(4096, 4096) + 0x123;
	char *y = (char*)aligned_alloc(4096, 4096) + 0x123;
	strcpy(x, "This is OS homework 4.");
	strcpy(y, "You have to modify my page table.");

	fd = open("/dev/os", O_RDONLY);
	if(fd < 0) 
	{
		printf("Cannot open device!\n");
		return 0;
	}

	printf("Before\n");
	printf("x : %s\n", x);
	printf("y : %s\n", y);

	/* TODO 1 */
	// ------------------------------------------------
	// Modify page table entry of y
	// Let y point to x's physical address
	uint64_t vir_x = (uint64_t)x, vir_y = (uint64_t)y;

	uint64_t pte_addr_x = addr_trans(vir_x), pte_addr_y = addr_trans(vir_y);	

    uint64_t pte_val_x = read_physical_address(pte_addr_x);	//get page table entry of x
	uint64_t pte_val_y = read_physical_address(pte_addr_y);	//get page table entry of y

    write_physical_address(pte_addr_y, pte_val_x);
	// ------------------------------------------------

	getchar();

	printf("After modifying page table\n");
	printf("x : %s\n", x);
	printf("y : %s\n", y);

	getchar();

	strcpy(y, "When you modify y, x is modified actually.");
	printf("After modifying string y\n");
	printf("x : %s\n", x);
	printf("y : %s\n", y);

	/* TODO 2 */
	// ------------------------------------------------
	// Recover page table entry of y
	// Let y point to its original address
	// You may need to store y's original address at previous step
	write_physical_address(pte_addr_y, pte_val_y);
	// ------------------------------------------------

	getchar();

	printf("After recovering page table of y\n");
	printf("x : %s\n", x);
	printf("y : %s\n", y);

	close(fd);
}
