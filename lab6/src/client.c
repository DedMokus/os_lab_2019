#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

struct Server {
  char ip[255];
  int port;
};

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
  uint64_t result = 0;
  a = a % mod;
  while (b > 0) {
    if (b % 2 == 1)
      result = (result + a) % mod;
    a = (a * 2) % mod;
    b /= 2;
  }

  return result % mod;
}

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

int ParseServer(const char* line, struct Server* server) {
    char* colonPointer = strchr(line, ':');
    if (colonPointer == NULL) return -1;

    int ipLength = colonPointer - line;
    if (ipLength >= sizeof(server->ip)) return -1;
    
    strncpy(server->ip, line, ipLength);
    server->ip[ipLength] = '\0';

    server->port = atoi(colonPointer + 1);
    return 0;
}

struct Server* GetServers(const char *fileName, unsigned int *serversCount) {
    int capacity = 10;
    *serversCount = 0;

    struct Server* servers = malloc(capacity * sizeof(struct Server));
    if (servers == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    FILE* file = fopen(fileName, "r");
    if (file == NULL) {
        perror("Error opening file");
        free(servers);
        return NULL;
    }


    char line[255];
    while (fgets(line, sizeof(line), file) != NULL) {
        *strchr(line, '\n') = '\0';

        if (ParseServer(line, servers + *serversCount) == 0) {
            (*serversCount)++;

            if (*serversCount >= capacity) {
                capacity *= 2;
                servers = realloc(servers, capacity * sizeof(struct Server));
            }
        } else {
            printf("Skipping invalid line: %s\n", line);
        }
    }


    fclose(file);
    return servers;
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char serversFile[255] = {'\0'}; // TODO: explain why 255

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        ConvertStringToUI64(optarg, &k);
        if (k <= 0) {
          printf("K must be positive\n");
          return 1;
        }
        // TODO: your code here
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        if (mod <= 0) {
          printf("mod must be positive\n");
          return 1;
        }
        // TODO: your code here
        break;
      case 2:
        if (strlen(optarg) > 255) {
          printf("too long file name\n");
          return 1;
        }
        // TODO: your code here
        memcpy(serversFile, optarg, strlen(optarg));
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(serversFile)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  unsigned int serversCount = 1;
  struct Server* servers = GetServers(serversFile, &serversCount);

  int baseChunkSize = k / serversCount;
  int extendedChunksCount = k % serversCount;
  int prevEnd = 1;
  int* sockets = malloc(serversCount * sizeof(int));

  // TODO: work continiously, rewrite to make parallel
  for (int i = 0; i < serversCount; i++) {
      struct hostent *hostname = gethostbyname(servers[i].ip);
      if (hostname == NULL) {
        fprintf(stderr, "gethostbyname failed with %s\n", servers[i].ip);
        exit(1);
      }

      struct sockaddr_in server;
      server.sin_family = AF_INET;
      server.sin_port = htons(servers[i].port);
      server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

      sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
      if (sockets[i] < 0) {
        fprintf(stderr, "Socket creation failed!\n");
        exit(1);
      }

      if (connect(sockets[i], (struct sockaddr *)&server, sizeof(server)) < 0) {
        fprintf(stderr, "Connection failed\n");
        exit(1);
      }

      // TODO: for one server
      // parallel between servers
      uint64_t begin = prevEnd;
      uint64_t end = begin + baseChunkSize + (i < extendedChunksCount ? 1 : 0);
      prevEnd = end;

      char task[sizeof(uint64_t) * 3];
      memcpy(task, &begin, sizeof(uint64_t));
      memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
      memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

      if (send(sockets[i], task, sizeof(task), 0) < 0) {
        fprintf(stderr, "Send failed\n");
        exit(1);
      }
    }

    uint64_t answer = 1;
    for (int i = 0; i < serversCount; i++) {
      char response[sizeof(uint64_t)];
      if (recv(sockets[i], response, sizeof(response), 0) < 0) {
        fprintf(stderr, "Recieve failed\n");
        exit(1);
      }

      uint64_t convertedResponse = 0;
      memcpy(&convertedResponse, response, sizeof(uint64_t));
      printf("server %d responded: %llu\n", i, convertedResponse);

      answer = MultModulo(answer, convertedResponse, mod);

      close(sockets[i]);
    }


    printf("The overall answer is: %lld\n", answer);

  free(servers);

  return 0;
}
