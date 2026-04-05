#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

// ─── HTTP primitives ──────────────────────────────────────────────────────────

struct HttpRequest {
    std::string method;
    std::string path;
    std::string body;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> params; // named path parameters
};

struct HttpResponse {
    int status = 200;
    std::string content_type = "application/json";
    std::string body;

    std::string serialize() const;

    static HttpResponse ok(const std::string& body);
    static HttpResponse created(const std::string& body);
    static HttpResponse no_content();
    static HttpResponse bad_request(const std::string& msg);
    static HttpResponse not_found(const std::string& msg = "Not found");
    static HttpResponse conflict(const std::string& msg);
    static HttpResponse internal_error(const std::string& msg = "Internal server error");
};

using Handler = std::function<HttpResponse(const HttpRequest&)>;

// ─── Router ───────────────────────────────────────────────────────────────────

class Router {
public:
    void get(const std::string& pattern, Handler handler);
    void post(const std::string& pattern, Handler handler);
    void put(const std::string& pattern, Handler handler);
    void del(const std::string& pattern, Handler handler); // "delete" is a keyword

    HttpResponse dispatch(const HttpRequest& req) const;

private:
    struct Route {
        std::string method;
        std::vector<std::string> segments;
        std::vector<bool> is_param;
        std::vector<std::string> param_names;
        Handler handler;
    };

    void add(const std::string& method, const std::string& pattern, Handler handler);
    bool match(const Route& route,
               const std::string& method,
               const std::string& path,
               std::unordered_map<std::string, std::string>& params) const;

    std::vector<Route> routes_;
};

// ─── HTTP server (epoll, Linux only) ─────────────────────────────────────────

class HttpServer {
public:
    HttpServer(const std::string& host, int port);
    ~HttpServer();

    void use(Router& router);
    void run();
    void stop();

private:
    std::string host_;
    int port_;
    int listen_fd_;
    int poller_fd_; // epoll fd
    bool running_;
    Router* router_;

    struct Connection {
        int fd;
        std::string recv_buf;
        std::string send_buf;
    };

    std::unordered_map<int, Connection> connections_;

    void setup_socket();
    void setup_poller();
    void poller_watch_read(int fd);
    void poller_watch_write(int fd);
    void poller_remove(int fd);
    bool try_parse_request(const std::string& raw, HttpRequest& req);
};
