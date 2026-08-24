//Reliable Command-Line Utility — Expense Tracker

#include <iostream>
#include <iomanip>
#include <limits>
#include <map>
#include <string>
#include <vector>

// Data model
// One expense entry: what it was for, how much it cost, and which
// category it belongs to (e.g. "Food", "Transport", "Rent").
struct Expense {
    std::string description;
    std::string category;
    double amount;
};

// Input helpers (defensive parsing — never trust raw std::cin)
// Discards everything left on the current input line. Used after a
// failed or partial read so leftover characters don't corrupt the next prompt.

void clearInputLine() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// True once the input stream has genuinely run out (piped input ended,
// or the user sent EOF with Ctrl+D / Ctrl+Z). This is distinct from a
// single bad token: on real EOF no amount of re-prompting will ever
// produce new input, so callers must stop looping and exit instead of spinning forever.
bool inputExhausted() {
    return std::cin.eof();
}

// Prompts until the user enters a non-empty line of text, or the
// stream runs out (in which case an empty string is returned and the
// caller's std::cin.eof() check — surfaced via inputExhausted() at the
// call site in main() — ends the program).
std::string promptString(const std::string& prompt) {
    std::string value;
    while (true) {
        std::cout << prompt;
        if (!std::getline(std::cin, value)) {
            return ""; // stream ended; let the caller notice via inputExhausted()
        }

        // Trim leading/trailing whitespace.
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

// Prompts until the user enters a positive, finite number, or the
// stream runs out (returns 0.0; caller checks inputExhausted()).
double promptPositiveDouble(const std::string& prompt) {
    double value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;

        if (std::cin.fail()) {
            if (inputExhausted()) {
                return 0.0; // real EOF — stop retrying, caller will exit
            }
            std::cout << "  That doesn't look like a number. Please try again.\n";
            clearInputLine();
            continue;
        }
        clearInputLine(); // discard trailing newline / extra tokens

        if (value <= 0) {
            std::cout << "  Amount must be greater than 0. Please try again.\n";
            continue;
        }
        return value;
    }
}

// Prompts until the user enters an integer within [minValue, maxValue],
// or the stream runs out (returns minValue - 1 as a sentinel; caller
// checks inputExhausted() rather than trusting that sentinel).
int promptMenuChoice(int minValue, int maxValue) {
    int choice;
    while (true) {
        std::cout << "\nEnter your choice (" << minValue << "-" << maxValue << "): ";
        std::cin >> choice;

        if (std::cin.fail()) {
            if (inputExhausted()) {
                return minValue - 1; // real EOF — stop retrying, caller will exit
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

// Core actions (each does one job, so main() stays readable)

void printMenu() {
    std::cout << "\n=====\n";
    std::cout << "        EXPENSE TRACKER\n";
    std::cout << "=====\n";
    std::cout << "1. Add expense\n";
    std::cout << "2. List all expenses\n";
    std::cout << "3. Show total for a category\n";
    std::cout << "4. Show overall total\n";
    std::cout << "5. Exit\n";
}

void addExpense(std::vector<Expense>& expenses) {
    std::cout << "\n-- Add Expense --\n";
    std::string description = promptString("Description: ");
    if (inputExhausted()) return;
    std::string category = promptString("Category: ");
    if (inputExhausted()) return;
    double amount = promptPositiveDouble("Amount: ");
    if (inputExhausted()) return;

    expenses.push_back(Expense{description, category, amount});
    std::cout << "  Added: " << description << " ($"
              << std::fixed << std::setprecision(2) << amount
              << ", " << category << ")\n";
}

void listExpenses(const std::vector<Expense>& expenses) {
    std::cout << "\n-- All Expenses --\n";
    if (expenses.empty()) {
        std::cout << "  No expenses recorded yet.\n";
        return;
    }

    std::cout << std::left
              << std::setw(4)  << "#"
              << std::setw(28) << "Description"
              << std::setw(16) << "Category"
              << std::right << std::setw(10) << "Amount" << "\n";
    std::cout << std::string(58, '-') << "\n";

    for (size_t i = 0; i < expenses.size(); ++i) {
        const Expense& e = expenses[i];
        std::cout << std::left
                  << std::setw(4)  << (i + 1)
                  << std::setw(28) << e.description
                  << std::setw(16) << e.category
                  << std::right << std::fixed << std::setprecision(2)
                  << std::setw(10) << e.amount << "\n";
    }
}

void showCategoryTotal(const std::vector<Expense>& expenses) {
    if (expenses.empty()) {
        std::cout << "\n  No expenses recorded yet.\n";
        return;
    }

    std::string category = promptString("\nWhich category? ");
    if (inputExhausted()) return;

    double total = 0.0;
    int count = 0;
    for (const Expense& e : expenses) {
        // Case-insensitive-ish match kept simple: exact match on trimmed text.
        if (e.category == category) {
            total += e.amount;
            ++count;
        }
    }

    if (count == 0) {
        std::cout << "  No expenses found in category \"" << category << "\".\n";
    } else {
        std::cout << "  " << count << " expense(s) in \"" << category << "\" total: $"
                  << std::fixed << std::setprecision(2) << total << "\n";
    }
}

void showOverallTotal(const std::vector<Expense>& expenses) {
    if (expenses.empty()) {
        std::cout << "\n  No expenses recorded yet.\n";
        return;
    }

    double total = 0.0;
    std::map<std::string, double> byCategory; // sorted breakdown for readability
    for (const Expense& e : expenses) {
        total += e.amount;
        byCategory[e.category] += e.amount;
    }

    std::cout << "\n-- Overall Total --\n";
    for (const auto& entry : byCategory) {
        const std::string& category = entry.first;
        double sum = entry.second;
        std::cout << "  " << std::left << std::setw(20) << category
                  << "$" << std::fixed << std::setprecision(2) << sum << "\n";
    }
    std::cout << std::string(30, '-') << "\n";
    std::cout << "  " << std::left << std::setw(20) << "TOTAL"
              << "$" << std::fixed << std::setprecision(2) << total << "\n";
}

// Entry point — wires the menu to the actions above.

int main() {
    std::vector<Expense> expenses;
    bool running = true;

    std::cout << "Welcome to Expense Tracker!\n";

    while (running) {
        printMenu();
        int choice = promptMenuChoice(1, 5);
        if (inputExhausted()) {
            std::cout << "\nInput ended — exiting.\n";
            break;
        }

        switch (choice) {
            case 1: addExpense(expenses);        break;
            case 2: listExpenses(expenses);       break;
            case 3: showCategoryTotal(expenses);  break;
            case 4: showOverallTotal(expenses);   break;
            case 5:
                std::cout << "\nGoodbye!\n";
                running = false;
                break;
            default:
                // Unreachable: promptMenuChoice already validates the range.
                break;
        }
    }

    return 0;
}
