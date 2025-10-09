#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

extern int _fileno(FILE *stream);

extern const char *tools_file;
extern int ci_strcmp(const char *a, const char *b);
extern FILE *ReadFile(void);
extern void DisplayFile(void);
extern int similarity(const char *a, const char *b);
extern void OrderIDPlus1(char *newID);
extern void AddOrder(void);
extern void SearchOrder(void);
extern void DeleteOrder(void);
extern int Menu(void);
extern int run_app(void);

static const char *unit_csv = "tools_unit.csv";
static const char *stdin_path = "unit_stdin.txt";
static const char *stdout_path = "unit_stdout.txt";
static char *original_csv_content;
static int original_csv_exists;
static const char *original_csv_name;
static int saved_stdin_fd = -1;
static int saved_stdout_fd = -1;

static int write_text_file(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    if (!f) return 0;
    if (content && *content) fputs(content, f);
    fclose(f);
    return 1;
}

static char *read_text_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long len = ftell(f);
    if (len < 0) {
        fclose(f);
        return NULL;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }
    char *buf = (char *)malloc((size_t)len + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    size_t got = fread(buf, 1, (size_t)len, f);
    buf[got] = '\0';
    fclose(f);
    return buf;
}

static int file_exists(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static int str_contains(const char *haystack, const char *needle) {
    if (!haystack || !needle) return 0;
    return strstr(haystack, needle) != NULL;
}

static int redirect_stdin_to(const char *path) {
    if (saved_stdin_fd != -1) {
        _dup2(saved_stdin_fd, _fileno(stdin));
        _close(saved_stdin_fd);
        saved_stdin_fd = -1;
    }
    saved_stdin_fd = _dup(_fileno(stdin));
    int file = _open(path, _O_RDONLY);
    if (file == -1) return 0;
    _dup2(file, _fileno(stdin));
    _close(file);
    return 1;
}

static int redirect_stdout_to(const char *path) {
    fflush(stdout);
    if (saved_stdout_fd != -1) {
        _dup2(saved_stdout_fd, _fileno(stdout));
        _close(saved_stdout_fd);
        saved_stdout_fd = -1;
    }
    saved_stdout_fd = _dup(_fileno(stdout));
    int file = _open(path, _O_CREAT | _O_TRUNC | _O_WRONLY, _S_IREAD | _S_IWRITE);
    if (file == -1) return 0;
    _dup2(file, _fileno(stdout));
    _close(file);
    return 1;
}

static void restore_stdio(void) {
    if (saved_stdin_fd != -1) {
        _dup2(saved_stdin_fd, _fileno(stdin));
        _close(saved_stdin_fd);
        saved_stdin_fd = -1;
    }
    if (saved_stdout_fd != -1) {
        fflush(stdout);
        _dup2(saved_stdout_fd, _fileno(stdout));
        _close(saved_stdout_fd);
        saved_stdout_fd = -1;
    }
}

static void reset_unit_state(void) {
    remove(unit_csv);
    remove(stdin_path);
    remove(stdout_path);
    remove("temp.csv");
    tools_file = unit_csv;
}

static int prepare_csv(const char *content) {
    if (!write_text_file(unit_csv, content ? content : "")) return 0;
    tools_file = unit_csv;
    return 1;
}

static int test_ci_strcmp(void) {
    if (ci_strcmp("abc", "ABC") != 0) return 0;
    if (ci_strcmp("abc", "abd") >= 0) return 0;
    if (ci_strcmp("abd", "abc") <= 0) return 0;
    if (ci_strcmp(NULL, NULL) != 0) return 0;
    if (ci_strcmp(NULL, "x") != -1) return 0;
    if (ci_strcmp("x", NULL) != 1) return 0;
    return 1;
}

static int test_ReadFile(void) {
    if (!prepare_csv("T001,Hammer,5,100\n")) return 0;
    FILE *f = ReadFile();
    if (!f) return 0;
    char line[64];
    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return str_contains(line, "Hammer");
}

static int test_DisplayFile(void) {
    if (!prepare_csv("T001,Hammer,5,100\nT002,Saw,3,70\n")) return 0;
    if (!redirect_stdout_to(stdout_path)) return 0;
    DisplayFile();
    restore_stdio();
    char *out = read_text_file(stdout_path);
    if (!out) return 0;
    int ok = str_contains(out, "T001,Hammer,5,100") && str_contains(out, "T002,Saw,3,70");
    free(out);
    return ok;
}

static int test_similarity(void) {
    if (similarity("Hammer", "hammer") != 1) return 0;
    if (similarity("Drill", "Drilx") != 1) return 0;
    if (similarity("Saw", "Hammer") != 0) return 0;
    if (similarity("", "Hammer") != 0) return 0;
    return 1;
}

static int test_OrderIDPlus1(void) {
    char id[10];
    remove(unit_csv);
    tools_file = unit_csv;
    OrderIDPlus1(id);
    if (strcmp(id, "T001") != 0) return 0;
    if (!prepare_csv("T001,Hammer,5,100\nT010,Saw,3,70\n")) return 0;
    OrderIDPlus1(id);
    return strcmp(id, "T011") == 0;
}

static int test_AddOrder_new(void) {
    if (!prepare_csv("T001,Hammer,5,100\n")) return 0;
    if (!write_text_file(stdin_path, "\nDrill\n4\n200\nY\n")) return 0;
    if (!redirect_stdin_to(stdin_path)) return 0;
    if (!redirect_stdout_to(stdout_path)) return 0;
    AddOrder();
    restore_stdio();
    char *csv = read_text_file(unit_csv);
    if (!csv) return 0;
    int ok = str_contains(csv, "Drill,4,200");
    free(csv);
    return ok;
}

static int test_AddOrder_merge(void) {
    if (!prepare_csv("T001,Hammer,5,100\n")) return 0;
    if (!write_text_file(stdin_path, "\nHammer\n3\n150\nY\nY\n")) return 0;
    if (!redirect_stdin_to(stdin_path)) return 0;
    if (!redirect_stdout_to(stdout_path)) return 0;
    AddOrder();
    restore_stdio();
    char *csv = read_text_file(unit_csv);
    if (!csv) return 0;
    int ok = str_contains(csv, "T001,Hammer,8,150");
    free(csv);
    return ok;
}

static int test_SearchOrder_exact(void) {
    if (!prepare_csv("T001,Hammer,5,100\nT002,Saw,3,70\n")) return 0;
    if (!write_text_file(stdin_path, "\nT002\n")) return 0;
    if (!redirect_stdin_to(stdin_path)) return 0;
    if (!redirect_stdout_to(stdout_path)) return 0;
    SearchOrder();
    restore_stdio();
    char *out = read_text_file(stdout_path);
    if (!out) return 0;
    int ok = str_contains(out, "Order ID: T002") && str_contains(out, "Saw");
    free(out);
    return ok;
}

static int test_SearchOrder_similar(void) {
    if (!prepare_csv("T001,Screwdriver,5,20\n")) return 0;
    if (!write_text_file(stdin_path, "\nScrewdrivr\nY\n")) return 0;
    if (!redirect_stdin_to(stdin_path)) return 0;
    if (!redirect_stdout_to(stdout_path)) return 0;
    SearchOrder();
    restore_stdio();
    char *out = read_text_file(stdout_path);
    if (!out) return 0;
    int ok = str_contains(out, "Screwdriver");
    free(out);
    return ok;
}

static int test_DeleteOrder(void) {
    if (!prepare_csv("T001,Hammer,5,100\nT002,Saw,3,70\n")) return 0;
    if (!write_text_file(stdin_path, "T001\nY\nY\n")) return 0;
    if (!redirect_stdin_to(stdin_path)) return 0;
    if (!redirect_stdout_to(stdout_path)) return 0;
    DeleteOrder();
    restore_stdio();
    char *csv = read_text_file(unit_csv);
    if (!csv) return 0;
    int ok = !str_contains(csv, "T001,Hammer") && str_contains(csv, "T002,Saw");
    free(csv);
    return ok;
}

static int test_Menu(void) {
    if (!write_text_file(stdin_path, "3\n")) return 0;
    if (!redirect_stdin_to(stdin_path)) return 0;
    int choice = Menu();
    restore_stdio();
    return choice == 3;
}

static int test_run_app(void) {
    if (!prepare_csv("T001,Hammer,5,100\n")) return 0;
    if (!write_text_file(stdin_path, "1\n5\n")) return 0;
    if (!redirect_stdin_to(stdin_path)) return 0;
    if (!redirect_stdout_to(stdout_path)) return 0;
    int rc = run_app();
    restore_stdio();
    char *out = read_text_file(stdout_path);
    if (!out) return 0;
    int ok = rc == 0 && str_contains(out, "Hammer") && str_contains(out, "Exiting");
    free(out);
    return ok;
}

static int run_test(const char *name, int (*fn)(void)) {
    reset_unit_state();
    int ok = fn();
    if (ok) {
        printf("[PASS] %s\n", name);
    } else {
        printf("[FAIL] %s\n", name);
    }
    return ok;
}

int main(void) {
    original_csv_name = tools_file;
    original_csv_exists = file_exists(original_csv_name);
    original_csv_content = original_csv_exists ? read_text_file(original_csv_name) : NULL;
    int all_ok = 1;
    all_ok &= run_test("ci_strcmp", test_ci_strcmp);
    all_ok &= run_test("ReadFile", test_ReadFile);
    all_ok &= run_test("DisplayFile", test_DisplayFile);
    all_ok &= run_test("similarity", test_similarity);
    all_ok &= run_test("OrderIDPlus1", test_OrderIDPlus1);
    all_ok &= run_test("AddOrder new", test_AddOrder_new);
    all_ok &= run_test("AddOrder merge", test_AddOrder_merge);
    all_ok &= run_test("SearchOrder exact", test_SearchOrder_exact);
    all_ok &= run_test("SearchOrder similar", test_SearchOrder_similar);
    all_ok &= run_test("DeleteOrder", test_DeleteOrder);
    all_ok &= run_test("Menu", test_Menu);
    all_ok &= run_test("run_app", test_run_app);
    tools_file = original_csv_name;
    remove(unit_csv);
    remove(stdin_path);
    remove(stdout_path);
    remove("temp.csv");
    if (original_csv_exists) {
        write_text_file(original_csv_name, original_csv_content ? original_csv_content : "");
    } else {
        remove(original_csv_name);
    }
    free(original_csv_content);
    if (all_ok) {
        printf("UNIT TESTS PASSED\n");
        return 0;
    } else {
        printf("UNIT TESTS FAILED\n");
        return 1;
    }
}
