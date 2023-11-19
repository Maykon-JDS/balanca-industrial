#include <LiquidCrystal_I2C.h>

#include <HX711.h>

#include <math.h>

#include <Keypad.h>

#include <WString.h>


const byte LINHAS = 4;

const byte COLUNAS = 4;

char teclas[LINHAS][COLUNAS] = {

  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}

};

byte pinosLinhas[LINHAS] = {52, 50, 48, 46}; // Pinos conectados às linhas do teclado

byte pinosColunas[COLUNAS] = {44, 42, 40, 38}; // Pinos conectados às colunas do teclado

Keypad * teclado = new Keypad(makeKeymap(teclas), pinosLinhas, pinosColunas, LINHAS, COLUNAS);


#define pinDT  6

#define pinSCK  7

HX711 scale;

LiquidCrystal_I2C lcd(0x27, 16, 2);


class Balanca {

  private:

    int delayTime       = 1000;

    int pesoReal        = 0;

    float pesoSemOffset = 0;

    float fatorEscala   = 0;

    bool estadoMenu = true;

    int menu = 0;


    byte checkChar[8] = {

      B00000,
      B00000,
      B00001,
      B00010,
      B10100,
      B01000,
      B00000,
      B00000

    };


    LiquidCrystal_I2C * lcd;

    Keypad * teclado;

    HX711 * scale;


  public:

    Balanca(LiquidCrystal_I2C lcd, Keypad * teclado, HX711 scale) {

      this->lcd = &lcd;

      this->teclado = teclado;

      this->scale = &scale;

      this->configurarLCD();

    }


  private:

    configurarLCD() {

      this->lcd->init();

      this->lcd->createChar(0, this->checkChar);

    }


  public:

    iniciar() {

      this->lcd->backlight();

      this->lcd->setCursor(0, 0);

    }


    setTara() {

      this->lcd->clear();

      this->scale->set_scale(); // LIMPANDO O VALOR DA ESCALA

      this->lcd->setCursor(0, 0);

      this->lcd->print("Config. Tara...");

      this->scale->tare(100); // ZERANDO A BALANÇA PARA DESCONSIDERAR A MASSA DA ESTRUTURA

      this->lcd->setCursor(0, 0);

      this->lcd->print("Tara Config. "); this->lcd->write(0); this->lcd->print("  ");

      this->delayClearLCD(this->delayTime);

    }


    setFatorEscala() {

      this->lcd->clear();

      this->lcd->print("Regular Balanca");

      char caracter = 'A';

      this->lcd->setCursor(0, 1);

      this->lcd->print("OK - A");

      this->confirmarViaTeclado(caracter);

      this->lcd->setCursor(0, 0);

      this->lcd->print("Peso em Gramas!");

      this->confirmarViaTeclado(caracter);

      this->setPesoRealFatorEscala();

      this->delayClearLCD(this->delayTime);

    }


    confirmarViaTeclado(char tecla) {

      char teclaPressionada = this->teclado->waitForKey();

      while (teclaPressionada != tecla) {

        teclaPressionada = this->teclado->waitForKey();

      }

    }


    setPesoRealFatorEscala() {

      LiquidCrystal_I2C lcd(0x27, 16, 2);

      lcd.init();

      lcd.backlight();

      lcd.clear();

      lcd.setCursor(0, 0);

      lcd.print("Max. 25000 g");

      lcd.setCursor(0, 1);

      lcd.print("Peso: ");

      int posicaoCursor = 6;

      char tecla = "";

      long pesoReal = 0;

      String pesoRealStr = "";

      lcd.setCursor(0, 0);

      while (tecla != 'A' ||  pesoReal > 25000) {

        tecla = this->teclado->waitForKey();

        if (isDigit(tecla) && posicaoCursor < 14) {

          Serial.println(pesoRealStr);

          pesoRealStr += tecla;

          pesoReal = pesoRealStr.toInt();

          Serial.println(pesoReal);

          Serial.println(pesoReal);

          lcd.setCursor(posicaoCursor, 1);

          lcd.print(tecla);

          lcd.printstr(" g");

          posicaoCursor++;

        }
        else if (tecla == 'D' && posicaoCursor > 6) {

          pesoRealStr.remove(pesoRealStr.length() - 1, 1);

          posicaoCursor--;

          lcd.setCursor(posicaoCursor, 1);

          lcd.print(" g ");

        }

      }

      this->pesoReal = pesoRealStr.toInt();

      this->delayClearLCD(this->delayTime);

    }


    printRawLCD(int row = 0, int col = 0) {

      this->lcd->setCursor(col, row);

      this->lcd->print(this->scale->read_average(3));

    }


    printUnitsLCD(int row = 0, int col = 0) {

      this->lcd->setCursor(col, row);

      this->lcd->print(this->scale->get_units(3));

    }

    delayClearLCD(int tempoDelay) {

      delay(tempoDelay);

      this->lcd->clear();

    }


    void mostrarMenu()
    {

      LiquidCrystal_I2C lcd(0x27, 16, 2);

      lcd.init();

      bool loop1 = estadoMenu;

      while (loop1) {

        this->mostrarOpcoesMenu();

        char acaoEscolhida = this->teclado->waitForKey();

        Serial.println(acaoEscolhida);

        loop1 = executarAcaoEscolhidaMenu(acaoEscolhida);

      }
    }

  
    bool executarAcaoEscolhidaMenu(char acaoEscolhida) {

      char voltar = '*';

      char avancar = 'D';

      char confirmar = 'A';


      if (acaoEscolhida == confirmar) {

        this->lcd->clear();

        switch (this->menu)
        {
          case 0:

            this->lcd->setCursor(0, 0);

            this->lcd->print("  Pesar         ");

            this->lcd->setCursor(0, 1);

            this->lcd->print("Confirmado     D");

            break;

          case 1:

            this->mostrarSubmenu();

            break;

          case 2:

            this->lcd->setCursor(0, 0);

            this->lcd->print("  Configurar    ");

            this->lcd->setCursor(0, 1);

            this->lcd->print("Confirmado     D");

            break;
        }

        return false;

      }
      else if (acaoEscolhida == voltar && this->menu >= 0) {

        this->menu--;

      }
      else if (acaoEscolhida == avancar && this->menu <= 2) {

        this->menu++;

      }

      return true;

    }

    void mostrarOpcoesMenu() {

      switch (this->menu)
      {
        case 0:

          this->lcd->setCursor(0, 0);

          this->lcd->print("  Pesar        >");

          this->lcd->setCursor(0, 1);

          this->lcd->print("               D");

          break;

        case 1:

          this->lcd->setCursor(0, 0);

          this->lcd->print("< Contar Itens >");

          this->lcd->setCursor(0, 1);

          this->lcd->print("*              D");

          break;

        case 2:

          this->lcd->setCursor(0, 0);

          this->lcd->print("< Configurar    ");

          this->lcd->setCursor(0, 1);

          this->lcd->print("*               ");

          break;

      }

    }



    void mostrarSubmenu() {

      LiquidCrystal_I2C lcd(0x27, 16, 2);

      lcd.init();

      this->menu = 0;

      bool loop2 = estadoMenu;

      while (loop2) {

        this->mostrarOpcoesSubMenu1();

        char acaoEscolhida = this->teclado->waitForKey();

        Serial.println(acaoEscolhida);

        loop2 = this->executarAcaoEscolhidaSubmenu(acaoEscolhida);

      }

    }
    

    void mostrarOpcoesSubMenu1() {

      switch (this->menu)
      {
        
        case 0:
          
          this->lcd->setCursor(0, 0);
          
          this->lcd->print("  Contar       >");
          
          this->lcd->setCursor(0, 1);
          
          this->lcd->print("               D");
          
          break;
        
        case 1:
          
          this->lcd->setCursor(0, 0);
          
          this->lcd->print("< Ver Config.  >");
          
          this->lcd->setCursor(0, 1);

          this->lcd->print("*              D");
          
          break;
        
        case 2:
          
          this->lcd->setCursor(0, 0);
          
          this->lcd->print("< Config. Item >");
          
          this->lcd->setCursor(0, 1);
          
          this->lcd->print("*              D");
          
          break;
        
        case 3:
          
          this->lcd->setCursor(0, 0);
          
          this->lcd->print("< Config. Caixa >");
          
          this->lcd->setCursor(0, 1);
          
          this->lcd->print("*               ");
          
          break;
      }
      
    }

    bool executarAcaoEscolhidaSubmenu(char acaoEscolhida){
      
        char voltar = '*';
        
        char avancar = 'D';
        
        char confirmar = 'A';

        if (acaoEscolhida == confirmar) {
          
          this->lcd->clear();

          switch (this->menu)
          {
            
            case 0:
              
              this->lcd->setCursor(0, 0);
              
              this->lcd->print("  Contar        ");
              
              this->lcd->setCursor(0, 1);
              
              this->lcd->print("Confirmado     D");
              
              break;
            
            case 1:
              
              this->lcd->setCursor(0, 0);
              
              this->lcd->print("  Ver Config.  ");
              
              this->lcd->setCursor(0, 1);
              
              this->lcd->print("Confirmado     D");
              
              break;
            
            case 2:
              
              this->lcd->setCursor(0, 0);
              
              this->lcd->print("  Config. Item  ");
              
              this->lcd->setCursor(0, 1);
              
              this->lcd->print("Confirmado     D");
              
              break;
            
            case 3:
              
              this->lcd->setCursor(0, 0);
              
              this->lcd->print("  Config. Caixa ");
              
              this->lcd->setCursor(0, 1);
              
              this->lcd->print("Confirmado     D");
              
              break;
              
          }

          return false;

        }
        else if (acaoEscolhida == voltar && this->menu >= 0) {

          this->menu--;

        }
        else if (acaoEscolhida == avancar && this->menu <= 3) {

          this->menu++;

        }

        return true;
    
    }
   
};

Balanca * balanca = NULL;

void setup() {

  Serial.begin(115200);
  scale.begin(pinDT, pinSCK); // CONFIGURANDO OS PINOS DA BALANÇA

  balanca = new Balanca(lcd, teclado, scale);

  //  char texto[] = "Coloque o peso e envie o valor deste, em gramas!              ";
  //  char texto[] = "Coloque o peso e envie o valor deste, em gramas, pelo serial! ";

  //  delay(1000);
  //
  //  Serial.begin(115200);
  //
  //  scale.begin(pinDT, pinSCK); // CONFIGURANDO OS PINOS DA BALANÇA
  //  //  scale.set_scale(50.24108); // LIMPANDO O VALOR DA ESCALA
  //  scale.set_scale(); // LIMPANDO O VALOR DA ESCALA
  //  delay(2000);
  //  lcd.setCursor(0, 0);
  //  // Serial.println("Zerando a Balança");
  //  lcd.print("Zerando...");
  //  lcd.setCursor(0, 0);
  //  scale.tare(100); // ZERANDO A BALANÇA PARA DESCONSIDERAR A MASSA DA ESTRUTURA
  //  lcd.setCursor(0, 0);
  //  lcd.print("Zerado "); lcd.write(0); lcd.print("   ");
  //  // Serial.println("Balança Zerada"
  //  delay(1000);
  //  lcd.clear();

  //Serial.println("Coloque o peso e envie o valor deste, em gramas");

  //  bool loopW = true;
  //  lcd.clear();
  //  int j = 0;
  //
  //
  //
  //  while (loopW) {
  //    int x = 0;
  //    lcd.setCursor(0, 0);
  //
  //
  //    for (int i = 0; i < 16; i++) {
  //
  //      if (j < 2) {
  //        lcd.print(texto[i]);
  //      }
  //      else if (j > 1 && j <= 45) {
  //
  //        lcd.print(texto[i + j]);
  //
  //      }
  //      else if (j > 45) {
  //
  //        if ( i > 15 - (j - 46) ) {
  //
  //          lcd.print(texto[x]);
  //          x++;
  //        }
  //        else {
  //          lcd.print(texto[i + j]);
  //        }
  //      }
  //
  //      if (j == 61) {
  //        j = 0;
  //      }
  //    }
  //
  //    j++;
  //
  //    char tecla = teclado.getKey();
  //    if (tecla == 'A') {
  //      loopW = false;
  //    }
  //    delay(100);
  //    lcd.clear();
  //  }
  //  lcd.setCursor(0, 0);
  //  lcd.print("Max. 25000 g");
  //
  //  lcd.setCursor(0, 1);
  //  lcd.print("Peso: ");
  //
  //  int posicao = 6;
  //  String texto2 = "";
  //
  //  char tecla = ' ';
  //
  //  while (tecla != 'A') {
  //    tecla = teclado.getKey();
  //
  //    if (isDigit(tecla)) {
  //
  //
  //      lcd.setCursor(posicao, 1);
  //      texto2.concat(tecla);
  //      lcd.print(tecla);
  //      Serial.println(texto2);
  //      posicao++;
  //    }
  //    else if (tecla == 'D' && posicao > 6) {
  //      texto2.remove(texto2.length() - 1, 1);
  //      posicao--;
  //      lcd.setCursor(posicao, 1);
  //      lcd.print(" ");
  //      Serial.println(texto2);
  //    }
  //
  //  }


  //  lcd.setCursor(0, 0);
  //  pesoReal = texto2.toInt();
  //
  //
  //  if (pesoReal > 0) {

  //    lcd.print(pesoReal);
  //    pesoSemOffset = scale.get_units(100);
  //    fator = pesoSemOffset / pesoReal;
  //    scale.set_scale(fator);
  //
  //
  //  }
  //  else {
  //    Serial.flush();
  //  }
  //
  //  lcd.clear();

  balanca->iniciar();
  balanca->mostrarMenu();

}

void loop() {

  //  Serial.println(teclado.waitForKey());

  //  balanca->printRawLCD(0,0);
  //  balanca->printUnitsLCD(0,1);

  //  lcd.print(scale.get_units(3));
  //  delay(250);
  //  lcd.clear();
  //medida = scale.get_units(3); // SALVANDO NA VARIAVEL O VALOR DA MÉDIA DE 5 MEDIDAS
  // Serial.println(medida); // ENVIANDO PARA MONITOR SERIAL A MEDIDA COM 3 CASAS DECIMAIS
  // Serial.println(floor(medida), 2);
  // Serial.print("RAW:\t"); Serial.println(scale.get_offset());
  // Serial.print("RAW:\t"); Serial.println(scale.read());
  // Serial.print("UNITS:\t\t"); Serial.println(scale.get_units(3));
  //  Serial.print(scale.get_units(3)); Serial.println(" g");
  // Serial.print("PARSE INT:\t"); Serial.println(Serial.parseInt());
  // Serial.print("READ STRING:\t"); Serial.println(Serial.readString());
  // delay(2000);
  //  scale.power_down(); // DESLIGANDO O SENSOR
  //  delay(5); // AGUARDA 5 SEGUNDOS
  //  scale.power_up(); // LIGANDO O SENSOR
}
