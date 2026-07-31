import Foundation

public enum OrganizerLimits {
    public static let notesMax = 6
    public static let todoMax = 4
    public static let titleMaxBytes = 48
}

private func organizerUTF8Prefix(_ value: String, maxBytes: Int) -> String {
    var result = ""
    for scalar in value.unicodeScalars {
        let part = String(scalar)
        if result.utf8.count + part.utf8.count > maxBytes { break }
        result.append(contentsOf: part)
    }
    return result
}

private func organizerTitles(_ values: [String], limit: Int) -> [String] {
    values.map { organizerUTF8Prefix($0, maxBytes: OrganizerLimits.titleMaxBytes) }
        .filter { !$0.isEmpty }
        .prefix(limit)
        .map { $0 }
}

public struct NotesState: Codable, Equatable {
    public var titles: [String]

    public init(titles: [String] = []) {
        self.titles = organizerTitles(titles, limit: OrganizerLimits.notesMax)
    }
}

public struct TodoState: Codable, Equatable {
    public var today: [String]
    public var overdue: [String]

    public init(today: [String] = [], overdue: [String] = []) {
        self.today = organizerTitles(today, limit: OrganizerLimits.todoMax)
        self.overdue = organizerTitles(overdue, limit: OrganizerLimits.todoMax)
    }
}

public struct NotesFrame: Codable {
    public var notes: NotesState
    public let v: Int

    public init(notes: NotesState) {
        self.notes = NotesState(titles: notes.titles)
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

public struct TodoFrame: Codable {
    public var todo: TodoState
    public let v: Int

    public init(todo: TodoState) {
        self.todo = TodoState(today: todo.today, overdue: todo.overdue)
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
