#include <stdio.h>
#include <curses.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <unistd.h>
#include <math.h>
#include <string.h>


#define ROWS 190
#define COLS 40
#define Max_Rooms 80
#define Max_Traps 30
#define Max_Inventory 6
#define Max_Weapons 15
#define Max_Monsters 55
#define Max_Medkits 30


typedef struct Monsters {
    char *name;
    int damage;
    int x;
    int y;
    int vida;
} Monster;

typedef struct Weapons {
    char *name;
    int damage;
    int indexWeapon;
} Weapon;

typedef struct Wname {
    char *name;
} Name;

typedef struct Character {
    int x;
    int y;
    int damage;
    int vida;
    int xp;
    int level;
} Player;

Weapon weapons[] = {
    {"Sword", 4, 0},
    {"Axe", 6, 1},
    {"Pistol", 5, 2},
    {"Dagger", 3, 3},
    {"BOOM", 10, 4},
    {"FlASHBANG", 0, 5}
};
Name lista[Max_Inventory * (sizeof(Name))];

typedef struct Traps {
    int xa;
    int ya;
} Trap;

struct Room
{
    int posx;
    int posy;
    int width;
    int height;
};

struct Map
{
    char tela[ROWS][COLS];
    struct Room rooms[Max_Rooms];
    int num_rooms;
};

Player jogador;

int weapon_collected[Max_Inventory] = {0};

int isdead = 0; 

void spawn_monsters (struct Map *map, Monster *monstros, int j1, int j2) {
    //Esta função irá dar "spawn", alocar no mapa, monstros, que tentam atacar e matar jogador, de forma aleatória
    //Temos em conta informação já presente na posição do mapa onde queremos alocar, (!= '#' / == '.')
    int xaux;
    int yaux;
    for (int i = 0; i<Max_Monsters; i++) {
        while (1) {
            yaux = rand() % ROWS;
            xaux = rand() % COLS;
            if ((map->tela[yaux][xaux]) == '.' && (xaux != j1 || yaux != j2)) {
                monstros[i].x = xaux;
                monstros[i].y = yaux; 
                break;
            }
    }
    }
}

void move_monsterS (Monster *monstros, int player_x, int player_y, struct Map *map) {
    //Esta função, sempre que chamada, irá executar o movimento de cada monstro presente no mapa
for (int i=0; i<Max_Monsters; i++) {
    int monster_x = monstros[i].x;
    int monster_y = monstros[i].y;
    int dir_y;
    int dir_x;
    int ranx, rany;
    //Cálculo da distância entre o jogador e o montro
    int dx = player_x - monster_x;
    int dy = player_y - monster_y;
    double distance = sqrt(dx*dx + dy*dy);
    //Se o jogador estiver raio de atenção do monstro, o monstro mover-se-à na sua direção
    if (distance < 7.0) {
        //Cáculo da direção a mover
        if (dx < 0) {
            dir_x = -1;
        } 
        else if (dx > 0) {
            dir_x = 1;
        } 
        else {
            dir_x = 0;
        }

        if (dy < 0) {
            dir_y = -1;
        }
        else if (dy > 0) {
            dir_y = 1;
        }
        else {
            dir_y = 0;
        }  
        int new_x = monster_x + dir_x;
        int new_y = monster_y + dir_y;
        if (map->tela[new_y][new_x] != '#') {
            //Verificamos se o monstro pode alterar a sua posição antes de o fazer
            monstros[i].x = new_x;
            monstros[i].y = new_y;
        }
    }
    else {//Se os monstros não tiverem visto o jogador eles irão apenas mover-se de forma aleatória
        ranx = rand() % 2;
        rany = rand() % 2;
        if (ranx == 1 && map->tela[monstros[i].y][monstros[i].x+1] != '#') monstros[i].x+=1;
        else if (ranx == 0 && map->tela[monstros[i].y][monstros[i].x-1] != '#') monstros[i].x-=1;
        if (rany == 1 && map->tela[monstros[i].y+1][monstros[i].x] != '#') monstros[i].y+=1;
        else if (rany == 0 && map->tela[monstros[i].y-1][monstros[i].x] != '#') monstros[i].y-=1;
    }
}
}

void initialize_monster (Monster *monstros) {
    //Esta função irá, de forma aleatória, preencher a struct de monstros com um dos tipos de monstros criados
    int aux;
    for (int i=0; i<Max_Monsters; i++) {
        aux = rand() % 3;
        if (aux == 0) {
            monstros[i].name = "Minotaur";
            monstros[i].damage = 7;
            monstros[i].vida = 14;
        }
        else if (aux == 1) {
            monstros[i].name = "Goblin";
            monstros[i].damage = 2;
            monstros[i].vida = 5;
        }
        else {
            monstros[i].name = "Werewolf";
            monstros[i].damage = 4;
            monstros[i].vida = 8;
        }
    }

}

void warnings (Monster *monstros, int x, int y) {
    int sentence;
    //Esta função irá dar display, de forma aleatória, a mensagens de perigo para cada tipo de monstro
     for (int i=0; i<Max_Monsters; i++) {
        if (monstros[i].x > x-4 && monstros[i].x < x+4 && monstros[i].y < y + 4 && monstros[i].y > y-4 && strcmp(monstros[i].name, "Minotaur") == 0) {
            sentence = rand() % 3;
            if (sentence == 0) mvprintw (COLS + 4, 55, "Holy sh*t it's a MINOTAUR, you better run!!                         ");
       else if (sentence == 1) mvprintw (COLS + 4, 55, "Ahmm... A Minotaur is behind you, I'd start praying.                ");
                           else mvprintw(COLS + 4, 55, "It's a Minotaur, how bad can it be? Only 2 horns and an huge axe... ");
        }
        else if (monstros[i].x > x-4 && monstros[i].x < x+4 && monstros[i].y < y + 4 && monstros[i].y > y-4 && strcmp(monstros[i].name, "Goblin") == 0) {
            sentence = rand() % 3;
            if (sentence == 0) mvprintw (COLS + 4, 55, "Look at that little thief, there's a Goblin behind you.             ");
       else if (sentence == 1) mvprintw (COLS + 4, 55, "He's so small and green, Goblins almost look cute... and evil.      ");
                            else mvprintw(COLS + 4, 55,"It's a Goblin, if you kill him he might drop some gold.             ");
        }
        else if (monstros[i].x > x-4 && monstros[i].x < x+4 && monstros[i].y < y + 4 && monstros[i].y > y-4 && strcmp(monstros[i].name, "Werewolf") == 0) {
            sentence = rand() % 3;
            if (sentence == 0) mvprintw (COLS + 4, 55, "If I were you I wouldn't mess with a Werewolf!                      ");
       else if (sentence == 1) mvprintw (COLS + 4, 55, "Dogs use to be friendly, that one looks mad, is it a Werewolf?      ");
                           else mvprintw(COLS + 4, 55, "It's a Werewolf and you look like wolf meat, kill him or get killed!");
        }
        }
}
void combat_monsters (Monster *monstros, int playerdamage, int x, int y, int indicador, struct Map *map) {
    //Esta função irá traduzir os ataques do jogador aos montros
    //sempre que chamada tirará vida aos monstros se as condições de proximidade com o jogador forem cumpridas
    for (int i=0; i<Max_Monsters; i++) {
        if (monstros[i].x > x-2 && monstros[i].x < x+2 && monstros[i].y < y + 2 && monstros[i].y > y-2) {
            monstros[i].vida-= playerdamage;
        }
    } 
    if (strcmp(lista[indicador-1].name, "  BOOM") == 0 && x < COLS-2 && y < ROWS-2) {
        if (map->tela[y+1][x] == '#') map->tela[y+1][x] = '.';
        if (map->tela[y][x+1] == '#') map->tela[y][x+1] = '.';
        if (map->tela[y-1][x] == '#') map->tela[y-1][x] = '.';
        if (map->tela[y][x-1] == '#') map->tela[y][x-1] = '.';
        if (map->tela[y+1][x+1] == '#') map->tela[y+1][x+1] = '.';
        if (map->tela[y-1][x-1] == '#') map->tela[y-1][x-1] = '.';
        if (map->tela[y+1][x-1] == '#') map->tela[y+1][x-1] = '.';
        if (map->tela[y-1][x+1] == '#') map->tela[y-1][x+1] = '.';
    }
}

void kill_monsters (Monster *monstros, struct Map *map, int dificuldade) {
    //A função "kill_monsters" irá mostrar-nos graficamente que derrotamos um monstro, isto é, 
    //quando tiramos toda a vida de um monstro, e irá imediatamente dar-lhe respawn agora com melhores estatísticas, com mais vida
    int r1, r2;
    for (int i=0; i<Max_Monsters; i++) {
        if (monstros[i].vida <= 0) {
            while(1) {
                r1 = rand() % COLS;
                r2 = rand() % ROWS;
                if (map->tela[r2][r1] == '.') {
                    monstros[i].x = r1;//Respawn noutra posição livre do mapa
                    monstros[i].y = r2;
                    break;
                }
            }
            isdead = 1; //Indicador da derrota/morte de monstros
            monstros[i].vida = 12 + dificuldade * 2;
        }
    }
}

void spawn_traps (struct Map *map, Trap *armadilhas, int j1, int j2) {
    //Esta função irá dar "spawn", alocar no mapa, armadilhas, que condicionam a movimentação do jogador, de forma aleatória
    //Temos em conta informação já presente na posição do mapa onde queremos alocar, (!= '#' / == '.')
    int xaux;
    int yaux;
    for (int i = 0; i<Max_Traps; i++) {
        while (1) {
            yaux = rand() % ROWS;
            xaux = rand() % COLS;
            if ((map->tela[yaux][xaux]) == '.' && (xaux != j1 || yaux != j2)) {
                map->tela[yaux][xaux] = '^';
                armadilhas[i].xa = xaux;
                armadilhas[i].ya = yaux; 
                break;
            }
    }
    }
}

void spawn_weapon(int j1, int j2, struct Map *map) {
    //Esta função irá dar "spawn", alocar no mapa, armas, que podem ser coletadas e usadas pelo jogador, de forma aleatória
    //Temos em conta informação já presente na posição do mapa onde queremos alocar, (!= '#' / == '.')
    for (int w=0; w<Max_Weapons*3; w++) {
    int i = rand() % 6;
    int x = rand() % COLS;
    int y = rand() % ROWS;
        if (map->tela[y][x] == '.' && j1 != x && j2 != y && weapons[i].indexWeapon == 0) {
            map->tela[y][x] = 'S';
        }
        else if (map->tela[y][x] == '.' && j1 != x && j2 != y && weapons[i].indexWeapon == 1) {
            map->tela[y][x] = 'A';
        }
        else if (map->tela[y][x] == '.' && j1 != x && j2 != y && weapons[i].indexWeapon == 2) {
            map->tela[y][x] = 'P';
        }
        else if (map->tela[y][x] == '.' && j1 != x && j2 != y && weapons[i].indexWeapon == 3) {
            map->tela[y][x] = 'D';
        }
        else if (map->tela[y][x] == '.' && j1 != x && j2 != y && weapons[i].indexWeapon == 4) {
            map->tela[y][x] = 'B';
        }
        else if (map->tela[y][x] == '.' && j1 != x && j2 != y && weapons[i].indexWeapon == 5) {
            map->tela[y][x] = 'F';
        }
    }
    }

void spawn_medkits (struct Map *map, int j1, int j2) {
    //Esta função irá dar "spawn", alocar no mapa, "medkits", com objetivo de dar vida ao jogador, de forma aleatória
    //Temos em conta informação já presente na posição do mapa onde queremos alocar, (!= '#' / == '.')
    int xaux;
    int yaux;
    for (int i = 0; i<Max_Medkits; i++) {
        while (1) {
            yaux = rand() % ROWS;
            xaux = rand() % COLS;
            if ((map->tela[yaux][xaux]) == '.' && (xaux != j1 || yaux != j2)) {
                map->tela[yaux][xaux] = '+';
                break;
            }
    }
    }
}

 int get_medkitlife (struct Map *map, int j1, int j2) {
    //Esta função irá devolver um valor verdadeiro ,(1), se o jogador se encontrar nas mesmas coordenadas de um "medkit" 
    int v = 0;
    int n = 0;
    for (v = 0; v < COLS; v++) {
        for (n = 0; n < ROWS; n++) {
            if (map->tela[n][v] == '+' && n == j2 && v == j1) {
                return 1;
            }
        }
    }
    return 0;
 }

void collect_weapon(int x, int y, struct Map *map) {
    //Esta função é responsável pelo ato de coletar armas da parte do jogador
    //Ao coletar uma arma o seu nome será guardado num indíce de um array coincidente ao slot de inventário onde ficará
    //Se nesse slot/indíce a "weapon_collected[i] == 1", isto é, se o slot já tiver preenchido por outra arma, uma nova arma 
    //só poderá ser guardada noutro slot de indíce maior ou não será coletada de todo se o inventário estiver cheio
    //Depois de coletar a arma ela desaparece, ficando mais um espaço vazio no mapa
    for (int i = 0; i < Max_Inventory; i++) {
        if (weapon_collected[i] == 0 && (map->tela[y][x] == 'S')) {
            weapon_collected[i] = 1;
            map->tela[y][x] = '.';
            lista[i].name = "  Sword";
        }
        else if (weapon_collected[i] == 0 && (map->tela[y][x] == 'A')) {
            weapon_collected[i] = 1;
            map->tela[y][x] = '.';
            lista[i].name = "   Axe";
        }
        else if (weapon_collected[i] == 0 && (map->tela[y][x] == 'P')) {
            weapon_collected[i] = 1;
            map->tela[y][x] = '.';
            lista[i].name = "  Pistol";
        }
        else if (weapon_collected[i] == 0 && (map->tela[y][x] == 'D')) {
            weapon_collected[i] = 1;
            map->tela[y][x] = '.';
            lista[i].name = "  Dagger";
        }
        else if (weapon_collected[i] == 0 && (map->tela[y][x] == 'B')) {
            weapon_collected[i] = 1;
            map->tela[y][x] = '.';
            lista[i].name = "  BOOM";
        }
        else if (weapon_collected[i] == 0 && (map->tela[y][x] == 'F')) {
            weapon_collected[i] = 1;
            map->tela[y][x] = '.';
            lista[i].name = "FLASHBANG";
        }
    }
}

void print_weapons() {
    //Esta função recorre a um loop pelo array de coleção de armas à procura de índices, slots, que tenham sido já ocupados por armas
    for (int i = 0; i < Max_Inventory; i++) {
        if (weapon_collected[i] == 1) {
            // Dá print à arma em questão dentro do slot
            mvprintw(COLS+3, 131+i*10, "%s", lista[i].name);
         }
    }
}

void print_inventory() {
    //Esta função é simplesmente a representação gráfica e não atualizável do inventário, a sua estrutura
    init_pair(9, COLOR_CYAN, COLOR_BLACK);
    attron(COLOR_PAIR(9));
    mvprintw (COLS, 156, "INVENTORY");
    //Criação de caixas, slots
    for (int i=0; i < Max_Inventory; i++) {
        mvprintw(COLS+1, 130+i*10, "___________");
        mvprintw(COLS+2, 130+i*10, "|");
        mvprintw(COLS+3, 130+i*10, "|");
        mvprintw(COLS+4, 130+i*10, "|");
        mvprintw(COLS+4, 131+i*10, "___________");
    }
    //Teclas referentes a cada slot 
    mvprintw(COLS+5, 135, "Q");
    mvprintw(COLS+5, 145, "W");
    mvprintw(COLS+5, 155, "E");
    mvprintw(COLS+5, 165, "R");
    mvprintw(COLS+5, 175, "D");
    mvprintw(COLS+5, 185, "F");
    mvprintw(COLS+2, 190, "|");
    mvprintw(COLS+3, 190, "|");
    mvprintw(COLS+4, 190, "|");
    mvprintw(COLS+4, 191, " ");
    attroff(COLOR_PAIR(9));
}

void print_indicador (int ind) {
    //Esta função é um auxiliar gráfico para o jogador saber que slot está a usar
    //Cada indicador corresponde a um dos slots escolhidos pelo jogador
        if (ind == 1) {
            clear();
            attron(COLOR_PAIR(9));
            mvprintw(COLS+6, 135, "/\\");
            mvprintw(COLS+7, 135, "||");
            attroff(COLOR_PAIR(9));
        }
        else if (ind == 2) {
            clear();
            attron(COLOR_PAIR(9));
            mvprintw(COLS+6, 145, "/\\");
            mvprintw(COLS+7, 145, "||");
            attroff(COLOR_PAIR(9));
        }
        else if (ind == 3) {
            clear();
            attron(COLOR_PAIR(9));
            mvprintw(COLS+6, 155, "/\\");
            mvprintw(COLS+7, 155, "||");
            attroff(COLOR_PAIR(9));
        }
        else if (ind == 4) {
            clear();
            attron(COLOR_PAIR(9));
            mvprintw(COLS+6, 165, "/\\");
            mvprintw(COLS+7, 165, "||");
            attroff(COLOR_PAIR(9));
        }
        else if (ind == 5) {
            clear();
            attron(COLOR_PAIR(9));
            mvprintw(COLS+6, 175, "/\\");
            mvprintw(COLS+7, 175, "||");
            attroff(COLOR_PAIR(9));
        }
        else if (ind == 6) {
            clear();
            attron(COLOR_PAIR(9));
            mvprintw(COLS+6, 185, "/\\");
            mvprintw(COLS+7, 185, "||");
            attroff(COLOR_PAIR(9));
        }
}

void equippedNprint (int ind) {
    //Esta função irá analisar que armas o jogador está a equipar num dado slot
    //Se estiver efetivamente a equipar alguma arma vamos dar display a uma mensagem característica da mesma
    //bem como atualizar o dano que o jogador pode inferir agora com a arma equipada
    for (int i = 0; i < Max_Inventory; i++) {
        if (ind == i+1 && strcmp(lista[i].name, "  Sword") == 0) {
            mvprintw (COLS + 2, 55, "You are wielding a mighty sword!");
            jogador.damage = weapons[0].damage;
        }
    }
    for (int i = 0; i < Max_Inventory; i++) {
        if (ind == i+1 && strcmp(lista[i].name, "   Axe") == 0) {
            mvprintw (COLS + 2, 55, "The axe is in your hands, bring everything to the ground!");
            jogador.damage = weapons[1].damage;
        }
    }
    for (int i = 0; i < Max_Inventory; i++) {
        if (ind == i+1 && strcmp(lista[i].name, "  Pistol") == 0) {
            mvprintw (COLS + 2, 55, "You have a pistol! Use it like a true hitman!");
            jogador.damage = weapons[2].damage;
        }
    }
    for (int i = 0; i < Max_Inventory; i++) {
        if (ind == i+1 && strcmp(lista[i].name, "  Dagger") == 0) {
            mvprintw (COLS + 2, 55, "Rely on your dagger, be stealth and merge with it!");
            jogador.damage = weapons[3].damage;
        }
    }
    for (int i = 0; i < Max_Inventory; i++) {
        if (ind == i+1 && strcmp(lista[i].name, "  BOOM") == 0) {
            mvprintw (COLS + 2, 55, "It's a real bomb on your hands! Don't contain yourself, let it BOOM!");
            jogador.damage = weapons[4].damage;
        }
    }
    for (int i = 0; i < Max_Inventory; i++) {
        if (ind == i+1 && strcmp(lista[i].name, "FLASHBANG") == 0) {
            mvprintw (COLS + 2, 55, "Power of blinding it's in your hands, dont forget to look away!");
            jogador.damage = weapons[5].damage;
        }
    }
    for (int i = 0; i < Max_Inventory; i++) {
        if (ind == i+1 && strcmp(lista[i].name, "  Sword") == 0 && strcmp(lista[i].name, "   Axe") == 0 && strcmp(lista[i].name, "  Pistol") == 0 && strcmp(lista[i].name, "  Dagger") == 0 && strcmp(lista[i].name, "  BOOM") == 0 && strcmp(lista[i].name, "FLASHBANG") == 0) {
            jogador.damage = 1;
        } 
    }
}

void generate_map(struct Map *map) {
    //Esta função vai gerar o nosso mapa de forma aleatória
    int i, j, k, x, y, w, h;
    int num_rooms = 0;
    int max_attempts = 2000;
    int overlap;
    srand(time(NULL));

    //Inicializamos um mapa só com paredes
    for (i = 0; i < ROWS; i++)
    {
        for (j = 0; j < COLS; j++)
        {
            map->tela[i][j] = '#';
        }
    }

    //Gerador de salas (através de um Carving Method)
    while (num_rooms < Max_Rooms && max_attempts > 0)
    {
        //Gera as dimensões das salas
        w = rand() % 8 + 3;
        h = rand() % 6 + 2;
        x = rand() % (ROWS - w - 2) + 1;
        y = rand() % (COLS - h - 2) + 1;

        //Verifica se existe algum "overlap", isto é, alguma sobrepisoção com salas já existentes
        overlap = 0;
        for (k = 0; k < num_rooms; k++)
        {
            if (x <= map->rooms[k].posx + map->rooms[k].width + 1 &&
                x + w + 1 >= map->rooms[k].posx &&
                y <= map->rooms[k].posy + map->rooms[k].height + 1 &&
                y + h + 1 >= map->rooms[k].posy)
            {
                overlap = 1;
                break;
            }
        }

        //Se a condição de "overlap" não se verificar então podemos adicionar a sala atual ao mapa
        if (!overlap)
        {
            map->rooms[num_rooms].posx = x;
            map->rooms[num_rooms].posy = y;
            map->rooms[num_rooms].width = w;
            map->rooms[num_rooms].height = h;
            for (i = x + 1; i < x + w + 1; i++)
            {
                for (j = y + 1; j < y + h + 1; j++)
                {
                    map->tela[i][j] = '.';
                }
            }
            num_rooms++;
        }

        max_attempts--;
    }

    map->num_rooms = num_rooms;

    //Conexão entre as salas por túneis;
    for (i = 0; i < num_rooms - 1; i++)
    {
        int x1 = map->rooms[i].posx + map->rooms[i].width / 2;
        int y1 = map->rooms[i].posy + map->rooms[i].height / 2;
        int x2 = map->rooms[i + 1].posx + map->rooms[i + 1].width / 2;
        int y2 = map->rooms[i + 1].posy + map->rooms[i + 1].height / 2;

        while (x1 != x2 || y1 != y2)
        {
            if (x1 < x2)
            {
                x1++;
            }
            else if (x1 > x2)
            {
                x1--;
            }
            else if (y1 < y2)
            {
                y1++;
            }
            else if (y1 > y2)
            {
                y1--;
            }

            if (map->tela[x1][y1] == '#')
            {
                map->tela[x1][y1] = '.';
            }
        }
    }
}

int check_traps (Trap *traps, int i, int j) {
    //Esta função compara as coordendas de "traps", armadilhas, com dadas coordenadas "i" e "j";
    for (int x = 0; x < Max_Traps; x++) {
        if (i == traps[x].ya && j == traps[x].xa) {
            return 1;
        }
    }
    return 0;
}

int check_monsters (Monster *monstros, int i, int j) {
    //Esta função compara as coordenadas de monstros com dadas coordenadas "i" e "j";
    for (int x = 0; x < Max_Monsters; x++) {
        if (i == monstros[x].y && j == monstros[x].x) {
            return 1;
        }
    }
    return 0;
}

int check_monsters2 (Monster *monstros, int i, int j) {
    //Esta função compara as coordenadas de cada tipo monstro individualmente e se são coincidentes com "i" e "j";
    for (int x = 0; x < Max_Monsters; x++) {
        if (i == monstros[x].y && j == monstros[x].x && strcmp(monstros[x].name, "Minotaur") == 0) {
            return 1;
        }
        else if (i == monstros[x].y && j == monstros[x].x && strcmp(monstros[x].name, "Goblin") == 0) {
            return 2;
        }
        else if (i == monstros[x].y && j == monstros[x].x && strcmp(monstros[x].name, "Werewolf") == 0) {
            return 3;
        }
    }
    return 0;
}


int check_monsterdamage (Monster *monstros, int i, int j, int vida) {
    //Esta função será a nossa forma de atualizar a vida do jogador dado ter sido atacado por algum monstro
    //Será considerado atacado por algum monstro se o monstro se encontrar já "em cima" do jogador
    for (int x = 0; x < Max_Monsters; x++) {
        if (i == monstros[x].y && j == monstros[x].x) {
            vida-=monstros[x].damage;
            return vida;
        }
    }
    return vida;
}

void print_map(struct Map *map, int x, int y, Trap *traps, Monster *monstros) {
    int i, j;
    int raio = 7;
    // Esta função irá desenhar o nosso mapa bem como todos os integrantes do mesmo
    //Terá ainda em conta as condições de visibilidade do jogador
    for (j = 0; j < COLS; j++) {
        for (i = 0; i < ROWS; i++) {
            int dist = sqrt((i - y) * (i - y) + (j - x) * (j - x));
        if (check_traps (traps, y, x)) {
            raio = 3;//Quando em cima duma armadilha, o raio de visão será menor
        }
        else raio = 5;
            if (dist<=raio) {
                if (i == y && j == x) {
                    printw("@");
                }
                else if (check_monsters2 (monstros, i, j) == 1) {
                    printw("M");
                }
                else if (check_monsters2 (monstros, i, j) == 2) {
                    printw("G");
                }
                else if (check_monsters2 (monstros, i, j) == 3) {
                    printw("W");
                }
                else {
                    if (map->tela[i][j] == '#') {
                        init_pair(77, COLOR_BLACK, COLOR_MAGENTA);
                        attron(COLOR_PAIR(77));
                        mvprintw(j, i, " ");
                        attroff(COLOR_PAIR(77));
                    }
                    else {
                        printw("%c", map->tela[i][j]);
                    }
                }
            }
            else {
                init_pair(00, COLOR_BLACK, COLOR_BLACK);
                attron(COLOR_PAIR(00));
                mvprintw(j, i, " ");
                attroff(COLOR_PAIR(00));
            }
        }
        printw("\n");
    }
}

void print_menu()
{
    //Definir/associar cores
    start_color();
    init_pair(10, COLOR_GREEN, COLOR_BLACK);
    init_pair(27, COLOR_RED, COLOR_BLACK);

    //Estilo de letra
    attron(A_BOLD);

    //Print do menu inical de jogo
    clear();
    mvprintw(21, 85, "   Welcome to Brogue   ");
    mvprintw(22, 85, "Please select an option:");
    attron(COLOR_PAIR(10));
    attron(A_STANDOUT);
    mvprintw(24, 85, "  Press ENTER to Play  ");
    attroff(A_STANDOUT);
    attroff(COLOR_PAIR(10));
    attron(COLOR_PAIR(27));
    attron(A_STANDOUT);
    mvprintw(26, 85, "  Press ESC to Leave   ");
    attroff(A_STANDOUT);
    attroff(COLOR_PAIR(27));

    //Reseta o estilo de letra
    attroff(A_BOLD);
}

void print_stats(int vida, int xp, int level)
{
    //Definição das cores que usaremos para a vida e a experiência do jogador
    init_pair(1, COLOR_BLACK, COLOR_GREEN);
    init_pair(2, COLOR_BLACK, COLOR_YELLOW);
    init_pair(4, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_BLACK, COLOR_RED);
    init_pair(77, COLOR_BLACK, COLOR_MAGENTA);
    
    //Print da barra de vida, hp, health percentage
    attron(COLOR_PAIR(77));
    mvprintw(COLS + 2, 0, "HP");
    attroff(COLOR_PAIR(77));
    attroff(standout);
    if (vida > 13) {
    attron(COLOR_PAIR(1));
    for (int i = 0; i < vida; i++)
    {
        mvprintw(COLS + 2, 2 + 2 * i, "  ");
    }
    attroff(COLOR_PAIR(1));
    }
    else if (vida>7) {
        attron(COLOR_PAIR(2));
        for (int i = 0; i < vida; i++)
    {
        mvprintw(COLS + 2, 2 + 2 * i, "  ");
    }
    attroff(COLOR_PAIR(2));
    }
    else {
        attron(COLOR_PAIR(3));
        for (int i = 0; i < vida; i++)
    {
        mvprintw(COLS + 2, 2 + 2 * i, "  ");
    }
    attroff(COLOR_PAIR(3));
    }   

    //Print da barra de experiência
    attron(COLOR_PAIR(77));
    mvprintw(COLS + 4, 0, "XP");
    attroff(standout);
    attroff(COLOR_PAIR(77));
    attron(COLOR_PAIR(2));
    for (int i = 0; i < xp; i++)
    {
        mvprintw(COLS + 4, 2 + 3 * i, "   ");
    }
    attroff(COLOR_PAIR(2));
    attron(COLOR_PAIR(77));
    //Print do nível atual
    mvprintw(COLS+5, 0, "               LEVEL %d                ", level);
    attroff(standout);
    attroff(COLOR_PAIR(77));
}

void print_gameover(int level) {
    //Tela de "GAMEOVER", diz-nos que a nossa experiência de jogo chegou ao fim 
    attron(COLOR_PAIR(4));
    mvprintw(COLS / 2 - 1, ROWS / 2 - 29, "  _____          __  __ ______    ______      ________ _____  ");
    mvprintw(COLS / 2 , ROWS / 2 - 29, " / ____|   /\\   |  \\/  |  ____|  / __ \\ \\    / /  ____|  __ \\ ");
    mvprintw(COLS / 2 + 1, ROWS / 2 - 29, "| |  __   /  \\  | \\  / | |__    | |  | \\ \\  / /| |__  | |__) |");
    mvprintw(COLS / 2 + 2, ROWS / 2 - 29, "| | |_ | / /\\ \\ | |\\/| |  __|   | |  | |\\ \\/ / |  __| |  _  / ");
    mvprintw(COLS / 2 + 3, ROWS / 2 - 29, "| |__| |/ ____ \\| |  | | |____  | |__| | \\  /  | |____| | \\ \\ ");
    mvprintw(COLS / 2 + 4, ROWS / 2 - 29, " \\_____/_/    \\_\\_|  |_|______|  \\____/   \\/   |______|_|  \\_\\");
    attroff(standout);
    attroff(COLOR_PAIR(4));

    attron(COLOR_PAIR(4));
    mvprintw(COLS / 2 + 7, ROWS / 2 - 13, "You died! Better luck next time!");
    mvprintw(COLS / 2 + 9, ROWS / 2 - 13, "      You finished level %d!", level);
    attroff(standout);
    attroff(COLOR_PAIR(4));
    refresh();
}



int main()
{
    // "map" é o nosso mapa inicial, e que mostra por quais indíces de coordenadas é decorrido o jogo
    struct Map map;
    generate_map(&map);

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    }
    
    //Inicialização do áudio 
    Mix_Music *musicmenu = Mix_LoadMUS("menu.mp3"); // Música ambiente do menu
    Mix_Chunk *sound_enter = Mix_LoadWAV("enter.wav"); // Som ao clicar no Enter
    Mix_Chunk *sound_welcome = Mix_LoadWAV("welcome.wav"); // Som ao entrar no jogo
    Mix_Music *musicjogo = Mix_LoadMUS("jogo.mp3");    // Música ambiente do jogo
    Mix_Chunk *sound_pain = Mix_LoadWAV("dor.wav"); // Som de dor ou perda de vida, hp
    Mix_Chunk *sound_boom = Mix_LoadWAV("boom.wav"); // Som da "weapon" bomba, "BOOM"
    Mix_Chunk *sound_gameover = Mix_LoadWAV("gameover.wav"); // Som de final de jogo, Game Over

    Mix_PlayMusic(musicmenu, -1); 
    // Esta música/som ambiente irá tocar infinatamente até ler uma indicação de paragem

    // Definimos aqui a seed para o criar um número random
    srand(time(NULL));

    // Coordenadas inciais, aleatoriamente geradas, do jogador;
    Player jogador;
    while (1) {
        jogador.y = rand() % ROWS;
        jogador.x = rand() % COLS;
        if ((map.tela[jogador.y][jogador.x]) == '.') {
            break;
        }
    }

    // Vida inicial do jogador
    jogador.vida = 25;
    // Jogador começa no nível 0
    jogador.level = 0;
    // Contador de xp que condiciona o level
    jogador.xp = 0;
    // O dano que o jogador dá, sem qualquer arma não é muito
    jogador.damage = 1;

    // Inicialização das armadilhas
    Trap *traps = malloc(Max_Traps*sizeof(Trap));
    spawn_traps(&map, traps, jogador.x, jogador.y);

    // Inicialização dos monstros
    Monster *monsters = malloc (Max_Monsters * sizeof(Monster));
    spawn_monsters (&map, monsters, jogador.x, jogador.y);
    initialize_monster (monsters);

    for (int i = 0; i < Max_Inventory; i++) {
    lista[i].name = "default"; //Inicialização da lista que guarda as armas apanhadas
    }
    spawn_weapon(jogador.x, jogador.y, &map);

    spawn_medkits(&map, jogador.x, jogador.y);

    // Necessário para inicializar a biblioteca "curses"    
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    nodelay(stdscr, TRUE);

    // Jogo começa no menu
    print_menu();
    int choice = 0;
    while (1)
    { // Loop para garantir que outras teclas não interfiram com o programa;
        choice = getch();
        if (choice == 10)
        { // Tecla ENTER para Jogar
            Mix_PlayChannel(-1, sound_enter, 0); // Este efeito sonoro vai ser tocado uma vez
            SDL_Delay(1000); // Delay de 1 segundo
            Mix_PlayChannel(-1, sound_welcome, 0);// Este efeito sonoro vai ser tocado uma vez com um delay de 1000ms (1segundo)
            Mix_FreeMusic(musicmenu); // A música ambiente do menu vai parar de tocar
            break;
        }
        else if (choice == 27)
        { // Tecla "ESC" para Sair
            endwin();
            return 0;
        }
    }
    clear();
    int indicador = 1;
    int clock1 = 0;
    int clock2 = 0;
    int clock3 = 0;
    int clock4 = 0;
    int clock5 = 0;
    int dificuldade = 0;
    Mix_PlayMusic(musicjogo, -1); // Esta música/som ambiente irá tocar infinatamente até ler uma indicação de paragem 
    // Main loop do jogo que permite que o jogo continue a ser jogado até que algo dentro deste loop o interrompa
    while (1) {
        print_map(&map, jogador.x, jogador.y, traps, monsters); //Desenha o mapa de jogo bem como o jogador e outros integrantes
        print_stats(jogador.vida, jogador.xp, jogador.level);//Print dos dados referentes ao jogador, a sua vida, xp e nível
        collect_weapon(jogador.x,jogador.y, &map);//Controla as armas apanhadas pelo jogador
        print_weapons(); //Print das armas apanhadas pelo jogador nos seus devidos slots
        print_inventory();//Print da estrutura gráfica de inventário
        if (clock5 >= 1900) {//Dentro de um intervalo de tempo moderado surgem "warnings" referentes às redondezas do jogador
            warnings(monsters, jogador.x, jogador.y);
            clock5=0;
        }
        else clock5++;
        if (clock1 == 550) {//Dentro de um intervalo de tempo controlado e jogável os monstros movem-se
            move_monsterS(monsters, jogador.x, jogador.y, &map);
            clock1 = 0;
        }
        else clock1++;
        init_pair(9, COLOR_CYAN, COLOR_BLACK);
        int ch = getch(); // Recebe o input para mover o jogador, que serão os números do Numpad
        // Interpreta depois esse input e atualiza as coordendas através destes if statements que têm em atenção as paredes
        if (ch != ERR) {
        if ((ch == 52) && (map.tela[jogador.y - 1][jogador.x] != '#')) {
            jogador.y--; // Esquerda
        }
        else if ((ch == 54) && (map.tela[jogador.y + 1][jogador.x] != '#')) {
            jogador.y++; // Direita
        }
        else if ((ch == 56) && (map.tela[jogador.y][jogador.x - 1] != '#')) {
            jogador.x--; // Cima
        }
        else if ((ch == 50) && (map.tela[jogador.y][jogador.x + 1] != '#')) {
            jogador.x++; // Baixo
        }
        else if ((ch == 55) && (map.tela[jogador.y - 1][jogador.x - 1] != '#')) {
            jogador.y--; //Esquerda e cima
            jogador.x--;
        }
        else if ((ch == 49) && (map.tela[jogador.y - 1][jogador.x + 1] != '#')) {
            jogador.y--; //Esquerda e baixo
            jogador.x++;
        }
        else if ((ch == 51) && (map.tela[jogador.y + 1][jogador.x + 1] != '#')) {
            jogador.x++; //Direita e baixo
            jogador.y++;
        }
        else if ((ch == 57) && (map.tela[jogador.y + 1][jogador.x - 1] != '#')) {
            jogador.x--; //Direita e cima
            jogador.y++;
        }

        else if (ch == 27) { //Tecla "ESC"
            endwin(); // Ao premir a tecla "ESC" o jogo vai ser interrompido
            return 0;
        }

        else if (ch == 113) {//Tecla "q"
        //Sempre que clicada uma das teclas referentes ao inventário o indicador referente ao slot pretendido
        //será atualiazdo e serão chamadas as funções com a informação gráfica referente ao slot e ao que o mesmo contém
            indicador = 1;
            print_indicador(indicador);
            equippedNprint(indicador);
        }
        else if (ch == 119) {//Tecla "w"
            indicador = 2;
            print_indicador(indicador);
            equippedNprint(indicador);
        }
        else if (ch == 101) {//Tecla "e"
            indicador = 3;
            print_indicador(indicador);
            equippedNprint(indicador);
        }
        else if (ch == 114) {//Tecla "r"
            indicador = 4;
            print_indicador(indicador);
            equippedNprint(indicador);
        }
        else if (ch == 100) {//Tecla "d"
            indicador = 5;
            print_indicador(indicador);
            equippedNprint(indicador);
        }
        else if (ch == 102) {//Tecla "f"
            indicador = 6;
            print_indicador(indicador);
            equippedNprint(indicador);
        }
        else if (ch == 32) { //Premida a tecla "espaço" o jogador ataca qualquer monstro na sua proximidade
                if (clock4 >= 500) {//Note-se que não adianta ao jogador clicar na tecla incessassantemente pois é obrigado a esperar
                    combat_monsters(monsters, jogador.damage, jogador.x, jogador.y, indicador, &map);//um pequeno intervalo de tempo entre ataques
                    clock4=0;
                    if (strcmp(lista[indicador-1].name, "  BOOM") == 0) { //Se a bomba estiver nas mãos do jogador
                        Mix_PlayChannel(-1, sound_boom, 0); //Este efeito sonoro de dor vai ser tocado uma vez
                    }
                }
        }
        }
        clock4++;
        if (get_medkitlife(&map, jogador.x, jogador.y) && jogador.vida != 25) {
            //Ao pisar um "medkit", se a vida do jogador ainda não estiver completa, recebida vida
            jogador.vida+=1;
            map.tela[jogador.y][jogador.x] = '.';
            //Faz com que o medkit desapareca do mapa, substituindo-o no mapa pelo vazio (o ponto)
        }
        if (clock2 >= 250) {
            //Verificamos num intervalo de tempo moderado se o jogador está a pisar uma armadilha
            if (check_traps(traps, jogador.y, jogador.x)) { 
                jogador.vida--; //Se sim, perde vida
                clear();
                Mix_PlayChannel(-1, sound_pain, 0); //Este efeito sonoro de dor vai ser tocado uma vez
            }
            clock2=0;
        }
        else clock2++;
        if (clock3 >= 400) {
            if (check_monsters(monsters, jogador.y, jogador.x)) {
                //A partir de um intervalo de tempo moderado vamos perceber se está algum monstro nas mesmas posições
                //do nosso jogador, pronto para atacar 
                jogador.vida = check_monsterdamage(monsters, jogador.y, jogador.x, jogador.vida);
                //Atualiza a vida do jogador com base no dano que recebeu
                clear();
                Mix_PlayChannel(-1, sound_pain, 0); //Este efeito sonoro de dor vai ser tocado uma vez
            }
            clock3=0;
        }
        else clock3++;
        kill_monsters(monsters, &map, dificuldade);
        if (isdead == 1) {//A variável "isdead" vai ter um valor 1 sempre que um monstro for morto;
            jogador.xp +=3;//É ganho "xp" sempre que um monstros é morto
            isdead = 0;
            dificuldade++;//Sempre que um monstro é morto a dificuldade do jogo aumenta
            if (jogador.xp > 12) {
                jogador.level++;
                jogador.xp = 0;
                clear();
            }
        }
        if (jogador.vida <= 0) {
            //Sinalizamos aqui a morte do jogador quando o mesmo perde toda a vida, o ecrã de jogo é limpo e saímos do loop de jogo
            clear();
            break;
        }
    }
    Mix_PlayChannel(-1, sound_gameover, 0); //Este efeito sonoro de dor vai ser tocado uma vez
    while (1) {
                print_gameover(jogador.level); //Repesentação gráfica de ter perdido o jogo
                int ch1 = getch();
                if (ch1 == 27) { //Tecla "ESC"
                    endwin();   //Ao premir a tecla "ESC" o jogo vai ser interrompido
                    return 0;
                }
            }
    return 0;
}



