# Changelog
All notable changes to **Freia Thiwi Client** will be documented here.

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
- Messages are currently **not encrypted**
- No user presence/identity validation
- No reconnection handling after server disconnect
- UI is functional but not yet theme-styled

