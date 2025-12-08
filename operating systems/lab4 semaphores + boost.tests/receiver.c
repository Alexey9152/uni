#include <windows.h>
#include <stdio.h>
#include <string.h>

int main() {
    HANDLE hPipe;
    char buffer[1024];
    DWORD bytesRead;

    hPipe = CreateNamedPipeA(
        "\\\\.\\pipe\\my_pipe",
        PIPE_ACCESS_INBOUND,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1,
        1024,
        1024,
        0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Failed to create pipe\n");
        return 1;
    }

    printf("Waiting for sender...\n");

    // Одно подключение, много сообщений
    if (!ConnectNamedPipe(hPipe, NULL) && GetLastError() != ERROR_PIPE_CONNECTED) {
        fprintf(stderr, "Failed to connect pipe\n");
        CloseHandle(hPipe);
        return 1;
    }

    while (1) {
        BOOL ok = ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        if (!ok || bytesRead == 0) {
            printf("Sender disconnected\n");
            break;
        }

        buffer[bytesRead] = '\0';
        printf("Received: %s\n", buffer);

        if (strcmp(buffer, "exit") == 0) {
            printf("Exit command received, shutting down receiver.\n");
            break;
        }
    }

    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    return 0;
}
