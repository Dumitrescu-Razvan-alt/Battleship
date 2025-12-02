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
    int *hits;  
    int total_hits;
    int destroyed;
} Ship;

// Structure for cell in sparse matrix
typedef struct CellNode {
    int x, y;
    int ship_index;  
    struct CellNode *next;
} CellNode;

// Structure for player board
typedef struct {
    int N, M;
    int ship_count;
    Ship *ships;
    CellNode **board;  
    int *row_sizes;    // Sizes of each row's linked list
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

// Create a new board with sparse matrix representation
PlayerBoard* create_board(int N, int M, int ship_count) {
    PlayerBoard *board = (PlayerBoard*)malloc(sizeof(PlayerBoard));
    board->N = N;
    board->M = M;
    board->ship_count = ship_count;
    board->ships_remaining = ship_count;
    
    // Allocate ships array
    board->ships = (Ship*)malloc(ship_count * sizeof(Ship));
    
    // Allocate sparse matrix (array of linked lists for each row)
    board->board = (CellNode**)malloc((N + 1) * sizeof(CellNode*));
    board->row_sizes = (int*)calloc((N + 1), sizeof(int));
    
    for (int i = 1; i <= N; i++) {
        board->board[i] = NULL;
    }
    
    return board;
}

// Destroy board and free all memory
void destroy_board(PlayerBoard *board) {
    if (!board) return;
    
    // Free ships
    for (int i = 0; i < board->ship_count; i++) {
        if (board->ships[i].hits) {
            free(board->ships[i].hits);
        }
    }
    free(board->ships);
    
    // Free sparse matrix
    for (int i = 1; i <= board->N; i++) {
        CellNode *current = board->board[i];
        while (current) {
            CellNode *temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(board->board);
    free(board->row_sizes);
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
    
    // Check bounds
    if (x < 1 || x > board->N || y < 1 || y > board->M) {
        return 0;
    }
    
    if (orientation == 'H') {  // Horizontal
        if (y + length - 1 > board->M) {
            return 0;
        }
        // Check for collisions
        for (int i = 0; i < length; i++) {
            CellNode *current = board->board[x];
            while (current) {
                if (current->y == y + i) {
                    return 0;  // Collision
                }
                current = current->next;
            }
        }
    } else if (orientation == 'V') {  // Vertical
        if (x - length + 1 < 1) {
            return 0;
        }
        // Check for collisions
        for (int i = 0; i < length; i++) {
            CellNode *current = board->board[x - i];
            while (current) {
                if (current->y == y) {
                    return 0;  // Collision
                }
                current = current->next;
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
    board->ships[ship_index].hits = (int*)calloc(length, sizeof(int));
    board->ships[ship_index].total_hits = 0;
    board->ships[ship_index].destroyed = 0;
    
    // Add ship cells to sparse matrix
    if (orientation == 'H') {
        for (int i = 0; i < length; i++) {
            CellNode *new_node = (CellNode*)malloc(sizeof(CellNode));
            new_node->x = x;
            new_node->y = y + i;
            new_node->ship_index = ship_index;
            new_node->next = board->board[x];
            board->board[x] = new_node;
            board->row_sizes[x]++;
        }
    } else {  // Vertical
        for (int i = 0; i < length; i++) {
            CellNode *new_node = (CellNode*)malloc(sizeof(CellNode));
            new_node->x = x - i;
            new_node->y = y;
            new_node->ship_index = ship_index;
            new_node->next = board->board[x - i];
            board->board[x - i] = new_node;
            board->row_sizes[x - i]++;
        }
    }
    
    return 1;
}

// Print board (for debugging/display)
void print_board(PlayerBoard *board) {
    // Create a temporary dense representation for printing
    int **temp_board = (int**)malloc((board->N + 1) * sizeof(int*));
    for (int i = 1; i <= board->N; i++) {
        temp_board[i] = (int*)calloc((board->M + 1), sizeof(int));
    }
    
    // Fill from sparse matrix
    for (int i = 1; i <= board->N; i++) {
        CellNode *current = board->board[i];
        while (current) {
            int ship_idx = current->ship_index;
            temp_board[current->x][current->y] = board->ships[ship_idx].length;
            current = current->next;
        }
    }
    
    // Print
    for (int i = 1; i <= board->N; i++) {
        for (int j = 1; j <= board->M; j++) {
            printf("%d", temp_board[i][j]);
            if (j < board->M) printf(" ");
        }
        printf("\n");
    }
    
    // Free temporary board
    for (int i = 1; i <= board->N; i++) {
        free(temp_board[i]);
    }
    free(temp_board);
}

// Process an attack on a board
int attack(PlayerBoard *board, int x, int y, int player_num) {
    // Check bounds
    if (x < 1 || x > board->N || y < 1 || y > board->M) {
        return 0;  // Miss
    }
    
    // Search for ship at position
    CellNode *current = board->board[x];
    while (current) {
        if (current->y == y) {
            int ship_idx = current->ship_index;
            Ship *ship = &board->ships[ship_idx];
            
            if (ship->destroyed) {
                return 0;  // Already destroyed, counts as miss
            }
            
            // Check if this is the start coordinate
            if (x == ship->start_x && y == ship->start_y) {
                // Destroy entire ship immediately
                ship->destroyed = 1;
                board->ships_remaining--;
                printf("Jucătorul %d a lovit o navă %s la coordonata (%d, %d).\n", 
                       player_num, get_ship_name(ship->type), x, y);
                return 2;  // Ship destroyed
            }
            
            // Calculate which segment was hit
            int segment = 0;
            if (ship->orientation == 'H') {
                segment = y - ship->start_y;
            } else {
                segment = ship->start_x - x;
            }
            
            // Mark hit if not already hit
            if (!ship->hits[segment]) {
                ship->hits[segment] = 1;
                ship->total_hits++;
                
                printf("Jucătorul %d a lovit o navă %s la coordonata (%d, %d).\n", 
                       player_num, get_ship_name(ship->type), x, y);
                
                // Check if ship is now destroyed
                if (ship->total_hits == ship->length) {
                    ship->destroyed = 1;
                    board->ships_remaining--;
                    return 2;  // Ship destroyed
                }
                return 1;  // Hit but not destroyed
            } else {
                return 0;  // Already hit this segment, counts as miss
            }
        }
        current = current->next;
    }
    
    return 0;  // Miss (no ship at position)
}

int main() {
    int J;
    scanf("%d", &J);
    
    for (int game = 0; game < J; game++) {
        int N, M;
        scanf("%d %d", &N, &M);
        
        // Calculate number of ships for each type
        int ships_per_type[5];
        ships_per_type[0] = (N * M) / 70;  // Shinano (5)
        ships_per_type[1] = (N * M) / 55;  // Yamato (4)
        ships_per_type[2] = (N * M) / 40;  // Belfast (3)
        ships_per_type[3] = (N * M) / 30;  // Laffey (2)
        ships_per_type[4] = (N * M) / 20;  // Albacore (1)
        
        int total_ships = 0;
        for (int i = 0; i < 5; i++) {
            total_ships += ships_per_type[i];
        }
        
        // Create boards for both players
        PlayerBoard *player1 = create_board(N, M, total_ships);
        PlayerBoard *player2 = create_board(N, M, total_ships);
        
        // Place ships for player 1
        char ship_types[] = {'S', 'Y', 'B', 'L', 'A'};
        int ship_index = 0;
        
        for (int type_idx = 0; type_idx < 5; type_idx++) {
            for (int i = 0; i < ships_per_type[type_idx]; i++) {
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
            for (int i = 0; i < ships_per_type[type_idx]; i++) {
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
                if (player2->ships_remaining == 0) {
                    printf("Jucătorul 1 a câștigat!\n");
                    game_over = 1;
                }
            } else {
                result = attack(player1, attack_x, attack_y, 2);
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
    }
    
    return 0;
}
