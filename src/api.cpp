#include "api.h"
#include "server.h"
#include "store.h"
#include "json.h"

#include <stdexcept>
#include <string>

void register_api_routes(Router& router, ListStore& store) {

    // ── List management ───────────────────────────────────────────────────────

    // POST /lists  {"key": "mylist"}
    router.post("/lists", [&store](const HttpRequest& req) {
        std::string key = json::get(req.body, "key");
        if (key.empty())
            return HttpResponse::bad_request("Missing required field: key");
        if (!store.create(key))
            return HttpResponse::conflict("Key already exists: " + key);
        return HttpResponse::created(json::make_created(key));
    });

    // GET /lists
    router.get("/lists", [&store](const HttpRequest& /*req*/) {
        return HttpResponse::ok(json::make_keys(store.keys()));
    });

    // DELETE /lists/:key
    router.del("/lists/:key", [&store](const HttpRequest& req) {
        const std::string& key = req.params.at("key");
        if (!store.drop(key))
            return HttpResponse::not_found("Key not found: " + key);
        return HttpResponse::no_content();
    });

    // GET /lists/:key
    router.get("/lists/:key", [&store](const HttpRequest& req) {
        const std::string& key = req.params.at("key");
        if (!store.exists(key))
            return HttpResponse::not_found("Key not found: " + key);
        return HttpResponse::ok(json::make_list(key, store.get(key).to_vector()));
    });

    // GET /lists/:key/size
    router.get("/lists/:key/size", [&store](const HttpRequest& req) {
        const std::string& key = req.params.at("key");
        if (!store.exists(key))
            return HttpResponse::not_found("Key not found: " + key);
        return HttpResponse::ok(json::make_size(key, store.get(key).size()));
    });

    // ── Head operations ───────────────────────────────────────────────────────

    // POST /lists/:key/head  {"value": 10}
    router.post("/lists/:key/head", [&store](const HttpRequest& req) {
        const std::string& key = req.params.at("key");
        if (!store.exists(key))
            return HttpResponse::not_found("Key not found: " + key);
        std::string val_str = json::get(req.body, "value");
        if (val_str.empty())
            return HttpResponse::bad_request("Missing required field: value");
        try {
            store.get(key).push_front(std::stoi(val_str));
            return HttpResponse::ok(json::make_list(key, store.get(key).to_vector()));
        } catch (const std::exception& e) {
            return HttpResponse::bad_request(e.what());
        }
    });

    // DELETE /lists/:key/head
    router.del("/lists/:key/head", [&store](const HttpRequest& req) {
        const std::string& key = req.params.at("key");
        if (!store.exists(key))
            return HttpResponse::not_found("Key not found: " + key);
        try {
            int value = store.get(key).pop_front();
            return HttpResponse::ok(json::make_popped(key, value));
        } catch (const std::exception& e) {
            return HttpResponse::bad_request(e.what());
        }
    });

    // ── Tail operations ───────────────────────────────────────────────────────

    // POST /lists/:key/tail  {"value": 10}
    router.post("/lists/:key/tail", [&store](const HttpRequest& req) {
        const std::string& key = req.params.at("key");
        if (!store.exists(key))
            return HttpResponse::not_found("Key not found: " + key);
        std::string val_str = json::get(req.body, "value");
        if (val_str.empty())
            return HttpResponse::bad_request("Missing required field: value");
        try {
            store.get(key).push_back(std::stoi(val_str));
            return HttpResponse::ok(json::make_list(key, store.get(key).to_vector()));
        } catch (const std::exception& e) {
            return HttpResponse::bad_request(e.what());
        }
    });

    // DELETE /lists/:key/tail
    router.del("/lists/:key/tail", [&store](const HttpRequest& req) {
        const std::string& key = req.params.at("key");
        if (!store.exists(key))
            return HttpResponse::not_found("Key not found: " + key);
        try {
            int value = store.get(key).pop_back();
            return HttpResponse::ok(json::make_popped(key, value));
        } catch (const std::exception& e) {
            return HttpResponse::bad_request(e.what());
        }
    });

    // ── Position-based operations (1-indexed) ─────────────────────────────────

    // GET /lists/:key/nodes/:pos
    router.get("/lists/:key/nodes/:pos", [&store](const HttpRequest& req) {
        const std::string& key = req.params.at("key");
        if (!store.exists(key))
            return HttpResponse::not_found("Key not found: " + key);
        try {
            int pos   = std::stoi(req.params.at("pos"));
            int value = store.get(key).get_at(pos);
            return HttpResponse::ok(json::make_node_value(key, pos, value));
        } catch (const std::exception& e) {
            return HttpResponse::bad_request(e.what());
        }
    });

    // POST /lists/:key/nodes/:pos  {"value": 10}  — insert after position
    router.post("/lists/:key/nodes/:pos", [&store](const HttpRequest& req) {
        const std::string& key = req.params.at("key");
        if (!store.exists(key))
            return HttpResponse::not_found("Key not found: " + key);
        std::string val_str = json::get(req.body, "value");
        if (val_str.empty())
            return HttpResponse::bad_request("Missing required field: value");
        try {
            int pos = std::stoi(req.params.at("pos"));
            store.get(key).insert_after(pos, std::stoi(val_str));
            return HttpResponse::ok(json::make_list(key, store.get(key).to_vector()));
        } catch (const std::exception& e) {
            return HttpResponse::bad_request(e.what());
        }
    });

    // PUT /lists/:key/nodes/:pos  {"value": 10}  — update at position
    router.put("/lists/:key/nodes/:pos", [&store](const HttpRequest& req) {
        const std::string& key = req.params.at("key");
        if (!store.exists(key))
            return HttpResponse::not_found("Key not found: " + key);
        std::string val_str = json::get(req.body, "value");
        if (val_str.empty())
            return HttpResponse::bad_request("Missing required field: value");
        try {
            int pos = std::stoi(req.params.at("pos"));
            store.get(key).update_at(pos, std::stoi(val_str));
            return HttpResponse::ok(json::make_list(key, store.get(key).to_vector()));
        } catch (const std::exception& e) {
            return HttpResponse::bad_request(e.what());
        }
    });

    // DELETE /lists/:key/nodes/:pos
    router.del("/lists/:key/nodes/:pos", [&store](const HttpRequest& req) {
        const std::string& key = req.params.at("key");
        if (!store.exists(key))
            return HttpResponse::not_found("Key not found: " + key);
        try {
            int pos   = std::stoi(req.params.at("pos"));
            int value = store.get(key).remove_at(pos);
            return HttpResponse::ok(json::make_removed(key, value));
        } catch (const std::exception& e) {
            return HttpResponse::bad_request(e.what());
        }
    });

    // ── Sort ──────────────────────────────────────────────────────────────────

    // POST /lists/:key/sort  {"order": "asc"|"desc"}
    router.post("/lists/:key/sort", [&store](const HttpRequest& req) {
        const std::string& key = req.params.at("key");
        if (!store.exists(key))
            return HttpResponse::not_found("Key not found: " + key);
        bool ascending = (json::get(req.body, "order") != "desc");
        store.get(key).sort(ascending);
        return HttpResponse::ok(json::make_list(key, store.get(key).to_vector()));
    });
}
