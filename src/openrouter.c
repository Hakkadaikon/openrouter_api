#include "openrouter.h"
#include <curl/curl.h>

typedef CURL*               PCURL;
typedef struct curl_slist   CurlList;
typedef struct curl_slist*  PCurlList;
typedef struct curl_slist** PPCurlList;

typedef struct {
  PCHAR               url;
  PCurlList           headers;
  PCHAR               json_data;
  bool                is_verbose;
  POpenRouterResponse response;
} CurlOptionArgs;
typedef CurlOptionArgs* PCurlOptionArgs;

static size_t writeCallback(PVOID ptr, size_t size, size_t nmemb, PVOID userdata);
static bool   isBlankStr(const PCHAR str, const size_t size);
static void   makeAuthHeader(const size_t capacity, const PCHAR api_key, PCHAR auth_header);
static void   makeHeaders(const PCHAR auth_header, PPCurlList headers);
static void   makeJsonData(const PCHAR model, const PCHAR content, const size_t capacity, PCHAR json_data);
static void   setCurlOpt(PCurlOptionArgs args, PCURL curl);

static PUserWriteCallback g_write = NULL;

bool reqOpenRouter(POpenRouterArgs args, POpenRouterResponse response)
{
  CURLcode res = CURLE_OK;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  PCURL curl = curl_easy_init();

  if (!curl) {
    curl_global_cleanup();
    return false;
  }

  PCurlList headers = NULL;
  char      json_data[2048];
  char      auth_header[256];

  makeAuthHeader(sizeof(auth_header), args->api_key, auth_header);
  makeHeaders(auth_header, &headers);
  makeJsonData(args->model, args->content, sizeof(json_data), json_data);

  CurlOptionArgs curl_args;
  curl_args.url        = args->url;
  curl_args.headers    = headers;
  curl_args.json_data  = json_data;
  curl_args.is_verbose = args->is_verbose;
  curl_args.response   = response;
  setCurlOpt(&curl_args, curl);

  g_write = args->write;

  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    goto FINALIZE;
  }

FINALIZE:
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  curl_global_cleanup();
  return (res == CURLE_OK);
}

static size_t writeCallback(
  PVOID  ptr,
  size_t size,
  size_t nmemb,
  PVOID  userdata)
{
  size_t              realsize = size * nmemb;
  POpenRouterResponse response = (POpenRouterResponse)userdata;

  //printf("WriteCallback: %s\n", (char*)ptr);
  //fflush(stdout);

  if (isBlankStr(ptr, realsize)) {
    return realsize;
  }

  return g_write(ptr, realsize, response);
}

static bool isBlankStr(const PCHAR str, const size_t size)
{
  for (int i = 0; i < size; i++) {
    char c = str[i];
    if (c != ' ' && c != '\r' && c != '\n') {
      return false;
    }
  }

  return true;
}

static void makeAuthHeader(
  const size_t capacity,
  const PCHAR  api_key,
  PCHAR        auth_header)
{
  snprintf(auth_header, capacity, "Authorization: Bearer %s", api_key);
}

static void makeHeaders(const PCHAR auth_header, PPCurlList headers)
{
  *headers = curl_slist_append(*headers, "Content-Type: application/json");
  *headers = curl_slist_append(*headers, auth_header);
}

static void makeJsonData(
  const PCHAR  model,
  const PCHAR  content,
  const size_t capacity,
  PCHAR        json_data)
{
  snprintf(
    json_data,
    capacity,
    "{\"model\":\"%s\",\"messages\":[{\"role\":\"user\",\"content\":\"%s\"}]}",
    model,
    content);
}

static void setCurlOpt(
  PCurlOptionArgs args,
  PCURL           curl)
{
  curl_easy_setopt(curl, CURLOPT_URL, args->url);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, args->headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, args->json_data);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (PVOID)args->response);
  curl_easy_setopt(curl, CURLOPT_VERBOSE, (args->is_verbose) ? 1L : 0L);
}
