#include "UserSpace/HttpPacket.h"

#include <stdexcept>

HttpPacket::HttpPacket(PacketData* data)
    : Packet(data)
{
    if (!IsHttp(data))
    {
        throw std::runtime_error("Not HTTP/S packet");
    }

    _hostName = ExtractHostName();
}

HttpPacket::~HttpPacket()
{    
}

std::string HttpPacket::ExtractHostName()
{
    if (_data->data != nullptr)
    {                
        std::string hostData(_data->data);
        std::size_t findHost = hostData.find("Host:");

        if (findHost != std::string::npos)
        {
            std::size_t hostnameIndex = findHost + 6; // TOOD(eran): Make it space independent
            _hostName = hostData.substr(hostnameIndex, hostData.find("\n", hostnameIndex) - hostnameIndex - 1);
            return _hostName;
        }
    }
    return "no host";
}

bool HttpPacket::IsHttp(PacketData* data)
{
    if (data->incomming)
    {
        return ((data->srcPort == HTTP) || (data->srcPort == HTTPS));
    }    
    return ((data->dstPort == HTTP) || (data->dstPort == HTTPS));
}

std::string HttpPacket::GetHostName()
{
    return _hostName;
}



