# Linked List Server

A Redis-style in-memory server where each key maps to a singly linked list of integers. Clients interact via a simple HTTP/JSON API. The server is implemented in C++17 with a `poll()`-based event loop and no external dependencies.

## Building

Requires **clang++** (or **g++**) with C++17 support.

```bash
make
```

This produces a binary called `listd` in the project root.

To clean build artefacts:

```bash
make clean
```

## Running

```bash
./listd [port]        # default port: 8080
```

Press **Ctrl+C** to stop the server.

## API Reference

All request and response bodies are JSON. Positions are **1-indexed**.

### List management

| Method | Path | Body | Description |
|--------|------|------|-------------|
| `POST` | `/lists` | `{"key": "name"}` | Create a new list |
| `GET` | `/lists` | — | Return all keys |
| `DELETE` | `/lists/:key` | — | Delete a list |
| `GET` | `/lists/:key` | — | Get all values in a list |
| `GET` | `/lists/:key/size` | — | Get the number of nodes |

### Head / tail operations

| Method | Path | Body | Description |
|--------|------|------|-------------|
| `POST` | `/lists/:key/head` | `{"value": 10}` | Push a value to the front |
| `DELETE` | `/lists/:key/head` | — | Pop a value from the front |
| `POST` | `/lists/:key/tail` | `{"value": 10}` | Push a value to the back |
| `DELETE` | `/lists/:key/tail` | — | Pop a value from the back |

### Position operations

| Method | Path | Body | Description |
|--------|------|------|-------------|
| `GET` | `/lists/:key/nodes/:pos` | — | Get value at position |
| `POST` | `/lists/:key/nodes/:pos` | `{"value": 10}` | Insert a new node **after** position |
| `PUT` | `/lists/:key/nodes/:pos` | `{"value": 10}` | Update the value at position |
| `DELETE` | `/lists/:key/nodes/:pos` | — | Remove the node at position |

### Sort

| Method | Path | Body | Description |
|--------|------|------|-------------|
| `POST` | `/lists/:key/sort` | `{"order": "asc"}` or `{"order": "desc"}` | Sort the list in place |

## Quick-start examples

```bash
# Create a list
curl -s -X POST http://localhost:8080/lists \
  -H 'Content-Type: application/json' \
  -d '{"key": "scores"}'

# Push values onto the tail
curl -s -X POST http://localhost:8080/lists/scores/tail \
  -H 'Content-Type: application/json' -d '{"value": 42}'
curl -s -X POST http://localhost:8080/lists/scores/tail \
  -H 'Content-Type: application/json' -d '{"value": 7}'
curl -s -X POST http://localhost:8080/lists/scores/head \
  -H 'Content-Type: application/json' -d '{"value": 99}'

# Inspect the list
curl -s http://localhost:8080/lists/scores
# {"key":"scores","values":[99,42,7],"size":3}

# Sort ascending
curl -s -X POST http://localhost:8080/lists/scores/sort \
  -H 'Content-Type: application/json' -d '{"order": "asc"}'
# {"key":"scores","values":[7,42,99],"size":3}

# Get the node at position 2
curl -s http://localhost:8080/lists/scores/nodes/2
# {"key":"scores","position":2,"value":42}

# Insert 50 after position 1
curl -s -X POST http://localhost:8080/lists/scores/nodes/1 \
  -H 'Content-Type: application/json' -d '{"value": 50}'

# Update position 3
curl -s -X PUT http://localhost:8080/lists/scores/nodes/3 \
  -H 'Content-Type: application/json' -d '{"value": 55}'

# Remove the node at position 2
curl -s -X DELETE http://localhost:8080/lists/scores/nodes/2

# Pop from the head
curl -s -X DELETE http://localhost:8080/lists/scores/head

# List all keys
curl -s http://localhost:8080/lists

# Delete the list
curl -s -X DELETE http://localhost:8080/lists/scores
```

## Architecture

```
src/
├── linked_list.h / .cpp   — Singly linked list (Node + LinkedList)
├── store.h / .cpp         — In-memory key → LinkedList store
├── json.h / .cpp          — JSON serialisation helpers
├── server.h / .cpp        — HTTP server, router, poll()-based event loop
├── api.h / .cpp           — REST API route definitions
└── main.cpp               — Entry point
Makefile
```

- **Event loop** — single-threaded, non-blocking I/O via `poll()`. One thread handles all connections; no mutexes needed.
- **Storage** — all data lives in a `std::unordered_map<std::string, LinkedList>` in process memory. Data is lost on restart (by design — this is an in-memory store).
- **Protocol** — HTTP/1.1 subset; connections are closed after each response.
