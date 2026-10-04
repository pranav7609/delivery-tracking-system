# Code Walkthrough

## `main.cpp`

```cpp
int main() {
    try {
        DeliveryTracker tracker("packages.csv");   // (1)
        UI ui(tracker);                             // (2)
        ui.run();                                   // (3)
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
```

| Line | What happens |
|---|---|
| (1) | `DeliveryTracker` is constructed with the filename `"packages.csv"`. Its constructor immediately calls `loadFromFile()`, which populates the in-memory package map if the file exists. |
| (2) | `UI` is constructed. It stores a non-owning reference (`tracker_`) to the tracker. No I/O happens here. |
| (3) | `ui.run()` blocks for the entire session. It returns only when the user chooses option 0 (Exit). |

The `try/catch` wraps everything. Any `std::exception` thrown from any layer will be
caught here, printed to `stderr`, and the program exits with code 1.

---

## `Package.h` and `Package.cpp`

### `DeliveryStatus` enum

```cpp
enum class DeliveryStatus {
    PENDING, PICKED_UP, IN_TRANSIT, OUT_FOR_DELIVERY,
    DELIVERED, FAILED, RETURNED
};
```

A scoped enum with seven values. The integer mapping (0–6) is used in
`DeliveryTracker::printSummary` to index a fixed-size counts array.

### `TrackingEvent` struct

```cpp
struct TrackingEvent {
    std::string    timestamp;   // "YYYY-MM-DD HH:MM:SS"
    DeliveryStatus status;      // enum value at this event
    std::string    location;    // free-text location string
    std::string    note;        // free-text note (may equal statusToString)
};
```

Plain aggregate struct. No constructor. `DeliveryTracker` uses this type directly
when encoding/decoding history.

### `Package` class

#### Constructor

```cpp
Package(const std::string& id, sender, receiver, origin, destination, double weightKg)
```

- Initialises all private fields via member-initializer list.
- Sets `status_` to `PENDING`.
- Calls `currentTimestamp()` to set `createdAt_`.
- Pushes one `TrackingEvent` into `history_` with the origin as location and
  `"Package registered."` as the note. This is always `history_[0]`.

#### `updateStatus()`

```cpp
void Package::updateStatus(DeliveryStatus newStatus,
                           const std::string& location,
                           const std::string& note)
```

- Updates `status_` to `newStatus`.
- Creates a new `TrackingEvent` with `currentTimestamp()`, the new status, the
  provided location, and the note (defaults to `statusToString(newStatus)` if
  the note string is empty).
- Appends the event to `history_`.

This is the only path for live status changes during a session.

#### `restoreFromHistory()`

```cpp
void Package::restoreFromHistory(const std::vector<TrackingEvent>& history,
                                 const std::string& createdAt)
```

- Replaces `history_` wholesale with the provided vector.
- Sets `createdAt_` to the saved value from CSV (not to `currentTimestamp()`).
- Sets `status_` to `history.back().status` so the current status matches the
  last recorded event.

This method exists solely for `DeliveryTracker::loadFromFile()`. It bypasses the
timestamp-generating side-effects of `updateStatus()` so that loaded packages
preserve their original event timeline.

#### Static utilities

| Method | Purpose |
|---|---|
| `statusToString(DeliveryStatus)` | `switch` over all 7 enum values → human-readable string. Returns `"Unknown"` for any unhandled value. |
| `stringToStatus(const std::string&)` | Reverse map. Throws `std::invalid_argument` if the string does not match any known value. |
| `currentTimestamp()` | Calls `std::time(nullptr)` → `std::localtime` → `std::put_time` with format `"%Y-%m-%d %H:%M:%S"`. Returns a `std::string`. |

#### Private fields

| Field | Type | Description |
|---|---|---|
| `id_` | `string` | Tracking ID, e.g. `PKG-847321` |
| `sender_` | `string` | Sender name |
| `receiver_` | `string` | Receiver name |
| `origin_` | `string` | Origin city/location |
| `destination_` | `string` | Destination city/location |
| `weightKg_` | `double` | Weight in kilograms |
| `status_` | `DeliveryStatus` | Current status (mirrors last history entry) |
| `createdAt_` | `string` | Timestamp when the Package object was first constructed |
| `history_` | `vector<TrackingEvent>` | Ordered list of all status events, oldest first |

---

## `DeliveryTracker.h` and `DeliveryTracker.cpp`

### Private state

```cpp
std::string dataFile_;
std::unordered_map<std::string, Package> packages_;
```

`packages_` is the sole in-memory store. Keys are tracking ID strings. Values are
`Package` objects stored by value (not pointer). All public query methods return
`Package*` — raw pointers into this map. Those pointers are valid as long as the map
is not modified (no insertions or erasures after the pointer is taken).

### Constructor

```cpp
DeliveryTracker::DeliveryTracker(const std::string& dataFile)
    : dataFile_(dataFile)
{
    loadFromFile();
}
```

Stores the filename and immediately loads from it. If the file does not exist,
`loadFromFile()` returns `false` and the map stays empty — this is normal on first run.

### `addPackage(const Package& pkg)`

- Checks `packages_.count(pkg.getId())` — returns `false` without inserting if the
  ID is already present (duplicate guard).
- Inserts via `packages_.emplace(id, pkg)`.
- Calls `saveToFile()` immediately after insertion.
- Returns `true` on success.

### `updateStatus(id, newStatus, location, note)`

- Looks up the package by ID with `packages_.find`.
- Returns `false` if not found.
- Calls `pkg.updateStatus()` on the found package (mutates the in-memory object).
- Calls `saveToFile()` immediately.
- Returns `true` on success.

### `findPackage(const std::string& id)`

- Calls `packages_.find(id)`.
- Returns `nullptr` if not found, otherwise `&it->second`.

### `searchBySender` / `searchByReceiver`

Both follow the same pattern:
1. Iterate all entries in `packages_`.
2. Copy both the stored field and the query to local strings.
3. Convert both to lowercase with `std::transform(begin, end, begin, ::tolower)`.
4. Check `stored.find(query) != std::string::npos`.
5. Push matching `Package*` into the result vector.

This is a linear O(n) scan — no index exists.

### `filterByStatus(DeliveryStatus status)`

Linear O(n) scan. Compares `kv.second.getStatus() == status` for each entry.
Returns a `vector<Package*>` of matches.

### `getAllPackages()`

Iterates the entire map and pushes every `&kv.second` into a vector. Uses
`reserve(packages_.size())` to avoid reallocations.

### `printSummary()`

Single pass over `packages_`:
- Increments `counts[static_cast<int>(status)]` for each package.
- Accumulates `totalWeight`.

Then prints total count, total weight, and a breakdown of only the statuses with
a non-zero count.

### CSV helpers

#### `escapeCsv(const std::string& s)`

Wraps the string in double-quotes and doubles any internal double-quote characters.
Example: `He said "hello"` → `"He said ""hello"""`.

#### `unescapeCsv(const std::string& s)`

Strips the surrounding double-quotes and converts any `""` pair back to a single `"`.
Returns the string unchanged if it does not start and end with `"`.

#### `encodeHistory(const vector<TrackingEvent>& history)`

Serialises the history vector into a single string:
```
timestamp|status|location|note;timestamp|status|location|note;...
```
Fields within one event are separated by `|`. Events are separated by `;`.

#### `decodeHistory(const std::string& encoded)`

Reverses `encodeHistory`:
1. Splits on `;` using `std::getline` with delimiter.
2. For each chunk, splits on `|` to extract `timestamp`, `status`, `location`.
3. The remainder after the third `|` is treated as the note (allows `|` in notes).
4. Calls `Package::stringToStatus`; on exception defaults to `PENDING`.

#### `saveToFile()`

Opens `dataFile_` for writing (overwrites). Writes a fixed header line, then one CSV
line per package. The `history` column is the output of `encodeHistory` wrapped in
`escapeCsv` (because the encoded history contains commas and pipes).

#### `parseCsvLine(const std::string& line)` (file-local function)

A state-machine tokenizer. Tracks a boolean `inQuotes` flag:
- Inside quotes: `""` → single `"`, closing `"` exits quote mode.
- Outside quotes: `,` splits a field, `"` enters quote mode.

Returns a `vector<string>` of unquoted field values.

#### `loadFromFile()`

1. Opens `dataFile_` for reading. Returns `false` if it cannot be opened.
2. Skips the first line (header).
3. For each subsequent non-empty line:
   - Calls `parseCsvLine` to get a field vector.
   - Skips lines with fewer than 9 fields.
   - Constructs a `Package` from fields 0–5 (id, sender, receiver, origin,
     destination, `stod(f[5])` for weight).
   - Calls `decodeHistory(f[8])` to rebuild the event list.
   - If the history is non-empty, calls `pkg.restoreFromHistory(savedHistory, f[7])`
     where `f[7]` is the saved `createdAt` timestamp.
   - Inserts the package into `packages_` via `emplace`.
   - Any exception from a malformed line is silently caught and the line is skipped.

---

## `UI.h` and `UI.cpp`

### Private state

```cpp
DeliveryTracker& tracker_;
```

A reference to the tracker constructed in `main`. This is the only state `UI` holds.

### `run()`

On Windows, calls `SetConsoleOutputCP(CP_UTF8)` to configure the console code page.
Prints the application banner. Enters a `while(running)` loop that calls
`showMainMenu()`, reads an integer choice via `promptInt("Choice", 0, 7)`, and
dispatches to the appropriate handler. Sets `running = false` on choice `0`.

### `showMainMenu()`

Prints the menu options 0–7 between divider lines. No logic.

### `handleAddPackage()`

- Creates a `static std::mt19937 rng` seeded from `std::random_device{}()`. The
  `static` keyword means the RNG is seeded only once across calls.
- Generates a six-digit number with `std::uniform_int_distribution<int>(100000, 999999)`.
- Prefixes it with `"PKG-"`.
- Prompts for sender, receiver, origin, destination, weight.
- Constructs a `Package` on the stack.
- Calls `tracker_.addPackage(pkg)` and prints the result.

### `handleUpdateStatus()`

- Prints the section header.
- Reads a line for the admin key. If it does not equal `"12345"`, prints
  `[DENIED]` and returns.
- On success: prompts for tracking ID, looks it up, shows current status, calls
  `pickStatus`, prompts for location and note, calls `tracker_.updateStatus`.

### `handleTrackPackage()`

Prompts for a tracking ID, calls `tracker_.findPackage`, and either prints an error
or calls `printPackageDetail`.

### `handleSearchPackages()`

Prompts for search type (1=sender, 2=receiver), then a search term. Calls either
`tracker_.searchBySender` or `tracker_.searchByReceiver`. Prints the result count
followed by `printPackageBrief` for each match.

### `handleListByStatus()`

Calls `pickStatus` to get a `DeliveryStatus` enum value, then
`tracker_.filterByStatus`, and prints results with `printPackageBrief`.

### `handleListAll()`

Calls `tracker_.getAllPackages()`. If the vector is empty, prints a message.
Otherwise, prints a fixed-width column header and one row per package, truncating
sender and receiver names to 16 characters via `substr(0, 16)`.

### `handleSummary()`

Delegates entirely to `tracker_.printSummary()`.

### Display helpers

| Method | Behaviour |
|---|---|
| `printPackageBrief(pkg)` | One line: ID (14w), sender (12 chars max), receiver (12 chars max), status |
| `printPackageDetail(pkg)` | All fields plus full history. Notes equal to the status string are suppressed. |
| `printDivider(char, int)` | Prints `width` repetitions of `char`, default `'-'` × 60 |

### Input helpers

| Method | Behaviour |
|---|---|
| `prompt(label)` | Prints `"  label: "`, reads a full line with `getline`, returns it |
| `promptInt(label, min, max)` | Loops until a valid integer in `[min, max]` is entered via `getline` + `stoi` |
| `promptDouble(label)` | Loops until a positive `double` is entered via `getline` + `stod` |
| `pickStatus(label)` | Displays the 7 status options, calls `promptInt(1,7)`, maps to `DeliveryStatus` |
| `pauseScreen()` | Prints `"Press Enter to continue..."`, calls `getline` to consume the newline |
| `clearScreen()` | Calls `system("cls")` on Windows, `system("clear")` elsewhere. Not called anywhere in the current menu flow. |

---

## Data Flow Summary

```
User input
    │
    ▼
UI::promptInt / prompt / promptDouble
    │  (validated string → typed value)
    ▼
UI::handle*   (orchestrates the operation)
    │
    ├──► DeliveryTracker::addPackage / updateStatus / findPackage / search* / filter*
    │           │
    │           ├──► Package::updateStatus / restoreFromHistory
    │           │           (mutates Package in the map)
    │           │
    │           └──► DeliveryTracker::saveToFile
    │                       (serialises entire map to packages.csv)
    │
    └──► UI::printPackageDetail / printPackageBrief
                (read-only access via Package getters → stdout)
```

All mutation flows downward. Display flows upward through const getters.
No data ever moves from `Package` directly to `UI` — `UI` receives `Package*`
from `DeliveryTracker` and reads it through public const getters.
