#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"
using namespace std;

class ComportamientoJugador : public Comportamiento{

  public:
    ComportamientoJugador(unsigned int size) : Comportamiento(size){
      restart();
    }

    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
    ~ComportamientoJugador(){}

    Action think(Sensores sensores);

    //Reiniciar player:
    void restart();

    //Juntar memoria con matriz resultado:
    void MergeMap();

    //Añadir el cono de visión al mapa:
    void AniadeAlMap(Sensores sensores);

    //Seleccionar una ruta:
    void FijaRuta(int &num_avances, Action & Giro);

    int interact(Action accion, int valor);

    ComportamientoJugador * clone(){return new ComportamientoJugador(*this);}

  private:
    int fil, col, brujula;
    int vfil, vcol;
    int girar_en;
    Action ultimaAccion;
    int objetoDado;
    bool girar_derecha, bien_situado;
    char memoriaTemp[200][200];
    char objetoNecesario;

    //Mochila auxiliar para conocer los objetos recogidos.
    bool objetos[4];

    //Variables necesarias cuando se fija una Ruta:
    int pos_objetivo;
    bool objetivo_fijado;
    Action Giro;
    int num_avances;
};


#endif
