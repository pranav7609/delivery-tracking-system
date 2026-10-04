# System Workflow

## Application Lifecycle

```mermaid
flowchart TD
    A([Program Start]) --> B[main: construct DeliveryTracker\nwith 'packages.csv']
    B --> C[DeliveryTracker constructor\ncalls loadFromFile]
    C --> D{packages.csv\nexists?}
    D -- No --> E[Empty package map\nstart fresh]
    D -- Yes --> F[Parse CSV, rebuild\nPackage objects in memory]
    F --> E
    E --> G[main: construct UI\nwith tracker reference]
    G --> H[UI::run — print banner\nenter main loop]
    H --> I[showMainMenu]
    I --> J[promptInt: choice 0–7]
    J --> K{choice}
    K -- 1 --> L[handleAddPackage]
    K -- 2 --> M[handleUpdateStatus]
    K -- 3 --> N[handleTrackPackage]
    K -- 4 --> O[handleSearchPackages]
    K -- 5 --> P[handleListByStatus]
    K -- 6 --> Q[handleListAll]
    K -- 7 --> R[handleSummary]
    K -- 0 --> S([Print Goodbye\nloop exits\nprogram returns 0])
    L & M & N & O & P & Q & R --> I
```

---

## Package Creation (Menu Option 1)

```mermaid
flowchart TD
    A[handleAddPackage] --> B[Generate tracking ID\nmt19937 RNG → PKG-XXXXXX\nwhere XXXXXX is 100000–999999]
    B --> C[Prompt: sender, receiver,\norigin, destination, weight]
    C --> D[Construct Package object\nstatus = PENDING\ncreatedAt = currentTimestamp\nfirst TrackingEvent pushed to history_]
    D --> E[DeliveryTracker::addPackage]
    E --> F{ID already\nin map?}
    F -- Yes --> G[return false\nprint error]
    F -- No --> H[packages_.emplace\nID → Package]
    H --> I[saveToFile — rewrite\nentire packages.csv]
    I --> J[return true\nprint OK + tracking ID]
```

The tracking ID is generated inside `handleAddPackage` using a `static std::mt19937`
seeded from `std::random_device`. The format is always `PKG-` followed by a six-digit
integer in the range 100000–999999.

---

## Status Update (Menu Option 2)

```mermaid
flowchart TD
    A[handleUpdateStatus] --> B[Print header]
    B --> C[Prompt: admin access key]
    C --> D{key == '12345'?}
    D -- No --> E[Print DENIED\nreturn to menu]
    D -- Yes --> F[Print Access granted]
    F --> G[Prompt: Tracking ID]
    G --> H[DeliveryTracker::findPackage]
    H --> I{Found?}
    I -- No --> J[Print ERROR\nreturn to menu]
    I -- Yes --> K[Display current status]
    K --> L[pickStatus: user selects\nnew DeliveryStatus 1–7]
    L --> M[Prompt: location, note]
    M --> N[DeliveryTracker::updateStatus]
    N --> O[Package::updateStatus\nnew TrackingEvent appended\nstatus_ updated]
    O --> P[saveToFile]
    P --> Q[Print OK]
```

The admin key `"12345"` is checked with a plain string comparison in `handleUpdateStatus`.
If it does not match, the function returns immediately without touching the tracker.

---

## Package Tracking (Menu Option 3)

```mermaid
flowchart TD
    A[handleTrackPackage] --> B[Prompt: Tracking ID]
    B --> C[DeliveryTracker::findPackage\nlookup in unordered_map]
    C --> D{Found?}
    D -- No --> E[Print ERROR]
    D -- Yes --> F[printPackageDetail\nprint all fields +\ncomplete history vector]
    F --> G[For each TrackingEvent\nprint index, timestamp,\nstatus, location, note]
```

`printPackageDetail` iterates the `history_` vector in order (index 0 is always the
registration event). Notes that equal the status string are suppressed to avoid
redundancy.

---

## Package Search (Menu Option 4)

```mermaid
flowchart TD
    A[handleSearchPackages] --> B{Search by\n1=sender\n2=receiver}
    B -- 1 --> C[Prompt search term\nDeliveryTracker::searchBySender]
    B -- 2 --> D[Prompt search term\nDeliveryTracker::searchByReceiver]
    C & D --> E[Iterate all packages_\nconvert both strings to lowercase\ncheck s.find query != npos]
    E --> F[Return vector of Package*]
    F --> G[Print count\nFor each: printPackageBrief]
```

Search is case-insensitive and substring-based. Both the stored field and the query
are converted to lowercase with `std::transform` before comparison.

---

## Filter by Status (Menu Option 5)

```mermaid
flowchart TD
    A[handleListByStatus] --> B[pickStatus\nuser selects enum value 1–7]
    B --> C[DeliveryTracker::filterByStatus]
    C --> D[Iterate packages_\ncollect Package* where\ngetStatus == requested status]
    D --> E[Print count + matching packages\nvia printPackageBrief]
```

---

## List All Packages (Menu Option 6)

```mermaid
flowchart TD
    A[handleListAll] --> B[DeliveryTracker::getAllPackages\ncollect all Package* from map]
    B --> C{Empty?}
    C -- Yes --> D[Print no packages message]
    C -- No --> E[Print column header row\nID / Sender / Receiver / Status / Weight]
    E --> F[For each Package*\nprint one formatted row\nSender and Receiver\ntruncated to 16 chars]
```

Iteration order over the `unordered_map` is implementation-defined, so the list order
is not guaranteed to be insertion order or alphabetical.

---

## Summary / Statistics (Menu Option 7)

```mermaid
flowchart TD
    A[handleSummary] --> B[DeliveryTracker::printSummary]
    B --> C[Iterate packages_\naccumulate counts per status\naccumulate total weight]
    C --> D[Print total package count]
    D --> E[Print total weight in kg]
    E --> F[For each of 7 statuses\nif count > 0 print label + count]
```

---

## CSV Load (Startup)

```mermaid
flowchart TD
    A[loadFromFile] --> B{File\nexists?}
    B -- No --> C[Return false\nstart with empty map]
    B -- Yes --> D[Open ifstream\nskip header line]
    D --> E[Read each line]
    E --> F[parseCsvLine\ntokenize respecting quoted fields]
    F --> G{At least\n9 fields?}
    G -- No --> H[Skip malformed line]
    G -- Yes --> I[Construct Package\nf0..f5: id sender receiver\norigin destination weight]
    I --> J[decodeHistory f8\nsplit on semicolons\nparse each event on pipes]
    J --> K[restoreFromHistory\nreplace history_ and createdAt_\nset status_ from last event]
    K --> L[packages_.emplace]
    L --> E
```

---

## CSV Save (After Every Write Operation)

```mermaid
flowchart TD
    A[saveToFile] --> B[Open ofstream\noverwrite file]
    B --> C[Write header row]
    C --> D[For each Package in packages_]
    D --> E[escapeCsv each string field\nwrite weight as plain double\nencodeHistory → pipe-separated events\nwrapped in escapeCsv]
    E --> F[Write one CSV line per package]
    F --> D
```

`saveToFile` always rewrites the entire file from the in-memory map. There is no
append-only mode.
