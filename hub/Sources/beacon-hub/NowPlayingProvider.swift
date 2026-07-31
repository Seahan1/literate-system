/* USER CODE BEGIN now_playing_provider */
import AppKit
import Foundation
import BeaconHubKit

final class NowPlayingProvider {
    private let queue = DispatchQueue(label: "beacon.now-playing", qos: .utility)
    private var timer: Timer?
    private var onUpdate: ((MediaState) -> Void)?
    private var lastState = MediaState.unavailable
    private var lastEmit = Date.distantPast

    func start(onUpdate: @escaping (MediaState) -> Void) {
        self.onUpdate = onUpdate
        poll()
        timer = Timer.scheduledTimer(withTimeInterval: 3, repeats: true) { [weak self] _ in
            self?.poll()
        }
        timer?.tolerance = 1
    }

    func stop() {
        timer?.invalidate()
        timer = nil
        onUpdate = nil
    }

    private func poll() {
        queue.async { [weak self] in
            let state = Self.readCurrent()
            let shouldEmit = state != self?.lastState || Date().timeIntervalSince(self?.lastEmit ?? .distantPast) >= 30
            self?.lastState = state
            if shouldEmit { self?.lastEmit = Date() }
            guard shouldEmit else { return }
            Task { @MainActor in
                self?.onUpdate?(state)
            }
        }
    }

    private static func readCurrent() -> MediaState {
        var paused: [MediaState] = []
        for app in ["com.apple.Music", "com.spotify.client"] {
            guard running(bundleID: app), let state = read(app) else { continue }
            if state.playing { return state }
            if !state.title.isEmpty { paused.append(state) }
        }
        return paused.first ?? .unavailable
    }

    private static func running(bundleID: String) -> Bool {
        NSWorkspace.shared.runningApplications.contains { $0.bundleIdentifier == bundleID }
    }

    private static func read(_ bundleID: String) -> MediaState? {
        let source: String
        if bundleID == "com.apple.Music" {
            source = """
            tell application "Music"
                set playback to (player state as text)
                if playback is "stopped" then return "none"
                set trackInfo to current track
                set sep to ASCII character 9
                try
                    set lyricText to lyrics of trackInfo
                on error
                    set lyricText to ""
                end try
                set positionText to (player position) as text
                set durationText to (duration of trackInfo) as text
                return playback & sep & (name of trackInfo) & sep & (artist of trackInfo) & sep & (album of trackInfo) & sep & positionText & sep & durationText & sep & lyricText
            end tell
            """
        } else {
            source = """
            tell application "Spotify"
                set playback to (player state as text)
                if playback is "stopped" then return "none"
                set trackInfo to current track
                set sep to ASCII character 9
                set positionText to (player position) as text
                set durationText to ((duration of trackInfo) / 1000) as text
                return playback & sep & (name of trackInfo) & sep & (artist of trackInfo) & sep & (album of trackInfo) & sep & positionText & sep & durationText & sep
            end tell
            """
        }

        guard let script = NSAppleScript(source: source) else { return nil }
        var error: NSDictionary?
        let result = script.executeAndReturnError(&error)
        guard let value = result.stringValue else { return nil }
        let fields = value.components(separatedBy: "\t")
        guard fields.count >= 7, !fields[1].isEmpty else { return .unavailable }
        return MediaState(title: fields[1], artist: fields[2], album: fields[3], playing: fields[0] == "playing",
                          positionSeconds: Int(Double(fields[4]) ?? 0),
                          durationSeconds: Int(Double(fields[5]) ?? 0), lyrics: fields[6])
    }
}
/* USER CODE END now_playing_provider */
