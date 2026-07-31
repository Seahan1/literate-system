import XCTest
@testable import BeaconHubKit

final class NotesTodoFrameTests: XCTestCase {
    func testNotesFrameEncodesBoundedTitles() throws {
        let state = NotesState(titles: [String(repeating: "n", count: 80), "Second"])
        let frame = try NotesFrame(notes: state).encoded()
        let object = try XCTUnwrap(JSONSerialization.jsonObject(with: frame) as? [String: Any])
        let notes = try XCTUnwrap(object["notes"] as? [String: Any])
        let titles = try XCTUnwrap(notes["titles"] as? [String])

        XCTAssertEqual(object["v"] as? Int, 1)
        XCTAssertEqual(titles.count, 2)
        XCTAssertEqual(titles[0].count, OrganizerLimits.titleMaxBytes)
        XCTAssertEqual(frame.last, 0x0A)
    }

    func testTodoFrameSeparatesTodayAndOverdue() throws {
        let frame = try TodoFrame(todo: TodoState(today: ["Call"], overdue: ["Pay bill"])).encoded()
        let object = try XCTUnwrap(JSONSerialization.jsonObject(with: frame) as? [String: Any])
        let todo = try XCTUnwrap(object["todo"] as? [String: Any])

        XCTAssertEqual(todo["today"] as? [String], ["Call"])
        XCTAssertEqual(todo["overdue"] as? [String], ["Pay bill"])
    }
}
