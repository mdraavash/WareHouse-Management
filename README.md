Warehouse Management System
----------------------------
This is a simple warehouse management system made using C++. 
This utilizes the Bresenham's Line Generation algorithm to position the items along the diagonals of the warehouse.
It also utilizes the Manhattan distance algorithm to place item when the main diagonal is filled.

Operations in Programs
----------------------
**Initialization and Configuration:**
– The warehouse dimensions (width and height) are provided by the user.
– The user may change the dimensions after the initial input (which clears any stored items).
– The warehouse has “entry” and “exit” ports whose coordinates are set by default (entry at (0,0) and exit at (sizeX–1, sizeY–1)) but can be changed.

**Item Management:**
– Items (each with an ID, quantity, and priority) can be added. Their positions within the warehouse grid are automatically determined.
– Items can be deleted by ID.
– Items can be “reordered” based on priority (higher-priority items are given preferential placement).

**Persistence:**
– The warehouse (dimensions, port locations, and items) can be saved to and loaded from a file.

**Display:**
– The program can display a list of items and a grid-like layout of the warehouse

Running The Program
---------
To use this program, simply download a C++ compiler and compile and run the cpp file.
OR,
use the exe file present in the output.
(Be sure to have the warehouse_data file)
