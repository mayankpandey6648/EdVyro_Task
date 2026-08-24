/*
    ================================================================
    C++ APPLICATION CAPSTONE
    Library Manager - Release Ready Version
    ================================================================

    Purpose:
      A dependable command-line library management application
      demonstrating clean architecture, validation, reporting,
      error handling, and automated self-tests.

    Build:
      g++ -std=c++17 -Wall -Wextra -pedantic library_capstone.cpp -o library_capstone

    Run:
      ./library_capstone
      Windows: library_capstone.exe

    Architecture:
      - Book              : domain model
      - LibraryManager    : business logic and data operations
      - Reports           : analytics/reporting
      - Tests              : automated verification
      - Application/Menu  : user interface

    The application uses standard C++ containers and RAII, so there
    are no raw new/delete operations or manual resource ownership.

    Capstone requirements covered:
      1. Refactored naming, implementation, validation, and errors
      2. Useful reporting/analytics
      3. Automated tests and clean-build instructions
      4. Architecture and trade-off documentation in this source
*/

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// ============================================================================
// DOMAIN MODEL
// ============================================================================

struct Book {
    int id = 0;
    string title;
    string author;
    int publicationYear = 0;
    bool issued = false;
};

// ============================================================================
// RESULT / ERROR HANDLING
// ============================================================================

enum class OperationStatus {
    Success,
    NotFound,
    AlreadyExists,
    InvalidInput,
    OperationNotAllowed
};

struct OperationResult {
    OperationStatus status;
    string message;

    bool ok() const {
        return status == OperationStatus::Success;
    }
};

// ============================================================================
// LIBRARY MANAGER - BUSINESS LOGIC
// ============================================================================

class LibraryManager {
private:
    vector<Book> books;

    static bool containsIgnoreCase(const string& text,
                                   const string& keyword) {
        string a = text;
        string b = keyword;

        transform(a.begin(), a.end(), a.begin(),
                  [](unsigned char c) {
                      return static_cast<char>(tolower(c));
                  });

        transform(b.begin(), b.end(), b.begin(),
                  [](unsigned char c) {
                      return static_cast<char>(tolower(c));
                  });

        return a.find(b) != string::npos;
    }

public:
    LibraryManager() = default;

    explicit LibraryManager(const vector<Book>& initialBooks)
        : books(initialBooks) {}

    // ----------------------------
    // Validation
    // ----------------------------

    static bool isValidBook(const Book& book) {
        return book.id > 0 &&
               !book.title.empty() &&
               !book.author.empty() &&
               book.publicationYear >= 1000 &&
               book.publicationYear <= 2100;
    }

    bool idExists(int id) const {
        return findById(id) != nullptr;
    }

    // ----------------------------
    // CRUD / business operations
    // ----------------------------

    OperationResult addBook(const Book& book) {
        if (!isValidBook(book)) {
            return {
                OperationStatus::InvalidInput,
                "Invalid book. ID/title/author/year must be valid."
            };
        }

        if (idExists(book.id)) {
            return {
                OperationStatus::AlreadyExists,
                "A book with this ID already exists."
            };
        }

        books.push_back(book);

        return {
            OperationStatus::Success,
            "Book added successfully."
        };
    }

    OperationResult removeBook(int id) {
        auto it = find_if(books.begin(), books.end(),
                          [id](const Book& book) {
                              return book.id == id;
                          });

        if (it == books.end()) {
            return {
                OperationStatus::NotFound,
                "Book not found."
            };
        }

        if (it->issued) {
            return {
                OperationStatus::OperationNotAllowed,
                "Issued books cannot be removed. Return the book first."
            };
        }

        books.erase(it);

        return {
            OperationStatus::Success,
            "Book removed successfully."
        };
    }

    OperationResult issueBook(int id) {
        Book* book = findById(id);

        if (book == nullptr) {
            return {
                OperationStatus::NotFound,
                "Book not found."
            };
        }

        if (book->issued) {
            return {
                OperationStatus::OperationNotAllowed,
                "Book is already issued."
            };
        }

        book->issued = true;

        return {
            OperationStatus::Success,
            "Book issued successfully."
        };
    }

    OperationResult returnBook(int id) {
        Book* book = findById(id);

        if (book == nullptr) {
            return {
                OperationStatus::NotFound,
                "Book not found."
            };
        }

        if (!book->issued) {
            return {
                OperationStatus::OperationNotAllowed,
                "Book is already available."
            };
        }

        book->issued = false;

        return {
            OperationStatus::Success,
            "Book returned successfully."
        };
    }

    // ----------------------------
    // Search
    // ----------------------------

    const Book* findById(int id) const {
        for (const auto& book : books) {
            if (book.id == id) {
                return &book;
            }
        }

        return nullptr;
    }

    Book* findById(int id) {
        for (auto& book : books) {
            if (book.id == id) {
                return &book;
            }
        }

        return nullptr;
    }

    vector<Book> searchByTitle(const string& keyword) const {
        vector<Book> result;

        if (keyword.empty()) {
            return result;
        }

        for (const auto& book : books) {
            if (containsIgnoreCase(book.title, keyword)) {
                result.push_back(book);
            }
        }

        return result;
    }

    vector<Book> searchByAuthor(const string& keyword) const {
        vector<Book> result;

        if (keyword.empty()) {
            return result;
        }

        for (const auto& book : books) {
            if (containsIgnoreCase(book.author, keyword)) {
                result.push_back(book);
            }
        }

        return result;
    }

    // ----------------------------
    // Sorting
    // ----------------------------

    void sortByIdAscending() {
        sort(books.begin(), books.end(),
             [](const Book& a, const Book& b) {
                 return a.id < b.id;
             });
    }

    void sortByYearDescending() {
        sort(books.begin(), books.end(),
             [](const Book& a, const Book& b) {
                 if (a.publicationYear != b.publicationYear) {
                     return a.publicationYear > b.publicationYear;
                 }

                 return a.id < b.id;
             });
    }

    // ----------------------------
    // Accessors
    // ----------------------------

    const vector<Book>& getBooks() const {
        return books;
    }

    size_t size() const {
        return books.size();
    }
};

// ============================================================================
// REPORTING / ANALYTICS
// ============================================================================

struct LibraryReport {
    size_t totalBooks = 0;
    size_t availableBooks = 0;
    size_t issuedBooks = 0;

    int oldestPublicationYear = 0;
    int newestPublicationYear = 0;

    double availabilityPercentage = 0.0;
    double issuePercentage = 0.0;

    string mostRepresentedAuthor;
};

LibraryReport generateReport(const LibraryManager& library) {
    LibraryReport report;

    const auto& books = library.getBooks();

    report.totalBooks = books.size();

    if (books.empty()) {
        return report;
    }

    unordered_map<string, int> authorCount;

    report.oldestPublicationYear = books.front().publicationYear;
    report.newestPublicationYear = books.front().publicationYear;

    for (const auto& book : books) {
        if (book.issued) {
            ++report.issuedBooks;
        } else {
            ++report.availableBooks;
        }

        report.oldestPublicationYear =
            min(report.oldestPublicationYear, book.publicationYear);

        report.newestPublicationYear =
            max(report.newestPublicationYear, book.publicationYear);

        ++authorCount[book.author];
    }

    report.availabilityPercentage =
        (static_cast<double>(report.availableBooks) /
         static_cast<double>(report.totalBooks)) * 100.0;

    report.issuePercentage =
        (static_cast<double>(report.issuedBooks) /
         static_cast<double>(report.totalBooks)) * 100.0;

    int highestCount = 0;

    for (const auto& entry : authorCount) {
        if (entry.second > highestCount) {
            highestCount = entry.second;
            report.mostRepresentedAuthor = entry.first;
        }
    }

    return report;
}

// ============================================================================
// USER INTERFACE HELPERS
// ============================================================================

void printLine() {
    cout << string(90, '-') << '\n';
}

void printHeader(const string& title) {
    cout << '\n';
    printLine();
    cout << "  " << title << '\n';
    printLine();
}

void printBookTable(const vector<Book>& books) {
    if (books.empty()) {
        cout << "No books found.\n";
        return;
    }

    cout << left
         << setw(7) << "ID"
         << setw(30) << "Title"
         << setw(25) << "Author"
         << setw(10) << "Year"
         << setw(12) << "Status" << '\n';

    printLine();

    for (const auto& book : books) {
        string title = book.title;
        string author = book.author;

        if (title.length() > 28) {
            title = title.substr(0, 28);
        }

        if (author.length() > 23) {
            author = author.substr(0, 23);
        }

        cout << left
             << setw(7) << book.id
             << setw(30) << title
             << setw(25) << author
             << setw(10) << book.publicationYear
             << setw(12) << (book.issued ? "Issued" : "Available")
             << '\n';
    }

    printLine();
}

int readInteger(const string& prompt) {
    while (true) {
        cout << prompt;

        int value;

        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }

        cout << "Error: Please enter a valid integer.\n";

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

string readText(const string& prompt) {
    while (true) {
        cout << prompt;

        string value;
        getline(cin, value);

        if (!value.empty()) {
            return value;
        }

        cout << "Error: Input cannot be empty.\n";
    }
}

void displayOperationResult(const OperationResult& result) {
    if (result.ok()) {
        cout << "[SUCCESS] " << result.message << '\n';
    } else {
        cout << "[ERROR] " << result.message << '\n';
    }
}

// ============================================================================
// APPLICATION OPERATIONS
// ============================================================================

void addBookUI(LibraryManager& library) {
    printHeader("ADD NEW BOOK");

    int id = readInteger("Enter book ID: ");

    if (id <= 0) {
        cout << "[ERROR] ID must be positive.\n";
        return;
    }

    string title = readText("Enter title: ");
    string author = readText("Enter author: ");
    int year = readInteger("Enter publication year: ");

    Book book;
    book.id = id;
    book.title = title;
    book.author = author;
    book.publicationYear = year;
    book.issued = false;

    displayOperationResult(library.addBook(book));
}

void removeBookUI(LibraryManager& library) {
    printHeader("REMOVE BOOK");

    int id = readInteger("Enter book ID: ");

    displayOperationResult(library.removeBook(id));
}

void issueBookUI(LibraryManager& library) {
    printHeader("ISSUE BOOK");

    int id = readInteger("Enter book ID: ");

    displayOperationResult(library.issueBook(id));
}

void returnBookUI(LibraryManager& library) {
    printHeader("RETURN BOOK");

    int id = readInteger("Enter book ID: ");

    displayOperationResult(library.returnBook(id));
}

void searchUI(const LibraryManager& library) {
    printHeader("SEARCH BOOKS");

    cout << "1. Search by ID\n";
    cout << "2. Search by title\n";
    cout << "3. Search by author\n";

    int choice = readInteger("Choose search method: ");

    if (choice == 1) {
        int id = readInteger("Enter ID: ");

        const Book* book = library.findById(id);

        if (book == nullptr) {
            cout << "[ERROR] Book not found.\n";
        } else {
            printBookTable({*book});
        }
    } else if (choice == 2) {
        string keyword = readText("Enter title keyword: ");
        vector<Book> result = library.searchByTitle(keyword);
        printBookTable(result);
    } else if (choice == 3) {
        string keyword = readText("Enter author keyword: ");
        vector<Book> result = library.searchByAuthor(keyword);
        printBookTable(result);
    } else {
        cout << "[ERROR] Invalid search option.\n";
    }
}

void sortingUI(LibraryManager& library) {
    printHeader("SORT BOOKS");

    cout << "1. Sort by ID (ascending)\n";
    cout << "2. Sort by publication year (newest first)\n";

    int choice = readInteger("Choose sorting method: ");

    if (choice == 1) {
        library.sortByIdAscending();
        cout << "[SUCCESS] Books sorted by ID.\n";
        printBookTable(library.getBooks());
    } else if (choice == 2) {
        library.sortByYearDescending();
        cout << "[SUCCESS] Books sorted by publication year.\n";
        printBookTable(library.getBooks());
    } else {
        cout << "[ERROR] Invalid sorting option.\n";
    }
}

void reportUI(const LibraryManager& library) {
    printHeader("LIBRARY ANALYTICS REPORT");

    LibraryReport report = generateReport(library);

    cout << fixed << setprecision(2);

    cout << "Total books              : " << report.totalBooks << '\n';
    cout << "Available books          : " << report.availableBooks << '\n';
    cout << "Issued books             : " << report.issuedBooks << '\n';
    cout << "Availability percentage  : "
         << report.availabilityPercentage << "%\n";
    cout << "Issue percentage         : "
         << report.issuePercentage << "%\n";

    if (report.totalBooks > 0) {
        cout << "Oldest publication year  : "
             << report.oldestPublicationYear << '\n';

        cout << "Newest publication year  : "
             << report.newestPublicationYear << '\n';

        cout << "Most represented author  : "
             << report.mostRepresentedAuthor << '\n';
    } else {
        cout << "No analytics available because the library is empty.\n";
    }
}

// ============================================================================
// AUTOMATED TEST FRAMEWORK
// ============================================================================

class TestRunner {
private:
    int passed = 0;
    int failed = 0;

    void check(const string& testName, bool condition) {
        if (condition) {
            cout << "[PASS] " << testName << '\n';
            ++passed;
        } else {
            cout << "[FAIL] " << testName << '\n';
            ++failed;
        }
    }

public:
    void runAll() {
        passed = 0;
        failed = 0;

        printHeader("AUTOMATED TEST SUITE");

        testAddBook();
        testDuplicateBook();
        testInvalidBook();
        testSearch();
        testIssueAndReturn();
        testRemoveBook();
        testSorting();
        testReporting();
        testEmptyLibrary();

        printLine();

        cout << "Tests passed : " << passed << '\n';
        cout << "Tests failed : " << failed << '\n';

        if (failed == 0) {
            cout << "TEST RESULT  : ALL TESTS PASSED\n";
        } else {
            cout << "TEST RESULT  : SOME TESTS FAILED\n";
        }

        printLine();
    }

private:
    static LibraryManager createTestLibrary() {
        return LibraryManager({
            {105, "Computer Networks", "Andrew Tanenbaum", 2010, false},
            {101, "Clean Code", "Robert Martin", 2008, true},
            {110, "Operating Systems", "Silberschatz", 2018, false},
            {103, "C++ Primer", "Stanley Lippman", 2012, false},
            {107, "Algorithms", "Robert Sedgewick", 2011, true}
        });
    }

    void testAddBook() {
        LibraryManager library;

        Book book{
            1,
            "Test Book",
            "Test Author",
            2024,
            false
        };

        OperationResult result = library.addBook(book);

        check("Add valid book",
              result.ok() &&
              library.size() == 1 &&
              library.findById(1) != nullptr);
    }

    void testDuplicateBook() {
        LibraryManager library;

        Book book{
            1,
            "Test Book",
            "Test Author",
            2024,
            false
        };

        library.addBook(book);
        OperationResult result = library.addBook(book);

        check("Reject duplicate ID",
              result.status == OperationStatus::AlreadyExists &&
              library.size() == 1);
    }

    void testInvalidBook() {
        LibraryManager library;

        Book invalidBook{
            -1,
            "",
            "",
            500,
            false
        };

        OperationResult result = library.addBook(invalidBook);

        check("Reject invalid book data",
              result.status == OperationStatus::InvalidInput &&
              library.size() == 0);
    }

    void testSearch() {
        LibraryManager library = createTestLibrary();

        const Book* byId = library.findById(103);
        vector<Book> byTitle = library.searchByTitle("clean");
        vector<Book> byAuthor = library.searchByAuthor("tanenen");

        check("Search existing ID",
              byId != nullptr && byId->id == 103);

        check("Case-insensitive title search",
              byTitle.size() == 1 &&
              byTitle[0].title == "Clean Code");

        // Deliberately misspelled keyword should return no result.
        check("Invalid/missing author search",
              byAuthor.empty());
    }

    void testIssueAndReturn() {
        LibraryManager library = createTestLibrary();

        OperationResult issue = library.issueBook(105);
        const Book* issuedBook = library.findById(105);
        bool wasIssued = issuedBook != nullptr && issuedBook->issued;

        OperationResult issueAgain = library.issueBook(105);

        OperationResult returnBook = library.returnBook(105);
        const Book* returnedBook = library.findById(105);

        OperationResult returnAgain = library.returnBook(105);

        check("Issue available book",
              issue.ok() &&
              wasIssued);

        check("Reject issuing already issued book",
              issueAgain.status == OperationStatus::OperationNotAllowed);

        check("Return issued book",
              returnBook.ok() &&
              returnedBook != nullptr &&
              !returnedBook->issued);

        check("Reject returning available book",
              returnAgain.status == OperationStatus::OperationNotAllowed);
    }

    void testRemoveBook() {
        LibraryManager library = createTestLibrary();

        OperationResult removeAvailable = library.removeBook(105);

        OperationResult removeMissing = library.removeBook(999);

        OperationResult removeIssued = library.removeBook(101);

        check("Remove available book",
              removeAvailable.ok() &&
              library.findById(105) == nullptr);

        check("Reject removing missing book",
              removeMissing.status == OperationStatus::NotFound);

        check("Reject removing issued book",
              removeIssued.status == OperationStatus::OperationNotAllowed);
    }

    void testSorting() {
        LibraryManager library = createTestLibrary();

        library.sortByIdAscending();

        bool idSorted = true;
        const auto& booksById = library.getBooks();

        for (size_t i = 1; i < booksById.size(); ++i) {
            if (booksById[i - 1].id > booksById[i].id) {
                idSorted = false;
                break;
            }
        }

        library.sortByYearDescending();

        bool yearSorted = true;
        const auto& booksByYear = library.getBooks();

        for (size_t i = 1; i < booksByYear.size(); ++i) {
            if (booksByYear[i - 1].publicationYear <
                booksByYear[i].publicationYear) {
                yearSorted = false;
                break;
            }
        }

        check("Sort books by ID ascending", idSorted);
        check("Sort books by year descending", yearSorted);
    }

    void testReporting() {
        LibraryManager library = createTestLibrary();

        LibraryReport report = generateReport(library);

        check("Report total book count",
              report.totalBooks == 5);

        check("Report available/issued counts",
              report.availableBooks == 3 &&
              report.issuedBooks == 2);

        check("Report oldest/newest year",
              report.oldestPublicationYear == 2008 &&
              report.newestPublicationYear == 2018);

        check("Report percentages",
              report.availabilityPercentage == 60.0 &&
              report.issuePercentage == 40.0);
    }

    void testEmptyLibrary() {
        LibraryManager library;

        LibraryReport report = generateReport(library);

        OperationResult issue = library.issueBook(1);
        OperationResult remove = library.removeBook(1);

        check("Empty library report",
              report.totalBooks == 0 &&
              report.availableBooks == 0 &&
              report.issuedBooks == 0);

        check("Empty library issue handling",
              issue.status == OperationStatus::NotFound);

        check("Empty library remove handling",
              remove.status == OperationStatus::NotFound);
    }
};

// ============================================================================
// CAPSTONE DOCUMENTATION
// ============================================================================

void showArchitecture() {
    printHeader("ARCHITECTURE AND DESIGN TRADE-OFFS");

    cout << "1. Domain Model\n";
    cout << "   Book stores only book-related data.\n\n";

    cout << "2. Business Logic\n";
    cout << "   LibraryManager owns library operations such as add, remove,\n";
    cout << "   search, issue, return, and sorting.\n\n";

    cout << "3. Reporting Layer\n";
    cout << "   LibraryReport and generateReport() calculate useful analytics\n";
    cout << "   without mixing reporting logic into the menu code.\n\n";

    cout << "4. User Interface\n";
    cout << "   Menu/UI functions handle input/output only and call the\n";
    cout << "   LibraryManager instead of implementing business rules.\n\n";

    cout << "5. Error Handling\n";
    cout << "   Operations return OperationResult with explicit status values.\n";
    cout << "   Invalid IDs, duplicates, missing records, and invalid state\n";
    cout << "   transitions are handled without crashing the program.\n\n";

    cout << "6. Memory Safety\n";
    cout << "   std::vector, std::string and automatic objects are used.\n";
    cout << "   No raw memory allocation is required.\n\n";

    cout << "7. Trade-off\n";
    cout << "   The current version keeps data in memory, making it simple,\n";
    cout << "   portable, and easy to demonstrate. A production system could\n";
    cout << "   replace the storage layer with a database or file repository.\n";
}

void showBuildInstructions() {
    printHeader("BUILD / RELEASE CHECKLIST");

    cout << "Compiler requirement: C++17 or newer\n\n";

    cout << "GCC / MinGW:\n";
    cout << "g++ -std=c++17 -Wall -Wextra -pedantic "
            "library_capstone.cpp -o library_capstone\n\n";

    cout << "Run on Linux/macOS:\n";
    cout << "./library_capstone\n\n";

    cout << "Run on Windows:\n";
    cout << "library_capstone.exe\n\n";

    cout << "Recommended verification:\n";
    cout << "1. Compile with warnings enabled.\n";
    cout << "2. Confirm all automated tests pass.\n";
    cout << "3. Test invalid menu input.\n";
    cout << "4. Test duplicate IDs.\n";
    cout << "5. Test issue/return state transitions.\n";
    cout << "6. Test removing an issued book.\n";
    cout << "7. Demonstrate the analytics report.\n";
    cout << "8. Repeat the build on another compiler/environment.\n";
}

// ============================================================================
// SAMPLE DATA
// ============================================================================

LibraryManager createApplicationLibrary() {
    return LibraryManager({
        {101, "Clean Code", "Robert C. Martin", 2008, true},
        {102, "Database System Concepts", "Abraham Silberschatz", 2019, false},
        {103, "C++ Primer", "Stanley Lippman", 2012, false},
        {104, "Artificial Intelligence", "Stuart Russell", 2021, false},
        {105, "Computer Networks", "Andrew Tanenbaum", 2010, false},
        {106, "Software Engineering", "Ian Sommerville", 2016, false},
        {107, "Algorithms", "Robert Sedgewick", 2011, true},
        {108, "Data Structures", "Mark Allen Weiss", 2013, false},
        {109, "Computer Architecture", "David Patterson", 2017, false},
        {110, "Operating Systems", "Abraham Silberschatz", 2018, true}
    });
}

// ============================================================================
// MENU
// ============================================================================

void displayMenu() {
    cout << '\n';
    printLine();
    cout << "              LIBRARY MANAGER\n";
    printLine();

    cout << "  1. Display all books\n";
    cout << "  2. Add a book\n";
    cout << "  3. Remove a book\n";
    cout << "  4. Search books\n";
    cout << "  5. Sort books\n";
    cout << "  6. Issue a book\n";
    cout << "  7. Return a book\n";
    cout << "  8. Library analytics report\n";
    cout << "  9. Run automated tests\n";
    cout << " 10. Architecture and design\n";
    cout << " 11. Build/release instructions\n";
    cout << "  0. Exit\n";

    printLine();
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    cout << "\n";
    printLine();
    cout << "       C++ APPLICATION CAPSTONE\n";
    cout << "       RELEASE-READY LIBRARY MANAGER\n";
    printLine();

    // Run tests before interactive use.
    TestRunner testRunner;
    testRunner.runAll();

    LibraryManager library = createApplicationLibrary();

    while (true) {
        displayMenu();

        int choice = readInteger("Enter your choice: ");

        switch (choice) {
            case 1:
                printHeader("ALL LIBRARY BOOKS");
                printBookTable(library.getBooks());
                break;

            case 2:
                addBookUI(library);
                break;

            case 3:
                removeBookUI(library);
                break;

            case 4:
                searchUI(library);
                break;

            case 5:
                sortingUI(library);
                break;

            case 6:
                issueBookUI(library);
                break;

            case 7:
                returnBookUI(library);
                break;

            case 8:
                reportUI(library);
                break;

            case 9:
                testRunner.runAll();
                break;

            case 10:
                showArchitecture();
                break;

            case 11:
                showBuildInstructions();
                break;

            case 0:
                cout << "\nApplication closed successfully.\n";
                return 0;

            default:
                cout << "[ERROR] Invalid menu choice. Please try again.\n";
                break;
        }
    }
}
