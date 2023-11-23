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

    bool estadoMenu = true;

    bool estadoSubMenu = false;

    bool configuracaoGeral = false;





    float offset = 0;

    float pesoSemOffset = 0;

    int pesoReal        = 0;

    float fatorEscala   = 0;


    //Manter uma das duas variaveis

    float offsetCaixa     = 0;

    int pesoCaixa     = 0;

    int pesoItem      = 0;

    int qtdItem       = 0;

    bool configuracaoOffsetCaixa = false;



    int menu = 0;

    int submenu = 0;

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


    pesar() {

      HX711 scale;

      scale.begin(pinDT, pinSCK);

      float peso = 0;

      bool loopPesar = true;

      while (loopPesar) {

        for (int i = 0; i < 10; i++) {

          if (this->teclado->getKey() == 'B') {

            loopPesar = false;

            break;

          }

          delay(15);

        }

        peso = (scale.read_median(7) - this->offset) / this->fatorEscala;

        this->lcd->clear();

        this->lcd->setCursor(0, 0);

        if (peso >= 0) {

          this->lcd->print(peso);

        }
        else {

          this->lcd->print(0);

        }

        this->lcd->print(" g");

        this->lcd->setCursor(0, 1);

        this->lcd->print("               ");

      }

    }

    configurarFator() {

      HX711 scale;

      scale.begin(pinDT, pinSCK);

      float pesoRaw = scale.read_average(100);

      this->pesoSemOffset = pesoRaw - this->offset;

      this->fatorEscala = this->pesoSemOffset / this->pesoReal;

      Serial.print("fatorEscala: "); Serial.println(this->fatorEscala);

    }

    iniciar() {

      this->lcd->backlight();

      this->lcd->setCursor(0, 0);

      this->mostrarMenu();

    }

    zerarBalanca() {

      this->lcd->clear();

      char caracter = 'A';

      this->lcd->print("Retire o Peso ");

      this->lcd->setCursor(0, 1);

      this->lcd->print("OK - A        ");

      this->confirmarViaTeclado(caracter);

      this->lcd->setCursor(0, 0);

      this->lcd->print("Peso Retirado?");

      this->lcd->setCursor(0, 1);

      this->lcd->print("Sim - A       ");

      this->confirmarViaTeclado(caracter);

      this->lcd->setCursor(0, 0);

      this->lcd->print("Zerar Balanca  ");

      this->lcd->setCursor(0, 1);

      this->lcd->print("OK - A         ");

      this->confirmarViaTeclado(caracter);

      this->lcd->setCursor(0, 0);

      this->lcd->print("Zerando...     ");

      this->lcd->setCursor(0, 1);

      this->lcd->print("               ");

      this->offset = this->scale->read_average(100); 

      Serial.print("offset: "); Serial.println(this->offset);

      this->lcd->setCursor(0, 0);

      this->lcd->print("Zerada "); this->lcd->write(0); this->lcd->print("  ");

      this->delayClearLCD(this->delayTime);

    }

    configurarBalanca() {

      this->zerarBalanca();

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

      this->configuracaoGeral = true;

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

          this->pesoReal = pesoRealStr.toInt();

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

      this->configurarFator();

      this->delayClearLCD(this->delayTime);

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

        if (acaoEscolhida == 'B') {

          this->menu = 0;

          break;

        }

        loop1 = executarAcaoEscolhidaMenu(acaoEscolhida);

      }
    }


    bool executarAcaoEscolhidaMenu(char acaoEscolhida) {

      char voltar = '*'; //Tornar essa variável uma variável de classe

      char avancar = 'D'; //Tornar essa variável uma variável de classe

      char confirmar = 'A'; //Tornar essa variável uma variável de classe


      if (acaoEscolhida == confirmar) {

        this->lcd->clear();

        switch (this->menu)
        {
          case 0:

            if (!this->configuracaoGeral) {

              this->lcd->print("Config. Balanca");

              this->lcd->setCursor(0, 1);

              this->lcd->print("OK - A         ");

              char caracter = 'A';

              this->confirmarViaTeclado(caracter);

              this->configurarBalanca();
            }

            this->pesar();



            break;

          case 1:

            this->estadoSubMenu = true;

            this->mostrarSubmenu();

            break;

          case 2:

            this->configurarBalanca();

            break;
        }

        return false;

      }
      else if (acaoEscolhida == voltar && this->menu > 0) {

        this->menu--;

      }
      else if (acaoEscolhida == avancar && this->menu < 2) {

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

      bool loop2 = estadoSubMenu;

      while (estadoMenu && loop2) {

        this->mostrarOpcoesSubMenu();

        char acaoEscolhida = this->teclado->waitForKey();

        if (acaoEscolhida == 'B') {

          break;

        }

        Serial.println(acaoEscolhida);

        loop2 = this->executarAcaoEscolhidaSubmenu(acaoEscolhida);

      }

    }


    void mostrarOpcoesSubMenu() {

      switch (this->submenu)
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

          this->lcd->print("< Config. QTD  >");

          this->lcd->setCursor(0, 1);

          this->lcd->print("*              D");

          break;

        case 3:

          this->lcd->setCursor(0, 0);

          this->lcd->print("< Config. Item >");

          this->lcd->setCursor(0, 1);

          this->lcd->print("*              D");

          break;

        case 4:

          this->lcd->setCursor(0, 0);

          this->lcd->print("< Config. Caixa >");

          this->lcd->setCursor(0, 1);

          this->lcd->print("*               ");

          break;
      }

    }

    bool executarAcaoEscolhidaSubmenu(char acaoEscolhida) {

      char voltar = '*';

      char avancar = 'D';

      char confirmar = 'A';

      if (acaoEscolhida == confirmar) {

        this->lcd->clear();

        switch (this->submenu)
        {

          case 0:

            this->contarItens();

            break;

          case 1:

            this->mostrarConfiguracaoContagem();

            break;

          case 2:
            this->configurarQtdItem();

            break;

          case 3:

            this->configurarItem();


            break;

          case 4:

            this->configurarCaixa();

            break;

        }

        return false;

      }
      else if (acaoEscolhida == voltar && this->submenu > 0) {

        this->submenu--;

      }
      else if (acaoEscolhida == avancar && this->submenu < 4) {

        this->submenu++;

      }

      return true;

    }

    contarItens(){
      
      HX711 scale;

      scale.begin(pinDT, pinSCK);

      float peso = 0;

      float itens = 0;

      bool loopContar = true;

      while (loopContar) {

        for (int i = 0; i < 10; i++) {

          if (this->teclado->getKey() == 'B') {

            loopContar = false;

            break;

          }

          delay(15);

        }

        peso = (scale.read_median(7) - this->offset) / this->fatorEscala;

        itens = (peso - this->pesoCaixa) / this->pesoItem;

        this->lcd->clear();

        this->lcd->setCursor(0, 0);

        this->lcd->print("QTD: ");

        this->lcd->print(this->qtdItem);

        this->lcd->setCursor(0, 1);

        if (itens > 0) {

          Serial.println(floor(itens));
          
          this->lcd->print(itens);

        }
        else {

          this->lcd->print(0);

        }

        this->lcd->print(" UN");

        
        if(floor(itens) == this->qtdItem){
          
            this->lcd->setCursor(8, 1);
            
            this->lcd->print(" CERTA!");
            
        }
        else if(floor(itens) > this->qtdItem){
          
            this->lcd->setCursor(8, 1);
            
            this->lcd->print("PASSOU!");
            
        }

        

      }
      
    }

    mostrarConfiguracaoContagem(){

      char caracter = 'A';
      
      this->lcd->clear();

      this->lcd->print("QTD: ");

      this->lcd->print(this->qtdItem);

      this->lcd->print(" UN");

      this->lcd->setCursor(0, 1);

      this->lcd->print("OK - A");

      this->confirmarViaTeclado(caracter);

      this->lcd->setCursor(0, 0);

      this->lcd->print("                ");

      this->lcd->setCursor(0, 0);

      this->lcd->print("Item: ");

      this->lcd->print(this->pesoItem);

      this->lcd->print(" g");

      this->lcd->setCursor(0, 1);

      this->lcd->print("OK - A");

      this->confirmarViaTeclado(caracter);

      this->lcd->setCursor(0, 0);

      this->lcd->print("                ");

      this->lcd->setCursor(0, 0);

      this->lcd->print("Caixa: ");

      this->lcd->print(this->pesoCaixa);

      this->lcd->print(" g");

      this->lcd->setCursor(0, 1);

      this->lcd->print("OK - A");

      this->confirmarViaTeclado(caracter);
    
    }

    configurarQtdItem(){
      
      LiquidCrystal_I2C lcd(0x27, 16, 2);

      lcd.init();

      lcd.clear();

      lcd.setCursor(0, 0);

      lcd.print("Max. 25000 UN");

      lcd.setCursor(0, 1);

      lcd.print("QTD: ");

      int posicaoCursor = 5;

      char tecla = "";

      String qtdItemStr = "";

      lcd.setCursor(0, 0);

      while (tecla != 'A' ||  this->qtdItem > 25000) {

        tecla = this->teclado->waitForKey();

        if (isDigit(tecla) && posicaoCursor < 13) {

          qtdItemStr += tecla;

          this->qtdItem = qtdItemStr.toInt();

          lcd.setCursor(posicaoCursor, 1);

          lcd.print(tecla);

          lcd.printstr(" UN");

          posicaoCursor++;

        }
        else if (tecla == 'D' && posicaoCursor > 5) {

          qtdItemStr.remove(qtdItemStr.length() - 1, 1);

          posicaoCursor--;

          lcd.setCursor(posicaoCursor, 1);

          lcd.print(" UN ");

        }

      }

      Serial.println(this->qtdItem);

      this->delayClearLCD(this->delayTime);
    }

    configurarItem() {

      LiquidCrystal_I2C lcd(0x27, 16, 2);

      lcd.init();

      lcd.clear();

      lcd.setCursor(0, 0);

      lcd.print("Max. 500 g");

      lcd.setCursor(0, 1);

      lcd.print("Peso: ");

      int posicaoCursor = 6;

      char tecla = "";

      String pesoItemStr = "";

      lcd.setCursor(0, 0);

      while (tecla != 'A' ||  this->pesoItem > 500) {

        tecla = this->teclado->waitForKey();

        if (isDigit(tecla) && posicaoCursor < 14) {

          pesoItemStr += tecla;

          this->pesoItem = pesoItemStr.toInt();

          lcd.setCursor(posicaoCursor, 1);

          lcd.print(tecla);

          lcd.printstr(" g");

          posicaoCursor++;

        }
        else if (tecla == 'D' && posicaoCursor > 6) {

          pesoItemStr.remove(pesoItemStr.length() - 1, 1);

          posicaoCursor--;

          lcd.setCursor(posicaoCursor, 1);

          lcd.print(" g ");

        }

      }

      Serial.println(this->pesoItem);

      this->delayClearLCD(this->delayTime);

    }



    configurarCaixa() {

      LiquidCrystal_I2C lcd(0x27, 16, 2);

      lcd.init();

      lcd.clear();

      lcd.setCursor(0, 0);

      lcd.print("Max. 500 g");

      lcd.setCursor(0, 1);

      lcd.print("Peso: ");

      int posicaoCursor = 6;

      char tecla = "";

      String pesoCaixaStr = "";

      lcd.setCursor(0, 0);

      while (tecla != 'A' ||  this->pesoCaixa > 500) {

        tecla = this->teclado->waitForKey();

        if (isDigit(tecla) && posicaoCursor < 14) {

          pesoCaixaStr += tecla;

          this->pesoCaixa = pesoCaixaStr.toInt();

          lcd.setCursor(posicaoCursor, 1);

          lcd.print(tecla);

          lcd.printstr(" g");

          posicaoCursor++;

        }
        else if (tecla == 'D' && posicaoCursor > 6) {

          pesoCaixaStr.remove(pesoCaixaStr.length() - 1, 1);

          posicaoCursor--;

          lcd.setCursor(posicaoCursor, 1);

          lcd.print(" g ");

        }
        
      }

      Serial.println(this->pesoCaixa);

      this->delayClearLCD(this->delayTime);

    }


};

Balanca * balanca = NULL;

void setup() {

  Serial.begin(115200);
  scale.begin(pinDT, pinSCK); // CONFIGURANDO OS PINOS DA BALANÇA

  balanca = new Balanca(lcd, teclado, scale);

}

void loop() {

  balanca->iniciar();

}
