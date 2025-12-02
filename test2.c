#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Structure for ship
typedef struct {
    char type;
    int length;
    int start_x, start_y;
    char orientation;
    int **cells;  // Array to track hits on each cell (x, y)
    int hits_received;
    int destroyed;
} Ship;

// Structure for player board
typedef struct {
    int N, M;
    int ship_count;
    Ship *ships;
    int **board;  // 2D array: 0 = empty, 1-5 = ship index
    int **hits;   // 2D array: 0 = not hit, 1 = hit
    int ships_remaining;
} PlayerBoard;

// Function prototypes
PlayerBoard* create_board(int N, int M, int ship_count);
void destroy_board(PlayerBoard *board);
int place_ship(PlayerBoard *board, char type, char orientation, int x, int y, int ship_index);
int is_valid_placement(PlayerBoard *board, char type, char orientation, int x, int y);
void print_board(PlayerBoard *board);
int attack(PlayerBoard *board, int x, int y, int player_num);
int get_ship_length(char type);
char* get_ship_name(char type);
int calculate_ships_per_type(int N, int M, char type);

// Calculate number of ships for a given type
int calculate_ships_per_type(int N, int M, char type) {
    switch (type) {
        case 'S': return (N * M) / 70;  // Shinano
        case 'Y': return (N * M) / 55;  // Yamato
        case 'B': return (N * M) / 40;  // Belfast
        case 'L': return (N * M) / 30;  // Laffey
        case 'A': return (N * M) / 20;  // Albacore
        default: return 0;
    }
}

// Create a new board with 2D arrays
PlayerBoard* create_board(int N, int M, int ship_count) {
    PlayerBoard *board = (PlayerBoard*)malloc(sizeof(PlayerBoard));
    board->N = N;
    board->M = M;
    board->ship_count = ship_count;
    board->ships_remaining = ship_count;
    
    // Allocate ships array
    board->ships = (Ship*)malloc(ship_count * sizeof(Ship));
    
    // Allocate and initialize board (ship indices)
    board->board = (int**)malloc((N + 2) * sizeof(int*));
    for (int i = 0; i <= N + 1; i++) {
        board->board[i] = (int*)calloc((M + 2), sizeof(int));
    }
    
    // Allocate and initialize hits array
    board->hits = (int**)malloc((N + 2) * sizeof(int*));
    for (int i = 0; i <= N + 1; i++) {
        board->hits[i] = (int*)calloc((M + 2), sizeof(int));
    }
    
    // Initialize ships
    for (int i = 0; i < ship_count; i++) {
        board->ships[i].cells = NULL;
        board->ships[i].destroyed = 0;
        board->ships[i].hits_received = 0;
    }
    
    return board;
}

// Destroy board and free all memory
void destroy_board(PlayerBoard *board) {
    if (!board) return;
    
    // Free ships and their cell arrays
    for (int i = 0; i < board->ship_count; i++) {
        if (board->ships[i].cells) {
            free(board->ships[i].cells);
        }
    }
    free(board->ships);
    
    // Free board array
    for (int i = 0; i <= board->N + 1; i++) {
        free(board->board[i]);
    }
    free(board->board);
    
    // Free hits array
    for (int i = 0; i <= board->N + 1; i++) {
        free(board->hits[i]);
    }
    free(board->hits);
    
    free(board);
}

// Get ship length based on type
int get_ship_length(char type) {
    switch (toupper(type)) {
        case 'S': return 5;  // Shinano
        case 'Y': return 4;  // Yamato
        case 'B': return 3;  // Belfast
        case 'L': return 2;  // Laffey
        case 'A': return 1;  // Albacore
        default: return 0;
    }
}

// Get ship name
char* get_ship_name(char type) {
    switch (toupper(type)) {
        case 'S': return "Shinano";
        case 'Y': return "Yamato";
        case 'B': return "Belfast";
        case 'L': return "Laffey";
        case 'A': return "Albacore";
        default: return "Unknown";
    }
}

// Check if placement is valid
int is_valid_placement(PlayerBoard *board, char type, char orientation, int x, int y) {
    int length = get_ship_length(type);
    if (length == 0) return 0;
    
    // Check bounds (1-indexed coordinates)
    if (x < 1 || x > board->N || y < 1 || y > board->M) {
        return 0;
    }
    
    if (orientation == 'H') {  // Horizontal
        if (y + length - 1 > board->M) {
            return 0;
        }
        // Check for collisions
        for (int i = 0; i < length; i++) {
            if (board->board[x][y + i] != 0) {
                return 0;  // Collision
            }
        }
    } else if (orientation == 'V') {  // Vertical
        if (x - length + 1 < 1) {
            return 0;
        }
        // Check for collisions
        for (int i = 0; i < length; i++) {
            if (board->board[x - i][y] != 0) {
                return 0;  // Collision
            }
        }
    } else {
        return 0;  // Invalid orientation
    }
    
    return 1;
}

// Place a ship on the board
int place_ship(PlayerBoard *board, char type, char orientation, int x, int y, int ship_index) {
    if (!is_valid_placement(board, type, orientation, x, y)) {
        return 0;
    }
    
    int length = get_ship_length(type);
    
    // Initialize ship
    board->ships[ship_index].type = type;
    board->ships[ship_index].length = length;
    board->ships[ship_index].start_x = x;
    board->ships[ship_index].start_y = y;
    board->ships[ship_index].orientation = orientation;
    board->ships[ship_index].hits_received = 0;
    board->ships[ship_index].destroyed = 0;
    
    // Allocate cells array to track ship's occupied positions
    board->ships[ship_index].cells = (int**)malloc(length * sizeof(int*));
    for (int i = 0; i < length; i++) {
        board->ships[ship_index].cells[i] = (int*)malloc(2 * sizeof(int));
    }
    
    // Mark ship on board and store cell coordinates
    if (orientation == 'H') {
        for (int i = 0; i < length; i++) {
            board->board[x][y + i] = length;  // Store ship length
            board->ships[ship_index].cells[i][0] = x;
            board->ships[ship_index].cells[i][1] = y + i;
        }
    } else {  // Vertical
        for (int i = 0; i < length; i++) {
            board->board[x - i][y] = length;  // Store ship length
            board->ships[ship_index].cells[i][0] = x - i;
            board->ships[ship_index].cells[i][1] = y;
        }
    }
    
    return 1;
}

// Print board
void print_board(PlayerBoard *board) {
    for (int i = 1; i <= board->N; i++) {
        for (int j = 1; j <= board->M; j++) {
            printf("%d", board->board[i][j]);
            if (j < board->M) printf(" ");
        }
        printf("\n");
    }
}

// Process an attack on a board
int attack(PlayerBoard *board, int x, int y, int player_num) {
    // Check bounds
    if (x < 1 || x > board->N || y < 1 || y > board->M) {
        return 0;  // Miss (out of bounds)
    }
    
    // Check if position was already hit
    if (board->hits[x][y]) {
        return -1;  // Already hit, lose turn
    }
    
    // Mark as hit
    board->hits[x][y] = 1;
    
    // Check if there's a ship at this position
    if (board->board[x][y] == 0) {
        return 0;  // Miss (water)
    }
    
    // Find which ship is at this position
    int ship_length = board->board[x][y];
    Ship *found_ship = NULL;
    int ship_index = -1;
    
    // Search for the ship containing this cell
    for (int i = 0; i < board->ship_count; i++) {
        if (board->ships[i].destroyed) continue;
        
        for (int j = 0; j < board->ships[i].length; j++) {
            int ship_x = board->ships[i].cells[j][0];
            int ship_y = board->ships[i].cells[j][1];
            
            if (ship_x == x && ship_y == y) {
                found_ship = &board->ships[i];
                ship_index = i;
                break;
            }
        }
        if (found_ship) break;
    }
    
    if (!found_ship) {
        return 0;  // Should not happen
    }
    
    // Check if hitting the start coordinate
    if (x == found_ship->start_x && y == found_ship->start_y) {
        // Destroy entire ship immediately
        found_ship->destroyed = 1;
        board->ships_remaining--;
        printf("Jucătorul %d a lovit o navă %s la coordonata (%d, %d).\n", 
               player_num, get_ship_name(found_ship->type), x, y);
        return 2;  // Ship destroyed
    }
    
    // Regular hit
    found_ship->hits_received++;
    
    printf("Jucătorul %d a lovit o navă %s la coordonata (%d, %d).\n", 
           player_num, get_ship_name(found_ship->type), x, y);
    
    // Check if ship is now destroyed
    if (found_ship->hits_received == found_ship->length) {
        found_ship->destroyed = 1;
        board->ships_remaining--;
        return 2;  // Ship destroyed
    }
    
    return 1;  // Hit but not destroyed
}

int main() {
    int J;
    scanf("%d", &J);
    
    for (int game = 0; game < J; game++) {
        int N, M;
        scanf("%d %d", &N, &M);
        
        // Calculate total number of ships
        int total_ships = 0;
        total_ships += calculate_ships_per_type(N, M, 'S');
        total_ships += calculate_ships_per_type(N, M, 'Y');
        total_ships += calculate_ships_per_type(N, M, 'B');
        total_ships += calculate_ships_per_type(N, M, 'L');
        total_ships += calculate_ships_per_type(N, M, 'A');
        
        // Create boards for both players
        PlayerBoard *player1 = create_board(N, M, total_ships);
        PlayerBoard *player2 = create_board(N, M, total_ships);
        
        // Place ships for player 1
        char ship_types[] = {'S', 'Y', 'B', 'L', 'A'};
        int ship_index = 0;
        
        for (int type_idx = 0; type_idx < 5; type_idx++) {
            int ships_count = calculate_ships_per_type(N, M, ship_types[type_idx]);
            for (int i = 0; i < ships_count; i++) {
                while (1) {
                    char type, orientation;
                    int x, y;
                    scanf(" %c %c %d %d", &type, &orientation, &x, &y);
                    
                    if (place_ship(player1, type, orientation, x, y, ship_index)) {
                        ship_index++;
                        break;
                    } else {
                        printf("Eroare: navă invalidă. Încercați din nou.\n");
                    }
                }
            }
        }
        
        // Place ships for player 2
        ship_index = 0;
        for (int type_idx = 0; type_idx < 5; type_idx++) {
            int ships_count = calculate_ships_per_type(N, M, ship_types[type_idx]);
            for (int i = 0; i < ships_count; i++) {
                while (1) {
                    char type, orientation;
                    int x, y;
                    scanf(" %c %c %d %d", &type, &orientation, &x, &y);
                    
                    if (place_ship(player2, type, orientation, x, y, ship_index)) {
                        ship_index++;
                        break;
                    } else {
                        printf("Eroare: navă invalidă. Încercați din nou.\n");
                    }
                }
            }
        }
        
        // Print both boards
        print_board(player1);
        printf("\n");
        print_board(player2);
        
        // Game loop
        int current_player = 1;
        int game_over = 0;
        
        while (!game_over) {
            int attack_x, attack_y;
            scanf("%d %d", &attack_x, &attack_y);
            
            int result;
            if (current_player == 1) {
                result = attack(player2, attack_x, attack_y, 1);
                if (result == -1) {
                    // Player loses turn for hitting already hit position
                    current_player = 2;
                    continue;
                }
                if (player2->ships_remaining == 0) {
                    printf("Jucătorul 1 a câștigat!\n");
                    game_over = 1;
                }
            } else {
                result = attack(player1, attack_x, attack_y, 2);
                if (result == -1) {
                    // Player loses turn for hitting already hit position
                    current_player = 1;
                    continue;
                }
                if (player1->ships_remaining == 0) {
                    printf("Jucătorul 2 a câștigat!\n");
                    game_over = 1;
                }
            }
            
            // Switch players
            current_player = (current_player == 1) ? 2 : 1;
        }
        
        // Clean up
        destroy_board(player1);
        destroy_board(player2);
        
        // Add newline between games if not the last game
        if (game < J - 1) {
            printf("\n");
        }
    }
    
    return 0;
}
