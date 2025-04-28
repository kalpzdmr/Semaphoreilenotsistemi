#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#define MAX_OGRENCI 1000
#define MAX_PUAN 1000

typedef struct {
    int id;
    int puan[MAX_PUAN];
    int num_puan;
} Ogrenci;

int num_ogrenci, num_puan;
Ogrenci ogrenciler[MAX_OGRENCI];
float ortalama[MAX_OGRENCI];
char ogrenci_kimligi[MAX_OGRENCI][10];
int gecen_sayisi[MAX_PUAN] = {0};
int toplam_gecen_ogrenciler = 0;
int en_yuksek_puan = -1;
int en_dusuk_puan = 101;

sem_t stats_semaphore;

void* process_ogrenci(void* arg) {
    int index = *(int*)arg;
    Ogrenci* ogrenci = &ogrenciler[index];
    float top = 0;
    int i;

    for (i = 0; i < ogrenci->num_puan; i++) {
        top += ogrenci->puan[i];
    }
    ortalama[index] = top / ogrenci->num_puan;
    strcpy(ogrenci_kimligi[index], ortalama[index] >= 60 ? "Passed" : "Failed");

    sem_wait(&stats_semaphore);
    for (i = 0; i < ogrenci->num_puan; i++) {
        if (ogrenci->puan[i] >= 60) {
            gecen_sayisi[i]++;
        }
        if (ogrenci->puan[i] > en_yuksek_puan) {
            en_yuksek_puan = ogrenci->puan[i];
        }
        if (ogrenci->puan[i] < en_dusuk_puan) {
            en_dusuk_puan = ogrenci->puan[i];
        }
    }
    if (ortalama[index] >= 60) {
        toplam_gecen_ogrenciler++;
    }
    sem_post(&stats_semaphore);

    free(arg);
    return NULL;
}

int main() {
    FILE *input_file, *output_file;
    pthread_t threads[MAX_OGRENCI];

    input_file = fopen("input.txt", "r");

    fscanf(input_file, "%d %d", &num_ogrenci, &num_puan);
    for (int i = 0; i < num_ogrenci; i++) {
        fscanf(input_file, "%d", &ogrenciler[i].id);
        for (int j = 0; j < num_puan; j++) {
            fscanf(input_file, "%d", &ogrenciler[i].puan[j]);
        }
        ogrenciler[i].num_puan = num_puan;
    }
    fclose(input_file);

    sem_init(&stats_semaphore, 0, 1);

    for (int i = 0; i < num_ogrenci; i++) {
        int* arg = malloc(sizeof(*arg));
        *arg = i;
        pthread_create(&threads[i], NULL, process_ogrenci, arg);
    }

    for (int i = 0; i < num_ogrenci; i++) {
        pthread_join(threads[i], NULL);
    }

    output_file = fopen("results.txt", "w"); //results ile gönderilecek

    for (int i = 0; i < num_ogrenci; i++) {
        fprintf(output_file, "%d %.2f %s\n", ogrenciler[i].id, ortalama[i], ogrenci_kimligi[i]);
    }
	fprintf(output_file,"\n");
    fprintf(output_file, "--- Overall Statistics ---\n");
    fprintf(output_file, "Number of students passing each question:\n");
    for (int i = 0; i < num_puan; i++) {
        fprintf(output_file, "Question %d: %d students passed.\n", i + 1, gecen_sayisi[i]);
    }
    fprintf(output_file, "Total number of students who passed overall: %d\n", toplam_gecen_ogrenciler);
    fprintf(output_file, "Highest grade: %d\n", en_yuksek_puan);
    fprintf(output_file, "Lowest grade: %d\n", en_dusuk_puan);

    fclose(output_file);
    sem_destroy(&stats_semaphore);

    return 0;
}
