#include "crow.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include "jwt-cpp/jwt.h"
#include <unordered_map>
#include <mutex>
#include <random>
// #include<pqxx/pqxx>
    

using namespace std;

// class DatabaseController{
//     private:
//         string username;
//         string dbname;
//         string password;
//         string server_addr;
//     public:
//         DatabaseController(string uname, string dbasename,string pwd,string serveraddr){
//             username = uname;
//             dbname=dbasename;
//             password = pwd;
//             server_addr = serveraddr;
//             try {
//                 pqxx::connection conn("host= port=5432 dbname=postgres user=postgres password=1234");
//                 if (conn.is_open()) {
//                     std::cout << "Connected successfully 🚀\n";
//                 } else {
//                     std::cout << "Failed to connect ❌\n";
//                 }
//             } catch (const std::exception &e) {
//                 std::cerr << e.what() << std::endl;
//             }
//         }
//         ~DatabaseController(){
//             std::cout << "Connection Stopped" << std::endl;        
//         }
//         void create_table(string tableNm){
//             // code to create table in database
//         }
//         void fetch_table_data(string tableNm){
//             // code to create table in database
//         }
//         void insert_data(string tableNm,string data){
//             // code to insert data into table in database
//         }
//         void delete_table(string tableNm){
//             // code to create table in database
//         }
// };

class Auth{
    private:
        string username;
        string password;
        string sessionToken;
    public:
        Auth(){
            std::cout << "Authentication Object initialized" << std::endl;        
        }
        void signup(){
            // code to register the user
        }
        void login(){
            // code to register the user
        }
        ~Auth(){
            std::cout << "Authentication Object Shutting Down." << std::endl;        
        }
};


set<string> sessions;  // To store JWT tokens for each connection



int main() {
    crow::SimpleApp app;
    CROW_ROUTE(app, "/communities")([](){
        auto page = crow::mustache::load_text("communities-list.html");
        return page;
    });
    CROW_ROUTE(app, "/chat")([](){
        auto page = crow::mustache::load_text("testConnection.html");
        return page;
    });
    CROW_WEBSOCKET_ROUTE(app, "/ws")
    .onopen([&](crow::websocket::connection& conn) {
        std::cout << "Opened the connection" << std::endl;

        // Generate a JWT token for the new connection
        auto token = jwt::create()
                         .set_payload_claim("connection_id", jwt::claim(std::to_string(rand())))
                         .sign(jwt::algorithm::hs256{"secret"});

        sessions.insert(token);  // Store the token in the set

        // Send the JWT token to the client
        json response;
        response["token"] = token;
        conn.send_text(response.dump());  // Send the JSON response (serialized)
    })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t code) {
        std::cout << "Closed the connection" << std::endl;
    })
    .onmessage([&](crow::websocket::connection& conn, const std::string& message, bool is_binary) {
        std::cout << "Received message: " << message << std::endl;

        // Parse the incoming message
        json message_data;
        try {
            message_data = json::parse(message);
        } catch (const json::exception& e) {
            std::cerr << "Error parsing message: " << e.what() << std::endl;
            return;
        }

        if (message_data.contains("message")) {
            string message_content = message_data["message"];
            // Broadcast the message to all connected clients
            for (const auto& session_token : sessions) {
                // Prepare the message to send to each client
                
                if(session_token!=message_data["token"]){
                    json broadcast_message;
                    broadcast_message["from"] = "server";
                    broadcast_message["message"] = message_content;
                    std::cout << "Broadcasting message to session: " << session_token << std::endl;
                conn.send_text(broadcast_message.dump());
                }
                else{
                    std::cout << "Dont send it to same shit" << std::endl;
                }
                
                
                // Here, we send the broadcast message to each connection in `sessions`
                // Note: You will need to keep track of connections to actually send the message
                 // Example. Adjust for real connections.
            }
        }
    });

    // Run the app on port 18080 with multiple threads
    app.port(18080).multithreaded().run();
}

    
