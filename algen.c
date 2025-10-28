#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#if defined _WIN32
	#include <Windows.h>
	#define FORMULA (((los/100000)+0.953290) / formula_dz)
	#define ZMIANA kolor = (int*) SetConsoleTextAttribute(kolor, 6);
	#define ORYG kolor = (int*) SetConsoleTextAttribute(kolor, 2);
	
		
#else
	#include <pthread.h>
	void *sortuj( void *ptr );
	void *walidacja( void *ptr );
	
	#define FORMULA (((los/100000)+0.950200) / formula_dz)
	//#define ZMIANA printf("\033[5;7m");
	#define ZMIANA printf("\033[92;5m");
	#define ORYG printf("\033[0;0m");
#endif


#define POP 350
#define POPF POP
#define POPM POP

//#define FORMULA (los/100000)+0.953290 // win
//#define FORMULA (los/100000)+0.950200 // linux

// 2 znaki + '0' byte
char mode[3];

double formula_dz = 1;

// struktura na rekord danych
struct s_rekord {
	double target, a, b;
};

// struktura na sumy
struct s_suma {
	double sumtarget, suma, sumb;
};


// wektor wag
double wagi[3];


struct s_found {
	int id, generacja;
	float WA, WB, intercept, sse, mse;
	char gender;
};

struct s_found fwynik;

struct s_czlek {
	long unsigned id;
	int generacja;
	float sse, mse, attr_index;
	char gender;
	float WA, WB, WC, WD, intercept;              
};

// obecne pokolenie
struct s_czlek populacja_f[POPF];
struct s_czlek populacja_m[POPM];

// kolejne pokolenie
struct s_czlek populacja2_f[POPF];
struct s_czlek populacja2_m[POPM];

// tablice dla posortowanych po attr_index pointerów w obu populacjach, będzie potrzebne do dobierania w pary
struct s_czlek *f[POPF];
struct s_czlek *m[POPM];



// inicjalizacja countera id i pokolenia (generacji)
long idCounter = 0;
long generacjaCounter = 0;



// struktury do przekazania argumentu jako pointer

// do sortowania: struktura też ma 2 pointery: 1: na wektor z daną populacją, 2: na tablicę pointerów do tej populacji
struct args_ct {
	struct s_czlek *a_populacja;
	struct s_czlek **a_p;
	const char *deb;
};

// do walidacji: 	
struct s_args_w {
	struct s_rekord *pRekord;
	struct s_czlek *populacja;
	int lines;
};
		
		


// funkcje


struct s_suma sumuj(struct s_rekord *rec, size_t n) {

	unsigned int i;
	struct s_suma sumy;
	
	
	for (i = 0; i < n; i++) {
		sumy.sumtarget += rec[i].target;
		sumy.suma += rec[i].a;
		sumy.sumb += rec[i].b;
	}
	
	return sumy;
	
}
 

void ustaw_wagi (struct s_rekord *pRekord, size_t lines) {
	
	double suma, sumb, sumt;
	double wagaa, wagab, avga, avgb, avgt, intercept;
	
	struct s_suma sumy = sumuj(pRekord, lines);
	
	sumt = sumy.sumtarget;
	avgt = sumt/lines;
	
	suma = sumy.suma;
	sumb = sumy.sumb;
	
	avga = suma/lines;
	avgb = sumb/lines;
	
	wagaa = (avgt/avga) / 2;
	wagab = (avgt/avgb) / 2;
	
	intercept = avgt - (wagaa + wagab);
	
	wagi[0] = wagaa;
	wagi[1] = wagab;
	wagi[2] = intercept;
	
	//printf("Suma wektora to %d\n",sumuj(ai, ac));
	
}




struct s_czlek * inicjacja(struct s_czlek *pSource, char GR) {
	
	idCounter++;
	pSource -> id = idCounter;

	pSource -> generacja = generacjaCounter;
	pSource -> WA = wagi[0];
	pSource -> WB = wagi[1];
	pSource -> intercept = wagi[2];


	pSource -> gender = GR;


	return pSource;
}

double mutacja (double param) {
	
	double los, modifier;


	// zwraca liczbę losową 0 - 10000
	los = (double) (rand()%10000+1);
	modifier = FORMULA;
	
	return param * modifier;
	
}
	
 

void potomstwo10 (struct s_czlek **fem, struct s_czlek **mas) {

	int i;
	
	
	for (i = 0; i < POP/10; i++) {
		
		// nowe atrybuty dla pokolenia - tu mutacja, 2 razy losowanie żeby potomkowie w obu populacjach inaczej się mutowali
		// docelowo w pętli
		
		populacja2_f[i].WA = mutacja(fem[i] -> WA);
		populacja2_f[i].WB = mutacja(fem[i] -> WB);
		populacja2_f[i].intercept = mutacja(fem[i] -> intercept);
		
		populacja2_m[i].WA = mutacja(mas[i] -> WA);
		populacja2_m[i].WB = mutacja(mas[i] -> WB);
		populacja2_m[i].intercept = mutacja(mas[i] -> intercept);
		
	}
	
	
	// teraz przeniesienie rotacja pokoleń, danych z wektora potomków do wektora rodziców oryginalnych
	// przenosimy tylko potrzebne cechy, reszta i tak jest skopiowana w docelowej populacji
	
	//rozmnaża się tylko 10% najlepszych ale x 10 czyli po 20 osobników potomstwa (10F i 10M)
	int k;
	k = 0;
	
	for (k = 0; k  < 10; k++) {
		for (i = 0; i < POP/10; i++) {
			
			//WA dziedziczony po F
			//WB dziedziczony po M
			
			// populacja F
			idCounter++;
			populacja_f[k*10 + i].id = idCounter;
			populacja_f[k*10 + i].generacja = generacjaCounter;

			populacja_f[k*10 + i].WA = populacja2_f[i].WA;
			populacja_f[k*10 + i].WB = populacja2_f[i].WB;
			populacja_f[k*10 + i].intercept = populacja2_f[i].intercept;

			
			populacja_f[k*10 + i].sse = populacja2_f[i].sse;
			populacja_f[k*10 + i].mse = populacja2_f[i].mse;
			
			populacja_f[k*10 + i].attr_index = populacja2_f[i].attr_index;

			
			// populacja M
			idCounter++;
			populacja_m[k*10 + i].id = idCounter;
			populacja_m[k*10 + i].generacja = generacjaCounter;

			populacja_m[k*10 + i].WA = populacja2_m[i].WA;
			populacja_m[k*10 + i].WB = populacja2_m[i].WB;
			populacja_m[k*10 + i].intercept = populacja2_m[i].intercept;

			
			populacja_m[k*10 + i].sse = populacja2_m[i].sse;
			populacja_m[k*10 + i].mse = populacja2_m[i].mse;
			
			populacja_m[k*10 + i].attr_index = populacja2_m[i].attr_index;

		}
	}

}

void wypisz_attr (void) {

	int i;
	
	
	for (i = 0; i < POP; i++) {
		
		printf("%d para to: %c[%lu] attr_index: %.2f <<==>> %c[%lu] attr_index %.2f\n", i+1, f[i] -> gender, f[i] -> id, f[i] -> attr_index, m[i] -> gender, m[i] -> id, m[i] -> attr_index); 
	}
	
	return;
}



#if defined _WIN32
	#define WAL_TYP DWORD __stdcall
	#define WAL_RET (DWORD) 0
#else
	#define WAL_TYP void*
	#define WAL_RET (void*) walidacja	
#endif

WAL_TYP walidacja (void *args) {
	
	struct s_args_w *pArgs;
	pArgs = (struct s_args_w*)args;
	
	struct s_rekord *pRekord = pArgs -> pRekord;
	struct s_czlek *vec = pArgs -> populacja;
	size_t vec_cnt = pArgs -> lines;
	
	unsigned int i, j;
	double diff, diffsq;
	
	//puts("\n[VAL]");
	for (i=0; i < POP; i++) {
		
		vec[i].sse = 0;
				
		for (j=0; j < vec_cnt; j++) {
			
			// dany potomek przez wszystkie elementy wektorów zmiennych i funkcji celu = suma kwadratów odchyleń na koniec
			diff = pRekord[j].target - (vec[i].WA * pRekord[j].a + vec[i].WB * pRekord[j].b + vec[i].intercept);
			diffsq = diff * diff;
			vec[i].sse += diffsq;
		}
		
		vec[i].mse = vec[i].sse / vec_cnt;
		vec[i].attr_index = vec[i].mse;
		
		if ((vec[i].mse < fwynik.mse) || (fwynik.mse == 0)) {
			fwynik.id = vec[i].id;
			fwynik.generacja = vec[i].generacja;
			fwynik.gender = vec[i].gender;
			fwynik.WA = vec[i].WA;
			fwynik.WB = vec[i].WB;
			fwynik.intercept = vec[i].intercept;
			fwynik.sse = vec[i].mse;
			fwynik.mse = vec[i].mse;
		}
	
	//printf("Generacja %d, %c[%lu] MSE to %.2f\n", vec[i].generacja, vec[i].gender, vec[i].id, vec[i].mse);
	
	}
	
	
	return WAL_RET;
	
}


#if defined _WIN32
	#define SORT_TYP DWORD __stdcall
	#define SORT_RET (DWORD) 0
#else
	#define SORT_TYP void*
	#define SORT_RET (void*) sortuj
#endif

	SORT_TYP sortuj(void *args) {
		
		
		// jedyny możliwy argument to void *pointer, jeśli więcej argumentów niż 1 musi wskazywać na strukturę
		// uwaga, void pointer to jest adres przekazanej struktury a nie struktura!
		// aby argumenty działały musi być zmienna struktury args_ct i idpowiedni cast z void pointera
		
		struct args_ct *argumenty;
		argumenty = (struct args_ct*) args;
		
		struct s_czlek *vec;
		struct s_czlek **cel;
		
		vec = argumenty -> a_populacja;
		cel = argumenty -> a_p;
		
		// Dalej tak jak normalna funkcja
		
		int i, j, d;
		double a;
		
		long kolejnosc_id[POP];
		double kolejnosc[POP];
		
		struct s_czlek *p;
		
		//debug
		//printf("debug multi: %c\n",(const char) argumenty.deb);
		
		
		for (i = 0; i < POP; i++) {
			
			// przepisanie id oraz attr_index do osobnych tablic
			kolejnosc_id[i] = vec[i].id;
			kolejnosc[i] = vec[i].attr_index;
			// cel[i] = &vec[i]; //adres obecnego elementu to ma byc wartosc w tablicy pointerow na ktore wskazuje p lub f
			cel[i] = vec + i; // poprawniejszy zapis 
		}
		
		//sortowanie
		for (i = 0; i < POP; ++i)
		{
			for (j = i + 1; j < POP; ++j)
			{
				if (kolejnosc[i] > kolejnosc[j])
				{
					a =  kolejnosc[i];
					kolejnosc[i] = kolejnosc[j];
					kolejnosc[j] = a;
					
					d =  kolejnosc_id[i];
					kolejnosc_id[i] = kolejnosc_id[j];
					kolejnosc_id[j] = d;
					
					p =  cel[i];
					cel[i] = cel[j];
					cel[j] = p;
								
				}
			}
		}
		
		//printf("Posortowane wg indeksu attr:\n");
		//for (i = 0; i < POP; ++i) printf("Id: %d, indeks atrakcyjnosci: %f\n", kolejnosc_id[i], kolejnosc[i]);
		//for (i = 0; i < POP; ++i) printf("Id z wektora cel: %lu, indeks atrakcyjnosci z wektora cel: %f\n", cel[i] -> id, cel[i] -> attr_index);
		
		return SORT_RET;

	} 



int main (int argc, char **argv) {

	struct s_czlek *pIni;
	
	int minpok, i, j, generations;
	minpok = 2;
	
	FILE *fp;
	int lines = 0;
    int columns = 1;
	int koniec, ch;
	
	
	//reset generatora liczb losowych
	srand(time(NULL));
	
	
	
	if (argc < 3) {
	   printf("\n[!] USAGE: %s <filename.csv> <liczba pokoleń>\n\n", argv[0]);
	   exit(1);
	}
	
	if (argc > 3 && ((strcmp(argv[3],"s") == 0) || (strcmp(argv[3],"S") == 0))) {
		strcpy(mode,"ST");
	} else {
		strcpy(mode,"MT");
	}
	
	
	generations = atoi(argv[2]);
	if (generations < minpok) {
		printf("\n[!] Populacja powinna ewoluować co najmniej przez %d pokoleń\n\n", minpok);
		exit(4);
	}
	
	
    fp = fopen(argv[1], "r");
    
    if (fp == NULL) {
        puts("[-] Nie udało się otworzyć pliku, sprawdź czy podany plik isnieje!");
        exit(2);
    }
    
	printf("\n[+] Parametry algorytmu: N = %d, G = %d, %s\n", (POPF + POPM), generations, mode);
	printf("[+] Wczytywanie pliku %s...", argv[1]);
	fflush(stdout);
    
    // liczba kolumn - liczyny separator od 1
    while (!feof(fp)) {
        ch = fgetc(fp);
        if (ch == ',') columns++;
        if (ch == '\n') break;
    }
    
    // kursor znowu na początek
    fseek(fp, 0, SEEK_SET);
    
    // liczba wierszy
    // wszystkie znaki i sprawdzam ile razy jest '\n'
    while (!feof(fp)) {
        ch = fgetc(fp);
        if (ch == '\n') lines++;
    }
    
    
    
    // ta pętla przesunęła już kursor na koniec pliku
    koniec = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    printf(" Ok!\n");
	fflush(stdout);
	
	printf("[+] Alokacja pamięci...");
	fflush(stdout);
    
    struct s_rekord *pRekord = malloc ( lines * sizeof(struct s_rekord));
	
    if (pRekord == NULL) {
        puts("[!] Nie udało się zaalokować pamięci!");
		fclose(fp);
        exit(3);
    }
	
	printf(" Ok!\n");
	fflush(stdout);
	
	printf("[+] Rozmiar danych: %d bitów, liczba wierszy: %d, liczba kolumn: %d\n",koniec, lines, columns);
	puts("[+] Pierwsze 3 wiersze:");
    
    for (i = 1; i <= lines; i++) {
		double target, a, b;
        fscanf(fp,"\%lf,\%lf,\%lf", &target, &a, &b);
        if (i < 4) printf("    %d: target = %.2lf, a = %.2lf, b = %.2lf\n", i, target, a, b);
        
        (pRekord + (i-1)) -> target = target;
        pRekord[i-1].a = a;
        pRekord[i-1].b = b;
    }
            
    fclose(fp);
	
	
	// inicjalne wagi na podstawie średnich z wektorów predyktorów i targetu
	ustaw_wagi(pRekord, lines);
	printf("[+] Startowe oszacowanie wag: WA = %.4f, WB = %.4f, intercept = %.4f \n", wagi[0], wagi[1], wagi[2]);
	
	
	printf("[+] Inicjalizacja populacji... ");
	fflush(stdout);
	
		
	// inicjalizacja argumentów do struktur dla sortuj() i walidacja(), nie ma sensu tego robić w pętli więc teraz
	// struktura argumentów do wywołania funkcji sortuj() w wątkach
	struct args_ct ctf;
	ctf.a_populacja = populacja_f;
	ctf.a_p = f;
	ctf.deb = (const char*) 'F';
	
	struct args_ct ctm;
	ctm.a_populacja = populacja_m;
	ctm.a_p = m;
	ctm.deb = (const char*) 'M';
				
	// struktur argumentów do wywołania w funkcji walidacja() w wątkach	
	struct s_args_w args_wf;
	args_wf.pRekord = pRekord;
	args_wf.populacja = populacja_f;
	args_wf.lines = lines;
	
	struct s_args_w args_wm;
	args_wm.pRekord = pRekord;
	args_wm.populacja = populacja_m;
	args_wm.lines = lines;
	
	
	// inicjacja 1 generacji i nastepne pokolenia;
	for (i = 0; i <= generations; i++) {
		
		if (i > 0) generacjaCounter++; // licznik pokoleń
		
		
		// Obie populacje
		for (j = 0; j < POP; j++) {
			
			//tylko pierwsze pokolenie, inicjacja każdego elementu
			if (i == 0) {
			
				//można tak
				//inicjacja(&populacja_f[j],'F');
				//inicjacja(&populacja_m[j],'M');
				
				//ale zgodnie ze sztuką tak indeksuje się elementy tablicy wskaznikow
				//ponieważ inkrementujemy wskaźnik a nie wartość, index j wskazuje na nastepny element tablicy 
				pIni = inicjacja(populacja_f + j,'F');
				if (pIni == NULL) {
					puts("[!] Nie można zainicjować populacji F\n");
					free(pRekord);
					exit(5);
				}
				pIni = inicjacja(populacja_m + j,'M');
				if (pIni == NULL) {
					puts("[!] Nie można zainicjować populacji M\n");
					free(pRekord);
					exit(5);
				}
				
			}
				
		}
		
		// pierwsza iteracja przed potomstwem
		
		if (i == 0) {
			printf("Ok!\n");
			printf("[+] Szukam dopasowania... ");
			fflush(stdout);

			//printf("Ok!\n\nGeneracja +%d:\n",i);
			//wypisz_attr();
		}
		
		
		//if (i == 100 || i == 2000 || i == 15000) formula_dz = formula_dz / 10;
		
		// Wielowątkowo ?
		if (strcmp(mode,"MT") == 0) {
		
			// sortuj wielowątkowo:
			#if defined _WIN32
					
				HANDLE h[2] = {
					
					CreateThread(0,0,sortuj, (void*) &ctf,0,0),
					CreateThread(0,0,sortuj, (void*) &ctm,0,0)
				};
				
										
				WaitForMultipleObjects(2, h, TRUE, INFINITE);
		
			#else
				
				// zmienne do wątków w linuksie
				pthread_t thread1, thread2;
				int  iret1, iret2;
				
				iret1 = pthread_create( &thread1, NULL, sortuj, (void*) &ctf);
				 if(iret1)
				 {
					 fprintf(stderr,"Error - pthread_create() return code: %d\n",iret1);
					 free(pRekord);
					 exit(EXIT_FAILURE);
				 }
			 
				 iret2 = pthread_create( &thread2, NULL, sortuj, (void*) &ctm);
				 if(iret2)
				 {
					 fprintf(stderr,"Error - pthread_create() return code: %d\n",iret2);
					 free(pRekord);
					 exit(EXIT_FAILURE);
				 }
				 
				 
				pthread_join( thread1, NULL);
				pthread_join( thread2, NULL);

			#endif
			
			
			potomstwo10(f, m);
			
			
			// walidacja wielowątkowo:
			#if defined _WIN32
					
				HANDLE h2[2] = {
					
					CreateThread(0,0,walidacja, (void*) &args_wf,0,0),
					CreateThread(0,0,walidacja, (void*) &args_wm,0,0)
				};
				
										
				WaitForMultipleObjects(2, h2, TRUE, INFINITE);
		
			#else
				
				// zmienne do wątków w linuksie
				pthread_t thread3, thread4;
				int  iret3, iret4;
				
				iret3 = pthread_create( &thread3, NULL, walidacja, (void*) &args_wf);
				 if(iret3)
				 {
					 fprintf(stderr,"Error - pthread_create() return code: %d\n",iret3);
					 free(pRekord);
					 exit(EXIT_FAILURE);
				 }
			 
				 iret4 = pthread_create( &thread4, NULL, walidacja, (void*) &args_wm);
				 if(iret4)
				 {
					 fprintf(stderr,"Error - pthread_create() return code: %d\n",iret4);
					 free(pRekord);
					 exit(EXIT_FAILURE);
				 }
				 
				 
				pthread_join( thread3, NULL);
				pthread_join( thread4, NULL);
			
			#endif
		
		
		// Jednowątkowo (opcja "S" lub "s")
		} else {
		
			// gdyby jednowątkowo to:
			sortuj((void*) &ctf);
			sortuj((void*) &ctm);
						
			potomstwo10(f, m);
			
			walidacja((void*)&args_wf);
			walidacja((void*)&args_wm);
		}
		
	}
		
	
	printf("Gotowe!\n");
	#if defined _WIN32
		HANDLE kolor;
		kolor = GetStdHandle(STD_OUTPUT_HANDLE);
	#endif
	ZMIANA
	printf("[*] Najlepsze dopasowanie to: WA = %.4f, WB = %.4f, intercept = %.4f, mse = %.4f, residual standard error = %.4f\n", fwynik.WA, fwynik.WB, fwynik.intercept, fwynik.mse, pow(fwynik.mse,0.5));
	#if defined _WIN32
		kolor = GetStdHandle(STD_OUTPUT_HANDLE);
	#endif
	ORYG
	printf("[*] Znalezione w elemencie: id = %d, Generacja = %d, populacja = %c\n\n", fwynik.id, fwynik.generacja, fwynik.gender);
	
	
	free(pRekord);

	
	return 0;

}
