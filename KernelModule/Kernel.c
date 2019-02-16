#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/string.h>

#define DEBUG // Remove after you finish writing the project
#define PROC_NAME "my_proc"
#define DB_PROC_NAME "logs_proc"
#define COUNTER_PROC_NAME "firewall_counter"
#define TCP_ID 6
#define UDP_ID 17
#define ICMP_ID 1
#define MAX_PROC_SIZE 4096
#define PORT 0
#define IP 1
#define PROTOCOL 2
#define ICMP_MAX_SIZE 65535
#define FW_POLICY NF_QUEUE
#define IP_CE       0x8000      /* Flag: "Congestion"       */
#define IP_DF       0x4000      /* Flag: "Don't Fragment"   */
#define IP_MF       0x2000      /* Flag: "More Fragments"   */
#define IP_OFFSET   0x1FFF      /* "Fragment Offset" part   */
#define MAX_XMAS_COUNT 20
#define MAX_SYN_COUNT 20
#define MAX_UNIQUE_PORTS 10
#define PACKET_TIL_LOG 100
#define RESET_BUFF_MAX_LENGTH 20
#define RESET_MSG "reset"
#define RESET_HOSTS_MSG "reset_hosts"

static int proc_length = 0;
static int db_proc_length = 0;
static int reset_buff_length = 0;
static int counter_buff_length = 0;

static int packet_count = 0;

static char msg[MAX_PROC_SIZE];
static char db_msg[MAX_PROC_SIZE];
static char reset_msg[RESET_BUFF_MAX_LENGTH];
static char counter_msg[MAX_PROC_SIZE];

static int count_incoming_packet = 0;
static int count_outgoing_packet = 0;
static int count_dropped_packets = 0;
static int count_accepted_packets = 0;
static int count_accepted_incoming_packets = 0;
static int count_dropped_incoming_packets = 0;
static int count_accepted_outgoing_packets = 0;
static int count_dropped_outgoing_packets = 0;
static int count_dropped_by_ip = 0;
static int count_dropped_by_port = 0;
static int count_accepted_by_port = 0;
static int count_accepted_by_ip = 0;
static int count_dropped_by_user_rules = 0;
static int count_accepted_by_user_rules = 0;
static int count_dropped_by_protocol = 0;
static int count_accepted_by_protocol = 0;

static int id_count = 0;

static int dynamic_id_count = 0;

static int count_incoming_bytes = 0;

static int count_outgoing_bytes = 0;

enum protocol_type
{
    TCP,
    UDP
};
    
char * protocolToString(enum protocol_type protocol)
{
    switch (protocol)
    {
    case TCP:
        return "TCP";
        break;

    case UDP:
        return "UDP";
        break;
    default:
        return "non";
        
    }
}

struct firewall_rules
{
    int id;
    int type;
    bool source;
    char ip_parameter[17];
    char protocol_parameter[7];
    int port_parameter;
    int packet_filtered;
    
    struct list_head list;
};

struct suspicious_host
{
    int xmas_count;
    int ports[MAX_UNIQUE_PORTS];
    char ip[17];
    int syn_count;

    struct list_head list;
};

void printList(struct firewall_rules * ruleList);
void freeList(struct firewall_rules* ruleList);

struct firewall_rules accept_incoming_rules;
struct firewall_rules drop_incoming_rules;
struct firewall_rules accept_outgoing_rules;
struct firewall_rules drop_outgoing_rules;
struct firewall_rules drop_incoming_dynamic;

struct suspicious_host suspect_list;

void freeSuspiciousHosts(void);

struct packet_headers
{
    char sourceIp[16];
    int sourcePort;
    char destIp [16];
    int destPort;
    int length;
    enum protocol_type protocol;
};

bool getParam(const char * rule, char * parameter, int maxCount)
{
    int count = 0;
    int i = 0;
    
    int len = 0;
    
    len = strcspn(rule, "=");
    
    // TODO(Eran): Use strstr and strscpn to get parameter
    if (len != strlen(rule))
    {
        for (i=len+1; i < strlen(rule); i++)
        {
            if (rule[i] == ',')
            {
                parameter[count] = '\0';
                break;
            }
            else if (count == maxCount)
            {
                return false;
            }
            
            parameter[count] = rule[i];
            count++;
        }
        
        return true;
    }
    
    return false;
}

int getRuleType(char * rule)
{
    int accept = -1;

    if (strstr(rule, "accept") != NULL)
    {
        accept = 1;
    }
    else if (strstr(rule, "drop") != NULL)
    {
        accept = 0;
    }

    return accept;
}

int getRuleId(char * rule)
{
    int i = 3;
    
    char id[8];
    
    int count = 0;
    
    long res;
    
    for (i = 3; i < strlen(rule); i++)
    {  
        id[i-3] = rule[i];
        
        count++;
    }
    
    id[count] = '\0';
    
    kstrtol(id, 10, &res);
    
    return res;
}

void removeRuleFromList(struct firewall_rules* ruleList, int id)
{
    struct firewall_rules * tmp;
    struct list_head * pos, *q;
    
    list_for_each_safe(pos, q, &ruleList->list)
    {
        tmp = list_entry(pos, struct firewall_rules, list);
        {
            if (tmp->id > id)
            {
                tmp->id--;
            }
            else if (tmp->id == id)
            {
            
                list_del(pos);
                kfree(tmp);
            }
        }
    }
    
}

void createProtocolRule(struct firewall_rules * ruleList, char * rule)
{
    struct firewall_rules * node;
    char protocol_parameter [16];

    node = (struct firewall_rules *)kmalloc(sizeof(struct firewall_rules), GFP_KERNEL);
    node->type = PROTOCOL;
    
    if (getParam(rule, protocol_parameter, 16))
    {
        strcpy(node->protocol_parameter, protocol_parameter);
    }
    else    
    {
        kfree(node);
    }

    id_count++;
    node->id = id_count;
    node->packet_filtered = 0;

    INIT_LIST_HEAD(&node->list);

    list_add(&(node->list), &(ruleList->list)); 
}

void createIpRule(struct firewall_rules * ruleList, bool source, char * rule, bool is_dynamic)
{
    struct firewall_rules * node;
    char ip_parameter [16];

    node = (struct firewall_rules *)kmalloc(sizeof(struct firewall_rules), GFP_KERNEL);
    node->source = source;
    node->type = IP;
    
    if (getParam(rule, ip_parameter, 16))
    {
        strcpy(node->ip_parameter, ip_parameter);
    }
    else    
    {
        kfree(node);
    }
    
    if (!is_dynamic)
    {
        id_count++;
        node->id = id_count;
    }
    else
    {
        dynamic_id_count++;
        node->id = dynamic_id_count;
    }

    node->packet_filtered = 0;
    
    INIT_LIST_HEAD(&node->list);

    list_add(&(node->list), &(ruleList->list)); 
}

void createPortRule(struct firewall_rules * ruleList, bool source, char * rule)
{
    struct firewall_rules * node;
    char port_parameter [6];
    
    long res;

    node = (struct firewall_rules *)kmalloc(sizeof(struct firewall_rules), GFP_KERNEL);
    node->source = source;
    node->type = PORT;
    
    if (getParam(rule, port_parameter, 5))
    {
        kstrtol(port_parameter, 10, &res);
        node->port_parameter = res;
    }
    else    
    {
        kfree(node);
    }

    id_count++;
    node->id = id_count;
    node->packet_filtered = 0;

    INIT_LIST_HEAD(&node->list);

    list_add(&(node->list), &(ruleList->list));    
}

void createRuleByType(struct firewall_rules * ruleList, char * rule)
{
    if (strstr(rule, "dport") != NULL)
    {
        createPortRule(ruleList, false, rule);
    }
    else if (strstr(rule, "sport") != NULL)
    {
        createPortRule(ruleList, true, rule);
    }
    else if (strstr(rule, "dip") != NULL)
    {
        createIpRule(ruleList, false, rule, false);
    }
    else if (strstr(rule, "sip") != NULL)
    {
        createIpRule(ruleList, true, rule, false);
    }
    else 
    {
        createProtocolRule(ruleList, rule);
    }
}

void removeRule(char * rule)
{
    int id = getRuleId(rule);
    
    removeRuleFromList(&accept_incoming_rules, id);
    removeRuleFromList(&drop_incoming_rules, id);
    removeRuleFromList(&accept_outgoing_rules, id);
    removeRuleFromList(&drop_outgoing_rules, id);
    
    
    id_count--;
}

bool buildRule(char * rule)
{
    bool accept = false;

    int acceptTmp = 0;
    acceptTmp = getRuleType(rule);

    if (acceptTmp == 1)
    {
        accept = true;
    }
    else if (acceptTmp == -1)
    {
        return false;
    }

    if (strstr(rule, "incoming") != NULL)
    {
        if (accept)
        {
            createRuleByType(&accept_incoming_rules, rule);
        }
        else
        {
            createRuleByType(&drop_incoming_rules, rule);
        }
    }
    else if (strstr(rule, "outgoing") != NULL)
    {
        if (accept)
        {
            createRuleByType(&accept_outgoing_rules, rule);
        }
        else
        {
            createRuleByType(&drop_outgoing_rules, rule);
        }
    }
    
    return false;
}

void insertStatistics(char * buff, int * statistics_numbers, int len)
{
    int i = 0;
    char numBuff [12];

    for (i = 0; i<len-1; i++)
    {
        sprintf(numBuff, "%d," ,*(statistics_numbers + i));
        strcat(buff, numBuff);
    }

    sprintf(numBuff, "%d" ,*(statistics_numbers + (len-1)));
    strcat(buff, numBuff);
}

void insertGeneralStatistics(char * buff)
{
    int stat_nums [8];

    stat_nums[0] = count_accepted_by_user_rules;
    stat_nums[1] = count_dropped_by_user_rules;
    stat_nums[2] = count_accepted_by_ip;
    stat_nums[3] = count_dropped_by_ip;
    stat_nums[4] = count_accepted_by_port;
    stat_nums[5] = count_dropped_by_port;
    stat_nums[6] = count_accepted_by_protocol;
    stat_nums[7] = count_dropped_by_protocol;

    insertStatistics(buff, stat_nums, 8);
}

void insertNormalStatistics(char * buff)
{
    int stat_nums [11];

    stat_nums[0] = count_incoming_packet + count_outgoing_packet;
    stat_nums[1] = count_incoming_packet;
    stat_nums[2] = count_outgoing_packet;
    stat_nums[3] = count_accepted_packets;
    stat_nums[4] = count_dropped_packets;
    stat_nums[5] = count_accepted_incoming_packets;
    stat_nums[6] = count_dropped_incoming_packets;
    stat_nums[7] = count_accepted_outgoing_packets;
    stat_nums[8] = count_dropped_outgoing_packets;
    stat_nums[9] = count_incoming_bytes;
    stat_nums[10] = count_outgoing_bytes;

    count_incoming_bytes = 0;
    count_outgoing_bytes = 0;

    insertStatistics(buff, stat_nums, 11);
}

int getPacketCountList(int id, struct firewall_rules * fw_rules)
{
    struct list_head * pos;
    struct firewall_rules * tmp;
    
    list_for_each(pos, &fw_rules->list)
    {
        tmp = list_entry(pos, struct firewall_rules, list);

        if (tmp->id == id)
        {
            return tmp->packet_filtered;
        }
    }
    
    return -1;
}

int getPacketCount(int id)
{
    int packets = -1;

    packets = getPacketCountList(id, &accept_incoming_rules);

    if (packets != -1)
    {
        return packets;
    }

    packets = getPacketCountList(id, &accept_outgoing_rules);

    if (packets != -1)
    {
        return packets;
    }

    packets = getPacketCountList(id, &drop_incoming_rules);

    if (packets != -1)
    {
        return packets;
    }
    
    return getPacketCountList(id, &drop_outgoing_rules);
}

int ruleEffectivnesList(int packets, struct firewall_rules * fw_rules)
{
    struct list_head * pos;
    struct firewall_rules * tmp;

    int count = 0;
    
    list_for_each(pos, &fw_rules->list)
    {
        tmp = list_entry(pos, struct firewall_rules, list);

        if (tmp->packet_filtered > packets)
        {
            count++;
        }
    }

    return count;
}

int ruleEffectivnes(int packets)
{
    int count = 1;
    
    count += ruleEffectivnesList(packets, &accept_incoming_rules);
    count += ruleEffectivnesList(packets, &drop_incoming_rules);
    count += ruleEffectivnesList(packets, &accept_outgoing_rules);
    count += ruleEffectivnesList(packets, &drop_outgoing_rules);

    return count;
}

void insertRuleStatistics(char * buff, int id)
{
    int packets = getPacketCount(id);
    int num_stats [5];

    if (packets == -1)
    {
        strcat(buff, "Error: no rule with this id exists");
    }
    else
    {
        num_stats[0] = packets;
        num_stats[1] = count_incoming_packet + count_outgoing_packet;
        num_stats[2] = count_accepted_by_user_rules + count_dropped_by_user_rules;
        num_stats[3] = ruleEffectivnes(num_stats[0]);
        num_stats[4] = id_count;
        insertStatistics(buff, num_stats ,5);
    }
}

ssize_t write_counter_proc(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
    if (count > MAX_PROC_SIZE)
    {
        counter_buff_length = RESET_BUFF_MAX_LENGTH;
    }
    else
    {
        counter_buff_length = count;
    }

    copy_from_user(counter_msg, buf, counter_buff_length);
    
    counter_msg[counter_buff_length] = '\0';

    printk(KERN_INFO "message is %s", counter_msg);        

    return counter_buff_length;
}

ssize_t read_counter_proc(struct file *filp,char *buf,size_t count,loff_t *offp )
{
    static int counter_finished = 0 ;
    char buff [150] = "";

    if (counter_finished == 1)
    {
        counter_finished = 0;
        printk(KERN_INFO "Read from counter proc file");
        return 0;
    }

    if (count > counter_buff_length)
    {
        count = counter_buff_length;
    }

    if (strcmp(counter_msg, "10") == 0)
    {
        insertNormalStatistics(buff);
        count = strlen(buff);
        printk(KERN_INFO "message sent to user is: %s\n" ,buff);
        copy_to_user(buf, buff, count);
    }
    else if (strcmp(counter_msg, "11") == 0)
    {
        insertGeneralStatistics(buff);
        count = strlen(buff);
        printk(KERN_INFO "message sent to user is: %s\n" ,buff);
        copy_to_user(buf, buff, count);
    }
    else if (strlen(counter_msg) > 2 && counter_msg[0] == '1' && counter_msg[1] == '2')
    {
        insertRuleStatistics(buff, getRuleId(counter_msg));
        count = strlen(buff);
        printk(KERN_INFO "message sent to user is: %s\n" ,buff);
        copy_to_user(buf, buff, count);
    }
  
    //copy_to_user(buf, db_msg, count);    

    counter_finished = 1;

    return count;
}


ssize_t write_reset_proc(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
    if (count > RESET_BUFF_MAX_LENGTH)
    {
        reset_buff_length = RESET_BUFF_MAX_LENGTH;
    }
    else
    {
        reset_buff_length = count;
    }

    copy_from_user(reset_msg, buf, reset_buff_length);
    
    reset_msg[reset_buff_length] = '\0';

    if (strcmp(reset_msg, RESET_MSG) == 0)
    {
        printk(KERN_INFO "reseting dynamic rules\n");
        freeSuspiciousHosts();        
        freeList(&drop_incoming_dynamic);
        INIT_LIST_HEAD(&suspect_list.list);
        INIT_LIST_HEAD(&drop_incoming_dynamic.list);
    }
    else if (strcmp(reset_msg, RESET_HOSTS_MSG) == 0)
    {
        printk(KERN_INFO "reseting hosts\n");
        freeSuspiciousHosts();
        INIT_LIST_HEAD(&suspect_list.list);
    }

    printk(KERN_INFO "message is %s", reset_msg);
    
    memset(reset_msg, 0, RESET_BUFF_MAX_LENGTH);    

    return reset_buff_length;
}

ssize_t read_db_proc(struct file *filp,char *buf,size_t count,loff_t *offp )
{
    static int db_finished = 0 ;

    if (db_finished == 1)
    {
        db_finished = 0;
        printk(KERN_INFO "Read from db proc file");
        return 0;
    }

    if (count > db_proc_length)
    {
        count = db_proc_length;
    }
  
    copy_to_user(buf, db_msg, count);
    
    memset(db_msg, 0, MAX_PROC_SIZE);

    db_proc_length = 0;

    db_finished = 1;

    return count;
}

ssize_t read_proc(struct file *filp,char *buf,size_t count,loff_t *offp )
{
    static int finished = 0 ;

    if (finished == 1)
    {
        finished = 0;
        printk(KERN_INFO "Read from proc file");
        return 0;
    }

    if (count > proc_length)
    {
        count = proc_length;
    }
  
    copy_to_user(buf, msg, count);

    finished = 1;

    return count;
}

ssize_t write_proc(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
    if (count > MAX_PROC_SIZE)
    {
        proc_length = MAX_PROC_SIZE;
    }
    else
    {
        proc_length = count;
    }

    copy_from_user(msg,buf,proc_length);
    
    msg[proc_length] = '\0';
    
    if (proc_length > 2)
    {
        if (msg[0] == '0' && msg[1] == '4')
        {
            buildRule(msg);
        }
        else if (msg[0] == '0' && msg[1] == '5')
        {
            printk(KERN_INFO "removing!");
            removeRule(msg);
        }
    }
    

    
    printk(KERN_INFO "message is: %s" ,msg);

    return proc_length;
}

/*static int module_permission(struct inode *inode, int op, struct nameidata *foo)
{
    return 0;
}

static struct inode_operations Inode_Ops_4_Our_Proc_File = {
    .permission = module_permission // check for permissions
};*/



static struct file_operations proc_fops = {
 read: read_proc,
 write: write_proc,
 owner: THIS_MODULE 
};

static struct file_operations db_proc_fops = {
 read: read_db_proc,
 write: write_reset_proc,
 owner: THIS_MODULE
};

static struct file_operations counter_proc_fops = {
 read: read_counter_proc,
 write: write_counter_proc,
 owner: THIS_MODULE 
};


struct proc_dir_entry *proc_file_entry;
struct proc_dir_entry* db_proc_file;
struct proc_dir_entry* counter_proc_file;

static struct nf_hook_ops nfho;
static struct nf_hook_ops nfho_out;

bool scanRules(struct packet_headers packet, struct firewall_rules * ruleList, bool is_accept)
{
    struct list_head * pos;
    struct firewall_rules * tmp;
    
    list_for_each(pos, &ruleList->list)
    {
        tmp = list_entry(pos, struct firewall_rules, list);
        
        if (tmp->type == PORT)
        {
            if (tmp->source)
            {
                if (packet.sourcePort == tmp->port_parameter)
                {
                    tmp->packet_filtered++;
                    !is_accept ? count_dropped_by_port++ : count_accepted_by_port++;                                        
                    return true;
                }
            }
            else
            {
                if (packet.destPort == tmp->port_parameter)
                {
                    tmp->packet_filtered++;
                    !is_accept ? count_dropped_by_port++ : count_accepted_by_port++;
                    return true;
                }
            }
        }
        else if (tmp->type == IP)
        {
            if (tmp->source)
            {
                if (!strcmp(packet.sourceIp, tmp->ip_parameter))
                {
                    tmp->packet_filtered++;                    
                    !is_accept ? count_dropped_by_ip++ : count_accepted_by_ip++;
                    return true;
                }
            }
            else
            {
                if (!strcmp(packet.destIp, tmp->ip_parameter))
                {
                    tmp->packet_filtered++;
                    !is_accept ? count_dropped_by_ip++ : count_accepted_by_ip++;
                    return true;
                }
            }
        }
        else
        {
            if (!strcmp(protocolToString(packet.protocol), tmp->protocol_parameter))
            {
                tmp->packet_filtered++;
                !is_accept ? count_dropped_by_protocol++ : count_accepted_by_protocol++;
                return true;
            }
        }
    }
        
    return false;       
}

void setUDPValues(struct packet_headers * packet, struct udphdr * udp_hdr)
{
    packet->sourcePort = (unsigned short int)ntohs(udp_hdr->source);
    packet->destPort = (unsigned short int)ntohs(udp_hdr->dest);
    
    //printk(KERN_INFO "Source Port: %u\n Dest Port: %u\n", packet->sourcePort, packet->destPort);
}

void setTCPValues(struct packet_headers * packet, struct tcphdr * tcp_hdr)
{
    packet->sourcePort = (unsigned short int)ntohs(tcp_hdr->source);
    packet->destPort = (unsigned short int)ntohs(tcp_hdr->dest);
    
    //printk(KERN_INFO "Source Port: %u\n Dest Port: %u\n", packet->sourcePort, packet->destPort);
}
void freeSuspiciousHost(char * ip)
{
    struct suspicious_host * tmp;
    struct list_head * pos, *q;
    
    list_for_each_safe(pos, q, &suspect_list.list)
    {
        tmp = list_entry(pos, struct suspicious_host, list);
        {            
            if (strcmp(tmp->ip, ip) == 0)
            {            
                list_del(pos);
                kfree(tmp);             
            }
        }
    }    
}

void addSuspiciousHost(struct packet_headers packet, bool is_syn)
{
    struct suspicious_host * node;
    
    int i = 0;
    
    node = (struct suspicious_host *)kmalloc(sizeof(struct suspicious_host), GFP_KERNEL);

    for (i = 0; i < MAX_UNIQUE_PORTS; i++)
    {
        node->ports[i] = -1;
    }
    
    if (is_syn)
    {
        node->ports[0] = packet.destPort;
        node->syn_count = 1;
        node->xmas_count = 0;
    }
    else
    {
        node->xmas_count = 1;
        node->syn_count = 0;
    }

    memset(node->ip, '\0', 17);
    strcpy(node->ip, packet.sourceIp);
    
    INIT_LIST_HEAD(&node->list);

    list_add(&(node->list), &(suspect_list.list));

    printk(KERN_INFO "Added suspicious host: %s\n", node->ip);
}

bool shouldDropSuspect(struct packet_headers packet, bool is_syn, bool is_xmas)
{
    struct list_head * pos;
    struct suspicious_host * tmp;
    int i = 0;    

    list_for_each(pos, &suspect_list.list)
    {
        tmp = list_entry(pos, struct suspicious_host, list);        
        
        if (strcmp(tmp->ip, packet.sourceIp) == 0)
        {
            
            if (is_syn)
            {
                tmp->syn_count++;                

                if (tmp->syn_count == MAX_SYN_COUNT)
                {
                    return true;
                }
                                                
            }                        
            else if (is_xmas)
            {
                tmp->xmas_count++;
                
                if (tmp->xmas_count < MAX_XMAS_COUNT)
                {
                    return false;
                }
            }

            for (i = 0; i < MAX_UNIQUE_PORTS; i++)
                {
                    if (tmp->ports[i] == packet.destPort)
                    {
                        return false;
                    }
                    else if (tmp->ports[i] == -1)
                    {
                        tmp->ports[i] = packet.destPort;

                        return false;
                    }                
                }
            
            return true;
        }
    }
    
    addSuspiciousHost(packet, is_syn);    
    return false;   
}

bool isSyn(struct tcphdr* tcp_hdr)
{
    if ((unsigned int)ntohs(tcp_hdr->syn) == 256 && (unsigned int)ntohs(tcp_hdr->ack) == 0)
    {
        return true;
    }

    return false;
}

bool isXmas(struct tcphdr* tcp_hdr)
{        
    if ((unsigned int)ntohs(tcp_hdr->urg) == 256 && (unsigned int)ntohs(tcp_hdr->psh) == 256 && (unsigned int)ntohs(tcp_hdr->fin) == 256)
    {        
        return true;        
    }
    
    return false;
}

bool isFragment(struct iphdr * ip_hdr)
{
    if ((unsigned int)ntohs(ip_hdr->frag_off & IP_MF) > 0 || ((unsigned int)ntohs(ip_hdr->frag_off) & IP_OFFSET) > 0)
        {
            printk(KERN_INFO "dropped fragmented ICMP packet\n");           
            printk(KERN_INFO "fragments: %d\noffset: %d\n", (unsigned int)ntohs(ip_hdr->frag_off & IP_MF), (unsigned int)ntohs(ip_hdr->frag_off) & IP_OFFSET);

            return true;
        }

    return false;
}

void logPacket(struct packet_headers packet, bool incoming, bool accept, char * more_info)
{
    char log_msg[100]="";    
    
    if (db_proc_length >= 4000)
    {
        return;
    }
    
    memset(log_msg, 0, 100);    
    sprintf(log_msg, "%s,%s,%d,%d" ,packet.sourceIp ,packet.destIp ,packet.sourcePort, packet.destPort);
    
    if (incoming)
    {
        strcat(log_msg, ",incoming,");
    }
    else
    {
        strcat(log_msg, ",outgoing,");
    }

    if (accept)
    {
        strcat(log_msg, "accept");
    }
    else
    {
        strcat(log_msg, "drop");
    }

    strcat(log_msg, more_info);
    strcat(log_msg, "\n");

    strcat(db_msg, log_msg);

    db_proc_length += strlen(log_msg);
    
}

unsigned int hook_out(void *priv, struct sk_buff *skb, const struct nf_hook_state * state)
{
    struct iphdr *ip_header = (struct iphdr *)skb_network_header(skb); 
    struct tcphdr *tcp_header = NULL;
    struct udphdr *udp_header = NULL;
    struct packet_headers headers;
    packet_count++;
    count_outgoing_packet++;
    
#ifdef DEBUG
    //printk(KERN_ALERT "Hooked Outgoing");
#endif

    snprintf(headers.destIp, 16, "%pI4", &ip_header->daddr);
    snprintf(headers.sourceIp, 16, "%pI4", &ip_header->saddr);

    count_outgoing_bytes += (unsigned int)ntohs(ip_header->tot_len);
   
   
    if (ip_header->protocol == TCP_ID) //if the packet is tcp protocol 
    {
        //printk(KERN_INFO "TCP Packet\n");        
        tcp_header = tcp_hdr(skb);
        headers.protocol = TCP; 

        if (tcp_header)
        {
            setTCPValues(&headers, tcp_header);
        }
    }
    else if (ip_header->protocol == UDP_ID)
    {
        //printk(KERN_INFO "UDP Packet\n");
        udp_header = udp_hdr(skb);
        headers.protocol = UDP;
        
        if (udp_header)
        {
            setUDPValues(&headers, udp_header);
        }
    }
    
    if (scanRules(headers, &accept_outgoing_rules, true))
    {
        //printk(KERN_INFO "packet accepted\n");

        if (packet_count >= PACKET_TIL_LOG)
        {
            logPacket(headers, false, true, "");
            packet_count -= PACKET_TIL_LOG;
        }

        count_accepted_by_user_rules++;
        count_accepted_packets++;
        count_accepted_outgoing_packets++;
        return NF_ACCEPT;
    }
    else if (scanRules(headers, &drop_outgoing_rules, false))
    {
        //printk(KERN_INFO "packet droppped\n");

        if (packet_count >= PACKET_TIL_LOG)
        {
            logPacket(headers, false, false, "");
            packet_count -= PACKET_TIL_LOG;
        }

        count_dropped_by_user_rules++;
        count_dropped_packets++;
        count_dropped_outgoing_packets++;
            
        return NF_DROP;
    }

    
    //printk(KERN_INFO  "activating firewall policy\n");    

    count_accepted_packets++;
    count_accepted_outgoing_packets++;
    return NF_ACCEPT;
}

unsigned int hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state * state)
{
    struct iphdr *ip_header = (struct iphdr *)skb_network_header(skb);
    struct tcphdr *tcp_header = NULL;
    struct udphdr *udp_header = NULL;
    char dynamicRule [100] = "";
    struct packet_headers headers;
    packet_count++;
    count_incoming_packet++;

    count_incoming_bytes += (unsigned int)ntohs(ip_header->tot_len);

    strcat(dynamicRule, "04 incoming,sip=");
    snprintf(headers.destIp, 16, "%pI4", &ip_header->daddr);
    snprintf(headers.sourceIp, 16, "%pI4", &ip_header->saddr);

    if (strcmp(headers.sourceIp, "127.0.1.1") == 0 || strcmp(headers.sourceIp, "127.0.0.1") == 0)
    {
        return NF_ACCEPT;
    }

    if (ip_header->protocol == TCP_ID) //if the packet is tcp protocol 
    {
        //printk(KERN_INFO "TCP Packet\n");
        tcp_header = tcp_hdr(skb);
        headers.protocol = TCP; 

        if (tcp_header)
        {
            setTCPValues(&headers, tcp_header);
        }
    }
    else if (ip_header->protocol == UDP_ID)
    {
        //printk(KERN_INFO "UDP Packet\n");
        udp_header = udp_hdr(skb);
        headers.protocol = UDP;
        
        if (udp_header)
        {
            setUDPValues(&headers, udp_header);
        }
    }
    else if (ip_header->protocol == ICMP_ID)
    {
        //printk(KERN_INFO "ICMP Packet\n");
        
        if (isFragment(ip_header))
        {
            printk(KERN_INFO "logging packet\n");
            logPacket(headers, true, false, "ICMP fragment");
            return NF_DROP;
        }                            
    }
    if (scanRules(headers, &accept_incoming_rules, true))
    {
        //printk(KERN_INFO "packet accepted\n");
        if (packet_count >= PACKET_TIL_LOG)
        {
            logPacket(headers, true, true, "");
            packet_count -= PACKET_TIL_LOG;
        }

        count_accepted_packets++;
        count_accepted_incoming_packets++;
        count_accepted_by_user_rules++;
        return NF_ACCEPT;
    }
    else if (scanRules(headers, &drop_incoming_rules, false))
    {
        //printk(KERN_INFO "packet dropped\n");
        
        if (packet_count >= PACKET_TIL_LOG)
        {
            logPacket(headers, true, false, "");
            packet_count -= PACKET_TIL_LOG;
        }

        count_dropped_incoming_packets++;        
        count_dropped_packets++;
        count_dropped_by_user_rules++;
        return NF_DROP;
    }
    else if (scanRules(headers, &drop_incoming_dynamic, false))
    {
        if (packet_count >= PACKET_TIL_LOG)
        {
            logPacket(headers, true, false, "");
            packet_count -= PACKET_TIL_LOG;
        }

        count_dropped_incoming_packets++;        
        count_dropped_packets++;
        return NF_DROP;
    }
    
    if (tcp_header)
    {
        if (isXmas(tcp_header))
        {
            printk(KERN_INFO "dropped XMAS packet - logging packet\n");
            logPacket(headers, true, false, ",XMAS packet");
            
            if (shouldDropSuspect(headers, false, true))
            {
                printk("blocking ip address: %s\n", headers.sourceIp);
                strcat(dynamicRule, headers.sourceIp);
                strcat(dynamicRule, ",drop");
                printk("adding dynamic rule: %s\n", dynamicRule);
                createIpRule(&drop_incoming_dynamic, true, dynamicRule, true);
                printList(&drop_incoming_dynamic);
                freeSuspiciousHost(headers.sourceIp);
            }
            
            return NF_DROP;
        }
        else if (isSyn(tcp_header))
        {
            if (shouldDropSuspect(headers, true, false))
            {
                logPacket(headers, true, false, "Port scan/SYN ACK flood");
                strcat(dynamicRule, headers.sourceIp);
                strcat(dynamicRule, ",drop");
                createIpRule(&drop_incoming_dynamic, true, dynamicRule, true);
                printk("adding dynamic rule: %s\n", dynamicRule);
                freeSuspiciousHost(headers.sourceIp);
                return NF_DROP;
            }
        }
        else if (shouldDropSuspect(headers, false, false))
        {
            strcat(dynamicRule, headers.sourceIp);
            strcat(dynamicRule, ",drop"); 
            createIpRule(&drop_incoming_dynamic, true, dynamicRule, true);
            printk("adding dynamic rule: %s\n", dynamicRule);
            freeSuspiciousHost(headers.sourceIp);
            return NF_DROP;
        }
    }
    else if (udp_header)
    {
        if (shouldDropSuspect(headers, false, false))
        {
            strcat(dynamicRule, headers.sourceIp);
            strcat(dynamicRule, ",drop");
            createIpRule(&drop_incoming_dynamic, true, dynamicRule, true);
            printk("adding dynamic rule: %s\n", dynamicRule);
            freeSuspiciousHost(headers.sourceIp);
            return NF_DROP;
        }
    }
         
    //printk(KERN_INFO  "activating firewall policy\n");    

    count_accepted_packets++;
    count_accepted_incoming_packets++;
    return NF_ACCEPT;
}

void freeSuspiciousHosts(void)
{
    struct list_head * pos, *q;
    struct suspicious_host * tmp;

    list_for_each_safe(pos, q, &suspect_list.list)
    {
        tmp = list_entry(pos, struct suspicious_host, list);
        {
            list_del(pos);
            kfree(tmp);
        }
    }
}

void freeList(struct firewall_rules* ruleList)
{
    struct list_head *pos, *q;
    struct firewall_rules *tmp;
    
    list_for_each_safe(pos, q, &ruleList->list)
    {
        tmp = list_entry(pos, struct firewall_rules, list);
        {
            list_del(pos);
            kfree(tmp);
        }
    }
}

void printPortRule(struct firewall_rules * rule)
{
    printk(KERN_INFO "the packet is filtered by port %d\n its source is %d\n and its id is %d", rule->port_parameter, rule->source, rule->id);
}


void printIpRule(struct firewall_rules * rule)
{
    printk(KERN_INFO "the packet is filtered by ip %s\n its source is %d\n and its id is %d", rule->ip_parameter, rule->source, rule->id);
}

void printProtocolRule(struct firewall_rules * rule)
{
    printk(KERN_INFO "the packet is filtered by port %sd\n its source is %d\n and its id is %d", rule->protocol_parameter, rule->source, rule->id);
}

void printList(struct firewall_rules * ruleList)
{
    struct list_head *pos;
    struct firewall_rules * rule;

    list_for_each(pos, &ruleList->list)
    {
        rule = list_entry(pos, struct firewall_rules, list);
        
        if (rule->type == PORT)
        {
            printPortRule(rule);
        }
        else if (rule->type == IP)
        {
            printIpRule(rule);
        }
        else
        {
            printProtocolRule(rule);
        }
    }
}   

void PrintRules(void)
{
    printk(KERN_INFO "printing accept rules for incoming packets:\n");
    printList(&accept_incoming_rules);
    printk(KERN_INFO "printing drop rules for incoming packets:\n");
    printList(&drop_incoming_rules);
    printk(KERN_INFO "printing accept rules for outgoing packets:\n");
    printList(&accept_outgoing_rules);
    printk(KERN_INFO "printing drop rules for outgoing packets:\n");
    printList(&drop_outgoing_rules);
}


void freeLists(void)
{    
    freeList(&accept_incoming_rules);
    freeList(&drop_incoming_rules);
    freeList(&accept_outgoing_rules);
    freeList(&drop_outgoing_rules);
    freeList(&drop_incoming_dynamic);
    freeSuspiciousHosts();
}

MODULE_LICENSE("GPL");

int init_module()
{

    proc_file_entry = proc_create(PROC_NAME, 0, NULL, &proc_fops);
   
    if (proc_file_entry == NULL)
    {
        printk(KERN_ALERT "failed creating proc file");
        return -ENOMEM;
    }

    db_proc_file = proc_create(DB_PROC_NAME, 0, NULL, &db_proc_fops);

    if (db_proc_file == NULL)
    {
        printk(KERN_ALERT "failed creating proc file");
        return -ENOMEM;
    }

    counter_proc_file = proc_create(COUNTER_PROC_NAME, 0, NULL, &counter_proc_fops);

    if (counter_proc_file == NULL)
    {
        printk(KERN_ALERT "failed creating proc file");
        return -ENOMEM;
    }
    
    printk(KERN_ALERT "Hello");
    
    //initializing incoming hook
    nfho.hook = hook_func;                       
    nfho.hooknum = NF_INET_PRE_ROUTING;                                                                   
    nfho.pf = PF_INET;                        
    nfho.priority = NF_IP_PRI_FIRST;
    
    //initializing outgoing hook
    nfho_out.hook = hook_out; 
    nfho_out.hooknum = NF_INET_LOCAL_OUT;
    nfho_out.pf = PF_INET;
    nfho_out.priority = NF_IP_PRI_FIRST;

    memset(msg, 0, MAX_PROC_SIZE);
    memset(db_msg, 0, MAX_PROC_SIZE);
    memset(reset_msg, 0, RESET_BUFF_MAX_LENGTH);
    
    INIT_LIST_HEAD(&accept_incoming_rules.list);
    INIT_LIST_HEAD(&drop_incoming_rules.list);
    INIT_LIST_HEAD(&accept_outgoing_rules.list);
    INIT_LIST_HEAD(&drop_outgoing_rules.list);
    INIT_LIST_HEAD(&drop_incoming_dynamic.list);
    INIT_LIST_HEAD(&suspect_list.list);
    nf_register_net_hook(&init_net, &nfho);
    nf_register_net_hook(&init_net, &nfho_out);
    
    return 0;                                    
}

void cleanup_module()
{
    remove_proc_entry(PROC_NAME, NULL);
    remove_proc_entry(DB_PROC_NAME, NULL);
    remove_proc_entry(COUNTER_PROC_NAME, NULL);
    printk(KERN_ALERT "Bye");
    nf_unregister_net_hook(&init_net, &nfho);
    nf_unregister_net_hook(&init_net, &nfho_out);
    freeLists();
}
