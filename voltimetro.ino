/* Este código funciona com sensor de tensão CA ESP8266 12E ou Arduino e ZMPT101B até 250 VAC 50/60 Hz
* Permite a medição do valor True RMS de qualquer sinal AC, não apenas a onda senoidal
* O código usa o método Sigma "Desvio padrão" e exibe o valor a cada "período de impressão"
*/
#define BLYNK_TEMPLATE_ID "<BLYNK_TAMPLATE_ID>"
#define BLYNK_DEVICE_NAME "<BLYNK_DEVICE_NAME>"
#define BLYNK_AUTH_TOKEN "<BLYNK_AUTH_TOKEN>"


#define LED 16
#define BLYNK_PRINT Serial
#define ZMPT101B A0                             // Entrada analógica

#include <Filters.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "<WIFI_SSID>";
char pass[] = "<WIFI_PASSWORD>";

float testFrequency = 50;                     // teste de frequência do sinal (Hz)
float windowLength = 100/testFrequency;       // quanto tempo para calcular a média do sinal, para estatista, mudar isso pode ter um efeito drástico
                                              // Teste conforme necessário

int RawValue = 0;     
double Volts_TRMS;     // tensão real estimada em Volts

float intercept = -2.2;  // a ser ajustado com base no teste de calibração
float slope = 127/100.95;      

/* Como obter a interceptação e o declive? Primeiro, mantenha-os como acima, intercepte = 0 e declive = 1,
 * também abaixo continua exibindo valores calibrados e não calibrados para ajudá-lo neste processo.
 * Coloque a entrada AC como 0 Volts, carregue o código e verifique o monitor serial, normalmente você deve ter 0
 * se você vir outro valor e ele estiver estável, a interceptação será o oposto desse valor
 * Exemplo, você carrega pela primeira vez e então vê um 1,65 V estável, então a interceptação será -1,65
 * Para definir a inclinação agora, você precisa colocar a tensão em algo superior a 0 e medir isso usando seu multímetro TRMS de referência
 * carregue o novo código com a nova interceptação e verifique o valor exibido como valores calibrados
 * Slope = (Valores medidos pelo multímetro) / (Valores medidos pelo código)
 * Coloque sua nova inclinação e recarregue o código, se você tiver problemas com a calibração, tente ajustar os dois
 * ou adicione uma nova linha para calibrar ainda mais
 * Inclinação e interceptação não têm nada a ver com o cálculo do TRMS, é apenas um ajuste da linha de soluções
 */

unsigned long printPeriod = 1000; //A frequência de medição, a cada 1s, pode ser alterada
unsigned long previousMillis = 0;

RunningStatistics inputStats; //Esta classe coleta o valor para que possamos aplicar algumas funções

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(D0, OUTPUT);
  Blynk.begin(auth, ssid, pass);

  Serial.begin(115200);    // inicie a porta serial
  Serial.println("Série iniciada");
  inputStats.setWindowSecs( windowLength );  
}


double ReadVoltage(){
    RawValue = analogRead(ZMPT101B);  // leia o valor analógico:
    inputStats.input(RawValue);       // logar na função de estatísticas
        
    if((unsigned long)(millis() - previousMillis) >= printPeriod) { //Calculamos e exibimos a cada 1s
      previousMillis = millis();   // tempo de atualização
      
      Volts_TRMS = inputStats.sigma()* slope + intercept;
      Volts_TRMS = Volts_TRMS*0.979; // Calibração adicional, se necessário
      
      Serial.print("Não calibrado: ");
      Serial.print("\t");
      Serial.print(inputStats.sigma()); 
      Serial.print("\t");
      Serial.print("Calibrado: ");
      Serial.print("\t");
      Serial.println(Volts_TRMS);    

      Blynk.virtualWrite(V4, Volts_TRMS);
    }

  
}

BLYNK_WRITE(V0) {
    if (V0 > 0) {
      digitalWrite(LED, HIGH);
    }
    else{
      digitalWrite(LED, LOW);
    } 
}

BLYNK_READ(V0) {
    if (V0 > 0) {
      digitalWrite(LED, HIGH);
    }
    else{
      digitalWrite(LED, LOW);
    } 
}

// BLYNK_CONNECTED()
// {
//   // Change Web Link Button message to "Congratulations!"
//   Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
//   Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
//   Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
// }

void loop() {
    Blynk.run();     

    ReadVoltage();   // A única função que estou executando, tome cuidado ao usar com este tipo de placa
                      // Não use atrasos muito longos ou loops infinitos dentro do loop                      
}
