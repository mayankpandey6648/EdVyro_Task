/*
    Library Manager - Algorithms and Automated Testing
    Internship Task 04

    Features:
    1. Linear Search
    2. Binary Search
    3. Bubble Sort
    4. Merge Sort
    5. Library reporting
    6. Automated unit tests:
       - normal cases
       - boundary cases
       - invalid cases
    7. Compiler-friendly C++17 code
    8. No manual memory allocation; RAII/STL prevents memory leaks

    Complexity:
    Linear Search:
        Time  : O(n) worst case, O(1) best case
        Space : O(1)

    Binary Search:
        Time  : O(log n)
        Space : O(1) iterative
        Note  : Data must be sorted by ID.

    Bubble Sort:
        Time  : O(n^2) worst/average, O(n) best with early exit
        Space : O(1)

    Merge Sort:
        Time  : O(n log n) best/average/worst
        Space : O(n) auxiliary

    Run:
        g++ -std=c++17 -Wall -Wextra -pedantic library_manager.cpp -o library_manager
        ./library_manager

    The program first runs automated tests and then opens the menu.
*/

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Book {
    int id{};
    string title;
    string author;
    int year{};
    bool issued{false};

    bool operator==(const Book& other) const {
        return id == other.id &&
               title == other.title &&
               author == other.author &&
               year == other.year &&
               issued == other.issued;
    }
};

// ------------------------------------------------------------
// Utility functions
// ------------------------------------------------------------

void printSeparator() {
    cout << string(78, '-') << '\n';
}

void printBook(const Book& book) {
    cout << left
         << setw(6) << book.id
         << setw(28) << book.title.substr(0, 27)
         << setw(22) << book.author.substr(0, 21)
         << setw(8) << book.year
         << setw(12) << (book.issued ? "Issued" : "Available")
         << '\n';
}

void printBooks(const vector<Book>& books) {
    if (books.empty()) {
        cout << "No books found.\n";
        return;
    }

    printSeparator();
    cout << left
         << setw(6) << "ID"
         << setw(28) << "Title"
         << setw(22) << "Author"
         << setw(8) << "Year"
         << setw(12) << "Status" << '\n';
    printSeparator();

    for (const auto& book : books) {
        printBook(book);
    }

    printSeparator();
}

bool containsCaseInsensitive(string text, string key) {
    transform(text.begin(), text.end(), text.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });
    transform(key.begin(), key.end(), key.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });

    return text.find(key) != string::npos;
}

// ------------------------------------------------------------
// Searching strategies
// ------------------------------------------------------------

// Strategy 1: Linear Search
int linearSearchById(const vector<Book>& books, int id) {
    for (size_t i = 0; i < books.size(); ++i) {
        if (books[i].id == id) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

vector<Book> linearSearchByTitle(const vector<Book>& books,
                                 const string& keyword) {
    vector<Book> result;

    for (const auto& book : books) {
        if (containsCaseInsensitive(book.title, keyword)) {
            result.push_back(book);
        }
    }

    return result;
}

// Strategy 2: Binary Search
// Requires books to be sorted by ID.
int binarySearchById(const vector<Book>& books, int id) {
    int left = 0;
    int right = static_cast<int>(books.size()) - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (books[mid].id == id) {
            return mid;
        }

        if (books[mid].id < id) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return -1;
}

// ------------------------------------------------------------
// Sorting strategies
// ------------------------------------------------------------

// Strategy 1: Bubble Sort
void bubbleSortById(vector<Book>& books) {
    if (books.size() < 2) {
        return;
    }

    for (size_t i = 0; i < books.size() - 1; ++i) {
        bool swapped = false;

        for (size_t j = 0; j < books.size() - i - 1; ++j) {
            if (books[j].id > books[j + 1].id) {
                swap(books[j], books[j + 1]);
                swapped = true;
            }
        }

        if (!swapped) {
            break;
        }
    }
}

// Merge helper for Merge Sort
void mergeBooks(vector<Book>& books, int left, int mid, int right) {
    vector<Book> temp;
    temp.reserve(static_cast<size_t>(right - left + 1));

    int i = left;
    int j = mid + 1;

    while (i <= mid && j <= right) {
        if (books[i].id <= books[j].id) {
            temp.push_back(books[i++]);
        } else {
            temp.push_back(books[j++]);
        }
    }

    while (i <= mid) {
        temp.push_back(books[i++]);
    }

    while (j <= right) {
        temp.push_back(books[j++]);
    }

    for (size_t k = 0; k < temp.size(); ++k) {
        books[left + static_cast<int>(k)] = temp[k];
    }
}

void mergeSortRecursive(vector<Book>& books, int left, int right) {
    if (left >= right) {
        return;
    }

    int mid = left + (right - left) / 2;

    mergeSortRecursive(books, left, mid);
    mergeSortRecursive(books, mid + 1, right);
    mergeBooks(books, left, mid, right);
}

void mergeSortById(vector<Book>& books) {
    if (books.size() > 1) {
        mergeSortRecursive(books, 0,
                           static_cast<int>(books.size()) - 1);
    }
}

// ------------------------------------------------------------
// Reporting
// ------------------------------------------------------------

void showReport(const vector<Book>& books) {
    int issued = 0;
    int available = 0;

    for (const auto& book : books) {
        if (book.issued) {
            ++issued;
        } else {
            ++available;
        }
    }

    cout << "\n========== LIBRARY REPORT ==========\n";
    cout << "Total books     : " << books.size() << '\n';
    cout << "Available books : " << available << '\n';
    cout << "Issued books    : " << issued << '\n';

    if (!books.empty()) {
        int oldest = books.front().year;
        int newest = books.front().year;

        for (const auto& book : books) {
            oldest = min(oldest, book.year);
            newest = max(newest, book.year);
        }

        cout << "Oldest year     : " << oldest << '\n';
        cout << "Newest year     : " << newest << '\n';
    }

    cout << "====================================\n";
}

// ------------------------------------------------------------
// Input validation
// ------------------------------------------------------------

int readInt(const string& prompt) {
    while (true) {
        cout << prompt;

        int value;
        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }

        cout << "Invalid input. Please enter a number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

string readNonEmptyLine(const string& prompt) {
    while (true) {
        cout << prompt;

        string value;
        getline(cin, value);

        if (!value.empty()) {
            return value;
        }

        cout << "Input cannot be empty.\n";
    }
}

// ------------------------------------------------------------
// Library operations
// ------------------------------------------------------------

bool idExists(const vector<Book>& books, int id) {
    return linearSearchById(books, id) != -1;
}

void addBook(vector<Book>& books) {
    cout << "\n========== ADD BOOK ==========\n";

    int id = readInt("Enter book ID: ");

    if (id <= 0) {
        cout << "Book ID must be positive.\n";
        return;
    }

    if (idExists(books, id)) {
        cout << "A book with this ID already exists.\n";
        return;
    }

    string title = readNonEmptyLine("Enter title: ");
    string author = readNonEmptyLine("Enter author: ");
    int year = readInt("Enter publication year: ");

    if (year <= 0) {
        cout << "Invalid publication year.\n";
        return;
    }

    books.push_back({id, title, author, year, false});

    cout << "Book added successfully.\n";
}

void issueOrReturnBook(vector<Book>& books) {
    int id = readInt("Enter book ID: ");
    int index = linearSearchById(books, id);

    if (index == -1) {
        cout << "Book not found.\n";
        return;
    }

    books[index].issued = !books[index].issued;

    cout << "Book status changed to: "
         << (books[index].issued ? "Issued" : "Available") << '\n';
}

void searchBook(const vector<Book>& books) {
    cout << "\n========== SEARCH ==========\n";
    cout << "1. Linear Search by ID\n";
    cout << "2. Binary Search by ID\n";
    cout << "3. Search by title keyword\n";

    int choice = readInt("Choose an option: ");

    if (choice == 1) {
        int id = readInt("Enter ID: ");
        int index = linearSearchById(books, id);

        if (index == -1) {
            cout << "Book not found.\n";
        } else {
            cout << "Book found:\n";
            printBooks({books[index]});
        }
    } else if (choice == 2) {
        int id = readInt("Enter ID: ");

        vector<Book> sortedBooks = books;
        mergeSortById(sortedBooks);

        int index = binarySearchById(sortedBooks, id);

        if (index == -1) {
            cout << "Book not found.\n";
        } else {
            cout << "Book found using binary search:\n";
            printBooks({sortedBooks[index]});
        }
    } else if (choice == 3) {
        string keyword = readNonEmptyLine("Enter title keyword: ");
        vector<Book> result = linearSearchByTitle(books, keyword);

        if (result.empty()) {
            cout << "No matching books found.\n";
        } else {
            printBooks(result);
        }
    } else {
        cout << "Invalid option.\n";
    }
}

void sortBooks(vector<Book>& books) {
    cout << "\n========== SORT ==========\n";
    cout << "1. Bubble Sort\n";
    cout << "2. Merge Sort\n";

    int choice = readInt("Choose an option: ");

    if (choice == 1) {
        bubbleSortById(books);
        cout << "Books sorted using Bubble Sort.\n";
        printBooks(books);
    } else if (choice == 2) {
        mergeSortById(books);
        cout << "Books sorted using Merge Sort.\n";
        printBooks(books);
    } else {
        cout << "Invalid option.\n";
    }
}

// ------------------------------------------------------------
// Automated unit tests
// ------------------------------------------------------------

int testsPassed = 0;
int testsFailed = 0;

void testResult(const string& name, bool condition) {
    if (condition) {
        cout << "[PASS] " << name << '\n';
        ++testsPassed;
    } else {
        cout << "[FAIL] " << name << '\n';
        ++testsFailed;
    }
}

vector<Book> createTestBooks() {
    return {
        {105, "Computer Networks", "Andrew Tanenbaum", 2010, false},
        {101, "Clean Code", "Robert Martin", 2008, true},
        {110, "Operating Systems", "Abraham Silberschatz", 2018, false},
        {103, "C++ Primer", "Stanley Lippman", 2012, false},
        {107, "Algorithms", "Robert Sedgewick", 2011, true}
    };
}

void runAutomatedTests() {
    testsPassed = 0;
    testsFailed = 0;

    cout << "\n";
    printSeparator();
    cout << "AUTOMATED UNIT TESTS\n";
    printSeparator();

    // ---------------- Normal cases ----------------

    {
        vector<Book> books = createTestBooks();
        int index = linearSearchById(books, 103);

        testResult("Linear search - existing ID",
                   index != -1 && books[index].id == 103);
    }

    {
        vector<Book> books = createTestBooks();
        mergeSortById(books);
        int index = binarySearchById(books, 107);

        testResult("Binary search - existing ID",
                   index != -1 && books[index].id == 107);
    }

    {
        vector<Book> books = createTestBooks();
        bubbleSortById(books);

        bool sorted = true;
        for (size_t i = 1; i < books.size(); ++i) {
            if (books[i - 1].id > books[i].id) {
                sorted = false;
            }
        }

        testResult("Bubble sort - normal dataset", sorted);
    }

    {
        vector<Book> books = createTestBooks();
        mergeSortById(books);

        bool sorted = true;
        for (size_t i = 1; i < books.size(); ++i) {
            if (books[i - 1].id > books[i].id) {
                sorted = false;
            }
        }

        testResult("Merge sort - normal dataset", sorted);
    }

    {
        vector<Book> books = createTestBooks();
        vector<Book> result = linearSearchByTitle(books, "code");

        testResult("Title search - case insensitive",
                   result.size() == 1 &&
                   result[0].title == "Clean Code");
    }

    // ---------------- Boundary cases ----------------

    {
        vector<Book> emptyBooks;
        testResult("Linear search - empty collection",
                   linearSearchById(emptyBooks, 1) == -1);
    }

    {
        vector<Book> emptyBooks;
        mergeSortById(emptyBooks);
        testResult("Merge sort - empty collection",
                   emptyBooks.empty());
    }

    {
        vector<Book> singleBook = {
            {1, "Single Book", "Author", 2020, false}
        };

        bubbleSortById(singleBook);

        testResult("Bubble sort - single element",
                   singleBook.size() == 1 && singleBook[0].id == 1);
    }

    {
        vector<Book> singleBook = {
            {1, "Single Book", "Author", 2020, false}
        };

        int index = binarySearchById(singleBook, 1);

        testResult("Binary search - one element",
                   index == 0);
    }

    {
        vector<Book> books = {
            {1, "First", "A", 2000, false},
            {2, "Second", "B", 2001, false},
            {3, "Third", "C", 2002, false}
        };

        int first = binarySearchById(books, 1);
        int last = binarySearchById(books, 3);

        testResult("Binary search - boundary elements",
                   first == 0 && last == 2);
    }

    // ---------------- Invalid / negative cases ----------------

    {
        vector<Book> books = createTestBooks();

        testResult("Linear search - missing ID",
                   linearSearchById(books, 9999) == -1);
    }

    {
        vector<Book> books = createTestBooks();
        mergeSortById(books);

        testResult("Binary search - missing ID",
                   binarySearchById(books, 9999) == -1);
    }

    {
        vector<Book> books = createTestBooks();
        vector<Book> result = linearSearchByTitle(books, "this-does-not-exist");

        testResult("Title search - invalid keyword",
                   result.empty());
    }

    {
        vector<Book> books = {
            {3, "C", "Author", 2020, false},
            {2, "B", "Author", 2020, false},
            {1, "A", "Author", 2020, false}
        };

        mergeSortById(books);

        testResult("Merge sort - reverse ordered input",
                   books[0].id == 1 &&
                   books[1].id == 2 &&
                   books[2].id == 3);
    }

    // ---------------- Data integrity ----------------

    {
        vector<Book> books = createTestBooks();
        vector<Book> original = books;

        bubbleSortById(books);

        bool allIDsPresent = true;
        for (const auto& oldBook : original) {
            if (linearSearchById(books, oldBook.id) == -1) {
                allIDsPresent = false;
            }
        }

        testResult("Sorting - no book lost", allIDsPresent);
    }

    printSeparator();
    cout << "Tests passed: " << testsPassed << '\n';
    cout << "Tests failed: " << testsFailed << '\n';

    if (testsFailed == 0) {
        cout << "TEST RESULT: ALL TESTS PASSED\n";
    } else {
        cout << "TEST RESULT: SOME TESTS FAILED\n";
    }

    printSeparator();
}

// ------------------------------------------------------------
// Complexity report
// ------------------------------------------------------------

void showComplexityNotes() {
    cout << "\n========== COMPLEXITY NOTES ==========\n";

    cout << "\n1. Linear Search\n";
    cout << "   Time : O(n) worst case, O(1) best case\n";
    cout << "   Space: O(1)\n";
    cout << "   Use  : Works even when data is unsorted.\n";

    cout << "\n2. Binary Search\n";
    cout << "   Time : O(log n)\n";
    cout << "   Space: O(1) for iterative implementation\n";
    cout << "   Use  : Requires IDs to be sorted.\n";

    cout << "\n3. Bubble Sort\n";
    cout << "   Time : O(n^2) worst/average, O(n) best case\n";
    cout << "   Space: O(1)\n";
    cout << "   Use  : Simple algorithm, suitable for small datasets.\n";

    cout << "\n4. Merge Sort\n";
    cout << "   Time : O(n log n) in all major cases\n";
    cout << "   Space: O(n) auxiliary memory\n";
    cout << "   Use  : Efficient for larger datasets.\n";

    cout << "\nMemory/resource handling:\n";
    cout << "   - Uses std::vector and std::string (RAII).\n";
    cout << "   - No raw new/delete operations are used.\n";
    cout << "   - Resources are automatically released by C++ containers.\n";

    cout << "\nCompiler warnings recommended:\n";
    cout << "   g++ -std=c++17 -Wall -Wextra -pedantic library_manager.cpp\n";

    cout << "======================================\n";
}

// ------------------------------------------------------------
// Main menu
// ------------------------------------------------------------

void displayMenu() {
    cout << "\n========== LIBRARY MANAGER ==========\n";
    cout << "1. Display all books\n";
    cout << "2. Add a book\n";
    cout << "3. Search for a book\n";
    cout << "4. Sort books\n";
    cout << "5. Issue / Return a book\n";
    cout << "6. Library report\n";
    cout << "7. Show complexity notes\n";
    cout << "8. Run automated tests\n";
    cout << "0. Exit\n";
    cout << "=====================================\n";
}

int main() {
    // Sample data. This also gives an immediate working demonstration.
    vector<Book> books = {
        {105, "Computer Networks", "Andrew Tanenbaum", 2010, false},
        {101, "Clean Code", "Robert Martin", 2008, true},
        {110, "Operating Systems", "Abraham Silberschatz", 2018, false},
        {103, "C++ Primer", "Stanley Lippman", 2012, false},
        {107, "Algorithms", "Robert Sedgewick", 2011, true},
        {102, "Database Systems", "Raghu Ramakrishnan", 2015, false},
        {109, "Computer Architecture", "David Patterson", 2017, true},
        {104, "Artificial Intelligence", "Stuart Russell", 2021, false},
        {108, "Data Structures", "Mark Allen Weiss", 2013, false},
        {106, "Software Engineering", "Ian Sommerville", 2016, false}
    };

    cout << "============================================\n";
    cout << "       LIBRARY MANAGER - TASK 04\n";
    cout << " Algorithms and Automated Testing\n";
    cout << "============================================\n";

    // Automated tests run first so the submission demonstrates testing.
    runAutomatedTests();

    while (true) {
        displayMenu();

        int choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1:
                printBooks(books);
                break;

            case 2:
                addBook(books);
                break;

            case 3:
                searchBook(books);
                break;

            case 4:
                sortBooks(books);
                break;

            case 5:
                issueOrReturnBook(books);
                break;

            case 6:
                showReport(books);
                break;

            case 7:
                showComplexityNotes();
                break;

            case 8:
                runAutomatedTests();
                break;

            case 0:
                cout << "Thank you for using Library Manager.\n";
                return 0;

            default:
                cout << "Invalid choice. Please select a valid menu option.\n";
        }
    }
}
