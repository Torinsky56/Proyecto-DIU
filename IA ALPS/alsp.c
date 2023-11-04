#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct NuevoAvionInfo {
    int tiempo_temprano;
    int tiempo_ideal;
    int tiempo_tardio;
    double penalizacion_aterrizar_antes;
    double penalizacion_aterrizar_despues;
    int* tiempos_separacion;
};

struct infoInstancia {
    int cantidad_aviones;
    struct NuevoAvionInfo* aviones;
};

struct infoInstancia* readFile(char* nombre_archivo) {
    FILE* archivo = fopen(nombre_archivo, "r");

    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return NULL;
    }

    char linea[256];
    if (fgets(linea, sizeof(linea), archivo) == NULL) {
        perror("Error al leer la cantidad de aviones");
        fclose(archivo);
        return NULL;
    }

    int cantidad_aviones = atoi(linea); // Leer la cantidad de aviones

    struct infoInstancia* instancia = (struct infoInstancia*)malloc(sizeof(struct infoInstancia));

    instancia->aviones = (struct NuevoAvionInfo*)malloc(cantidad_aviones * sizeof(struct NuevoAvionInfo));

    instancia->cantidad_aviones = cantidad_aviones;

    for (int i = 0; i < cantidad_aviones; i++) {
        if (fgets(linea, sizeof(linea), archivo) == NULL) {
            perror("Error al leer la primera línea de información del avión");
            fclose(archivo);
            free(instancia->aviones);
            return NULL;
        }

        // Parsing de la primera línea en la nueva estructura
        sscanf(linea, "%d %d %d %lf %lf", 
               &instancia->aviones[i].tiempo_temprano, &instancia->aviones[i].tiempo_ideal, 
               &instancia->aviones[i].tiempo_tardio, &instancia->aviones[i].penalizacion_aterrizar_antes,
               &instancia->aviones[i].penalizacion_aterrizar_despues);

        if (fgets(linea, sizeof(linea), archivo) == NULL) {
            perror("Error al leer la segunda línea de información del avión");
            fclose(archivo);
            free(instancia->aviones);
            return NULL;
        }

        // Leer y almacenar los tiempos de separación
        instancia->aviones[i].tiempos_separacion = (int*)malloc(cantidad_aviones * sizeof(int));
        char* token = strtok(linea, " \t");
        int j = 0;
        while (token != NULL && j < cantidad_aviones) {
            instancia->aviones[i].tiempos_separacion[j] = atoi(token);
            token = strtok(NULL, " \t");
            j++;
        }
    }

    fclose(archivo);

    return instancia;
}

void freeInstancia(struct infoInstancia* instancia) {
    for (int i = 0; i < instancia->cantidad_aviones; i++) {
        free(instancia->aviones[i].tiempos_separacion);
    }
    free(instancia->aviones);
    free(instancia);
}

struct AvionAterrizaje {
    int indice; // Índice del avión en la lista original
    int tiempo_aterrizaje;
};

// Comparador para ordenar aviones por tiempo de aterrizaje
int compararAviones(const void* a, const void* b) {
    return ((struct AvionAterrizaje*)a)->tiempo_aterrizaje - ((struct AvionAterrizaje*)b)->tiempo_aterrizaje;
}

// Función Greedy para generar una solución inicial
void alspGreedy(struct infoInstancia* instancia) {

    int cantidad_aviones = instancia->cantidad_aviones;
    struct NuevoAvionInfo* aviones = instancia->aviones;

    // Crear una lista de aviones con sus índices y tiempos ideales de aterrizaje
    struct AvionAterrizaje* lista_aviones = (struct AvionAterrizaje*)malloc(cantidad_aviones * sizeof(struct AvionAterrizaje));
    
    for (int i = 0; i < cantidad_aviones; i++) {
        lista_aviones[i].indice = i;
        lista_aviones[i].tiempo_aterrizaje = aviones[i].tiempo_ideal;
    }

    // Ordenar los aviones por tiempo ideal de aterrizaje
    qsort(lista_aviones, cantidad_aviones, sizeof(struct AvionAterrizaje), compararAviones);

    // Inicializar los tiempos de aterrizaje de los aviones
    int* tiempos_aterrizaje = (int*)malloc(cantidad_aviones * sizeof(int));
    double penalizacion_total = 0.0; // Inicializar la penalización total

    // Asignar tiempos de aterrizaje a los aviones
    for (int i = 0; i < cantidad_aviones; i++) {
        int avion_indice = lista_aviones[i].indice;
        int tiempo_ideal = aviones[avion_indice].tiempo_ideal;


        // Encontrar un tiempo de aterrizaje válido que cumpla con los tiempos de separación
        int tiempo_a_asignar = tiempo_ideal;
        for (int j = 0; j < i; j++) {
            int otro_avion_indice = lista_aviones[j].indice;
            int tiempo_separacion = aviones[avion_indice].tiempos_separacion[otro_avion_indice];
            int otro_tiempo_aterrizaje = tiempos_aterrizaje[otro_avion_indice];
            if (tiempo_a_asignar <= otro_tiempo_aterrizaje + tiempo_separacion) {
                tiempo_a_asignar = otro_tiempo_aterrizaje + tiempo_separacion;
            }
        }


        // Asignar el tiempo de aterrizaje al avión
        tiempos_aterrizaje[avion_indice] = tiempo_a_asignar;
        // Calcular la penalización
        int tiempo_necesario = aviones[avion_indice].tiempo_ideal;
        int tiempo_asignado = tiempo_a_asignar;
        double penalizacion = 0.0;

        if (tiempo_asignado < tiempo_necesario) {
            penalizacion = aviones[avion_indice].penalizacion_aterrizar_antes * (tiempo_necesario - tiempo_asignado);
        } else if (tiempo_asignado > tiempo_necesario) {
            penalizacion = aviones[avion_indice].penalizacion_aterrizar_despues * (tiempo_asignado - tiempo_necesario);
        }

        printf("Avión %d aterriza en tiempo %d, Penalización: %lf\n", avion_indice, tiempo_asignado, penalizacion);  


        // Añadir la penalización del avión a la penalización total
        penalizacion_total += penalizacion;
    }
    // Imprimir la penalización total
    printf("Suma total de penalizaciones: %lf\n", penalizacion_total);


    // Liberar la memoria
    free(tiempos_aterrizaje);
    free(lista_aviones);

}


    //GREEDY
    /*
    rep -> lista con los datos de los aviones [n_avion, tiempo_aterriza, penalizacion_temprana, penalizacion_tardia]
    punto partida -> avion con menor tiempo ideal -> por lo tanto aterriza en su tiempo ideal.
    
    f eval -> minimizar las penalizaciones
    f miope -> Sumar los tiempos de separacion entre aviones con el avion que acaba de aterrizar y buscar un siguiente
    avion que pueda aterrizar en ese tiempo proximo minimizando penalizacion



    */
    //Definir avion 3 como avion inicial



int main() {
    struct infoInstancia* aviones = readFile("airland13.txt");

    if (aviones) {
        alspGreedy(aviones);
        freeInstancia(aviones);
    } else {
        printf("Error al leer la instancia de aviones.\n");
    }

    return 0;
}
