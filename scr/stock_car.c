
// ----------------------- STOCK CAR ------------------------- //
// Jogo de corrida simples para microcontrolador AT89C52 com GLCD
// Desenvolvido por: Lucas Paixão Cruz de Castro
// Data: Julho de 2025  
// Descrição: Jogo de corrida onde o jogador controla um carro por meio de dois botões e deve desviar de obstáculos.
// ----------------------------------------------------------- //

// --------------------------------- //
// --- Declarações de variáveis --- //
// -------------------------------- //

// Importando Bibliotecas necessárias
#include <reg51.h>
#include <stdlib.h>

// Definições de hardware
#define GlcdDataBus  P3 // Porta de dados do GLCD
#define NUM_LINHAS 8 // Número de linhas do GLCD (8 páginas)
#define LARGURA_PISTA 48 // Largura da pista (48 colunas)
#define LARGURA_CARRO 6 // Largura do carro (6 colunas)
#define LARGURA_OBSTACULO 7 // Largura do obstáculo (7 colunas)
#define MAX_OBSTACULOS 1 // Máximo de obstáculos na tela em um ciclo

// Definições de pinos no microcontrolador
sbit RS = P2^0;
sbit RW = P2^1;
sbit EN = P2^2;
sbit CS1 = P2^3;
sbit CS2 = P2^4;
sbit RST = P2^5;

// Botões
sbit BTN_DIREITA = P1^0;
sbit BTN_ESQUERDA = P1^1;

// Variáveis globais
unsigned char posicoes_pista[NUM_LINHAS]; // Posições horizontais da pista
unsigned char posicao_carro = 61;  // Posição inicial do carro (central)
unsigned char obstaculos_x[MAX_OBSTACULOS];  // Posições horizontais (0 a 127)
unsigned char obstaculos_y[MAX_OBSTACULOS];  // Posições verticais (0 a 7)
unsigned int delay_jogo = 80000; // Delay do jogo (inicialmente 80.000 ms)
unsigned int contador_frames = 0; // Contador de frames para aumentar a velocidade do jogo

// Sprites
const char code num_3[] = {0x21, 0x41, 0x45, 0x4B, 0x31, 0x00}; // "3"
const char code num_2[] = {0x42, 0x61, 0x51, 0x49, 0x46, 0x00}; // "2"
const char code num_1[] = {0x00, 0x21, 0x7F, 0x01, 0x00, 0x00}; // "1"
const char code letra_G[] = {0x3E, 0x41, 0x49, 0x49, 0x3A, 0x00}; // "G"
const char code letra_O[] = {0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00}; // "O"
const char code letra_exclamacao[] = {0x00, 0x00, 0x5F, 0x00, 0x00, 0x00}; // "!"
const char code carro[] = {0xE7,0x42,0xFF,0xFF,0x42,0xE7}; // Carro principal 6 colunas
const char code carro_2[] = {0xE7,0x42,0xFF,0xE7,0xFF,0x42,0xE7}; // Carro secundário 7 colunas

// ------------------------------ //
// ----- Funções auxiliares ----- //
// ------------------------------ //

// Delay simples
void delay(int d) {
    while(d--);
}

// GLCD funções básicas
void Glcd_SelectPage0() { CS1 = 0; CS2 = 1; }
void Glcd_SelectPage1() { CS1 = 1; CS2 = 0; }

void Glcd_CmdWrite(unsigned char cmd) {
    GlcdDataBus = cmd;
    RS = 0; RW = 0; EN = 1; EN = 0;
}

void Glcd_DataWrite(unsigned char dat) {
    GlcdDataBus = dat;
    RS = 1; RW = 0; EN = 1; EN = 0;
}

void Glcd_Clear() {
    unsigned char page, col;
    for (page = 0; page < 8; page++) {
        Glcd_SelectPage0(); Glcd_CmdWrite(0xB8 | page); Glcd_CmdWrite(0x40);
        for (col = 0; col < 64; col++) Glcd_DataWrite(0x00);
        Glcd_SelectPage1(); Glcd_CmdWrite(0xB8 | page); Glcd_CmdWrite(0x40);
        for (col = 0; col < 64; col++) Glcd_DataWrite(0x00);
    }
}

void Glcd_Init() {
    Glcd_SelectPage0(); Glcd_CmdWrite(0x3F);
    Glcd_SelectPage1(); Glcd_CmdWrite(0x3F);
    Glcd_Clear();
}

void Glcd_DisplayChar4x6(const char *ptr)
{
    int i;
    for (i = 0; i < 4; i++)
        Glcd_DataWrite(ptr[i]);
    Glcd_DataWrite(0x00); // espaço entre caracteres
}

void Glcd_Print6x8(const char *letra, int col_start, int page) {
    int i;
    for (i = 0; i < 6; i++) {
        if (col_start + i < 64) {
            Glcd_SelectPage0();
            Glcd_CmdWrite(0xB8 | page);
            Glcd_CmdWrite(0x40 | (col_start + i));
            Glcd_DataWrite(letra[i]);
        } else {
            Glcd_SelectPage1();
            Glcd_CmdWrite(0xB8 | page);
            Glcd_CmdWrite(0x40 | ((col_start + i) - 64));
            Glcd_DataWrite(letra[i]);
        }
    }
}

void Glcd_Clear6x8(int col_start, int page) {
    int i;
    for (i = 0; i < 6; i++) {
        if (col_start + i < 64) {
            Glcd_SelectPage0();
            Glcd_CmdWrite(0xB8 | page);
            Glcd_CmdWrite(0x40 | (col_start + i));
            Glcd_DataWrite(0x00);
        } else {
            Glcd_SelectPage1();
            Glcd_CmdWrite(0xB8 | page);
            Glcd_CmdWrite(0x40 | ((col_start + i) - 64));
            Glcd_DataWrite(0x00);
        }
    }
}

void Contador_Largada() {
    int col;
    int page;
    int total_largura;
    int col_go;

    col = (128 - 6) / 2; // centraliza caractere 6 colunas
    page = 3;

    // 3
    Glcd_Print6x8(num_3, col, page);
    delay(100000);
    Glcd_Clear6x8(col, page);

    // 2
    Glcd_Print6x8(num_2, col, page);
    delay(100000);
    Glcd_Clear6x8(col, page);

    // 1
    Glcd_Print6x8(num_1, col, page);
    delay(100000);
    Glcd_Clear6x8(col, page);

    // G O !
    total_largura = 3 * 6;
    col_go = (128 - total_largura) / 2;

    Glcd_Print6x8(letra_G, col_go, page);
    Glcd_Print6x8(letra_O, col_go + 6, page);
    Glcd_Print6x8(letra_exclamacao, col_go + 12, page);

    delay(100000);

    Glcd_Clear6x8(col_go, page);
    Glcd_Clear6x8(col_go + 6, page);
    Glcd_Clear6x8(col_go + 12, page);
}

// Desenha bordas verticais da pista
void desenhar_bordas(unsigned char page, unsigned char x_borda_esq) {
    unsigned char x_borda_dir = x_borda_esq + LARGURA_PISTA;

    if (x_borda_esq < 64) {
        Glcd_SelectPage0(); Glcd_CmdWrite(0xB8 | page); Glcd_CmdWrite(0x40 | x_borda_esq);
        Glcd_DataWrite(0xFF);
    } else {
        Glcd_SelectPage1(); Glcd_CmdWrite(0xB8 | page); Glcd_CmdWrite(0x40 | (x_borda_esq - 64));
        Glcd_DataWrite(0xFF);
    }

    if (x_borda_dir < 64) {
        Glcd_SelectPage0(); Glcd_CmdWrite(0xB8 | page); Glcd_CmdWrite(0x40 | x_borda_dir);
        Glcd_DataWrite(0xFF);
    } else {
        Glcd_SelectPage1(); Glcd_CmdWrite(0xB8 | page); Glcd_CmdWrite(0x40 | (x_borda_dir - 64));
        Glcd_DataWrite(0xFF);
    }
}

// Desenha o carro na linha 7
void desenhar_carro(unsigned char x) {
    unsigned char i;
    unsigned char chip, pos;

    for (i = 0; i < LARGURA_CARRO; i++) {
        unsigned char col = x + i;
        chip = (col < 64) ? 0 : 1;
        pos = (col < 64) ? col : (col - 64);

        if (chip == 0) Glcd_SelectPage0();
        else Glcd_SelectPage1();

        Glcd_CmdWrite(0xB8 | 7);    // Linha 7 (última)
        Glcd_CmdWrite(0x40 | pos);  // Posição horizontal
        Glcd_DataWrite(carro[i]);
    }
}

// Inicializa pista centralizada
void inicializar_pista() {
    unsigned char i;
    unsigned char centro = (127 - LARGURA_PISTA) / 2;
    for (i = 0; i < NUM_LINHAS; i++) {
        posicoes_pista[i] = centro;
    }
}

// Faz scroll da pista (apenas atualiza posições)
void scroll_pista() {
    unsigned char i;
    unsigned char nova_pos;

    for (i = NUM_LINHAS - 1; i > 0; i--) {
        posicoes_pista[i] = posicoes_pista[i - 1];
    }

    nova_pos = posicoes_pista[0];
    if ((rand() % 2) && nova_pos < (127 - LARGURA_PISTA)) nova_pos++;
    else if (nova_pos > 1) nova_pos--;
    posicoes_pista[0] = nova_pos;
}

// Desenha a pista inteira
void desenhar_pista() {
    unsigned char i;
    for (i = 0; i < NUM_LINHAS; i++) {
        desenhar_bordas(i, posicoes_pista[i]);
    }
}

// Atualiza posição do carro com base nos botões
void atualizar_carro() {
    if (!BTN_DIREITA && posicao_carro < (127 - LARGURA_CARRO))
        posicao_carro++;
    if (!BTN_ESQUERDA && posicao_carro > 0)
        posicao_carro--;

    desenhar_carro(posicao_carro);
}

bit verificar_colisao() {
    unsigned char x_borda = posicoes_pista[7]; // posição da pista na linha 7
    unsigned char x_fim_borda = x_borda + LARGURA_PISTA;

    if (posicao_carro < x_borda || (posicao_carro + LARGURA_CARRO - 1) > x_fim_borda) {
        return 1; // Colidiu
    }
    return 0; // Seguro
}

void inicializar_obstaculos() {
    unsigned char i;
    for (i = 0; i < MAX_OBSTACULOS; i++) {
        obstaculos_y[i] = 255; // 255 = inativo
    }
}

void gerar_obstaculo() {
    unsigned char i;
    for (i = 0; i < MAX_OBSTACULOS; i++) {
        if (obstaculos_y[i] == 255) {
            // Gera dentro da pista na linha 0
            unsigned char x_borda = posicoes_pista[0];
            obstaculos_x[i] = x_borda + 2 + (rand() % (LARGURA_PISTA - LARGURA_OBSTACULO - 4));
            obstaculos_y[i] = 0;
            break;
        }
    }
}

void atualizar_obstaculos() {
    unsigned char i;
    for (i = 0; i < MAX_OBSTACULOS; i++) {
        if (obstaculos_y[i] < NUM_LINHAS) {
            obstaculos_y[i]++;
            if (obstaculos_y[i] >= NUM_LINHAS) {
                obstaculos_y[i] = 255; // Apaga se saiu da tela
            }
        }
    }

    // Chance aleatória de gerar novo obstáculo
    if (rand() % 5 == 0) {
        gerar_obstaculo();
    }
}

void desenhar_obstaculos() {
    unsigned char i, j;
    for (i = 0; i < MAX_OBSTACULOS; i++) {
        if (obstaculos_y[i] < NUM_LINHAS) {
            unsigned char y = obstaculos_y[i];
            unsigned char x = obstaculos_x[i];

            for (j = 0; j < LARGURA_OBSTACULO; j++) {
                unsigned char col = x + j;
                unsigned char chip = (col < 64) ? 0 : 1;
                unsigned char pos = (col < 64) ? col : (col - 64);

                if (chip == 0) Glcd_SelectPage0();
                else Glcd_SelectPage1();

                Glcd_CmdWrite(0xB8 | y);        // Página
                Glcd_CmdWrite(0x40 | pos);      // Posição horizontal
                Glcd_DataWrite(carro_2[j]);     // Desenha
            }
        }
    }
}

bit verificar_colisao_obstaculos() {
    unsigned char i;
    for (i = 0; i < MAX_OBSTACULOS; i++) {
        if (obstaculos_y[i] == 7) {
            // Linha do carro
            if ((posicao_carro + LARGURA_CARRO - 1) >= obstaculos_x[i] &&
                posicao_carro <= (obstaculos_x[i] + LARGURA_OBSTACULO - 1)) {
                return 1; // Colidiu
            }
        }
    }
    return 0;
}

void game_over() {
    Glcd_Clear();
    delay(60000);
    posicao_carro = 61;
    inicializar_pista();
    inicializar_obstaculos();
    delay_jogo = 80000;         // Reinicia velocidade
    contador_frames = 0;        // Reinicia contador
}

// ------------------------------- //
// ------- Loop principal ------- //
// ------------------------------- //

void main() {
    Glcd_Init();
    P1 = 0xFF;
		Contador_Largada();
    inicializar_pista();
    inicializar_obstaculos();

    while (1) {
        Glcd_Clear();
        scroll_pista();
        atualizar_obstaculos();
        desenhar_pista();
        desenhar_obstaculos();
        atualizar_carro();

        if (verificar_colisao() || verificar_colisao_obstaculos()) {
            game_over();
        }

        delay(delay_jogo);

				contador_frames++;
				if (contador_frames >= 100 && delay_jogo >= 20000) {
					delay_jogo -= 5000;        // Aumenta a velocidade
					contador_frames = 0;       // Reinicia o contador
                };
    }
}