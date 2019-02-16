#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>


#define TCP 6
#define UDP 17
#define COMMAND_TYPE 2
#define DEC 10
#define QUEUE_NUM 1
#define INCOMMING_QUEUE_NUM 0
#define OUTGOING_QUEUE_NUM 1
#define PIPE_PATH "/tmp/myfifo"
#define MAX_PIPE_SIZE 1024

        /*bool filter = false;
bool stop = false;
struct nfq_handle* h;
struct nfq_q_handle* incomming_qh;
struct nfq_q_handle* outgoing_qh;

struct rule_node
{
    char* ip;
    int port;
    char* prot;
    bool src;
    struct rule_node* next;
};

struct url_rules
{
    char* url;
    struct url_rules* next;
};

struct rule_node* incoming_accept_rule_list = NULL;
struct rule_node* outgoing_accept_rule_list = NULL;
struct rule_node* incoming_drop_rule_list = NULL;
struct rule_node* outgoing_drop_rule_list = NULL;
struct url_rules* url_rules_list = NULL;

static int outgoing_cb(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg, struct nfq_data* nfa, void* data)
{
    
}

static int incomming_cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data)
{
    u_int32_t id;
    unsigned char* payload_data;
    struct in_addr ip_addr;
    struct iphdr* ip_hdr;
    struct udphdr* udp_hdr;
    struct tcphdr* tcp_hdr;
    size_t len = 0;
    ssize_t read;
    char* prot = NULL;
    struct nfqnl_msg_packet_hdr *ph;
    int src_port;
    int dst_port;
    char port[12];
    struct rule_node* incoming_accept_rule = incoming_accept_rule_list;
    struct rule_node* outgoing_accept_rule = outgoing_accept_rule_list;
    struct rule_node* incoming_drop_rule = incoming_drop_rule_list;
    struct rule_node* outgoing_drop_rule = outgoing_drop_rule_list;
    struct url_rules* url_rule = url_rules_list;

    ph = nfq_get_msg_packet_hdr(nfa);
    id = ntohl(ph->packet_id);
    
    nfq_get_payload(nfa, &payload_data);
    ip_hdr = ((struct iphdr*) payload_data);
    if(ip_hdr->protocol == TCP)
    {
        prot = "TCP";
        tcp_hdr = (struct tcphdr*)(payload_data + (ip_hdr->ihl << 2));


        src_port = (unsigned int)ntohs(tcp_hdr->source);
        dst_port = (unsigned int)ntohs(tcp_hdr->dest);
        
        printf("source = %u\ndest = %u\n", src_port, dst_port);
    }
    else if(ip_hdr->protocol == UDP)
    {
        prot = "UDP";
        udp_hdr = (struct udphdr*)(payload_data + (ip_hdr->ihl <<2));
        src_port = udp_hdr->source;
        dst_port = udp_hdr->dest;
        printf("source = %u\ndest = %u\n", src_port, dst_port);
    }
    else
    {
        prot = "else";
    }
    printf("protocol = %s\n",prot);
    ip_addr.s_addr = ip_hdr->saddr;
    printf("source ip = %s\n", inet_ntoa(ip_addr));
    ip_addr.s_addr = ip_hdr->daddr;
    printf("destination ip = %s\n", inet_ntoa(ip_addr));
    
    
    while(rule != NULL)
    {
        if(strcmp(rule->first, "incoming") == 0)
        {
            type = strtok(rule->second, divider1);
            if(strcmp(type, "ip") == 0)
            {
                type = strtok(NULL, divider1);
                ip_addr.s_addr = ip_hdr->saddr;
                if(strcmp(type, inet_ntoa(ip_addr)) == 0)
                {
                    if(strcmp(rule->third, "block\n") == 0)
                    {
                        printf("packet droped\n");
                        printf("\n");
                        return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
                    }
                }
            }
            else if(strcmp(type, "protocol") == 0)
            {
                type = strtok(NULL, divider1);
                if(strcmp(type, prot) == 0)
                {
                    if(strcmp(rule->third, "block\n") == 0)
                    {
                        printf("packet droped\n");
                        printf("\n");
                        return nfq_set_verdict(qh, id, NF_DROP, 0 ,NULL);
                    }
                }
            }
            else if(strcmp(type, "port") == 0)
            {
                type = strtok(NULL, divider1);
                sprintf(port, "%d", src_port);
                if(strcmp(port, type) == 0)
                {
                    if(strcmp(rule->third, "block\n") == 0)
                    {
                        printf("packet blocked");
                        printf("\n");
                        return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
                    }
                }
            }
        }
        rule = rule->next;
    }
    
    printf("\n");
    return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}
/*
the thread that gets messeges from the pipe
 */
        /*
void* pipe_thread(void* vargp)
{
    int fd;
    char pipe_msg[MAX_PIPE_SIZE] = {NULL};

    printf("entered pipe thread\n");   

    while(strcmp(pipe_msg, "03") != 0)
    {
        fd = open(PIPE_PATH, O_RDONLY);
        read(fd, pipe_msg, 80);
        printf("Message Recieved: %s\n", pipe_msg);
        close(fd);
        if(strcmp(pipe_msg, "00") == 0)
        {
            filter = true;
        }
        if(strcmp(pipe_msg, "01") == 0)
        {
            nfq_destroy_queue(qh);
            nfq_close(h);
            printf("exit pipe thread\n");
            stop = true;
            return NULL;
        }
        else if(strcmp(pipe_msg, "02") == 0)
        {
            nfq_destroy_queue(qh);
            nfq_create_queue(h, QUEUE_NUM, &cb, NULL);
            nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff);
        }
        else if(strcmp(pipe_msg, "03") == 0)
        {
            filter = false;
        }
        else if(strncmp(pipe_msg, "04", COMMAND_TYPE) == 0)
        {
            create_rule(pipe_msg);
        }
        else if(strncmp(pipe_msg, "05", COMMAND_TYPE) == 0)
        {
            delete_rule(pipe_msg);
        }
    }
}

/*
this fuction gets a format of a rule and creates a new node in the linked list of the rules
 */
        /*
void create_rule(char* rule)
{
    struct rule_node* new_rule = (struct rule_node*) malloc(sizeof(struct rule_node));
    struct rule_node* node = head;
    const char divider1[2] = " ";
    const char divider2[2] = ",";
    strtok(rule, divider1);
    
    rule = strtok(NULL, divider1);
    new_rule->first = strtok(rule, divider2);
    new_rule->second = strtok(NULL, divider2);
    new_rule->third = strtok(NULL, divider2);
    new_rule->next = NULL;
    
    while(node->next != NULL)
    {
        node = node->next;
    }
    node->next = new_rule;
}

void delete_rule(char* msg)
{
    struct rule_node* rule = head;
    struct rule_node* rule_to_delete;
    int index = 0;
    const char divider1[2] = " ";
    strtok(msg, divider1);
    
    msg = strtok(NULL, divider1);
    index = strtol(msg, NULL, DEC);

    if(index = 1)
    {
        head = rule->next;
        free(rule);
    }
    else if(index > 1)
    {
        while(index != 1 && rule->next != NULL)
        {
            rule = rule->next;
            index--;
        }
        if(index == 1)
        {
            rule_to_delete = rule->next;
            rule->next = rule_to_delete->next;
            free(rule_to_delete);
        }
    }
}
*/
int main(int argc, char **argv)
{
    /*int fd;
    int rv;
    char buf[4096] __attribute__ ((aligned));
    pthread_t id;
    char* type;

    printf("opening library handle\n");
    h = nfq_open();
    if (!h)
    {
        fprintf(stderr, "error during nfq_open()\n");
        exit(1);
    }

    printf("binding this socket to queue %d\n", QUEUE_NUM);
    qh = nfq_create_queue(h, QUEUE_NUM, &cb, NULL);
    if (!qh)
    printf("binding this socket to queue '%d'\n",INCOMMING_QUEUE_NUM);
    incomming_qh = nfq_create_queue(h, INCOMMING_QUEUE_NUM, &incomming_cb, NULL);
    if (!incomming_qh)
    {
        fprintf(stderr, "error opening incomming_qh\n");
        exit(1);
    }

    printf("binding this socket to queue '%d'\n", OUTGOING_QUEUE_NUM);
    outgoing_qh = nfq_create_queue(h, OUTGOING_QUEUE_NUM, &outgoing_cb, NULL);
    if(!outgoing_qh)
    {
        fprintf(stderr, "error opening outgoing_qh\n");
        exit(1);
    }

    printf("setting copy_packet mode\n");
    if (nfq_set_mode(incomming_qh, NFQNL_COPY_PACKET, 0xffff) < 0 && nfq_set_mode(outgoing_qh, NFQL_COPY_PACKET, 0xffff) < 0)
    {
        fprintf(stderr, "can't set packet_copy mode\n");
        exit(1);
    }
    
    fd = nfq_fd(h);

    pthread_create(&id, NULL, pipe_thread, NULL);
    
    printf("starting the loop\n");
    while(!stop)
    {
        if(strcmp(pipe_msg, "01") == 0)
        if(!filter)

        {
            continue;
        }        
        else
        {
            rv = recv(fd, buf, sizeof(buf), 0);
            printf("pkt received\n");
            nfq_handle_packet(h, buf, rv);
        }
    }
    pthread_join(id, NULL);
    */   
    exit(0);
}



