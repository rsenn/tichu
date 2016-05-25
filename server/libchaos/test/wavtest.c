#include <libchaos/io.h>
#include <libchaos/mem.h>
#include <libchaos/log.h>
#include <libchaos/wav.h>

int wavtest_log;

void wavtest()
{
  struct wav *wav;
  int i;
  
  wav = wav_new("sawtooth", WAV_16BIT, WAV_MONO, 44100);
  
  for(i = -65536; i < 65536; i++)
    wav_sample_putmono(wav, i);
  
  wav_save(wav);
  wav_delete(wav);
}

int main()
{
  log_init(STDOUT_FILENO, LOG_ALL, L_status);
  wavtest_log = log_source_register("wavtest");
  io_init_except(STDOUT_FILENO, STDOUT_FILENO, STDOUT_FILENO);
  mem_init();
  dlink_init();
  wav_init();
  
  wavtest();
  
  wav_shutdown();
  dlink_shutdown();
  mem_shutdown();
  log_shutdown();
  io_shutdown();
  
  return 0;
}
 
