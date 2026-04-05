#include "server.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>

static constexpr int kMaxEvents = 1024;

// ─── HttpResponse ─────────────────────────────────────────────────────────────

static const char* http_status_text(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 409: return "Conflict";
        case 500: return "Internal Server Error";
        default:  return "Unknown";
    }
}

std::string HttpResponse::serialize() const {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status << " " << http_status_text(status) << "\r\n"
        << "Content-Type: " << content_type << "\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << body;
    return oss.str();
}

HttpResponse HttpResponse::ok(const std::string& b) {
    HttpResponse r; r.status = 200; r.body = b; return r;
}
HttpResponse HttpResponse::created(const std::string& b) {
    HttpResponse r; r.status = 201; r.body = b; return r;
}
HttpResponse HttpResponse::no_content() {
    HttpResponse r; r.status = 204; return r;
}
HttpResponse HttpResponse::bad_request(const std::string& msg) {
    HttpResponse r; r.status = 400; r.body = "{\"error\":\"" + msg + "\"}"; return r;
}
HttpResponse HttpResponse::not_found(const std::string& msg) {
    HttpResponse r; r.status = 404; r.body = "{\"error\":\"" + msg + "\"}"; return r;
}
HttpResponse HttpResponse::conflict(const std::string& msg) {
    HttpResponse r; r.status = 409; r.body = "{\"error\":\"" + msg + "\"}"; return r;
}
HttpResponse HttpResponse::internal_error(const std::string& msg) {
    HttpResponse r; r.status = 500; r.body = "{\"error\":\"" + msg + "\"}"; return r;
}

// ─── Router ───────────────────────────────────────────────────────────────────

void Router::add(const std::string& method, const std::string& pattern, Handler handler) {
    Route route;
    route.method  = method;
    route.handler = handler;

    std::istringstream ss(pattern);
    std::string seg;
    while (std::getline(ss, seg, '/')) {
        if (seg.empty()) continue;
        if (seg[0] == ':') {
            route.segments.push_back(seg);
            route.is_param.push_back(true);
            route.param_names.push_back(seg.substr(1));
        } else {
            route.segments.push_back(seg);
            route.is_param.push_back(false);
            route.param_names.push_back("");
        }
    }
    routes_.push_back(std::move(route));
}

void Router::get(const std::string& p, Handler h)  { add("GET",    p, h); }
void Router::post(const std::string& p, Handler h) { add("POST",   p, h); }
void Router::put(const std::string& p, Handler h)  { add("PUT",    p, h); }
void Router::del(const std::string& p, Handler h)  { add("DELETE", p, h); }

bool Router::match(const Route& route,
                   const std::string& method,
                   const std::string& path,
                   std::unordered_map<std::string, std::string>& params) const {
    if (route.method != method) return false;

    std::vector<std::string> path_segs;
    {
        std::istringstream ss(path);
        std::string seg;
        while (std::getline(ss, seg, '/')) {
            if (seg.empty()) continue;
            auto q = seg.find('?');
            if (q != std::string::npos) seg = seg.substr(0, q);
            path_segs.push_back(seg);
        }
    }

    if (route.segments.size() != path_segs.size()) return false;

    params.clear();
    for (size_t i = 0; i < route.segments.size(); ++i) {
        if (route.is_param[i]) {
            params[route.param_names[i]] = path_segs[i];
        } else if (route.segments[i] != path_segs[i]) {
            return false;
        }
    }
    return true;
}

HttpResponse Router::dispatch(const HttpRequest& req) const {
    for (const auto& route : routes_) {
        std::unordered_map<std::string, std::string> params;
        if (match(route, req.method, req.path, params)) {
            HttpRequest enriched = req;
            enriched.params = params;
            try {
                return route.handler(enriched);
            } catch (const std::exception& e) {
                return HttpResponse::internal_error(e.what());
            }
        }
    }
    return HttpResponse::not_found("No route matched: " + req.method + " " + req.path);
}

// ─── HttpServer ───────────────────────────────────────────────────────────────

static void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags >= 0) fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

HttpServer::HttpServer(const std::string& host, int port)
    : host_(host), port_(port), listen_fd_(-1), poller_fd_(-1), running_(false), router_(nullptr) {}

HttpServer::~HttpServer() {
    if (poller_fd_ >= 0) { close(poller_fd_); poller_fd_ = -1; }
    if (listen_fd_ >= 0) { close(listen_fd_); listen_fd_ = -1; }
}

void HttpServer::use(Router& router) {
    router_ = &router;
}

void HttpServer::setup_socket() {
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0)
        throw std::runtime_error(std::string("socket() failed: ") + strerror(errno));

    int opt = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(static_cast<uint16_t>(port_));
    addr.sin_addr.s_addr = inet_addr(host_.c_str());

    if (bind(listen_fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
        throw std::runtime_error(std::string("bind() failed: ") + strerror(errno));

    if (listen(listen_fd_, 128) < 0)
        throw std::runtime_error(std::string("listen() failed: ") + strerror(errno));

    set_nonblocking(listen_fd_);
}

bool HttpServer::try_parse_request(const std::string& raw, HttpRequest& req) {
    size_t hdr_end = raw.find("\r\n\r\n");
    if (hdr_end == std::string::npos) return false;

    size_t line_end = raw.find("\r\n");
    {
        std::istringstream rls(raw.substr(0, line_end));
        std::string version;
        rls >> req.method >> req.path >> version;
    }
    if (req.method.empty()) return false;

    auto q = req.path.find('?');
    if (q != std::string::npos) req.path = req.path.substr(0, q);

    size_t pos = line_end + 2;
    while (pos < hdr_end) {
        size_t end = raw.find("\r\n", pos);
        if (end == std::string::npos) break;
        std::string hdr = raw.substr(pos, end - pos);
        size_t colon = hdr.find(':');
        if (colon != std::string::npos) {
            std::string k = hdr.substr(0, colon);
            std::string v = hdr.substr(colon + 1);
            std::transform(k.begin(), k.end(), k.begin(), ::tolower);
            while (!v.empty() && (v[0] == ' ' || v[0] == '\t')) v.erase(0, 1);
            req.headers[k] = v;
        }
        pos = end + 2;
    }

    size_t body_start = hdr_end + 4;
    auto it = req.headers.find("content-length");
    if (it != req.headers.end()) {
        size_t content_len = static_cast<size_t>(std::stoi(it->second));
        if (raw.size() < body_start + content_len) return false;
        req.body = raw.substr(body_start, content_len);
    }

    return true;
}

// ─── epoll helpers ────────────────────────────────────────────────────────────

void HttpServer::setup_poller() {
    poller_fd_ = epoll_create1(EPOLL_CLOEXEC);
    if (poller_fd_ < 0)
        throw std::runtime_error(std::string("epoll_create1() failed: ") + strerror(errno));
}

void HttpServer::poller_watch_read(int fd) {
    struct epoll_event ev{};
    ev.events  = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    epoll_ctl(poller_fd_, EPOLL_CTL_ADD, fd, &ev);
}

void HttpServer::poller_watch_write(int fd) {
    struct epoll_event ev{};
    ev.events  = EPOLLOUT | EPOLLET;
    ev.data.fd = fd;
    epoll_ctl(poller_fd_, EPOLL_CTL_MOD, fd, &ev);
}

void HttpServer::poller_remove(int fd) {
    epoll_ctl(poller_fd_, EPOLL_CTL_DEL, fd, nullptr);
}

// ─── Event loop ───────────────────────────────────────────────────────────────
//
// Edge-triggered epoll: the kernel notifies us once per state change, so every
// read/write handler must drain its fd fully (loop until EAGAIN) before
// returning control to epoll_wait.
//
//  1. listen_fd_ fires EPOLLIN → accept() loop until EAGAIN; each new client
//     fd is set non-blocking and registered for reads.
//  2. Client fd fires EPOLLIN → recv() loop until EAGAIN; once a complete HTTP
//     request is buffered, dispatch it, store the response, re-arm for writes.
//  3. Client fd fires EPOLLOUT → send() loop until EAGAIN or buffer empty;
//     empty buffer means the response is fully delivered → close.

void HttpServer::run() {
    setup_socket();
    setup_poller();
    running_ = true;
    poller_watch_read(listen_fd_);

    std::cout << "Server listening on " << host_ << ":" << port_ << "\n"
              << "Press Ctrl+C to stop.\n";

    std::vector<struct epoll_event> events(kMaxEvents);

    while (running_) {
        int nready = epoll_wait(poller_fd_, events.data(), kMaxEvents, 500 /*ms*/);
        if (nready < 0) {
            if (errno == EINTR) continue;
            break;
        }

        for (int i = 0; i < nready; ++i) {
            int  fd       = events[i].data.fd;
            bool is_read  = events[i].events & (EPOLLIN | EPOLLERR | EPOLLHUP);
            bool is_write = events[i].events & EPOLLOUT;

            if (fd == listen_fd_) {
                int cfd;
                while ((cfd = accept(listen_fd_, nullptr, nullptr)) >= 0) {
                    set_nonblocking(cfd);
                    connections_[cfd] = {cfd, {}, {}};
                    poller_watch_read(cfd);
                }
                continue;
            }

            bool close_conn = false;

            if (is_read) {
                auto& conn = connections_[fd];
                char buf[4096];
                ssize_t r;
                while ((r = recv(fd, buf, sizeof(buf), 0)) > 0)
                    conn.recv_buf.append(buf, static_cast<size_t>(r));

                if (r == 0 || (r < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                    close_conn = true;
                } else {
                    HttpRequest req;
                    if (try_parse_request(conn.recv_buf, req)) {
                        HttpResponse resp = router_
                            ? router_->dispatch(req)
                            : HttpResponse::internal_error("No router configured");
                        conn.send_buf = resp.serialize();
                        conn.recv_buf.clear();
                        poller_watch_write(fd);
                    }
                }
            }

            if (is_write && !close_conn) {
                auto& conn = connections_[fd];
                ssize_t w;
                while (!conn.send_buf.empty()) {
                    w = send(fd, conn.send_buf.data(), conn.send_buf.size(), 0);
                    if (w > 0) {
                        conn.send_buf.erase(0, static_cast<size_t>(w));
                    } else if (w < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                        break;
                    } else {
                        close_conn = true;
                        break;
                    }
                }
                if (conn.send_buf.empty()) close_conn = true;
            }

            if (close_conn) {
                poller_remove(fd);
                close(fd);
                connections_.erase(fd);
            }
        }
    }

    for (auto& [fd, _] : connections_) close(fd);
    connections_.clear();
    if (poller_fd_ >= 0) { close(poller_fd_); poller_fd_ = -1; }
    if (listen_fd_ >= 0) { close(listen_fd_); listen_fd_ = -1; }
}

void HttpServer::stop() {
    running_ = false;
}
