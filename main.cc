//  g++ -std=c++17 -o websocket_app main.cc -lcurl -lcrypto -pthread -lpqxx -lpq 
// net start postgresql-x64-17
#include "crow.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include "jwt-cpp/jwt.h"
#include <fstream>
#include <sstream>
#include <unordered_set>
#include<pqxx/pqxx>
#include <memory>
#include <mutex>
#include <random>
#include <stdexcept>
#include <regex>
#include<cstdlib>
#include <curl/curl.h>

using namespace std;

std::string getEnvValue(const std::string& key) {
    std::ifstream file("data.env"); // Assuming the env file is named ".env" and is in the same directory
    if (!file.is_open()) {
        std::cerr << "Error: Could not open .env file." << std::endl;
        return "";
    }

    std::string line;
    while (std::getline(file, line)) {
        // Ignore comments and empty lines
        if (line.empty() || line[0] == '#') continue;

        std::size_t delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos) continue;

        std::string fileKey = line.substr(0, delimiterPos);
        std::string fileValue = line.substr(delimiterPos + 1);

        // Trim whitespace (optional, improves robustness)
        fileKey.erase(0, fileKey.find_first_not_of(" \t"));
        fileKey.erase(fileKey.find_last_not_of(" \t") + 1);
        fileValue.erase(0, fileValue.find_first_not_of(" \t"));
        fileValue.erase(fileValue.find_last_not_of(" \t") + 1);

        if (fileKey == key) {
            std::cout << fileValue << std::endl;
            return fileValue;
        }
    }

    return ""; // Key not found
}

std::string IP_ADDR = getEnvValue("IP_ADDR"); 
std::string DEEPL_API_TOKEN = getEnvValue("API_KEY");

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <regex>
#include <nlohmann/json.hpp>

using std::string;

class Translator {
private:
    string API_KEY;
    string API_URL = "https://api-free.deepl.com/v2/translate";
    string target_lang = "ES"; // Default to Spanish

public:
    Translator(const string& api_key, const string& target_language = "ES")
        : API_KEY(api_key), target_lang(target_language) {}

    void set_target_language(const string& lang) {
        target_lang = lang;
    }

    string translate(const string& textToTranslate) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cerr << "Failed to initialize CURL\n";
            return "[Error]";
        }

        string readBuffer;

        char* escapedText = curl_easy_escape(curl, textToTranslate.c_str(), 0);
char* escapedLang = curl_easy_escape(curl, target_lang.c_str(), 0);

string postFields = "text=" + std::string(escapedText) +
                    "&target_lang=" + std::string(escapedLang);

// Clean up memory allocated by curl_easy_escape
curl_free(escapedText);
curl_free(escapedLang);

        // Set up headers
        struct curl_slist* headers = nullptr;
        string authHeader = "Authorization: DeepL-Auth-Key " + API_KEY;
        headers = curl_slist_append(headers, authHeader.c_str());
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

        curl_easy_setopt(curl, CURLOPT_URL, API_URL.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

        // Set write callback
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
            return "[Error]";
        }

        try {
            auto json_response = nlohmann::json::parse(readBuffer);
            return json_response["translations"][0]["text"];
        } catch (...) {
            std::cerr << "Failed to parse JSON response:\n" << readBuffer << "\n";
            return "[Parse Error]";
        }
    }

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
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
    // string username;
    // string password;
    // string sessionToken;
    std::string extract_username(string email) {
    return email.substr(0, email.find('@'));
}
std::string generate_jwt(const std::string &email, const std::string &secret, bool permanent) {
    auto token = jwt::create()
        .set_issuer("auth-service")
        .set_type("JWS")
        .set_subject(email)
        .set_payload_claim("perm", jwt::claim(std::string(permanent ? "1" : "0")))
        .set_issued_at(std::chrono::system_clock::now())
        .sign(jwt::algorithm::hs256{secret});
    return token;
}

std::string generate_session_token(const std::string &email, const std::string &secret) {
    auto session_token = jwt::create()
        .set_issuer("auth-service")
        .set_type("JWS")
        .set_subject(email)
        .set_payload_claim("perm", jwt::claim(std::string("0")))  // session token
        .set_issued_at(std::chrono::system_clock::now())
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::minutes{60})
        .sign(jwt::algorithm::hs256{secret});
    return session_token;
}
public:
    Auth() {
        std::cout << "Authentication Object initialized" << std::endl;
    }
    string signup(string emailValue, string passwordValue) {
        try{
            pqxx::connection conn("host=" + IP_ADDR + " port=5432 dbname=swiftchat user=postgres password=1234");
            string username = extract_username(emailValue);
            string master_token = generate_jwt(emailValue,passwordValue,true);
            cout << master_token << endl;
            pqxx::work txn(conn);
        txn.exec_params(
            "INSERT INTO users (email, password, username, master_token) VALUES ($1, $2, $3, $4)",
            emailValue, passwordValue, username, master_token
        );

        txn.commit();
        std::cout << "User signed up successfully.\n";
        std::cout << "Permanent token (store securely):\n" << master_token << std::endl;
        return "success";
        } catch (const std::exception &e) {
         return "Signup failed: $1",e.what();
    }
        
    }
    string login(string emailValue, string passwordValue) {
        try{
            pqxx::connection conn("host=" + IP_ADDR + " port=5432 dbname=swiftchat user=postgres password=1234");
            pqxx::work txn(conn);
            pqxx::result r = txn.exec_params(
            "SELECT * FROM users WHERE email = $1", emailValue
        );
        std::string stored_password = r[0]["password"].as<std::string>();
        std::string masterToken = r[0]["master_token"].as<std::string>();
        if (r.empty()) {
            std::cout << "Login failed: user not found.\n";
            return "Login Failed";
        }
        if (passwordValue != stored_password){
            return "Invalid Credentials";
        }
        std::string session_token = generate_session_token(emailValue, masterToken);
        std::cout << "Login successful.\nSession token:\n" << session_token << std::endl;
        return session_token;
        } catch (const std::exception &e) {
        std::cerr << "Login error: " << e.what() << '\n';
        return "LoginError";
    }
    }
    ~Auth() {
        std::cout << "Authentication Object Shutting Down." << std::endl;
    }
};

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
            server_addr = system("hostname -I");
            std::cout << server_addr << std::endl;
            try {
                pqxx::connection conn("host=" + IP_ADDR + " port=5432 dbname=swiftchat user=postgres password=1234");
                if (conn.is_open()) {
                    std::cout << "Connected successfully 🚀\n";
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


int main() {
    crow::SimpleApp app;
    Auth auth_controller;
    DatabaseController controller("postgres","swiftchat","1234","");
    CROW_ROUTE(app, "/signup")([]() {
        auto page = crow::mustache::load_text("signup.html");

        // Create a response object and set Content-Type header to text/html
        crow::response res;
        res.set_header("Content-Type", "text/html");
        
        // Return the response with the rendered HTML page
        res.write(page);
        
        return res;
    });

    CROW_ROUTE(app, "/login")([]() {
        auto page = crow::mustache::load_text("login.html");

        // Create a response object and set Content-Type header to text/html
        crow::response res;
        res.set_header("Content-Type", "text/html");
        
        // Return the response with the rendered HTML page
        res.write(page);
        
        return res;
    });

    CROW_ROUTE(app, "/profile")([]() {
        auto page = crow::mustache::load_text("communities-list.html");
        return page;
    });

    CROW_ROUTE(app, "/communities")([]() {
        auto page = crow::mustache::load_text("communities-list.html");
        return page;
    });

    CROW_ROUTE(app, "/chat")([](const crow::request& req) {
        // Load the login.html template
        auto page = crow::mustache::load_text("index.html");

        // Create a response object and set Content-Type header to text/html
        crow::response res;
        res.set_header("Content-Type", "text/html");
        
        // Return the response with the rendered HTML page
        res.write(page);
        
        return res;
    });

    CROW_ROUTE(app, "/translate")
    ([&](const crow::request& req) -> crow::response{
        std::cout << "Got the request" << std::endl;
        try {
            // Parse the body using nlohmann::json
            // json request_json = json::parse(req.body);
            // std::string text = request_json.at("text").get<std::string>();
            auto text_param = req.url_params.get("text");
             if (!text_param) {
            return crow::response(400, "Missing 'text' parameter.");
        }
            Translator translator((string)DEEPL_API_TOKEN,"EN");
            std::string translated_text = translator.translate(text_param);
            std::cout << "Translated into English : " << translated_text << std::endl;
            // Build response using nlohmann::json (optional)
            json response_json = {
                {"status", "success"},
                {"text", translated_text}
            };
            crow::response res;
            res.code = 200;
            res.set_header("Content-Type", "application/json");
            res.set_header("Access-Control-Allow-Origin", "*");

            res.body = response_json.dump();
            return res;
        }
        catch (const std::exception& e) {
            return crow::response(400, std::string("Invalid request: ") + e.what());
        }
    });

    CROW_ROUTE(app, "/login").methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req) -> crow::response {
        try {
            // Parse the body using nlohmann::json
            json request_json = json::parse(req.body);

            // Extract fields
            std::string email = request_json.at("email").get<std::string>();
            std::string password = request_json.at("password").get<std::string>();

            CROW_LOG_INFO << "Received login for user: " << email;
            string token = auth_controller.login(email,password);
            string username = "";
            crow::response response;

            if(token == "Invalid Credentials"){
                std::cout << "Invalid Credentials" << std::endl;
                json response_json = {
                {"status", "invalid"}
            };
            response.set_header("Content-Type", "application/json");
                response.body = response_json.dump();
            }
            else{
                std::cout << "User successfully logged in" << std::endl;
                json response_json = {
                {"status", "success"},
                {"user", username},
                {"session_token",token}
            };
                response.code = 200;
                response.set_header("Content-Type", "application/json");
                response.body = response_json.dump();
            }
            // Build response using nlohmann::json (optional)
                return response;
        }
        catch (const std::exception& e) {
    return crow::response(400, std::string("Error: ") + e.what());
}
    });

    CROW_ROUTE(app, "/signup").methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req) -> crow::response {
        try {
            // Parse the body using nlohmann::json
            json request_json = json::parse(req.body);

            // Extract fields
            std::string email = request_json.at("email").get<std::string>();
            std::string password = request_json.at("password").get<std::string>();

            CROW_LOG_INFO << "Received signup for user: " << email;
            auth_controller.signup(email,password);
            
            // Build response using nlohmann::json (optional)
            json response_json = {
                {"status", "success"}
            };
            crow::response res;
            res.code = 200;
            res.set_header("Content-Type", "application/json");
            res.body = response_json.dump();
            return res;
        }
        catch (const std::exception& e) {
            return crow::response(400, std::string("Invalid request: ") + e.what());
        }
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
