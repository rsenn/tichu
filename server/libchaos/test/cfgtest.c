#include <libchaos/mem.h>
#include <libchaos/timer.h>
#include <libchaos/log.h>
#include <libchaos/io.h>
#include <libchaos/cfg.h>

int main()
{
  log_init(STDOUT_FILENO, LOG_ALL, L_status);
  io_init_except(STDOUT_FILENO, STDOUT_FILENO, STDOUT_FILENO);
  mem_init();
  dlink_init();
  timer_init();
  cfg_init();
  
  
  
  cfg_shutdown();
  timer_shutdown();
  dlink_shutdown();
  mem_shutdown();
  log_shutdown();
  io_shutdown();
  
  return 0;
}
 
