#ifndef TR_HELP_H
#define TR_HELP_H
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>


void send_icmp(int sockfd, const char * ip, u_int16_t pid, u_int16_t seq, int ttl);
int is_it_proper_package(struct icmp* pack, u_int16_t pid, u_int16_t n);

#endif
