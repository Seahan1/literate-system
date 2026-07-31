/* USER CODE BEGIN media_tests */
import XCTest
@testable import BeaconHubKit

final class MediaFrameTests: XCTestCase {
    func testMediaFrameEncodesBoundedPlaybackState() throws {
        let frame = MediaFrame(media: MediaState(title: "Song", artist: "Artist", album: "Album", playing: true,
                                                 positionSeconds: 42, durationSeconds: 240, lyrics: "line one"))
        let data = try frame.encoded()
        let object = try XCTUnwrap(JSONSerialization.jsonObject(with: data) as? [String: Any])
        let media = try XCTUnwrap(object["media"] as? [String: Any])

        XCTAssertEqual(object["v"] as? Int, 1)
        XCTAssertEqual(media["title"] as? String, "Song")
        XCTAssertEqual(media["artist"] as? String, "Artist")
        XCTAssertEqual(media["album"] as? String, "Album")
        XCTAssertEqual(media["playing"] as? Bool, true)
        XCTAssertEqual(media["positionSeconds"] as? Int, 42)
        XCTAssertEqual(media["durationSeconds"] as? Int, 240)
        XCTAssertEqual(media["lyrics"] as? String, "line one")
        XCTAssertEqual(data.last, 0x0A)
    }

    func testMediaStateClampsFields() {
        let state = MediaState(title: String(repeating: "t", count: 100),
                               artist: String(repeating: "a", count: 70),
                               album: String(repeating: "b", count: 70), playing: false,
                               positionSeconds: -2, durationSeconds: -1,
                               lyrics: String(repeating: "l", count: 240))
        XCTAssertEqual(state.title.count, MediaLimits.titleMaxChars)
        XCTAssertEqual(state.artist.count, MediaLimits.artistMaxChars)
        XCTAssertEqual(state.album.count, MediaLimits.albumMaxChars)
        XCTAssertEqual(state.positionSeconds, 0)
        XCTAssertEqual(state.durationSeconds, 0)
        XCTAssertEqual(state.lyrics.count, MediaLimits.lyricsMaxChars)
    }
}
/* USER CODE END media_tests */
