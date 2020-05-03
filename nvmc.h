#ifndef NVMC_H
#define NVMC_H
#define FLASH_PAGE_SIZE (4096)
int flash_write(uint32_t address, const uint8_t *data, int len);
int flash_erase(uint32_t address, int len);
#endif
