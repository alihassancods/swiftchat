//  g++ -std=c++17 -o websocket_app main.cc -lcrypto -pthread -lpqxx -lpq
#include "crow.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include "jwt-cpp/jwt.h"
#include <unordered_set>
#include<pqxx/pqxx>
#include <memory>
#include <mutex>
#include <random>

using namespace std;
class Translator{
    private:
        string API_KEY;
    public:
        string translate(string textToTranslate){
            // logic implemented here to translate
        }
}
class DatabaseController{
    private:
        string username;
        string dbname;
        string password;
        string server_addr;
    public:
        DatabaseController(string uname, string dbasename,string pwd,string serveraddr){
            username = uname;
            dbname=dbasename;
            password = pwd;
            server_addr = serveraddr;
            try {
                pqxx::connection conn("host=172.28.144.1 port=5432 dbname=postgres user=postgres password=1234");
                if (conn.is_open()) {
                    std::cout << "Connected successfully 🚀\n";
                    pqxx::work txn(conn);
                    txn.exec("CREATE TABLE IF NOT EXISTS students(id SERIAL PRIMARY KEY, name TEXT)");

                    // Commit transaction
                    txn.commit();

                    std::cout << "Table created successfully.\n";
                    conn.disconnect();
                } else {
                    std::cout << "Failed to connect ❌\n";
                }
            } catch (const std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
        }
        ~DatabaseController(){
            std::cout << "Connection Stopped" << std::endl;        
        }
        void create_table(string tableNm){
            // code to create table in database
        }
        void fetch_table_data(string tableNm){
            // code to create table in database
        }
        void insert_data(string tableNm,string data){
            // code to insert data into table in database
        }
        void delete_table(string tableNm){
            // code to create table in database
        }
};
// Struct to hold connection, token, and name
struct Session {
    crow::websocket::connection* connection;
    std::string jwtToken;
    std::string name;

    bool operator==(const Session& other) const {
        return jwtToken == other.jwtToken;
    }
};

// Custom hash for Session
struct SessionHash {
    std::size_t operator()(const Session& s) const {
        return std::hash<std::string>()(s.jwtToken);
    }
};

// Global sessions set (thread-safe with mutex)
std::unordered_set<Session, SessionHash> sessions;
std::mutex sessions_mutex;

class Auth {
private:
    string username;
    string password;
    string sessionToken;
public:
    Auth() {
        std::cout << "Authentication Object initialized" << std::endl;
    }
    void signup(string emailValue, string passwordValue) {
        // Go and Make the entry with this email and set it as password as well
        // Generate its unique token as well

    }
    void login() {}
    ~Auth() {
        std::cout << "Authentication Object Shutting Down." << std::endl;
    }
};

int main() {
    crow::SimpleApp app;
    CROW_ROUTE(app, "/signup")([]() {
        auto page = crow::mustache::load_text("signup.html");
        return page;
    });
    CROW_ROUTE(app, "/login")([]() {
        auto page = crow::mustache::load_text("login.html");
        return page;
    });
    CROW_ROUTE(app, "/profile")([]() {
        auto page = crow::mustache::load_text("communities-list.html");
        return page;
    });
    CROW_ROUTE(app, "/communities")([]() {
        auto page = crow::mustache::load_text("communities-list.html");
        return page;
    });

    CROW_ROUTE(app, "/chat")([]() {
        auto page = crow::mustache::load_text("index.html");
        return page;
    });

    CROW_WEBSOCKET_ROUTE(app, "/ws")
    .onopen([&](crow::websocket::connection& conn) {
        std::cout << "Opened the connection" << std::endl;
        conn.userdata(nullptr);  // not used, but called
    })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t code) {
        std::cout << "Closed the connection: " << reason << std::endl;
        std::lock_guard<std::mutex> lock(sessions_mutex);
        for (auto it = sessions.begin(); it != sessions.end(); ) {
            if (it->connection == &conn) {
                std::cout << "Removing session for token: " << it->jwtToken << std::endl;
                it = sessions.erase(it);
            } else {
                ++it;
            }
        }
    })
    .onmessage([&](crow::websocket::connection& conn, const std::string& message, bool is_binary) {
        std::cout << "Received message: " << message << std::endl;

        json message_data;
        try {
            message_data = json::parse(message);
        } catch (const json::exception& e) {
            std::cerr << "Error parsing message: " << e.what() << std::endl;
            return;
        }

        if (message_data.contains("type")) {
            std::string token = jwt::create()
                                    .set_payload_claim("connection_id", jwt::claim(std::to_string(rand())))
                                    .sign(jwt::algorithm::hs256{"secret"});

            std::string username = message_data.value("name", "anonymous");

            // Store the session
            {
                std::lock_guard<std::mutex> lock(sessions_mutex);
                sessions.insert({&conn, token, username});
            }

            json broadcast_message;
            broadcast_message["from"] = "server";
            broadcast_message["token"] = token;
            broadcast_message["name"] = username;
            broadcast_message["type"] = "token";

            std::cout << "Sending initial token to create session " << token << std::endl;
            conn.send_text(broadcast_message.dump());
        } else if (message_data.contains("message")) {
            std::string message_content = message_data["message"];
            std::string sender_token = message_data["token"];

            std::lock_guard<std::mutex> lock(sessions_mutex);
            for (const auto& session : sessions) {
                if (session.jwtToken == sender_token) {
                    std::cout << "Skipping sender session: " << session.jwtToken << std::endl;
                    
                } else {
                    json broadcast_message;
                    broadcast_message["from"] = session.name;
                    broadcast_message["message"] = message_content;
                    session.connection->send_text(broadcast_message.dump());
                    std::cout << "Broadcasted message to: " << session.jwtToken << std::endl;
                }
            }
        }
    });

    app.port(18080).multithreaded().run();
}
