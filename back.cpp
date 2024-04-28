#include <iostream>
#include <string>
#include <boost/asio.hpp>

using namespace boost::asio;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(ip::tcp::socket socket) : socket_(std::move(socket)) {}

    void start() {
        do_read();
    }

private:
    void do_read() {
        auto self(shared_from_this());
        async_read_until(socket_, dynamic_buffer(data_), '\n',
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    on_read(length);
                }
            });
    }

    void on_read(std::size_t length) {
        try {
            // Extract parameters from the URL
            size_t pos = data_.find('?');
            std::cout<<data_<<std::endl;
            std::cout<<pos<<std::endl;
            if (pos != std::string::npos) {
                std::string params = data_.substr(pos + 1);
                std::istringstream iss(params);

                int parameter1, parameter2;
                char separator;

                if (iss >> parameter1 >> separator >> parameter2 && separator == '&') {
                    // Process the parameters (calculate sum in this case)
                    int result = parameter1 + parameter2;

                    // Prepare the response
                    std::string response = std::to_string(result);

                    do_write(response);
                    return;
                }
            }

            // Invalid parameters
            do_write("Invalid parameters");
        } catch (const std::exception& e) {
            std::cerr << "Error processing parameters: " << e.what() << "\n";
            do_write("Error processing parameters");
        }
    }

    void do_write(const std::string& response) {
        auto self(shared_from_this());

        // Set CORS headers
        std::string headers =
            "HTTP/1.1 200 OK\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: " + std::to_string(response.length()) + "\r\n\r\n";

        // Write headers
        boost::asio::write(socket_, boost::asio::buffer(headers));

        // Write the actual response
        async_write(socket_, buffer(response.c_str(), response.length()),
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                if (ec) {
                    std::cerr << "Error sending response: " << ec.message() << "\n";
                }
            });
    }

    ip::tcp::socket socket_;
    std::string data_;
};

class Server {
public:
    Server(io_context& io_context, short port)
        : acceptor_(io_context, ip::tcp::endpoint(ip::tcp::v4(), port)),
          socket_(io_context) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(socket_,
            [this](boost::system::error_code ec) {
                if (!ec) {
                    std::make_shared<Session>(std::move(socket_))->start();
                }
                do_accept();
            });
    }

    ip::tcp::acceptor acceptor_;
    ip::tcp::socket socket_;
};

int main() {
    try {
        io_context io_context;

        // Run the server on port 8080
        Server server(io_context, 8080);

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
