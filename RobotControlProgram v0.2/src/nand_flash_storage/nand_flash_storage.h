#ifndef NAND_FLASH_STORAGE_H_
#define NAND_FLASH_STORAGE_H_

#define LARGE_PAGE_SPARE_AREA 64
#define PAGE_SIZE 2048
#define BLOCK_COUNT	2048
#define BLOCK_USAGE 10

int8_t nand_flash_storage_init(void);
int8_t nand_flash_storage_write(uint8_t* buf, uint8_t size);
int16_t nand_flash_storage_read(uint8_t* buf);
int8_t nand_flash_storage_erase(void);


#endif /* NAND_FLASH_STORAGE_H_ */