#include <am.h>
#include <nemu.h>

#define DISK_PRESENT_ADDR   (DISK_ADDR + 0x00)
#define DISK_BLKSZ_ADDR     (DISK_ADDR + 0x04)
#define DISK_BLKCNT_ADDR    (DISK_ADDR + 0x08)
#define DISK_IO_BUF_ADDR    (DISK_ADDR + 0x0c)
#define DISK_IO_BLKNO_ADDR  (DISK_ADDR + 0x10)
#define DISK_IO_BLKCNT_ADDR (DISK_ADDR + 0x14)
#define DISK_IO_CMD_ADDR    (DISK_ADDR + 0x18)

void __am_disk_config(AM_DISK_CONFIG_T *cfg) {
  cfg->present = inl(DISK_PRESENT_ADDR);
  cfg->blksz = inl(DISK_BLKSZ_ADDR);
  cfg->blkcnt = inl(DISK_BLKCNT_ADDR);
}

void __am_disk_status(AM_DISK_STATUS_T *stat) {
  stat->ready = true;
}

void __am_disk_blkio(AM_DISK_BLKIO_T *io) {
  outl(DISK_IO_BUF_ADDR, (uint32_t)io->buf);
  outl(DISK_IO_BLKNO_ADDR, io->blkno);
  outl(DISK_IO_BLKCNT_ADDR, io->blkcnt);
  outl(DISK_IO_CMD_ADDR, io->write ? 2 : 1);
}
