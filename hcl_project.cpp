#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <queue>
#include <algorithm>
#include <limits>

using namespace std;

class OrgTree; // Forward declaration

// Employee represents one node in the organisation tree.
class Employee {
private:
    string id;
    string name;
    string designation;
    vector<Employee*> reports; // Employees directly reporting to this employee

    friend class OrgTree; // OrgTree manages the employee's private data

public:
    Employee(string empId, string empName, string role)
        : id(empId), name(empName), designation(role) {}

    ~Employee() {
        for (Employee* employee : reports) {
            delete employee; // Delete the complete team below this employee
        }
    }
};

class OrgTree {
private:
    Employee* root;
    string organisationName;
    string organisationType;

    // Find an employee anywhere in the tree.
    Employee* findEmployee(Employee* current, const string& id) const {
        if (current == nullptr) return nullptr;
        if (current->id == id) return current;

        for (Employee* child : current->reports) {
            Employee* result = findEmployee(child, id);
            if (result != nullptr) return result;
        }
        return nullptr;
    }

    // Find the manager (parent) of a particular employee.
    Employee* findParent(Employee* current, const string& id) const {
        if (current == nullptr) return nullptr;

        for (Employee* child : current->reports) {
            if (child->id == id) return current;

            Employee* result = findParent(child, id);
            if (result != nullptr) return result;
        }
        return nullptr;
    }

    // Check whether an ID exists below the given employee.
    bool isInSubtree(Employee* current, const string& id) const {
        if (current == nullptr) return false;
        if (current->id == id) return true;

        for (Employee* child : current->reports) {
            if (isInSubtree(child, id)) return true;
        }
        return false;
    }

    // Preorder: print the current employee before their reports.
    void preorder(Employee* current, int level) const {
        if (current == nullptr) return;

        cout << string(level * 4, ' ') << "|- "
             << current->name << " [" << current->id << "] - "
             << current->designation << '\n';

        for (Employee* child : current->reports) {
            preorder(child, level + 1);
        }
    }

    // Postorder: print reports before the current employee.
    void postorder(Employee* current) const {
        if (current == nullptr) return;

        for (Employee* child : current->reports) {
            postorder(child);
        }
        cout << current->name << " [" << current->id << "]\n";
    }

    int calculateHeight(Employee* current) const {
        if (current == nullptr) return -1;

        int largestChildHeight = -1;
        for (Employee* child : current->reports) {
            largestChildHeight = max(largestChildHeight,
                                     calculateHeight(child));
        }
        return largestChildHeight + 1;
    }

    int countTeam(Employee* current) const {
        if (current == nullptr) return 0;

        int total = 0;
        for (Employee* child : current->reports) {
            total += 1 + countTeam(child);
        }
        return total;
    }

    // Save each employee with their manager's ID. Parent is saved first.
    void saveEmployees(Employee* current, const string& parentId,
                       ofstream& file) const {
        if (current == nullptr) return;

        file << current->id << ' ' << parentId << ' '
             << quoted(current->name) << ' '
             << quoted(current->designation) << '\n';

        for (Employee* child : current->reports) {
            saveEmployees(child, current->id, file);
        }
    }

public:
    OrgTree() : root(nullptr), organisationName("Unnamed organisation"),
                organisationType("Not specified") {}

    ~OrgTree() {
        delete root;
    }

    // Prevent accidental copying of a tree containing raw pointers.
    OrgTree(const OrgTree&) = delete;
    OrgTree& operator=(const OrgTree&) = delete;

    bool isEmpty() const {
        return root == nullptr;
    }

    bool createRoot(const string& id, const string& name,
                    const string& designation, const string& orgName,
                    const string& orgType) {
        if (root != nullptr || id.empty() || id == "0" ||
            name.empty() || designation.empty() || orgName.empty() ||
            orgType.empty()) {
            return false;
        }
        root = new Employee(id, name, designation);
        organisationName = orgName;
        organisationType = orgType;
        return true;
    }

    bool addEmployee(const string& managerId, const string& id,
                     const string& name, const string& designation) {
        if (id.empty() || id == "0" || name.empty() || designation.empty())
            return false;
        if (findEmployee(root, id) != nullptr) return false;

        Employee* manager = findEmployee(root, managerId);
        if (manager == nullptr) return false;

        manager->reports.push_back(new Employee(id, name, designation));
        return true;
    }

    bool removeEmployee(const string& id) {
        if (root == nullptr || root->id == id) return false; // CEO stays

        Employee* parent = findParent(root, id);
        if (parent == nullptr) return false;

        auto& team = parent->reports;
        auto position = find_if(team.begin(), team.end(),
            [&](Employee* employee) { return employee->id == id; });

        if (position == team.end()) return false;

        delete *position; // Destructor also deletes this employee's team
        team.erase(position);
        return true;
    }

    bool transferEmployee(const string& employeeId,
                          const string& newManagerId) {
        if (root == nullptr || employeeId == root->id ||
            employeeId == newManagerId) return false;

        Employee* employee = findEmployee(root, employeeId);
        Employee* newManager = findEmployee(root, newManagerId);
        if (employee == nullptr || newManager == nullptr) return false;

        // Prevent moving an employee under their own subordinate.
        if (isInSubtree(employee, newManagerId)) return false;

        Employee* oldManager = findParent(root, employeeId);
        if (oldManager == nullptr) return false;

        auto& oldTeam = oldManager->reports;
        auto position = find_if(oldTeam.begin(), oldTeam.end(),
            [&](Employee* item) { return item->id == employeeId; });
        if (position == oldTeam.end()) return false;

        oldTeam.erase(position); // Remove pointer from old manager
        newManager->reports.push_back(employee); // Attach same employee node
        return true;
    }

    void displayHierarchy() const {
        if (root == nullptr) {
            cout << "Organisation is empty.\n";
            return;
        }
        cout << "\nORGANISATION HIERARCHY (PREORDER)\n";
        preorder(root, 0);
    }

    void displayPostorder() const {
        if (root == nullptr) {
            cout << "Organisation is empty.\n";
            return;
        }
        cout << "\nPOSTORDER TRAVERSAL\n";
        postorder(root);
    }

    void showReportingChain(const string& id) const {
        vector<Employee*> path;
        Employee* target = findEmployee(root, id);
        if (target == nullptr) {
            cout << "Employee not found.\n";
            return;
        }

        // Build the path from CEO to employee, then print it backwards.
        buildPath(root, id, path);
        cout << "Reporting chain (employee to CEO): ";
        for (int i = static_cast<int>(path.size()) - 1; i >= 0; --i) {
            cout << path[i]->name;
            if (i > 0) cout << " -> ";
        }
        cout << '\n';
    }

    // Helper used to build the path from root to the requested employee.
    bool buildPath(Employee* current, const string& id,
                   vector<Employee*>& path) const {
        if (current == nullptr) return false;
        path.push_back(current);
        if (current->id == id) return true;

        for (Employee* child : current->reports) {
            if (buildPath(child, id, path)) return true;
        }
        path.pop_back();
        return false;
    }

    void showDepth(const string& id) const {
        vector<Employee*> path;
        if (!buildPath(root, id, path)) {
            cout << "Employee not found.\n";
            return;
        }
        cout << "Depth of employee " << id << " = "
             << path.size() - 1 << '\n';
    }

    void showHeight() const {
        if (root == nullptr) {
            cout << "Organisation is empty.\n";
            return;
        }
        cout << "Organisation height = " << calculateHeight(root)
             << " edge(s).\n";
    }

    void showTeamDetails(const string& id) const {
        Employee* employee = findEmployee(root, id);
        if (employee == nullptr) {
            cout << "Employee not found.\n";
            return;
        }
        cout << "Employee: " << employee->name << '\n';
        cout << "Direct reports: " << employee->reports.size() << '\n';
        cout << "Total team size: " << countTeam(employee) << '\n';
    }

    // Breadth-first search counts employees at each level.
    void showEmployeesPerLevel() const {
        if (root == nullptr) {
            cout << "Organisation is empty.\n";
            return;
        }

        queue<pair<Employee*, int>> pending;
        vector<int> counts;
        pending.push({root, 0});

        while (!pending.empty()) {
            Employee* current = pending.front().first;
            int level = pending.front().second;
            pending.pop();

            if (static_cast<int>(counts.size()) <= level)
                counts.push_back(0);
            counts[level]++;

            for (Employee* child : current->reports)
                pending.push({child, level + 1});
        }

        cout << "Employees per level:\n";
        for (size_t i = 0; i < counts.size(); ++i)
            cout << "Level " << i << ": " << counts[i] << '\n';
    }

    void showOrganisationReport() const {
        if (root == nullptr) {
            cout << "Organisation is empty.\n";
            return;
        }

        cout << "\n================ ORGANISATION REPORT ================\n";
        cout << "Organisation Name : " << organisationName << '\n';
        cout << "Type / Industry   : " << organisationType << '\n';
        cout << "Total Employees   : " << 1 + countTeam(root) << '\n';
        cout << "Organisation Height: " << calculateHeight(root) << " edge(s)\n\n";

        cout << left
             << setw(12) << "ID"
             << setw(24) << "Employee Name"
             << setw(24) << "Designation"
             << setw(18) << "Direct Reports"
             << setw(12) << "Team Size" << '\n';
        cout << string(90, '-') << '\n';

        queue<Employee*> pending;
        pending.push(root);
        while (!pending.empty()) {
            Employee* current = pending.front();
            pending.pop();
            cout << left
                 << setw(12) << current->id
                 << setw(24) << current->name
                 << setw(24) << current->designation
                 << setw(18) << current->reports.size()
                 << setw(12) << countTeam(current) << '\n';
            for (Employee* child : current->reports)
                pending.push(child);
        }
        cout << string(90, '-') << '\n';
    }

    bool saveToFile(const string& filename) const {
        if (root == nullptr) return false;
        ofstream file(filename);
        if (!file) return false;
        file << "ORG_TREE_V2\n";
        file << quoted(organisationName) << ' ' << quoted(organisationType) << '\n';
        saveEmployees(root, "0", file);
        return static_cast<bool>(file);
    }

    bool loadFromFile(const string& filename) {
        ifstream file(filename);
        if (!file) return false;

        OrgTree newTree; // Build separately so a failed load keeps old data
        string id, parentId, name, designation;
        bool firstRecord = true;

        if (!(file >> id)) return false;

        // Read organisation details from the new format. Older files that
        // start directly with an employee ID are still supported.
        if (id == "ORG_TREE_V2") {
            if (!(file >> quoted(newTree.organisationName)
                      >> quoted(newTree.organisationType))) return false;
            if (!(file >> id)) return false;
        }

        do {
            if (!(file >> parentId >> quoted(name) >> quoted(designation)))
                return false; // Incomplete or damaged record

            if (firstRecord) {
                if (parentId != "0" ||
                    !newTree.createRoot(id, name, designation,
                                        newTree.organisationName,
                                        newTree.organisationType)) return false;
                firstRecord = false;
            } else {
                // Saved files are parent-first, so the manager must exist.
                if (!newTree.addEmployee(parentId, id, name, designation))
                    return false;
            }
        } while (file >> id);

        if (firstRecord || !file.eof()) return false;

        delete root;
        root = newTree.root;
        newTree.root = nullptr;
        organisationName = newTree.organisationName;
        organisationType = newTree.organisationType;
        return true;
    }
};

string readLine(const string& prompt) {
    cout << prompt;
    string value;
    getline(cin, value);
    return value;
}

int main() {
    OrgTree organisation;
    int startChoice;

    cout << "========================================\n";
    cout << " COMPANY ORG CHART & EMPLOYEE MANAGER\n";
    cout << "========================================\n";
    cout << "1. Create new organisation\n";
    cout << "2. Load organisation from file\n";
    cout << "0. Exit\n";
    cout << "Enter choice: ";

    if (!(cin >> startChoice)) return 0;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (startChoice == 0) return 0;

    if (startChoice == 1) {
        string orgName = readLine("Organisation name: ");
        string orgType = readLine("Organisation type/industry: ");
        string id = readLine("CEO ID: ");
        string name = readLine("CEO name: ");
        string role = readLine("CEO designation: ");
        if (!organisation.createRoot(id, name, role, orgName, orgType)) {
            cout << "Invalid CEO details.\n";
            return 1;
        }
    } else if (startChoice == 2) {
        string filename = readLine("File name: ");
        if (!organisation.loadFromFile(filename)) {
            cout << "Could not load organisation.\n";
            return 1;
        }
        cout << "Organisation loaded successfully.\n";
    } else {
        cout << "Invalid choice.\n";
        return 1;
    }

    int choice;
    do {
        cout << "\n--------------- MAIN MENU ---------------\n";
        cout << "1. Add employee\n";
        cout << "2. Remove employee and their team\n";
        cout << "3. Transfer employee to another manager\n";
        cout << "4. Display hierarchy (preorder)\n";
        cout << "5. Display postorder traversal\n";
        cout << "6. Show reporting chain\n";
        cout << "7. Show employee depth\n";
        cout << "8. Show organisation height\n";
        cout << "9. Show team size and direct reports\n";
        cout << "10. Show employees per level (BFS)\n";
        cout << "11. Generate organisation report\n";
        cout << "12. Save organisation\n";
        cout << "0. Exit\n";
        cout << "-----------------------------------------\n";
        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Please enter a valid menu number.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        string id, name, role, managerId, filename;

        switch (choice) {
        case 1:
            managerId = readLine("Manager ID: ");
            id = readLine("New employee ID: ");
            name = readLine("Employee name: ");
            role = readLine("Designation: ");
            cout << (organisation.addEmployee(managerId, id, name, role)
                     ? "Employee added successfully.\n"
                     : "Could not add employee. Check the entered details.\n");
            break;

        case 2:
            id = readLine("Employee ID to remove: ");
            cout << (organisation.removeEmployee(id)
                     ? "Employee and their team removed.\n"
                     : "Removal failed. Check the ID; CEO cannot be removed.\n");
            break;

        case 3:
            id = readLine("Employee ID to transfer: ");
            managerId = readLine("New manager ID: ");
            cout << (organisation.transferEmployee(id, managerId)
                     ? "Employee transferred successfully.\n"
                     : "Transfer failed. Check IDs and reporting hierarchy.\n");
            break;

        case 4:
            organisation.displayHierarchy();
            break;
        case 5:
            organisation.displayPostorder();
            break;
        case 6:
            id = readLine("Employee ID: ");
            organisation.showReportingChain(id);
            break;
        case 7:
            id = readLine("Employee ID: ");
            organisation.showDepth(id);
            break;
        case 8:
            organisation.showHeight();
            break;
        case 9:
            id = readLine("Employee ID: ");
            organisation.showTeamDetails(id);
            break;
        case 10:
            organisation.showEmployeesPerLevel();
            break;
        case 11:
            organisation.showOrganisationReport();
            break;
        case 12:
            filename = readLine("File name to save (e.g. organisation.txt): ");
            cout << (organisation.saveToFile(filename)
                     ? "Organisation saved successfully.\n"
                     : "Could not save organisation.\n");
            break;
        case 0:
            cout << "Thank you for using the application.\n";
            break;
        default:
            cout << "Invalid choice. Select 0 to 12.\n";
        }
    } while (choice != 0);

    return 0;
}