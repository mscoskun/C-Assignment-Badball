#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <conio.h> // For keyboard control (Windows/Dev-C++ standard)
#include <windows.h>

// OS compatibility (Custom cursor settings for Windows)
#ifdef _WIN32
#include <windows.h>
void wait_ms(int ms) { Sleep(ms); }

void hide_cursor() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = 0; // Disable visibility
    SetConsoleCursorInfo(hOut, &cursorInfo);
}
#else
#include <unistd.h>
void wait_ms(int ms) { usleep(ms * 1000); }
void hide_cursor() { printf("\033[?25l"); }
#endif


#define FIELD_WIDTH 80
#define FIELD_HEIGHT 20
#define PLAYER_COUNT 8 
#define REAL_SEC_PER_MINUTE 1.5
#define FPS 60 // 60 Frames/Second 
#define COLLISION_DISTANCE 2.0
#define SCREEN_BUFFER_SIZE 65536

char screen_buffer[SCREEN_BUFFER_SIZE];

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

typedef struct {
    double x;
    double y;
    double vx;
    double vy;
} Ball;

typedef enum { goalkeeper, defender, midfielder, forward } Role;

typedef struct {
    int team_id;
    int id;
    int jersey_no;
    double x, y;
    Role role;
} Player;

Player Blue_team[PLAYER_COUNT];
Player Red_team[PLAYER_COUNT];
Ball ball;
int score_b = 0;
int score_r = 0;
int passes_b = 0;
int passes_r = 0;
int shots_b = 0, shots_r = 0, shots_on_target_r = 0, shots_on_target_b = 0;
char commentator_text[100] = "Match is starting! Press [ESC] to exit.";

double calculate_distance(double x1, double y1, double x2, double y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

void init_teams() {
    int i;
    Blue_team[0].x = 3;  Blue_team[0].y = 10; Blue_team[0].role = goalkeeper;
    Blue_team[1].x = 15; Blue_team[1].y = 5;  Blue_team[1].role = defender;
    Blue_team[2].x = 15; Blue_team[2].y = 10; Blue_team[2].role = defender;
    Blue_team[3].x = 15; Blue_team[3].y = 15; Blue_team[3].role = defender;
    Blue_team[4].x = 35; Blue_team[4].y = 8;  Blue_team[4].role = midfielder;
    Blue_team[5].x = 35; Blue_team[5].y = 12; Blue_team[5].role = midfielder;
    Blue_team[6].x = 48; Blue_team[6].y = 9;  Blue_team[6].role = forward;
    Blue_team[7].x = 48; Blue_team[7].y = 11; Blue_team[7].role = forward;

    Red_team[0].x = 76; Red_team[0].y = 10; Red_team[0].role = goalkeeper;
    Red_team[1].x = 64; Red_team[1].y = 5;  Red_team[1].role = defender;
    Red_team[2].x = 64; Red_team[2].y = 10; Red_team[2].role = defender;
    Red_team[3].x = 64; Red_team[3].y = 15; Red_team[3].role = defender;
    Red_team[4].x = 44; Red_team[4].y = 8;  Red_team[4].role = midfielder;
    Red_team[5].x = 44; Red_team[5].y = 12; Red_team[5].role = midfielder;
    Red_team[6].x = 31; Red_team[6].y = 9;  Red_team[6].role = forward;
    Red_team[7].x = 31; Red_team[7].y = 11; Red_team[7].role = forward;
    
    for (i = 0; i < PLAYER_COUNT; i++) {
        Blue_team[i].id = i; Blue_team[i].team_id = 0; Blue_team[i].jersey_no = i + 1;
        Red_team[i].id = i; Red_team[i].team_id = 1; Red_team[i].jersey_no = i + 1;
    }
}
void init_ball() {
    ball.x = FIELD_WIDTH / 2.0; ball.y = FIELD_HEIGHT / 2.0; ball.vx = 0; ball.vy = 0;
}

int is_position_available(double x, double y, int ignore_id, int current_team_id) {
    int i;
    double d;
    for (i = 0; i < PLAYER_COUNT; i++) {
        if (current_team_id == 0 && Blue_team[i].id == ignore_id) continue;
        d = calculate_distance(x, y, Blue_team[i].x, Blue_team[i].y);
        if (d < COLLISION_DISTANCE) return 0; 
    }
    for (i = 0; i < PLAYER_COUNT; i++) {
        if (current_team_id == 1 && Red_team[i].id == ignore_id) continue;
        d = calculate_distance(x, y, Red_team[i].x, Red_team[i].y);
        if (d < COLLISION_DISTANCE) return 0; 
    }
    return 1;
}


void move_towards(Player *p, double target_x, double target_y, double speed) {
    double dx = (target_x - p->x);
    double dy = (target_y - p->y);
    double d = sqrt((dx * dx) + (dy * dy));
    double frame = speed * 0.4;
    
    
    if (d > 0.5) {
        double vx = (dx / d) * frame;
        double vy = (dy / d) * frame;
        
        double new_x = p->x + vx;
        double new_y = p->y + vy;

        if (is_position_available(new_x, new_y, p->id, p->team_id)) {
            p->x = new_x;
            p->y = new_y;
        } else {
           int r = rand() % 4; // 0:Up, 1:Down, 2:Left, 3:Right
            int has_moved = 0;

            
            if (r == 0 && is_position_available(p->x, p->y - frame, p->id, p->team_id)) { p->y -= frame; has_moved=1; }
            else if (r == 1 && is_position_available(p->x, p->y + frame, p->id, p->team_id)) { p->y += frame; has_moved=1; }
            else if (r == 2 && is_position_available(p->x - frame, p->y, p->id, p->team_id)) { p->x -= frame; has_moved=1; }
            else if (r == 3 && is_position_available(p->x + frame, p->y, p->id, p->team_id)) { p->x += frame; has_moved=1; }
            
            
            if (!has_moved) {
                if (is_position_available(p->x, p->y - frame, p->id, p->team_id)) p->y -= frame;      // Up
                else if (is_position_available(p->x + frame, p->y, p->id, p->team_id)) p->x += frame; // Right
                else if (is_position_available(p->x - frame, p->y, p->id, p->team_id)) p->x -= frame; // Left
                else if (is_position_available(p->x, p->y + frame, p->id, p->team_id)) p->y += frame;} // Down (Last choice)
        }
    }
    if (p->x < 1.0) p->x = 1.0;
    if (p->x > FIELD_WIDTH - 2.0) p->x = FIELD_WIDTH - 2.0;
    if (p->y < 1.0) p->y = 1.0;
    if (p->y > FIELD_HEIGHT - 2.0) p->y = FIELD_HEIGHT - 2.0;
}

void ai_logic(Player *p, Ball *b) {
    double dist_ball = calculate_distance(p->x, p->y, b->x, b->y);
    double target_x = p->x, target_y = p->y;
    double speed = 1.0; 
    double dist_goal;
    if (p->team_id == 0) {
        dist_goal = fabs(p->x - (FIELD_WIDTH - 1));}
    else {
        dist_goal = fabs(p->x - 0);}
    
    if (p->role == goalkeeper) {
        if(dist_ball < 1.1) {
            if (p->team_id == 0) {
                b->x = p->y + 0.5; b->y = p->y;
                b->vx = 0; b->vy = 0;
                shots_on_target_r++;
                sprintf(commentator_text,"Great save by the goalkeeper!!!!");
                gotoxy(0, 21); 
                printf("COMMENTATOR: %-50s", commentator_text);
                wait_ms(1000);
                init_teams();
                b->vx = 2; b->vy = 0;
                sprintf(commentator_text,"Goalkeeper clears the ball");}
            else {
                b->x = p->x - 0.5; b->y = p->y;
                b->vx = 0; b->vy = 0;
                shots_on_target_b++;
                sprintf(commentator_text,"Great save by the goalkeeper!!!!");
                gotoxy(0, 21); 
                printf("COMMENTATOR: %-50s", commentator_text);
                wait_ms(1000);
                init_teams();
                b->vx = -2; b->vy = 0;
                sprintf(commentator_text,"Goalkeeper clears the ball");}
        }
        else {
        speed = 0.5;
        target_y = b->y;
        if (target_y < 8) target_y = 8;
        if (target_y > 12) target_y = 12;
        if (p->team_id == 0) target_x = 3; else target_x = 76;}
    }
    else {
        if (dist_ball < 6.0) { 
            target_x = b->x; target_y = b->y; speed = 1.3; 
            if (dist_ball < 1.5) {
                if ( dist_goal <= 8.0){
                int direction = (p->team_id == 0) ? 1 : -1;
                double goal_y = 10.0;
                double dy_goal = goal_y - b->y;
                b->vx = (rand() % 3 + 1.2) * direction; 
                b->vy = (dy_goal * 0.08) + ((rand() % 10 - 5) * 0.1); 
                if(p->team_id == 0){
                    if(rand() % 30 == 0) sprintf(commentator_text, "Blue number %d shoots hard!", p->jersey_no);
                    shots_b++;}
                else{
                    if(rand() % 30 == 0) sprintf(commentator_text, "Red number %d shoots hard!", p->jersey_no);
                    shots_r++;}}
            else{
                Player *team;
                double target_goal;
                
                if(p->team_id == 0){
                    target_goal = FIELD_WIDTH - 1;
                    team = Blue_team;}
                else{
                    target_goal = 0;
                    team = Red_team;}
                
                int closest_teammate = -1;
                int best_distance = 10; 
                int i;
                
                for(i = 0; i < PLAYER_COUNT; i++){
                    if(team[i].id == p->id) 
                    continue;
                    
                    double d_mate = calculate_distance(p->x, p->y, team[i].x, team[i].y);
                    double mate_dist_goal = fabs(team[i].x - target_goal);
                    
                    if (mate_dist_goal < dist_goal && d_mate < best_distance){
                        best_distance = d_mate;
                        closest_teammate = i;
                    }
                }
                if(p->team_id == 0){
                if (closest_teammate != -1) {
                    double target_x = team[closest_teammate].x;
                    double target_y = team[closest_teammate].y;
                    
                    double dx = target_x - b->x;
                    double dy = target_y - b->y;
                    double hypotenuse = sqrt(dx*dx + dy*dy);
                    
                    double pass_speed = 2.0; 
                    
                    b->vx = (dx / hypotenuse) * pass_speed;
                    b->vy = (dy / hypotenuse) * pass_speed;
                    
                    sprintf(commentator_text, "Blue number %d passes the ball!", p->jersey_no);
                    passes_b++;
                } 
                else {
                    int direction = (p->team_id == 0) ? 1 : -1;
                    ball.vx = 0.5 * direction; 
                    ball.vy = (rand() % 3 - 1) * 0.1; 
                    
                    ball.x = p->x + (0.8 * direction);
                    ball.y = p->y;
                    sprintf(commentator_text, "Blue number %d dribbles the ball", p->jersey_no);
                }
                }
                else{
                    if (closest_teammate != -1) {
                    double target_x = team[closest_teammate].x;
                    double target_y = team[closest_teammate].y;
                    
                    double dx = target_x - b->x;
                    double dy = target_y - b->y;
                    double hypotenuse = sqrt(dx*dx + dy*dy);
                    
                    double pass_speed = 2.0; 
                    
                    b->vx = (dx / hypotenuse) * pass_speed;
                    b->vy = (dy / hypotenuse) * pass_speed;
                    
                    sprintf(commentator_text, "Red number %d passes the ball!", p->jersey_no);
                    passes_r++;
                } 
                else {
                    int direction = (p->team_id == 0) ? 1 : -1;
                    ball.vx = 0.5 * direction; 
                    ball.vy = (rand() % 3 - 1) * 0.1; 
                    
                    ball.x = p->x + (0.8 * direction);
                    ball.y = p->y;
                    sprintf(commentator_text, "Red number %d is attacking", p->jersey_no);
                }
                }
            }            
            }
        } else {
            if (p->role == defender) {
                if (p->team_id == 0) {
                target_x = b->x * 0.5 + 5;
                if (target_x > 29){
                    target_x = 29.5;}}
                else {
                target_x = 80 - ((80 - b->x) * 0.5 + 5);
                if(target_x < 49){
                    target_x = 49.5;}}
                target_y = (10 * 0.95 + b->y * 0.05); 
            }
            else if (p->role == midfielder) {
                if(p->team_id == 1){
                    target_x = b->x; 
                    if (p->id % 2 == 0) target_y = b->y - 3.5; else target_y = b->y + 3.5;
                    if(target_x > 55){
                    target_x = 55.5;}}
                else{
                    target_x = b->x; 
                    if (p->id % 2 == 0) target_y = b->y - 3.5; else target_y = b->y + 3.5;
                    if(target_x < 24){
                        target_x = 24.5;}} 
            }
            else if (p->role == forward) {
                if (p->team_id == 0) {
                    target_x = b->x + 22;
                    if(target_x < 36){
                        target_x = 36.5;}}
                else{
                    target_x = b->x - 22;
                    if(target_x > 42){
                        target_x = 42.5;}}
                target_y = b->y;
            }
            if (target_x < 2) target_x = 2; if (target_x > 78) target_x = 78;
            if (target_y < 1) target_y = 1; if (target_y > 19) target_y = 19;
        }
    }
    move_towards(p, target_x, target_y, speed);
}

void play_game() {
    int i;
    for (i = 0; i < PLAYER_COUNT; i++) {
        ai_logic(&Blue_team[i], &ball);
        ai_logic(&Red_team[i], &ball);
    }
    ball.x += ball.vx; ball.y += ball.vy;
    ball.vx *= 0.97; ball.vy *= 0.97;
    
    if (ball.y <= 1 || ball.y >= FIELD_HEIGHT - 2) ball.vy *= -1;
    if ((ball.x <= 1 || ball.x >= FIELD_WIDTH - 2) && (ball.y < 8 || ball.y > 12)) ball.vx *= -1;
    
    if (ball.x <= 1 && ball.y >= 8 && ball.y <= 12) {
        score_r++;
        gotoxy(15, 11);
        printf("GOAAL! Red scores! The goalkeeper couldn't do anything!!!");
        gotoxy(34, 12);
        printf("[ENTER]");
        init_teams(); init_ball(); getchar();
        system("cls");
        shots_on_target_r++;
    }
    if (ball.x >= FIELD_WIDTH - 2 && ball.y >= 8 && ball.y <= 12) {
        score_b++;
        gotoxy(15, 11);
        printf("GOAAL! Blue scores! What a fantastic strike!!!!");
        gotoxy(34, 12);
        printf("[ENTER]");
        init_teams(); init_ball(); getchar();
        system("cls");
        shots_on_target_b++;
    }
    
    if (ball.x < 0) ball.x = 1; if (ball.x > FIELD_WIDTH) ball.x = FIELD_WIDTH - 1;
    if (ball.y < 0) ball.y = 1; if (ball.y > FIELD_HEIGHT) ball.y = FIELD_HEIGHT - 1;
}

void draw_pitch(int match_minute) {
    char matrix[FIELD_HEIGHT][FIELD_WIDTH + 1];
    int x, y, i;
    int p = 0; // Buffer position pointer

   
    for (y = 0; y < FIELD_HEIGHT; y++) {
        for (x = 0; x < FIELD_WIDTH; x++) {
            if (y == 0 && x == FIELD_WIDTH / 2) matrix[y][x] = (unsigned char)194; 
            else if (y == FIELD_HEIGHT - 1 && x == FIELD_WIDTH / 2) matrix[y][x] = (unsigned char)193; 
            else if (x == 0 || x == FIELD_WIDTH - 1) matrix[y][x] = (unsigned char)179;
            else if (y == 0 || y == FIELD_HEIGHT - 1) matrix[y][x] = (unsigned char)196;
            else if (x == FIELD_WIDTH / 2) matrix[y][x] = (unsigned char)179;
            else matrix[y][x] = ' ';
        }
        matrix[y][FIELD_WIDTH] = '\0';
    }
    
    for (y = 8; y <= 12; y++) { 
        if (y == 8) { matrix[y][0] = (unsigned char)218; matrix[y][FIELD_WIDTH - 1] = (unsigned char)191; }
        else if (y == 12) { matrix[y][0] = (unsigned char)192; matrix[y][FIELD_WIDTH - 1] = (unsigned char)217; }
        else { matrix[y][0] = (unsigned char)179; matrix[y][FIELD_WIDTH - 1] = (unsigned char)179; }
    }
    
    matrix[0][0] = (char)218;
    matrix[0][79] = (char)191;
    matrix[19][0] = (char)192;
    matrix[19][79] = (char)217;
    
    for (y = 5; y <= 14; y++) {
        matrix[y][12] = (unsigned char)179; 
        if (y == 5) { 
            matrix[y][12] = (unsigned char)191; 
            matrix[y][0]  = (unsigned char)195; 
        }
        if (y == 14) {
            matrix[y][12] = (unsigned char)217; 
            matrix[y][0]  = (unsigned char)195; 
        }
    }
    for (x = 1; x < 12; x++) {
        matrix[5][x] = (unsigned char)196; 
        matrix[14][x] = (unsigned char)196; 
    }
    
    for (y = 5; y <= 14; y++) {
        matrix[y][67] = (unsigned char)179; 
        if (y == 5) {
            matrix[y][67] = (unsigned char)218; 
            matrix[y][FIELD_WIDTH - 1] = (unsigned char)180; 
        }
        if (y == 14) {
            matrix[y][67] = (unsigned char)192; 
            matrix[y][FIELD_WIDTH - 1] = (unsigned char)180; 
        }
    }
    for (x = 68; x < FIELD_WIDTH - 1; x++) {
        matrix[5][x] = (unsigned char)196; 
        matrix[14][x] = (unsigned char)196; 
    }
    
    for (x = 35; x <= 45; x++) {
        if (x == 35) { 
             matrix[7][x] = (unsigned char)218; 
             matrix[13][x] = (unsigned char)192; 
             for(y = 8; y < 13; y++) matrix[y][x] = (unsigned char)179;
        }
        else if (x == 45) { 
             matrix[7][x] = (unsigned char)191; 
             matrix[13][x] = (unsigned char)217; 
             for(y = 8; y < 13; y++) matrix[y][x] = (unsigned char)179;
        }
        else { 
            if (x == FIELD_WIDTH / 2) { 
                matrix[7][x] = (unsigned char)197;  
                matrix[13][x] = (unsigned char)197; 
            } else {
                matrix[7][x] = (unsigned char)196;
                matrix[13][x] = (unsigned char)196;
            }
        }
    }
    
    for (i = 0; i < PLAYER_COUNT; i++) {
        int sx = (int)(Blue_team[i].x + 0.5); 
        int sy = (int)(Blue_team[i].y + 0.5);
        if (sx > 0 && sx < FIELD_WIDTH - 1 && sy > 0 && sy < FIELD_HEIGHT - 1) matrix[sy][sx] = (Blue_team[i].role == goalkeeper) ? 'C' : 'M';

        int kx = (int)(Red_team[i].x + 0.5); 
        int ky = (int)(Red_team[i].y + 0.5);
        if (kx > 0 && kx < FIELD_WIDTH - 1 && ky > 0 && ky < FIELD_HEIGHT - 1) matrix[ky][kx] = (Red_team[i].role == goalkeeper) ? 'C' : 'K';
    }
    
    int tx = (int)(ball.x + 0.5); int ty = (int)(ball.y + 0.5);
    if (tx > 0 && tx < FIELD_WIDTH - 1 && ty > 0 && ty < FIELD_HEIGHT - 1) matrix[ty][tx] = 'o';

    // Buffer Setup
    
    // Reset Cursor (ANSI code)
    p += sprintf(&screen_buffer[p], "\033[H");
    
    // Header
    p += sprintf(&screen_buffer[p], "\033[42;97m        SCORE: BLUE %d - %d RED | MINUTE: %d   Press [ESC] to Pause\033[K\n", score_b, score_r, match_minute);

    for (y = 0; y < FIELD_HEIGHT; y++) {
        // Line start: Green background
        p += sprintf(&screen_buffer[p], "\033[42m");
        
        for (x = 0; x < FIELD_WIDTH; x++) {
            char c = matrix[y][x];
            
            if (c == 'M') { // Blue Player
                p += sprintf(&screen_buffer[p], "\033[44;37m%c\033[42m", c);
            }
            else if (c == 'K') { // Red Player
                p += sprintf(&screen_buffer[p], "\033[41;37m%c\033[42m", c);
            }
            else if (c == 'C') { // Goalkeeper
                p += sprintf(&screen_buffer[p], "\033[43;30m%c\033[42m", c);
            }
            else if (c == 'o') { // Ball
                p += sprintf(&screen_buffer[p], "\033[97m%c\033[42m", c);
            }
            else if (c == (char)179 || c == (char)196 || c == (char)218 || c == (char)217 || c == (char)191 || c == (char)192 || c == (char)193 || c == (char)194 
            || c == (char)195 || c == (char)180 || c== (char)197 ) { // Lines
                p += sprintf(&screen_buffer[p], "\033[97m%c\033[42m", c);
            }
            else { // Empty Space
                p += sprintf(&screen_buffer[p], " "); 
            }
        }
        // Line end: Reset colors
        p += sprintf(&screen_buffer[p], "\033[0m\n");
    }

    // Footer and Commentator
    p += sprintf(&screen_buffer[p], "\033[42;97mCOMMENTATOR: %-50s\033[K\n", commentator_text);

    // Print to screen
    printf("%s", screen_buffer);
}




int main(void) {
    SetConsoleOutputCP(437);
    system("color 2F");
    int match_minute = 0;
    int frame = 0;
    int frames_per_minute = (int)(REAL_SEC_PER_MINUTE * FPS);
    
    
    srand(time(NULL));
    hide_cursor();
    
    system("cls");
    
    gotoxy(18, 9);
    printf("Match is starting !!!!!!!!\n");
    gotoxy(15, 10);
    printf("Press [Enter] to start.");
    init_teams();
    init_ball();
    getchar();
    system("cls");
    
    while (match_minute <= 90) {
        if (_kbhit()) {
            int key = getch();
            if (key == 27) {
                int k;
                gotoxy(15, 7);
                printf("%c", 218); 
                for (k = 0; k < 47; k++) printf("%c", 196); 
                printf("%c", 191);
                gotoxy(15, 8);
                printf("%c                GAME PAUSED                    %c", 179, 179);
                gotoxy(15, 9);
                printf("%c    EXIT [ESC]         RESUME [ENTER]          %c", 179, 179);
                gotoxy(15, 10);
                printf("%c                                               %c", 179, 179);
                gotoxy(15, 11);
                printf("%c", 192); 
                for (k = 0; k < 47; k++) printf("%c", 196); 
                printf("%c", 217);
                key = getch();
                if (key == 27 ) {
                system("cls");
                gotoxy(15, 10);
                printf("%c", 218); 
                for (k = 0; k < 47; k++) printf("%c", 196); 
                printf("%c", 191);
                gotoxy(15, 11);
                printf("%c       Game cancelled, see you again           %c", 179, 179);
                break;}
                else if (key == 13) {
                    continue;
                }
            }
        }
        play_game();
        draw_pitch(match_minute);
        
        frame++;
        if (frame % frames_per_minute == 0) {
            match_minute++;
            if (match_minute == 45) {
                gotoxy(18, 9);
                printf("Half Time!! Press [Enter] to start ");
                getchar();
                init_teams();
                init_ball();
                system("cls");
            }
        }
        wait_ms(1000 / FPS);
    }
    if (match_minute >= 90 ) {
        int k;
        gotoxy(15, 6);
        printf("%c", 218); 
        for (k = 0; k < 47; k++) printf("%c", 196); 
        printf("%c", 191); 
        gotoxy(15, 7);
        printf("%c    MATCH FINISHED     Blue : %d - %d : Red      %c", (char)179, score_b, score_r, (char)179);
        gotoxy(15, 8);
        printf("%c                                               %c", 179, 179);
        gotoxy(15, 9);
        printf("%c  Passes Made:           (%4d) -- (%4d)        %c", 179, passes_b, passes_r, 179);
        gotoxy(15, 10);
        printf("%c                                               %c", 179, 179);
        gotoxy(15, 11);
        printf("%c Shots(On Target)       %3d(%d) -- %3d(%d)      %c", (char)179, shots_b, shots_on_target_b, shots_r, shots_on_target_r, (char)179);
        gotoxy(15, 12);
        printf("%c                                               %c", 179, 179);
        
    }
    gotoxy(15, 13);
    printf("%c       Press Any Key to Exit...                %c", (char)179, (char)179);
    gotoxy(15, 14);
    printf("%c", 192); 
    int k;
    for (k = 0; k < 47; k++) printf("%c", 196); 
    printf("%c", 217); 
    _getch();
    return 0;
}

