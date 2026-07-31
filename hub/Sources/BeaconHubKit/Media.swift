/* USER CODE BEGIN media_protocol */
import Foundation

public enum MediaLimits {
    public static let titleMaxChars = 64
    public static let artistMaxChars = 48
    public static let albumMaxChars = 48
    public static let lyricsMaxChars = 180
}

private func utf8Prefix(_ value: String, maxBytes: Int) -> String {
    var result = ""
    for scalar in value.unicodeScalars {
        let part = String(scalar)
        if result.utf8.count + part.utf8.count > maxBytes { break }
        result.append(contentsOf: part)
    }
    return result
}

public struct MediaState: Codable, Equatable {
    public var title: String
    public var artist: String
    public var album: String
    public var playing: Bool
    public var positionSeconds: Int
    public var durationSeconds: Int
    public var lyrics: String

    public init(title: String = "", artist: String = "", album: String = "", playing: Bool = false,
                positionSeconds: Int = 0, durationSeconds: Int = 0, lyrics: String = "") {
        self.title = utf8Prefix(String(title.prefix(MediaLimits.titleMaxChars)), maxBytes: 64)
        self.artist = utf8Prefix(String(artist.prefix(MediaLimits.artistMaxChars)), maxBytes: 48)
        self.album = utf8Prefix(String(album.prefix(MediaLimits.albumMaxChars)), maxBytes: 48)
        self.playing = playing
        self.positionSeconds = max(0, positionSeconds)
        self.durationSeconds = max(0, durationSeconds)
        self.lyrics = utf8Prefix(String(lyrics.prefix(MediaLimits.lyricsMaxChars)), maxBytes: 512)
    }

    public static let unavailable = MediaState()
}

public struct MediaFrame: Codable {
    public var media: MediaState
    public let v: Int

    public init(media: MediaState) {
        self.media = MediaState(title: media.title, artist: media.artist,
                                album: media.album, playing: media.playing,
                                positionSeconds: media.positionSeconds,
                                durationSeconds: media.durationSeconds,
                                lyrics: media.lyrics)
        self.v = 1
    }

    public func encoded() throws -> Data {
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.sortedKeys]
        var data = try encoder.encode(self)
        data.append(0x0A)
        return data
    }
}
/* USER CODE END media_protocol */
