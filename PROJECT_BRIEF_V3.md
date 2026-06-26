# Distributed Key-Value Store — Project Brief
> Paste this at the start of every new Claude session to restore full context.

---

## 👤 About the Developer
- **C++ experience:** Built a multi-threaded HTTP/1.1 server and a Redis-like
  in-memory cache server from scratch.
- **Known concepts:** Classes, OOP, RAII, smart pointers, sockets, threads,
  mutexes, condition variables, singletons, CMake, POSIX APIs, binary
  serialization, move semantics, std::chrono, unit testing with Catch2.
- **Daily time:** 1–2 hours
- **Goal:** 7-year level C++ engineering. Distributed systems knowledge.
- **OS:** Mac (Apple Silicon, clang 17)

---

## 🎯 Project Goal

Build a **distributed key-value store** in C++17 where multiple server nodes
replicate data between each other. If one node goes down, the others keep
serving reads and writes. Clients can connect to any node.

Simulated locally — 3 nodes on localhost, different ports:
```
Node 1 → localhost:8081  (can be leader)
Node 2 → localhost:8082  (follower)
Node 3 → localhost:8083  (follower)
```

By the end it will:
- Elect a leader automatically using a simplified Raft-like protocol
- Replicate writes from leader to followers
- Serve reads from any node
- Survive a node crash and recover when it restarts
- Handle network partition scenarios (simulated)

---

## 🆕 New C++ Concepts This Project Introduces
| Concept | Why |
|---------|-----|
| Templates (properly) | Generic message types, reusable network layer |
| `std::variant` | Represent different message/command types cleanly |
| Lock-free data structures | High-throughput replication log |
| `std::future` / `std::promise` | Async operations with timeouts |
| Custom serialization protocol | Node-to-node binary messaging |
| Exception handling | Network failures, partial writes |
| Strategy pattern | Pluggable consensus algorithms |
| Integration testing | Multi-process test harness |

---

## 📋 Conventions (same as before — non-negotiable)
| What | Rule |
|------|------|
| Classes | `PascalCase` → `RaftNode`, `ReplicationLog` |
| Functions / methods | `camelCase` → `appendEntry()`, `requestVote()` |
| Member variables | `m_` prefix → `m_nodeId`, `m_currentTerm` |
| Constants | `ALL_CAPS` → `ELECTION_TIMEOUT_MS` |
| Files | `snake_case` → `raft_node.cpp` |
| Headers | Always `#pragma once` |
| Memory | No raw `new`/`delete`. Smart pointers only. |
| Namespace | Never `using namespace std;` |
| Function length | Max ~40 lines. One function = one job. |
| Compile flags | Always `-Wall -Wextra -std=c++17` |

---

## 🗂️ Target Project Structure
```
distributed-kv/
├── CMakeLists.txt
├── PROJECT_BRIEF_V3.md
├── README.md
├── node.config             # Per-node configuration
├── src/
│   ├── main.cpp            # Entry point — starts a node
│   ├── kv_store.cpp        # Local key-value store (from cache project)
│   ├── raft_node.cpp       # Core Raft consensus logic
│   ├── replication_log.cpp # Append-only log of commands
│   ├── peer_connection.cpp # TCP connection to another node
│   ├── peer_manager.cpp    # Manages all peer connections
│   ├── client_handler.cpp  # Handles client requests
│   ├── rpc_server.cpp      # Listens for node-to-node RPCs
│   ├── message.cpp         # Message serialization/deserialization
│   └── logger.cpp          # Reuse from previous project
└── include/
    ├── kv_store.h
    ├── raft_node.h
    ├── replication_log.h
    ├── peer_connection.h
    ├── peer_manager.h
    ├── client_handler.h
    ├── rpc_server.h
    ├── message.h
    └── logger.h
```

---

## 🗺️ Phases Overview
| Phase | Module | Key concepts |
|-------|--------|-------------|
| 0 | Project setup + local KV store | Reuse, CMake multi-target |
| 1 | Node identity + peer connections | Node IDs, peer TCP connections |
| 2 | Replication log | Append-only log, log entries, `std::variant` |
| 3 | Leader election (simplified Raft) | Terms, votes, election timeouts, `std::future` |
| 4 | Log replication | AppendEntries RPC, commit index, followers |
| 5 | Fault tolerance | Node crash/restart, log recovery, catch-up |
| 6 | Client routing | Forward writes to leader, reads from any node |
| 7 | Integration tests + polish | Multi-process test harness, README, benchmarks |

---

## ✅ Current Status

**Current Phase:** 1
**Status:** In Progress

### Prerequisites (complete before Day 1)
- [X ] Read the Raft paper summary — search "Raft consensus algorithm explained simply"
      Read until you understand: terms, leader election, log replication. Not the
      full paper — just enough to have a mental model. 30 minutes.
- [ X] Push cache-server project to GitHub if not done
- [X ] Create `distributed-kv/` folder
- [X ] Understand what an RPC is — "Remote Procedure Call explained"
      5 minutes. You'll use this concept heavily.

---

## 📝 Design Decisions Log
*(filled as we go)*

| Decision | Reason |
|----------|--------|
| Simplified Raft over Paxos | Raft is designed to be understandable; Paxos is notoriously hard |
| Simulate locally on different ports | No cloud cost, identical network behavior |
| Reuse KV store from cache project | Build on what works, focus on distribution |
| Binary RPC protocol | Consistent with persistence layer you already know |
|Single binary with CLI arguments for node identity | maintaing multiple binaries are complex and not standard |
| KvStore stripped of TTL  |  durable store, not a cache |
---

## ⚠️ Mistakes / Rethinks Log
*(filled as we go)*

---

## 🔖 C++ Concepts — Status
| Concept | Introduced in Phase | Understood? |
|---------|--------------------:|-------------|
| Templates (writing your own) | 2 | — |
| `std::variant` | 2 | — |
| `std::future` / `std::promise` | 3 | — |
| Lock-free structures | 4 | — |
| Exception handling | 5 | — |
| Integration testing | 7 | — |

---

## 📌 How to Use This File
1. **Save it** in your `distributed-kv/` folder as `PROJECT_BRIEF_V3.md`
2. **Update it daily** — tick off tasks, fill in logs
3. **Paste it at the top** of every new Claude session
4. Start with: *"Here is my project brief. I want to continue."*

---

## 💼 Why This Project for 7-Year Level

Distributed systems is the hardest and most valued domain in backend
engineering. Understanding Raft — how nodes agree on a leader, how writes
are safely replicated, how the system recovers from failures — puts you in
a small category of engineers who truly understand what happens underneath
tools like etcd, CockroachDB, and Kafka.

Interview topics this directly prepares you for:
- "How does leader election work?"
- "What is the CAP theorem and where does your system sit?"
- "How do you handle split-brain scenarios?"
- "Walk me through what happens when a follower falls behind."

Combined with your HTTP server and cache server, you now have a portfolio
that spans: networking → caching → distributed consensus. That arc tells
a clear story of someone who understands systems at depth.

---

## 📚 Key Concepts to Know Before Starting

**Term** — a logical clock in Raft. Increments every election. Every
message carries the sender's term. Higher term always wins.

**Leader** — the one node that accepts writes. Replicates to followers.

**Follower** — passive node. Accepts entries from leader. Votes in elections.

**Candidate** — a follower that timed out waiting for the leader and started
an election. Asks others for votes.

**Log entry** — one command (e.g. SET name John) with a term number and index.

**Commit** — an entry is committed once a majority of nodes have it. Only
committed entries are applied to the KV store.

**Heartbeat** — leader sends periodic empty AppendEntries to followers to
prove it's alive and reset their election timers.

---
*Last updated: Day 1 - In Progess*
