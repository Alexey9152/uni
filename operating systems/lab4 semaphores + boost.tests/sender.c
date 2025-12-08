#include <windows.h>
#include <stdio.h>
#include <string.h>

int main() {
    HANDLE hPipe;
    char buffer[1024];
    DWORD bytesWritten;

    hPipe = CreateFileA(
        "\\\\.\\pipe\\my_pipe",
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Failed to connect to pipe\n");
        return 1;
    }

    printf("Connected to receiver. Type messages.\n");
    printf("Type 'exit' to stop receiver and quit.\n");

    while (1) {
        printf("> ");
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            break;
        }

        // убираем \n
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        if (!WriteFile(hPipe, buffer, (DWORD)(len + 1), &bytesWritten, NULL)) {
            fprintf(stderr, "WriteFile failed\n");
            break;
        }

        if (strcmp(buffer, "exit") == 0) {
            printf("Exit command sent, closing sender.\n");
            break;
        }
    }

    CloseHandle(hPipe);
    return 0;
}
