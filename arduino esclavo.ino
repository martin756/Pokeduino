#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Wire.h>

LiquidCrystal lcd(A0,A1,A2,A3,3,2);
const byte numFilas = 4;
const byte numColumnas = 4;
byte sadFace[8] = { B00000, B10001, B00000, B00000, B01110, B10001, B00000 }; 
char teclaPresionada = '\0', listo = '\0', accionConfirmada = '\0';
bool seleccion = true, batalla = false, volver = false, volver2 = false, rivalConfirmoAtaque = false, 
pokemonVencido = false, noMandarFlag = false, cambiarPokemonVencido = false, finJuego = false, cambiadoRecientemente = false;
int tipoAtaqueConfirmado, tipoAtaqueRivalConfirmado, turnoJugador = -1, i = 0,j = 0,n;
volatile bool rivalSeleccionLista = false, equipoSeleccionLista = false, 
cambioPokemonRivalListo = false, ataqueConfirmado = false, finTurnoRival = false;
struct tipo{
  int id;
  char nombre[7];
};
tipo tipos[6] = {{1,"Acero"},{2,"Agua"},{3,"Dragon"},{4,"Fuego"},{5,"Hada"},{6,"Planta"}};
struct pokemon{
  int id;
  char nombre[10];
  tipo claseTipo;
  tipo debilidad[2];
  tipo resistencia[3];
  int vida;
  int ataque_neutro;
  int ataque_elemental;
};
pokemon pokemones[6] = {
  {1,"SERPERIOR",tipos[5],{tipos[3]},{tipos[1]},115,10,24},
  {2,"BLASTOISE",tipos[1],{tipos[5]},{tipos[3],tipos[0]},105,12,24},
  {3,"ARCANINE",tipos[3],{tipos[1]},{tipos[5],tipos[0]},80,18,30},
  {4,"GOODRA",tipos[2],{tipos[4],tipos[2]},{tipos[5],tipos[3],tipos[1]},95,14,26},
  {5,"SYLVEON",tipos[4],{tipos[0]},{tipos[2]},110,12,24},
  {6,"MELMETAL",tipos[0],{tipos[3]},{tipos[2],tipos[4],tipos[5]},100,10,28}};
pokemon equipoPokemon[3], equipoPokemonRival[3],
		pokemonEnBatalla, pokemonRivalEnBatalla;

char botonesKeymap[numFilas][numColumnas]= 
{
  {'1', '2', '3', 'A'}, 
  {'4', '5', '6', 'B'}, 
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pinsFilas[numFilas] = {12,11,10,9}; //Filas 0 a 3
byte pinsColumnas[numColumnas]= {8,7,6,5}; //Columnas 0 a 3

Keypad keypadInstance = Keypad(makeKeymap(botonesKeymap), 
  pinsFilas, pinsColumnas, 
  numFilas, numColumnas);
void setup()
{
  Wire.begin(1);
  lcd.begin(16, 2);
  lcd.createChar(0, sadFace);
  Wire.onReceive(atenderOrden);
  Wire.onRequest(atenderPeticion);
}

void loop()
{
  if(seleccion)
  {
    lcd.clear();
    lcd.print("Selecciona a tu");
    lcd.setCursor(0,1);
    lcd.print("equipo: #");
    while(1)
    {
      teclaPresionada = keypadInstance.getKey();
      if(teclaPresionada == '#')
      {
		seleccionEquipo();
        lcd.clear();
        lcd.print("Esperando al");
        lcd.setCursor(0,1);
        lcd.print("rival...");
        while(1)
        {
          if(rivalSeleccionLista)
          {
            pokemonEnBatalla = equipoPokemon[0];
            pokemonRivalEnBatalla = equipoPokemonRival[0];
            rivalSeleccionLista = false;
            break;
          }
        }
        seleccion = false;
        batalla = true;
        break;
      }
    }
  }
  else if(batalla)
  {
    iniciarBatalla();
  }
}
void seleccionEquipo()
{
  bool pokemonSeleccionado = false;
  for(int i = 1; i <= 3; i++)
  {
    lcd.clear();
    lcd.print("Pokemon ");
    lcd.print(i);
    lcd.print(":");
    while(1)
    {
      teclaPresionada = keypadInstance.getKey();
      int n = teclaPresionada - '0';
      if(n >= 1 && n <= 6)
      {
        equipoPokemon[i-1] = pokemones[n-1];
        lcd.setCursor(0,1);
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print(equipoPokemon[i-1].nombre);
        pokemonSeleccionado = true;
      }
      else if(teclaPresionada == '#' && pokemonSeleccionado)
      {
        pokemonSeleccionado = false;
        break;
      }
    }
  }
  accionConfirmada = 'E';
}
void iniciarBatalla()
{
  lcd.clear();
  lcd.print("1.Atk 2.Cambiar");
  lcd.setCursor(0,1);
  lcd.print("3.Ver Info");
  volver = false;
  accionConfirmada = '\0';
  while(!volver)
  {
    teclaPresionada = keypadInstance.getKey();
    int n = teclaPresionada - '0';
    while(n == 1 && !volver)
    {
      atacar();
    }
    while(n == 2 && !volver)
    {
      cambiarPokemon();
    }
    while(n == 3 && !volver)
    {
      verInformacion();
    }
  }
}
void atacar()
{
  bool confirmarAtaque = false;
  lcd.clear();
  lcd.print("1.Neutro Volver*");
  lcd.setCursor(0,1);
  lcd.print("2.Elemental");
  volver2 = false;
  while(!volver2)
  {
    teclaPresionada = keypadInstance.getKey();
    tipoAtaqueConfirmado = teclaPresionada - '0';
    if(tipoAtaqueConfirmado >= 1 && tipoAtaqueConfirmado <= 2)
    {
      lcd.clear();
      if(tipoAtaqueConfirmado == 1)
      {
        lcd.print("1.Neutro");
      }
      else if(tipoAtaqueConfirmado == 2)
      {
        lcd.print("2.Elemental");
      }
      lcd.setCursor(0,1);
      lcd.print("OK# Volver*");
      confirmarAtaque = true;
      while(1)
      {
        teclaPresionada = keypadInstance.getKey();
        if(teclaPresionada == '#' && confirmarAtaque)
        {
          accionConfirmada = 'A';
          confirmarAccion();
          while(1)
          {
            if(rivalConfirmoAtaque == true)
            {
              rivalConfirmoAtaque = false;
              break;
            }
          }
          /*while(turnoJugador == 1)
          {
            if(finTurnoJugador)
            {
              accionConfirmada = 'F';
            }
          }*/
          /////
          //accionConfirmada = '\0';
          if(pokemonEnBatalla.vida > 0 && !pokemonVencido && !cambiadoRecientemente)
          {
            calcularAtaque(tipoAtaqueConfirmado,1);
          }
          cambiadoRecientemente = false;
          /////
          /*if(tipoAtaqueConfirmado == 1)
          {
            delay(3000);
          }*/
          if(pokemonEnBatalla.vida == 0)
          {
            while(pokemonEnBatalla.vida == 0 && !finJuego)
            {
              cambiarPokemonVencido = true;
              cambiarPokemon();
              pokemonVencido = false;
              noMandarFlag = true;
            }
          }
          else if(pokemonRivalEnBatalla.vida == 0)
          {
            accionConfirmada = 'F';
            while(pokemonRivalEnBatalla.vida == 0 && !finJuego)
            {
              while(1)
              {
                if(cambioPokemonRivalListo || finJuego)
                {
                  cambioPokemonRivalListo = false;
                  break;
                }
              }
            }
          }
          if(noMandarFlag == false)
          {
            accionConfirmada = 'F';
          }
          //turnoJugador = !turnoJugador;
          noMandarFlag = false;
          volver2 = true; volver = true;
          pokemonVencido = false;
          recuperarVida();
          break;
        }
        else if(teclaPresionada == '*')
        {
          volver2 = true;
          break;
        }
      }
    }
    else if(teclaPresionada == '*')
    {
      volver = true;
      break;
    }
  }
}
void recuperarVida()
{
  for(int c = 0; c < 3; c++)
  {
    if(pokemonEnBatalla.id != equipoPokemon[c].id && 
       equipoPokemon[c].vida > 0 && 
       pokemones[equipoPokemon[c].id-1].vida > equipoPokemon[c].vida)
    {
      equipoPokemon[c].vida += 10;
      equipoPokemon[c].vida = equipoPokemon[c].vida > pokemones[equipoPokemon[c].id-1].vida ? pokemones[equipoPokemon[c].id-1].vida : equipoPokemon[c].vida;
    }
    if(pokemonRivalEnBatalla.id != equipoPokemonRival[c].id && 
       equipoPokemonRival[c].vida > 0 &&
      pokemones[equipoPokemonRival[c].id-1].vida > equipoPokemonRival[c].vida)
    {
      equipoPokemonRival[c].vida += 10;
      equipoPokemonRival[c].vida = equipoPokemonRival[c].vida > pokemones[equipoPokemonRival[c].id-1].vida ? pokemones[equipoPokemonRival[c].id-1].vida : equipoPokemonRival[c].vida;
    }
  }
}
void calcularAtaque(int tipoAtaque, int turno)
{
  switch(tipoAtaque)
  {
    case 1: //ataque neutro
    if(turno == 0) //turno del maestro
    {
      pokemonEnBatalla.vida -= (pokemonRivalEnBatalla.ataque_neutro);
      pokemonEnBatalla.vida = pokemonEnBatalla.vida < 0 ? 0 : pokemonEnBatalla.vida;
      for(int c = 0; c < 3; c++)///PASO INFO A LOS EQUIPOS
      {
        if(equipoPokemon[c].id == pokemonEnBatalla.id)
        {
          equipoPokemon[c] = pokemonEnBatalla;
          break;
        }
      }
      if(pokemonEnBatalla.vida == 0)
      {
        lcd.clear();
        lcd.print(pokemonEnBatalla.nombre);
        lcd.print(" fue");
        lcd.setCursor(0,1);
        lcd.print("debilitado!!!");
        accionConfirmada = 'V';
        //cambiarPokemon();
      }
    }
    else if(turno == 1) //turno del esclavo
    {
      if(pokemonRivalEnBatalla.vida > 0)
      {
        pokemonRivalEnBatalla.vida -= (pokemonEnBatalla.ataque_neutro);
        pokemonRivalEnBatalla.vida = pokemonRivalEnBatalla.vida < 0 ? 0 : pokemonRivalEnBatalla.vida;
      }
      for(int c = 0; c < 3; c++)///PASO INFO A LOS EQUIPOS
      {
        if(equipoPokemonRival[c].id == pokemonRivalEnBatalla.id)
        {
          equipoPokemonRival[c] = pokemonRivalEnBatalla;
          break;
        }
      }
      if(pokemonRivalEnBatalla.vida == 0)
      {
        lcd.clear();
        lcd.print(pokemonRivalEnBatalla.nombre);
        lcd.print(" fue");
        lcd.setCursor(0,1);
        lcd.print("debilitado!!!");
        //accionConfirmada = 'V';
      }
    }
    break;
    case 2: //ataque elemental
	bool calculaResistencia = true, danioModificado = false;
    if(turno == 0) //turno del maestro
    {
      for(int c = 0; c < 2; c++) //calcula debilidades
      {
        if(pokemonRivalEnBatalla.claseTipo.id == pokemonEnBatalla.debilidad[c].id)
        {
		  pokemonEnBatalla.vida -= (pokemonRivalEnBatalla.ataque_elemental+(pokemonRivalEnBatalla.ataque_elemental*0.5));
		  pokemonEnBatalla.vida = pokemonEnBatalla.vida < 0 ? 0 : pokemonEnBatalla.vida;
		  lcd.clear();
		  lcd.print("...es super");
		  lcd.setCursor(0,1);
		  lcd.print("efectivo!!!");
		  calculaResistencia = false;
          danioModificado = true;
          break;
        }
      }
	  if(calculaResistencia)
	  {
	    for(int c = 0; c < 3; c++) //calcula resistencias
	    {
		  if(pokemonRivalEnBatalla.claseTipo.id == pokemonEnBatalla.resistencia[c].id)
		  {
			pokemonEnBatalla.vida -= (pokemonRivalEnBatalla.ataque_elemental/2);
            pokemonEnBatalla.vida = pokemonEnBatalla.vida < 0 ? 0 : pokemonEnBatalla.vida;
			lcd.clear();
            lcd.print("...es poco");
            lcd.setCursor(0,1);
            lcd.print("efectivo.");
            danioModificado = true;
		    break;
		  }
	    }
	  }
	  if(!danioModificado)
	  {
        pokemonEnBatalla.vida -= (pokemonRivalEnBatalla.ataque_elemental);
        pokemonEnBatalla.vida = pokemonEnBatalla.vida < 0 ? 0 : pokemonEnBatalla.vida;
	  }
      for(int c = 0; c < 3; c++)///PASO INFO A LOS EQUIPOS
      {
        if(equipoPokemon[c].id == pokemonEnBatalla.id)
        {
          equipoPokemon[c] = pokemonEnBatalla;
          break;
        }
      }
      if(pokemonEnBatalla.vida == 0)
      {
        lcd.clear();
        lcd.print(pokemonEnBatalla.nombre);
        lcd.print(" fue");
        lcd.setCursor(0,1);
        lcd.print("debilitado!!!");
        accionConfirmada = 'V';
        //cambiarPokemon();
      }
    }
    else if(turno == 1) //turno del esclavo
    {
      if(pokemonRivalEnBatalla.vida > 0)
      {
        for(int c = 0; c < 2; c++) //calcula debilidades
        {
          if(pokemonEnBatalla.claseTipo.id == pokemonRivalEnBatalla.debilidad[c].id)
          {
		    pokemonRivalEnBatalla.vida -= (pokemonEnBatalla.ataque_elemental+(pokemonEnBatalla.ataque_elemental*0.5));
		    pokemonRivalEnBatalla.vida = pokemonRivalEnBatalla.vida < 0 ? 0 : pokemonRivalEnBatalla.vida;
		    delay(3000);
            lcd.clear();
		    lcd.print("...es super");
		    lcd.setCursor(0,1);
		    lcd.print("efectivo!!!");
		    calculaResistencia = false;
            danioModificado = true;
            break;
          }
        }
	    if(calculaResistencia)
	    {
	      for(int c = 0; c < 3; c++) //calcula resistencias
	      {
		    if(pokemonEnBatalla.claseTipo.id == pokemonRivalEnBatalla.resistencia[c].id)
		    {
			  pokemonRivalEnBatalla.vida -= (pokemonEnBatalla.ataque_elemental/2);
              pokemonRivalEnBatalla.vida = pokemonRivalEnBatalla.vida < 0 ? 0 : pokemonRivalEnBatalla.vida;
			  delay(3000);
              lcd.clear();
              lcd.print("...es poco");
              lcd.setCursor(0,1);
              lcd.print("efectivo.");
              danioModificado = true;
		      break;
		    }
	      }
	    }
	    if(!danioModificado)
	    {
          pokemonRivalEnBatalla.vida -= (pokemonEnBatalla.ataque_elemental);
          pokemonRivalEnBatalla.vida = pokemonRivalEnBatalla.vida < 0 ? 0 : pokemonRivalEnBatalla.vida;
	    }
      }
      for(int c = 0; c < 3; c++)
      {
        if(equipoPokemonRival[c].id == pokemonRivalEnBatalla.id)
        {
          equipoPokemonRival[c] = pokemonRivalEnBatalla;
          break;
        }
      }
      if(pokemonRivalEnBatalla.vida == 0)
      {
        lcd.clear();
        lcd.print(pokemonRivalEnBatalla.nombre);
        lcd.print(" fue");
        lcd.setCursor(0,1);
        lcd.print("debilitado!!!");
      }
    }
    break;
  }
}
void validarFinJuego()
{
  int cantPokemonesRivalVencidos = 0, cantPokemonesVencidos = 0;
  for(int c = 0; c < 3; c++)
  {
    if(equipoPokemon[c].vida == 0)
    {
      cantPokemonesVencidos++;
    }
  }
  for(int c = 0; c < 3; c++)
  {
    if(equipoPokemonRival[c].vida == 0)
    {
      cantPokemonesRivalVencidos++;
    }
  }
  if(cantPokemonesRivalVencidos == 3)
  {
    lcd.clear();
    lcd.print("Felicitaciones!");
    lcd.setCursor(0,1);
    lcd.print("Ganaste!!!");
    seleccion = true;
    batalla = false;
  }
  else if(cantPokemonesVencidos == 3)
  {
    lcd.clear();
    lcd.print("Tu equipo fue ");
    lcd.setCursor(0,1);
    lcd.print("debilitado ");
    lcd.write((byte)0);
    seleccion = true;
    batalla = false;
  }
}
void cambiarPokemon()
{
  lcd.clear();
  int opciones[2];j = 0;
  for(int i = 0; i < 3; i++)
  {
    if(pokemonEnBatalla.id != equipoPokemon[i].id && equipoPokemon[i].vida > 0) //TODO: validar vida > 0
    {
      lcd.print(i+1);lcd.print(".");
      lcd.print(equipoPokemon[i].nombre);
      lcd.print(":");
      lcd.print(equipoPokemon[i].vida);
      lcd.setCursor(0,1);
      opciones[j] = i+1;j++;
    }
  }
  if(j > 0 || cambiarPokemonVencido == false)
  {
    volver2 = false;
    while(!volver2)
    {
      teclaPresionada = keypadInstance.getKey();
      n = teclaPresionada - '0';
      if((n == opciones[0] || n == opciones[1]) && j > 0)
      {
        lcd.clear();
        lcd.print(n);lcd.print(".");
        lcd.print(equipoPokemon[n-1].nombre);
        lcd.print(":");
        lcd.print(equipoPokemon[n-1].vida);
        lcd.setCursor(0,1);
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print("OK# Volver*");
        while(1)
        {
          teclaPresionada = keypadInstance.getKey();
          if(teclaPresionada == '#')
          {
            pokemonEnBatalla = equipoPokemon[n-1];
            accionConfirmada = 'C';
            if(cambiarPokemonVencido == false)
            {
              confirmarAccion();
              recuperarVida();
            }
            cambiarPokemonVencido = false;
            volver = true; volver2 = true;
            break;
          }
          else if(teclaPresionada == '*')
          {
            volver2 = true;
            break;
          }
        }
      }
      else if(teclaPresionada == '*')
      {
        volver = true;
        break;
      }
    }
  }
  else if(j == 0 && cambiarPokemonVencido == true)
  {
    //pokemonEnBatalla = equipoPokemon[n-1];
    accionConfirmada = 'T';
    cambiarPokemonVencido = false;
    volver = true;finJuego = true;
    validarFinJuego();
  }
}
void verInformacion()
{
  lcd.clear();
  lcd.print(pokemonEnBatalla.nombre);
  lcd.print(":");
  lcd.print(pokemonEnBatalla.vida);
  lcd.setCursor(0,1);
  lcd.print(pokemonRivalEnBatalla.nombre);
  lcd.print(":");
  lcd.print(pokemonRivalEnBatalla.vida);
  do{
    teclaPresionada = keypadInstance.getKey();
  }while(teclaPresionada != '*');
  volver = true;
}
void confirmarAccion()
{
  lcd.clear();
  lcd.print("Esperando al");
  lcd.setCursor(0,1);
  lcd.print("rival...");
  while(1)
  {
    if(cambioPokemonRivalListo)
    {
      cambioPokemonRivalListo = false;
      break;
    }
    else if(ataqueConfirmado)
    {
      while(turnoJugador == 0)
      {
        if(finTurnoRival)
        {
          if(pokemonVencido)
          {
            pokemonRivalEnBatalla.vida = 0;
          }
          finTurnoRival = false;
          turnoJugador = 1;
        }
      }
      if(pokemonRivalEnBatalla.vida > 0)
      {
        lcd.clear();
        lcd.print(pokemonRivalEnBatalla.nombre);
        lcd.print(" uso");
        lcd.setCursor(0,1);
        if(tipoAtaqueRivalConfirmado == 1)
        {
          lcd.print("ataque Neutro.");
        }
        else if(tipoAtaqueRivalConfirmado == 2)
        {
          lcd.print("ataque ");
          lcd.print(pokemonRivalEnBatalla.claseTipo.nombre);
        }
        calcularAtaque(tipoAtaqueRivalConfirmado,0);
      }else{
        calcularAtaque(tipoAtaqueRivalConfirmado,1);
      }
      delay(3000);
      /*while(pokemonRivalEnBatalla.vida == 0)
      {
        while(1)
        {
          if(cambioPokemonRivalListo || finJuego)
          {
            cambioPokemonRivalListo = false;
            finJuego = false;
            break;
          }
        }
      }*/
      if(pokemonEnBatalla.vida == 0 && !cambiarPokemonVencido)
      {
        while(pokemonEnBatalla.vida == 0 && !finJuego)
        {
          cambiarPokemonVencido = true;
          cambiarPokemon();
          pokemonVencido = false;
          noMandarFlag = true;
          cambiadoRecientemente = true;
        }
      }
      else if(pokemonRivalEnBatalla.vida == 0)
      {
        accionConfirmada = 'F';
        while(pokemonRivalEnBatalla.vida == 0 && !finJuego)
        {
          while(1)
          {
            if(cambioPokemonRivalListo || finJuego)
            {
              cambioPokemonRivalListo = false;
              break;
            }
          }
        }
      }
      ataqueConfirmado = false;
      //turnoJugador = !turnoJugador;
      break;
    }
  }
}
void atenderOrden(int k)
{
  char flag = (char) Wire.read();
  if (flag == 'S')
  {
    rivalSeleccionLista = true;
  }
  else if (flag == 'P') //recibe pokemones seleccionados del rival
  {
    int idPokemon = Wire.read();
    equipoPokemonRival[i] = pokemones[idPokemon-1];
    i++;
  }
  else if (flag == 'C')
  {
    int indice = Wire.read();
    pokemonRivalEnBatalla = equipoPokemonRival[indice];
    cambioPokemonRivalListo = true;
  }
  else if (flag == 'A')
  {
    tipoAtaqueRivalConfirmado = Wire.read();
    turnoJugador = Wire.read();
    ataqueConfirmado = true;
    finTurnoRival = false;
  }
  else if (flag == 'F')//fin turno del maestro
  {
    finTurnoRival = true;
  }
  else if (flag == 'V')
  {
    finTurnoRival = true;
    pokemonRivalEnBatalla.vida = 0;
    pokemonVencido = true;
  }
  else if (flag == 'T')
  {
    finJuego = true;
  }
}
void atenderPeticion()
{
  if(accionConfirmada == 'E')
  {
    Wire.write('S');
    Wire.write('P');
    for(int k = 1; k <= 3; k++)
    {
      Wire.write(equipoPokemon[k-1].id);
    }
  }
  else if(accionConfirmada == 'C')
  {
    Wire.write(accionConfirmada);
    Wire.write(n-1);//indice
  }
  else if(accionConfirmada == 'A')
  {
    Wire.write(accionConfirmada);
    Wire.write(tipoAtaqueConfirmado);
    rivalConfirmoAtaque = true;
  }
  else if(accionConfirmada == 'F')//fin turno del esclavo
  {
    Wire.write(accionConfirmada);
  }
  else if(accionConfirmada == 'V')
  {
    Wire.write('F');
    Wire.write(accionConfirmada);
  }
  else if(accionConfirmada == 'T')
  {
    Wire.write(accionConfirmada);
  }
  accionConfirmada = '\0';
}