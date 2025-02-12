#include <iostream>
#include <list>
#include <queue>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <stdexcept>

using namespace std;

// Class Item: Represents an item stored in the warehouse.
class Item {
    string id;
    int quantity;
    int priority;
    int x, y;
public:
    Item() : id(""), quantity(0), priority(0), x(0), y(0) {}
    Item(const string &id, int quantity, int priority, int x, int y)
        : id(id), quantity(quantity), priority(priority), x(x), y(y) {}

    string getId() const { return id; }
    int getQuantity() const { return quantity; }
    int getPriority() const { return priority; }
    int getPositionX() const { return x; }
    int getPositionY() const { return y; }
    void getPosition(int &out_x, int &out_y) const { out_x = x; out_y = y; }
    void setPosition(int new_x, int new_y) { x = new_x; y = new_y; }

    void display() const {
        cout << "ID: " << id << ", Quantity: " << quantity
             << ", Priority: " << priority << ", Position: (" << x << ", " << y << ")\n";
    }

    // Items with higher priority should come first.
    bool operator<(const Item &other) const {
        return priority < other.priority;
    }
};

// Merge sort functions to sort items in descending order of priority.
void merge(vector<Item>& items, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    vector<Item> leftItems(items.begin() + left, items.begin() + mid + 1);
    vector<Item> rightItems(items.begin() + mid + 1, items.begin() + right + 1);
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (leftItems[i].getPriority() > rightItems[j].getPriority())
            items[k++] = leftItems[i++];
        else
            items[k++] = rightItems[j++];
    }
    while (i < n1)
        items[k++] = leftItems[i++];
    while (j < n2)
        items[k++] = rightItems[j++];
}

void mergeSort(vector<Item>& items, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(items, left, mid);
        mergeSort(items, mid + 1, right);
        merge(items, left, mid, right);
    }
}

// Class Warehouse: Manages items, placement, dimensions, and file I/O.
class Warehouse {
    list<Item> items;
    int sizeX, sizeY;
    pair<int, int> entry, exitPoint; // renamed exit to exitPoint for clarity.
    vector<vector<bool>> occupied;

    // Bresenham's algorithm: generates points along the line from (x0, y0) to (x1, y1)
    vector<pair<int, int>> generateLinePoints(int x0, int y0, int x1, int y1) {
        vector<pair<int, int>> points;
        int dx = abs(x1 - x0);
        int dy = abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;
        while (true) {
            points.emplace_back(x0, y0);
            if (x0 == x1 && y0 == y1)
                break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx) { err += dx; y0 += sy; }
        }
        return points;
    }

    // Reinitializes the occupied grid.
    void resetOccupied() {
        occupied.assign(sizeX, vector<bool>(sizeY, false));
    }

public:
    // Constructor asks the user for initial warehouse dimensions.
    Warehouse() {
        cout << "Enter warehouse dimensions (sizeX sizeY): ";
        cin >> sizeX >> sizeY;
        if (sizeX <= 0 || sizeY <= 0)
            throw invalid_argument("Warehouse dimensions must be positive.");
        occupied.assign(sizeX, vector<bool>(sizeY, false));
        entry = {0, 0};
        exitPoint = {sizeX - 1, sizeY - 1}; // Default exit at the opposite corner.
    }

    // Returns the current warehouse dimensions.
    pair<int, int> getDimensions() const { return {sizeX, sizeY}; }

    // Allows the user to change warehouse dimensions.
    // NOTE: Changing dimensions clears any stored items.
    void changeDimensions() {
        int newX, newY;
        cout << "Current warehouse dimensions are " << sizeX << " x " << sizeY << ".\n";
        cout << "Enter new warehouse dimensions (sizeX sizeY): ";
        cin >> newX >> newY;
        if (newX <= 0 || newY <= 0) {
            cout << "Invalid dimensions entered. Keeping old dimensions.\n";
            return;
        }
        sizeX = newX;
        sizeY = newY;
        resetOccupied();
        items.clear();  // Clear items since positions may no longer be valid.
        // Reset default ports.
        entry = {0, 0};
        exitPoint = {sizeX - 1, sizeY - 1};
        cout << "Warehouse dimensions updated to " << sizeX << " x " << sizeY << ".\n";
    }

    // Sets new entry and exit ports and then repositions the items.
    void setEntryExitPort() {
        cout << "Enter new entry (x y) and exit (x y): ";
        cin >> entry.first >> entry.second >> exitPoint.first >> exitPoint.second;
        if (entry.first < 0 || entry.first >= sizeX || entry.second < 0 || entry.second >= sizeY)
            throw out_of_range("Entry point out of bounds.");
        if (exitPoint.first < 0 || exitPoint.first >= sizeX || exitPoint.second < 0 || exitPoint.second >= sizeY)
            throw out_of_range("Exit point out of bounds.");
        cout << "Entry and exit ports updated.\n";
        repositionItemsRelativeToEntry();
    }

    // Repositions items based on sorted priority and available positions.
    void repositionItemsRelativeToEntry() {
        vector<Item> sortedItems(items.begin(), items.end());
        if (!sortedItems.empty())
            mergeSort(sortedItems, 0, sortedItems.size() - 1);

        // Create a list of all coordinates.
        vector<pair<int, int>> coords;
        for (int i = 0; i < sizeX; ++i)
            for (int j = 0; j < sizeY; ++j)
                coords.push_back({i, j});

        // Determine available coordinates.
        vector<pair<int, int>> availableCoords;
        for (const auto &coord : coords) {
            if (!isOccupied(coord.first, coord.second))
                availableCoords.push_back(coord);
        }

        // Sort available coordinates relative to the line from entry to exit.
        auto entryPos = entry;
        auto exitPos = exitPoint;
        int dx = exitPos.first - entryPos.first;
        int dy = exitPos.second - entryPos.second;
        sort(availableCoords.begin(), availableCoords.end(), [dx, dy, entryPos](const pair<int, int>& a, const pair<int, int>& b) {
            int lineDistA = abs(dy * (a.first - entryPos.first) - dx * (a.second - entryPos.second));
            int lineDistB = abs(dy * (b.first - entryPos.first) - dx * (b.second - entryPos.second));
            if (lineDistA == lineDistB) {
                int manhattanA = abs(a.first - entryPos.first) + abs(a.second - entryPos.second);
                int manhattanB = abs(b.first - entryPos.first) + abs(b.second - entryPos.second);
                return manhattanA < manhattanB;
            }
            return lineDistA < lineDistB;
        });

        // Clear and reset the warehouse.
        resetOccupied();
        items.clear();

        size_t n = sortedItems.size();
        if (n > availableCoords.size())
            n = availableCoords.size();

        for (size_t i = 0; i < n; i++) {
            sortedItems[i].setPosition(availableCoords[i].first, availableCoords[i].second);
            markOccupied(availableCoords[i].first, availableCoords[i].second);
            items.push_back(sortedItems[i]);
        }

        cout << "Items repositioned relative to the new entry and exit ports.\n";
    }

    bool isOccupied(int x, int y) {
        if (x < 0 || x >= sizeX || y < 0 || y >= sizeY)
            throw out_of_range("Invalid position: out of bounds");
        return occupied[x][y];
    }

    void markOccupied(int x, int y) {
        if (x < 0 || x >= sizeX || y < 0 || y >= sizeY)
            throw out_of_range("Invalid position: out of bounds");
        occupied[x][y] = true;
    }

    void markAvailable(int x, int y) {
        if (x < 0 || x >= sizeX || y < 0 || y >= sizeY)
            throw out_of_range("Invalid position: out of bounds");
        occupied[x][y] = false;
    }

    // Returns the first available spot in the warehouse.
    pair<int, int> findNearestAvailableSpot() {
        for (int i = 0; i < sizeX; ++i)
            for (int j = 0; j < sizeY; ++j)
                if (!isOccupied(i, j))
                    return {i, j};
        return {-1, -1}; // No spot available.
    }

    // Adds a new item, trying first to place it along the line from entry to exit.
    void addItem() {
        string id;
        int quantity, priority;
        cout << "Enter Item ID, Quantity, Priority: ";
        cin >> id >> quantity >> priority;

        int x = -1, y = -1;
        vector<pair<int, int>> linePoints = generateLinePoints(entry.first, entry.second, exitPoint.first, exitPoint.second);
        for (auto &p : linePoints) {
            try {
                if (!isOccupied(p.first, p.second)) {
                    x = p.first;
                    y = p.second;
                    break;
                }
            } catch (const out_of_range &) {
                continue;
            }
        }

        // If no point along the line is available, try a diagonal placement.
        if (x == -1) {
            int limit = min(sizeX, sizeY);
            for (int d = 0; d < limit; ++d) {
                try {
                    if (!isOccupied(d, d)) {
                        x = d;
                        y = d;
                        break;
                    }
                } catch (const out_of_range &) {
                    continue;
                }
            }
        }

        // If still not found, search the entire warehouse.
        if (x == -1)
            tie(x, y) = findNearestAvailableSpot();

        if (x != -1 && y != -1) {
            items.emplace_back(id, quantity, priority, x, y);
            markOccupied(x, y);
            cout << "Item placed at: (" << x << ", " << y << ")\n";
        } else {
            cout << "Warehouse is full!\n";
        }
    }

    // Deletes an item by its ID.
    void deleteItem(const string &id) {
        for (auto it = items.begin(); it != items.end(); ++it) {
            if (it->getId() == id) {
                int x, y;
                it->getPosition(x, y);
                markAvailable(x, y);
                items.erase(it);
                cout << "Item with ID " << id << " deleted.\n";
                return;
            }
        }
        cout << "Item not found.\n";
    }

    // Reorders items based on priority using a priority queue.
    void reorderItems() {
        priority_queue<Item> pq;
        for (auto &itm : items) {
            pq.push(itm);
            int x, y;
            itm.getPosition(x, y);
            markAvailable(x, y);
        }
    
        items.clear();
        resetOccupied();
    
        while (!pq.empty()) {
            Item itm = pq.top();
            pq.pop();
    
            int x = -1, y = -1;
            int limit = min(sizeX, sizeY);
            for (int d = 0; d < limit; ++d) {
                if (!isOccupied(d, d)) {
                    x = d;
                    y = d;
                    break;
                }
            }
            if (x == -1)
                tie(x, y) = findNearestAvailableSpot();
    
            if (x != -1 && y != -1) {
                itm.setPosition(x, y);
                markOccupied(x, y);
                items.push_back(itm);
            }
        }
        cout << "Items reordered based on priority (using default diagonal placement).\n";
    }

    // Displays all items.
    void viewItems() const {
        for (const auto &itm : items)
            itm.display();
    }

    // Displays the warehouse layout.
    void displayWarehouse() {
        cout << "\nWarehouse Layout:\n";
        for (int i = 0; i < sizeX; ++i) {
            for (int j = 0; j < sizeY; ++j) {
                bool found = false;
                for (const auto &itm : items) {
                    if (itm.getPositionX() == i && itm.getPositionY() == j) {
                        cout << "  " <<itm.getId() << "(" << itm.getPriority() << ") "<<"  ";
                        found = true;
                        break;
                    }
                }
                if (!found)
                    cout<<"  " << ". "<<"  ";
            }
            cout << "\n";
            cout<< "--------------------------------\n";
        }
    }

    // Loads warehouse data from a file.
    void loadWarehouse() {
        ifstream inFile("warehouse_data.txt");
        if (!inFile) {
            cout << "No saved warehouse data found.\n";
            return;
        }

        int fileSizeX, fileSizeY;
        if (!(inFile >> fileSizeX >> fileSizeY)) {
            cout << "No valid saved warehouse data found.\n";
            inFile.close();
            return;
        }
        if (fileSizeX <= 0 || fileSizeY <= 0) {
            cerr << "Invalid warehouse dimensions in file.\n";
            inFile.close();
            return;
        }

        cout << "Saved warehouse dimensions: " << fileSizeX << " x " << fileSizeY << "\n";
        if (fileSizeX != sizeX || fileSizeY != sizeY) {
            cout << "Current warehouse dimensions: " << sizeX << " x " << sizeY << "\n";
            cout << "Do you want to override the current dimensions with the saved data? (Y/N): ";
            char overrideChoice;
            cin >> overrideChoice;
            if (overrideChoice == 'Y' || overrideChoice == 'y') {
                sizeX = fileSizeX;
                sizeY = fileSizeY;
                resetOccupied();
                items.clear();
                cout << "Warehouse dimensions updated to " << sizeX << " x " << sizeY << ".\n";
            }
        }

        inFile >> entry.first >> entry.second >> exitPoint.first >> exitPoint.second;
        // Validate entry and exit points.
        if (entry.first < 0 || entry.first >= sizeX || entry.second < 0 || entry.second >= sizeY) {
            cout << "Invalid entry point in file. Resetting to (0,0).\n";
            entry = {0, 0};
        }
        if (exitPoint.first < 0 || exitPoint.first >= sizeX || exitPoint.second < 0 || exitPoint.second >= sizeY) {
            cout << "Invalid exit point in file. Resetting to (" << sizeX - 1 << ", " << sizeY - 1 << ").\n";
            exitPoint = {sizeX - 1, sizeY - 1};
        }
    
        items.clear();
        string id;
        int priority, quantity, x, y;
        while (inFile >> id >> priority >> quantity >> x >> y) {
            if (x < 0 || x >= sizeX || y < 0 || y >= sizeY) {
                cerr << "Invalid item position in file: (" << x << ", " << y << ")\n";
                continue;
            }
            items.push_back(Item(id, quantity, priority, x, y));
            markOccupied(x, y);
        }
        inFile.close();
        cout << "Warehouse data loaded successfully from file.\n";
    }

    // Saves warehouse data to a file.
    void saveWarehouse() {
        ofstream outFile("warehouse_data.txt");
        if (!outFile) {
            cerr << "Error opening file for writing.\n";
            return;
        }
    
        outFile << sizeX << " " << sizeY << "\n";
        outFile << entry.first << " " << entry.second << " "
                << exitPoint.first << " " << exitPoint.second << "\n";
    
        for (const auto &item : items) {
            outFile << item.getId() << " " << item.getPriority() << " "
                    << item.getQuantity() << " " << item.getPositionX() << " "
                    << item.getPositionY() << "\n";
        }
        outFile.close();
        cout << "Warehouse data saved successfully.\n";
    }
};

int main() {
    try {
        // Create warehouse using initial dimensions provided by the user.
        Warehouse warehouse;
        
        // Display current dimensions and ask if the user wants to change them.
        auto dims = warehouse.getDimensions();
        cout << "\nCurrent warehouse dimensions: " << dims.first << " x " << dims.second << "\n";
        cout << "Do you want to change the warehouse dimensions? (Y/N): ";
        char changeDim;
        cin >> changeDim;
        if (changeDim == 'Y' || changeDim == 'y')
            warehouse.changeDimensions();
        
        // Ask the user if they want to load saved data from file.
        cout << "Do you want to load saved warehouse data from file? (Y/N): ";
        char loadData;
        cin >> loadData;
        if (loadData == 'Y' || loadData == 'y')
            warehouse.loadWarehouse();
    
        int choice;
        do {
            cout << "\nWarehouse Management System\n";
            cout << "1. Add Item\n";
            cout << "2. Delete Item\n";
            cout << "3. Reorder Items\n";
            cout << "4. View Items\n";
            cout << "5. Save Warehouse\n";
            cout << "6. Display Warehouse Layout\n";
            cout << "7. Set Entry and Exit Port\n";
            cout << "8. Exit\n";
            cout << "Enter your choice: ";
            cin >> choice;
    
            switch (choice) {
                case 1:
                    warehouse.addItem();
                    break;
                case 2: {
                    string id;
                    cout << "Enter Item ID to delete: ";
                    cin >> id;
                    warehouse.deleteItem(id);
                    break;
                }
                case 3:
                    warehouse.reorderItems();
                    break;
                case 4:
                    warehouse.viewItems();
                    break;
                case 5:
                    warehouse.saveWarehouse();
                    break;
                case 6:
                    warehouse.displayWarehouse();
                    break;
                case 7:
                    warehouse.setEntryExitPort();
                    break;
                case 8:
                    warehouse.saveWarehouse();
                    cout << "Exiting the system.\n";
                    break;
                default:
                    cout << "Invalid choice. Please try again.\n";
            }
        } while (choice != 8);
    } catch (const exception &ex) {
        cerr << "Error: " << ex.what() << "\n";
    }
    return 0;
}
