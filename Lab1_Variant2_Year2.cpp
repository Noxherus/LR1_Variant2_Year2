#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

// Базовый уровень: композиция Car и Wheel.
class Wheel {
private:
    int diameter;
    string type;

public:
    Wheel(int d, const string& t) : diameter(d), type(t) {
        if (diameter <= 0 || (type != "Winter" && type != "Summer"))
            throw invalid_argument("Invalid wheel parameters");
        cout << "Wheel created: " << diameter << " inches, " << type << '\n';
    }

    ~Wheel() { cout << "Wheel destroyed\n"; }

    void print() const {
        cout << diameter << " inches, " << type << '\n';
    }
};

class Car {
private:
    string model;
    // Колёса являются частью автомобиля и уничтожаются вместе с ним.
    array<Wheel, 4> wheels;

public:
    Car(const string& m, int d, const string& t)
        : model(m), wheels{Wheel(d, t), Wheel(d, t), Wheel(d, t), Wheel(d, t)} {
        cout << "Car created: " << model << '\n';
    }

    ~Car() { cout << "Car destroyed: " << model << '\n'; }

    void print() const {
        cout << "Car: " << model << '\n';
        for (size_t i = 0; i < wheels.size(); ++i) {
            cout << "  Wheel " << i + 1 << ": ";
            wheels[i].print();
        }
    }
};

// Средний уровень: агрегация Team и Player.
class Player {
private:
    string name;
    int number;
    string position;
    static int count;

public:
    Player(const string& n, int num, const string& p)
        : name(n), number(num), position(p) {
        if (name.empty() || number <= 0 || position.empty())
            throw invalid_argument("Invalid player data");
        ++count;
        cout << "Player created: " << name << '\n';
    }

    // Запрещаем копирование, чтобы не нарушать учёт объектов.
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;
    ~Player() { --count; cout << "Player destroyed: " << name << '\n'; }

    static int getCount() { return count; }
    void print() const { cout << name << ", #" << number << ", " << position << '\n'; }
};

int Player::count = 0;

class Team {
private:
    string name;
    // Команда не владеет игроками. Игроки должны жить дольше команды.
    vector<Player*> players;

public:
    explicit Team(const string& n) : name(n) { cout << "Team created: " << name << '\n'; }
    ~Team() { cout << "Team destroyed: " << name << '\n'; }

    bool addPlayer(Player* player) {
        if (player == nullptr || find(players.begin(), players.end(), player) != players.end())
            return false;
        players.push_back(player);
        return true;
    }

    void print() const {
        cout << "Team: " << name << '\n';
        for (const auto* player : players) player->print();
    }
};

// Продвинутый уровень: система управления библиотекой.
class Book {
private:
    string title;
    string author;
    int year;
    static int count;
    static int totalCreated;

public:
    Book(const string& t, const string& a, int y) : title(t), author(a), year(y) {
        if (title.empty() || author.empty() || year <= 0)
            throw invalid_argument("Invalid book data");
        ++count;
        ++totalCreated;
        cout << "Book created: " << title << '\n';
    }

    Book(const Book&) = delete;
    Book& operator=(const Book&) = delete;
    ~Book() { --count; cout << "Book destroyed: " << title << '\n'; }

    const string& getTitle() const { return title; }
    static int getCount() { return count; }
    static int getTotalCreated() { return totalCreated; }
    void print() const { cout << title << " | " << author << " | " << year << '\n'; }
};

int Book::count = 0;
int Book::totalCreated = 0;

class Library {
private:
    // Слабые указатели не владеют книгами и не продлевают их жизнь.
    vector<weak_ptr<Book>> books;

    Library() { cout << "Library created\n"; }
    ~Library() { cout << "Library destroyed\n"; }

    void removeExpired() {
        books.erase(remove_if(books.begin(), books.end(),
            [](const weak_ptr<Book>& b) { return b.expired(); }), books.end());
    }

public:
    Library(const Library&) = delete;
    Library& operator=(const Library&) = delete;

    // Единственный экземпляр создаётся при первом обращении.
    static Library& getInstance() {
        static Library instance;
        return instance;
    }

    bool addBook(const shared_ptr<Book>& book) {
        removeExpired();
        if (!book) return false;
        for (const auto& entry : books)
            if (entry.lock() == book) return false;
        books.push_back(book);
        return true;
    }

    bool removeBook(const shared_ptr<Book>& book) {
        removeExpired();
        if (!book) return false;
        auto it = find_if(books.begin(), books.end(),
            [&book](const weak_ptr<Book>& entry) { return entry.lock() == book; });
        if (it == books.end()) return false;
        books.erase(it);
        return true;
    }

    shared_ptr<Book> findBook(const string& title) const {
        for (const auto& entry : books) {
            auto book = entry.lock();
            if (book && book->getTitle() == title) return book;
        }
        return nullptr;
    }

    size_t getBookCount() {
        removeExpired();
        return books.size();
    }

    void printBooks() {
        removeExpired();
        cout << "Books in library: " << books.size() << '\n';
        if (books.empty()) cout << "Library is empty\n";
        for (const auto& entry : books) {
            auto book = entry.lock();
            if (book) book->print();
        }
    }

    void clear() { books.clear(); }
};

// Проверка ожидаемых результатов без отключаемых assert.
void check(bool condition, const string& message) {
    cout << message << ": " << (condition ? "PASS" : "FAIL") << '\n';
    if (!condition) throw runtime_error(message);
}

int main() {
    try {
        cout << "LAB 1 - YEAR 2 - VARIANT 2\n";
        cout << "\n=== BASIC LEVEL: COMPOSITION ===\n";
        {
            Car car("Toyota Corolla", 16, "Summer");
            car.print();
        }
        cout << "Car and its four wheels no longer exist.\n";

        cout << "\n=== MEDIUM LEVEL: AGGREGATION ===\n";
        {
            Player p1("Alex", 10, "Forward");
            Player p2("Daniel", 1, "Goalkeeper");
            {
                Team team("Eagles");
                check(team.addPlayer(&p1) && team.addPlayer(&p2), "Add players");
                check(!team.addPlayer(nullptr), "Reject null player");
                check(!team.addPlayer(&p1), "Reject duplicate player");
                team.print();
            }
            check(Player::getCount() == 2, "Players still exist after team destruction");
            p1.print();
            p2.print();
        }
        check(Player::getCount() == 0, "All players destroyed");

        cout << "\n=== ADVANCED LEVEL: LIBRARY SINGLETON ===\n";
        Library& library = Library::getInstance();
        Library& second = Library::getInstance();
        check(&library == &second, "Same Singleton instance");
        check(library.getBookCount() == 0 && Book::getCount() == 0, "Initially empty");

        {
            // Владельцы книг находятся вне библиотеки.
            auto b1 = make_shared<Book>("War and Peace", "Leo Tolstoy", 1869);
            auto b2 = make_shared<Book>("Crime and Punishment", "Fyodor Dostoevsky", 1866);
            auto b3 = make_shared<Book>("The Hobbit", "J. R. R. Tolkien", 1937);
            check(library.addBook(b1) && library.addBook(b2) && library.addBook(b3), "Add three books");
            library.printBooks();
            check(Book::getCount() == 3 && library.getBookCount() == 3, "Book counts are correct");
            check(!library.addBook(nullptr), "Reject null book");
            check(!library.addBook(b1), "Reject duplicate book");
            check(library.findBook("The Hobbit") == b3, "Find existing book");
            check(!library.findBook("Unknown"), "Find missing book");
            check(!library.removeBook(nullptr), "Reject null removal");
            check(library.removeBook(b2), "Remove book from library");
            check(!library.removeBook(b2), "Reject missing book removal");
            check(Book::getCount() == 3 && library.getBookCount() == 2, "Removed book still exists");
            b2->print();
            b3.reset();
            check(Book::getCount() == 2 && library.getBookCount() == 1, "Expired book handled safely");
            library.clear();
            check(library.getBookCount() == 0 && Book::getCount() == 2, "Books survive library clear");
            library.printBooks();
            b1->print();
        }
        check(Book::getCount() == 0, "All books destroyed");

        bool invalidRejected = false;
        try {
            Book invalid("", "Unknown", 0);
        } catch (const invalid_argument&) {
            invalidRejected = true;
        }
        check(invalidRejected, "Reject invalid book data");
        check(Book::getCount() == 0 && Book::getTotalCreated() == 3, "Final counters are correct");
        cout << "Total books created: " << Book::getTotalCreated() << '\n';
        cout << "\nProgram completed successfully.\n";
        return 0;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
