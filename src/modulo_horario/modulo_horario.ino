#include <WiFi.h>
#include <time.h>

class DateTimeModule {
private:
    int dia;
    int mes;
    int ano;
    int hora;
    int minuto;
    int segundo;

public:
    DateTimeModule() {
        // Conecta-se à rede WiFi
        WiFi.begin("Galaxy S21 FE 5Gb878", "cocozeira");
        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
            Serial.println("Conectando ao WiFi...");
        }
        Serial.println("Conectado ao WiFi!");

        // Configura o RTC para GMT-3 (3 horas a menos que o UTC)
        configTime(-3 * 3600, 0, "pool.ntp.org"); // Configura para usar o servidor NTP e subtrai o deslocamento de 3 horas

        Serial.println("Obtendo o horário...");

        // Aguarda a sincronização do horário
        time_t now = time(nullptr);
        while (now < 1609459200) { // Verifica se o horário é válido (01/01/2021)
            delay(1000);
            Serial.println("Aguardando sincronização do horário...");
            now = time(nullptr);
        }

        // Obtem a data e hora atual
        struct tm *localTime = localtime(&now);

        // Armazena a data em variáveis separadas
        dia = localTime->tm_mday;
        mes = localTime->tm_mon + 1; // Adiciona 1 pois tm_mon começa de 0 (janeiro é 0)
        ano = localTime->tm_year + 1900; // tm_year é o ano desde 1900

        // Armazena o horário em variáveis separadas
        hora = localTime->tm_hour;
        minuto = localTime->tm_min;
        segundo = localTime->tm_sec;
    }

    int getDia() { return dia; }
    int getMes() { return mes; }
    int getAno() { return ano; }
    int getHora() { return hora; }
    int getMinuto() { return minuto; }
    int getSegundo() { return segundo; }
};

