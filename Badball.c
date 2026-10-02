#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <conio.h> // Klavye kontrolü için (Windows/Dev-C++ standart)
#include <windows.h>

// Ýþletim sistemi uyumluluðu (Windows için özel imleç ayarý)
#ifdef _WIN32
#include <windows.h>
void bekle_ms(int ms) { Sleep(ms); }

void imleci_gizle() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = 0; // Görünürlüðü kapat
    SetConsoleCursorInfo(hOut, &cursorInfo);
}
#else
#include <unistd.h>
void bekle_ms(int ms) { usleep(ms * 1000); }
void imleci_gizle() { printf("\033[?25l"); }
#endif


#define SAHA_EN 80
#define SAHA_BOY 20
#define OYUNCU_SAYISI 8 
#define GERCEK_SN_PER_DAKIKA 1.5
#define FPS 60 // 60 Kare/Saniye 
#define CARPISMA_MESAFESI 2.0
#define EKRAN_BUFFER_BOYUTU 65536

char ekran_buffer[EKRAN_BUFFER_BOYUTU];

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
} Top;

typedef enum{kaleci,defans,orta_saha,forvet} Rol;

typedef struct {
	int takim;
	int no;
	int forma_no;
	double x,y;
	Rol rol;
} oyuncu;

oyuncu Mavi_takim[OYUNCU_SAYISI];
oyuncu Kirmizi_takim[OYUNCU_SAYISI];
Top top;
int skor_m = 0;
int skor_k = 0;
int pas_m = 0;
int pas_k = 0;
int sut_m=0,sut_k=0,is_sut_k=0,is_sut_m=0;
char spiker_metni[100] = "Mac basliyor! Cikmak icin [ESC] basin.";

double mesafe(double x1, double y1,double x2,double y2){
	return sqrt(pow(x2-x1,2)+pow(y2-y1,2));
}

void takimlar(){
	int i;
    Mavi_takim[0].x = 3;  Mavi_takim[0].y = 10; Mavi_takim[0].rol = kaleci;
    Mavi_takim[1].x = 15; Mavi_takim[1].y = 5;  Mavi_takim[1].rol = defans;
    Mavi_takim[2].x = 15; Mavi_takim[2].y = 10; Mavi_takim[2].rol = defans;
    Mavi_takim[3].x = 15; Mavi_takim[3].y = 15; Mavi_takim[3].rol = defans;
    Mavi_takim[4].x = 35; Mavi_takim[4].y = 8;  Mavi_takim[4].rol = orta_saha;
    Mavi_takim[5].x = 35; Mavi_takim[5].y = 12; Mavi_takim[5].rol = orta_saha;
    Mavi_takim[6].x = 48; Mavi_takim[6].y = 9;  Mavi_takim[6].rol = forvet;
    Mavi_takim[7].x = 48; Mavi_takim[7].y = 11; Mavi_takim[7].rol = forvet;

    Kirmizi_takim[0].x = 76; Kirmizi_takim[0].y = 10; Kirmizi_takim[0].rol = kaleci;
    Kirmizi_takim[1].x = 64; Kirmizi_takim[1].y = 5;  Kirmizi_takim[1].rol = defans;
    Kirmizi_takim[2].x = 64; Kirmizi_takim[2].y = 10; Kirmizi_takim[2].rol = defans;
    Kirmizi_takim[3].x = 64; Kirmizi_takim[3].y = 15; Kirmizi_takim[3].rol = defans;
    Kirmizi_takim[4].x = 44; Kirmizi_takim[4].y = 8;  Kirmizi_takim[4].rol = orta_saha;
    Kirmizi_takim[5].x = 44; Kirmizi_takim[5].y = 12; Kirmizi_takim[5].rol = orta_saha;
    Kirmizi_takim[6].x = 31; Kirmizi_takim[6].y = 9;  Kirmizi_takim[6].rol = forvet;
    Kirmizi_takim[7].x = 31; Kirmizi_takim[7].y = 11; Kirmizi_takim[7].rol = forvet;
    
    for(i=0;i<OYUNCU_SAYISI;i++){
    	Mavi_takim[i].no=i;Mavi_takim[i].takim=0;Mavi_takim[i].forma_no=i+1;
    	Kirmizi_takim[i].no=i;Kirmizi_takim[i].takim=1;Kirmizi_takim[i].forma_no=i+1;
	}
}
void top_basla(){
	top.x = SAHA_EN / 2.0; top.y = SAHA_BOY / 2.0; top.vx = 0; top.vy = 0;
}

int yer_musait_mi(double x, double y, int gozardi_id, int takim_id) {
    int i;
    double d;
    for(i=0; i<OYUNCU_SAYISI; i++) {
        if(takim_id == 0 && Mavi_takim[i].no == gozardi_id) continue;
        d = mesafe(x, y, Mavi_takim[i].x, Mavi_takim[i].y);
        if(d < CARPISMA_MESAFESI) return 0; 
    }
    for(i=0; i<OYUNCU_SAYISI; i++) {
        if(takim_id == 1 && Kirmizi_takim[i].no == gozardi_id) continue;
        d = mesafe(x, y, Kirmizi_takim[i].x, Kirmizi_takim[i].y);
        if(d < CARPISMA_MESAFESI) return 0; 
    }
    return 1;
}


void  git(oyuncu *o,double hdfx,double hdfy,double hiz){
	double dx=(hdfx - o->x);
	double dy=(hdfy - o->y);
	double d=sqrt((dx*dx)+(dy*dy));
	double frame=hiz*0.4;
	
	
	if (d > 0.5) {
        double vx = (dx / d) * frame;
        double vy = (dy / d) * frame;
        
        double yeni_x = o->x + vx;
        double yeni_y = o->y + vy;

        if (yer_musait_mi(yeni_x, yeni_y, o->no, o->takim)) {
            o->x = yeni_x;
            o->y = yeni_y;
        } else {
           int r = rand() % 4; // 0:Yukarý, 1:Aþaðý, 2:Sol, 3:Sað
            int hareket_edildi = 0;

            
            if (r == 0 && yer_musait_mi(o->x, o->y - frame, o->no, o->takim)) { o->y -= frame; hareket_edildi=1; }
            else if (r == 1 && yer_musait_mi(o->x, o->y + frame, o->no, o->takim)) { o->y += frame; hareket_edildi=1; }
            else if (r == 2 && yer_musait_mi(o->x - frame, o->y, o->no, o->takim)) { o->x -= frame; hareket_edildi=1; }
            else if (r == 3 && yer_musait_mi(o->x + frame, o->y, o->no, o->takim)) { o->x += frame; hareket_edildi=1; }
            
            
            if (!hareket_edildi) {
                if (yer_musait_mi(o->x, o->y - frame, o->no, o->takim)) o->y -= frame;      // Yukarý
                else if (yer_musait_mi(o->x + frame, o->y, o->no, o->takim)) o->x += frame; // Saða
                else if (yer_musait_mi(o->x - frame, o->y, o->no, o->takim)) o->x -= frame; // Sola
                else if (yer_musait_mi(o->x, o->y + frame, o->no, o->takim)) o->y += frame;} // Aþaðý (En son tercih)}
        }
    }
    if (o->x < 1.0) o->x = 1.0;
    if (o->x > SAHA_EN - 2.0) o->x = SAHA_EN - 2.0;
    if (o->y < 1.0) o->y = 1.0;
    if (o->y > SAHA_BOY - 2.0) o->y = SAHA_BOY - 2.0;
}

void yapay_zeka(oyuncu *o, Top *t) {
    double dist_top = mesafe(o->x, o->y, t->x, t->y);
    double hedef_x = o->x, hedef_y = o->y;
    double hiz = 1.0; 
    double dist_kale;
    if (o->takim==0){
    	dist_kale = fabs(o->x - (SAHA_EN-1));}
	else{
		dist_kale = fabs(o->x - 0);}
	
    if (o->rol == kaleci) {
    	if(dist_top<1.1){
    		if (o->takim == 0){
    			t->x = o->y+0.5; t->y = o->y;
    			t->vx =0; t->vy= 0;
    			is_sut_k++;
    			sprintf(spiker_metni,"Son anda kaleciiii !!!!");
    			gotoxy(0, 21); 
    			printf("SPIKER: %-50s", spiker_metni);
    			bekle_ms(1000);
    			takimlar();
    			t->vx = 2; t->vy=0;
				sprintf(spiker_metni,"Kaleci degaj yapti");}
			else{
				t->x = o->x-0.5; t->y = o->y;
				t->vx =0; t->vy= 0;
				is_sut_m++;
				sprintf(spiker_metni,"Son anda kaleciiii !!!!");
				gotoxy(0, 21); 
    			printf("SPIKER: %-50s", spiker_metni);
    			bekle_ms(1000);
    			takimlar();
    			t->vx = -2; t->vy=0;
				sprintf(spiker_metni,"Kaleci degaj yapti");}
		}
		else {
        hiz = 0.5;
        hedef_y = t->y;
        if (hedef_y < 8) hedef_y = 8;
        if (hedef_y > 12) hedef_y = 12;
        if (o->takim == 0) hedef_x = 3; else hedef_x = 76;}
    }
    else {
        if (dist_top < 6.0) { 
            hedef_x = t->x; hedef_y = t->y; hiz = 1.3; 
            if (dist_top < 1.5) {
            	if ( dist_kale <= 8.0){
                int yon = (o->takim == 0) ? 1 : -1;
                double kale_y = 10.0;
                double dy_kale = kale_y - t->y;
                t->vx = (rand()%3 + 1.2) * yon; 
                t->vy = (dy_kale * 0.08) + ((rand()%10 - 5) * 0.1); 
                if(o->takim == 0){
                	if(rand()%30==0) sprintf(spiker_metni, "Mavi %d numara sert vurdu!", o->forma_no);
					sut_m++;}
                else{
                	if(rand()%30==0) sprintf(spiker_metni, "Kirmizi %d numara sert vurdu!", o->forma_no);
					sut_k++;}}
            else{
            	oyuncu *takim;
            	double hedef_kale;
            	
            	if(o->takim == 0){
            		hedef_kale = SAHA_EN-1;
            		takim = Mavi_takim;}
            	else{
            		hedef_kale = 0;
            		takim = Kirmizi_takim;}
            	
            	int yakin_ark=-1;
            	int iyi_mesafe = 10; 
            	int i;
            	
            	for(i=0;i<OYUNCU_SAYISI;i++){
            		if(takim[i].no == o->no) 
					continue;
            		
            		double d_ark= mesafe(o->x,o->y,takim[i].x,takim[i].y);
            		double ark_dist_kale = fabs(takim[i].x - hedef_kale);
            		
            		if (ark_dist_kale < dist_kale && d_ark < iyi_mesafe){
            			iyi_mesafe = d_ark;
            			yakin_ark = i;
					}
				}
				if(o->takim == 0){
				if (yakin_ark != -1) {
                    double hedef_x = takim[yakin_ark].x;
                    double hedef_y = takim[yakin_ark].y;
                    
                    double dx = hedef_x - t->x;
                    double dy = hedef_y - t->y;
                    double hipotenus = sqrt(dx*dx + dy*dy);
                    
                    double pas_hizi = 2.0; 
                    
                    t->vx = (dx / hipotenus) * pas_hizi;
                    t->vy = (dy / hipotenus) * pas_hizi;
                    
                    sprintf(spiker_metni, "Mavi %d numara pasini aktardi!", o->forma_no);
                    pas_m++;
                } 
                else {
                    int yon = (o->takim == 0) ? 1 : -1;
                    top.vx = 0.5 * yon; 
    				top.vy = (rand() % 3 - 1) * 0.1; 
                    
                    top.x = o->x + (0.8 * yon);
    				top.y = o->y;
                    sprintf(spiker_metni, "Mavi %d numara topu surdu", o->forma_no);
                }
				}
                else{
                	if (yakin_ark != -1) {
                    double hedef_x = takim[yakin_ark].x;
                    double hedef_y = takim[yakin_ark].y;
                    
                    double dx = hedef_x - t->x;
                    double dy = hedef_y - t->y;
                    double hipotenus = sqrt(dx*dx + dy*dy);
                    
                    double pas_hizi = 2.0; 
                    
                    t->vx = (dx / hipotenus) * pas_hizi;
                    t->vy = (dy / hipotenus) * pas_hizi;
                    
                    sprintf(spiker_metni, "Kirmizi %d numara pasini aktardi!", o->forma_no);
                    pas_k++;
                } 
                else {
                    int yon = (o->takim == 0) ? 1 : -1;
                    top.vx = 0.5 * yon; 
    				top.vy = (rand() % 3 - 1) * 0.1; 
                    
                    top.x = o->x + (0.8 * yon);
    				top.y = o->y;
                    sprintf(spiker_metni, "Kirmizi %d numara Ataga kalkti", o->forma_no);
                }
				}
			}            
            }
        } else {
            if (o->rol == defans) {
                if (o->takim == 0) {
				hedef_x = t->x * 0.5 + 5;
				if (hedef_x >29){
                	hedef_x = 29.5;}}
                else {
				hedef_x = 80 - ((80 - t->x) * 0.5 + 5);
				if(hedef_x<49){
					hedef_x=49.5;}}
                hedef_y = (10 * 0.95 + t->y * 0.05); 
            }
            else if (o->rol == orta_saha) {
            	if(o->takim==1){
                	hedef_x = t->x; 
                	if (o->no % 2 == 0) hedef_y = t->y - 3.5; else hedef_y = t->y + 3.5;
                	if(hedef_x > 55){
                	hedef_x=55.5;}}
                else{
                	hedef_x = t->x; 
                	if (o->no % 2 == 0) hedef_y = t->y - 3.5; else hedef_y = t->y + 3.5;
                	if(hedef_x < 24){
                		hedef_x=24.5;}}	
            }
            else if (o->rol == forvet) {
                if (o->takim == 0) {
					hedef_x = t->x + 22;
					if(hedef_x < 36){
						hedef_x=36.5;}}
                else{
					hedef_x = t->x - 22;
					if(hedef_x > 42){
						hedef_x=42.5;}}
                hedef_y = t->y;
            }
            if (hedef_x < 2) hedef_x = 2; if (hedef_x > 78) hedef_x = 78;
            if (hedef_y < 1) hedef_y = 1; if (hedef_y > 19) hedef_y = 19;
        }
    }
    git(o, hedef_x, hedef_y, hiz);
}

void oyun(){
	int i;
	for(i=0;i<OYUNCU_SAYISI;i++){
		yapay_zeka(&Mavi_takim[i],&top);
		yapay_zeka(&Kirmizi_takim[i],&top);
	}
	top.x += top.vx; top.y += top.vy;
    top.vx *= 0.97; top.vy *= 0.97;
    
    if(top.y <= 1 || top.y >= SAHA_BOY-2) top.vy *= -1;
    if((top.x <= 1 || top.x >= SAHA_EN-2) && (top.y < 8 || top.y > 12)) top.vx *= -1;
    
    if(top.x <= 1 && top.y >= 8 && top.y <= 12) {
        skor_k++;
        gotoxy(15,11);
        printf("GOOOL! Kirmizi atti! Kaleci hicbirsey yapamadii!!!");
		gotoxy(34,12);
        printf("[ENTER]");
        takimlar(); top_basla(); getchar();
        system("cls");
        is_sut_k++;
    }
    if(top.x >= SAHA_EN-2 && top.y >= 8 && top.y <= 12) {
        skor_m++;
        gotoxy(15,11);
        printf("GOOOL! Mavi atti! Orumcek aglarini temizlediii !!!!");
        gotoxy(34,12);
        printf("[ENTER]");
        takimlar(); top_basla(); getchar();
        system("cls");
        is_sut_m++;
    }
    
    if(top.x < 0) top.x = 1; if(top.x > SAHA_EN) top.x = SAHA_EN-1;
    if(top.y < 0) top.y = 1; if(top.y > SAHA_BOY) top.y = SAHA_BOY-1;
}

void ciz(int dakika){
    char matris[SAHA_BOY][SAHA_EN+1];
    int x, y, i;
    int p = 0; // Buffer pozisyon iþaretçisi

   
    for(y=0;y<SAHA_BOY;y++){
        for(x=0;x<SAHA_EN;x++){
        	if(y==0 && x==SAHA_EN/2) matris[y][x] = (unsigned char)194; 
        	else if(y==SAHA_BOY-1 && x==SAHA_EN/2) matris[y][x] = (unsigned char)193; 
            else if(x==0 || x==SAHA_EN-1) matris[y][x] = (unsigned char)179;
            else if(y==0 || y==SAHA_BOY-1) matris[y][x] = (unsigned char)196;
            else if(x==SAHA_EN/2) matris[y][x] = (unsigned char)179;
            else matris[y][x] = ' ';
        }
        matris[y][SAHA_EN]='\0';
    }
    
    for(y=8; y<=12; y++) { 
        if (y == 8){ matris[y][0] = (unsigned char)218; matris[y][SAHA_EN-1] = (unsigned char)191; }
        else if(y == 12){ matris[y][0] = (unsigned char)192; matris[y][SAHA_EN-1] = (unsigned char)217; }
        else { matris[y][0] = (unsigned char)179; matris[y][SAHA_EN-1] = (unsigned char)179;}
    }
    
    matris[0][0] = (char)218;
    matris[0][79] = (char)191;
    matris[19][0] = (char)192;
    matris[19][79] = (char)217;
    
    for(y=5; y<=14; y++) {
        matris[y][12] = (unsigned char)179; 
        if(y==5) { 
            matris[y][12] = (unsigned char)191; 
            matris[y][0]  = (unsigned char)195; 
        }
        if(y==14) {
            matris[y][12] = (unsigned char)217; 
            matris[y][0]  = (unsigned char)195; 
	        }
    }
    for(x=1; x<12; x++) {
        matris[5][x] = (unsigned char)196; 
        matris[14][x] = (unsigned char)196; 
    }
    
    for(y=5; y<=14; y++) {
        matris[y][67] = (unsigned char)179; 
        if(y==5) {
            matris[y][67] = (unsigned char)218; 
            matris[y][SAHA_EN-1] = (unsigned char)180; 
        }
        if(y==14) {
            matris[y][67] = (unsigned char)192; 
            matris[y][SAHA_EN-1] = (unsigned char)180; 
        }
    }
    for(x=68; x<SAHA_EN-1; x++) {
        matris[5][x] = (unsigned char)196; 
        matris[14][x] = (unsigned char)196; 
    }
    
    for(x=35; x<=45; x++) {
        if(x==35) { 
             matris[7][x] = (unsigned char)218; 
             matris[13][x] = (unsigned char)192; 
             for(y=8; y<13; y++) matris[y][x] = (unsigned char)179;
        }
        else if(x==45) { 
             matris[7][x] = (unsigned char)191; 
             matris[13][x] = (unsigned char)217; 
             for(y=8; y<13; y++) matris[y][x] = (unsigned char)179;
        }
        else { 
            if(x == SAHA_EN/2) { 
                matris[7][x] = (unsigned char)197;  
                matris[13][x] = (unsigned char)197; 
            } else {
                matris[7][x] = (unsigned char)196;
                matris[13][x] = (unsigned char)196;
            }
        }
    }
    
    for(i=0; i<OYUNCU_SAYISI; i++) {
        int sx = (int)(Mavi_takim[i].x + 0.5); 
        int sy = (int)(Mavi_takim[i].y + 0.5);
        if(sx>0 && sx<SAHA_EN-1 && sy>0 && sy<SAHA_BOY-1) matris[sy][sx] = (Mavi_takim[i].rol==kaleci)?'C':'M';

        int kx = (int)(Kirmizi_takim[i].x + 0.5); 
        int ky = (int)(Kirmizi_takim[i].y + 0.5);
        if(kx>0 && kx<SAHA_EN-1 && ky>0 && ky<SAHA_BOY-1) matris[ky][kx] = (Kirmizi_takim[i].rol==kaleci)?'C':'K';
    }
    
    int tx = (int)(top.x + 0.5); int ty = (int)(top.y + 0.5);
    if(tx>0 && tx<SAHA_EN-1 && ty>0 && ty<SAHA_BOY-1) matris[ty][tx] = 'o';

    // Buffer'ý Doldur (Tek printf için hazýrlýk)
    
    // Ýmleci baþa al (ANSI kodu)
    p += sprintf(&ekran_buffer[p], "\033[H");
    
    // Baþlýk (Yeþil Zemin, Beyaz Yazý)
    p += sprintf(&ekran_buffer[p], "\033[42;97m        SKOR: MAVI %d - %d KIRMIZI | DAKIKA: %d   Duraklatmak Icin  [ESC]\033[K\n", skor_m, skor_k, dakika);

    for(y=0; y<SAHA_BOY; y++){
        // Satýr baþý: Yeþil zemin
        p += sprintf(&ekran_buffer[p], "\033[42m");
        
        for(x=0; x<SAHA_EN; x++){
            char c = matris[y][x];
            
            if (c == 'M') { // Mavi Oyuncu (Mavi Zemin, Beyaz Yazý)
                p += sprintf(&ekran_buffer[p], "\033[44;37m%c\033[42m", c);
            }
            else if (c == 'K') { // Kýrmýzý Oyuncu (Kýrmýzý Zemin, Beyaz Yazý)
                p += sprintf(&ekran_buffer[p], "\033[41;37m%c\033[42m", c);
            }
            else if (c == 'C') { // Kaleci (Sarý Zemin, Siyah Yazý)
                p += sprintf(&ekran_buffer[p], "\033[43;30m%c\033[42m", c);
            }
            else if (c == 'o') { // Top (Yeþil Zemin, Parlak Beyaz Yazý - Ýsteðiniz bu)
                p += sprintf(&ekran_buffer[p], "\033[97m%c\033[42m", c);
            }
            else if (c == (char)179 || c == (char)196 || c == (char)218 || c == (char)217 || c == (char)191 || c == (char)192 || c == (char)193 || c == (char)194 
			|| c == (char)195 || c == (char)180 || c== (char)197 ) { // Çizgiler (Yeþil Zemin, Beyaz Yazý)
                p += sprintf(&ekran_buffer[p], "\033[97m%c\033[42m", c);
            }
            else { // Boþ Saha (Yeþil Zemin)
                p += sprintf(&ekran_buffer[p], " "); 
            }
        }
        // Satýr sonu: Renkleri sýfýrla, yeni satýr
        p += sprintf(&ekran_buffer[p], "\033[0m\n");
    }

    // Alt bilgi ve Spiker
    p += sprintf(&ekran_buffer[p], "\033[42;97mSPIKER: %-50s\033[K\n", spiker_metni);
    //p += sprintf(&ekran_buffer[p], "[ESC] Cikis\033[0m\033[K\n");

    // 3. Tek Seferde Ekrana Bas! (Burasý kasmayý önleyen sihirli kýsým)
    printf("%s", ekran_buffer);
}




int main(void){
	SetConsoleOutputCP(437);
	system("color 2F");
	int dakika = 0;
    int kare = 0;
    int kare_per_dakika = (int)(GERCEK_SN_PER_DAKIKA * FPS);
	
	
	srand(time(NULL));
	imleci_gizle();
	
	system("cls");
	
	gotoxy(18,9);
	printf("Mac basliyooor !!!!!!!!\n");
	gotoxy(15,10);
	printf("Baslamak icin [Enter] tusuna basin.");
	takimlar();
	top_basla();
	getchar();
	system("cls");
	
	while(dakika<=90){
		if(_kbhit()){
			int tus = getch();
			if (tus == 27){
				int k;
				gotoxy(15,7);
                printf("%c", 218); 
                for(k=0; k<47; k++) printf("%c", 196); 
                printf("%c", 191);
				gotoxy(15,8);
                printf("%c                OYUN DURAKLATILDI              %c", 179, 179);
                gotoxy(15,9);
                printf("%c    CIKIS [ESC]         DEVAM ETTIR [ENTER]    %c", 179, 179);
                gotoxy(15,10);
                printf("%c                                               %c", 179, 179);
				gotoxy(15,11);
                printf("%c", 192); 
                for(k=0; k<47; k++) printf("%c", 196); 
                printf("%c", 217);
				tus = getch();
				if (tus == 27 ){
				system("cls");
				gotoxy(15,10);
                printf("%c", 218); 
                for(k=0; k<47; k++) printf("%c", 196); 
                printf("%c", 191);
				gotoxy(15,11);
                printf("%c       Oyun iptal edildi tekrar bekleriz       %c", 179, 179);
				break;}
				else if(tus == 13){
					continue;
				}
			}
		}
		oyun();
		ciz(dakika);
		
		kare++;
		if(kare % kare_per_dakika == 0){
			dakika++;
			if(dakika == 45){
				gotoxy(18,9);
				printf("Devre Arasi!! Baslamak icin [Enter] basin ");
				getchar();
				takimlar();
				top_basla();
				system("cls");
			}
		}
		bekle_ms(1000/FPS);
	}
	if (dakika >= 90 ){
		int k;
		gotoxy(15,6);
        printf("%c", 218); 
        for(k=0; k<47; k++) printf("%c", 196); 
		printf("%c", 191); 
		gotoxy(15,7);
        printf("%c    MAC Bittii     Mavi : %d - %d : Kirmizi      %c", (char)179, skor_m, skor_k,(char)179);
        gotoxy(15,8);
        printf("%c                                               %c",179,179);
        gotoxy(15,9);
        printf("%c  Atilan Pas:          (%4d) -- (%4d)        %c", 179, pas_m, pas_k, 179);
        gotoxy(15,10);
        printf("%c                                               %c",179,179);
        gotoxy(15,11);
        printf("%c Atilan Sut(isabetli)   %3d(%d) -- %3d(%d)     %c", (char)179,sut_m,is_sut_m,sut_k,is_sut_k,(char)179);
        gotoxy(15,12);
        printf("%c                                               %c",179,179);
        
	}
	gotoxy(15,13);
    printf("%c       Cikmak icin Bir Tusa Basin...           %c", (char)179, (char)179);
	gotoxy(15,14);
    printf("%c", 192); 
    int k;
    for(k=0; k<47; k++) printf("%c", 196); 
    printf("%c", 217); 
	_getch();
	return 0;
}

