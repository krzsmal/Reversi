#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <pthread.h>

typedef char Board[8][8];

typedef struct {
    int clientSock1;
    int clientSock2;
} ThreadArgs;

// Funkcja inicjalizująca planszę
void initializeBoard(Board board) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = '.';
        }
    }

    // Plansza startowa
    board[3][3] = 'O';
    board[3][4] = 'X';
    board[4][3] = 'X';
    board[4][4] = 'O';
    
    // Brak możliwych ruchów - pass
    // board[2][3] = 'X';
    // board[2][4] = 'X';
    // board[3][2] = 'X';
    // board[4][2] = 'X';
    // board[5][3] = 'X';
    // board[5][4] = 'X';
    // board[3][5] = 'X';
    // board[4][5] = 'X';
    // board[2][2] = 'X';
    // board[2][5] = 'X';
    // board[5][2] = 'X';
    // board[5][5] = 'X';

    // Podwójny pass - koniec gry
    // board[3][3] = 'O';
    // board[3][4] = 'X';
    // board[7][7] = 'X';
    // board[0][0] = 'O';

    // Po ruchu X koniec gry (brak O)
    // board[3][3] = 'O';
    // board[3][4] = 'X';
    
    // Po ruchu X koniec gry (cała plansza zapełniona)
    // for (int i = 0; i < 8; ++i) {
    //     for (int j = 0; j < 8; ++j) {
    //         board[i][j] = 'O';
    //     }
    // }
    // board[0][0] = '.';
    // board[7][7] = 'X';
}

// Funkcja zliczająca punkty
int countPoints(Board board, char player) {
    int points = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] == player) {
                points++;
            }
        }
    }
    return points;
}

// Funkcja zamieniająca planszę na tekst
void boardToText(Board board, char *text) {
    int index = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            text[index++] = board[i][j];
        }
        text[index++] = ' ';
    }
    text[index-1] = '\0';
}

// Funkcja usuwająca możliwe ruchy
void clearValidMoves(Board board) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] == '*') {
                board[i][j] = '.';
            }
        }
    }
}

// Funkcja sprawdzająca czy ruch jest poprawny
bool isValidMove(Board board, int row, int col, char player, char opponent) {
    if (board[row][col] != '.') {
        return false;
    }

    // Przeszukuje wszystkie kierunki wokół danego pola
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            if (x == 0 && y == 0) {
                continue;
            }

            int r = row + x;
            int c = col + y;
            bool foundOpponent = false;

            // Przeszukuje w danym kierunku, dopóki jest w granicach planszy
            while (r >= 0 && r < 8 && c >= 0 && c < 8) {
                if (board[r][c] == opponent) {
                    foundOpponent = true; // Znalazł przeciwnika
                    r += x; // Przesuwa się dalej w tym samym kierunku
                    c += y; // Przesuwa się dalej w tym samym kierunku
                } else if (board[r][c] == player) {
                    if (foundOpponent) {
                        return true; // Znalazł gracza po przeciwniku
                    } else {
                        break; // Nie znalazł przeciwnika, przerywa pętlę
                    }
                } else {
                    break; // Znalazł puste pole lub inne, przerywa pętlę
                }
            }
        }
    }

    return false; // Nie znalazł ważnego ruchu w żadnym kierunku
}

// Funkcja zaznaczająca możliwe ruchy
void markValidMoves(Board board, int turn) {
    char player = (turn % 2 == 0) ? 'X' : 'O';
    char opponent = (turn % 2 == 0) ? 'O' : 'X';

    clearValidMoves(board);

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (isValidMove(board, i, j, player, opponent)) {
                board[i][j] = '*';
            }
        }
    }
}

// Funkcja wykonująca ruch
void makeMove(Board board, int row, int col, int turn) {
    char player = (turn % 2 == 0) ? 'X' : 'O';
    char opponent = (turn % 2 == 0) ? 'O' : 'X';
    board[row][col] = player; // Umieść pionek gracza na planszy
    clearValidMoves(board);

    // Przeszukuje wszystkie kierunki wokół danego pola
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            if (x == 0 && y == 0) {
                continue;
            }

            int r = row + x;
            int c = col + y;
            bool foundOpponent = false;

            // Przeszukuje w danym kierunku, dopóki jest w granicach planszy
            while (r >= 0 && r < 8 && c >= 0 && c < 8) {
                if (board[r][c] == opponent) {
                    foundOpponent = true;
                    r += x; // Przesuwa się dalej w tym samym kierunku
                    c += y; // Przesuwa się dalej w tym samym kierunku
                } else if (board[r][c] == player) {
                    if (foundOpponent) {
                        // Zmienia pionki przeciwnika na pionki gracza
                        while (board[r -= x][c -= y] == opponent) {
                            board[r][c] = player;
                        }
                    }
                    break; // Przerywa pętlę, gdy znajdzie pionek gracza
                } else {
                    break; // Przerywa pętlę, gdy znajdzie puste pole lub inne
                }
            }
        }
    }
}

// Funkcja sprawdzająca czy są możliwe ruchy
bool hasValidMoves(Board board) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] == '*') {
                return true;
            }
        }
    }
    return false;
}

// Funkcja sprawdzająca czy pole jest poprawne
bool isValidField(Board board, char *move){
    int row = move[1] - '1';
    int col = tolower(move[0]) - 'a';

    if (row < 0 || row >= 8 || col < 0 || col >= 8) {
        return false;
    }
    
    if (board[row][col] != '*') {
        return false;
    }
    
    return true;
}

// Funkcja wątku gry
void *game(void *arg){
    ThreadArgs *args = (ThreadArgs*)arg;
    int clientSock1 = args->clientSock1;
    int clientSock2 = args->clientSock2;

    free(args);

    // Losowanie symboli graczy
    srand(time(NULL));
    char roles[2] = {'X', 'O'};
    if (rand() % 2) {
        roles[0] = 'O';
        roles[1] = 'X';
    }

    // Wysłanie symboli graczy
    if (send(clientSock1, &roles[0], 1, MSG_NOSIGNAL) == -1 || send(clientSock2, &roles[1], 1, MSG_NOSIGNAL) == -1) {
        perror("Nie udało się wysłać symboli graczy");
        close(clientSock1);
        close(clientSock2);
        pthread_exit(NULL);
    }

    Board board;
    char boardText[71];
    char move[2];
    int turn = 0;
    int row, col;
    int passCount = 0;

    initializeBoard(board);
    boardToText(board, boardText);

    // Wysłanie planszy
    if (send(clientSock1, boardText, sizeof boardText, MSG_NOSIGNAL) == -1 || send(clientSock2, boardText, sizeof boardText, MSG_NOSIGNAL) == -1) {
        perror("Nie udało się wysłać planszy");
        close(clientSock1);
        close(clientSock2);
        pthread_exit(NULL);
    }

    while(1){
        int n;
        int currentPlayerSock = (roles[turn % 2] == 'X') ? clientSock1 : clientSock2;
        int otherPlayerSock = (roles[turn % 2] == 'X') ? clientSock2 : clientSock1;

        markValidMoves(board, turn);
        bzero(boardText, sizeof(boardText));

        // Sprawdzenie czy jest koniec gry
        if (passCount == 2 || countPoints(board, 'X') + countPoints(board, 'O') == 64 || countPoints(board, 'X') == 0 || countPoints(board, 'O') == 0) {
            bzero(boardText, sizeof(boardText));
            strcpy(boardText, "gameEnded");
            if (send(clientSock1, boardText, sizeof boardText, MSG_NOSIGNAL) == -1 || send(clientSock2, boardText, sizeof boardText, MSG_NOSIGNAL) == -1) {
                perror("Nie udało się wysłać informacji o końcu gry");
                break;
            }
            break;
        }

        // Wysłanie informacji o turze przeciwnika
        bzero(boardText, sizeof(boardText));
        strcpy(boardText, "oponentTurn");
        if (send(otherPlayerSock, boardText, sizeof boardText, MSG_NOSIGNAL) == -1) {
            perror("Nie udało się wysłać komunikatu o turze przeciwnika");
            break;
        }

        // Sprawdzenie czy gracz ma możliwe ruchy
        if (!hasValidMoves(board)) {
            // Wysłanie informacji o braku możliwych ruchów
            bzero(boardText, sizeof(boardText));
            strcpy(boardText, "noValidMoves");
            if (send(currentPlayerSock, boardText, sizeof boardText, MSG_NOSIGNAL) == -1) {
                perror("Nie udało się wysłać komunikatu o braku możliwych ruchów");
                break;
            }
            
            // Wysłanie planszy
            boardToText(board, boardText);
            if (send(otherPlayerSock, boardText, sizeof boardText, MSG_NOSIGNAL) == -1) {
                perror("Nie udało się wysłać planszy");
                break;
            }

            passCount++;
            turn++;
            continue;
        }
        passCount = 0;

        // Wysłanie planszy
        boardToText(board, boardText);
        if (send(currentPlayerSock, boardText, sizeof boardText, MSG_NOSIGNAL) == -1) {
            perror("Nie udało się wysłać planszy");
            break;
        }

        // Odczyt ruchu
        bool validMove = false;
        do {
            bzero(move, sizeof(move));
            n = read(currentPlayerSock, move, sizeof(move));
            if (n <= 0) {
                printf("Nie udało się odebrać ruchu gracza, opóścił on rozgrywkę lub utracono z nim połączenie\n");
                break;
            }

            if (!isValidField(board, move)) {
                bzero(boardText, sizeof(boardText));
                strcpy(boardText, "invalidMove");
                n = send(currentPlayerSock, boardText, strlen(boardText), MSG_NOSIGNAL);
                if (n <= 0) {
                    perror("Nie udało się wysłać informacji o niepoprawnym ruchu");
                    break;
                }
                continue;
            }
            validMove = true;
        } while (!validMove);

        if (n <= 0) {
            break;
        }

        row = move[1] - '1';
        col = tolower(move[0]) - 'a';

        makeMove(board, row, col, turn);
        
        // Wysłanie planszy do obu graczy
        bzero(boardText, sizeof(boardText));
        boardToText(board, boardText);
        if (send(currentPlayerSock, boardText, strlen(boardText), MSG_NOSIGNAL) == -1 || send(otherPlayerSock, boardText, strlen(boardText), MSG_NOSIGNAL) == -1) {
            perror("Nie udało się wysłać planszy");
            break;
        }
        turn++;
    }
    
    close(clientSock1);
    close(clientSock2);
    pthread_exit(NULL);
}

int main(void){
    struct sockaddr_in serverAddr;
    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == -1) {
        perror("Nie udało się utworzyć gniazda");
        exit(EXIT_FAILURE);
    }
    
    int one = 1;
    if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
        perror("Nie udało się ustawić opcji gniazda");
        close(serverSock);
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1100);
    serverAddr.sin_addr.s_addr = inet_addr("0.0.0.0");

    if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Nie udało się zrobić binda");
        close(serverSock);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSock, 5) == -1) {
        perror("Nie udało się zrobić listena");
        close(serverSock);
        exit(EXIT_FAILURE);
    }
    printf("Serwer czeka na graczy...\n");

    // Pętla tworząca nowe wątki dla każdej gry
    while(1){
        struct sockaddr_in clientAddr1;
        socklen_t clientAddrLen1 = sizeof(clientAddr1);
        int clientSock1 = accept(serverSock, (struct sockaddr*)&clientAddr1, &clientAddrLen1);
        if (clientSock1 == -1) {
            perror("Nie udało się połaczyć z klientem (1/2)");
            continue;
        }
        printf("Gracz dołączył (1/2)\n");
        
        struct sockaddr_in clientAddr2;
        socklen_t clientAddrLen2 = sizeof(clientAddr2);
        int clientSock2 = accept(serverSock, (struct sockaddr*)&clientAddr2, &clientAddrLen2);
        if (clientSock2 == -1) {
            perror("Nie udało się połaczyć z klientem (2/2)");
            close(clientSock1);
            continue;
        }
        printf("Gracz dołączył (2/2)\n");

        pthread_t thread;
        ThreadArgs *threadArgs = malloc(sizeof(ThreadArgs));
        if (threadArgs == NULL) {
            perror("Nie udało się zaalokować pamięci");
            close(clientSock1);
            close(clientSock2);
            continue;
        }
        threadArgs->clientSock1 = clientSock1;
        threadArgs->clientSock2 = clientSock2;
        
        if(pthread_create(&thread, NULL, game, (void*)threadArgs) != 0) {
            perror("Nie udało się utworzyć wątku");
            free(threadArgs);
            close(clientSock1);
            close(clientSock2);
            continue;
        }
        pthread_detach(thread);
    }
    close(serverSock);
    exit(EXIT_SUCCESS);
}