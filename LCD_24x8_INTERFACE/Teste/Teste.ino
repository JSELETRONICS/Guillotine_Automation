// Faixa de Valores do Eixo X: 1 --> 21; Eixo Y: 0 --> 7

// ============================================================================================================================================================================================
// --- Mapeamento de Bibliotecas ---
#include <Lcd50530.h>
#include <EEPROM.h>

// ============================================================================================================================================================================================
// --- Mapeamento de Hardware ---
// Definição dos Pinos do LCD
#define IOC1 13
#define IOC2 12
#define RW   11
#define ENB  10
#define D4    6
#define D5    7
#define D6    8
#define D7    9

// Definição dos Pinos da Matriz de Botões
// Definições das linhas verticais da Matriz
#define _A 24
#define _B 25
#define _C 26
#define _D 27
#define _E 28
#define _F 29
// Definições das linhas horizontais da Matriz
#define _1 30
#define _2 31
#define _3 32
#define _4 33

// Definição dos Pinos de Sensores
#define FC_T  A15 // Fim de Curso de trás
#define FC_F  A14 // Fim de Curso da frente

// Definição dos Pinos do Encoder
#define PIN_A 20 // Port_D1
#define PIN_B 21 // Port_D0

// Defição dos Pinos dos Relés
#define MOTOR_F A11 // Motor Frente
#define MOTOR_T A10 // Motor Trás
#define FREIO_M A8  // Freio do Motor 

// ============================================================================================================================================================================================
// --- Mapeamento de Objetos ---
// 4bits bus initialization
Lcd50530 lcd(IOC1,IOC2,RW,ENB,D4,D5,D6,D7);

// Padrão dado pela Biblioteca do LCD_338008
byte data0[8] = {
  B01110,
  B11011,
  B10001,
  B10001,
  B11111,
  B11111, 
  B11111,
};

// ============================================================================================================================================================================================
// --- Mapeamento de Constantes Globais ---
const long cont_max = 10000; // 
const float passo = 0.01;

// ============================================================================================================================================================================================
// --- Mapeamento de Variáveis Globais ---
byte estado_freio;
byte leitura[_4 + 1][_F + 1]; // Matriz que é atribuido os valores de leitura do botão
byte estado[4][6]; // Matriz para armazenar os valores da primeira leitura do botão
byte estado1[4][6]; // Matriz para armazenar os valores da segunda leitura do botão
byte tela; // Variável que recebe o valor da tela atual
byte tecla; // Variável que recebe o valor da tecla pressionada
byte teclas[6]; // Vetor que recebe os valor das teclas distribuidos nas colunas para a tela 1
byte colun_ant; // Variável que recebe o valor da coluna anterior da tela 1
byte colun; // Variável que recebe o valor da coluna atual da tela 1
byte col; // Variável que recebe o valor atual da coluna do cursor
byte row; // Variável que recebe o valor atual da linha do cursor
byte aux; // Variável auxiliar
byte aux1;
byte corte;
byte prog;
byte address = 0;
long cont_ant; // Cont anterior
volatile long cont = 0; // Variável que recebe o valor atual da contagem dos pulsos do encoder
unsigned long delay1;
float num1, num2, res; // Variáveis usadas na calculadora
float posicao; // Variável que recebe o valor da posição do carro
float soma = 0;
float data = 0.00f;
float valor_int[3]; // Vetor para armazenar os valores inteiros
float valor_dec[2]; // Vetor para armazenar os valores decimais
float medidas[12][12] = {0};

// ============================================================================================================================================================================================
// --- Configurações Iniciais ---
void setup() 
{
  Serial.begin(9600); // Inicialização do Monitor serial

  // ============================================================================================================================================================================================
  // --- Configurações de Pinos de Entrada---
  // Laço para setar os modos dos pinos verticais como entrada com resistor interno ativado
  for(int x = _A; x <= _F; x++)
  {
    pinMode(x, INPUT_PULLUP);
  }

  pinMode(FC_T, INPUT_PULLUP); 
  pinMode(FC_F, INPUT_PULLUP);
  pinMode(PIN_A, INPUT);
  pinMode(PIN_B, INPUT);

  // ============================================================================================================================================================================================
  // --- Configurações de Pinos de Saída ---
  // Laço para setar os modos dos pinos horizontais como saída e acionando os mesmos com nível lógico alto
  for(int y = _1; y <= _4; y++)
  {
    pinMode(y, OUTPUT);  

    digitalWrite(y, HIGH);
  }

  pinMode(MOTOR_F, OUTPUT);
  pinMode(MOTOR_T, OUTPUT);
  pinMode(FREIO_M, OUTPUT);

  digitalWrite(MOTOR_F, LOW);
  digitalWrite(MOTOR_T, LOW);
  digitalWrite(FREIO_M, LOW);

  // Inicialização do LCD
  lcd.begin(); 
  lcd.createChar(0, data0);
  lcd.home();
  lcd.clear();

  // Inicialização da Matriz
  //byte z = 1; // Variável coluniliar para o próximo laço FOR
  
  // Laço para atribuir os significados dos valores de leituras
  for(byte x = _1, z = 11; x <= _4; x++)
  {
    for(byte y = _A; y <= _F; y++, z++)
    {
      leitura[x][y] = z;
    }
  }

  // Laço para atribuir os significados dos valores de leituras
  for(byte x = _1, z = 7; x <= _3; x++, z-=6)
  {
    for(byte y = _A; y <= _C; y++,z++)
    {
      leitura[x][y] = z;
    } 
  }
  leitura[_4][_A] = 0;
  
  // Definições das Interrupções Externas
  attachInterrupt(digitalPinToInterrupt(PIN_A), cont_A, CHANGE); 
  attachInterrupt(digitalPinToInterrupt(PIN_B), cont_B, CHANGE); 
  
  // // ============================================================================================================================================================================================
  // // --- Inicialização do Sistema ---
  // // Inicializando a Interface
  // for(byte y = 1; y <= 21; y++)
  // {
  //   lcd.setCursor(y, 0); // Coluns, Lines
  //   lcd.print("-");
  // }
  
  // for(byte y = 1; y <= 21; y++)
  // {
  //   lcd.setCursor(y, 7); // Coluns, Lines
  //   lcd.print("-");
  // }   
  
  // lcd.setCursor(1, 1); // Coluns, Lines
  // lcd.print("-MACHINE  AUTOMATION-");

  // lcd.setCursor(3, 6); // Coluns, Lines
  // lcd.print("by: JS ELETORNICS");

  // lcd.setCursor(7, 3); // Coluns, Lines
  // lcd.print("Carregando...");

  // lcd.setCursor(1, 4); // Coluns, Lines
  // lcd.write(15);

  // // -----------------------------------------------------------------------------------------------------------------------
  // //  Verificação de Periféricos de Entrada e Saída (Motor, Encoder e Fim de Curso)
  // delay1 = millis(); 
  // cont = 0;
  
  // // Se o Fim de Curso do Fim da Mesa Estiver Pressionado, ou seja, o Pino FC_T Estiver em Nivel Lógico Baixo
  // if(!digitalRead(FC_T))
  // {
  //   /* O processo basea-se em mover o carro para frente, verificar se foi recebido pulsos do encoder, se não for recebido,
  //     o programa printa uma mensagem de erro, porém se for recebido, o programa verificará se o fim de curso ainda está 
  //     pressionado, se ele ainda é estiver, o programa printa uma mensagem de erro, se não, o programa move o carro para 
  //     trás até que o fim de curso seja pressionado
  //   */

  //   // Motor é ligado para frente
  //   digitalWrite(FREIO_M, HIGH);
  //   digitalWrite(MOTOR_F, HIGH);

  //   // Laço de veriifcação de encoder
  //   while(cont > -50)
  //   {
  //     if((millis() - delay1) >= 2000)
  //     {
  //       // Motor é parado
  //       digitalWrite(FREIO_M, LOW);
  //       digitalWrite(MOTOR_F, LOW);
  //       lcd.clear();

  //       lcd.setCursor(2, 0); // Coluns, Lines
  //       lcd.print("ERRO NA  LEITURA DO ");
  //       lcd.setCursor(7, 1); // Coluns, Lines
  //       lcd.print("ENCODER");
  //       lcd.setCursor(1, 3); // Coluns, Lines
  //       lcd.print("VERIFIQUE  SE O MOTOR");
  //       lcd.setCursor(4, 4); // Coluns, Lines
  //       lcd.print("ESTA  ACOPLADO E");
  //       lcd.setCursor(2, 5); // Coluns, Lines
  //       lcd.print("REINICIE  A MÁQUINA");

  //       delay(999999);
  //     }
  //   }

  //   lcd.write(15);

  //   // Laço "delay"
  //   while((millis() - delay1) < 3000)
  //   {  
  //   }

  //   // Motor é parado
  //   digitalWrite(FREIO_M, LOW);
  //   digitalWrite(MOTOR_F, LOW);

  //   delay1 = millis();
    
  //   // Laços de verificação do fim de curso
  //   if(!digitalRead(FC_T))
  //   {
  //     lcd.clear();

  //     lcd.setCursor(2, 0); // Coluns, Lines
  //     lcd.print("ERRO NA  LEITURA DO ");
  //     lcd.setCursor(5, 1); // Coluns, Lines
  //     lcd.print("FIM DE CURSO");
  //     lcd.setCursor(5, 3); // Coluns, Lines
  //     lcd.print("FIM DE CURSO");
  //     lcd.setCursor(6, 4); // Coluns, Lines
  //     lcd.print("DANIFICADO");
  //     lcd.setCursor(2, 5); // Coluns, Lines
  //     lcd.print("REINICIE  A MÁQUINA");

  //     delay(999999);
  //   }

  //   else
  //   {
  //     lcd.write(15);
  //     delay(100);
  //     lcd.write(15);

  //     // Motor é ligado para trás
  //     digitalWrite(FREIO_M, HIGH);
  //     digitalWrite(MOTOR_T, HIGH); 
      
  //     while(digitalRead(FC_T))
  //     {
  //        if((millis() - delay1) >= 5000)
  //        {
  //           // Motor é parado
  //           digitalWrite(FREIO_M, LOW);
  //           digitalWrite(MOTOR_T, LOW);
            
  //           lcd.clear();
            
  //           lcd.setCursor(2, 0);
  //           lcd.print("ERRO NA  LEITURA DO ");
  //           lcd.setCursor(5, 1); // Coluns, Lines
  //           lcd.print("FIM DE CURSO");
  //           lcd.setCursor(5, 3); // Coluns, Lines
  //           lcd.print("FIM DE CURSO");
  //           lcd.setCursor(6, 4); // Coluns, Lines
  //           lcd.print("DANIFICADO");
  //           lcd.setCursor(2, 5); // Coluns, Lines
  //           lcd.print("REINICIE  A MÁQUINA"); 

  //           delay(999999);
  //        }
  //     }

  //     // Motor é parado
  //     digitalWrite(FREIO_M, LOW);
  //     digitalWrite(MOTOR_T, LOW);
  //   }

  //   lcd.write(15);
  //   lcd.write(15);
  // }

  // else
  // {
  //   /* O processo basea-se em mover o carro para trás, verificar se foi recebido pulsos do encoder, se não for recebido,
  //     o programa printa uma mensagem de erro e para o motor, porém se for recebido, o programa moverá carro para trás até  
  //     que o fim de curso seja pressionado
  //   */ 
    
  //   // Motor é ligado para trás
  //   digitalWrite(FREIO_M, HIGH);
  //   digitalWrite(MOTOR_T, HIGH);

  //   // Laço de verificação de encoder 
  //   while(cont < 50)
  //   {
  //     if((millis() - delay1) >= 2000)
  //     {
  //       // Motor é parado
  //       digitalWrite(FREIO_M, LOW);
  //       digitalWrite(MOTOR_T, LOW);
        
  //       lcd.clear();

  //       lcd.setCursor(2, 0); // Coluns, Lines
  //       lcd.print("ERRO NA  LEITURA DO ");
  //       lcd.setCursor(7, 1); // Coluns, Lines
  //       lcd.print("ENCODER");
  //       lcd.setCursor(1, 3); // Coluns, Lines
  //       lcd.print("VERIFIQUE  SE O MOTOR");
  //       lcd.setCursor(4, 4); // Coluns, Lines
  //       lcd.print("ESTA  ACOPLADO E");
  //       lcd.setCursor(2, 5); // Coluns, Lines
  //       lcd.print("REINICIE  A MÁQUINA");

  //       delay(999999);
  //     }
  //   }

  //   lcd.write(15);
  //   delay(100);
  //   lcd.write(15);
  //   delay(100);
  //   lcd.write(15);

  //   // Laço de verificação do fim de curso
  //   while(digitalRead(FC_T))  
  //   {
  //      if((millis() - delay1) >= 15000)
  //      {
  //         // Motor é parado
  //         digitalWrite(FREIO_M, LOW);
  //         digitalWrite(MOTOR_T, LOW);
           
  //         lcd.clear();
            
  //         lcd.setCursor(2, 0); // Coluns, Lines
  //         lcd.print("ERRO NA  LEITURA DO ");
  //         lcd.setCursor(5, 1); // Coluns, Lines
  //         lcd.print("FIM DE CURSO");
  //         lcd.setCursor(5, 3); // Coluns, Lines
  //         lcd.print("FIM DE CURSO");
  //         lcd.setCursor(6, 4); // Coluns, Lines
  //         lcd.print("DANIFICADO");
  //         lcd.setCursor(2, 5); // Coluns, Lines
  //         lcd.print("REINICIE  A MÁQUINA"); 

  //         delay(999999);
  //      }
  //   }

  //   // Motor é parado
  //   digitalWrite(FREIO_M, LOW);
  //   digitalWrite(MOTOR_T, LOW);

  //   lcd.write(15);
  //   delay(100);
  //   lcd.write(15);
  // } 


  // for(byte x = 8; x <= 21; )
  // {
  //   if(digitalRead(FC_T) == 0)
  //   {
  //     lcd.write(15);
      
  //     delay(300);
      
  //     x++;
  //   }
  // }
  
  // Inicialização da Interface no Modo de Inserção Manual
  lcd.clear(); // Clear screen

  estado_freio = 0;
  cont = cont_max;
  tela = 1;
  colun = 5;

  lcd.setCursor(3, 0);  
    lcd.print(F("---MODO  MANUAL---"));
  lcd.setCursor(1, 2);
  lcd.print(F("Posicao  Atual:"));
  lcd.setCursor(1, 3);
  lcd.print(F("--->"));
  lcd.setCursor(11, 3);
  lcd.print(F("cm"));
  lcd.setCursor(1,4);
  lcd.print(F("Insira a Medida:"));
  lcd.setCursor(11, 5);
  lcd.print(F("cm"));
  lcd.setCursor(1,5);
  lcd.print(F("--->"));
  lcd.setCursor(colun,5);

  colun_ant = colun;
} // End setup

// ============================================================================================================================================================================================
// --- Loop Infinito ---
void loop() 
{ 
  lcd.cursorStyle(0); // Define o estilo do cursor com underline

  // Tela de Modo Manual com Carro Automático
  while(tela == 1)
  {
    lcd.setCursor(colun,5);

    // Se o valor de cont for alterado, ou seja, o encoder movimentar-se, os valores da posição do carro são atualizados 
    // no display    
    if(cont_ant != cont) 
    {
      posicao = cont * passo; 

      lcd.clear_specificTerm(5, 10, 3);
      lcd.setCursor(5, 3);
      lcd.print(posicao);
      
      cont_ant = cont;
    }

    // Se o valor de colun for alterado, ou seja, algo foi digitado pelo operador; os valores da posição do carro são 
    // atualizados no display 
    if(colun_ant != colun)
    {
      colun--;

      if(aux != 0) aux++;

      // Se for pressionado a tecla '.' E a variável aux = 0, ou seja, a tecla '.' não tenha sido pressionada
      if(teclas[colun-5]  == 30 && aux == 0)
      {
        lcd.setCursor(colun,5);
        lcd.print(F("."));

        valor_int[0] = valor_int[1] = valor_int[2] = 0; 

        // É executado esse laço para que seja atribuído os valores referente a parte inteira do número inserido
        for(byte y = 1; colun >= 6; colun--, y = y * 10) valor_int[colun-6] = teclas[colun-6] * y;

        soma = valor_int[0] + valor_int[1] + valor_int[2]; // Definição do valor inteiro total

        colun = colun_ant;
        aux = 1;
        aux1 = colun;
      }

      // Se (teclas[colun-5] == 31, ou seja, o botão limpar foi pressionado) OU (aux = 0 E colun = 8, ou seja, o '.' não 
      // foi pressionado até a coluna 8) OU (teclas[colun-5] = 30 E colun != 8, ou seja, tecla '.' foi pressionada em local 
      // indevido) OU (colun > 10)
      else if ((teclas[colun-5] == 31) || ((aux == 0) && (colun == 8) && (teclas[colun-5] < 10)) /*|| (teclas[colun-5] == 30 && colun != 8) */|| (colun > 10))
      {
        colun = 4;
        aux = 0;

        lcd.clear_specificTerm(5, 10, 5);
        lcd.setCursor(colun,5);
      }

      // Laço de decisão para printar valores digitados
      else
      {
        // Se a tecla pressionada for alguma tecla númerica
        if(teclas[colun-5] < 10)
        {
          lcd.setCursor(colun,5);
          lcd.print(teclas[colun-5]);
        }

        // Caso não seja
        else colun--, aux--;
      }


      // Se aux = 3, ou seja, '.' já foi pressionado E colun = 10, ou seja, todos os números decimais já foram inseridos
      if(aux == 3)
      {
        byte x = 0;

        valor_dec[0] = valor_dec[1] = 0;

        // Laço para tranformar os dados inseridos em valores decimais
        for(float y = 0.01; colun > aux1; colun--, y = y * 10, x++) valor_dec[x] = teclas[colun-5] * y;

        soma = soma + valor_dec[0] + valor_dec[1]; // É atribuido a variável soma, o valor de soma + o valor decimal

        // Se o valor inserido for maior do que 0.00 e menor do que o tamanho da mesa, é convocado o procedimento motor()
        if((soma >= 0.00) && (soma <= (cont_max * passo)))
        {
          motor();
          colun = colun_ant;
        } 

        else 
        {
          colun = 4;

          lcd.clear_specificTerm(5, 10, 5);
          lcd.setCursor(colun,5);
        }        
        aux = 0;
      } 

      colun++;
      colun_ant = colun;     
    }  

    matriz(); // Convoca a função responsável pela leitura da matriz de botões

    telas(); // Convoca o procedimento responsável pela definição da tela a ser mostrada
  }

  // Tela de menu de programas
  while(tela == 2)
  {
    if(tecla == 33) tecla = 0, row++; // Se a tecla pressionada for igual a 33, ou seja, seta para baixo; é acrescido o valor da linha

    if(tecla == 27) tecla = 0, row--; // Se a tecla pressionada for igual a 27, ou seja, seta para cima; é decrescido o valor da linha

    // Laços para movimentação do cursor pelos programas na tela
    if(row > 7 && col == 7)  
    {
      col+=12;
      row-=6;
      
      if(row > 7 && col == 19) row--;

      lcd.setCursor(col, row);
      
      prog = row + col - 15;
      col-=12;
      row+=6;
    }

    else
    {
      if(row < 2 && col == 7) row = 2;

      prog = row + col - 9;

      lcd.setCursor(col, row);
    }

    matriz(); // Convoca a função responsável pela leitura da matriz de botões
    telas(); // Convoca o procedimento responsável pela definição da tela a ser mostrada
  }

  // Tela de Menu de cortes referente à programação escolhida pelo o operador
  while(tela == 3)
  {
    matriz(); // Convoca a função responsável pela leitura da matriz de botões
    telas(); // Convoca o procedimento responsável pela definição da tela a ser mostrada
    
    // Se tecla == 31, ou seja, o botão limpar foi pressionado, o procedimento basea-se em ler os dados inserido e armazena-los
    // na EEPROM
    if(tecla == 31)
    {
      // Se o cursor estiver na segunda coluna, ou seja, o corte selecionado for maior ou igual a 7; é feito o procedimento 
      // de setar o cursor na posição de inserção de valores
      if(row > 7 && col == 10)
      {
        col+=11;
        row-=6;

        lcd.clear_specificTerm(col-7, col-2, row);

        colun = col - 7;
        lcd.setCursor(colun,row);

        col-=11;
        row+=6;
      }

      // Se o cursor estiver na primeira coluna...
      else
      {
        lcd.clear_specificTerm(col-7, col-2, row); 

        colun = col - 7;
        
        lcd.setCursor(colun,row);
      } 

      colun_ant = colun;
      
      byte sub; // Variável local que define o quanto vai ser subtraído de colun, de acordo com a coluna 
      
      // Se o cursor estiver na segunda coluna, ou seja, o corte selecionado for maior ou igual a 7; sub recebe 14
      if(row > 7 && col == 10) sub = 14;
      // Se o cursor estiver na primeira coluna; sub recebe 3
      else sub = 3;

      while(1)
      {
        matriz();

        // Se o valor de colun for alterado, ou seja, algo foi digitado pelo operador; o valor do corte é atualizado 
        if(colun_ant != colun)
        {
          // Se o cursor estiver na segunda coluna, ou seja, sub == 14; é subtraído de row 6, para que o valor da linha seja 
          // possível 
          if(sub == 14) row-=6;

          colun--;

          if(aux != 0) aux++;

          // Se for pressionado a tecla '.' E a variável aux = 0, ou seja, a tecla '.' não tenha sido pressionada; é printado
          // o '.' e atribuído à soma o valor inteiro inserido
          if(teclas[colun-sub]  == 30 && aux == 0)
          {
            lcd.setCursor(colun, row);
            lcd.print(F("."));

            valor_int[0] = valor_int[1] = valor_int[2] = 0; 

            // É executado esse laço para que seja atribuído os valores referente a parte inteira do número inserido
            for(byte y = 1; colun >= sub+1; colun--, y = y * 10) 
            {
              valor_int[colun-(sub+1)] = teclas[colun-(sub+1)] * y;
            }
            soma = valor_int[0] + valor_int[1] + valor_int[2]; // Definição do valor inteiro total

            colun = colun_ant;
            aux = 1;
            aux1 = colun;
          }

          // Se (teclas[colun-sub] == 31, ou seja, o botão limpar foi pressionado) OU [(aux = 0 E colun = sub + 3) E teclas[colun-sub] < 10, , ou seja, o '.' não 
          // foi pressionado até a coluna sub + 3)]  OU (colun > sub + 7); é feito o processo de limpeza da tela
          else if ((teclas[colun-sub] == 31) || ((aux == 0) && (colun == (sub + 3)) && (teclas[colun-sub] < 10)) || (colun > (sub + 7)))
          {
            colun = sub - 1;
            aux = 0;

            lcd.clear_specificTerm(sub, sub+5, row);
            lcd.setCursor(colun,row);
          }

          // Laço de decisão para printar valores digitados
          else
          {          
            // Se a tecla pressionada for alguma tecla númerica
            if(teclas[colun-sub] < 10)
            {
              lcd.setCursor(colun,row);
              lcd.print(teclas[colun-sub]);
            }

            // Caso não seja
            else colun--, aux--;
          }

          // Se aux = 3, ou seja, '.' já foi pressionado E colun = 10, ou seja, todos os números decimais já foram inseridos
          // é atribuído a soma os valores decimais inserido somados com os valores inteiros já atribuídos
          if(aux == 3)
          {
            byte x = 0;

            valor_dec[0] = valor_dec[1] = 0;

            // Laço para tranformar os dados inseridos em valores decimais
            for(float y = 0.01; colun > aux1; colun--, y = y * 10, x++) valor_dec[x] = teclas[colun-sub] * y;

            soma = soma + valor_dec[0] + valor_dec[1]; // É atribuido a variável soma, o valor de soma + o valor decimal
            aux = 0;

            // Se o valor inserido for maior do que 0.00 e menor do que o tamanho da mesa, é convocado o procedimento motor()
            if((soma >= 0.00) && (soma <= (cont_max * passo)))
            {
              colun = colun_ant;             
              address = sizeof(float) * ((12 * prog) + (x - 1)); // Setando endereço através do prog e do corte
              data = soma; 

              EEPROM.put(address, data); // Armazenando o dado no endereço setado

              if(sub == 14) row+=6;
              
              return 0;
            } 

            else 
            {
              colun = sub - 1;

              lcd.clear_specificTerm(sub, sub+5, row);
              lcd.setCursor(colun,row);
            }    
          } 
          colun++;
          colun_ant = colun;     

          if(sub == 14) row+=6;
        }
      }
    }

    if(tecla == 33) tecla = 0, row++; // Se a tecla pressionada for igual a 33, ou seja, seta para baixo; é acrescido o valor da linha

    if(tecla == 27) tecla = 0, row--; // Se a tecla pressionada for igual a 27, ou seja, seta para cima; é decrescido o valor da linha

    // Laços para movimentação do cursor pelos programas na tela
    if(row > 7 && col == 10)  
    {
      col+=11;
      row-=6;
      
      if(row > 7) row--; // Laço para limitar a movimentação do cursor nas linhas, para que ele nunca ultrapasse a linha 7

      lcd.setCursor(col, row);
      
      corte = row + col - 17;
      col-=11;
      row+=6;
    }

    else
    {
      if(row < 2 && col == 10) row = 2;

      corte = row + col - 12;

      lcd.setCursor(col, row);
    }
  }

  // Tela da Calculadora
  while(tela == 4)
  {
    matriz(); // Convoca a função responsável pela leitura da matriz de botões
    telas(); // Convoca o procedimento responsável pela definição da tela a ser mostrada

    lcd.setCursor(colun, row);

    if(tecla == 33 && row < 4) // Se a tecla pressionada for igual a 33, ou seja, seta para baixo; é acrescido o valor da linha
    {
      tecla = 0;
      row++;
      colun = 11;
      colun_ant = colun;

      lcd.clear_specificTerm(11, 16, row);
    }  

    if(tecla == 27 && row > 2) // Se a tecla pressionada for igual a 27, ou seja, seta para cima; é decrescido o valor da linha
    {
      tecla = 0;
      row--;
      colun = 11;
      colun_ant = colun;

      lcd.clear_specificTerm(11, 16, row);
    }  

    if(row < 4)
    {
      // Se o valor de colun for alterado, ou seja, algo foi digitado pelo operador; o valor do corte é atualizado 
      if(colun_ant != colun)
      {
        while(1)
        {
          matriz(); // Convoca a função responsável pela leitura da matriz de botões
          telas(); // Convoca o procedimento responsável pela definição da tela a ser mostrada

          lcd.setCursor(colun, row);

          if(colun_ant != colun)
          {
            byte sub = 11;
            colun--;

            if(aux != 0) aux++;

            // Se for pressionado a tecla '.' E a variável aux = 0, ou seja, a tecla '.' não tenha sido pressionada; é printado
            // o '.' e atribuído à soma o valor inteiro inserido
            if(teclas[colun-sub]  == 30 && aux == 0)
            {
              lcd.setCursor(colun, row);
              lcd.print(F("."));

              valor_int[0] = valor_int[1] = valor_int[2] = 0; 

              // É executado esse laço para que seja atribuído os valores referente a parte inteira do número inserido
              for(byte y = 1; colun >= sub+1; colun--, y = y * 10) 
              {
                valor_int[colun-(sub+1)] = teclas[colun-(sub+1)] * y;
              }
              soma = valor_int[0] + valor_int[1] + valor_int[2]; // Definição do valor inteiro total

              colun = colun_ant;
              aux = 1;
              aux1 = colun;
            }

            // Se (teclas[colun-sub] == 31, ou seja, o botão limpar foi pressionado) OU [(aux = 0 E colun = sub + 3) E teclas[colun-sub] < 10, , ou seja, o '.' não 
            // foi pressionado até a coluna sub + 3)]  OU (colun > sub + 7); é feito o processo de limpeza da tela
            else if ((teclas[colun-sub] == 31) || ((aux == 0) && (colun == (sub + 3)) && (teclas[colun-sub] < 10)) || (colun > (sub + 5)))
            {
              colun = sub - 1;
              aux = 0;

              lcd.clear_specificTerm(sub, sub+5, row);
              lcd.setCursor(colun,row);
            }

            // Laço de decisão para printar valores digitados
            else
            {          
              // Se a tecla pressionada for alguma tecla númerica
              if(teclas[colun-sub] < 10)
              {
                lcd.setCursor(colun,row);
                lcd.print(teclas[colun-sub]);
              }

              // Caso não seja
              else colun--, aux--;
            }

            // Se aux = 3, ou seja, '.' já foi pressionado E colun = 10, ou seja, todos os números decimais já foram inseridos
            // é atribuído a soma os valores decimais inserido somados com os valores inteiros já atribuídos
            if(aux == 3)
            {
              byte x = 0;

              valor_dec[0] = valor_dec[1] = 0;
              // Laço para tranformar os dados inseridos em valores decimais
              for(float y = 0.01; colun > aux1; colun--, y = y * 10, x++) valor_dec[x] = teclas[colun-sub] * y;

              soma = soma + valor_dec[0] + valor_dec[1]; // É atribuido a variável soma, o valor de soma + o valor decimal
              
              if(row == 2) num1 = soma;
              if(row == 3) num2 = soma;

              aux = 0;
              colun = colun_ant;   

              return 0;
            } 
            colun++;
            colun_ant = colun;
          }
        }     
      }  
    }

    else
    {
      if(tecla == 14)
      {
        tecla = 0; 
        res = num1 / num2;
        colun = 12;
        row = 6;
        
        lcd.print(F(" : "));
        lcd.setCursor(colun, row);
        lcd.print(res, 2);

        colun = 11;
        row = 2;

        lcd.setCursor(colun, row);
      }

      if(tecla == 20)
      {
        tecla = 0; 
        res = num1 * num2;
        colun = 12;
        row = 6;
        
        lcd.print(F(" * "));
        lcd.setCursor(colun, row);
        lcd.print(res, 2);

        colun = 11;
        row = 2;

        lcd.setCursor(colun, row);
      }

      if(tecla == 26)
      {
        tecla = 0; 
        res = num1 + num2;
        colun = 12;
        row = 6;
        
        lcd.print(F(" + "));
        lcd.setCursor(colun, row);
        lcd.print(res, 2);

        colun = 11;
        row = 2;

        lcd.setCursor(colun, row);
      }

      if(tecla == 32)
      {
        tecla = 0; 
        res = num1 - num2;
        colun = 12;
        row = 6;
        
        lcd.print(F(" - "));
        lcd.setCursor(colun, row);
        lcd.print(res, 2);

        colun = 11;
        row = 2;

        lcd.setCursor(colun, row);
      }
    }
  }

  // Tela de modo manual, com controle manual
  while(tela == 5)
  {
    matriz(); // Convoca a função responsável pela leitura da matriz de botões
    telas(); // Convoca o procedimento responsável pela definição da tela a ser mostrada

    if(cont_ant != cont) 
    {
      posicao = cont * passo; 

      lcd.clear_specificTerm(5, 10, 3);
      lcd.setCursor(5, 3);
      lcd.print(posicao);
      
      cont_ant = cont;
    }
  }
} // End loop

// Leitura de Encoder, tendo como referência a mudança de estado do PIN_A
void cont_A()
{
  if((PIND & (1 << 1)) && (!(PIND & 1))) cont++; // Se PIN_A = 1, e PIN_B = 0

  if((!(PIND & (1 << 1))) && (PIND & 1)) cont++; // Se PIN_A = 0, e PIN_B = 1

  if((PIND & (1 << 1)) && (PIND & 1)) cont--; // Se PIN_A = 1, e PIN_B = 1

  if((!(PIND & (1 << 1))) && (!(PIND & 1))) cont--; // Se PIN_A = 0, e PIN_B = 0
} // End cont_A

// Leitura de Encoder, tendo como referência a mudança de estado do PIN_B
void cont_B()
{
  if((PIND & (1 << 1)) && (PIND & 1)) cont++; // Se PIN_A = 1, e PIN_B = 1
  
  if((!(PIND & (1 << 1))) && (!(PIND & 1))) cont++;// Se PIN_A = 0, e PIN_B = 0

  if((PIND & (1 << 1)) && (!(PIND & 1))) cont--; // Se PIN_A = 1, e PIN_B = 0
  
  if((!(PIND & (1 << 1))) && (PIND & 1)) cont--; // Se PIN_A = 0, e PIN_B = 1
} // End cont_B

// Função responsável pela leitura da matriz de botões
void matriz()
{
  byte q = tela; // Variável para corrigir problema. Após a variável ser invocada algumas vezes, tela recebia o valor de tecla

  // Laço para acionar gradualmente as linhas horizontais 
  for(byte x = _1, w = 0; x <= _4; x++, w++)
  {
    digitalWrite(x, LOW); // Linha acionada

    // Laço para verificar nas linhas verticais, os botões da linha acionada
    for(byte y = _A, z = 0; y <= _F; y++, z++)
    {
      estado[w][z] = digitalRead(y); // É atribuido o valor da leitura à matriz estado[][]

      if(estado[w][z] == 0) // Se o valor atribuído for igual à 0...
      {
        delay1 = millis();

        while((millis() - delay1) < 2) // Timer para efeito boucing
        {
        }
        
        estado[w][z] = digitalRead(y); // // A porta é novamente verificada

        if(estado[w][z] == 0 && estado1[w][z] == 1) // Se essa verificação for verdadeira...
        { 
          tecla = leitura[x][y]; // É atribuido a teclas os valores da leitura da Matriz
          
          if(tecla == 16)
          {
            tecla = 0;

            if(estado_freio == 0) estado_freio = 1, digitalWrite(FREIO_M, HIGH);
            else estado_freio = 0, digitalWrite(FREIO_M, LOW);
          }

          // Se tela for igual à 1, ou seja, tela de modo manual
          if(tela == 1) teclas[colun-5] = leitura[x][y], colun++;  // É atribuido a teclas[] os valores da leitura da Matriz, 
                                                                   // o local onde esse valor vai ser atribuido depende exclusivamente  
                                                                   // da coluna na qual o cursor está localizado subtraido 5
          // Se tela for igual à 3, ou seja, tela de menu de cortes
          if(tela == 3) 
          { 
            // Se o cursor estiver na segunda coluna, ou seja, o corte selecionado for maior ou igual a 7; o valor subtraído é 14   
            if(row > 7 && col == 10)
            { 
              teclas[colun-14] = leitura[x][y];
            }

            else
            {
              teclas[colun-3] = leitura[x][y];
            }

            colun++;
          }

          if((tela == 4 && row < 4) && (tecla < 10 || tecla == 31 || tecla == 30) ) teclas[colun-11] = leitura[x][y], colun++; 
        } 
      }  
      estado1[w][z] = estado[w][z]; 
    }
    digitalWrite(x, HIGH); // Linha desacionada
  }
  tela = q;
} // End Matriz

// Procedimento responsável pela movimentação do carro até o set-point
void motor()
{
  float diferenca = 0;
  // float set_Point;

  posicao = cont * passo;
  diferenca = soma - posicao;

  //set_Point = posicao + (diferenca);

  //soma = 99.7f;

  while(diferenca != 0)
  {
    posicao = cont * passo;
    diferenca = soma - posicao;
  
    if(diferenca < 0)
    {
      digitalWrite(MOTOR_T, LOW);
      digitalWrite(FREIO_M, HIGH);
      digitalWrite(MOTOR_F, HIGH);
    }

    if (diferenca > 0)
    {
      digitalWrite(MOTOR_F, LOW);
      digitalWrite(FREIO_M, HIGH);
      digitalWrite(MOTOR_T, HIGH);  
    }

    if(diferenca == 0)
    {
      digitalWrite(FREIO_M, LOW);
      digitalWrite(MOTOR_F, LOW);
      digitalWrite(MOTOR_T, LOW);

      delay(500);

      posicao = cont * 0.01;

      diferenca = soma - posicao;
    }

    lcd.clear_specificTerm(5, 10, 3);
    lcd.setCursor(5, 3);
    lcd.print(posicao);
  }
} // End Motor

// Função responsável pela mudança das telas
void telas()
{
  // Se a tecla 21 for pressionada, ou seja, a tecla do modo de inserção manual for solicitado...
  if(tecla == 21)
  {
    tela = 1; 
    tecla = 0; // Para que não vire um laço infinito
    colun = 5;
    posicao = cont * passo;

    lcd.clear();
    lcd.setCursor(3, 0);  
    lcd.print(F("---MODO  MANUAL---"));
    lcd.setCursor(1, 2);
    lcd.print(F("Posicao  Atual:"));
    lcd.setCursor(1, 3);
    lcd.print(F("--->"));
    lcd.print(posicao);
    lcd.setCursor(11, 3);
    lcd.print(F("cm"));
    lcd.setCursor(1,4);
    lcd.print(F("Insira a Medida:"));
    lcd.setCursor(11, 5);
    lcd.print(F("cm"));
    lcd.setCursor(1,5);
    lcd.print(F("--->"));

    colun_ant = colun;
  }

  // Se a tecla 34 for pressionada, ou seja, a tecla para consultar os programas
  else if(tecla == 34)
  {
    // Se a tela for igual a 2, ou seja, a tela já esteja mostrando os programas; acontece que, se essa tecla continuar
    // sendo pressionada por mais de 3 segundos; o programa passa para a tela 3, que vai basear-se no programa que estava 
    // selecionado, mostrando assim, os cortes do tal programa
    if(tela == 2)
    {
      delay1 = millis();

      digitalWrite(_4, LOW);

      while(digitalRead(_F) == 0 && (millis() - delay1) < 2000)
      {
      }

      // Se a tecla for pressionada por mais de 2 segundos
      if(digitalRead(_F) == 0 && (millis() - delay1) >= 2000)
      {
        tela = 3;
        col = 10;
        row = 2;

        lcd.clear();
        lcd.setCursor(1,0);
        lcd.print(F("- CORTES - PROG - "));
        lcd.print(prog+1);
        lcd.print(F(" -"));

        for(byte y = 1, x = 1, z; y <= 15; y+=11)
        {
          for(z = 2; z <= 7 && x < 10; x++, z++)
          {
            lcd.setCursor(y+8,z);
            lcd.print(F("cm"));
            lcd.setCursor(y, z);
            lcd.print(x);
            lcd.print(F("-"));
            
            address = sizeof(float) * ((12 * prog) + (x - 1)); // Setando endereço através do prog,do corte e do espaço que um variável float ocupa

            EEPROM.get(address, data); // Pegando o dado armazenado no endereço

            if(data > 0.009) lcd.print(data);
          }

          if(x >= 10)
          {
            for(z = 5; z <= 7; x++, z++)
            {
              lcd.setCursor(y+8,z);
              lcd.print(F("cm"));
              lcd.setCursor(y-1, z);
              lcd.print(x);
              lcd.print(F("-"));
              
              address = sizeof(float) * ((12 * prog) + (x - 1)); // Setando endereço através do prog e do corte

              EEPROM.get(address, data); // Pegando o dado armazenado no endereço

              if(data > 0.009) lcd.print(data, 2);
            }
          }
        }
      }
    }

    // Se a tela for igual a 3, ou seja, a screen atual é a dos cortes de algum programa
    else if(tela == 3)
    {
    }

    // Se nenhuma das afirmações dos laços anteriores for verdadeira, ou seja, tela é diferente de 2 e 3; a tela atual passará
    // a ser a tela 2, menu de programas
    else 
    {
      tela = 2;
      tecla = 0;
      col = 7;
      row = 2;

      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print(F("- MENU DE PROGRAMAS -"));

      for(byte y = 2, x = 1, z; y <= 15; y+=12)
      {
        for(z = 2; z <= 7 && x < 10; x++, z++)
        {
          lcd.setCursor(y, z);
          lcd.print(x);
          lcd.print(F(" - ( )"));
        }

        if(x >= 10)
        {
          for(z = 5; z <= 7; x++, z++)
          {
            lcd.setCursor(y-1, z);
            lcd.print(x);
            lcd.print(F(" - ( )"));
          }
        }
      }
    }
  }

  else if(tecla == 22)
  {
    tecla = 0;
    tela = 4;
    colun = 11;
    row = 2;

    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print(F("--- CALCULADORA ---"));
    lcd.setCursor(1, 2);
    lcd.print(F("Numero 1: "));
    lcd.setCursor(1, 3);
    lcd.print(F("Numero 2: "));
    lcd.setCursor(1, 4);
    lcd.print(F("Operacao: "));
    lcd.setCursor(1, 6);
    lcd.print(F("Resultado: "));
    lcd.setCursor(colun, row);

    colun_ant = colun;
  }

  else if(tecla == 15)
  {
    tecla = 0;
    tela = 5;

    lcd.clear();
    lcd.setCursor(3, 0);  
    lcd.print(F("---MODO  MANUAL---"));
    lcd.setCursor(1, 2);
    lcd.print(F("Posicao  Atual:"));
    lcd.setCursor(1, 3);
    lcd.print(F("--->"));
    lcd.print(posicao);
  }
} // End telas