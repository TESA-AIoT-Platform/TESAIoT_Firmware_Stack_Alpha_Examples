/*******************************************************************************
 * File Name        : cm55_stdout_ipc.c
 *
 * Description      : _write() for CM55 stdout/stderr that sends output via
 *                    IPC_CMD_PRINT to CM33 for display on UART. Chunks data
 *                    to fit IPC_DATA_MAX_LEN. Drops output if pipe not ready.
 *
 *******************************************************************************/

#include "cm55_ipc_pipe.h"
#include "ipc_communication.h"
#include "task.h"
#include <stddef.h>

#define STDOUT_FD (1)
#define STDERR_FD (2)
#define CHUNK_SIZE (IPC_DATA_MAX_LEN - 1UL)
#define PUSH_RETRY_MS (5U)
#define PUSH_RETRY_COUNT (3U)

static int push_chunk(const char *data, uint32_t len)
{
  for (uint32_t r = 0U; r < PUSH_RETRY_COUNT; r++)
  {
    if (cm55_ipc_pipe_push_request((uint32_t)IPC_CMD_PRINT, data, len))
    {
      return 0;
    }
    vTaskDelay(pdMS_TO_TICKS(PUSH_RETRY_MS));
  }
  return -1;
}

int _write(int fd, const char *ptr, int len)
{
  if ((NULL == ptr) || (len < 0))
  {
    return -1;
  }
  if (0 == len)
  {
    return 0;
  }
  if ((fd != STDOUT_FD) && (fd != STDERR_FD))
  {
    return -1;
  }

  const char *p = ptr;
  int remaining = len;

  while (remaining > 0)
  {
    uint32_t chunk_len = (remaining > (int)CHUNK_SIZE) ? CHUNK_SIZE : (uint32_t)remaining;
    if (0 != push_chunk(p, chunk_len))
    {
      break;
    }
    p += chunk_len;
    remaining -= (int)chunk_len;
  }

  return len;
}
