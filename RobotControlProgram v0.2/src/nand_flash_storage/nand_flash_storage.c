#include <asf.h>
#include <string.h>
#include "nand_flash_storage.h"

/* NAND Flash memory size. */
static uint32_t mem_size;
/* Number of blocks in NAND Flash. */
static uint32_t num_block;
/* Size of one block in the NAND Flash, in bytes. */
static uint32_t block_size;
/* Size of one page in the NAND Flash, in bytes. */
static uint32_t page_size;
/* Number of page per block. */
static uint32_t pages_per_block;

/* NAND Flash device structure. */
static struct nand_flash_raw nf_raw;
/* Address for transferring command to the NAND Flash. */
static uint32_t cmd_address = BOARD_NF_COMMAND_ADDR;
/* Address for transferring address to the NAND Flash. */
static uint32_t addr_address = BOARD_NF_ADDRESS_ADDR;
/* Address for transferring data to the NAND Flash. */
static uint32_t data_address = BOARD_NF_DATA_ADDR;

/* Temporary buffer */
uint8_t write_buffer[PAGE_SIZE];
uint8_t read_buffer[PAGE_SIZE];
uint8_t spare_area_buffer[LARGE_PAGE_SPARE_AREA];

uint16_t block, page;
uint32_t i;

uint32_t error;

/* Initialize external NAND Flash */
int8_t nand_flash_storage_init(void)
{
	int8_t ret = 0;
	block = 0;
	memset(&nf_raw, 0, sizeof(nf_raw));

	if (nand_flash_raw_initialize(&nf_raw, 0, cmd_address,
	addr_address, data_address)) {

		printf("-E- Device Unknown\n\r");
		ret = -1;
	} else {
		printf("-I- NAND Flash driver initialized\n\r");
	}

	/* Get device parameters */
	mem_size = nand_flash_model_get_device_size_in_bytes(&nf_raw.model);
	block_size = nand_flash_model_get_block_size_in_bytes(&nf_raw.model);
	num_block = nand_flash_model_get_device_size_in_blocks(&nf_raw.model);
	page_size = nand_flash_model_get_page_data_size(&nf_raw.model);
	pages_per_block = nand_flash_model_get_block_size_in_pages(&nf_raw.model);

	printf("-I- Size of the whole device in bytes : 0x%x \n\r", mem_size);
	printf("-I- Size in bytes of one single block of a device : 0x%x \n\r",	block_size);
	printf("-I- Number of blocks in the entire device : 0x%x \n\r",	num_block);
	printf("-I- Number of blocks used by the application : 0x%x \n\r", BLOCK_USAGE);
	printf("-I- Size of the data area of a page in bytes : 0x%x \n\r", page_size);
	printf("-I- Number of pages in the entire device : 0x%x \n\r", pages_per_block);
	
	// TODO: Bad block handling
	
	return ret;
}

/* Write a page to the first good block in the NAND flash */
int8_t nand_flash_storage_write(uint8_t* buf, uint8_t size)
{
	if(size > PAGE_SIZE)
		size = PAGE_SIZE;
	
	uint8_t ret = 0;
	error = -1;
	/* Prepare buffer in SRAM. */
	printf("-I- Preparing buffer in SRAM ...\n\r");
	write_buffer[0] = size;								//First byte is size of data block 
	memcpy(write_buffer+1,buf,size);

	/* Select a page. */
	page = 0;
	/* Iterate through blocks until a successful write is performed. Starting from block 0. */
	for (i = 0; i < BLOCK_USAGE; i++) {
		/* Erase the block in the NAND flash */
		error = nand_flash_raw_erase_block(&nf_raw, i);

		if (error == NAND_COMMON_ERROR_BADBLOCK) {
			printf("-E- Block %u is BAD block. \n\r", i);
			ret = -1;
		} else {
			/* Write a page to the NAND Flash. */
			printf("-I- Writing the buffer in page %d of block %d without ECC\n\r", page, i);
			error = nand_flash_raw_write_page(&nf_raw, i, page, write_buffer, 0);
			if (!error) {
				block = i;
				break;
			}
			printf("-E- Cannot write page %d of block %d. Trying next block...\n\r", page, i);
			ret = -1;
		}
	}
	return ret;
}

/* Read a page from the first good block in the NAND flash */
int16_t nand_flash_storage_read(uint8_t* buf)
{
	int16_t ret;
	error = 0;
	/* Read the data page from the NAND Flash. */
	page = 0;
	/* Iterate through blocks until a successful read is performed. Data is written to the first good block. */
	for (i = block; i < BLOCK_USAGE; i++) {
		/* Read a page from the NAND Flash. */
		printf("-I- Reading page %d of block %d\n\r", page, i);
		nand_flash_raw_read_page(&nf_raw, i, page, read_buffer, spare_area_buffer);
		if(spare_area_buffer[0] == 0xFF) {
			block = i;
			break;
		}
		error = -1;
		printf("-E- Cannot read page %d of block %d. Trying next block...\n\r", page, i);
	}
	
	if(error < 0) {
		printf("-E- Could not read any blocks\n\r");
		ret = error;
	} else {
		uint8_t data_size = read_buffer[0];
		memcpy(buf,read_buffer+1,data_size);
		ret = data_size;
	}
	return ret;
}

/* Erases all data in the NAND flash */
int8_t nand_flash_storage_erase(void)
{
	/* Erase all used blocks */
	for (i = 0; i < BLOCK_USAGE; i++) {
		error = nand_flash_raw_erase_block(&nf_raw, i);

		if (error == NAND_COMMON_ERROR_BADBLOCK) {
			printf("-E- Block %u is BAD block. \n\r", i);
		}
	}
}