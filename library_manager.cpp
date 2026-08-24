/*Object-Oriented Library Manager-A menu-driven CLI that models a small library using three
 classes with clear, single responsibilities.*/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

// Small manual string-splitting helpers (no <sstream> tricks,
// just plain character-by-character loops so the logic is easy
// to follow and does not depend on any library edge cases).

std::vector<std::string> splitOn(const std::string& line, char delimiter) {
    std::vector<std::string> parts;
    std::string current;
    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] == delimiter) {
            parts.push_back(current);
            current.clear();
        } else {
            current += line[i];
        }
    }
    parts.push_back(current); // last field (or the whole string if no delimiter found)
    return parts;
}

std::string joinWith(const std::vector<std::string>& items, char delimiter) {
    std::string result;
    for (size_t i = 0; i < items.size(); ++i) {
        result += items[i];
        if (i + 1 < items.size()) {
            result += delimiter;
        }
    }
    return result;
}

// Case-insensitive "does haystack contain needle" check, used by search.
bool containsIgnoreCase(const std::string& haystack, const std::string& needle) {
    std::string h = haystack;
    std::string n = needle;
    for (size_t i = 0; i < h.size(); ++i) h[i] = static_cast<char>(tolower(h[i]));
    for (size_t i = 0; i < n.size(); ++i) n[i] = static_cast<char>(tolower(n[i]));
    return h.find(n) != std::string::npos;
}

// Book

class Book {
private:
    std::string id;
    std::string title;
    std::string author;
    int totalCopies;
    int availableCopies;

public:
    Book() : totalCopies(0), availableCopies(0) {}

    Book(const std::string& id_, const std::string& title_,
         const std::string& author_, int totalCopies_, int availableCopies_) {
        id = id_;
        title = title_;
        author = author_;
        totalCopies = totalCopies_;
        availableCopies = availableCopies_;
    }

    // -- getters (read-only access from outside the class) --
    std::string getId() const { return id; }
    std::string getTitle() const { return title; }
    std::string getAuthor() const { return author; }
    int getTotalCopies() const { return totalCopies; }
    int getAvailableCopies() const { return availableCopies; }
    bool isAvailable() const { return availableCopies > 0; }

    // -- behavior (the only way outside code can change copy counts) --
    // Returns false and changes nothing if no copy is free to give out.
    bool issueCopy() {
        if (availableCopies <= 0) {
            return false;
        }
        availableCopies = availableCopies - 1;
        return true;
    }

    // Returns false and changes nothing if every copy is already
    // accounted for (defensive guard against a bad/duplicate return).
    bool returnCopy() {
        if (availableCopies >= totalCopies) {
            return false;
        }
        availableCopies = availableCopies + 1;
        return true;
    }

    std::string serialize() const {
        return id + "|" + title + "|" + author + "|"
             + std::to_string(totalCopies) + "|" + std::to_string(availableCopies);
    }

    static Book deserialize(const std::string& line) {
        std::vector<std::string> f = splitOn(line, '|');
        // f[0]=id f[1]=title f[2]=author f[3]=totalCopies f[4]=availableCopies
        int total = std::stoi(f[3]);
        int available = std::stoi(f[4]);
        return Book(f[0], f[1], f[2], total, available);
    }
};

// Member

class Member {
private:
    std::string id;
    std::string name;
    std::vector<std::string> borrowedBookIds;

public:
    Member() {}

    Member(const std::string& id_, const std::string& name_) {
        id = id_;
        name = name_;
    }

    std::string getId() const { return id; }
    std::string getName() const { return name; }
    int borrowedCount() const { return static_cast<int>(borrowedBookIds.size()); }
    const std::vector<std::string>& getBorrowedBookIds() const { return borrowedBookIds; }

    bool hasBorrowed(const std::string& bookId) const {
        for (size_t i = 0; i < borrowedBookIds.size(); ++i) {
            if (borrowedBookIds[i] == bookId) {
                return true;
            }
        }
        return false;
    }

    void addBorrowedBook(const std::string& bookId) {
        borrowedBookIds.push_back(bookId);
    }

    bool removeBorrowedBook(const std::string& bookId) {
        for (size_t i = 0; i < borrowedBookIds.size(); ++i) {
            if (borrowedBookIds[i] == bookId) {
                borrowedBookIds.erase(borrowedBookIds.begin() + static_cast<long>(i));
                return true;
            }
        }
        return false;
    }

    std::string serialize() const {
        return id + "|" + name + "|" + joinWith(borrowedBookIds, ',');
    }

    static Member deserialize(const std::string& line) {
        std::vector<std::string> f = splitOn(line, '|');
        // f[0]=id f[1]=name f[2]=comma-separated borrowed ids (may be empty)
        Member m(f[0], f[1]);
        if (f.size() > 2 && !f[2].empty()) {
            std::vector<std::string> ids = splitOn(f[2], ',');
            for (size_t i = 0; i < ids.size(); ++i) {
                m.addBorrowedBook(ids[i]);
            }
        }
        return m;
    }
};

// Loan

class Loan {
private:
    int loanId;
    std::string bookId;
    std::string memberId;
    bool active; // true = currently out, false = returned

public:
    Loan() : loanId(0), active(false) {}

    Loan(int loanId_, const std::string& bookId_, const std::string& memberId_, bool active_) {
        loanId = loanId_;
        bookId = bookId_;
        memberId = memberId_;
        active = active_;
    }

    int getLoanId() const { return loanId; }
    std::string getBookId() const { return bookId; }
    std::string getMemberId() const { return memberId; }
    bool isActive() const { return active; }
    void markReturned() { active = false; }

    std::string serialize() const {
        return std::to_string(loanId) + "|" + bookId + "|" + memberId + "|"
             + (active ? "ACTIVE" : "RETURNED");
    }

    static Loan deserialize(const std::string& line) {
        std::vector<std::string> f = splitOn(line, '|');
        // f[0]=loanId f[1]=bookId f[2]=memberId f[3]=ACTIVE/RETURNED
        int id = std::stoi(f[0]);
        bool isActive = (f[3] == "ACTIVE");
        return Loan(id, f[1], f[2], isActive);
    }
};

// LibraryManager
//   Owns every collection, every ID lookup, every file read/write,
//   and every rule about what counts as a valid operation
//   (duplicate IDs, missing records, unavailable books, etc.).

class LibraryManager {
private:
    std::vector<Book> books;
    std::vector<Member> members;
    std::vector<Loan> loans;
    int nextLoanId;

    std::string booksFile;
    std::string membersFile;
    std::string loansFile;

    // -- private lookup helpers: linear search, -1 if not found --
    int findBookIndex(const std::string& id) const {
        for (size_t i = 0; i < books.size(); ++i) {
            if (books[i].getId() == id) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    int findMemberIndex(const std::string& id) const {
        for (size_t i = 0; i < members.size(); ++i) {
            if (members[i].getId() == id) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    int findActiveLoanIndex(const std::string& bookId, const std::string& memberId) const {
        for (size_t i = 0; i < loans.size(); ++i) {
            if (loans[i].getBookId() == bookId
                && loans[i].getMemberId() == memberId
                && loans[i].isActive()) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    void recomputeNextLoanId() {
        int highest = 0;
        for (size_t i = 0; i < loans.size(); ++i) {
            if (loans[i].getLoanId() > highest) {
                highest = loans[i].getLoanId();
            }
        }
        nextLoanId = highest + 1;
    }

public:
    LibraryManager(const std::string& booksFile_, const std::string& membersFile_,
                   const std::string& loansFile_) {
        booksFile = booksFile_;
        membersFile = membersFile_;
        loansFile = loansFile_;
        nextLoanId = 1;
    }

    //persistence---

    void loadAll() {
        books.clear();
        members.clear();
        loans.clear();

        std::ifstream bin(booksFile.c_str());
        std::string line;
        while (std::getline(bin, line)) {
            if (!line.empty()) {
                books.push_back(Book::deserialize(line));
            }
        }
        bin.close();

        std::ifstream min(membersFile.c_str());
        while (std::getline(min, line)) {
            if (!line.empty()) {
                members.push_back(Member::deserialize(line));
            }
        }
        min.close();

        std::ifstream lin(loansFile.c_str());
        while (std::getline(lin, line)) {
            if (!line.empty()) {
                loans.push_back(Loan::deserialize(line));
            }
        }
        lin.close();

        recomputeNextLoanId();
    }

    void saveAll() const {
        std::ofstream bout(booksFile.c_str());
        for (size_t i = 0; i < books.size(); ++i) {
            bout << books[i].serialize() << "\n";
        }
        bout.close();

        std::ofstream mout(membersFile.c_str());
        for (size_t i = 0; i < members.size(); ++i) {
            mout << members[i].serialize() << "\n";
        }
        mout.close();

        std::ofstream lout(loansFile.c_str());
        for (size_t i = 0; i < loans.size(); ++i) {
            lout << loans[i].serialize() << "\n";
        }
        lout.close();
    }

    bool hasAnyBooks() const { return !books.empty(); }

    // Populates a first-run library with sample records so the
    // program has something to demonstrate before you add your own.
    void seedSampleData() {
        addBook("B001", "The Hobbit", "J.R.R. Tolkien", 3);
        addBook("B002", "Clean Code", "Robert C. Martin", 2);
        addBook("B003", "Dune", "Frank Herbert", 1);
        addBook("B004", "The Pragmatic Programmer", "Andrew Hunt", 2);

        addMember("M001", "Asha Verma");
        addMember("M002", "Liam Chen");

        // Demonstrate a book already out on loan.
        issueBook("B003", "M001");
    }

    //book operations ---

    bool addBook(const std::string& id, const std::string& title,
                 const std::string& author, int totalCopies) {
        if (findBookIndex(id) != -1) {
            std::cout << "  A book with ID " << id << " already exists.\n";
            return false;
        }
        books.push_back(Book(id, title, author, totalCopies, totalCopies));
        std::cout << "  Added book: " << title << " (" << totalCopies << " cop"
                  << (totalCopies == 1 ? "y" : "ies") << ").\n";
        return true;
    }

    void listBooks() const {
        std::cout << "\n-- All Books --\n";
        if (books.empty()) {
            std::cout << "  No books in the library yet.\n";
            return;
        }
        std::cout << std::left
                  << std::setw(8)  << "ID"
                  << std::setw(28) << "Title"
                  << std::setw(20) << "Author"
                  << std::right << std::setw(12) << "Available\n";
        std::cout << std::string(68, '-') << "\n";
        for (size_t i = 0; i < books.size(); ++i) {
            const Book& b = books[i];
            std::string availText = std::to_string(b.getAvailableCopies())
                                   + "/" + std::to_string(b.getTotalCopies());
            std::cout << std::left
                      << std::setw(8)  << b.getId()
                      << std::setw(28) << b.getTitle()
                      << std::setw(20) << b.getAuthor()
                      << std::right << std::setw(12) << availText << "\n";
        }
    }

    void searchBooks(const std::string& keyword) const {
        std::cout << "\n-- Search results for \"" << keyword << "\" --\n";
        bool found = false;
        for (size_t i = 0; i < books.size(); ++i) {
            const Book& b = books[i];
            if (containsIgnoreCase(b.getTitle(), keyword)
                || containsIgnoreCase(b.getAuthor(), keyword)) {
                std::cout << "  [" << b.getId() << "] " << b.getTitle()
                          << " by " << b.getAuthor()
                          << " (" << b.getAvailableCopies() << "/" << b.getTotalCopies()
                          << " available)\n";
                found = true;
            }
        }
        if (!found) {
            std::cout << "  No books matched that search.\n";
        }
    }

    //member operations ---

    bool addMember(const std::string& id, const std::string& name) {
        if (findMemberIndex(id) != -1) {
            std::cout << "  A member with ID " << id << " already exists.\n";
            return false;
        }
        members.push_back(Member(id, name));
        std::cout << "  Added member: " << name << " (ID " << id << ").\n";
        return true;
    }

    void listMembers() const {
        std::cout << "\n-- All Members --\n";
        if (members.empty()) {
            std::cout << "  No members registered yet.\n";
            return;
        }
        for (size_t i = 0; i < members.size(); ++i) {
            const Member& m = members[i];
            std::cout << "  [" << m.getId() << "] " << m.getName()
                      << " - " << m.borrowedCount() << " book(s) borrowed";
            if (m.borrowedCount() > 0) {
                std::cout << ": " << joinWith(m.getBorrowedBookIds(), ',');
            }
            std::cout << "\n";
        }
    }

    //loan operations ---

    void issueBook(const std::string& bookId, const std::string& memberId) {
        int bi = findBookIndex(bookId);
        if (bi == -1) {
            std::cout << "  No book found with ID " << bookId << ".\n";
            return;
        }
        int mi = findMemberIndex(memberId);
        if (mi == -1) {
            std::cout << "  No member found with ID " << memberId << ".\n";
            return;
        }
        if (members[mi].hasBorrowed(bookId)) {
            std::cout << "  " << members[mi].getName()
                      << " has already borrowed this book.\n";
            return;
        }
        if (!books[bi].issueCopy()) {
            std::cout << "  \"" << books[bi].getTitle()
                      << "\" has no available copies right now.\n";
            return;
        }

        members[mi].addBorrowedBook(bookId);
        loans.push_back(Loan(nextLoanId, bookId, memberId, true));
        std::cout << "  Issued \"" << books[bi].getTitle() << "\" to "
                  << members[mi].getName() << " (Loan #" << nextLoanId << ").\n";
        nextLoanId = nextLoanId + 1;
    }

    void returnBook(const std::string& bookId, const std::string& memberId) {
        int bi = findBookIndex(bookId);
        if (bi == -1) {
            std::cout << "  No book found with ID " << bookId << ".\n";
            return;
        }
        int mi = findMemberIndex(memberId);
        if (mi == -1) {
            std::cout << "  No member found with ID " << memberId << ".\n";
            return;
        }
        if (!members[mi].hasBorrowed(bookId)) {
            std::cout << "  " << members[mi].getName()
                      << " has not borrowed this book, so it can't be returned.\n";
            return;
        }

        int li = findActiveLoanIndex(bookId, memberId);
        if (li != -1) {
            loans[li].markReturned();
        }
        members[mi].removeBorrowedBook(bookId);
        books[bi].returnCopy();
        std::cout << "  Returned \"" << books[bi].getTitle() << "\" from "
                  << members[mi].getName() << ".\n";
    }
};

// Defensive input helpers
//   Same pattern as before: retry on bad input, and stop cleanly
//   (rather than looping forever) if the input stream genuinely
//   runs out - piped input ending, or Ctrl+D / Ctrl+Z.

void clearInputLine() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

bool inputExhausted() {
    return std::cin.eof();
}

std::string promptString(const std::string& prompt) {
    std::string value;
    while (true) {
        std::cout << prompt;
        if (!std::getline(std::cin, value)) {
            return "";
        }
        size_t start = value.find_first_not_of(" \t");
        size_t end = value.find_last_not_of(" \t");
        if (start == std::string::npos) {
            std::cout << "  Input can't be empty. Please try again.\n";
            continue;
        }
        value = value.substr(start, end - start + 1);
        return value;
    }
}

int promptPositiveInt(const std::string& prompt) {
    int value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;
        if (std::cin.fail()) {
            if (inputExhausted()) {
                return 0;
            }
            std::cout << "  That doesn't look like a whole number. Please try again.\n";
            clearInputLine();
            continue;
        }
        clearInputLine();
        if (value <= 0) {
            std::cout << "  Please enter a number greater than 0.\n";
            continue;
        }
        return value;
    }
}

int promptMenuChoice(int minValue, int maxValue) {
    int choice;
    while (true) {
        std::cout << "\nEnter your choice (" << minValue << "-" << maxValue << "): ";
        std::cin >> choice;
        if (std::cin.fail()) {
            if (inputExhausted()) {
                return minValue - 1;
            }
            std::cout << "  Invalid input. Please enter a number.\n";
            clearInputLine();
            continue;
        }
        clearInputLine();
        if (choice < minValue || choice > maxValue) {
            std::cout << "  Please choose a number between "
                      << minValue << " and " << maxValue << ".\n";
            continue;
        }
        return choice;
    }
}

void printMenu() {
    std::cout << "\n=====\n";
    std::cout << "        LIBRARY MANAGER\n";
    std::cout << "=====\n";
    std::cout << "1. Add book\n";
    std::cout << "2. Add member\n";
    std::cout << "3. Issue book\n";
    std::cout << "4. Return book\n";
    std::cout << "5. Search books (title/author)\n";
    std::cout << "6. List all books\n";
    std::cout << "7. List all members\n";
    std::cout << "8. Save and exit\n";
}

// main
int main() {
    LibraryManager library("books.txt", "members.txt", "loans.txt");
    library.loadAll();

    if (!library.hasAnyBooks()) {
        std::cout << "No existing data found - loading sample records.\n";
        library.seedSampleData();
        library.saveAll();
    } else {
        std::cout << "Loaded existing library data from disk.\n";
    }

    bool running = true;
    while (running) {
        printMenu();
        int choice = promptMenuChoice(1, 8);
        if (inputExhausted()) {
            std::cout << "\nInput ended - saving and exiting.\n";
            library.saveAll();
            break;
        }

        if (choice == 1) {
            std::cout << "\n-- Add Book --\n";
            std::string id = promptString("Book ID: ");
            if (inputExhausted()) break;
            std::string title = promptString("Title: ");
            if (inputExhausted()) break;
            std::string author = promptString("Author: ");
            if (inputExhausted()) break;
            int copies = promptPositiveInt("Number of copies: ");
            if (inputExhausted()) break;
            library.addBook(id, title, author, copies);
            library.saveAll();

        } else if (choice == 2) {
            std::cout << "\n-- Add Member --\n";
            std::string id = promptString("Member ID: ");
            if (inputExhausted()) break;
            std::string name = promptString("Name: ");
            if (inputExhausted()) break;
            library.addMember(id, name);
            library.saveAll();

        } else if (choice == 3) {
            std::cout << "\n-- Issue Book --\n";
            std::string bookId = promptString("Book ID: ");
            if (inputExhausted()) break;
            std::string memberId = promptString("Member ID: ");
            if (inputExhausted()) break;
            library.issueBook(bookId, memberId);
            library.saveAll();

        } else if (choice == 4) {
            std::cout << "\n-- Return Book --\n";
            std::string bookId = promptString("Book ID: ");
            if (inputExhausted()) break;
            std::string memberId = promptString("Member ID: ");
            if (inputExhausted()) break;
            library.returnBook(bookId, memberId);
            library.saveAll();

        } else if (choice == 5) {
            std::string keyword = promptString("\nSearch keyword: ");
            if (inputExhausted()) break;
            library.searchBooks(keyword);

        } else if (choice == 6) {
            library.listBooks();

        } else if (choice == 7) {
            library.listMembers();

        } else if (choice == 8) {
            library.saveAll();
            std::cout << "\nData saved. Goodbye!\n";
            running = false;
        }
    }

    return 0;
}
