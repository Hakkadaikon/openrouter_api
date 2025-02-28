#include <stdio.h>
#include <stdlib.h>

#include "openrouter.h"

size_t userWriteCallback(PVOID ptr, size_t size, POpenRouterResponse response);

int main(void)
{
  const PCHAR url       = "https://openrouter.ai/api/v1/chat/completions";
  const PCHAR model     = "openai/gpt-4o";
  const char  api_key[] = "-- api-key --";
  const PCHAR content   = "What is the meaning of life?";

  OpenRouterArgs     args;
  OpenRouterResponse response;

  memset(&args, 0x00, sizeof(args));

  args.url        = url;
  args.model      = model;
  args.api_key    = (PCHAR)api_key;
  args.content    = content;
  args.is_verbose = false;
  args.write      = userWriteCallback;

  if (!reqOpenRouter(&args, &response)) {
    printf("response : %s\n", response.data);
  }

  if (response.data != NULL) {
    free(response.data);
  }

  return 0;
}

size_t userWriteCallback(PVOID ptr, size_t size, POpenRouterResponse response)
{
  PCHAR ptr_new = realloc(response->data, response->size + size + 1);
  if (ptr_new == NULL) {
    return 0;
  }

  response->data = ptr_new;
  memcpy(&(response->data[response->size]), ptr, size);
  response->size += size;
  response->data[response->size] = '\0';

  return size;
}
