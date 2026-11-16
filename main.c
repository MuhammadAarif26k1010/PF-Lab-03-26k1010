#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_BOOKS 200
#define MAX_STUDENTS 200
#define MAX_ROOMS 50
#define MAX_TRANSACTIONS 1000
#define MAX_FINES 500
#define MAX_COMPLAINTS 500
#define MAX_ACTIVITIES 1000
#define MAX_REQUESTS 1000

#define NUM_SLOTS 8      
#define SLOT_START 9     

#define DATA_FILE "library_requests.dat"
#define BACKUP_FILE "library_requests_backup.dat"

//STRUCTURES 

struct Book {
    int id;
    char title[100];
    char author[100];
    char category[50];
    int totalCopies;
    int availableCopies;
    int active;
};

struct Student {
    int id;
    char name[100];
    char department[50];
    int semester;
    char contact[30];
    char password[30];
    int active;
};

struct Room {
    int id;
    char name[50];
    int capacity;
    int type;                   
    int slotStudent[NUM_SLOTS]; 
};


struct Transaction {
    int studentID;
    int bookID;
    int issueDate;
    int dueDate;
    int returnDate;             
    int status;                 
};

struct Fine {
    int studentID;
    int amount;
    int paid;
    char reason[100];
};

struct Complaint {
    int id;
    int studentID;
    char message[200];
    char status[30];            // Pending / Accepted / Rejected
    char reason[100];           // rejection reason (set by admin)
};

struct Activity {
    char message[200];
};

struct Request {
    int id;
    int studentID;
    int type;
    int itemID;
    int slot;                   // slot number (0-7) for room requests
    char message[200];
    int status;
};

// ================= GLOBAL ARRAYS =================

struct Book books[MAX_BOOKS];
struct Student students[MAX_STUDENTS];
struct Room rooms[MAX_ROOMS];
struct Transaction transactions[MAX_TRANSACTIONS];
struct Fine fines[MAX_FINES];
struct Complaint complaints[MAX_COMPLAINTS];
struct Activity activities[MAX_ACTIVITIES];
struct Request requests[MAX_REQUESTS];

int bookCount = 0;
int studentCount = 0;
int roomCount = 0;
int transactionCount = 0;
int fineCount = 0;
int complaintCount = 0;
int activityCount = 0;
int requestCount = 0;

// ================= FUNCTION DECLARATIONS =================

int readInt(char message[]);
void readLine(char message[], char text[], int size);
int readSlot(char message[]);
int readDate(char message[]);
void removeSpaces(char in[], char out[]);

int isLeapYear(int year);
int daysInMonth(int month, int year);
int dateToDays(int date);
void formatDate(int date, char out[]);

void initializeBooks(void);
void initializeStudents(void);
void initializeRooms(void);
void initializeDefaults(void);
void loadData(void);
void saveData(void);
void backupData(void);
void restoreData(void);
void logActivity(char message[]);

int findBook(int id);
int findStudent(int id);
int findRoom(int id);
void slotLabel(int slot, char out[]);

void bookAdministration(void);
void studentAdministration(void);
void roomMonitoring(void);
void fineManagement(void);
void complaintManagement(void);
void systemMonitoring(void);
void reportsAnalytics(void);
void activityLog(void);
void backupRestore(void);
void studentPortal(void);
void requestManagement(void);

// ================= INPUT FUNCTIONS =================

int readInt(char message[]) {
    char line[100];
    int number;
    char extra;

    while (1) {
        printf("%s", message);

        if (fgets(line, sizeof(line), stdin) != NULL &&
            sscanf(line, "%d %c", &number, &extra) == 1) {
            return number;
        }

        printf("Invalid input! Enter a valid number.\n");
    }
}

void readLine(char message[], char text[], int size) {
    printf("%s", message);

    if (fgets(text, size, stdin) != NULL) {
        text[strcspn(text, "\n")] = '\0';
    }
}

// Copies a string without spaces, tabs and line endings
void removeSpaces(char in[], char out[]) {
    int i, j = 0;

    for (i = 0; in[i] != '\0'; i++) {
        if (in[i] != ' ' && in[i] != '\t' &&
            in[i] != '\n' && in[i] != '\r') {
            out[j++] = in[i];
        }
    }

    out[j] = '\0';
}

// Reads a time slot exactly as it is shown in the room view,
// e.g. "09:00 - 10:00".
// Returns slot index (0 - NUM_SLOTS-1), or -1 after 3 invalid attempts.
int readSlot(char message[]) {
    char line[100];
    char compact[100];
    char label[30];
    char expected[30];
    int attempts = 3;
    int s;

    while (attempts > 0) {
        printf("%s", message);

        if (fgets(line, sizeof(line), stdin) != NULL) {
            removeSpaces(line, compact);

            for (s = 0; s < NUM_SLOTS; s++) {
                slotLabel(s, label);
                removeSpaces(label, expected);

                if (strcmp(compact, expected) == 0) {
                    return s;
                }
            }
        }

        attempts--;

        printf("Invalid time slot! Enter exactly as shown, e.g. %02d:00 - %02d:00 "
               "(slots from %02d:00 to %02d:00).\n",
               SLOT_START, SLOT_START + 1,
               SLOT_START, SLOT_START + NUM_SLOTS);

        if (attempts > 0) {
            printf("Attempts remaining: %d\n", attempts);
        }
    }

    printf("Too many invalid attempts!\n");
    return -1;
}

// ================= DATE FUNCTIONS =================

int isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int daysInMonth(int month, int year) {
    int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month == 2 && isLeapYear(year)) {
        return 29;
    }

    return days[month - 1];
}

// Converts YYYYMMDD to a day number (used to count late days)
int dateToDays(int date) {
    int year = date / 10000;
    int month = (date / 100) % 100;
    int day = date % 100;
    int total = 0;
    int i;

    for (i = 2000; i < year; i++) {
        total += isLeapYear(i) ? 366 : 365;
    }

    for (i = 1; i < month; i++) {
        total += daysInMonth(i, year);
    }

    return total + day;
}

// Converts YYYYMMDD to "DD-MM-YYYY"
void formatDate(int date, char out[]) {
    sprintf(out, "%02d-%02d-%04d",
            date % 100, (date / 100) % 100, date / 10000);
}

// Reads a date in DD-MM-YYYY format (e.g. 07-11-2026).
// Returns date as YYYYMMDD, or 0 after 3 invalid attempts.
int readDate(char message[]) {
    char line[100];
    int d, m, y, i, ok;
    int attempts = 3;

    while (attempts > 0) {
        printf("%s", message);

        ok = 0;
        d = m = y = 0;

        if (fgets(line, sizeof(line), stdin) != NULL) {
            line[strcspn(line, "\r\n")] = '\0';

            if (strlen(line) == 10 && line[2] == '-' && line[5] == '-') {
                ok = 1;

                for (i = 0; i < 10; i++) {
                    if (i == 2 || i == 5) continue;

                    if (line[i] < '0' || line[i] > '9') {
                        ok = 0;
                    }
                }

                if (ok) {
                    d = (line[0] - '0') * 10 + (line[1] - '0');
                    m = (line[3] - '0') * 10 + (line[4] - '0');
                    y = (line[6] - '0') * 1000 + (line[7] - '0') * 100 +
                        (line[8] - '0') * 10 + (line[9] - '0');

                    if (y < 2000 || y > 2100 || m < 1 || m > 12 ||
                        d < 1 || d > daysInMonth(m, y)) {
                        ok = 0;
                    }
                }
            }
        }

        if (ok) {
            return y * 10000 + m * 100 + d;
        }

        attempts--;

        printf("Invalid date! Use format DD-MM-YYYY (e.g. 07-11-2026).\n");

        if (attempts > 0) {
            printf("Attempts remaining: %d\n", attempts);
        }
    }

    printf("Too many invalid attempts!\n");
    return 0;
}

// ================= FIND FUNCTIONS =================

int findBook(int id) {
    int i;

    for (i = 0; i < bookCount; i++) {
        if (books[i].id == id && books[i].active == 1) {
            return i;
        }
    }

    return -1;
}

int findStudent(int id) {
    int i;

    for (i = 0; i < studentCount; i++) {
        if (students[i].id == id && students[i].active == 1) {
            return i;
        }
    }

    return -1;
}

int findRoom(int id) {
    int i;

    for (i = 0; i < roomCount; i++) {
        if (rooms[i].id == id) {
            return i;
        }
    }

    return -1;
}

void slotLabel(int slot, char out[]) {
    sprintf(out, "%02d:00 - %02d:00", SLOT_START + slot, SLOT_START + slot + 1);
}

// ================= ACTIVITY LOG =================

void logActivity(char message[]) {
    FILE *fp;
    time_t now;
    char *date;

    fp = fopen("activity.txt", "a");

    if (fp != NULL) {
        now = time(NULL);
        date = ctime(&now);

        if (date != NULL) {
            fprintf(fp, "%s - %s\n", date, message);
        }

        fclose(fp);
    }

    if (activityCount < MAX_ACTIVITIES) {
        strcpy(activities[activityCount].message, message);
        activityCount++;
    }
}

// ================= DEFAULT BOOKS =================

void initializeBooks(void) {

    books[0] = (struct Book){1, "C Programming",
        "Dennis Ritchie", "Computer", 5, 5, 1};

    books[1] = (struct Book){2, "Data Structures",
        "Mark Allen", "Computer", 4, 4, 1};

    books[2] = (struct Book){3, "Database Systems",
        "Elmasri", "Database", 3, 3, 1};

    books[3] = (struct Book){4, "Operating Systems",
        "Galvin", "Computer", 5, 5, 1};

    books[4] = (struct Book){5, "Computer Networks",
        "Tanenbaum", "Networking", 4, 4, 1};

    books[5] = (struct Book){6, "Artificial Intelligence",
        "Russell", "AI", 3, 3, 1};

    books[6] = (struct Book){7, "Digital Logic Design",
        "Morris Mano", "Engineering", 5, 5, 1};

    books[7] = (struct Book){8, "Calculus",
        "Thomas", "Mathematics", 4, 4, 1};

    books[8] = (struct Book){9, "Physics",
        "Halliday", "Science", 3, 3, 1};

    books[9] = (struct Book){10, "English Grammar",
        "Wren and Martin", "English", 5, 5, 1};

    bookCount = 10;
}

// ================= DEFAULT STUDENTS =================
// IDs 1001 - 1010, password for each is "pass" + ID (e.g. pass1001)

void initializeStudents(void) {
    const char *names[10] = {
        "Ali Khan", "Sara Ahmed", "Usman Tariq", "Ayesha Malik",
        "Hassan Raza", "Fatima Noor", "Bilal Hussain", "Zainab Shah",
        "Hamza Iqbal", "Maryam Siddiqui"
    };

    const char *departments[10] = {
        "Computer Science", "Software Engineering", "Electrical",
        "Computer Science", "Mechanical", "Software Engineering",
        "Computer Science", "Mathematics", "Electrical", "Physics"
    };

    int i;

    studentCount = 0;

    for (i = 0; i < 10; i++) {
        memset(&students[i], 0, sizeof(struct Student));

        students[i].id = 1001 + i;
        strcpy(students[i].name, names[i]);
        strcpy(students[i].department, departments[i]);
        students[i].semester = (i % 8) + 1;
        sprintf(students[i].contact, "0300-12345%02d", i);
        sprintf(students[i].password, "pass%d", 1001 + i);
        students[i].active = 1;
    }

    studentCount = 10;
}

// ================= DEFAULT ROOMS =================

void initializeRooms(void) {
    int i;

    roomCount = 0;

    // 10 Computer Labs (ID 101 - 110)
    for (i = 0; i < 10; i++) {
        memset(&rooms[roomCount], 0, sizeof(struct Room));
        rooms[roomCount].id = 101 + i;
        sprintf(rooms[roomCount].name, "Computer Lab %d", i + 1);
        rooms[roomCount].capacity = 30;
        rooms[roomCount].type = 2;
        roomCount++;
    }

    // 10 Discussion Rooms (ID 201 - 210)
    for (i = 0; i < 10; i++) {
        memset(&rooms[roomCount], 0, sizeof(struct Room));
        rooms[roomCount].id = 201 + i;
        sprintf(rooms[roomCount].name, "Discussion Room %d", i + 1);
        rooms[roomCount].capacity = 8;
        rooms[roomCount].type = 1;
        roomCount++;
    }
}

void initializeDefaults(void) {
    initializeBooks();
    initializeStudents();
    initializeRooms();
}

// ================= SAVE DATA =================

void saveData(void) {
    FILE *fp;

    fp = fopen(DATA_FILE, "wb");

    if (fp == NULL) {
        printf("Error saving data!\n");
        return;
    }

    fwrite(&bookCount, sizeof(int), 1, fp);
    fwrite(books, sizeof(struct Book), MAX_BOOKS, fp);

    fwrite(&studentCount, sizeof(int), 1, fp);
    fwrite(students, sizeof(struct Student), MAX_STUDENTS, fp);

    fwrite(&roomCount, sizeof(int), 1, fp);
    fwrite(rooms, sizeof(struct Room), MAX_ROOMS, fp);

    fwrite(&transactionCount, sizeof(int), 1, fp);
    fwrite(transactions, sizeof(struct Transaction),
           MAX_TRANSACTIONS, fp);

    fwrite(&fineCount, sizeof(int), 1, fp);
    fwrite(fines, sizeof(struct Fine), MAX_FINES, fp);

    fwrite(&complaintCount, sizeof(int), 1, fp);
    fwrite(complaints, sizeof(struct Complaint), MAX_COMPLAINTS, fp);

    fwrite(&activityCount, sizeof(int), 1, fp);
    fwrite(activities, sizeof(struct Activity), MAX_ACTIVITIES, fp);

    fwrite(&requestCount, sizeof(int), 1, fp);
    fwrite(requests, sizeof(struct Request), MAX_REQUESTS, fp);

    fclose(fp);
}

// ================= LOAD DATA =================

void loadData(void) {
    FILE *fp;

    fp = fopen(DATA_FILE, "rb");

    if (fp == NULL) {
        initializeDefaults();
        return;
    }

    if (fread(&bookCount, sizeof(int), 1, fp) != 1 ||
        fread(books, sizeof(struct Book), MAX_BOOKS, fp) != MAX_BOOKS ||
        fread(&studentCount, sizeof(int), 1, fp) != 1 ||
        fread(students, sizeof(struct Student), MAX_STUDENTS, fp) != MAX_STUDENTS ||
        fread(&roomCount, sizeof(int), 1, fp) != 1 ||
        fread(rooms, sizeof(struct Room), MAX_ROOMS, fp) != MAX_ROOMS ||
        fread(&transactionCount, sizeof(int), 1, fp) != 1 ||
        fread(transactions, sizeof(struct Transaction),
              MAX_TRANSACTIONS, fp) != MAX_TRANSACTIONS ||
        fread(&fineCount, sizeof(int), 1, fp) != 1 ||
        fread(fines, sizeof(struct Fine), MAX_FINES, fp) != MAX_FINES ||
        fread(&complaintCount, sizeof(int), 1, fp) != 1 ||
        fread(complaints, sizeof(struct Complaint),
              MAX_COMPLAINTS, fp) != MAX_COMPLAINTS ||
        fread(&activityCount, sizeof(int), 1, fp) != 1 ||
        fread(activities, sizeof(struct Activity),
              MAX_ACTIVITIES, fp) != MAX_ACTIVITIES ||
        fread(&requestCount, sizeof(int), 1, fp) != 1 ||
        fread(requests, sizeof(struct Request),
              MAX_REQUESTS, fp) != MAX_REQUESTS) {

        printf("Invalid data file! Loading default data.\n");

        fclose(fp);

        bookCount = studentCount = roomCount = 0;
        transactionCount = fineCount = complaintCount = activityCount = requestCount = 0;

        initializeDefaults();
        saveData();
        return;
    }

    fclose(fp);

    if (bookCount < 0 || bookCount > MAX_BOOKS ||
        studentCount < 0 || studentCount > MAX_STUDENTS ||
        roomCount < 0 || roomCount > MAX_ROOMS ||
        transactionCount < 0 || transactionCount > MAX_TRANSACTIONS ||
        fineCount < 0 || fineCount > MAX_FINES ||
        complaintCount < 0 || complaintCount > MAX_COMPLAINTS ||
        activityCount < 0 || activityCount > MAX_ACTIVITIES ||
        requestCount < 0 || requestCount > MAX_REQUESTS) {

        printf("Invalid record counts! Loading default data.\n");

        bookCount = studentCount = roomCount = 0;
        transactionCount = fineCount = complaintCount = activityCount = requestCount = 0;

        initializeDefaults();
        saveData();
    }
}

// ================= BACKUP DATA =================

void backupData(void) {
    FILE *source, *destination;
    int ch;

    saveData();

    source = fopen(DATA_FILE, "rb");

    if (source == NULL) {
        printf("Original data file not found!\n");
        return;
    }

    destination = fopen(BACKUP_FILE, "wb");

    if (destination == NULL) {
        printf("Cannot create backup file!\n");
        fclose(source);
        return;
    }

    while ((ch = fgetc(source)) != EOF) {
        fputc(ch, destination);
    }

    fclose(source);
    fclose(destination);

    printf("Backup created successfully!\n");
    logActivity("Data backup created");
    saveData();
}

// ================= RESTORE DATA =================

void restoreData(void) {
    FILE *source, *destination;
    int ch;

    source = fopen(BACKUP_FILE, "rb");

    if (source == NULL) {
        printf("Backup file not found!\n");
        return;
    }

    destination = fopen(DATA_FILE, "wb");

    if (destination == NULL) {
        printf("Cannot restore data!\n");
        fclose(source);
        return;
    }

    while ((ch = fgetc(source)) != EOF) {
        fputc(ch, destination);
    }

    fclose(source);
    fclose(destination);

    loadData();

    printf("Data restored successfully!\n");
    logActivity("Backup data restored");
    saveData();
}

// =================================================
// 1. BOOK ADMINISTRATION
// =================================================

void addBook(void) {
    int id;

    if (bookCount >= MAX_BOOKS) {
        printf("Book storage full!\n");
        return;
    }

    id = readInt("Enter Book ID: ");

    if (findBook(id) != -1) {
        printf("Book already exists!\n");
        return;
    }

    books[bookCount].id = id;

    readLine("Enter Title: ", books[bookCount].title, 100);
    readLine("Enter Author: ", books[bookCount].author, 100);
    readLine("Enter Category: ", books[bookCount].category, 50);

    books[bookCount].totalCopies = readInt("Enter Copies: ");

    if (books[bookCount].totalCopies <= 0) {
        printf("Invalid copies!\n");
        return;
    }

    books[bookCount].availableCopies = books[bookCount].totalCopies;
    books[bookCount].active = 1;

    bookCount++;

    logActivity("New book added");
    saveData();

    printf("Book added successfully!\n");
}

void displayBooks(void) {
    int i;

    printf("\n========== ALL BOOKS ==========\n");

    for (i = 0; i < bookCount; i++) {
        if (books[i].active) {
            printf("\nID: %d\n", books[i].id);
            printf("Title: %s\n", books[i].title);
            printf("Author: %s\n", books[i].author);
            printf("Category: %s\n", books[i].category);
            printf("Total Copies: %d\n", books[i].totalCopies);
            printf("Available: %d\n", books[i].availableCopies);
        }
    }
}

void searchBook(void) {
    int id, index;

    id = readInt("Enter Book ID: ");
    index = findBook(id);

    if (index == -1) {
        printf("Book not found!\n");
        return;
    }

    printf("\nTitle: %s\n", books[index].title);
    printf("Author: %s\n", books[index].author);
    printf("Category: %s\n", books[index].category);
    printf("Available Copies: %d\n", books[index].availableCopies);
}

void updateBook(void) {
    int id, index;

    id = readInt("Enter Book ID: ");
    index = findBook(id);

    if (index == -1) {
        printf("Book not found!\n");
        return;
    }

    readLine("New Title: ", books[index].title, 100);
    readLine("New Author: ", books[index].author, 100);
    readLine("New Category: ", books[index].category, 50);

    logActivity("Book details updated");
    saveData();

    printf("Book updated!\n");
}

void removeBook(void) {
    int id, index;

    id = readInt("Enter Book ID: ");
    index = findBook(id);

    if (index == -1) {
        printf("Book not found!\n");
        return;
    }

    if (books[index].availableCopies != books[index].totalCopies) {
        printf("Cannot remove issued books!\n");
        return;
    }

    books[index].active = 0;

    logActivity("Book removed");
    saveData();

    printf("Book removed successfully!\n");
}

void issueBook(void) {
    int sid, bid, index;
    int issue, due;

    if (transactionCount >= MAX_TRANSACTIONS) {
        printf("Transaction storage full!\n");
        return;
    }

    sid = readInt("Enter Student ID: ");

    if (findStudent(sid) == -1) {
        printf("Student not found! Register student first.\n");
        return;
    }

    bid = readInt("Enter Book ID: ");
    index = findBook(bid);

    if (index == -1 || books[index].availableCopies <= 0) {
        printf("Book not available!\n");
        return;
    }

    issue = readDate("Enter Issue Date (DD-MM-YYYY): ");

    if (issue == 0) {
        return;
    }

    due = readDate("Enter Due Date (DD-MM-YYYY): ");

    if (due == 0) {
        return;
    }

    if (due < issue) {
        printf("Due date cannot be before issue date!\n");
        return;
    }

    transactions[transactionCount].studentID = sid;
    transactions[transactionCount].bookID = bid;
    transactions[transactionCount].issueDate = issue;
    transactions[transactionCount].dueDate = due;
    transactions[transactionCount].returnDate = 0;
    transactions[transactionCount].status = 1;

    transactionCount++;
    books[index].availableCopies--;

    logActivity("Book issued to student");
    saveData();

    printf("Book issued successfully!\n");
}

void returnBook(void) {
    int sid, bid, i, index, returnDate, lateDays;

    sid = readInt("Enter Student ID: ");
    bid = readInt("Enter Book ID: ");

    for (i = transactionCount - 1; i >= 0; i--) {
        if (transactions[i].studentID == sid &&
            transactions[i].bookID == bid &&
            transactions[i].status == 1) {

            returnDate = readDate("Enter Return Date (DD-MM-YYYY): ");

            if (returnDate == 0) {
                return;
            }

            if (returnDate < transactions[i].issueDate) {
                printf("Return date cannot be before issue date!\n");
                return;
            }

            transactions[i].returnDate = returnDate;
            transactions[i].status = 2;

            index = findBook(bid);

            if (index != -1) {
                books[index].availableCopies++;
            }

            lateDays = dateToDays(returnDate) - dateToDays(transactions[i].dueDate);

            if (lateDays > 0 && fineCount < MAX_FINES) {
                fines[fineCount].studentID = sid;
                fines[fineCount].amount = lateDays * 10;
                fines[fineCount].paid = 0;
                strcpy(fines[fineCount].reason, "Late book return");

                fineCount++;

                printf("Late days: %d\n", lateDays);
                printf("Fine added: Rs. %d\n", lateDays * 10);
            }

            logActivity("Book returned");
            saveData();

            printf("Book returned successfully!\n");
            return;
        }
    }

    printf("Active issue record not found!\n");
}

void overdueBooks(void) {
    int today, i, found = 0;
    char due[12];

    today = readDate("Enter Current Date (DD-MM-YYYY): ");

    if (today == 0) {
        return;
    }

    printf("\n--- OVERDUE BOOKS ---\n");

    for (i = 0; i < transactionCount; i++) {
        if (transactions[i].status == 1 &&
            today > transactions[i].dueDate) {

            formatDate(transactions[i].dueDate, due);

            printf("Student ID: %d | Book ID: %d | Due Date: %s\n",
                   transactions[i].studentID,
                   transactions[i].bookID,
                   due);

            found = 1;
        }
    }

    if (!found) {
        printf("No overdue books!\n");
    }
}

void damagedLostBooks(void) {
    int id, quantity, index;

    id = readInt("Enter Book ID: ");
    index = findBook(id);

    if (index == -1) {
        printf("Book not found!\n");
        return;
    }

    quantity = readInt("Enter damaged/lost quantity: ");

    if (quantity <= 0 || quantity > books[index].availableCopies) {
        printf("Invalid quantity!\n");
        return;
    }

    books[index].totalCopies -= quantity;
    books[index].availableCopies -= quantity;

    logActivity("Damaged or lost books recorded");
    saveData();

    printf("Book stock updated!\n");
}

void checkAvailableBooks(void) {
    int i, found = 0;

    printf("\n========== AVAILABLE BOOKS ==========\n");

    for (i = 0; i < bookCount; i++) {
        if (books[i].active && books[i].availableCopies > 0) {
            printf("ID: %d | %s | Available: %d\n",
                   books[i].id, books[i].title,
                   books[i].availableCopies);
            found = 1;
        }
    }

    if (!found)
        printf("No books are currently available.\n");
}

void viewIssuedBooks(void) {
    int i, found = 0;
    char due[12];

    printf("\n========== CURRENTLY ISSUED BOOKS ==========\n");

    for (i = 0; i < transactionCount; i++) {
        if (transactions[i].status == 1) {
            formatDate(transactions[i].dueDate, due);

            printf("Student ID: %d | Book ID: %d | Due Date: %s\n",
                   transactions[i].studentID,
                   transactions[i].bookID,
                   due);
            found = 1;
        }
    }

    if (!found)
        printf("No books are currently issued.\n");
}

void viewReturnedBooks(void) {
    int i, found = 0;
    char ret[12];

    printf("\n========== RETURNED BOOKS ==========\n");

    for (i = 0; i < transactionCount; i++) {
        if (transactions[i].status == 2) {
            formatDate(transactions[i].returnDate, ret);

            printf("Student ID: %d | Book ID: %d | Return Date: %s\n",
                   transactions[i].studentID,
                   transactions[i].bookID,
                   ret);
            found = 1;
        }
    }

    if (!found)
        printf("No returned-book records.\n");
}

void viewBookIssueHistory(void) {
    int i;
    char issue[12], due[12], ret[12];

    printf("\n========== BOOK ISSUE HISTORY ==========\n");

    if (transactionCount == 0) {
        printf("No issue history available.\n");
        return;
    }

    for (i = 0; i < transactionCount; i++) {
        formatDate(transactions[i].issueDate, issue);
        formatDate(transactions[i].dueDate, due);

        printf("Student: %d | Book: %d | Issue Date: %s | Due Date: %s | ",
               transactions[i].studentID,
               transactions[i].bookID,
               issue,
               due);

        if (transactions[i].status == 1) {
            printf("Status: Issued\n");
        } else {
            formatDate(transactions[i].returnDate, ret);
            printf("Status: Returned on %s\n", ret);
        }
    }
}

void manageBookCategories(void) {
    int i, j, found, id, index;
    char newCategory[50];

    printf("\n========== BOOK CATEGORIES ==========\n");

    for (i = 0; i < bookCount; i++) {
        if (!books[i].active) continue;

        found = 0;

        for (j = 0; j < i; j++) {
            if (books[j].active &&
                strcmp(books[i].category, books[j].category) == 0) {
                found = 1;
                break;
            }
        }

        if (!found)
            printf("- %s\n", books[i].category);
    }

    printf("\n1. Change category of a book\n");
    printf("2. Back\n");

    if (readInt("Enter Choice: ") == 1) {
        id = readInt("Enter Book ID: ");
        index = findBook(id);

        if (index == -1) {
            printf("Book not found.\n");
            return;
        }

        readLine("Enter new category: ", newCategory, 50);
        strcpy(books[index].category, newCategory);

        logActivity("Book category updated");
        saveData();

        printf("Category updated successfully.\n");
    }
}

void viewBookRequests(void) {
    int i, found = 0;

    printf("\n========== BOOK ISSUE REQUESTS ==========\n");

    for (i = 0; i < requestCount; i++) {
        if (requests[i].type == 1 && requests[i].status == 0) {
            printf("Request ID: %d | Student ID: %d | Book ID: %d\n",
                   requests[i].id,
                   requests[i].studentID,
                   requests[i].itemID);

            printf("Details: %s\n", requests[i].message);
            found = 1;
        }
    }

    if (!found)
        printf("No pending book issue requests.\n");
}

void bookAdministration(void) {
    int choice;

    do {
        printf("\n========== BOOK ADMINISTRATION ==========\n");
        printf("1. ADD BOOK\n");
        printf("2. DISPLAY ALL BOOKS\n");
        printf("3. SEARCH BOOK\n");
        printf("4. UPDATE BOOK DETAIL\n");
        printf("5. REMOVE BOOK\n");
        printf("6. ISSUE BOOK (ADMIN DIRECT)\n");
        printf("7. RETURN BOOK\n");
        printf("8. SYSTEM MONITORING\n");
        printf("9. CHECK AVAILABLE BOOKS\n");
        printf("10. VIEW ISSUED BOOKS\n");
        printf("11. VIEW RETURNED BOOKS\n");
        printf("12. VIEW OVERDUE BOOKS\n");
        printf("13. MANAGE BOOK CATEGORIES\n");
        printf("14. DAMAGED / LOST BOOKS\n");
        printf("15. BOOK ISSUE REQUESTS\n");
        printf("16. BOOK ISSUE HISTORY\n");
        printf("17. BACK TO ADMIN PANEL\n");

        choice = readInt("Enter Choice: ");

        switch (choice) {
            case 1: addBook(); break;
            case 2: displayBooks(); break;
            case 3: searchBook(); break;
            case 4: updateBook(); break;
            case 5: removeBook(); break;
            case 6: issueBook(); break;
            case 7: returnBook(); break;
            case 8: systemMonitoring(); break;
            case 9: checkAvailableBooks(); break;
            case 10: viewIssuedBooks(); break;
            case 11: viewReturnedBooks(); break;
            case 12: overdueBooks(); break;
            case 13: manageBookCategories(); break;
            case 14: damagedLostBooks(); break;
            case 15: viewBookRequests(); break;
            case 16: viewBookIssueHistory(); break;
            case 17: break;
            default: printf("Invalid choice!\n");
        }

    } while (choice != 17);
}

// =================================================
// 2. STUDENT ADMINISTRATION
// =================================================

void addStudent(void) {
    int id;
    int semester;
    char name[100];
    char department[50];
    char contact[30];
    char password[30];

    if (studentCount >= MAX_STUDENTS) {
        printf("Student storage full!\n");
        return;
    }

    id = readInt("Enter Student ID: ");

    if (id <= 0) {
        printf("Invalid student ID!\n");
        return;
    }

    if (findStudent(id) != -1) {
        printf("Student already exists!\n");
        return;
    }

    readLine("Enter Name: ", name, 100);
    readLine("Enter Department: ", department, 50);

    semester = readInt("Enter Semester: ");

    readLine("Enter Contact: ", contact, 30);

    if (semester < 1 || semester > 12) {
        printf("Invalid semester!\n");
        return;
    }

    readLine("Enter Password (min 4 characters): ", password, 30);

    if (strlen(password) < 4) {
        printf("Password too short!\n");
        return;
    }

    memset(&students[studentCount], 0, sizeof(struct Student));

    students[studentCount].id = id;
    strcpy(students[studentCount].name, name);
    strcpy(students[studentCount].department, department);
    students[studentCount].semester = semester;
    strcpy(students[studentCount].contact, contact);
    strcpy(students[studentCount].password, password);
    students[studentCount].active = 1;

    studentCount++;

    logActivity("New student registered");
    saveData();

    printf("Student registered successfully!\n");
}

void displayStudents(void) {
    int i;

    printf("\n========== STUDENTS ==========\n");

    for (i = 0; i < studentCount; i++) {
        if (students[i].active) {
            printf("\nID: %d\n", students[i].id);
            printf("Name: %s\n", students[i].name);
            printf("Department: %s\n", students[i].department);
            printf("Semester: %d\n", students[i].semester);
            printf("Contact: %s\n", students[i].contact);
        }
    }
}

void searchStudent(void) {
    int id, index;

    id = readInt("Enter Student ID: ");
    index = findStudent(id);

    if (index == -1) {
        printf("Student not found!\n");
        return;
    }

    printf("\nName: %s\n", students[index].name);
    printf("Department: %s\n", students[index].department);
    printf("Semester: %d\n", students[index].semester);
    printf("Contact: %s\n", students[index].contact);
}

void updateStudent(void) {
    int id, index;
    char password[30];

    id = readInt("Enter Student ID: ");
    index = findStudent(id);

    if (index == -1) {
        printf("Student not found!\n");
        return;
    }

    readLine("New Name: ", students[index].name, 100);
    readLine("New Department: ", students[index].department, 50);

    students[index].semester = readInt("New Semester: ");

    readLine("New Contact: ", students[index].contact, 30);

    readLine("New Password (leave empty to keep old): ", password, 30);

    if (strlen(password) >= 4) {
        strcpy(students[index].password, password);
    } else if (strlen(password) > 0) {
        printf("Password too short! Old password kept.\n");
    }

    logActivity("Student information updated");
    saveData();

    printf("Student updated!\n");
}

void removeStudent(void) {
    int id, index, i;

    id = readInt("Enter Student ID: ");
    index = findStudent(id);

    if (index == -1) {
        printf("Student not found!\n");
        return;
    }

    for (i = 0; i < transactionCount; i++) {
        if (transactions[i].studentID == id &&
            transactions[i].status == 1) {
            printf("Student has an issued book. Return it first!\n");
            return;
        }
    }

    students[index].active = 0;

    logActivity("Student removed");
    saveData();

    printf("Student removed!\n");
}

void studentHistory(void) {
    int id, i, found = 0;
    char issue[12], due[12], ret[12];

    id = readInt("Enter Student ID: ");

    if (findStudent(id) == -1) {
        printf("Student not found!\n");
        return;
    }

    for (i = 0; i < transactionCount; i++) {
        if (transactions[i].studentID == id) {
            formatDate(transactions[i].issueDate, issue);
            formatDate(transactions[i].dueDate, due);

            printf("Book ID: %d | Issue: %s | Due: %s | ",
                   transactions[i].bookID,
                   issue,
                   due);

            if (transactions[i].status == 1) {
                printf("Issued\n");
            } else {
                formatDate(transactions[i].returnDate, ret);
                printf("Returned on %s\n", ret);
            }

            found = 1;
        }
    }

    if (!found) {
        printf("No borrowing history!\n");
    }
}

void studentAdministration(void) {
    int choice;

    do {
        printf("\n========== STUDENT ADMINISTRATION ==========\n");
        printf("1. Add Student\n");
        printf("2. Display Students\n");
        printf("3. Search Student\n");
        printf("4. Update Student\n");
        printf("5. Remove Student\n");
        printf("6. Student Borrowing History\n");
        printf("7. Back\n");

        choice = readInt("Enter Choice: ");

        switch (choice) {
            case 1: addStudent(); break;
            case 2: displayStudents(); break;
            case 3: searchStudent(); break;
            case 4: updateStudent(); break;
            case 5: removeStudent(); break;
            case 6: studentHistory(); break;
            case 7: break;
            default: printf("Invalid choice!\n");
        }

    } while (choice != 7);
}

// =================================================
// 3. ROOM & LAB MONITORING
// =================================================

void addRoom(void) {
    int id, type, capacity;

    if (roomCount >= MAX_ROOMS) {
        printf("Room storage full!\n");
        return;
    }

    id = readInt("Enter Room/Lab ID: ");

    if (findRoom(id) != -1) {
        printf("Room ID already exists!\n");
        return;
    }

    memset(&rooms[roomCount], 0, sizeof(struct Room));
    rooms[roomCount].id = id;

    readLine("Enter Room/Lab Name: ", rooms[roomCount].name, 50);

    capacity = readInt("Enter Capacity: ");

    if (capacity <= 0) {
        printf("Invalid capacity!\n");
        return;
    }

    printf("1. Discussion Room\n");
    printf("2. Computer Lab\n");

    type = readInt("Enter Type: ");

    if (type != 1 && type != 2) {
        printf("Invalid type!\n");
        return;
    }

    rooms[roomCount].capacity = capacity;
    rooms[roomCount].type = type;

    roomCount++;

    logActivity("New room or lab added");
    saveData();

    printf("Room added successfully!\n");
}

void printRoomSlots(int r, int showWho) {
    int s;
    char label[30];

    for (s = 0; s < NUM_SLOTS; s++) {
        slotLabel(s, label);

        if (rooms[r].slotStudent[s] == 0) {
            printf("   %s  [ ] FREE\n", label);
        } else if (showWho) {
            printf("   %s  [X] BOOKED (Student ID: %d)\n",
                   label, rooms[r].slotStudent[s]);
        } else {
            printf("   %s  [X] BOOKED\n", label);
        }
    }
}

void viewRooms(int showWho) {
    int i, filter;

    printf("\n1. Computer Labs\n");
    printf("2. Discussion Rooms\n");
    printf("3. All\n");

    filter = readInt("Enter Choice: ");

    if (filter < 1 || filter > 3) {
        printf("Invalid choice!\n");
        return;
    }

    printf("\n========== ROOMS & LABS ==========\n");

    for (i = 0; i < roomCount; i++) {
        if (filter == 1 && rooms[i].type != 2) continue;
        if (filter == 2 && rooms[i].type != 1) continue;

        printf("\nID: %d | %s | %s | Capacity: %d\n",
               rooms[i].id,
               rooms[i].name,
               rooms[i].type == 1 ? "Discussion Room" : "Computer Lab",
               rooms[i].capacity);

        printRoomSlots(i, showWho);
    }
}

void bookRoom(void) {
    int id, sid, r, slot;

    id = readInt("Enter Room ID: ");
    r = findRoom(id);

    if (r == -1) {
        printf("Room not found!\n");
        return;
    }

    sid = readInt("Enter Student ID: ");

    if (findStudent(sid) == -1) {
        printf("Student not found!\n");
        return;
    }

    printRoomSlots(r, 1);
    slot = readSlot("Enter Time Slot (e.g. 09:00 - 10:00): ");

    if (slot == -1) {
        return;
    }

    if (rooms[r].slotStudent[slot] != 0) {
        printf("This slot is already booked!\n");
        return;
    }

    rooms[r].slotStudent[slot] = sid;

    logActivity("Room booked by admin");
    saveData();

    printf("Room booked successfully!\n");
}

void releaseRoom(void) {
    int id, r, slot;

    id = readInt("Enter Room ID: ");
    r = findRoom(id);

    if (r == -1) {
        printf("Room not found!\n");
        return;
    }

    printRoomSlots(r, 1);
    slot = readSlot("Enter Time Slot to release (e.g. 09:00 - 10:00): ");

    if (slot == -1) {
        return;
    }

    if (rooms[r].slotStudent[slot] == 0) {
        printf("Slot is already free!\n");
        return;
    }

    rooms[r].slotStudent[slot] = 0;

    logActivity("Room slot released");
    saveData();

    printf("Slot released successfully!\n");
}

void roomMonitoring(void) {
    int choice;

    do {
        printf("\n========== ROOM & LAB MONITORING ==========\n");
        printf("1. Add Room / Lab\n");
        printf("2. View Rooms / Labs\n");
        printf("3. Book Room / Lab\n");
        printf("4. Release Room / Lab\n");
        printf("5. Back\n");

        choice = readInt("Enter Choice: ");

        switch (choice) {
            case 1: addRoom(); break;
            case 2: viewRooms(1); break;
            case 3: bookRoom(); break;
            case 4: releaseRoom(); break;
            case 5: break;
            default: printf("Invalid choice!\n");
        }

    } while (choice != 5);
}

// =================================================
// 4. PENALTY & FINE MANAGEMENT
// =================================================

void addFine(void) {
    int sid, amount;

    if (fineCount >= MAX_FINES) {
        printf("Fine storage full!\n");
        return;
    }

    sid = readInt("Enter Student ID: ");

    if (findStudent(sid) == -1) {
        printf("Student not found!\n");
        return;
    }

    amount = readInt("Enter Fine Amount: ");

    if (amount <= 0) {
        printf("Invalid amount!\n");
        return;
    }

    fines[fineCount].studentID = sid;
    fines[fineCount].amount = amount;
    fines[fineCount].paid = 0;

    readLine("Enter Fine Reason: ", fines[fineCount].reason, 100);

    fineCount++;

    logActivity("New fine added");
    saveData();

    printf("Fine added successfully!\n");
}

void viewFines(void) {
    int i;

    printf("\n========== FINE RECORDS ==========\n");

    for (i = 0; i < fineCount; i++) {
        printf("\nStudent ID: %d\n", fines[i].studentID);
        printf("Amount: Rs. %d\n", fines[i].amount);
        printf("Reason: %s\n", fines[i].reason);
        printf("Status: %s\n", fines[i].paid ? "Paid" : "Pending");
    }
}

void payFine(void) {
    int sid, i, found = 0;

    sid = readInt("Enter Student ID: ");

    for (i = 0; i < fineCount; i++) {
        if (fines[i].studentID == sid && fines[i].paid == 0) {
            printf("Pending Fine: Rs. %d\n", fines[i].amount);

            fines[i].paid = 1;
            found = 1;
        }
    }

    if (found) {
        logActivity("Student fine payment recorded");
        saveData();
        printf("Fine payment completed!\n");
    } else {
        printf("No pending fines found!\n");
    }
}

void fineManagement(void) {
    int choice;

    do {
        printf("\n========== PENALTY & FINE MANAGEMENT ==========\n");
        printf("1. Add Fine\n");
        printf("2. View Fines\n");
        printf("3. Pay Fine\n");
        printf("4. Back\n");

        choice = readInt("Enter Choice: ");

        switch (choice) {
            case 1: addFine(); break;
            case 2: viewFines(); break;
            case 3: payFine(); break;
            case 4: break;
            default: printf("Invalid choice!\n");
        }

    } while (choice != 4);
}

// =================================================
// 5. COMPLAINT & SUGGESTION MANAGEMENT
// =================================================

// Used by both admin and student portal. New complaints are always Pending.
void submitComplaint(int sid, char message[]) {
    if (complaintCount >= MAX_COMPLAINTS) {
        printf("Complaint storage full!\n");
        return;
    }

    if (strlen(message) == 0) {
        printf("Complaint cannot be empty!\n");
        return;
    }

    complaints[complaintCount].id = complaintCount + 1;
    complaints[complaintCount].studentID = sid;

    strcpy(complaints[complaintCount].message, message);
    strcpy(complaints[complaintCount].status, "Pending");
    strcpy(complaints[complaintCount].reason, "");

    complaintCount++;

    logActivity("New complaint or suggestion received");
    saveData();

    printf("Complaint submitted! Your Complaint ID is %d\n",
           complaints[complaintCount - 1].id);
}

void addComplaint(void) {
    int sid;
    char message[200];

    sid = readInt("Enter Student ID: ");

    if (findStudent(sid) == -1) {
        printf("Student not found!\n");
        return;
    }

    readLine("Enter Complaint / Suggestion: ", message, 200);

    submitComplaint(sid, message);
}

void viewComplaints(void) {
    int i;

    printf("\n========== COMPLAINTS ==========\n");

    if (complaintCount == 0) {
        printf("No complaints found.\n");
        return;
    }

    for (i = 0; i < complaintCount; i++) {
        printf("\nComplaint ID: %d\n", complaints[i].id);
        printf("Student ID: %d\n", complaints[i].studentID);
        printf("Message: %s\n", complaints[i].message);
        printf("Status: %s\n", complaints[i].status);

        if (strcmp(complaints[i].status, "Rejected") == 0) {
            printf("Rejection Reason: %s\n", complaints[i].reason);
        }
    }
}

void updateComplaint(void) {
    int id, i, choice;

    id = readInt("Enter Complaint ID: ");

    for (i = 0; i < complaintCount; i++) {
        if (complaints[i].id == id) {

            if (strcmp(complaints[i].status, "Pending") != 0) {
                printf("Complaint already %s.\n", complaints[i].status);
                return;
            }

            printf("\nStudent ID: %d\n", complaints[i].studentID);
            printf("Message: %s\n", complaints[i].message);

            printf("\n1. Accept\n");
            printf("2. Reject\n");

            choice = readInt("Enter Choice: ");

            if (choice == 1) {
                strcpy(complaints[i].status, "Accepted");
                strcpy(complaints[i].reason, "");

                logActivity("Complaint accepted");
                saveData();

                printf("Complaint accepted!\n");
                return;
            }

            if (choice == 2) {
                while (1) {
                    readLine("Enter rejection reason: ",
                             complaints[i].reason, 100);

                    if (strlen(complaints[i].reason) > 0) {
                        break;
                    }

                    printf("Reason cannot be empty!\n");
                }

                strcpy(complaints[i].status, "Rejected");

                logActivity("Complaint rejected");
                saveData();

                printf("Complaint rejected!\n");
                return;
            }

            printf("Invalid choice!\n");
            return;
        }
    }

    printf("Complaint not found!\n");
}

void complaintManagement(void) {
    int choice;

    do {
        printf("\n========== COMPLAINT MANAGEMENT ==========\n");
        printf("1. Add Complaint / Suggestion\n");
        printf("2. View Complaints\n");
        printf("3. Accept / Reject Complaint\n");
        printf("4. Back\n");

        choice = readInt("Enter Choice: ");

        switch (choice) {
            case 1: addComplaint(); break;
            case 2: viewComplaints(); break;
            case 3: updateComplaint(); break;
            case 4: break;
            default: printf("Invalid choice!\n");
        }

    } while (choice != 4);
}

// =================================================
// 6. SYSTEM MONITORING
// =================================================

void systemMonitoring(void) {
    int i, s;
    int activeBooks = 0, available = 0;
    int activeStudents = 0, issued = 0;
    int occupied = 0, pendingFines = 0;
    int pendingComplaints = 0;

    for (i = 0; i < bookCount; i++) {
        if (books[i].active) {
            activeBooks++;
            available += books[i].availableCopies;
        }
    }

    for (i = 0; i < studentCount; i++) {
        if (students[i].active) {
            activeStudents++;
        }
    }

    for (i = 0; i < transactionCount; i++) {
        if (transactions[i].status == 1) {
            issued++;
        }
    }

    for (i = 0; i < roomCount; i++) {
        for (s = 0; s < NUM_SLOTS; s++) {
            if (rooms[i].slotStudent[s] != 0) {
                occupied++;
            }
        }
    }

    for (i = 0; i < fineCount; i++) {
        if (!fines[i].paid) {
            pendingFines += fines[i].amount;
        }
    }

    for (i = 0; i < complaintCount; i++) {
        if (strcmp(complaints[i].status, "Pending") == 0) {
            pendingComplaints++;
        }
    }

    printf("\n========== SYSTEM MONITORING ==========\n");
    printf("Active Book Titles: %d\n", activeBooks);
    printf("Available Book Copies: %d\n", available);
    printf("Active Students: %d\n", activeStudents);
    printf("Issued Transactions: %d\n", issued);
    printf("Total Transactions: %d\n", transactionCount);
    printf("Total Rooms/Labs: %d\n", roomCount);
    printf("Booked Room/Lab Slots: %d / %d\n", occupied, roomCount * NUM_SLOTS);
    printf("Pending Fine Amount: Rs. %d\n", pendingFines);
    printf("Pending Complaints: %d\n", pendingComplaints);
}

// =================================================
// 7. REPORT & ANALYTICS
// =================================================

void reportsAnalytics(void) {
    int choice, i, j;
    int totalIssued = 0, totalReturned = 0;
    int totalFine = 0, paidFine = 0;
    int bestIndex = -1, bestCount = 0;

    do {
        printf("\n========== REPORT & ANALYTICS ==========\n");
        printf("1. Book Issue Report\n");
        printf("2. Student Report\n");
        printf("3. Fine Collection Report\n");
        printf("4. Most Borrowed Book\n");
        printf("5. Back\n");

        choice = readInt("Enter Choice: ");

        switch (choice) {
            case 1:
                totalIssued = 0;
                totalReturned = 0;

                for (i = 0; i < transactionCount; i++) {
                    if (transactions[i].status == 1)
                        totalIssued++;
                    else
                        totalReturned++;
                }

                printf("Currently Issued: %d\n", totalIssued);
                printf("Returned Transactions: %d\n", totalReturned);
                printf("Total Transactions: %d\n", transactionCount);
                break;

            case 2:
                printf("Total Student Records: %d\n", studentCount);

                for (i = 0; i < studentCount; i++) {
                    if (students[i].active) {
                        printf("%d - %s - %s\n",
                               students[i].id,
                               students[i].name,
                               students[i].department);
                    }
                }
                break;

            case 3:
                totalFine = 0;
                paidFine = 0;

                for (i = 0; i < fineCount; i++) {
                    totalFine += fines[i].amount;

                    if (fines[i].paid) {
                        paidFine += fines[i].amount;
                    }
                }

                printf("Total Fine: Rs. %d\n", totalFine);
                printf("Collected Fine: Rs. %d\n", paidFine);
                printf("Pending Fine: Rs. %d\n", totalFine - paidFine);
                break;

            case 4:
                bestIndex = -1;
                bestCount = 0;

                for (i = 0; i < bookCount; i++) {
                    int count = 0;

                    if (!books[i].active)
                        continue;

                    for (j = 0; j < transactionCount; j++) {
                        if (transactions[j].bookID == books[i].id) {
                            count++;
                        }
                    }

                    if (count > bestCount) {
                        bestCount = count;
                        bestIndex = i;
                    }
                }

                if (bestIndex == -1 || bestCount == 0) {
                    printf("No borrowing records available!\n");
                } else {
                    printf("Most Borrowed Book: %s\n",
                           books[bestIndex].title);
                    printf("Times Borrowed: %d\n", bestCount);
                }
                break;

            case 5:
                break;

            default:
                printf("Invalid choice!\n");
        }

    } while (choice != 5);
}

// =================================================
// 8. ACTIVITY LOG
// =================================================

void activityLog(void) {
    FILE *fp;
    char line[300];

    fp = fopen("activity.txt", "r");

    if (fp == NULL) {
        printf("No activity log found!\n");
        return;
    }

    printf("\n========== ACTIVITY LOG ==========\n");

    while (fgets(line, sizeof(line), fp) != NULL) {
        printf("%s", line);
    }

    fclose(fp);
}

// =================================================
// 9. BACKUP & RESTORE
// =================================================

void backupRestore(void) {
    int choice;

    do {
        printf("\n========== BACKUP & RESTORE ==========\n");
        printf("1. Create Backup\n");
        printf("2. Restore Backup\n");
        printf("3. Back\n");

        choice = readInt("Enter Choice: ");

        switch (choice) {
            case 1:
                backupData();
                break;

            case 2:
                printf("WARNING: Current data will be replaced!\n");

                if (readInt("Enter 1 to confirm: ") == 1) {
                    restoreData();
                } else {
                    printf("Restore cancelled.\n");
                }
                break;

            case 3:
                break;

            default:
                printf("Invalid choice!\n");
        }

    } while (choice != 3);
}

// =================================================
// STUDENT PORTAL AND ADMIN REQUEST MANAGEMENT
// =================================================

void submitRequest(int sid, int type, int itemID, int slot, char message[]) {
    if (requestCount >= MAX_REQUESTS) {
        printf("Request storage full!\n");
        return;
    }

    requests[requestCount].id = requestCount + 1;
    requests[requestCount].studentID = sid;
    requests[requestCount].type = type;
    requests[requestCount].itemID = itemID;
    requests[requestCount].slot = slot;
    strcpy(requests[requestCount].message, message);
    requests[requestCount].status = 0;

    requestCount++;

    logActivity("New student request received");
    saveData();

    printf("Request submitted! Your Request ID is %d\n",
           requests[requestCount - 1].id);
}

void viewMyRequests(int sid) {
    int i, found = 0;

    printf("\n========== MY REQUESTS ==========\n");

    for (i = 0; i < requestCount; i++) {
        if (requests[i].studentID == sid) {
            printf("\nRequest ID: %d\n", requests[i].id);

            printf("Type: %s\n",
                   requests[i].type == 1 ? "Book Issue" :
                   "Room/Lab Booking");

            printf("Details: %s\n", requests[i].message);

            printf("Status: %s\n",
                   requests[i].status == 0 ? "Pending" :
                   requests[i].status == 1 ? "Approved" :
                   "Rejected");

            found = 1;
        }
    }

    if (!found)
        printf("No book or room requests found.\n");

    found = 0;

    printf("\n========== MY COMPLAINTS ==========\n");

    for (i = 0; i < complaintCount; i++) {
        if (complaints[i].studentID == sid) {
            printf("\nComplaint ID: %d\n", complaints[i].id);
            printf("Message: %s\n", complaints[i].message);
            printf("Status: %s\n", complaints[i].status);

            if (strcmp(complaints[i].status, "Rejected") == 0) {
                printf("Rejection Reason: %s\n", complaints[i].reason);
            }

            found = 1;
        }
    }

    if (!found)
        printf("No complaints found.\n");
}

// ---------- New student registration ----------
// Returns index of the new student, or -1 if registration failed.

int registerStudent(void) {
    int i, newID = 1001, index, semester;
    char name[100];
    char department[50];
    char contact[30];
    char password[30];
    char confirm[30];

    if (studentCount >= MAX_STUDENTS) {
        printf("Student storage full! Contact admin.\n");
        return -1;
    }

    printf("\n========== NEW STUDENT REGISTRATION ==========\n");

    // next free ID (never reuses an old ID)
    for (i = 0; i < studentCount; i++) {
        if (students[i].id >= newID) {
            newID = students[i].id + 1;
        }
    }

    readLine("Enter Name: ", name, 100);

    if (strlen(name) == 0) {
        printf("Name cannot be empty!\n");
        return -1;
    }

    readLine("Enter Department: ", department, 50);

    if (strlen(department) == 0) {
        printf("Department cannot be empty!\n");
        return -1;
    }

    semester = readInt("Enter Semester (1-12): ");

    if (semester < 1 || semester > 12) {
        printf("Invalid semester!\n");
        return -1;
    }

    readLine("Enter Contact: ", contact, 30);

    readLine("Create Password (min 4 characters): ", password, 30);

    if (strlen(password) < 4) {
        printf("Password too short!\n");
        return -1;
    }

    readLine("Confirm Password: ", confirm, 30);

    if (strcmp(password, confirm) != 0) {
        printf("Passwords do not match!\n");
        return -1;
    }

    index = studentCount;

    memset(&students[index], 0, sizeof(struct Student));

    students[index].id = newID;
    strcpy(students[index].name, name);
    strcpy(students[index].department, department);
    students[index].semester = semester;
    strcpy(students[index].contact, contact);
    strcpy(students[index].password, password);
    students[index].active = 1;

    studentCount++;

    logActivity("New student registered from student portal");
    saveData();

    printf("\nRegistration successful!\n");
    printf("Your Student ID is: %d\n", newID);
    printf("Please remember your ID and password for next login.\n");

    return index;
}

// ---------- Student login ----------
// Returns index of the logged in student, or -1 if login failed.

int studentLogin(void) {
    int sid, index;
    int attempts = 3;
    char password[30];

    while (attempts > 0) {
        printf("\n========== STUDENT LOGIN ==========\n");

        sid = readInt("Enter Student ID: ");
        readLine("Enter Password: ", password, 30);

        index = findStudent(sid);

        if (index != -1 &&
            strcmp(students[index].password, password) == 0) {

            printf("\nLogin Successful!\n");

            logActivity("Student logged in");

            return index;
        }

        attempts--;

        printf("Incorrect Student ID or password!\n");
        printf("Attempts remaining: %d\n", attempts);
    }

    printf("Login failed!\n");
    return -1;
}

// ---------- Student Dashboard ----------

void studentDashboard(int studentIndex) {
    int sid = students[studentIndex].id;
    int i, j;
    int issuedCount = 0;
    int pendingFine = 0;
    int pendingRequests = 0;
    int activeRoomBookings = 0;
    int bookIndex;

    printf("\n==================================================\n");
    printf("                 STUDENT DASHBOARD\n");
    printf("==================================================\n");

    printf("Welcome, %s!\n", students[studentIndex].name);
    printf("Student ID : %d\n", sid);
    printf("Department : %s\n", students[studentIndex].department);
    printf("Semester   : %d\n", students[studentIndex].semester);
    printf("Contact    : %s\n", students[studentIndex].contact);

    printf("\n---------------- LIBRARY SUMMARY ----------------\n");

    for (i = 0; i < transactionCount; i++) {
        if (transactions[i].studentID == sid &&
            transactions[i].status == 1) {
            issuedCount++;
        }
    }

    for (i = 0; i < fineCount; i++) {
        if (fines[i].studentID == sid && fines[i].paid == 0) {
            pendingFine += fines[i].amount;
        }
    }

    for (i = 0; i < requestCount; i++) {
        if (requests[i].studentID == sid && requests[i].status == 0) {
            pendingRequests++;
        }
    }

    for (i = 0; i < roomCount; i++) {
        for (j = 0; j < NUM_SLOTS; j++) {
            if (rooms[i].slotStudent[j] == sid) {
                activeRoomBookings++;
            }
        }
    }

    printf("Currently Issued Books : %d\n", issuedCount);
    printf("Pending Fine           : Rs. %d\n", pendingFine);
    printf("Pending Requests       : %d\n", pendingRequests);
    printf("Room/Lab Bookings      : %d\n", activeRoomBookings);

    printf("\n---------------- ISSUED BOOKS --------------------\n");

    if (issuedCount == 0) {
        printf("You have no currently issued books.\n");
    } else {
        for (i = 0; i < transactionCount; i++) {
            if (transactions[i].studentID == sid &&
                transactions[i].status == 1) {
                bookIndex = findBook(transactions[i].bookID);

                printf("Book ID: %d | ", transactions[i].bookID);

                if (bookIndex != -1) {
                    printf("Title: %s | ", books[bookIndex].title);
                }

                printf("Due Date: ");
                {
                    char due[12];
                    formatDate(transactions[i].dueDate, due);
                    printf("%s\n", due);
                }
            }
        }
    }

    printf("\n---------------- PENDING FINES -------------------\n");

    if (pendingFine == 0) {
        printf("You have no pending fines.\n");
    } else {
        for (i = 0; i < fineCount; i++) {
            if (fines[i].studentID == sid && fines[i].paid == 0) {
                printf("Rs. %d | Reason: %s\n",
                       fines[i].amount, fines[i].reason);
            }
        }
    }

    printf("\n---------------- RECENT REQUESTS -----------------\n");

    if (requestCount == 0 || pendingRequests == 0) {
        printf("No pending requests.\n");
    } else {
        for (i = 0; i < requestCount; i++) {
            if (requests[i].studentID == sid &&
                requests[i].status == 0) {
                printf("Request ID: %d | %s | Pending\n",
                       requests[i].id, requests[i].message);
            }
        }
    }

    printf("==================================================\n");
}

// ---------- Menu shown after student login / registration ----------

void studentMenu(int studentIndex) {
    int sid, choice, id, index;
    char message[200];

    sid = students[studentIndex].id;

    studentDashboard(studentIndex);

    do {
        printf("\n========== STUDENT PORTAL ==========\n");

        printf("Welcome, %s (ID: %d)\n", students[studentIndex].name, sid);

        printf("1. View Available Books\n");
        printf("2. Request a Book\n");
        printf("3. Request Room/Lab Booking\n");
        printf("4. Submit Complaint/Suggestion\n");
        printf("5. View My Request Status\n");
        printf("6. Logout\n");

        choice = readInt("Enter Choice: ");

        switch (choice) {

            case 1:
                displayBooks();
                break;

            case 2:
                id = readInt("Enter Book ID: ");

                index = findBook(id);

                if (index == -1 ||
                    books[index].availableCopies <= 0) {

                    printf("Book is not available.\n");
                    break;
                }

                sprintf(message, "Book request: %s",
                        books[index].title);

                submitRequest(sid, 1, id, 0, message);
                break;

            case 3: {
                int r, slot, k;
                char label[30];

                viewRooms(0);

                id = readInt("Enter Room/Lab ID: ");
                r = findRoom(id);

                if (r == -1) {
                    printf("Room/Lab not found.\n");
                    break;
                }

                printRoomSlots(r, 0);
                slot = readSlot("Enter Time Slot (e.g. 09:00 - 10:00): ");

                if (slot == -1) {
                    break;
                }

                if (rooms[r].slotStudent[slot] != 0) {
                    printf("This slot is already booked.\n");
                    break;
                }

                // check if same pending request already exists
                for (k = 0; k < requestCount; k++) {
                    if (requests[k].type == 2 && requests[k].status == 0 &&
                        requests[k].studentID == sid &&
                        requests[k].itemID == id &&
                        requests[k].slot == slot) {
                        printf("You already requested this slot.\n");
                        break;
                    }
                }

                if (k < requestCount) break;

                slotLabel(slot, label);
                sprintf(message, "Room/Lab request: %s (%s)",
                        rooms[r].name, label);

                submitRequest(sid, 2, id, slot, message);
                break;
            }

            case 4:
                readLine("Enter complaint/suggestion: ",
                         message, 200);

                submitComplaint(sid, message);
                break;

            case 5:
                viewMyRequests(sid);
                break;

            case 6:
                printf("Student logged out.\n");
                break;

            default:
                printf("Invalid choice!\n");
        }

    } while (choice != 6);
}

// ---------- Student portal entry: register or login ----------

void studentPortal(void) {
    int choice, studentIndex;

    do {
        printf("\n========== STUDENT PORTAL ==========\n");
        printf("1. New Student Registration\n");
        printf("2. Student Login\n");
        printf("3. Back\n");

        choice = readInt("Enter Choice: ");

        switch (choice) {
            case 1:
                studentIndex = registerStudent();

                if (studentIndex != -1) {
                    studentMenu(studentIndex);
                }
                break;

            case 2:
                studentIndex = studentLogin();

                if (studentIndex != -1) {
                    studentMenu(studentIndex);
                }
                break;

            case 3:
                break;

            default:
                printf("Invalid choice!\n");
        }

    } while (choice != 3);
}

void showRequests(int pendingOnly) {
    int i, found = 0;
    char label[30];

    printf("\n========== STUDENT REQUESTS ==========\n");

    for (i = 0; i < requestCount; i++) {
        if (pendingOnly && requests[i].status != 0)
            continue;

        printf("\nRequest ID: %d | Student ID: %d\n",
               requests[i].id,
               requests[i].studentID);

        printf("Type: %s\n",
               requests[i].type == 1 ? "Book Issue" :
               "Room/Lab Booking");

        printf("Related ID: %d\n", requests[i].itemID);

        if (requests[i].type == 2) {
            slotLabel(requests[i].slot, label);
            printf("Time Slot: %s\n", label);
        }

        printf("Details: %s\n", requests[i].message);

        printf("Status: %s\n",
               requests[i].status == 0 ? "Pending" :
               requests[i].status == 1 ? "Approved" :
               "Rejected");

        found = 1;
    }

    if (!found)
        printf("No matching requests.\n");
}

void processRequest(void) {
    int id, i, decision, index, issueDate, dueDate, slot;

    id = readInt("Enter Request ID: ");

    for (i = 0; i < requestCount; i++) {
        if (requests[i].id == id)
            break;
    }

    if (i == requestCount) {
        printf("Request not found.\n");
        return;
    }

    if (requests[i].status != 0) {
        printf("Request already processed.\n");
        return;
    }

    printf("1. Approve\n");
    printf("2. Reject\n");

    decision = readInt("Enter choice: ");

    if (decision == 2) {
        requests[i].status = 2;

        logActivity("Student request rejected");
        saveData();

        printf("Request rejected.\n");
        return;
    }

    if (decision != 1) {
        printf("Invalid choice.\n");
        return;
    }

    if (requests[i].type == 1) {

        index = findBook(requests[i].itemID);

        if (index == -1 || books[index].availableCopies <= 0) {
            printf("Book is no longer available. Request remains pending.\n");
            return;
        }

        if (transactionCount >= MAX_TRANSACTIONS) {
            printf("Transaction storage full.\n");
            return;
        }

        issueDate = readDate("Enter Issue Date (DD-MM-YYYY): ");

        if (issueDate == 0) {
            printf("Request remains pending.\n");
            return;
        }

        dueDate = readDate("Enter Due Date (DD-MM-YYYY): ");

        if (dueDate == 0) {
            printf("Request remains pending.\n");
            return;
        }

        if (dueDate < issueDate) {
            printf("Due date cannot be before issue date. Request remains pending.\n");
            return;
        }

        transactions[transactionCount].studentID =
            requests[i].studentID;

        transactions[transactionCount].bookID =
            requests[i].itemID;

        transactions[transactionCount].issueDate = issueDate;
        transactions[transactionCount].dueDate = dueDate;
        transactions[transactionCount].returnDate = 0;
        transactions[transactionCount].status = 1;

        transactionCount++;
        books[index].availableCopies--;

    } else if (requests[i].type == 2) {

        index = findRoom(requests[i].itemID);
        slot = requests[i].slot;

        if (index == -1) {
            printf("Room/Lab not found. You can reject this request.\n");
            return;
        }

        if (rooms[index].slotStudent[slot] != 0) {
            printf("This slot is already booked! Request remains pending, you can reject it.\n");
            return;
        }

        rooms[index].slotStudent[slot] = requests[i].studentID;
    }

    requests[i].status = 1;

    logActivity("Student request approved");
    saveData();

    printf("Request approved successfully.\n");
}

void requestManagement(void) {
    int choice;

    do {
        printf("\n========== STUDENT REQUEST MANAGEMENT ==========\n");
        printf("1. View All Requests\n");
        printf("2. View Pending Requests\n");
        printf("3. Approve / Reject Request\n");
        printf("4. Back\n");

        choice = readInt("Enter Choice: ");

        switch (choice) {
            case 1: showRequests(0); break;
            case 2: showRequests(1); break;
            case 3: processRequest(); break;
            case 4: break;
            default: printf("Invalid choice!\n");
        }

    } while (choice != 4);
}

// =================================================
// ADMIN LOGIN
// =================================================

int adminLogin(void) {
    char username[50];
    char password[50];
    int attempts = 3;

    while (attempts > 0) {
        printf("\n========== ADMIN LOGIN ==========\n");

        readLine("Username: ", username, 50);
        readLine("Password: ", password, 50);

        if (strcmp(username, "admin") == 0 &&
            strcmp(password, "1234") == 0) {

            printf("\nLogin Successful!\n");

            logActivity("Admin logged in");
            saveData();

            return 1;
        }

        attempts--;

        printf("Incorrect username or password!\n");
        printf("Attempts remaining: %d\n", attempts);
    }

    printf("Login failed!\n");
    return 0;
}

// =================================================
// MAIN ADMIN PANEL
// =================================================

int main(void) {
    int role, choice;

    loadData();
    saveData();

    do {
        printf("\n========== LIBRARY MANAGEMENT SYSTEM ==========\n");
        printf("1. Admin Login\n");
        printf("2. Student Portal\n");
        printf("3. Exit\n");

        role = readInt("Select Portal: ");

        if (role == 1) {

            if (!adminLogin())
                continue;

            do {
                printf("\n============================================\n");
                printf("       LIBRARY MANAGEMENT SYSTEM - ADMIN\n");
                printf("============================================\n");

                printf("1. BOOK ADMINISTRATION\n");
                printf("2. STUDENT ADMINISTRATION\n");
                printf("3. ROOM & LAB MONITORING\n");
                printf("4. PENALTY & FINE MANAGEMENT\n");
                printf("5. COMPLAINT & SUGGESTION MANAGEMENT\n");
                printf("6. SYSTEM MONITORING\n");
                printf("7. REPORT & ANALYTICS\n");
                printf("8. ACTIVITY LOG\n");
                printf("9. BACKUP & RESTORE DATA\n");
                printf("10. STUDENT REQUEST MANAGEMENT\n");
                printf("11. LOGOUT\n");

                choice = readInt("Enter Choice: ");

                switch (choice) {
                    case 1: bookAdministration(); break;
                    case 2: studentAdministration(); break;
                    case 3: roomMonitoring(); break;
                    case 4: fineManagement(); break;
                    case 5: complaintManagement(); break;
                    case 6: systemMonitoring(); break;
                    case 7: reportsAnalytics(); break;
                    case 8: activityLog(); break;
                    case 9: backupRestore(); break;
                    case 10: requestManagement(); break;

                    case 11:
                        logActivity("Admin logged out");
                        saveData();

                        printf("Admin logged out successfully!\n");
                        break;

                    default:
                        printf("Invalid choice!\n");
                }

            } while (choice != 11);

        } else if (role == 2) {

            studentPortal();

        } else if (role == 3) {

            printf("Thank you for using Library Management System.\n");

        } else {

            printf("Invalid choice!\n");
        }

    } while (role != 3);

    return 0;
}
