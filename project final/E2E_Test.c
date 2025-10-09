#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

extern int _fileno(FILE *stream);

extern const char *tools_file;
extern int run_app(void);

static const char *e2e_csv = "tools_e2e.csv";
static const char *stdin_path = "e2e_stdin.txt";
static const char *stdout_path = "e2e_stdout.txt";
static const char *original_csv_name;
static char *original_csv_content;
static int original_csv_exists;
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

static void reset_e2e_state(void) {
    remove(e2e_csv);
    remove(stdin_path);
    remove(stdout_path);
    remove("temp.csv");
    tools_file = e2e_csv;
}

static int prepare_csv(const char *content) {
    if (!write_text_file(e2e_csv, content ? content : "")) return 0;
    tools_file = e2e_csv;
    return 1;
}

static int test_full_flow(void) {
    const char *initial_csv =
        "T001,Hammer,5,100\n"
        "T002,Saw,3,70\n";
    if (!prepare_csv(initial_csv)) return 0;
    const char *input =
        "1\n"
        "2\n"
        "Wrench\n"
        "6\n"
        "80\n"
        "Y\n"
        "3\n"
        "Wrench\n"
        "4\n"
        "T002\n"
        "Y\n"
        "Y\n"
        "5\n";
    if (!write_text_file(stdin_path, input)) return 0;
    if (!redirect_stdin_to(stdin_path)) return 0;
    if (!redirect_stdout_to(stdout_path)) return 0;
    int rc = run_app();
    restore_stdio();
    char *out = read_text_file(stdout_path);
    char *csv = read_text_file(e2e_csv);
    if (!out || !csv) {
        free(out);
        free(csv);
        return 0;
    }
    int ok = rc == 0;
    ok &= str_contains(out, "T001,Hammer,5,100");
    ok &= str_contains(out, "New order added");
    ok &= str_contains(out, "Search Results for: 'Wrench'");
    ok &= str_contains(out, "Order ID: T003");
    ok &= str_contains(out, "Order T002 (Saw) deleted successfully.");
    ok &= str_contains(csv, "T003,Wrench,6,80");
    ok &= !str_contains(csv, "T002,Saw");
    free(out);
    free(csv);
    return ok;
}

static int test_cancel_add_flow(void) {
    const char *initial_csv = "T001,Hammer,5,100\n";
    if (!prepare_csv(initial_csv)) return 0;
    const char *input =
        "2\n"
        "MegaTool\n"
        "150\n"
        "N\n"
        "5\n";
    if (!write_text_file(stdin_path, input)) return 0;
    if (!redirect_stdin_to(stdin_path)) return 0;
    if (!redirect_stdout_to(stdout_path)) return 0;
    int rc = run_app();
    restore_stdio();
    char *out = read_text_file(stdout_path);
    char *csv = read_text_file(e2e_csv);
    if (!out || !csv) {
        free(out);
        free(csv);
        return 0;
    }
    int ok = rc == 0;
    ok &= str_contains(out, "Quantity 150 is very high");
    ok &= !str_contains(out, "New order added");
    ok &= !str_contains(csv, "MegaTool");
    ok &= str_contains(csv, "T001,Hammer,5,100");
    free(out);
    free(csv);
    return ok;
}

static int run_test(const char *name, int (*fn)(void)) {
    reset_e2e_state();
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
    all_ok &= run_test("full flow", test_full_flow);
    all_ok &= run_test("cancel add flow", test_cancel_add_flow);
    tools_file = original_csv_name;
    remove(e2e_csv);
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
        printf("E2E TESTS PASSED\n");
        return 0;
    } else {
        printf("E2E TESTS FAILED\n");
        return 1;
    }
}
