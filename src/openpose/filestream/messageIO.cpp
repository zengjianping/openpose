#include <openpose/filestream/messageIO.hpp>
#include <deque>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
using namespace boost::asio;


namespace op
{

class DataTransferUDP : public DataTransferIF
{
public:
    DataTransferUDP(const std::string& remote_ip, int socket_port, bool is_server)
    {
        remote_ip_ = remote_ip;
        if (is_server) {
            recv_port_ = socket_port;
            send_port_ = socket_port + 1;
        }
        else {
            recv_port_ = socket_port + 1;
            send_port_ = socket_port;
        }
        is_server_ = is_server;
        ip::udp::endpoint ep = ip::udp::endpoint(ip::udp::v4(), recv_port_);
        socket_ = boost::make_shared<ip::udp::socket>(io_service_, ep);
        thread_receive_ = std::thread([&] { DataTransferUDP::receive_data(); });
    }

    ~DataTransferUDP()
    {
        io_service_.stop();
        thread_receive_.join();
    }

public:
    bool IsOpen() override
    {
        return socket_->is_open();
    }

    int Write(const uint8_t *data, int datlen) override
    {
        int num_bytes = -1;

        ip::udp::socket socket(io_service_, ip::udp::v4());
        struct timeval tv{1,0};
        setsockopt(socket.native_handle(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        setsockopt(socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        ip::udp::endpoint ep = ip::udp::endpoint(ip::address::from_string(remote_ip_), send_port_);

        boost::system::error_code error;
        num_bytes = socket.send_to(boost::asio::buffer(data, datlen), ep, 0, error);
        std::string name = is_server_ ? "Server" : "Client";
        if (error) {
            std::cout << name << " sending failed: " << error.message() << std::endl;
        }
        else {
            std::cout << name << " sent " << num_bytes << " bytes." << std::endl;
        }
        socket.close();

        return num_bytes;
    }

    int Read(uint8_t *data, int max_datlen) override
    {
        int num_bytes = 0;

        data_mutex.lock();
        while (!recv_data_.empty() && num_bytes < max_datlen) {
            data[num_bytes++] = recv_data_.front();
            recv_data_.pop_front();
        }
        data_mutex.unlock();

        return num_bytes;
    }

private:
    void receive_data()
    {
        start_receive();

        std::string name = is_server_ ? "Server" : "Client";
        std::cout << name << " receiving data on port(" << recv_port_ << ")..." << std::endl;

        io_service_.run();

        std::cout << name << "Receiving ended!" << std::endl;
    }

    void start_receive()
    {
        ip::udp::endpoint remote_endpoint;
        socket_->async_receive_from(
            boost::asio::buffer(recv_buff_), remote_endpoint,
            boost::bind(&DataTransferUDP::handle_receive, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred)
    {
        std::string name = is_server_ ? "Server" : "Client";
        if (error) {
            std::cout << name << " receiving failed: " << error.message() << std::endl;
            return;
        }
        std::cout << name << " received bytes: " << bytes_transferred << " (" << error.message() << ")" << std::endl;

        if (bytes_transferred) {
            data_mutex.lock();
            for (int i = 0; i < bytes_transferred; i++) {
                recv_data_.push_back(recv_buff_[i]);
            }
            data_mutex.unlock();
        }

        start_receive();
    }

protected:
	std::string remote_ip_;
    int recv_port_, send_port_;
    bool is_server_;
    boost::asio::io_service io_service_;
    boost::shared_ptr<ip::udp::socket> socket_;
    boost::array<uint8_t, 1024*16> recv_buff_;
    std::deque<uint8_t> recv_data_;
    std::mutex data_mutex;
    std::thread thread_receive_;
};


class DataTransferTCP : public DataTransferIF
{
public:
    DataTransferTCP(const std::string& remote_ip, int socket_port, bool is_server)
    {
        remote_ip_ = remote_ip;
        socket_port_ = socket_port;
        is_server_ = is_server;
        is_stopped_ = false;
        thread_receive_ = std::thread([&] { receive_data(); });
    }

    ~DataTransferTCP()
    {
        is_stopped_ = true;
        io_service_.stop();
        thread_receive_.join();
    }

public:
    bool IsOpen() override
    {
        return socket_->is_open();
    }

    int Write(const uint8_t *data, int datlen) override
    {
        int num_bytes = -1;

        if (socket_.get()) {
            boost::system::error_code error;
            num_bytes = socket_->send(boost::asio::buffer(data, datlen), 0, error);
            std::string name = is_server_ ? "Server" : "Client";
            if (error) {
                std::cout << name << " sending failed: " << error.message() << std::endl;
            }
            else {
                std::cout << name << " sent " << num_bytes << " bytes." << std::endl;
            }
        }

        return num_bytes;
    }

    int Read(uint8_t *data, int max_datlen) override
    {
        int num_bytes = 0;

        data_mutex.lock();
        while (!recv_data_.empty() && num_bytes < max_datlen) {
            data[num_bytes++] = recv_data_.front();
            recv_data_.pop_front();
        }
        data_mutex.unlock();

        return num_bytes;
    }

private:
    void receive_data()
    {
        std::string name = is_server_ ? "Server" : "Client";
        std::cout << name << " receiving data...\n";
        std::chrono::duration<int,std::milli> wait_time(1000);

        while (true) {
            if (is_stopped_) {
                break;
            }
            if (!is_server_) {
                if (!socket_.get()) {
                    if (assure_connect()) {
                        start_receive();
                    }
                }
            }
            else {
                if (!acceptor_.get()) {
                    acceptor_ = boost::make_shared<ip::tcp::acceptor>(io_service_, ip::tcp::endpoint(ip::tcp::v4(), socket_port_));
                    start_accept();
                }
            }
            io_service_.run_for(wait_time);
            //std::this_thread::sleep_for(wait_time);
        }
    
        std::cout << name << " receiving exit!" << std::endl;
    }

    void start_accept()
    {
        auto socket = boost::make_shared<ip::tcp::socket>(io_service_);
        acceptor_->async_accept(*socket, boost::bind(&DataTransferTCP::handle_accept, this, placeholders::error, socket));
    }

    void handle_accept(boost::system::error_code ec, boost::shared_ptr<ip::tcp::socket> socket)
    {
        if (!ec) {
            const ip::tcp::endpoint& ep = socket->remote_endpoint();
            std::cout << "Connected client on: " << ep.address().to_string() << ":" << ep.port() << '\n';
            socket_ = socket;
            start_receive();
        }
        else {
            std::cout << "Acceptor error: " << ec.message() << std::endl;
        }

        start_accept();
    }

    bool assure_connect()
    {
        boost::shared_ptr<ip::tcp::socket> socket = boost::make_shared<ip::tcp::socket>(io_service_, ip::tcp::v4());
        ip::tcp::endpoint ep(ip::address::from_string(remote_ip_), socket_port_);
        boost::system::error_code error;

        socket->connect(ep, error);
        if (error) {
            //std::cout << "Connecting failed: " << error.message() << std::endl;
            return false;
        }

        struct timeval tv{1,0};
        setsockopt(socket->native_handle(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        setsockopt(socket->native_handle(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        const ip::tcp::endpoint& ep2 = socket->remote_endpoint();
        std::cout << "Connected server on: " << ep2.address().to_string() << ":" << ep2.port() << '\n';

        socket_ = socket;
        return true;
    }

    void start_receive()
    {
        socket_->async_receive(
            boost::asio::buffer(recv_buff_),
            boost::bind(&DataTransferTCP::handle_receive, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred)
    {
        std::string name = is_server_ ? "Server" : "Client";
        if (error) {
            std::cout << name << " receiving failed: " << error.message() << std::endl;
            return;
        }
        std::cout << name << " received bytes: " << bytes_transferred << " (" << error.message() << ")" << std::endl;

        if (bytes_transferred > 0) {
            data_mutex.lock();
            for (int i = 0; i < bytes_transferred; i++) {
                recv_data_.push_back(recv_buff_[i]);
            }
            data_mutex.unlock();
        }
        start_receive();
    }

protected:
	std::string remote_ip_;
    int socket_port_;
    bool is_server_;
    bool is_stopped_;

    boost::asio::io_service io_service_;
    boost::shared_ptr<ip::tcp::acceptor> acceptor_;
    boost::shared_ptr<ip::tcp::socket> socket_;
    boost::array<uint8_t, 1024*16> recv_buff_;
    std::deque<uint8_t> recv_data_;
    std::mutex data_mutex;
    std::thread thread_receive_;
};


boost::shared_ptr<DataTransferIF> DataTransferIF::CreateInstance(const DataTransferIF::Option& option, bool is_server)
{
    boost::shared_ptr<DataTransferIF> instance;
    if (option.type_name == "udp") {
        instance = boost::make_shared<DataTransferUDP>(option.udp.ip_address, option.udp.port_num, is_server);
    }
    else if (option.type_name == "tcp") {
        instance = boost::make_shared<DataTransferTCP>(option.udp.ip_address, option.udp.port_num, is_server);
    }
    return instance;
}

}

