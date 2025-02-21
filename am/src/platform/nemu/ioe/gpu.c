#include <am.h>
#include <nemu.h>
#include <klib.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

static uint32_t width, height;

void __am_gpu_init() {
  uint32_t size = inl(VGACTL_ADDR);
  width = size >> 16;
  height = size & 0xffff;
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t size = inl(VGACTL_ADDR);
  width = size >> 16;
  height = size & 0xffff;
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = width, .height = height,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  for (int i = 0; i < h; ++i) {
    for (int j = 0; j < w; ++j) {
      int di = y + i, dj = x + j;
      if (di >= height || dj >= width) continue;
      uint32_t pixel = ((uint32_t*)ctl->pixels)[i * w + j];
      outl(FB_ADDR + (di * width + dj) * sizeof(uint32_t), pixel);
    }
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
