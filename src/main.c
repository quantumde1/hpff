#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

#define FNAME_LEN 255  // Renamed from X to FNAME_LEN

void Y(const char *Z, char **A, int B) {
    FILE *C = fopen(Z, "wb");
    if (!C) {
        perror("Error");
        return;
    }

    fwrite(&B, sizeof(uint32_t), 1, C);

    for (int D = 0; D < B; D++) {
        const char *E = A[D];
        char F[FNAME_LEN];  // Use FNAME_LEN instead of X
        struct stat G;

        strcpy(F, E);
        char *H = strrchr(F, '/');
        if (H) {
            H++;
        } else {
            H = F;
        }

        if (stat(E, &G) != 0) {
            perror("Error");
            continue;
        }

        uint8_t I = strlen(H);
        fwrite(&I, sizeof(uint8_t), 1, C);
        fwrite(H, sizeof(char), I, C);

        uint32_t J = G.st_size;
        fwrite(&J, sizeof(uint32_t), 1, C);

        FILE *K = fopen(E, "rb");
        if (!K) {
            perror("Error");
            continue;
        }
        char *L = malloc(J);
        fread(L, sizeof(char), J, K);
        fwrite(L, sizeof(char), J, C);
        free(L);
        fclose(K);
    }

    fclose(C);
    printf("Packed %d files into %s\n", B, Z);
}

void M(const char *N, const char *O) {
    FILE *P = fopen(N, "rb");
    if (!P) {
        perror("Error");
        return;
    }

    uint32_t Q;
    fread(&Q, sizeof(uint32_t), 1, P);

    for (uint32_t R = 0; R < Q; R++) {
        uint8_t S;
        fread(&S, sizeof(uint8_t), 1, P);
        char *T = malloc(S + 1);
        fread(T, sizeof(char), S, P);
        T[S] = '\0';

        uint32_t U;
        fread(&U, sizeof(uint32_t), 1, P);

        char *V = malloc(U);
        fread(V, sizeof(char), U, P);

        char W[FNAME_LEN];  // Use FNAME_LEN instead of X
        snprintf(W, sizeof(W), "%s/%s", O, T);

        FILE *X = fopen(W, "wb");
        if (X) {
            fwrite(V, sizeof(char), U, X);
            fclose(X);
        } else {
            perror("Error");
        }

        free(T);
        free(V);
    }

    fclose(P);
    printf("Unpacked %d files to %s\n", Q, O);
}

char* get_file_data_from_archive(const char *input_file, const char *file_name, uint32_t *file_size_out) {
    FILE *f = fopen(input_file, "rb");
    if (!f) {
        perror("Failed to open input file");
        return NULL;
    }

    // Читаем количество файлов
    uint32_t num_files;
    fread(&num_files, sizeof(uint32_t), 1, f);

    for (uint32_t i = 0; i < num_files; i++) {
        // Читаем длину имени файла и само имя
        uint8_t name_length;
        fread(&name_length, sizeof(uint8_t), 1, f);
        char *current_file_name = malloc(name_length + 1);
        fread(current_file_name, sizeof(char), name_length, f);
        current_file_name[name_length] = '\0'; // Завершаем строку

        // Читаем размер файла
        uint32_t file_size;
        fread(&file_size, sizeof(uint32_t), 1, f);

        // Проверяем, совпадает ли имя файла с запрашиваемым
        if (strcmp(current_file_name, file_name) == 0) {
            // Файл найден, читаем его данные
            char *buffer = malloc(file_size);
            fread(buffer, sizeof(char), file_size, f);
            fclose(f);
            free(current_file_name);
            *file_size_out = file_size; // Возвращаем размер файла
            return buffer; // Возвращаем данные файла
        }

        // Пропускаем данные файла, если не найдено
        fseek(f, file_size, SEEK_CUR);
        free(current_file_name);
    }

    printf("File '%s' not found in the archive.\n", file_name);
    fclose(f);
    return NULL;
}