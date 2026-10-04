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
