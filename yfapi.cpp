#include <iostream>
#include <string>
#include <sstream>
#include <curl/curl.h>
#include "httplib.h"
#include <fstream>
#include <map> // Used for dynamic URL construction
#include <chrono>

using namespace std;


using Clock = std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

// --- GLOBAL URL MAP ---
// The URL map is defined here, with placeholders for easy substitution.
//
const string country="US";
const string language="en";
const string count="10";
const string timeRange="1m";
const string screenerType ="day_gainers";


const string modules = "assetProfile,summaryProfile,summaryDetail,esgScores,price,incomeStatementHistory,incomeStatementHistoryQuarterly,balanceSheetHistory,balanceSheetHistoryQuarterly,cashflowStatementHistory,cashflowStatementHistoryQuarterly,defaultKeyStatistics,financialData,calendarEvents,secFilings,recommendationTrend,upgradeDowngradeHistory,institutionOwnership,fundOwnership,majorDirectHolders,majorHoldersBreakdown,insiderTransactions,insiderHolders,netSharePurchaseActivity,earnings,earningsHistory,earningsTrend,industryTrend,indexTrend,sectorTrend";

//
//TYPES="balanceSheet"
const map<string, string> YAHOO_URLS = {
    {"quote", "https://query2.finance.yahoo.com/v7/finance/quote?symbols={symbol}{crumb}"},
    {"trending", "https://query2.finance.yahoo.com/v1/finance/trending/{country}?count={count}{crumb}"},
    {"screener", "https://query1.finance.yahoo.com/ws/screeners/v1/finance/screener/predefined/saved?count={count}&scrIds={screenerType}{crumb}"},
    {"recommendations", "https://query2.finance.yahoo.com/v6/finance/recommendationsbysymbol/{symbol}{crumb}"},
    {"insights", "https://query1.finance.yahoo.com/ws/insights/v1/finance/insights?symbol={symbol}{crumb}"},
    {"marketsummary", "https://query1.finance.yahoo.com/v6/finance/quote/marketSummary?lang={language}&region={country}{crumb}"},
    {"autocomplete", "https://query1.finance.yahoo.com/v6/finance/autocomplete?region={country}&lang={language}&query={searchTerm}{crumb}"},
    {"chart", "https://query1.finance.yahoo.com/v8/finance/chart/{symbol}?range={timeRange}&interval={timeInterval}{crumb}"},
    {"sparkchart", "https://query2.finance.yahoo.com/v8/finance/spark?interval={timeInterval}&range={timeRange}&symbols={symbols}{crumb}"},
    {"quotesummary", "https://query2.finance.yahoo.com/v10/finance/quoteSummary/{symbol}?lang={language}&region={country}&modules={modules}{crumb}"},
    {"realtimequote", "https://query2.finance.yahoo.com/v7/finance/quote?region={country}&lang={language}&symbols={symbols}{crumb}"},
    {"fundamentals", "https://query2.finance.yahoo.com/ws/fundamentals-timeseries/v1/finance/timeseries/{symbol}?&type={types}&period1={timestampStart}&period2={timestampEnd}"},
    {"news", "https://query2.finance.yahoo.com/v1/finance/search?q={symbol}{crumb}"},
    {"options", "https://query1.finance.yahoo.com/v7/finance/options/{symbol}{crumb}"},
    {"options_with_date", "https://query2.finance.yahoo.com/v7/finance/options/{symbol}?date={expirationTimestamp}{crumb}"},
    {"historical_prices", "https://query1.finance.yahoo.com/v7/finance/download/{symbol}?period1={timestampStart}&period2={timestampEnd}&interval={timeInterval}&events=history{crumb}"},
    {"search_quotes", "https://query1.finance.yahoo.com/v1/finance/search?q={searchTerm}&quotesCount=6&newsCount=4{crumb}"},
};

// --- UTILITY FUNCTIONS ---

// Crumb reading function is good, keeping it as is.
std::string read_crumb(const std::string& filename) {
    std::ifstream file(filename);
    std::string crumb_value;

    if (file.is_open() && std::getline(file, crumb_value)) {
        crumb_value.erase(0, crumb_value.find_first_not_of(" \t\n\r"));
        crumb_value.erase(crumb_value.find_last_not_of(" \t\n\r") + 1);
        // Note: Returning "&crumb=" prefix here is very specific to appending to a URL.
        return "&crumb=" + crumb_value;
    }
    return "";
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Helper to replace all occurrences of a substring
void replace_all(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty()) return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); 
    }
}

// --- CORE FETCH FUNCTION ---

std::string fetch_yahoo_finance_data(
    const std::string& base_url,
    const std::string& crumb,
    const std::string& cookie_file,
    const std::string& user_agent,
    const std::string& symbol,
    const httplib::Params& query_params)// Use httplib's parameter type for flexibility
{
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    std::string url = base_url;

    // 1. Substitute placeholders in the URL string
    // This makes the function universal for all URLs
    replace_all(url, "{symbol}", symbol);
    replace_all(url, "{crumb}", crumb);
    replace_all(url, "{modules}", modules);
    replace_all(url, "{language}", language);
    replace_all(url, "{country}", country);
    replace_all(url, "{count}", ::count);
    replace_all(url, "{timeRange}", timeRange);
    replace_all(url, "{screenerType}", screenerType);

    
    // Substitute all other URL parameters from the request query
    for (const auto& pair : query_params) {
        replace_all(url, "{" + pair.first + "}", pair.second);
    }

    // 2. Perform the cURL request (similar to your original code)
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookie_file.c_str());

        struct curl_slist *headers = NULL;
        std::string user_agent_header = "User-Agent: " + user_agent;
        headers = curl_slist_append(headers, user_agent_header.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::stringstream error_ss;
            error_ss << "curl_easy_perform() failed: " << curl_easy_strerror(res) << " for URL: " << url;
            readBuffer = "{\"__curl_fetch_error__\": \"" + error_ss.str() + "\"}";
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        readBuffer = "{\"error\": \"libcurl initialization failed\"}";
    }

    curl_global_cleanup();

    return readBuffer;
}

// --- MAIN SERVER LOGIC ---

int main() {
    using namespace httplib;
    Server svr;

    // --- Configuration ---
    const std::string CRUMB_PREFIX = read_crumb("./yahoo_crumb.txt");
    const std::string COOKIE_FILE = "./yahoo_cookies.txt";
    const std::string USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36";

    if (CRUMB_PREFIX.empty()) {
        std::cerr << "FATAL: Could not read valid crumb. Please ensure ./yahoo_crumb.txt exists and has content." << std::endl;
        return 1;
    }
    auto add_cors_headers = [](Response& res) {
          res.set_header("Access-Control-Allow-Origin", "*");
          res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
          res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
      };
    // --- Route 1: Universal Ticker Data (e.g., /quote/TSLA) ---
    // Matches routes like: /quote/TSLA, /chart/GOOG, /news/MSFT, etc.
    svr.Get(R"(/(\w+)/(\w+))", [&](const Request& req, Response& res) {
        // req.matches[1] is the data type (e.g., "quote", "chart")
        // req.matches[2] is the stock symbol (e.g., "TSLA", "GOOG")
        auto start_time = Clock::now();
        add_cors_headers(res);
        std::string data_type = req.matches[1];
        std::string ticker = req.matches[2];

        // 1. Check if the requested data type is supported
        if (YAHOO_URLS.find(data_type) == YAHOO_URLS.end()) {
            res.status = 404;
            res.set_content("{\"error\": \"Invalid data type requested. Try /quote, /chart, etc.\"}", "application/json");
            return;
        }

        std::cout << "Received request for " << data_type << ": " << ticker << std::endl;

        // 2. Fetch the data using the flexible function
        std::string json_data = fetch_yahoo_finance_data(
            YAHOO_URLS.at(data_type), // Get the base URL from the map
            CRUMB_PREFIX,             // Use the read crumb
            COOKIE_FILE,
            USER_AGENT,
            ticker,
            req.params                 // Pass all query parameters (e.g., ?range=1d&interval=5m)
        );

        // 3. Set response
        res.set_content(json_data, "application/json");
        res.status = (json_data.find("\"__curl_fetch_error__\"") != std::string::npos) ? 503 : 200;
        auto end_time = Clock::now();
        auto time_span = end_time - start_time;
        auto duration_ms = duration_cast<milliseconds>(time_span);
        // --- CORRECTED LINE ---
        std::cout << "Request [" << data_type << "] completed in: " 
                  << duration_ms.count() << " ms" << std::endl;

    });

    // --- Route 2: Trending/Market Data (e.g., /trending) ---
    // Matches routes that don't need a symbol (e.g., /trending, /marketsummary)
    svr.Get(R"(/(\w+))", [&](const Request& req, Response& res) {
        auto start_time = Clock::now();

        std::string data_type = req.matches[1];
        add_cors_headers(res);
        // Only handle types that don't require a symbol. Check against a predefined list.
        if (data_type == "trending" || data_type == "marketsummary") {
            std::cout << "Received request for market data: " << data_type << std::endl;
            std::string json_data = fetch_yahoo_finance_data(
                YAHOO_URLS.at(data_type),
                CRUMB_PREFIX,
                COOKIE_FILE,
                USER_AGENT,
                "", // Empty symbol
                req.params
            );

            res.set_content(json_data, "application/json");
            res.status = (json_data.find("\"error\"") != std::string::npos) ? 503 : 200;
            
            auto end_time = Clock::now();
            auto time_span = end_time - start_time;
            auto duration_ms = duration_cast<milliseconds>(time_span);
            
            // --- CORRECTED LINE ---
            std::cout << "Request [" << data_type << "] completed in: " 
                      << duration_ms.count() << " ms" << std::endl;
            // ----------------------
            
            return;
        }
        // If it was another path, let it fall through to the default or 404
    });

    // Default route for testing
    svr.Get("/", [](const Request&, Response& res) {
        res.set_content(
        "<h1>Server is running.</h1>"
        "<p>Try the following endpoints:</p>"
        "<ul>"
            // Clickable link: Clicking this will navigate to /quote/TSLA
            "<li><a href=\"/quote/TSLA\">/quote/TSLA</a></li>"
            "<li><a href=\"/chart/MSFT?range=1d&interval=5m\">/chart/MSFT?range=1d&interval=5m</a></li>"
            "<li><a href=\"/news/msft\">/news/msft</a></li>"
            "<li><a href=\"/insights/MSFT\">/insights/MSFT</a></li>"
            "<li><a href=\"/marketsummary\">/marketsummary</a></li>"
            "<li><a href=\"/quotesummary/v\">/quotesummary/V</a></li>"
        "</ul>",
      "text/html");
    });

    // Start the server
    std::cout << "Starting server on http://localhost:8082" << std::endl;
    if (svr.listen("0.0.0.0", 8082)) {
        std::cout << "Server stopped." << std::endl;
    } else {
        std::cerr << "Failed to start server." << std::endl;
    }

    return 0;
}
