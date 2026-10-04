# Testing Guide

There is no automated test framework in this project. This guide provides a complete
manual test plan covering every menu operation, valid and invalid inputs, edge cases,
and persistence verification.

Run `delivery_tracker.exe` from the project root for all tests.

---

## Test Environment Setup

Before starting, ensure a clean state:

```cmd
cd "d:\C++FILES\Delivery Tracking System"
del packages.csv
delivery_tracker.exe
```

Deleting `packages.csv` gives you a known-empty starting point.

---

## TC-01 — Startup with No Data File

**Steps:** Delete `packages.csv` (if it exists), then launch the application.

**Expected:**
- Application starts and shows the main menu without any error.
- No crash, no "file not found" error printed.
- Menu options function normally.

---

## TC-02 — Add a Package (Valid Input)

**Steps:** Choose option `1`.

```
Sender name:      Alice
Receiver name:    Bob
Origin city:      Dallas
Destination city: Chicago
Weight (kg):      3.5
```

**Expected:**
- A tracking ID is displayed in format `PKG-XXXXXX` (6 digits).
- `[OK] Package added successfully.` is printed.
- `packages.csv` is created (or updated) in the project directory.
- The CSV file contains one data row matching the entered values.
- Status in CSV is `"Pending"`.
- History column contains one entry ending with `|Package registered.`

---

## TC-03 — Add a Package (Invalid Weight)

**Steps:** Choose option `1`. Fill in valid name/city fields. For weight, enter:
- First attempt: `abc`
- Second attempt: `-5`
- Third attempt: `0`
- Fourth attempt: `2.0`

**Expected:**
- For `abc`, `-5`, `0`: prints `"Please enter a positive number."` and re-prompts.
- For `2.0`: accepts and proceeds.

---

## TC-04 — Track a Package (Valid ID)

**Prerequisite:** TC-02 completed. Note the generated ID.

**Steps:** Choose option `3`. Enter the tracking ID from TC-02.

**Expected:**
- All fields displayed: Tracking ID, Sender (Alice), Receiver (Bob), Origin (Dallas),
  Destination (Chicago), Weight (3.50 kg), Created At, Status (Pending).
- Tracking History shows exactly one entry: `[1]` with timestamp, `Pending`, `Dallas`,
  note `(Package registered.)`.

---

## TC-05 — Track a Package (ID Not Found)

**Steps:** Choose option `3`. Enter `PKG-000000`.

**Expected:**
- `[ERROR] Package not found.` printed.
- Returns to main menu without crash.

---

## TC-06 — Update Status (Wrong Admin Key)

**Steps:** Choose option `2`. When prompted for admin access key, enter `99999`.

**Expected:**
- `[DENIED] Incorrect admin key.` is printed.
- Returns to main menu immediately.
- No package data is modified.

---

## TC-07 — Update Status (Correct Admin Key, Valid Package)

**Prerequisite:** TC-02 completed. Note the tracking ID.

**Steps:** Choose option `2`.

```
Admin access key:  12345
Tracking ID:       <ID from TC-02>
New status:        3  (In Transit)
Current location:  Memphis
Note:              On the way
```

**Expected:**
- `[OK] Access granted.` after key entry.
- Current status displayed as `Pending` before the update.
- `[OK] Status updated.` printed.
- `packages.csv` updated: status column is `"In Transit"`.
- History column contains two entries — the original Pending and the new In Transit.

---

## TC-08 — Update Status (Package ID Not Found)

**Steps:** Choose option `2`.

```
Admin access key:  12345
Tracking ID:       PKG-000000
```

**Expected:**
- `[ERROR] Package not found.` printed.
- Returns to main menu.

---

## TC-09 — Update Status (Empty Note)

**Steps:** Choose option `2`. Enter correct key and a valid ID. When prompted for note,
press Enter without typing anything.

**Expected:**
- Status updates successfully.
- In the tracking history, the note field defaults to the status string
  (e.g. `"Delivered"` for status Delivered).
- When viewing the package detail, no note is printed in parentheses for that event
  (because the note equals the status string and is suppressed in display).

---

## TC-10 — Track Package After Status Update

**Prerequisite:** TC-07 completed.

**Steps:** Choose option `3`. Enter the same tracking ID.

**Expected:**
- Status field shows `In Transit`.
- Tracking History shows two entries:
  - `[1]` Pending @ Dallas (Package registered.)
  - `[2]` In Transit @ Memphis (On the way)
- Timestamps on the two entries are different.

---

## TC-11 — Search by Sender (Match Found)

**Prerequisite:** TC-02 completed (sender: Alice).

**Steps:** Choose option `4` → `1`. Enter search term: `ali`

**Expected:**
- `Found 1 result(s):` printed.
- The package from TC-02 appears in the result.
- Search is case-insensitive: `ALI`, `ali`, `Ali` all match.

---

## TC-12 — Search by Sender (No Match)

**Steps:** Choose option `4` → `1`. Enter: `zzz`

**Expected:**
- `Found 0 result(s):` printed.
- No packages listed.

---

## TC-13 — Search by Receiver

**Prerequisite:** TC-02 completed (receiver: Bob).

**Steps:** Choose option `4` → `2`. Enter: `bob`

**Expected:**
- `Found 1 result(s):` printed.
- The package from TC-02 appears.

---

## TC-14 — Filter by Status (Match)

**Prerequisite:** TC-02 completed (status: Pending at this point, or In Transit if TC-07 was run).

**Steps:** Choose option `5`. Select the current known status of the test package.

**Expected:**
- The package appears in the filtered list.
- Count is at least 1.

---

## TC-15 — Filter by Status (No Match)

**Steps:** Choose option `5`. Select `Returned` (status 7) — assuming no package has been returned.

**Expected:**
- `Packages with status [Returned]: 0` printed.
- No packages listed.

---

## TC-16 — List All Packages

**Prerequisite:** At least one package exists.

**Steps:** Choose option `6`.

**Expected:**
- Column headers printed: ID, Sender, Receiver, Status, Weight(kg).
- One row per package.
- Sender and Receiver columns truncated to 16 characters if names are longer.
- Weight shown with 2 decimal places.

---

## TC-17 — List All Packages (Empty)

**Steps:** Delete `packages.csv`, restart, choose option `6`.

**Expected:**
- `No packages on record.` printed.

---

## TC-18 — Summary / Statistics

**Prerequisite:** At least one package with known status.

**Steps:** Choose option `7`.

**Expected:**
- `Total packages  : N` where N is the number of packages.
- `Total weight    : X.XX kg` where X.XX is the sum of all package weights.
- Status breakdown lists only statuses with at least one package.

---

## TC-19 — Summary (Empty)

**Steps:** Delete `packages.csv`, restart, choose option `7`.

**Expected:**
- `Total packages  : 0`
- `Total weight    : 0.00 kg`
- No status breakdown lines printed.

---

## TC-20 — Persistence: Data Survives Restart

**Prerequisite:** TC-07 completed (package in In Transit state with 2 history events).

**Steps:**
1. Exit the application (option `0`).
2. Relaunch `delivery_tracker.exe`.
3. Choose option `3`. Enter the same tracking ID.

**Expected:**
- Package is found.
- All fields are identical to what was entered.
- History shows both events with their original timestamps (not new timestamps from
  load time).
- Status is `In Transit`.

---

## TC-21 — Persistence: Multiple Packages Survive Restart

**Steps:**
1. Add three packages with different senders and weights.
2. Exit.
3. Relaunch.
4. Choose option `6`.

**Expected:**
- All three packages appear in the list.
- Weights and statuses are correct.

---

## TC-22 — Invalid Menu Choice

**Steps:** At the main menu prompt `Choice [0-7]:`, enter `8`, then `abc`, then `0`.

**Expected:**
- For `8`: `Please enter a number between 0 and 7.` re-prompts.
- For `abc`: same re-prompt.
- For `0`: exits cleanly.

---

## TC-23 — Multiple Status Updates on One Package

**Steps:**
1. Add a package.
2. Update status to `Picked Up` (location: Warehouse A, note: Collected).
3. Update status to `In Transit` (location: Highway 40, note: En route).
4. Update status to `Out for Delivery` (location: Local Hub, note: On vehicle).
5. Track the package.

**Expected:**
- History shows 4 entries (1 creation + 3 updates).
- Each entry has a different timestamp.
- Current status shown as `Out for Delivery`.
- All locations and notes are correctly preserved.

---

## TC-24 — CSV Manual Inspection

After TC-23, open `packages.csv` in a text editor.

**Expected:**
- One data row for the package.
- History column contains 4 semicolon-separated events.
- Each event has 4 pipe-separated fields: timestamp, status string, location, note.
- All string fields are wrapped in double quotes.
- Weight column is a plain number (no quotes).
