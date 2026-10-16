# Emergency Response System (DSA Mini Project)

A console-based C program that decides **who is treated first** and **where the ambulance goes**.

## Modules
| Module | Files | Data structures |
|---|---|---|
| 1. Emergency Triage Queue | triage.c, triage.h | Priority queue using a max-heap |
| 2. Ambulance Routing and Hospital Finder | citymap.c, citymap.h | Graph (adjacency list), BFS, DFS |

`main.c` is the menu that joins the two modules (Dispatch option).

## Team
- [Member 1 Riddhi]
- [Member 2 Fiona]

## How to run
    gcc -Wall -Wextra -o hospital main.c triage.c citymap.c
    ./hospital          (Windows: hospital.exe)

Type `11` to load demo data, then `2` to view the queue and `3` to dispatch an ambulance.

## Note
All data (areas, travel times, hospitals, beds) is illustrative.
