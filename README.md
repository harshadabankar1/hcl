# 🌳 Company Org Chart & Employee Manager - C++

A powerful console-based Organisation Hierarchy Management System built in C++ using **N-ary Tree Data Structure**. It manages employees like a real company tree - CEO at root, managers as child nodes, with complete CRUD, transfer, and analytics.

![C++](https://img.shields.io/badge/Language-C++-blue)
![DS](https://img.shields.io/badge/DS-N--ary%20Tree%20%7C%20BFS%20%7C%20DFS-green)
![Pattern](https://img.shields.io/badge/Pattern-Tree%20with%20Raw%20Pointers-orange)

### ✨ Features

#### 1. Organisation Management
- Create New Organisation with CEO (Root Node)
- Load / Save Organisation from file (`ORG_TREE_V2` format)
- Organisation Name & Type / Industry tracking

#### 2. Employee Operations
- **Add Employee** under any manager
- **Remove Employee** - deletes employee + his entire team (recursive delete)
- **Transfer Employee** - move an employee with his whole subtree to a new manager
- Prevents invalid transfer (e.g., moving a manager under his own subordinate - cycle check)

#### 3. Traversals & Analytics
- **Display Hierarchy (Preorder DFS):** Visual tree with indentation `|-`
- **Postorder Traversal:** For bottom-up view
- **Reporting Chain:** Shows path from Employee -> ... -> CEO
- **Employee Depth:** How far from CEO (Level)
- **Organisation Height:** Longest chain from CEO (in edges)
- **Team Details:** Direct reports + Total team size (recursive)
- **Employees Per Level (BFS):** Uses `queue` to count employees at each level
- **Organisation Report:** Full table with ID, Name, Designation, Direct Reports, Team Size

### 🏗️ Data Structure Explained (Interview Gold)

This is NOT a simple linked list. This is a **Generic N-ary Tree.**

```cpp
class Employee {
    string id, name, designation;
    vector<Employee*> reports; // N children
    // Each employee can have N direct reports
};

class OrgTree {
    Employee* root; // CEO is root
};
