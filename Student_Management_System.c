#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STUDENT_FILE "students.txt"
#define CREDENTIAL_FILE "credentials.txt"
#define TEMP_FILE "temp.txt"

struct student {
    int roll;
    char name[100];
    float marks;
};

char currentRole[32];
char currentUser[64];

void strip_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len-1] == '\n') s[len-1] = '\0';
}

void create_default_credentials_if_missing(void) {
    FILE *fp = fopen(CREDENTIAL_FILE, "r");
    if (fp) { fclose(fp); return; }
    fp = fopen(CREDENTIAL_FILE, "w");
    if (!fp) return;
    fprintf(fp, "admin admin ADMIN\n");
    fclose(fp);
}

int loginSystem(void) {
    char username[64], password[64];
    char fileUser[64], filePass[64], fileRole[32];

    create_default_credentials_if_missing();

    printf("============= Login Screen =============\n");
    printf("Username: ");
    if (scanf("%63s", username) != 1) return 0;
    printf("Password: ");
    if (scanf("%63s", password) != 1) return 0;

    FILE *fp = fopen(CREDENTIAL_FILE, "r");
    if (!fp) return 0;

    int success = 0;
    while (fscanf(fp, "%63s %63s %31s", fileUser, filePass, fileRole) == 3) {
        if (strcmp(username, fileUser) == 0 && strcmp(password, filePass) == 0) {
            strncpy(currentRole, fileRole, sizeof(currentRole)-1);
            currentRole[sizeof(currentRole)-1] = '\0';
            strncpy(currentUser, fileUser, sizeof(currentUser)-1);
            currentUser[sizeof(currentUser)-1] = '\0';
            success = 1;
            break;
        }
    }
    fclose(fp);
    return success;
}

void addStudent(void) {
    FILE *fp = fopen(STUDENT_FILE, "a");
    if (!fp) { printf("Unable to open student file for appending.\n"); return; }

    struct student st;

    printf("Enter Roll: ");
    if (scanf("%d", &st.roll) != 1) { printf("Invalid roll.\n"); fclose(fp); return; }
    getchar(); // consume leftover newline

    printf("Enter Name: ");
    if (!fgets(st.name, sizeof(st.name), stdin)) { printf("Failed to read name.\n"); fclose(fp); return; }
    strip_newline(st.name);

    printf("Enter Marks: ");
    if (scanf("%f", &st.marks) != 1) { printf("Invalid marks.\n"); fclose(fp); return; }

    fprintf(fp, "%d\t%s\t%.2f\n", st.roll, st.name, st.marks);
    fclose(fp);

    printf("Student Added Successfully!\n");
}

int parse_student_line(const char *line, int *out_roll, char *out_name, size_t name_size, float *out_marks) {
    char tmp[256];
    strncpy(tmp, line, sizeof(tmp)-1);
    tmp[sizeof(tmp)-1] = '\0';
    strip_newline(tmp);

    char *first_tab = strchr(tmp, '\t');
    if (!first_tab) return 0;
    *first_tab = '\0';
    if (sscanf(tmp, "%d", out_roll) != 1) return 0;

    char *second_tab = strchr(first_tab + 1, '\t');
    if (!second_tab) return 0;
    *second_tab = '\0';

    strncpy(out_name, first_tab + 1, name_size-1);
    out_name[name_size-1] = '\0';

    if (sscanf(second_tab + 1, "%f", out_marks) != 1) return 0;

    return 1;
}

void displayStudent(void) {
    FILE *fp = fopen(STUDENT_FILE, "r");
    if (!fp) {
        printf("No records found.\n");
        return;
    }

    char line[512];
    printf("\nRoll\tName\t\tMarks\n");
    printf("-----------------------------------\n");

    while (fgets(line, sizeof(line), fp)) {
        int roll;
        char name[100];
        float marks;
        if (parse_student_line(line, &roll, name, sizeof(name), &marks)) {
            printf("%d\t%-15s\t%.2f\n", roll, name, marks);
        } else {
            //ignore
        }
    }

    fclose(fp);
}

void searchStudent(void) {
    FILE *fp = fopen(STUDENT_FILE, "r");
    if (!fp) {
        printf("No student records.\n");
        return;
    }

    int roll;
    printf("Enter Roll to search: ");
    if (scanf("%d", &roll) != 1) { printf("Invalid input.\n"); fclose(fp); return; }

    char line[512];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        int r;
        char name[100];
        float marks;
        if (!parse_student_line(line, &r, name, sizeof(name), &marks)) continue;

        if (r == roll) {
            printf("\nRecord Found:\n");
            printf("Roll: %d\nName: %s\nMarks: %.2f\n", r, name, marks);
            found = 1;
            break;
        }
    }

    if (!found) printf("Record not found.\n");

    fclose(fp);
}

void updateStudent(void) {
    FILE *fp = fopen(STUDENT_FILE, "r");
    if (!fp) { printf("No student file.\n"); return; }

    FILE *temp = fopen(TEMP_FILE, "w");
    if (!temp) { fclose(fp); printf("Unable to open temp file.\n"); return; }

    int roll;
    printf("Enter Roll to update: ");
    if (scanf("%d", &roll) != 1) { printf("Invalid input.\n"); fclose(fp); fclose(temp); return; }
    getchar();

    char line[512];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        int r;
        char name[100];
        float marks_old;
        if (!parse_student_line(line, &r, name, sizeof(name), &marks_old)) continue;

        if (r == roll) {
            found = 1;
            char newname[100];
            float newmarks;
            printf("Enter new name: ");
            if (!fgets(newname, sizeof(newname), stdin)) { strcpy(newname, name); }
            strip_newline(newname);
            printf("Enter new marks: ");
            if (scanf("%f", &newmarks) != 1) newmarks = marks_old;
            getchar();
            fprintf(temp, "%d\t%s\t%.2f\n", r, newname, newmarks);
        } else {
            fprintf(temp, "%d\t%s\t%.2f\n", r, name, marks_old);
        }
    }

    fclose(fp);
    fclose(temp);

    if (found) {
        remove(STUDENT_FILE);
        rename(TEMP_FILE, STUDENT_FILE);
        printf("Record updated successfully.\n");
    } else {
        remove(TEMP_FILE);
        printf("Roll not found.\n");
    }
}

void deleteStudent(void) {
    FILE *fp = fopen(STUDENT_FILE, "r");
    if (!fp) { printf("No student file.\n"); return; }

    FILE *temp = fopen(TEMP_FILE, "w");
    if (!temp) { fclose(fp); printf("Unable to open temp file.\n"); return; }

    int roll;
    printf("Enter Roll to delete: ");
    if (scanf("%d", &roll) != 1) { printf("Invalid input.\n"); fclose(fp); fclose(temp); return; }

    char line[512];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        int r;
        char name[100];
        float marks;
        if (!parse_student_line(line, &r, name, sizeof(name), &marks)) continue;

        if (r == roll) {
            found = 1;
            continue;
        } else {
            fprintf(temp, "%d\t%s\t%.2f\n", r, name, marks);
        }
    }

    fclose(fp);
    fclose(temp);

    if (found) {
        remove(STUDENT_FILE);
        rename(TEMP_FILE, STUDENT_FILE);
        printf("Record deleted successfully.\n");
    } else {
        remove(TEMP_FILE);
        printf("Roll not found.\n");
    }
}

void changePassword(void) {
    char oldpass[64], newpass[64], confirm[64];
    char fileUser[64], filePass[64], fileRole[32];

    printf("Enter old password: ");
    if (scanf("%63s", oldpass) != 1) return;

    FILE *fp = fopen(CREDENTIAL_FILE, "r");
    FILE *temp = fopen(TEMP_FILE, "w");
    if (!fp || !temp) { if (fp) fclose(fp); if (temp) fclose(temp); printf("Credential file error.\n"); return; }

    int ok = 0;
    while (fscanf(fp, "%63s %63s %31s", fileUser, filePass, fileRole) == 3) {
        if (strcmp(fileUser, currentUser) == 0) {
            if (strcmp(filePass, oldpass) != 0) {
                printf("Incorrect password.\n");
                fclose(fp);
                fclose(temp);
                remove(TEMP_FILE);
                return;
            }
            ok = 1;
            printf("Enter new password: ");
            if (scanf("%63s", newpass) != 1) { fclose(fp); fclose(temp); remove(TEMP_FILE); return; }
            printf("Confirm new password: ");
            if (scanf("%63s", confirm) != 1) { fclose(fp); fclose(temp); remove(TEMP_FILE); return; }
            if (strcmp(newpass, confirm) != 0) {
                printf("Passwords do not match.\n");
                fclose(fp);
                fclose(temp);
                remove(TEMP_FILE);
                return;
            }
            fprintf(temp, "%s %s %s\n", fileUser, newpass, fileRole);
        } else {
            fprintf(temp, "%s %s %s\n", fileUser, filePass, fileRole);
        }
    }

    fclose(fp);
    fclose(temp);

    if (!ok) {
        printf("User not found.\n");
        remove(TEMP_FILE);
        return;
    }

    remove(CREDENTIAL_FILE);
    rename(TEMP_FILE, CREDENTIAL_FILE);

    printf("Password changed successfully.\n");
}

void adminMenu() {
    int ch;
    while (1) {
        printf("\n===== ADMIN MENU =====\n");
        printf("1. Add Student\n");
        printf("2. Display Students\n");
        printf("3. Search Student\n");
        printf("4. Update Student\n");
        printf("5. Delete Student\n");
        printf("6. Change Password\n");
        printf("7. Logout\n");
        printf("Enter choice: ");
        if (scanf("%d", &ch) != 1) { getchar(); continue; }

        switch(ch) {
            case 1: addStudent(); break;
            case 2: displayStudent(); break;
            case 3: searchStudent(); break;
            case 4: updateStudent(); break;
            case 5: deleteStudent(); break;
            case 6: changePassword(); break;
            case 7: return;
            default: printf("Invalid!\n");
        }
    }
}

void staffMenu() {
    int ch;
    while (1) {
        printf("\n===== STAFF MENU =====\n");
        printf("1. Display Students\n");
        printf("2. Search Student\n");
        printf("3. Change Password\n");
        printf("4. Logout\n");
        printf("Enter choice: ");
        if (scanf("%d", &ch) != 1) { getchar(); continue; }

        switch(ch) {
            case 1: displayStudent(); break;
            case 2: searchStudent(); break;
            case 3: changePassword(); break;
            case 4: return;
            default: printf("Invalid!\n");
        }
    }
}

void guestMenu() {
    int ch;
    while (1) {
        printf("\n===== GUEST MENU =====\n");
        printf("1. Display Students\n");
        printf("2. Logout\n");
        if (scanf("%d", &ch) != 1) { getchar(); continue; }

        switch(ch) {
            case 1: displayStudent(); break;
            case 2: return;
            default: printf("Invalid!\n");
        }
    }
}

void mainMenu(void) {
    if (strcmp(currentRole, "ADMIN") == 0) adminMenu();
    else if (strcmp(currentRole, "STAFF") == 0) staffMenu();
    else guestMenu();
}

int main() {
    if (loginSystem()) mainMenu();
    else printf("Login Failed.\n");
    return 0;
}