#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    int row, col, value;
    bool active, head;
} Triple;

// Function to read input from user
void readInput(int *x, char *orientation, int *row, int *col) {
    printf("Enter: x orientation row col\n");
    scanf("%d %c %d %d", x, orientation, row, col);
}

// Function to build sparse matrix
Triple* buildSparseMatrix(int x, char orientation, int row, int col) {
    Triple *sparse = malloc(x * sizeof(Triple));
    if (!sparse) {
        printf("Memory allocation failed.\n");
        return NULL;
    }
    bool head = 1;
    for (int i = 0; i < x; i++) {
        if (orientation == 'h') {
            // horizontal: start from leftmost at (row, col)
            sparse[i].row = row;
            sparse[i].col = col + i;
            sparse[i].active = 1;
            sparse[i].head = head;
        } 
        else if (orientation == 'v') {
            // vertical: start from bottom at (row, col)
            sparse[i].row = row - i;
            sparse[i].col = col;
             sparse[i].active = 1;
            sparse[i].head = head;
        } 
        else {
            printf("Invalid orientation.\n");
            free(sparse);
            return NULL;
        }
        sparse[i].value = x;
        head=0;
    }
    
    return sparse;
}

// Function to print sparse matrix
void printSparseMatrix(Triple *sparse, int x) {
    printf("\nSparse matrix (row col value):\n");
    for (int i = 0; i < x; i++) {
        printf("%d %d %d %d %d\n", sparse[i].row, sparse[i].col, sparse[i].value, sparse[i].active, sparse[i].head);
    }
}

// Function to find the dimensions needed for dense matrix
void findMatrixDimensions(Triple *sparse, int x, int *maxRow, int *maxCol) {
    *maxRow = sparse[0].row;
    *maxCol = sparse[0].col;
    
    for (int i = 1; i < x; i++) {
        if (sparse[i].row > *maxRow) *maxRow = sparse[i].row;
        if (sparse[i].col > *maxCol) *maxCol = sparse[i].col;
    }
}

// Function to print both sparse and dense representations
void printBothRepresentations(Triple *sparse, int x) {
    // Print sparse matrix
    printSparseMatrix(sparse, x);
    
    // Find dimensions for dense matrix
    int maxRow, maxCol;
    findMatrixDimensions(sparse, x, &maxRow, &maxCol);
    
    // Allocate and initialize dense matrix
    int **dense = malloc((maxRow + 1) * sizeof(int*));
    if (!dense) {
        printf("Memory allocation failed for dense matrix.\n");
        return;
    }
    
    for (int i = 0; i <= maxRow; i++) {
        dense[i] = calloc(maxCol + 1, sizeof(int));
        if (!dense[i]) {
            printf("Memory allocation failed for dense matrix row.\n");
            // Free previously allocated rows
            for (int j = 0; j < i; j++) {
                free(dense[j]);
            }
            free(dense);
            return;
        }
    }
    
    // Fill dense matrix with values from sparse matrix
    for (int i = 0; i < x; i++) {
        dense[sparse[i].row][sparse[i].col] = sparse[i].value;
    }
    
    // Print dense matrix
    printf("\nDense matrix representation:\n");
    for (int i = 0; i <= maxRow; i++) {
        for (int j = 0; j <= maxCol; j++) {
            printf("%3d ", dense[i][j]);
        }
        printf("\n");
    }
    
    // Free dense matrix
    for (int i = 0; i <= maxRow; i++) {
        free(dense[i]);
    }
    free(dense);
}

int main() {
    int x, row, col;
    char orientation;
    
    // Read input
    readInput(&x, &orientation, &row, &col);
    
    // Build sparse matrix
    Triple *sparse = buildSparseMatrix(x, orientation, row, col);
    if (!sparse) {
        return 1;
    }
    
    // Print both representations
    printBothRepresentations(sparse, x);
    
    // Clean up
    free(sparse);
    return 0;
}
