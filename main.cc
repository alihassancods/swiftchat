#include "crow.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include "jwt-cpp/jwt.h"
#include <unordered_set>
#include <memory>
#include <mutex>
#include <random>

using namespace std;

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
    void signup() {}
    void login() {}
    ~Auth() {
        std::cout << "Authentication Object Shutting Down." << std::endl;
    }
};

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/communities")([]() {
        auto page = crow::mustache::load_text("communities-list.html");
        return page;
    });

    CROW_ROUTE(app, "/chat")([]() {
        auto page = crow::mustache::load_text("testConnection.html");
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
                if (session.jwtToken != sender_token) {
                    json broadcast_message;
                    broadcast_message["from"] = session.name;
                    broadcast_message["message"] = message_content;
                    session.connection->send_text(broadcast_message.dump());
                    std::cout << "Broadcasted message to: " << session.jwtToken << std::endl;
                } else {
                    std::cout << "Skipping sender session: " << session.jwtToken << std::endl;
                }
            }
        }
    });

    app.port(18080).multithreaded().run();
}
