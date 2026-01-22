# Changelog
All notable changes to **Freia Thiwi Client** will be documented here.

## [v0.4.0] - 2026-01-22

### Added
- Encrypted handshake (PROT2) sent immediately after TCP connect
  - Contains username, encrypted with serverSessionKey
  - Waits synchronously for server's encrypted "PROT2" reply
- Full server authentication verification
  - Connection only succeeds if server reply decrypts correctly and contains expected "OK"
  - Clear error messages on failure ("wrong server password?", incomplete reply, etc.)
- Handshake is now the first step — normal chat messages (PROT1) only sent after successful auth

### Changed
- `connectToServer()` now performs full handshake before starting receive thread
- Improved connection feedback in chat window during auth phase

### Security
- Username never appears in plaintext on the wire
- Immediate failure feedback if server password is incorrect

## [0.3.4] - 2026-01-18
### Changed
- FreiaUI: Proper text wrapping in the chat window

---

## [0.3.3] - 2026-01-15
### Changed
- FreiaUI: Cleaned up UI for better user experience

---

## [0.3.2] - 2026-01-14
### Changed
- ClientConnect: Refactored sendMessage function
- ClientConnect: dedicated PROT1 building function
- Validation: added username sanitization
- Validation: improved user name validation


---

## [0.3.1] - 2026-01-13
### Changed
- ClientConnect: better parameter handling via ConnectionParams struct
- ClientConnect: safer newline splitting implementation

---

## [0.3.0] - 2025-12-02
### Added
- Added Protocol and package framing
- Added Server password and server encryption
- Added Protocol one (PROT1)

---

## [0.2.3] - 2025-11-21
### Changed
- Fixed double shutdown bug

---

## [0.2.2] - 2025-11-21
### Added
- Added Error popup windows
- Added Auto scroll down in chat window

---

## [0.2.1] - 2025-11-19
### Changed
- Optimized encryption and decryption

---

## [0.2.0] - 2025-11-18
### Added
- End to End Encryption has been added
- All messages coming and going are now secure and even unreadable by the server.

---

## [0.1.4] - 2025-11-18
### added
- Added validation class
- Added UI and networking validation.
- If connection is not made in 3 seconds, there is a time out
- Added a disconnect button when connected
- Added menu bar with exit button

---

## [0.1.3] - 2025-11-17
### Added
- UI class
- Moved all ui logic to FreiaUI class
- Clean separation of the connection class and the UI class

---

## [0.1.2] - 2025-11-17
### Added
- Client class
- separated UI from connectivity logic.
- cleaned up connectivity code

---

## [0.1.1] - 2025-11-16
### Changed
- Separated the UI logic from the networking logic in preparation of separate class creation.

---

## [0.1.0] - 2025-11-16
### Added
- Initial project structure with CMake build system
- Basic ImGui-based UI: connection panel + chat window
- Socket connection to Freia Thiwi server
- Real-time message sending and receiving
- Threaded message listener
- Minimal mutex locking for thread-safe UI updates
- Fixed font loading path

### Removed
- All Windows-specific networking code (Linux-only for now)
- Legacy AES encryption logic (to be reintroduced and improved later)

### Known Limitations
- No user presence/identity validation
- No reconnection handling after server disconnect

