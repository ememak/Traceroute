//Maciej Korpalski
//299513
#include <netinet/ip.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include "trace_helpers.h"

char resp[3][20];
int dt(struct timeval a, struct timeval b){
  int r = a.tv_usec - b.tv_usec;
  if(r >= 0)
    return r;
  return 1000000 + r;
}

int main(int argc, char* argv[]){
  if(argc != 2){
    fprintf(stderr, "Usage: %s ipv4_address\n", argv[0]);
    return EXIT_FAILURE;
  }
  struct in_addr addr;
  if(inet_pton(AF_INET, argv[1], &addr) != 1){
    fprintf(stderr, "First argument have to be valid ip4 address\n");
    return EXIT_FAILURE;
  }

  int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (sockfd < 0) {
    fprintf(stderr, "socket error: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  u_int16_t pid = getpid(), n=0;
  struct timeval start, stop, tv;

  for(int ttl = 1; ttl <= 30; ttl++){

    gettimeofday(&start, NULL);
    tv.tv_sec = 1; tv.tv_usec = 0;
    for(int j=0; j<3; j++)
      send_icmp(sockfd, argv[1], pid, n++, ttl);

    int N=0, respn = 0;
    double respt = 0;
    while(respn<3){
      struct sockaddr_in sender;
      socklen_t sender_len = sizeof(sender);
      u_int8_t buffer[IP_MAXPACKET];
    
      fd_set descriptors;
      FD_ZERO (&descriptors);
      FD_SET (sockfd, &descriptors);
      int ready = select (sockfd+1, &descriptors, NULL, NULL, &tv);
      if(ready <= 0) {
        break;
      }
      ssize_t packet_len = recvfrom (sockfd, buffer, IP_MAXPACKET, MSG_DONTWAIT, (struct sockaddr*)&sender, &sender_len);
      if (packet_len < 0) {
        fprintf(stderr, "recvfrom error: %s\n", strerror(errno));
        return EXIT_FAILURE;
      }

      gettimeofday(&stop, NULL);
      char ip_str[20];
      const char * ptr = inet_ntop (AF_INET, &(sender.sin_addr), ip_str, sizeof(ip_str));
      if(ptr == NULL){
        fprintf(stderr, "inet_ntop error in main: %s", strerror(errno));
        return EXIT_FAILURE;
      }

      struct ip*  ip_header = (struct ip*) buffer;
      ssize_t ip_header_len = 4 * ip_header->ip_hl;

      struct icmp* pack = (struct icmp*)(buffer + ip_header_len);
      int ok=0;
      if(is_it_proper_package(pack, pid, n-1)){
        for(int i=0; i<N; i++){
          if(memcmp(resp[i], ip_str, strlen(ip_str)) == 0){
            ok = 1;
            break;
          }
        }
        if(ok == 0){
          memset(resp[N], 0, 20);
          memcpy(resp[N], ip_str, strlen(ip_str));
          N++;
        }
        respn++;
        respt += dt(stop, start);
      }
    }
    printf("%d. ", ttl);
    if(N>0){
      for(int i=0; i<N; i++)
        printf("%s ", resp[i]);
      if(respn == 3)
        printf("%.3lfms\n", respt/3000.0);
      else
        printf("???\n");
    }
    else
      printf("*\n");
    if(memcmp(resp[0], argv[1], strlen(argv[1])) == 0)
      break;
  }
  return 0;
}
