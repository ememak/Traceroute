//Maciej Korpalski
//299513
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "trace_helpers.h"

u_int16_t compute_icmp_checksum (const void *buff, int length)
{
	u_int32_t sum;
	const u_int16_t* ptr = buff;
	assert (length % 2 == 0);
	for (sum = 0; length > 0; length -= 2)
		sum += *ptr++;
	sum = (sum >> 16) + (sum & 0xffff);
	return (u_int16_t)(~(sum + (sum >> 16)));
}

void send_icmp(int sockfd, const char * ip, u_int16_t pid, u_int16_t seq, int ttl){
  struct icmp header;
  header.icmp_type = ICMP_ECHO;
  header.icmp_code = 0;
  header.icmp_hun.ih_idseq.icd_id = pid;
  header.icmp_hun.ih_idseq.icd_seq = seq;
  header.icmp_cksum = 0;
  header.icmp_cksum = compute_icmp_checksum ((u_int16_t*)&header, sizeof(header));

  struct sockaddr_in recipient;
  void * ptr = memset (&recipient, 0,  sizeof(recipient));
  if(ptr == NULL){
    fprintf(stderr, "memset error in send_icmp %s", strerror(errno));
    assert(0);
  }
  recipient.sin_family = AF_INET;
  
  int r = inet_pton(AF_INET, ip, &recipient.sin_addr);
  if(r != 1){
    fprintf(stderr, "inet_pton error in send_icmp %s", strerror(errno));
    assert(0);
  }

  r = setsockopt (sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));
  if(r != 0){
    fprintf(stderr, "setsockopt error in send_icmp %s", strerror(errno));
    assert(0);
  }

  r = sendto(sockfd, &header, sizeof(header), 0, (struct sockaddr*)&recipient, sizeof(recipient));
  if(r < 0){
    fprintf(stderr, "sendto error in send_icmp %s", strerror(errno));
    assert(0);
  }
}

int is_it_proper_package(struct icmp* pack, u_int16_t pid, u_int16_t n){
  if(pack->icmp_type == ICMP_ECHOREPLY){
    if(pack->icmp_hun.ih_idseq.icd_id == pid && pack->icmp_hun.ih_idseq.icd_seq/3 == n/3)
      return 1;
  }
  if(pack->icmp_type == ICMP_TIME_EXCEEDED){
    struct ip* ip_pack = (struct ip*)(&pack->icmp_dun.id_ip.idi_ip);
    ssize_t ip_pack_len = 4 * ip_pack->ip_hl;

    struct icmp* inn_pack = (struct icmp*)((void*)ip_pack + ip_pack_len);
    if(inn_pack->icmp_hun.ih_idseq.icd_id == pid && inn_pack->icmp_hun.ih_idseq.icd_seq/3 == n/3)
      return 1;
  }
  return 0;
}
