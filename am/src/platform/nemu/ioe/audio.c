#include <am.h>
#include <nemu.h>
#include <klib.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)

static int bufsize = 0;
static uint32_t count = 0;
static uint32_t offset = 0;

void __am_audio_init() {
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
  assert(bufsize >= 4);
  cfg->bufsize = bufsize;
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  outl(AUDIO_INIT_ADDR, 1);
  count = 0;
  offset = 0;
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  count = inl(AUDIO_COUNT_ADDR);
  stat->count = count;
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  int len = ctl->buf.end - ctl->buf.start;
  uint8_t *pdata = ctl->buf.start;
  int nwrite = 0;
  while (nwrite < len) {
    count = inl(AUDIO_COUNT_ADDR);
    int remain = bufsize - count;
    int n = len - nwrite;
    if (n > remain) n = remain;
    for (int i = 0; i < n; ) {
      if (i + 4 <= n && offset + 4 < bufsize) {
        outl(AUDIO_SBUF_ADDR + offset, *(uint32_t*)pdata);
        offset += 4;
        pdata += 4;
        i += 4;
      } else {
        outb(AUDIO_SBUF_ADDR + offset, *pdata);
        if (++offset >= bufsize) offset -= bufsize;
        ++pdata;
        ++i;
      }
    }
    nwrite += n;
    count = inl(AUDIO_COUNT_ADDR);
    count += n;
    outl(AUDIO_COUNT_ADDR, count);
  }
}
