#include <curl/curl.h>
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <regex>

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
            return textToTranslate;
        }

        string readBuffer;

        string postFields = std::string("auth_key=") + curl_easy_escape(curl, API_KEY.c_str(), 0) +
                    "&text=" + curl_easy_escape(curl, textToTranslate.c_str(), 0) +
                    "&target_lang=" + curl_easy_escape(curl, target_lang.c_str(), 0);

        curl_easy_setopt(curl, CURLOPT_URL, API_URL.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

        // Set write callback to capture response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
            return textToTranslate;
        }

        // Lightweight JSON parsing via regex (only for well-formed DeepL response)
        std::smatch match;
        std::regex rgx("\"text\":\"([^\"]+)\"");
        if (std::regex_search(readBuffer, match, rgx) && match.size() > 1) {
            return match[1].str();
        }

        std::cerr << "Failed to extract translated text from response:\n" << readBuffer << "\n";
        return textToTranslate;
    }

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
};

int main() {
    Translator translator("", "FR");
    string input = "Hello, how are you?";
    string output = translator.translate(input);

    std::cout << "Translated: " << output << std::endl;
    return 0;
}
