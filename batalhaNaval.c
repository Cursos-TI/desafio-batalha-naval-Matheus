#include <stdio.h>
#include <string.h>

#define N 10  // Tamanho padrão do tabuleiro para o nível Aventureiro/Mestre

// ------------------------------ Utilitários de impressão ------------------------------

void print_coords_header(const char* titulo) {
    printf("\n=== %s ===\n", titulo);
}

void print_board_int(const int rows, const int cols, int board[rows][cols]) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            printf("%d ", board[i][j]);
        }
        printf("\n");
    }
}

void print_separator() {
    printf("\n----------------------------------------\n");
}

// ------------------------------ Nível Novato ------------------------------

typedef struct {
    int linha;
    int coluna;
} Ponto;

/*
 * Gera as coordenadas de um navio retangular em linha/coluna.
 * orient = 'H' (horizontal) ou 'V' (vertical).
 * coords[] deve ter tamanho >= tamanho.
 */
void gerar_navio_basico(int linhaInicio, int colunaInicio, int tamanho, char orient, Ponto coords[]) {
    for (int k = 0; k < tamanho; ++k) {
        if (orient == 'H') {
            coords[k].linha  = linhaInicio;
            coords[k].coluna = colunaInicio + k;
        } else { // 'V'
            coords[k].linha  = linhaInicio + k;
            coords[k].coluna = colunaInicio;
        }
    }
}

void nivel_novato_demo() {
    print_coords_header("Nível Novato - Coordenadas dos Navios");

    // Configuração manual (sem entrada do usuário)
    // Navio 1: vertical
    int linhaV = 1, colunaV = 3, tamV = 4;
    Ponto navioV[16];
    gerar_navio_basico(linhaV, colunaV, tamV, 'V', navioV);

    // Navio 2: horizontal
    int linhaH = 6, colunaH = 2, tamH = 5;
    Ponto navioH[16];
    gerar_navio_basico(linhaH, colunaH, tamH, 'H', navioH);

    printf("Navio Vertical (tamanho %d) a partir de (%d,%d):\n", tamV, linhaV, colunaV);
    for (int i = 0; i < tamV; ++i) {
        printf("  parte %d -> (%d, %d)\n", i, navioV[i].linha, navioV[i].coluna);
    }

    printf("\nNavio Horizontal (tamanho %d) a partir de (%d,%d):\n", tamH, linhaH, colunaH);
    for (int i = 0; i < tamH; ++i) {
        printf("  parte %d -> (%d, %d)\n", i, navioH[i].linha, navioH[i].coluna);
    }
}

// ------------------------------ Nível Aventureiro ------------------------------

/*
 * Coloca um navio no tabuleiro:
 * orient:
 *    'H' = horizontal (cresce para a direita)
 *    'V' = vertical   (cresce para baixo)
 *    'D' = diagonal principal (↘, cresce linha+1, col+1)
 *    'A' = diagonal secundária (↙, cresce linha+1, col-1)
 * Marca as células com valor 3 (ocupado). Sem checagem de colisão para simplicidade.
 */
void colocar_navio(int board[N][N], int linhaInicio, int colunaInicio, int tamanho, char orient) {
    for (int k = 0; k < tamanho; ++k) {
        int r = linhaInicio;
        int c = colunaInicio;
        if (orient == 'H') {
            c += k;
        } else if (orient == 'V') {
            r += k;
        } else if (orient == 'D') { // diagonal principal ↘
            r += k; c += k;
        } else if (orient == 'A') { // diagonal secundária ↙
            r += k; c -= k;
        }
        if (r >= 0 && r < N && c >= 0 && c < N) {
            board[r][c] = 3;
        }
    }
}

void nivel_aventureiro_demo() {
    print_coords_header("Nível Aventureiro - Tabuleiro 10x10 com 4 navios (inclui diagonais)");

    int tab[N][N];
    memset(tab, 0, sizeof(tab));

    // Configuração manual dos navios (posições e tamanhos podem ser ajustados)
    colocar_navio(tab, 0, 0, 5, 'H'); // horizontal no topo
    colocar_navio(tab, 2, 7, 4, 'V'); // vertical à direita
    colocar_navio(tab, 4, 1, 4, 'D'); // diagonal principal (↘)
    colocar_navio(tab, 1, 8, 5, 'A'); // diagonal secundária (↙)

    // Exibição completa (0 = livre, 3 = ocupado)
    print_board_int(N, N, tab);
}

// ------------------------------ Nível Mestre (Habilidades) ------------------------------

/*
 * Todas as habilidades trabalham em uma matriz de 0/1 (não afetado/afetado).
 * Usamos clipping para bordas e loops aninhados.
 */

// Zera matriz de habilidades
void zerar_matriz(int rows, int cols, int m[rows][cols]) {
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            m[i][j] = 0;
}

/*
 * Cruz: afeta o centro e todos os elementos nas mesmas linha e coluna
 * dentro de um raio 'r'.
 */
void habilidade_cruz(int rows, int cols, int m[rows][cols], int centroL, int centroC, int r) {
    zerar_matriz(rows, cols, m);
    for (int d = -r; d <= r; ++d) {
        int lr = centroL + d;
        int lc = centroC + d; // não usamos lc aqui; apenas ilustrativo
        if (lr >= 0 && lr < rows) m[lr][centroC] = 1;
        int cc = centroC + d;
        if (cc >= 0 && cc < cols) m[centroL][cc] = 1;
    }
}

/*
 * Octaedro (diamante): |i - centroL| + |j - centroC| <= r
 */
void habilidade_octaedro(int rows, int cols, int m[rows][cols], int centroL, int centroC, int r) {
    zerar_matriz(rows, cols, m);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int manhattan = (i > centroL ? i - centroL : centroL - i) +
                            (j > centroC ? j - centroC : centroC - j);
            if (manhattan <= r) m[i][j] = 1;
        }
    }
}

/*
 * Cone apontando para baixo (triângulo isósceles):
 * altura = h; largura cresce de 1,3,5,... (2*linha+1) ao descer.
 * O topo (ápice) fica em (apexL, apexC).
 */
void habilidade_cone(int rows, int cols, int m[rows][cols], int apexL, int apexC, int h) {
    zerar_matriz(rows, cols, m);
    for (int t = 0; t < h; ++t) {
        int linha = apexL + t;
        if (linha < 0 || linha >= rows) continue;
        int half = t;                   // metade da largura crescente
        int ini = apexC - half;
        int fim = apexC + half;
        for (int j = ini; j <= fim; ++j) {
            if (j >= 0 && j < cols) {
                m[linha][j] = 1;
            }
        }
    }
}

void nivel_mestre_demo() {
    print_coords_header("Nível Mestre - Habilidades (0 = não afetado, 1 = afetado)");

    const int R = 5, C = 5; // exemplo compacto 5x5 para visual ficar próximo aos exemplos
    int H[R][C];

    // Cruz (raio 2) centrada em (2,2) -> parecido com o exemplo de cruz
    printf("\nHabilidade: CRUZ (centro 2,2; raio 2)\n");
    habilidade_cruz(R, C, H, 2, 2, 2);
    print_board_int(R, C, H);

    // Octaedro (raio 1) centrado em (1,2) -> lembra o exemplo de octaedro dado
    printf("\nHabilidade: OCTAEDRO (centro 1,2; raio 1)\n");
    habilidade_octaedro(R, C, H, 1, 2, 1);
    print_board_int(R, C, H);

    // Cone (altura 3) com ápice em (0,2) -> semelhante ao exemplo de cone
    printf("\nHabilidade: CONE (ápice 0,2; altura 3)\n");
    habilidade_cone(R, C, H, 0, 2, 3);
    print_board_int(R, C, H);
}

// ------------------------------ main ------------------------------

int main(void) {
    print_separator();
    nivel_novato_demo();

    print_separator();
    nivel_aventureiro_demo();

    print_separator();
    nivel_mestre_demo();

    print_separator();
    printf("Fim da demonstração.\n");
    return 0;
}
