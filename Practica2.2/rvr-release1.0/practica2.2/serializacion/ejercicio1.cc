#include "Serializable.h"

#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

static const size_t MAX_NAME = 20;

class Jugador: public Serializable
{
public:
    Jugador(const char * _n, int16_t _x, int16_t _y):pos_x(_x), pos_y(_y)
    {
        strncpy(name, _n, 20);
    };

    virtual ~Jugador(){};

    void to_bin()
    {
        //Tamaño del objeto (octet-counting) dos int16_t mas el nombre
        _size = sizeof(int16_t) * 2 + sizeof(char) * MAX_NAME;
        //Reserva size de memoria para el objeto
        alloc_data(_size);

        //Buffer generico char *
        char* buffer = _data;

        //Copia name a partir de la direccion de buffer (_data)
        memcpy(buffer, name, MAX_NAME * sizeof(char));

        //Avanza el puntero buffer hasta que no tenga nada de name
        buffer += MAX_NAME * sizeof(char);

        //Copia pos_x a partir de la direccion de buffer
        memcpy(buffer, &pos_x, sizeof(int16_t));

        //Avanza el puntero buffer hasta que no tenga nada de pos_x
        buffer += sizeof(int16_t);

        //Copiamos la variable pos_y a partir de la direccion de buffer
        memcpy(buffer, &pos_y, sizeof(int16_t));

    }

    int from_bin(char * data)
    {
        //Puntero buffer que apunta a la direccion de memoria en la que copiaremos
        //la informacion de nuestra clase player
        char* buffer = data;

        //Copia del nombre en el buffer
        memcpy(name, buffer, MAX_NAME * sizeof(char));

        //Avanza el puntero hasta pasar el nombre
        buffer += MAX_NAME * sizeof(char);

        //Copia de pos_x en el buffer
        memcpy(&pos_x, buffer, sizeof(int16_t));

        //Avanza el puntero hasta pasar pos_x
        buffer += sizeof(int16_t);

        //Copia de pos_y en el buffer
        memcpy(&pos_y, buffer, sizeof(int16_t));

        return 0;
    }


    int16_t pos_x;
    int16_t pos_y;

    char name[MAX_NAME];
};

int main(int argc, char **argv)
{
    Jugador one_r("", 0, 0);
    Jugador one_w("player one", 12, 345);

    // 1. Serializar el objeto one_w
    one_w.to_bin();

    // 2. Escribir la serialización en un fichero

    //abrimos/creamos el archivo
    int fichero = open("./playeroneData.txt", O_CREAT | O_TRUNC | O_RDWR, 0666);
    size_t tam = write(fichero, one_w.data(), one_w.size());

    if (tam != one_w.size()) std::cout << "No se ha guardado todo correctamente \n";
    close(fichero);

    // 3. Leer el fichero
    fichero = open("./playeroneData.txt", O_RDONLY, 0666);
    char buffer[tam];

    if (read(fichero, &buffer, tam) == -1) {
        std::cerr << "Ha ocurrido un error al leer el archivo playeroneData.txt\n";
        return -(EXIT_FAILURE);
    }

    close(fichero);

    // 4. "Deserializar" en one_r
    one_r.from_bin(buffer);

    // 5. Mostrar el contenido de one_r
    std::cout << "Nombre one_r: " << one_r.name << "\n" << "pos_x: " << one_r.pos_x  << "\n" << "pos_y: " << one_r.pos_y << "\n";

    return 0;
}

