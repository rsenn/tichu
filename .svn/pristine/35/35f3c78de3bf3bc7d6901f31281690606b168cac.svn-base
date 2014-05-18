#include <libchaos/io.h>
#include <libchaos/mem.h>
#include <libchaos/timer.h>
#include <libchaos/log.h>
#include <libchaos/gif.h>
#include <libchaos/image.h>
#include <libchaos/graph.h>

uint32_t bytes_in = 666;
uint32_t bytes_out = 666;

void graphtest_write(void)
{
  struct graph *graph;
  
  graph = graph_new("traffic", 400, 160, GRAPH_TYPE_LINE);

/*  graph_colorize(graph, GRAPH_COLOR_DARK);
  
  graph_source_add(graph, GRAPH_MEASURE_DIFFTIME,
                   GRAPH_SOURCE_UINT, &bytes_in, "in");
  
  graph_source_add(graph, GRAPH_MEASURE_DIFFTIME, 
                   GRAPH_SOURCE_UINT, &bytes_out, "out");

  graph_drain_add(graph, GRAPH_DATA_HOURLY);
  graph_drain_add(graph, GRAPH_DATA_DAILY);
  graph_drain_add(graph, GRAPH_DATA_WEEKLY);
  graph_drain_add(graph, GRAPH_DATA_MONTHLY);
  
  graph_drain_render(graph, GRAPH_DATA_HOURLY);
  graph_drain_save(graph, GRAPH_DATA_HOURLY);
  */
  graph_delete(graph);
}

int main()
{
  log_init(STDOUT_FILENO, LOG_ALL, L_status);
  io_init_except(STDOUT_FILENO, STDOUT_FILENO, STDOUT_FILENO);
  mem_init();
  dlink_init();
  gif_init();
  image_init();
  timer_init();
  graph_init();
  
  graphtest_write();

  graph_shutdown();
  timer_shutdown();
  image_shutdown();
  gif_shutdown();
  dlink_shutdown();
  mem_shutdown();
  log_shutdown();
  io_shutdown();
  
  return 0;
}
 
