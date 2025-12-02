#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Structura pentru navă
typedef struct {
    char tip;
    int lungime;
    int start_x, start_y;
    char orientare;
    int **celule;  // Vector pentru a urmări loviturile pe fiecare celulă (x, y)
    int lovituri_primite;
    int distrus;
} Nava;

// Structura pentru tabla jucător
typedef struct {
    int N, M;
    int numar_nave;
    Nava *nave;
    int **tabla;  // Array 2D: 0 = gol, 1-5 = index navă
    int **lovituri;   // Array 2D: 0 = nelovit, 1 = lovit
    int nave_ramase;
} TablaJucator;

// Prototipuri funcții
TablaJucator* creeaza_tabla(int N, int M, int numar_nave);
void distruge_tabla(TablaJucator *tabla);
int plaseaza_nava(TablaJucator *tabla, char tip, char orientare, int x, int y, int index_nava);
int este_plasare_valida(TablaJucator *tabla, char tip, char orientare, int x, int y);
void afiseaza_tabla(TablaJucator *tabla);
int atac(TablaJucator *tabla, int x, int y, int numar_jucator);
int obtine_lungime_nava(char tip);
char* obtine_nume_nava(char tip);
int calculeaza_nave_per_tip(int N, int M, char tip);

// Calculează numărul de nave pentru un tip dat
int calculeaza_nave_per_tip(int N, int M, char tip) {
    switch (tip) {
        case 'S': return (N * M) / 70;  // Shinano
        case 'Y': return (N * M) / 55;  // Yamato
        case 'B': return (N * M) / 40;  // Belfast
        case 'L': return (N * M) / 30;  // Laffey
        case 'A': return (N * M) / 20;  // Albacore
        default: return 0;
    }
}

// Creează o tablă nouă cu array-uri 2D
TablaJucator* creeaza_tabla(int N, int M, int numar_nave) {
    TablaJucator *tabla = (TablaJucator*)malloc(sizeof(TablaJucator));
    tabla->N = N;
    tabla->M = M;
    tabla->numar_nave = numar_nave;
    tabla->nave_ramase = numar_nave;
    
    // Alocă array-ul de nave
    tabla->nave = (Nava*)malloc(numar_nave * sizeof(Nava));
    
    // Alocă și inițializează tabla (indici nave)
    tabla->tabla = (int**)malloc((N + 2) * sizeof(int*));
    for (int i = 0; i <= N + 1; i++) {
        tabla->tabla[i] = (int*)calloc((M + 2), sizeof(int));
    }
    
    // Alocă și inițializează array-ul de lovituri
    tabla->lovituri = (int**)malloc((N + 2) * sizeof(int*));
    for (int i = 0; i <= N + 1; i++) {
        tabla->lovituri[i] = (int*)calloc((M + 2), sizeof(int));
    }
    
    // Inițializează navele
    for (int i = 0; i < numar_nave; i++) {
        tabla->nave[i].celule = NULL;
        tabla->nave[i].distrus = 0;
        tabla->nave[i].lovituri_primite = 0;
    }
    
    return tabla;
}

// Distruge tabla și eliberează toată memoria
void distruge_tabla(TablaJucator *tabla) {
    if (!tabla) return;
    
    // Eliberează navele și array-urile lor de celule
    for (int i = 0; i < tabla->numar_nave; i++) {
        if (tabla->nave[i].celule) {
            free(tabla->nave[i].celule);
        }
    }
    free(tabla->nave);
    
    // Eliberează array-ul tablă
    for (int i = 0; i <= tabla->N + 1; i++) {
        free(tabla->tabla[i]);
    }
    free(tabla->tabla);
    
    // Eliberează array-ul de lovituri
    for (int i = 0; i <= tabla->N + 1; i++) {
        free(tabla->lovituri[i]);
    }
    free(tabla->lovituri);
    
    free(tabla);
}

// Obține lungimea navei pe baza tipului
int obtine_lungime_nava(char tip) {
    switch (toupper(tip)) {
        case 'S': return 5;  // Shinano
        case 'Y': return 4;  // Yamato
        case 'B': return 3;  // Belfast
        case 'L': return 2;  // Laffey
        case 'A': return 1;  // Albacore
        default: return 0;
    }
}

// Obține numele navei
char* obtine_nume_nava(char tip) {
    switch (toupper(tip)) {
        case 'S': return "Shinano";
        case 'Y': return "Yamato";
        case 'B': return "Belfast";
        case 'L': return "Laffey";
        case 'A': return "Albacore";
        default: return "Necunoscut";
    }
}

// Verifică dacă plasarea este validă
int este_plasare_valida(TablaJucator *tabla, char tip, char orientare, int x, int y) {
    int lungime = obtine_lungime_nava(tip);
    if (lungime == 0) return 0;
    
    // Verifică limitele (coordonate indexate de la 1)
    if (x < 1 || x > tabla->N || y < 1 || y > tabla->M) {
        return 0;
    }
    
    if (orientare == 'H') {  // Orizontal
        if (y + lungime - 1 > tabla->M) {
            return 0;
        }
        // Verifică coliziuni
        for (int i = 0; i < lungime; i++) {
            if (tabla->tabla[x][y + i] != 0) {
                return 0;  // Coliziune
            }
        }
    } else if (orientare == 'V') {  // Vertical
        if (x - lungime + 1 < 1) {
            return 0;
        }
        // Verifică coliziuni
        for (int i = 0; i < lungime; i++) {
            if (tabla->tabla[x - i][y] != 0) {
                return 0;  // Coliziune
            }
        }
    } else {
        return 0;  // Orientare invalidă
    }
    
    return 1;
}

// Plasează o navă pe tablă
int plaseaza_nava(TablaJucator *tabla, char tip, char orientare, int x, int y, int index_nava) {
    if (!este_plasare_valida(tabla, tip, orientare, x, y)) {
        return 0;
    }
    
    int lungime = obtine_lungime_nava(tip);
    
    // Inițializează nava
    tabla->nave[index_nava].tip = tip;
    tabla->nave[index_nava].lungime = lungime;
    tabla->nave[index_nava].start_x = x;
    tabla->nave[index_nava].start_y = y;
    tabla->nave[index_nava].orientare = orientare;
    tabla->nave[index_nava].lovituri_primite = 0;
    tabla->nave[index_nava].distrus = 0;
    
    // Alocă array-ul de celule pentru a urmări pozițiile ocupate de navă
    tabla->nave[index_nava].celule = (int**)malloc(lungime * sizeof(int*));
    for (int i = 0; i < lungime; i++) {
        tabla->nave[index_nava].celule[i] = (int*)malloc(2 * sizeof(int));
    }
    
    // Marchează nava pe tablă și stochează coordonatele celulelor
    if (orientare == 'H') {
        for (int i = 0; i < lungime; i++) {
            tabla->tabla[x][y + i] = lungime;  // Stochează lungimea navei
            tabla->nave[index_nava].celule[i][0] = x;
            tabla->nave[index_nava].celule[i][1] = y + i;
        }
    } else {  // Vertical
        for (int i = 0; i < lungime; i++) {
            tabla->tabla[x - i][y] = lungime;  // Stochează lungimea navei
            tabla->nave[index_nava].celule[i][0] = x - i;
            tabla->nave[index_nava].celule[i][1] = y;
        }
    }
    
    return 1;
}

// Afișează tabla
void afiseaza_tabla(TablaJucator *tabla) {
    for (int i = 1; i <= tabla->N; i++) {
        for (int j = 1; j <= tabla->M; j++) {
            printf("%d", tabla->tabla[i][j]);
            if (j < tabla->M) printf(" ");
        }
        printf("\n");
    }
}

// Procesează un atac pe o tablă
int atac(TablaJucator *tabla, int x, int y, int numar_jucator) {
    // Verifică limitele
    if (x < 1 || x > tabla->N || y < 1 || y > tabla->M) {
        return 0;  // Ratat (în afara limitelor)
    }
    
    // Verifică dacă poziția a fost deja lovită
    if (tabla->lovituri[x][y]) {
        return -1;  // Deja lovit, pierde rândul
    }
    
    // Marchează ca lovit
    tabla->lovituri[x][y] = 1;
    
    // Verifică dacă există o navă la această poziție
    if (tabla->tabla[x][y] == 0) {
        return 0;  // Ratat (apă)
    }
    
    // Găsește care navă se află la această poziție
    int lungime_nava = tabla->tabla[x][y];
    Nava *nava_gasita = NULL;
    int index_nava = -1;
    
    // Caută nava care conține această celulă
    for (int i = 0; i < tabla->numar_nave; i++) {
        if (tabla->nave[i].distrus) continue;
        
        for (int j = 0; j < tabla->nave[i].lungime; j++) {
            int nava_x = tabla->nave[i].celule[j][0];
            int nava_y = tabla->nave[i].celule[j][1];
            
            if (nava_x == x && nava_y == y) {
                nava_gasita = &tabla->nave[i];
                index_nava = i;
                break;
            }
        }
        if (nava_gasita) break;
    }
    
    if (!nava_gasita) {
        return 0;  // Nu ar trebui să se întâmple
    }
    
    // Verifică dacă se lovește coordonata de start
    if (x == nava_gasita->start_x && y == nava_gasita->start_y) {
        // Distruge întreaga navă imediat
        nava_gasita->distrus = 1;
        tabla->nave_ramase--;
        printf("Jucătorul %d a lovit o navă %s la coordonata (%d, %d).\n", 
               numar_jucator, obtine_nume_nava(nava_gasita->tip), x, y);
        return 2;  // Navă distrusă
    }
    
    // Lovitură normală
    nava_gasita->lovituri_primite++;
    
    printf("Jucătorul %d a lovit o navă %s la coordonata (%d, %d).\n", 
           numar_jucator, obtine_nume_nava(nava_gasita->tip), x, y);
    
    // Verifică dacă nava este acum distrusă
    if (nava_gasita->lovituri_primite == nava_gasita->lungime) {
        nava_gasita->distrus = 1;
        tabla->nave_ramase--;
        return 2;  // Navă distrusă
    }
    
    return 1;  // Lovit dar nu distrus
}

int main() {
    int J;
    scanf("%d", &J);
    
    for (int joc = 0; joc < J; joc++) {
        int N, M;
        scanf("%d %d", &N, &M);
        
        // Calculează numărul total de nave
        int total_nave = 0;
        total_nave += calculeaza_nave_per_tip(N, M, 'S');
        total_nave += calculeaza_nave_per_tip(N, M, 'Y');
        total_nave += calculeaza_nave_per_tip(N, M, 'B');
        total_nave += calculeaza_nave_per_tip(N, M, 'L');
        total_nave += calculeaza_nave_per_tip(N, M, 'A');
        
        // Creează table pentru ambii jucători
        TablaJucator *jucator1 = creeaza_tabla(N, M, total_nave);
        TablaJucator *jucator2 = creeaza_tabla(N, M, total_nave);
        
        // Plasează nave alternant între cei 2 jucători
        char tipuri_nave[] = {'S', 'Y', 'B', 'L', 'A'};
        int index_nava_j1 = 0;
        int index_nava_j2 = 0;
        
        for (int idx_tip = 0; idx_tip < 5; idx_tip++) {
            int numar_nave = calculeaza_nave_per_tip(N, M, tipuri_nave[idx_tip]);
            for (int i = 0; i < numar_nave; i++) {
                // Jucătorul 1 plasează o navă
                while (1) {
                    char tip, orientare;
                    int x, y;
                    scanf(" %c %c %d %d", &tip, &orientare, &x, &y);
                    
                    if (plaseaza_nava(jucator1, tip, orientare, x, y, index_nava_j1)) {
                        index_nava_j1++;
                        break;
                    } else {
                        printf("Eroare: navă invalidă. Încercați din nou.\n");
                    }
                }
                
                // Jucătorul 2 plasează o navă
                while (1) {
                    char tip, orientare;
                    int x, y;
                    scanf(" %c %c %d %d", &tip, &orientare, &x, &y);
                    
                    if (plaseaza_nava(jucator2, tip, orientare, x, y, index_nava_j2)) {
                        index_nava_j2++;
                        break;
                    } else {
                        printf("Eroare: navă invalidă. Încercați din nou.\n");
                    }
                }
            }
        }
        
        // Afișează ambele table
        afiseaza_tabla(jucator1);
        printf("\n");
        afiseaza_tabla(jucator2);
        
        // Bucla jocului
        int jucator_curent = 1;
        int joc_terminat = 0;
        
        while (!joc_terminat) {
            int atac_x, atac_y;
            scanf("%d %d", &atac_x, &atac_y);
            
            int rezultat;
            if (jucator_curent == 1) {
                rezultat = atac(jucator2, atac_x, atac_y, 1);
                if (rezultat == -1) {
                    // Jucătorul pierde rândul pentru că a lovit o poziție deja lovită
                    jucator_curent = 2;
                    continue;
                }
                if (jucator2->nave_ramase == 0) {
                    printf("Jucătorul 1 a câștigat!\n");
                    joc_terminat = 1;
                }
            } else {
                rezultat = atac(jucator1, atac_x, atac_y, 2);
                if (rezultat == -1) {
                    // Jucătorul pierde rândul pentru că a lovit o poziție deja lovită
                    jucator_curent = 1;
                    continue;
                }
                if (jucator1->nave_ramase == 0) {
                    printf("Jucătorul 2 a câștigat!\n");
                    joc_terminat = 1;
                }
            }
            
            // Schimbă jucătorii
            jucator_curent = (jucator_curent == 1) ? 2 : 1;
        }
        
        // Curăță memoria
        distruge_tabla(jucator1);
        distruge_tabla(jucator2);
        
        // Adaugă linie nouă între jocuri dacă nu e ultimul joc
        if (joc < J - 1) {
            printf("\n");
        }
    }
    
    return 0;
}
