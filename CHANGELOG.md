# Changelog
All notable changes to **Freia Thiwi Client** will be documented here.

---

## [0.1.1] - 2025-11-16
### Changed
- Seperated the UI logic from the networking logic in preperation of seperate class creation.

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

