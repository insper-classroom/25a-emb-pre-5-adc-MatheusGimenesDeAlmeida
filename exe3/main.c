#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

#include "data.h"
QueueHandle_t xQueueData;

// não mexer! Alimenta a fila com os dados do sinal
void data_task(void *p) {
    vTaskDelay(pdMS_TO_TICKS(400));

    int data_len = sizeof(sine_wave_four_cycles) / sizeof(sine_wave_four_cycles[0]);
    for (int i = 0; i < data_len; i++) {
        xQueueSend(xQueueData, &sine_wave_four_cycles[i], 1000000);
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void process_task(void *p) {
    int janela[5] = {0};   // guarda as últimas 5 amostras
    int pos = 0;           // posição atual no vetor circular
    int count = 0;         // conta quantas amostras já foram lidas

    int nova_amostra = 0;

    while (true) {
        // tenta receber uma nova amostra da fila (timeout de 100 ticks)
        if (xQueueReceive(xQueueData, &nova_amostra, pdMS_TO_TICKS(100))) {
            // adiciona a nova amostra na posição atual da janela
            janela[pos] = nova_amostra;

            // avança a posição (circular: 0 → 1 → ... → 4 → 0 ...)
            pos = (pos + 1) % 5;

            // aumenta a contagem de amostras, até no máximo 5
            if (count < 5) count++;

            // só calcula a média se já tivermos 5 amostras
            if (count == 5) {
                int soma = 0;
                for (int i = 0; i < 5; i++) {
                    soma += janela[i];
                }
                int media = soma / 5;
                printf("%d\n", media); // imprime só a média
            }

            // manter esse delay
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}


int main() {
    stdio_init_all();

    xQueueData = xQueueCreate(64, sizeof(int));

    xTaskCreate(data_task, "Data task ", 4096, NULL, 1, NULL);
    xTaskCreate(process_task, "Process task", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
