#include <openpose/filestream/messageIO.hpp>
#include <fstream>


int main(int argc, char *argv[])
{
    op::DataTransferIF::Option option;
    option.type_name = "udp";
    //option.udp.ip_address = "192.168.10.255";
    option.udp.ip_address = "127.0.0.1";
    option.udp.port_num = 18089;
    boost::shared_ptr<op::DataTransferIF> sender = op::DataTransferIF::CreateInstance(option, false);
    std::string message = "ShotID";

    if (argc > 1) {
        std::ifstream fs(argv[1]);
        if (fs.is_open()) {
            message.clear();
            fs >> message;
        }
    }

    sender->Write((uint8_t*)message.c_str(), message.length());
}

