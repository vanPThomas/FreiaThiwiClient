# 𐍆𐍂𐌴𐌹𐌰 𐌸𐌹𐍅𐌹 — Freia Thiwi Client

**freia thiwi — “servant of freedom”**
**Version: 0.6.0 (development release)**

The lightweight C++20 Linux desktop client for **Freia Thiwi**, a self-hosted encrypted messaging project. It uses Dear ImGui, GLFW, and OpenGL for a graphical chat interface, with password-based encryption handled in the client.

> *“The server serves the users. The messages belong to the users.”*

Freia Thiwi is built with privacy, modest hardware requirements, and hands-on systems programming in mind. This is the **client** repository. The companion repository is [vanPThomas/FreiaThiwi](https://github.com/vanPThomas/FreiaThiwi).

## What's included in 0.6.0

- Graphical login and account creation, main chat, available-room list, and chatroom tabs.
- Create a password-protected room and automatically join it; join an existing room with its password.
- View the current online users in each room, send/receive room messages, and leave a room without disconnecting from the server.
- Receive recent persisted encrypted messages on joining a room.
- Main-chat and room-message encryption happens on the client; only the encrypted room message bodies are sent to the server.
- Background socket receiver handles incoming protocol packets; UI snapshots are protected by mutexes.

This release completes the initial **`PROT5` chatroom workflow** (creation, joining, messaging, live member lists, leaving, and SQLite persistence). Server and client **v0.6.0 should be upgraded together**: the room protocol changed from previous development versions.

## How the system works

Three independent passwords have different roles:

| Password | Purpose | Server access |
|---|---|---|
| Server password | Derives the transport key for all framed packets | Server and clients share it |
| Account password | Derives account credentials for `PROT4` registration/login | Server stores a Base64-encoded derived key for comparison; not the typed password |
| Main-chat / room password | Derives client-side keys to encrypt chat content | Server relays content without the message-decryption key; each room has its own password |

These mechanisms are **not equivalent to a formally verified or audited secure messenger**; read [Security status](#security-status) before relying on the program for sensitive communications.

### TCP packet framing

Each TCP packet consists of a **4-byte network-order unsigned length** followed by that many bytes of transport-encrypted data. After transport decryption the frame starts with a protocol name; most fields are newline-separated. The server has a 64 KiB transport-packet limit in v0.6.0.

### Protocols

| Protocol | Direction | Function |
|---|---|---|
| `PROT1` | Client → server → other clients | Main/public-chat messages; body separately encrypted by sender |
| `PROT2` | Client ↔ server | Server-password transport handshake and display-name claim |
| `PROT3` | Server → client(s) | Server-wide presence, room-name lists, and notices |
| `PROT4` | Client ↔ server | Account `CREATE` / `LOGIN`; `SUCCESS` / `FAIL` |
| `PROT5` | Client ↔ server | Persistent password-protected chatrooms |

A typical session is: encrypted `PROT2` handshake → `PROT4` account authentication → `PROT3` online-user and room lists → optional `PROT1` main chat or `PROT5` room actions. A `PROT2` display-name claim alone is **not** a logged-in account; the server requires successful `PROT4` authentication before room operations.

### PROT5 chatrooms

Client requests (inside the transport-encrypted frame):

```text
PROT5
CREATE|CONNECT
<room_name>
<username>
<room_login_token_base64>
```

```text
PROT5
LEAVE
<room_name>
```

```text
PROT5
MESSAGE
<room_name>
<message_ciphertext_base64>
```

Server responses/events include:

| Response | Purpose |
|---|---|
| `PROT5 ROOM` | Room metadata and the initial online-user snapshot; sent on successful join |
| `PROT5 USERS` | Complete replacement online-user list (`room_name`, count, usernames) |
| `PROT5 MESSAGE` | Room-only ciphertext broadcast; also used to replay recent stored history |
| `PROT5 LEFT` | Confirms leaving a room |
| `PROT5 ERROR` | Reports a room-specific error (including wrong password) |

Creating a room automatically joins its creator. `ROOM` is sent before history and `USERS` updates. Messages are Base64-encoded *inside* the newline-delimited protocol so encrypted binary data cannot break parsing. Messages are routed only to currently joined sockets; disconnecting removes the socket from live room membership, and remaining participants receive updated user lists. Joining again requires the room password.

### Client-side room keys

Each room's message-encryption key is derived locally from its password. For room creation and joining, the client produces a separate HMAC-based login token from that key; it does **not** transmit the AES room key as the room credential. The server checks the token against a salted stored verifier. A client must know the room password to decrypt messages returned from persistent history.

Room names, online presence, and account identity are available to the server; **message content is encrypted separately**. The application has no built-in secure invitation/password-exchange workflow in v0.6.0.

## Build and run (Linux)

**Dependencies:** CMake 3.16+, a C++20 compiler, OpenSSL development headers, GLFW3, OpenGL development headers, and `pkg-config`. Dear ImGui sources are included in this repository.

On Debian/Ubuntu:

```bash
sudo apt update
sudo apt install build-essential cmake git pkg-config libssl-dev libglfw3-dev libgl1-mesa-dev
```

On Arch Linux:

```bash
sudo pacman -S --needed base-devel cmake git pkgconf openssl glfw mesa
```

```bash
git clone https://github.com/vanPThomas/FreiaThiwiClient.git
cd FreiaThiwiClient
cmake -S . -B build
cmake --build build -j
cd bin
./freia-thiwi-client
```

Run from `bin/` so the relative `fonts/` path resolves. To connect, enter the server address and port, the server password, a username/account password, and a main-chat password. Use the **same room password** as other participants in that room. The corresponding [server repository](https://github.com/vanPThomas/FreiaThiwi) must be running.

## Security status

**v0.6.0 is an experimental/development release, not a security-audited product.**

- Transport and chat-body encryption currently use **AES-256-CBC**. CBC alone does **not** authenticate ciphertext or prevent active tampering; authenticated encryption (for example, AES-GCM) is an important future upgrade.
- Password-to-key derivation currently uses PBKDF2-HMAC-SHA256 with a fixed/empty salt for the general client/server keys. The account verifier in `accounts.db` is a reusable derived value, not a modern salted server-side password hash.
- Room verifiers are salted, but human-chosen room passwords can still be weak and subject to guessing attacks. Room metadata and the existence of messages are visible to the server.
- The server password is shared by all clients of a server. The present transport design is not a substitute for a fully authenticated end-to-end key exchange or a security review.
- **Ephemeral public/private-key-based secure password/invitation exchange is planned, not implemented.** Do not assume keys are exchanged securely merely because message bodies are encrypted once everyone knows the same password.

Avoid using Freia Thiwi to protect high-risk communications until these security limitations have been addressed and independently reviewed.

## Roadmap (not yet implemented)

- Authenticated encryption and protocol/security review.
- Authenticated secure exchange of room passwords/invitations using ephemeral key material (and a design for verifying recipient identities).
- Additional usability improvements, testing, and reliability hardening.
- Potential file sharing and further messaging features.

The goal is a **small, useful, maintainable messenger**, not an endlessly expanding list of prerequisites for 1.0.

## Project name and purpose

Freia Thiwi is Gothic: **𐍆𐍂𐌴𐌹𐌰** (*freia*, free) and **𐌸𐌹𐍅𐌹** (*thiwi*, servant/helper). *The servant of freedom.*

The project is intended to make independent communication more accessible, including on older and inexpensive computers. Contributions, testing, and constructive security feedback are welcome.

See [CHANGELOG.md](CHANGELOG.md) for the release history. Licensed under the terms in [LICENSE](LICENSE).
