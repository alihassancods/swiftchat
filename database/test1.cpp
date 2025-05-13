#include <iostream>
#include <pqxx/pqxx>
// AI Generated Code - Generated with Chatgpt to test the connection with database
int main() {
    try {
        pqxx::connection conn("host=172.28.144.1 port=5432 dbname=postgres user=postgres password=1234");
        if (conn.is_open()) {
            std::cout << "Connected successfully 🚀\n";
        } else {
            std::cout << "Failed to connect ❌\n";
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
