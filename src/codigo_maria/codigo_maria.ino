#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include <time.h>

#define WIFI_SSID "KarenRonan_Ext"
#define WIFI_PASSWORD "aventura2022"
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTOR_EMAIL "projetopilltimealarm@gmail.com"
#define AUTOR_SENHA "uiie xirw uurf bfdm" // Removidos espaços extras

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
    
    String obterHoraAtual() {
        return String(getHora()) + ":" + String(getMinuto()) + ":" + String(getSegundo());
    }
};

SMTPSession smtp;

bool enviaEmail(String messageTXT, String assunto, String emailDestinatario);

void smtpCallback(SMTP_Status status);

void setup() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
            Serial.println("Conectando ao WiFi...");
        }
        Serial.println("Conectado ao WiFi!");
    Serial.begin(115200);
    delay(1000);

    Serial.println("\nEnviar e-mail, digitar parâmetro");

    smtp.debug(1);
    smtp.callback(smtpCallback);
}

void loop() {
    // Verifica se há algo na porta serial
    if (Serial.available() > 0) {
        int parametro = Serial.parseInt(); // Lê o parâmetro recebido

        if (parametro == 1) {
            // Parâmetro 1: Remédio tomado no horário
            DateTimeModule dateTime; // Cria uma instância do módulo de data e hora
            String horaAtual = dateTime.obterHoraAtual(); // Obtém a hora atual do módulo
            String mensagem = "Remédio tomado no horário " + horaAtual;
            String assunto = "Remédio tomado";
            String destinatario = "me15degrees@gmail.com"; // Insira o e-mail do destinatário
            enviaEmail(mensagem, assunto, destinatario);
        } else if (parametro == 2) {
            // Parâmetro 2: Remédio não tomado no prazo de 15 minutos
            String mensagem = "Remédio não foi tomado no prazo de 15 minutos";
            String assunto = "Aviso de atraso no remédio";
            String destinatario = "me15degrees@gmail.com"; // Insira o e-mail do destinatário
            enviaEmail(mensagem, assunto, destinatario);
        } else {
            Serial.println("Parâmetro inválido");
        }
    }
    delay(1000); // Adiciona um pequeno atraso para evitar a execução rápida do loop
}

bool enviaEmail(String messageTXT, String assunto, String emailDestinatario) {
    ESP_Mail_Session session;
    session.server.host_name = SMTP_HOST;
    session.server.port = SMTP_PORT;
    session.login.email = AUTOR_EMAIL;
    session.login.password = AUTOR_SENHA;
    session.login.user_domain = "";
    session.time.ntp_server = F("time.google.com");
    session.time.gmt_offset = -3;
    session.time.day_light_offset = 0;

    SMTP_Message message;
    message.sender.name = "PillTime Alarm";
    message.sender.email = AUTOR_EMAIL;
    message.subject = assunto;
    message.addRecipient("", emailDestinatario); // Insira o nome do destinatário, se necessário
    message.text.content = messageTXT.c_str();
    message.text.charSet = "utf-8";
    message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
    message.response.notify = esp_mail_smtp_notify_success |
                                esp_mail_smtp_notify_failure |
                                esp_mail_smtp_notify_delay;

    if (!smtp.connect(&session))
        return false;

    if (!MailClient.sendMail(&smtp, &message)) {
        Serial.println("Erro ao enviar e-mail, " + smtp.errorReason());
        return false;
    }
    return true;
}

void smtpCallback(SMTP_Status status) {
    Serial.println(status.info());
    if (status.success()) {
        Serial.println("----------------");
        ESP_MAIL_PRINTF("Mensagem enviada com sucesso: %d\n", status.completedCount());
        ESP_MAIL_PRINTF("Falha na mensagem enviada: %d\n", status.failedCount());
        Serial.println("----------------\n");
        struct tm dt;

        for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
            SMTP_Result result = smtp.sendingResult.getItem(i);
            time_t ts = (time_t)result.timestamp;
            localtime_r(&ts, &dt);

            ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
            ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "sucesso" : "fracassado");
            ESP_MAIL_PRINTF("Data/Hora: %d/%d/%d %d:%d:%d\n",
                            dt.tm_year + 1900,
                            dt.tm_mon + 1,
                            dt.tm_mday,
                            dt.tm_hour,
                            dt.tm_min,
                            dt.tm_sec);
            ESP_MAIL_PRINTF("Recebedor: %s\n", result.recipients);
            ESP_MAIL_PRINTF("Sujeito: %s\n", result.subject);
        }
        Serial.println("----------------\n");
    }
}
