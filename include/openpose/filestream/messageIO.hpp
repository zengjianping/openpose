#ifndef OPENPOSE_MESSAGE_IO_HPP
#define OPENPOSE_MESSAGE_IO_HPP

#include <openpose/core/common.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>


namespace op
{

class DataTransferIF
{
public:
	struct Option {
		std::string type_name;
		struct {
			std::string ip_address;
			int port_num;
		} udp;

	} option;

	static boost::shared_ptr<DataTransferIF> CreateInstance(const Option& option, bool is_server);

public:
    virtual bool IsOpen() = 0;
    virtual int Write(const uint8_t *data, int datlen) = 0;
    virtual int Read(uint8_t *data, int max_datlen) = 0;
};

}

#endif // OPENPOSE_MESSAGE_IO_HPP

