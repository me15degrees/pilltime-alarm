#include "BluetoothSerial.h"
#include <Preferences.h>
#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include <time.h>
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library

#define TFT_GREY 0x5AEB
#define PURPLE_LIGHT 0xC80FF
#define YELLOW_LIGHT 0xFFFF00
#define WIFI_SSID "Galaxy 2S1 FE 5Gb878"
#define WIFI_PASSWORD "cocozeira"
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTOR_EMAIL "projetopilltimealarm@gmail.com"
#define AUTOR_SENHA "uiie xirw uurf bfdm" // Removidos espaços extras

BluetoothSerial SerialBT;
Preferences prefs;

#define MAX_EMAILS 10
#define MAX_LENGTH 80

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

uint32_t targetTime = 0;                    // for next 1 second timeout

static uint8_t conv2d(const char* p); // Forward declaration needed for IDE 1.6.x

uint8_t hh = conv2d(__TIME__), mm = conv2d(__TIME__ + 3), ss = conv2d(__TIME__ + 6); // Get H, M, S from compile time

byte omm = 99, oss = 99;
byte xcolon = 0, xsecs = 0;
unsigned int colour = 0;

struct Medicamento {
  char remedio[MAX_LENGTH];
  char dosagem[MAX_LENGTH];
  int intervalo;
  int quantidade;
  bool habilitada;
} container1, container2;

char emails[MAX_EMAILS][MAX_LENGTH];
int count_email = 0;

void cadastro_remedio(Medicamento& medicamento);
void trocar_dados(char *ct);
void mostra_remedio(const Medicamento& medicamento);
void editar_emails();
void salvarDados();
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
void printText(char *text, uint16_t color, int x, int y, int textSize) {
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextSize(textSize);
  tft.setTextWrap(true);
  tft.print(text);
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("PillAlarm");
  Serial.println("O dispositivo já pode ser pareado!");

  prefs.begin("dados_remedios", false); // Modo leitura/escrita
  prefs.getBytes("Emails", emails, sizeof(emails));
  count_email = prefs.getInt("CountEmail");
  prefs.end();
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

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(PURPLE_LIGHT);

  tft.setTextSize(1);
  tft.setTextColor(YELLOW_LIGHT, TFT_BLACK);

  targetTime = millis() + 1000;
}

void loop() {
  int containerNumber = -1; // Inicializamos com um valor inválido
  int numeroEmail = -1; // Inicializamos com um valor inválido

  SerialBT.println("Seja bem-vindo ao Pill Alarm!\n");
  SerialBT.println("Escolha uma opção:");
  SerialBT.println("1. Cadastrar um remédio");
  SerialBT.println("2. Alterar dados de um remédio");
  SerialBT.println("3. Cadastrar um e-mail");
  SerialBT.println("4. Alterar dados de um e-mail");
  SerialBT.println("5. Mostrar remédios cadastrados");
  SerialBT.println("6. Mostrar e-mails cadastrados");
  SerialBT.println("7. Sair\n");

  if (targetTime < millis()) {
    // Set next update for 1 second later
    targetTime = millis() + 1000;

    // Adjust the time values by adding 1 second
    ss++;              // Advance second
    if (ss == 60) {    // Check for roll-over
      ss = 0;          // Reset seconds to zero
      omm = mm;        // Save last minute time for display update
      mm++;            // Advance minute
      if (mm > 59) {   // Check for roll-over
        mm = 0;
        hh++;          // Advance hour
        if (hh > 23) { // Check for 24hr roll-over (could roll-over on 13)
          hh = 0;      // 0 for 24 hour clock, set to 1 for 12 hour clock
        }
      }
    }


    // Update digital time
    int xpos = 0;
    int ypos = 85; // Top left corner ot clock text, about half way down
    int ysecs = ypos + 24;

    if (omm != mm) { // Redraw hours and minutes time every minute
      omm = mm;
      // Draw hours and minutes
      if (hh < 10) xpos += tft.drawChar('0', xpos, ypos, 8); // Add hours leading zero for 24 hr clock
      xpos += tft.drawNumber(hh, xpos, ypos, 8);             // Draw hours
      xcolon = xpos; // Save colon coord for later to flash on/off later
      xpos += tft.drawChar(':', xpos, ypos - 8, 8);
      if (mm < 10) xpos += tft.drawChar('0', xpos, ypos, 8); // Add minutes leading zero
      xpos += tft.drawNumber(mm, xpos, ypos, 8);             // Draw minutes
      xsecs = xpos; // Sae seconds 'x' position for later display updates
    }
    if (oss != ss) { // Redraw seconds time every second
      oss = ss;
      xpos = xsecs;

      if (ss % 2) { // Flash the colons on/off
        tft.setTextColor(0x39C4, TFT_BLACK);        // Set colour to grey to dim colon
        tft.drawChar(':', xcolon, ypos - 8, 8);     // Hour:minute colon
        xpos += tft.drawChar(':', xsecs, ysecs, 6); // Seconds colon
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);    // Set colour back to yellow
      }
      else {
        tft.drawChar(':', xcolon, ypos - 8, 8);     // Hour:minute colon
        xpos += tft.drawChar(':', xsecs, ysecs, 6); // Seconds colon
      }

      //Draw seconds
      if (ss < 10) xpos += tft.drawChar('0', xpos, ysecs, 6); // Add leading zero
      tft.drawNumber(ss, xpos, ysecs, 6);                     // Draw seconds
    }
  }


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

  if (!SerialBT.available()) {
    delay(10);
    int opcao =(char) SerialBT.read() - '0';
    switch (opcao) {
      case 1:
        SerialBT.println("Para qual dos container será este remédio (1 ou 2)?");
        int container;
        while (!SerialBT.available()) {
          delay(10);
        }
        container = SerialBT.read() - '0';
        if (container == 1 || container == 2) {
          Medicamento novoMedicamento;
          cadastro_remedio(novoMedicamento);
          if (container == 1) {
            prefs.begin("dados_remedios", false);
            prefs.putBytes("Container1", &novoMedicamento, sizeof(novoMedicamento));
            prefs.end();
          } else {
            prefs.begin("dados_remedios", false);
            prefs.putBytes("Container2", &novoMedicamento, sizeof(novoMedicamento));
            prefs.end();
          }
        } else {
          Serial.println("Opção inválida.");
        }
        break;

      case 2:
        SerialBT.println("Em qual container está o remédio que deseja alterar (1 ou 2)?");
        while (!SerialBT.available()) {
          delay(10);
        }
        containerNumber = SerialBT.read() - '0';
        if (containerNumber == 1 || containerNumber == 2) {
          Medicamento* medicamento;
          if (containerNumber == 1) {
            medicamento = &container1;
          } else {
            medicamento = &container2;
          }

          SerialBT.println("Escolha o dado que deseja alterar:");
          SerialBT.println("1. Nome do remédio");
          SerialBT.println("2. Dosagem do remédio");
          SerialBT.println("3. Intervalo de tempo da medicação (em horas)");
          SerialBT.println("4. Quantidade do medicamento no container");

          while (!SerialBT.available()) {
            delay(10);
          }
          int option = SerialBT.read() - '0';

          switch (option) {
            case 1:
              trocar_dados(medicamento->remedio);
              break;
            case 2:
              trocar_dados(medicamento->dosagem);
              break;
            case 3:
              SerialBT.println("Digite o novo intervalo de tempo da medicação (em horas):");
              while (!SerialBT.available()) {
                delay(10);
              }
              medicamento->intervalo = SerialBT.parseInt();
              break;
            case 4:
              SerialBT.println("Digite a nova quantidade do medicamento no container:");
              while (!SerialBT.available()) {
                delay(10);
              }
              medicamento->quantidade = SerialBT.parseInt();
              break;
            default:
              SerialBT.println("Opção inválida.");
              break;
          }

          // Salvar as alterações nos dados persistidos
          prefs.begin("dados_remedios", false);
          if (containerNumber == 1) {
            prefs.putBytes("Container1", &container1, sizeof(container1));
          } else {
            prefs.putBytes("Container2", &container2, sizeof(container2));
          }
          prefs.end();
        } else {
          SerialBT.println("Opção inválida.");
        }
        break;

      case 3:
        if (count_email < MAX_EMAILS) {
          SerialBT.println("Digite o email que deseja cadastrar:");
          while (!SerialBT.available()) {
            delay(10);
          }
          String novoEmail = SerialBT.readStringUntil('\n');

          if (novoEmail.length() > 0) {
            novoEmail.toCharArray(emails[count_email], MAX_LENGTH);
            count_email++;

            SerialBT.println("Email cadastrado com sucesso!");
          } else {
            SerialBT.println("Email inválido.");
          }
        } else {
          SerialBT.println("Capacidade máxima de emails atingida.");
        }
        break;

      case 4:
        SerialBT.println("Digite o número do email que deseja alterar:");
        while (!SerialBT.available()) {
          delay(10);
        }
        numeroEmail = SerialBT.parseInt();

        if (numeroEmail >= 0 && numeroEmail < count_email) {
          SerialBT.println("Digite o novo email:");
          while (!SerialBT.available()) {
            delay(10);
          }
          String novoEmail = SerialBT.readStringUntil('\n');

          if (novoEmail.length() > 0) {
            novoEmail.toCharArray(emails[numeroEmail], MAX_LENGTH);
            SerialBT.println("Email alterado com sucesso!");
          } else {
            SerialBT.println("Email inválido.");
          }
        } else {
          SerialBT.println("Número de email inválido.");
        }
        break;

      case 5:
        // Ler os dados dos remédios salvos
        prefs.begin("dados_remedios", false); // Modo leitura
        prefs.getBytes("Container1", &container1, sizeof(container1));
        prefs.getBytes("Container2", &container2, sizeof(container2));
        prefs.end();

        // Mostrar os detalhes dos remédios
        SerialBT.println("Remédios cadastrados:");
        mostra_remedio(container1);
        mostra_remedio(container2);
        break;

      case 6:
        SerialBT.println("Emails cadastrados:");
        for (int i = 0; i < count_email; ++i) {
          Serial.println(emails[i]);
        }
        break;

      case 7:
        SerialBT.println("Até logo!");
        delay(1000);
        ESP.restart();
        break;

      default:
        SerialBT.println("Opção inválida. Tente novamente.");
    }
  }
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

void cadastro_remedio(Medicamento& medicamento) {
  SerialBT.println("Digite o nome do remédio:");
  while (!SerialBT.available()) {
    delay(10);
  }
  SerialBT.readStringUntil('\n').toCharArray(medicamento.remedio, MAX_LENGTH);

  Serial.println("Digite a dosagem do remédio:");
  while (!SerialBT.available()) {
    delay(10);
  }
  SerialBT.readStringUntil('\n').toCharArray(medicamento.dosagem, MAX_LENGTH);

  SerialBT.println("Digite o intervalo de tempo da medicação (em horas):");
  while (!SerialBT.available()) {
    delay(10);
  }
  medicamento.intervalo = SerialBT.parseInt();

  SerialBT.println("Digite a quantidade do medicamento que será colocada no container:");
  while (!SerialBT.available()) {
    delay(10);
  }
  medicamento.quantidade = SerialBT.parseInt();

  medicamento.habilitada = true;
}

void trocar_dados(char *ct) {
  SerialBT.println("Digite o novo dado:");
  while (!SerialBT.available()) {
    delay(10);
  }
  SerialBT.readStringUntil('\n').toCharArray(ct, MAX_LENGTH);
}

void mostra_remedio(const Medicamento& medicamento) {
  SerialBT.println("Remédio:");
  SerialBT.println(medicamento.remedio);
  SerialBT.println("Dosagem:");
  SerialBT.println(medicamento.dosagem);
  SerialBT.println("Intervalo do tempo (em horas):");
  SerialBT.println(medicamento.intervalo);
  SerialBT.println("Quantidade restante no container:");
  SerialBT.println(medicamento.quantidade);
}

void editar_emails() {
  char em[MAX_LENGTH];
  int p, v;
  Serial.println("Digite o email que quer alterar:");
  while (!SerialBT.available()) {
    delay(10);
  }
  SerialBT.readStringUntil('\n').toCharArray(em, MAX_LENGTH);

  for (p = 0; p < count_email; p++) {
    if (strcmp(em, emails[p]) == 0) {
      v = p;
      break;
    }
  }

  Serial.println("Digite o novo endereço de email:");
  while (!SerialBT.available()) {
    delay(10);
  }
  SerialBT.readStringUntil('\n').toCharArray(emails[v], MAX_LENGTH);
}

void salvarDados() {
  prefs.begin("dados_remedios", false); // Modo leitura/escrita
  prefs.putInt("CountEmail", count_email);
  for (int i = 0; i < count_email; i++) {
    prefs.putString(("Email" + String(i)).c_str(), String(emails[i]).c_str());
  }
  prefs.end();
}

// Function to extract numbers from compile time string
static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}

