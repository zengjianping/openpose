#include <openpose/filestream/messageIO.hpp>


int main(int argc, char *argv[])
{
    op::DataTransferIF::Option option;
    option.type_name = "udp";
    option.udp.ip_address = "127.0.0.1";
    option.udp.port_num = 18000;
    boost::shared_ptr<op::DataTransferIF> sender = op::DataTransferIF::CreateInstance(option, false);
    char* msg = "save";
    sender->Write((uint8_t*)msg, 4);
}

