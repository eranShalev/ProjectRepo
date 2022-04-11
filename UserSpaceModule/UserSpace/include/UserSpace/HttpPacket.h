#include "Packet.h"

#include <string>

#define HTTP 80
#define HTTPS 443

class HttpPacket : public Packet
{
 public:
    HttpPacket(PacketData* data);
    ~HttpPacket();
    
    std::string GetHostName();

 private:
    static bool IsHttp(PacketData* data);
    std::string ExtractHostName();
    
    std::string _hostName;
};
