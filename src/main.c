/* Contador de números primos
 *
 * Este programa recebera uma serie números inteiros positivos em sua
 * entrada. Ao receber um caractere fim de linha ('\n'), deve imprimir na tela a
 * quantidade de números primos que recebeu e, apos, encerrar.
 * Foi empregada a tecnica de Threadpool
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#define N_THREADS_MAX 4
#define ENTRADA_MAX 50
unsigned int n_primos;
pthread_mutex_t trava_pool, trava_args;

typedef struct {
    unsigned long N;
    uint8_t *THREAD;
} thread_args;

/*
 *  Verifica se x é primo.
 *  @return: 1 se x é primo, zero caso contrário
 */
int ehPrimo(unsigned long x);

/*
 *  Rotina de uma thread para calcular o primo
 *  @param: arg é o endereço do inteiro que se vai calcular o primo
 */
void *calculaPrimoThread(void *arg);


int main() {
    //Vetor dos elementos de entrada
    unsigned long entrada[ENTRADA_MAX];

    int index = 0;
    // Lê todos os numeros da entrada antes
    while(scanf("%lu ", &(entrada[index])) != EOF) index++;

    //Contador de primos compartilhado entre processos
    n_primos = 0;

    pthread_t threads[N_THREADS_MAX];
    uint8_t thread_ativa[N_THREADS_MAX];
    for(int i = 0; i < N_THREADS_MAX; i++)
        thread_ativa[i] = 0;

    unsigned int thread_index = 0;
    thread_args args;

    for(int i = 0; i < index; i++) {
        //Procura por um processo livre
        //printf("Verificando processo livre\n");
        while(1) {
            pthread_mutex_lock(&trava_pool);
            if(thread_ativa[thread_index] == 0)
            {
                //printf("Processo livre encontrado! %d\n", thread_index);
                thread_ativa[thread_index] = 1;
                pthread_mutex_lock(&trava_args);
                args.N = entrada[i];
                args.THREAD = &(thread_ativa[thread_index]);
                pthread_mutex_unlock(&trava_args);
                pthread_create(&(threads[thread_index]), NULL, calculaPrimoThread, &args);
                pthread_mutex_unlock(&trava_pool);
                break;
            }
            pthread_mutex_unlock(&trava_pool);
            thread_index = (thread_index+1)%N_THREADS_MAX;
        }

    }

    for(int i = 0; i < N_THREADS_MAX; i++)
        pthread_join(threads[i], NULL);

    printf("%d\n", n_primos);

    return 0;
}


int ehPrimo(unsigned long x) {
    if(x < 2)
        return 0;
    //Um numero primo só é divisível por 1 e por ele mesmo
    //Testamos a divisibilidade com i, 1<i<x
    for(unsigned long i = 2; i < x; i++) {
        if(!(x%i))
            return 0;
    }
    return 1;
}

void *calculaPrimoThread(void *arg) {

    pthread_mutex_lock(&trava_args);
    thread_args arg_local = * (thread_args *)arg;
    pthread_mutex_unlock(&trava_args);

    unsigned long N = arg_local.N;
    uint8_t *end_thread = arg_local.THREAD;

    //printf("Calculando se %lu eh primo (%p)\n", N, end_thread);
    uint8_t N_eh_primo = ehPrimo(N);
    pthread_mutex_lock(&trava_pool);
    if(N_eh_primo) {
        n_primos++;
    }
    *end_thread = 0;
    pthread_mutex_unlock(&trava_pool);
    return NULL;
}
