#include <Arduino.h>
#include "ff15/ff.h"
#include "ff15/diskio.h"
#include <hardware/flash.h>
#include <hardware/sync.h>

extern uint8_t _FS_start;
extern uint8_t _FS_end;
const size_t _FS_len = (uint32_t)&_FS_end - (uint32_t)&_FS_start + 1;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;
    if(pdrv != 0) {
        return STA_NOINIT;
    }
	return 0; /* OK */
}

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;
    if(pdrv != 0) {
    	return STA_NOINIT;
    }
	return 0; /* OK */
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;
    if(pdrv != 0) {
    	return RES_PARERR;
    }
    uint32_t target_addr = (uintptr_t)&_FS_start + (sector * FF_MAX_SS);
    Serial.println("Reading from sector " + String(sector) + " addr 0x" + String(target_addr, HEX) +  " count: " + String(count));
    // read from flash address directly
    memcpy(buff, (const void*)target_addr, count * FF_MAX_SS);
	return RES_OK;
}

/* No need for a division by FF_MAX_SS to calculate count for disk_read(): Make this available per-se */
void disk_read_direct (BYTE* buff, LBA_t sector, UINT num_bytes) {
    uint32_t target_addr = (uintptr_t)&_FS_start + (sector * FF_MAX_SS);
    memcpy(buff, (const void*)target_addr, num_bytes);
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;
    uint32_t target_offs = (uintptr_t)&_FS_start - (uintptr_t)XIP_BASE + (sector * FF_MAX_SS);
    Serial.println("Writing to sector " + String(sector) + " offset 0x" + String(target_offs, HEX) +  " count: " + String(count));
    Serial.flush();

    noInterrupts();
    rp2040.idleOtherCore();
    flash_range_erase(target_offs, count * FF_MAX_SS);
    flash_range_program(target_offs, buff, count * FF_MAX_SS);
    rp2040.resumeOtherCore();
    interrupts();
	return RES_OK;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;
    switch(cmd) {
        case CTRL_SYNC: 
            return RES_OK; /* nothing to do */
        case GET_SECTOR_COUNT: /* how many sectors we have */
            *(DWORD*)buff = _FS_len / FF_MAX_SS; 
            Serial.println("Returned sector count " + String(*(DWORD*)buff));
            return RES_OK;
        case GET_BLOCK_SIZE: /* Get erase block size in sectors */
            *(DWORD*)buff = 1; // We set FF_MAX_SS = 4096 so that one sector is one erasable block too
            return RES_OK;
        default:
            return RES_PARERR;
    }
}

/* Use standard C time functions to generate the FAT timestamp */
/* see http://elm-chan.org/fsw/ff/doc/fattime.html */
DWORD get_fattime (void) {
    time_t t;
    struct tm *stm;
    t = time(0);
    stm = localtime(&t);
    return (DWORD)(stm->tm_year - 80) << 25 |
           (DWORD)(stm->tm_mon + 1) << 21 |
           (DWORD)stm->tm_mday << 16 |
           (DWORD)stm->tm_hour << 11 |
           (DWORD)stm->tm_min << 5 |
           (DWORD)stm->tm_sec >> 1;
}
