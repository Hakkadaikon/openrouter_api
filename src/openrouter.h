#ifndef _OPEN_ROUTER_H_
#define _OPEN_ROUTER_H_

#include <stdbool.h>
#include <string.h>

typedef char* PCHAR;
typedef void* PVOID;

typedef struct {
  PCHAR  data;
  size_t size;
} OpenRouterResponse;
typedef OpenRouterResponse* POpenRouterResponse;

typedef size_t (*PUserWriteCallback)(
  PVOID ptr,
  size_t size,
  POpenRouterResponse response
);

typedef struct {
  PCHAR              url;
  PCHAR              model;
  PCHAR              api_key;
  PCHAR              content;
  bool               is_verbose;
  PUserWriteCallback write;
} OpenRouterArgs;
typedef OpenRouterArgs* POpenRouterArgs;

bool reqOpenRouter(const POpenRouterArgs args, POpenRouterResponse response);

#endif
