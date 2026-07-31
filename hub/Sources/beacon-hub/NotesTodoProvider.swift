import AppKit
import Foundation
import BeaconHubKit

struct OrganizerSnapshot: Equatable {
    let notes: NotesState
    let todo: TodoState
}

final class NotesTodoProvider {
    private let queue = DispatchQueue(label: "beacon.notes-todo", qos: .utility)
    private var timer: Timer?
    private var onUpdate: ((OrganizerSnapshot) -> Void)?
    private var lastSnapshot: OrganizerSnapshot?
    private var lastEmit = Date.distantPast

    func start(onUpdate: @escaping (OrganizerSnapshot) -> Void) {
        self.onUpdate = onUpdate
        poll()
        timer = Timer.scheduledTimer(withTimeInterval: 30, repeats: true) { [weak self] _ in
            self?.poll()
        }
        timer?.tolerance = 5
    }

    func stop() {
        timer?.invalidate()
        timer = nil
        onUpdate = nil
    }

    private func poll() {
        queue.async { [weak self] in
            guard let self, let snapshot = Self.readCurrent() else { return }
            let shouldEmit = snapshot != self.lastSnapshot ||
                Date().timeIntervalSince(self.lastEmit) >= 60
            self.lastSnapshot = snapshot
            guard shouldEmit else { return }
            self.lastEmit = Date()
            Task { @MainActor in
                self.onUpdate?(snapshot)
            }
        }
    }

    private static func readCurrent() -> OrganizerSnapshot? {
        guard let noteTitles = readNotes(), let todo = readReminders() else { return nil }
        return OrganizerSnapshot(notes: NotesState(titles: noteTitles), todo: todo)
    }

    private static func readNotes() -> [String]? {
        let source = """
        tell application "Notes"
            set noteTitles to {}
            repeat with accountRef in every account
                repeat with folderRef in folders of accountRef
                    repeat with noteRef in notes of folderRef
                        set end of noteTitles to name of noteRef
                    end repeat
                end repeat
            end repeat
            set AppleScript's text item delimiters to linefeed
            return noteTitles as text
        end tell
        """
        guard let output = execute(source) else { return nil }
        return output.split(whereSeparator: \.isNewline).map(String.init)
    }

    private static func readReminders() -> TodoState? {
        let source = """
        tell application "Reminders"
            set dayStart to current date
            set time of dayStart to 0
            set nextDay to dayStart + 1 * days
            set rows to {}
            repeat with reminderRef in every reminder
                if completed of reminderRef is false then
                    try
                        set dueAt to due date of reminderRef
                        if dueAt is not missing value then
                            set bucket to ""
                            if dueAt < dayStart then
                                set bucket to "overdue"
                            else if dueAt < nextDay then
                                set bucket to "today"
                            end if
                            if bucket is not "" then
                                set end of rows to bucket & (ASCII character 9) & (name of reminderRef)
                            end if
                        end if
                    end try
                end if
            end repeat
            set AppleScript's text item delimiters to linefeed
            return rows as text
        end tell
        """
        guard let output = execute(source) else { return nil }
        var today: [String] = []
        var overdue: [String] = []
        for line in output.split(whereSeparator: \.isNewline) {
            let fields = line.split(separator: "\t", maxSplits: 1, omittingEmptySubsequences: false)
            guard fields.count == 2 else { continue }
            let title = String(fields[1])
            if fields[0] == "today" { today.append(title) }
            if fields[0] == "overdue" { overdue.append(title) }
        }
        return TodoState(today: today, overdue: overdue)
    }

    private static func execute(_ source: String) -> String? {
        guard let script = NSAppleScript(source: source) else { return nil }
        var error: NSDictionary?
        let result = script.executeAndReturnError(&error)
        guard error == nil else { return nil }
        return result.stringValue ?? ""
    }
}
