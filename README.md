# Delivery Tracking System

A structured, file-backed delivery management and tracking application built using modern C++17.

The system provides a complete workflow for registering packages, automatically generating tracking IDs, updating delivery statuses, searching and filtering shipments, maintaining chronological tracking history, and generating delivery statistics.

Designed with a modular object-oriented architecture, the application separates package data, tracking operations, persistence, and user interaction into dedicated components. Delivery records are persisted locally using CSV, allowing the application state to survive between executions.

---

## Overview

The Delivery Tracking System simulates the core operational workflow of a package delivery management platform.

Instead of treating a shipment as a simple record with a single status, the system maintains a complete tracking history for every package. Each status change records the timestamp, delivery status, current location, and an optional tracking note.

This allows a package to be followed from its initial registration through its complete delivery lifecycle.

The application also provides administrative protection for status updates, duplicate tracking-ID prevention, searchable shipment records, status-based filtering, package summaries, and persistent CSV storage.

---

## Core Capabilities

### Package Registration

Create a new shipment by providing:

- Sender name
- Receiver name
- Origin city
- Destination city
- Package weight

A unique tracking ID is automatically generated in the following format:

```text
PKG-XXXXXX

The system prevents duplicate tracking IDs from being inserted into the active package collection.

#Delivery Status Management
Every package can move through a defined delivery lifecycle:
Pending
   ↓
Picked Up
   ↓
In Transit
   ↓
Out for Delivery
   ↓
Delivered

The system also supports exceptional delivery outcomes:
Failed
Returned

Whenever a package status is updated, the system records a new tracking event rather than overwriting the previous state.
Package Tracking
A package can be retrieved using its tracking ID.
The detailed tracking view displays:
- Tracking ID
- Sender
- Receiver
- Origin
- Destination
- Package weight
- Creation timestamp
- Current delivery status
- Complete tracking history
Example:
[1] 2026-10-04 10:15:20  Pending             @ Bhubaneswar
[2] 2026-10-04 12:40:11  Picked Up           @ Bhubaneswar
[3] 2026-10-04 18:25:44  In Transit          @ Kolkata
[4] 2026-10-05 08:10:32  Out for Delivery    @ Kolkata
[5] 2026-10-05 11:42:19  Delivered           @ Kolkata

The actual history is generated dynamically by the application.
Search and Filtering
The system provides multiple ways to locate shipment records.
Search by Sender
Search shipment records using the sender's name.
The search is case-insensitive and supports partial matching.
Search by Receiver
Search shipment records using the receiver's name.
The search is case-insensitive and supports partial matching.
Filter by Delivery Status
Packages can be filtered according to their current delivery state.
Supported filters include:
- Pending
- Picked Up
- In Transit
- Out for Delivery
- Delivered
- Failed
- Returned
#Package Management
The main application menu provides the following operations:
1. Add new package
2. Update package status
3. Track a package
4. Search packages
5. List by status
6. List all packages
7. Summary / statistics
0. Exit

This provides a complete command-line workflow for managing the shipment lifecycle.
Administrative Access
Updating a package's delivery status is treated as an administrative operation.
Before changing a shipment's status, the application requests an administrator access key.
The current implementation uses:
12345

for local demonstration purposes.
Security note: This key is intentionally part of the current demonstration implementation and should be replaced with a secure authentication mechanism before production use.

#Statistics and Reporting
The system includes a summary module that calculates:
- Total number of packages
- Total package weight
- Number of packages in each delivery state
Example:
Total packages  : 25
Total weight    : 184.50 kg

Status breakdown:
  Pending             : 4
  Picked Up           : 3
  In Transit          : 7
  Out for Delivery    : 2
  Delivered           : 6
  Failed              : 1
  Returned            : 2

Only statuses with existing packages are displayed in the breakdown.
#Technical Architecture
The project follows a modular object-oriented architecture consisting of three primary layers:
                    +----------------------+
                    |       UI Layer       |
                    |        UI.cpp        |
                    +----------+-----------+
                               |
                               v
                    +----------------------+
                    |   Business Logic     |
                    |  DeliveryTracker.cpp |
                    +----------+-----------+
                               |
                               v
                    +----------------------+
                    |     Domain Model     |
                    |      Package.cpp     |
                    +----------+-----------+
                               |
                               v
                    +----------------------+
                    |  Persistence Layer   |
                    |    packages.csv      |
                    +----------------------+

This separation keeps user interaction, business operations, domain data, and persistence responsibilities independent.
#Project Structure
delivery-tracking-system/
│
├── include/
│   ├── DeliveryTracker.h
│   ├── Package.h
│   └── UI.h
│
├── src/
│   ├── main.cpp
│   ├── DeliveryTracker.cpp
│   ├── Package.cpp
│   └── UI.cpp
│
├── packages.csv
├── CMakeLists.txt
├── build.bat
├── .gitignore
└── README.md

#Component Responsibilities
Package
The Package class represents the core shipment entity.
It stores:
- Tracking ID
- Sender
- Receiver
- Origin
- Destination
- Weight
- Current status
- Creation timestamp
- Tracking history
It also provides operations for updating shipment status and restoring persisted tracking history.
DeliveryTracker
DeliveryTracker acts as the central business-logic and persistence manager.
Its responsibilities include:
- Adding packages
- Preventing duplicate IDs
- Finding packages
- Updating package status
- Searching by sender
- Searching by receiver
- Filtering by status
- Returning all packages
- Loading data from CSV
- Saving data to CSV
- Encoding tracking history
- Decoding tracking history
- Generating summary statistics
Packages are maintained internally using:
std::unordered_map<std::string, Package>

where the tracking ID acts as the lookup key.
This provides efficient direct access to shipments by tracking ID.
UI
The UI class is responsible for the command-line interface.
It handles:
- Main menu navigation
- User input
- Package creation
- Administrative status updates
- Package tracking
- Search operations
- Status filtering
- Package listing
- Summary generation
- Input validation
- Console formatting
The UI communicates with DeliveryTracker instead of directly manipulating package storage.
main.cpp
The application entry point initializes the core components and starts the user interface.
DeliveryTracker
       ↓
      UI
       ↓
   UI::run()

The tracker loads existing package information from packages.csv, after which the UI starts the main application loop.
Exceptions at the top level are caught and reported as fatal application errors.
Data Persistence
The application uses a CSV file named:
packages.csv

to persist shipment data.
Each stored package contains information such as:
id
sender
receiver
origin
destination
weight
status
createdAt
history

#Example structure:
id,sender,receiver,origin,destination,weight,status,createdAt,history
PKG-123456,John Doe,Jane Doe,Delhi,Mumbai,2.50,In Transit,2026-10-04 10:15:20,...

The application follows this persistence workflow:
Application Start
       ↓
Load packages.csv
       ↓
Reconstruct package objects
       ↓
Run application
       ↓
Modify package data
       ↓
Save packages.csv

This means shipment information remains available after the application is closed and started again.
Tracking History Design
One of the key design aspects of the system is that status changes are recorded as events rather than simply replacing the previous status.
Each tracking event contains:
timestamp
status
location
note

For example:
Pending
   |
   +-- Picked Up
          |
          +-- In Transit
                  |
                  +-- Out for Delivery
                          |
                          +-- Delivered

This event-oriented approach allows the application to preserve the complete history of a shipment.
When data is saved, the tracking history is encoded into the CSV representation. When the application starts again, that history is decoded and reconstructed with its timestamps.
Input Validation
The application includes validation for interactive numeric input.
For menu selections, values outside the allowed range are rejected.
For package weight, only positive numeric values are accepted.
Example:
Choice [0-7]: abc
Please enter a number between 0 and 7.

Choice [0-7]: 10
Please enter a number between 0 and 7.

This prevents invalid menu input from terminating the application unexpectedly.
#CSV Handling
The persistence layer includes CSV escaping and parsing logic.
The implementation handles:
- Quoted CSV fields
- Embedded quotation marks
- Comma-separated fields
- Tracking-history serialization
- Tracking-history reconstruction
- Malformed records
This allows names, locations, notes, and other text fields to be stored more reliably than with a simplistic delimiter-only approach.
Build System
The project uses CMake and requires:
CMake 3.16+
C++17 compatible compiler

The project explicitly requires the C++17 standard.
Compiler warnings are enabled for supported compilers:
MSVC:
    /W4

GCC / MinGW / Clang:
    -Wall
    -Wextra
    -Wpedantic

Building the Project
Option 1: Windows Build Script
The repository includes:
build.bat

Run:
build.bat

The script creates a build directory when necessary, configures the project using CMake, and builds the project in Release configuration.
After a successful build, the executable can be found inside the generated build directory.
Option 2: CMake
Create a build directory:
mkdir build
cd build

Configure the project:
cmake ..

Build:
cmake --build . --config Release

On Windows, the resulting executable is:
delivery_tracker.exe

Running the Application
After building, run the generated executable.
On Windows:
build\delivery_tracker.exe

The application starts with the Delivery Tracking System interface and presents the main menu.
Typical Workflow
A typical shipment lifecycle looks like this:
1. Register Package
        |
        v
2. Tracking ID Generated
        |
        v
3. Package Stored
        |
        v
4. Package Picked Up
        |
        v
5. Shipment Moves In Transit
        |
        v
6. Shipment Reaches Destination
        |
        v
7. Out for Delivery
        |
        v
8. Delivered

At every status transition, the system records a tracking event containing the time, location, status, and optional note.
Design Principles
The project was structured around several software engineering principles.
Encapsulation
Package information is maintained inside the Package class instead of being exposed as unrestricted global data.
Separation of Concerns
Responsibilities are divided between:
Package
DeliveryTracker
UI

#Each component has a clearly defined role.
Object-Oriented Design
The system uses classes, encapsulation, constructors, member functions, enumerations, and composition to model the delivery domain.
Persistent State
Shipment data is stored externally in CSV rather than existing only in memory.
Defensive Input Handling
Interactive input is validated before being used by the application.
Modular Build Configuration
CMake is used to define the project structure and build configuration independently of a specific IDE.
Technology Stack
Technology	Purpose
C++17	Core application development
CMake	Build configuration and project generation
CSV	Local data persistence
Standard Template Library	Containers, strings, algorithms and utilities
MinGW / GCC / MSVC	C++ compilation environments
Windows Batch	Simplified Windows build workflow
Git	Version control
GitHub	Source-code hosting


#Key C++ Concepts Demonstrated
This project demonstrates practical usage of:
- Object-oriented programming
- Classes and objects
- Encapsulation
- Constructors
- Enumerations
- Structures
- References
- Pointers
- std::vector
- std::unordered_map
- std::string
- STL algorithms
- File streams
- Exception handling
- CSV parsing
- Serialization and deserialization
- Timestamp generation
- Input validation
- CMake-based compilation
- Modular source/header organization
Error Handling
The application handles several common failure scenarios, including:
- Duplicate package IDs
- Unknown tracking IDs
- Invalid menu input
- Invalid numeric input
- Incorrect administrative access key
- Missing data files
- Malformed CSV records
- File write failures
- Unexpected top-level exceptions
Where appropriate, invalid records are skipped while allowing the application to continue operating.
Current Scope
The current implementation focuses on a local delivery-management workflow.
It provides:
Package Registration
        +
Status Management
        +
Shipment Tracking
        +
Search
        +
Filtering
        +
Tracking History
        +
Statistics
        +
CSV Persistence

The architecture provides a foundation that can later be extended into a larger delivery-management platform.
Future Enhancements
Potential future improvements include:
Authentication
Replace the demonstration administrator key with:
- User accounts
- Password hashing
- Role-based access control
- Session management
Database Integration
Replace CSV persistence with:
- MySQL
- PostgreSQL
- SQLite
This would provide stronger querying capabilities, transactional updates, and better scalability.
REST API
Expose delivery operations through a backend API:
Client
  |
  v
REST API
  |
  v
Delivery Management Service
  |
  v
Database

#Web Interface
A browser-based interface could be added for:
- Customer shipment tracking
- Administrator dashboards
- Delivery management
- Shipment analytics
- Search and filtering
#Notifications
Possible integrations include:
- Email notifications
- SMS notifications
- Delivery alerts
- Status-change notifications
#Advanced Analytics
Future versions could provide:
- Delivery-time analysis
- Failure-rate statistics
- Return-rate analysis
- Shipment-volume trends
- Location-based analytics
#Automated Testing
The project could be extended with:
- Unit tests
- Integration tests
- Persistence tests
- Input-validation tests
- Regression tests
#Security Considerations
The current application is intended as an educational and project implementation rather than a production logistics platform.
Before production deployment, the following areas should be strengthened:
- Replace hard-coded administrative credentials
- Hash and securely store credentials
- Implement role-based authorization
- Validate and sanitize external input
- Protect persisted data
- Add audit logging
- Use a transactional database
- Implement secure API authentication
- Add proper access controls
- Avoid storing sensitive information in plaintext
#Limitations
The current version has several intentional limitations:
- Local command-line interface
- CSV-based persistence
- Single local data source
- Demonstration administrator credential
- No network synchronization
- No external delivery-provider integration
- No customer authentication
- No live GPS tracking
- No notification service
These limitations define the current scope and also provide clear directions for future development.
#Project Architecture at a Glance
                         DELIVERY TRACKING SYSTEM
                                    |
                 +------------------+------------------+
                 |                  |                  |
                 v                  v                  v
             Package          DeliveryTracker         UI
                 |                  |                  |
                 |                  |                  |
                 v                  v                  v
          Shipment Model       Business Logic     User Interaction
                 |                  |                  |
                 +------------------+------------------+
                                    |
                                    v
                              packages.csv
                                    |
                                    v
                              Persistent Data

#Repository Structure
.
├── include/
│   ├── DeliveryTracker.h
│   ├── Package.h
│   └── UI.h
│
├── src/
│   ├── main.cpp
│   ├── DeliveryTracker.cpp
│   ├── Package.cpp
│   └── UI.cpp
│
├── packages.csv
├── CMakeLists.txt
├── build.bat
├── .gitignore
└── README.md

#Project Status
Component	Status
Architecture	Complete
Package Management	Implemented
Tracking	Implemented
Status Management	Implemented
Search	Implemented
Filtering	Implemented
Statistics	Implemented
CSV Persistence	Implemented
CMake Build	Configured
Windows Build Script	Available


Author
Pranav Kumar
B.Tech Computer Science Engineering
Institute of Technical Education and Research (ITER)
SOA Deemed to be University
GitHub:
https://github.com/pranav7609
