#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define RED   "\x1B[31m"
#define BLUE  "\x1B[34m"
#define GREEN "\x1B[32m"
#define RESET "\x1B[0m"
#define YELLOW "\x1B[33m"
#define LIME "\x1B[92m"

typedef char Board[8][8];

// Funkcja zamieniająca tekst na planszę
void textToBoard(char *text, Board board) {
    int index = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = text[index++];
        }
        index++;
    }
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

// Funkcja rysująca planszę
void drawBoard(Board board) {
    system("clear");
    printf("%sGracz X: %d%s Gracz O: %d %s\n\n", RED, countPoints(board, 'X'), BLUE, countPoints(board, 'O'), RESET);
    printf("  a b c d e f g h\n");
    for (int i = 0; i < 8; i++) {
        printf("%d ", i + 1);
        for (int j = 0; j < 8; j++) {
            if (board[i][j] == 'X') {
                printf("%sX%s ", RED, RESET);
            } else if (board[i][j] == 'O') {
                printf("%sO%s ", BLUE, RESET);
            } else if (board[i][j] == '*') {
                printf("%s*%s ", GREEN, RESET);
            } else {
                printf("%c ", board[i][j]);
            }
        }
        printf("%d\n", i + 1);
    }
    printf("  a b c d e f g h\n\n");
}

int main(int argc, char *argv[]) {
    struct sockaddr_in serverAddr;
    int clientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSock == -1){
        perror("Nie udało się utworzyć gniazda!");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[2]));
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(clientSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Nie udało się połączyć z serwerem!");
        close(clientSock);
        exit(EXIT_FAILURE);
    }
    
    printf("%sOczekiwanie na drugiego gracza!%s\n", YELLOW, RESET);

    Board board;
    int n;
    char player, move[6], boardText[71];

    // Odczytanie symbolu gracza
    n = read(clientSock, &player, sizeof(player));
    if (n <= 0) {
        printf("Przeciwnik opuścił rozgrywkę lub utracono połaczenie z serwerem!\n");
        close(clientSock);
        exit(EXIT_FAILURE);
    }
    system("clear");
    printf("%sTwój symbol to: %c%s\n", (player == 'X') ? RED : BLUE, player, RESET);
    sleep(3);
    system("clear");


    // Odczytanie planszy
    n = read(clientSock, boardText, sizeof(boardText));
    if (n <= 0) {
        printf("Przeciwnik opuścił rozgrywkę lub utracono połaczenie z serwerem!\n");
        close(clientSock);
        exit(EXIT_FAILURE);
    }

    textToBoard(boardText, board);
    drawBoard(board);

    while(1) {
        // Odczytanie informacji o stanie gry
        bzero(boardText, sizeof boardText);
        n = read(clientSock, boardText, sizeof(boardText));
        if (n <= 0) {
            printf("Przeciwnik opuścił rozgrywkę lub utracono połaczenie z serwerem!\n");
            break;
        }

        // Koniec gry
        if (strcmp(boardText, "gameEnded") == 0) {
            int xPoints = countPoints(board, 'X');
            int oPoints = countPoints(board, 'O');
            if (xPoints == oPoints) {
                printf("%sKoniec gry, remis!%s\n", LIME, RESET);
            } else if ((xPoints > oPoints && player == 'X') || (oPoints > xPoints && player == 'O')) {
                printf("%sKoniec gry, wygrałeś!%s\n", LIME, RESET);
            } else {
                printf("%sKoniec gry, przegrałeś!%s\n", LIME, RESET);
            }
            break;
        }

        // Tura przeciwnika
        if (strcmp(boardText, "oponentTurn") == 0) {
            printf("%sOczekiwanie na ruch przeciwnika!%s\n", YELLOW, RESET);
            sleep(3);

            // Odczytanie planszy
            bzero(boardText, sizeof boardText);
            n = read(clientSock, boardText, sizeof(boardText));
            if (n <= 0) {
                printf("Przeciwnik opuścił rozgrywkę lub utracono połaczenie z serwerem!\n");
                close(clientSock);
                exit(EXIT_FAILURE);
            }

            // Rysowanie planszy
            textToBoard(boardText, board);
            drawBoard(board);
            continue;
        }

        // Brak możliwych ruchów - pass
        if (strcmp(boardText, "noValidMoves") == 0) {
            printf("%sNie możesz wykonać żadnego ruchu!%s\n", YELLOW, RESET);
            sleep(3);
            continue;
        }

        // Rysowanie planszy
        textToBoard(boardText, board);
        drawBoard(board);

        // Ruch gracza
        bool validMove = false;
        while (!validMove) {
            printf("%sPodaj swój ruch (%c): %s", (player == 'X') ? RED : BLUE, player, RESET);
            bzero(move, sizeof(move));

            if (fgets(move, sizeof(move), stdin) == NULL) {
                printf("Niepoprawny ruch\n");
                continue;
            }

            move[strcspn(move, "\n")] = '\0';

            if (strcmp(move, "quit") == 0) {
                break;
            }

            if (strlen(move) != 2) {
                printf("Niepoprawny ruch. Poprawny format to [litera][cyfra] np. a1, * oznacza ruchy możliwe do wykoania w danej turze.\n");
                continue;
            }

            if (send(clientSock, move, 2, MSG_NOSIGNAL) == -1) {
                perror("Nie udało się wysłać ruchu, przeciwnik opuścił rozgrywkę lub utracono połączenie z serwerem!\n");
                break;
            }

            // Sprawdzenie czy ruch jest poprawny
            bzero(boardText, sizeof(boardText));
            n = read(clientSock, boardText, sizeof(boardText));
            if (n <= 0) {
                printf("Przeciwnik opuścił rozgrywkę lub utracono połaczenie z serwerem\n");
                close(clientSock);
                exit(EXIT_FAILURE);
            }
            if (strcmp(boardText, "invalidMove") == 0) {
                printf("Niepoprawny ruch. Ruch powinien składać się z dwóch znaków np: a1, możliwe ruchy są oznaczone za pomocą *.\n");
            } else {
                validMove = true;
            }
        }
        if (strcmp(move, "quit") == 0) {
            break;
        }

        // Wyświetlenie planszy po ruchu
        textToBoard(boardText, board);
        drawBoard(board);
    }
    
    close(clientSock);
    exit(EXIT_SUCCESS);
}