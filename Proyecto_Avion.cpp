#include <iostream>
#include <string>
#include <random>
#include <iomanip>
#include <stdexcept>
#include <limits>
#include <fstream>
#include <cctype>

using namespace std;

namespace Configuracion
{

    constexpr double UMBRAL_OVERHEAT = 350.0;
    constexpr double UMBRAL_FIRE = 600.0;

    constexpr double TEMPERATURA_MINIMA = 250.0;
    constexpr double TEMPERATURA_MAXIMA = 700.0;

    constexpr int MAX_HISTORIAL = 20;

    constexpr int SENSORES_POR_LOOP = 2;
}

namespace Interfaz
{
    const string RESET = "\033[0m";

    const string ROJO = "\033[31m";
    const string VERDE = "\033[32m";
    const string AMARILLO = "\033[33m";
    const string AZUL = "\033[34m";
    const string CYAN = "\033[36m";

    const string NEGRITA = "\033[1m";
}

class ExcepcionSistema : public exception
{
private:

    string mensaje;

public:

    explicit ExcepcionSistema(const string& mensaje)
        : mensaje(mensaje)
    {
    }

    const char* what() const noexcept override
    {
        return mensaje.c_str();
    }
};

class Sensor
{
private:

    int id;
    double valorTemperatura;

public:

    explicit Sensor(int id)
        : id(id),
          valorTemperatura(0.0)
    {
    }

    void leer(
        mt19937& generador,
        uniform_real_distribution<double>& distribucion
    )
    {
        valorTemperatura = distribucion(generador);
    }

    int getId() const
    {
        return id;
    }

    double getTemperatura() const
    {
        return valorTemperatura;
    }

    void establecerTemperaturaPrueba(double temperatura)
    {
        valorTemperatura = temperatura;
    }

    bool esOverheat() const
    {
        return valorTemperatura >
               Configuracion::UMBRAL_OVERHEAT;
    }

    bool esFuego() const
    {
        return valorTemperatura >
               Configuracion::UMBRAL_FIRE;
    }

    void mostrar() const
    {
        cout << "Sensor "
             << id
             << " : "
             << fixed
             << setprecision(1)
             << valorTemperatura
             << " °C";
    }
};

class Loop
{
private:

    string nombre;

    Sensor sensores[Configuracion::SENSORES_POR_LOOP];

    bool esFuego;
    bool esOverheat;

public:

    Loop(
        const string& nombre,
        int idSensor1,
        int idSensor2
    )
        : nombre(nombre),
          sensores{Sensor(idSensor1), Sensor(idSensor2)},
          esFuego(false),
          esOverheat(false)
    {
    }

    void leerSensores(
        mt19937& generador,
        uniform_real_distribution<double>& distribucion
    )
    {
        for (int i = 0;
             i < Configuracion::SENSORES_POR_LOOP;
             i++)
        {
            sensores[i].leer(
                generador,
                distribucion
            );
        }
    }

    void establecerTemperaturasPrueba(double temperatura1, double temperatura2)
    {
        sensores[0].establecerTemperaturaPrueba(temperatura1);
        sensores[1].establecerTemperaturaPrueba(temperatura2);
        evaluarLoop();
    }

    void evaluarLoop()
    {
        bool sensor1Overheat =
            sensores[0].esOverheat();

        bool sensor2Overheat =
            sensores[1].esOverheat();

        bool sensor1Fire =
            sensores[0].esFuego();

        bool sensor2Fire =
            sensores[1].esFuego();

        esOverheat =
            sensor1Overheat &&
            sensor2Overheat;

        esFuego =
            sensor1Fire &&
            sensor2Fire;
    }

    string getNombre() const
    {
        return nombre;
    }

    bool getEsFuego() const
    {
        return esFuego;
    }

    bool getEsOverheat() const
    {
        return esOverheat;
    }

    bool operator==(const Loop& otro) const
    {
        return nombre == otro.nombre;
    }

    void mostrar() const
    {
        cout << "\n"
             << Interfaz::CYAN
             << "  "
             << nombre
             << Interfaz::RESET
             << "\n";

        for (int i = 0;
             i < Configuracion::SENSORES_POR_LOOP;
             i++)
        {
            cout << "    ";

            sensores[i].mostrar();

            if (sensores[i].esFuego())
            {
                cout << "  "
                     << Interfaz::ROJO
                     << "[FIRE]"
                     << Interfaz::RESET;
            }
            else if (sensores[i].esOverheat())
            {
                cout << "  "
                     << Interfaz::AMARILLO
                     << "[OVERHEAT]"
                     << Interfaz::RESET;
            }
            else
            {
                cout << "  "
                     << Interfaz::VERDE
                     << "[NORMAL]"
                     << Interfaz::RESET;
            }

            cout << "\n";
        }

        cout << "    Estado Overheat: ";

        if (esOverheat)
        {
            cout << Interfaz::AMARILLO
                 << "ACTIVO"
                 << Interfaz::RESET;
        }
        else
        {
            cout << Interfaz::VERDE
                 << "NORMAL"
                 << Interfaz::RESET;
        }

        cout << "\n";

        cout << "    Estado Fire: ";

        if (esFuego)
        {
            cout << Interfaz::ROJO
                 << "ACTIVO"
                 << Interfaz::RESET;
        }
        else
        {
            cout << Interfaz::VERDE
                 << "NORMAL"
                 << Interfaz::RESET;
        }

        cout << "\n";
    }
};

enum class EstadoAlerta
{
    NORMAL,
    MASTER_CAUTION,
    MASTER_WARNING
};

class ComputadoraMotor
{
private:

    Loop loopA;
    Loop loopB;

    bool valvulaSOV_Cerrada;

    bool botellaA_Detonada;
    bool botellaB_Detonada;

    bool masterWarning;
    bool masterCaution;

    bool fuegoConfirmado;

    bool overheatDetectado;

    bool monitoreoPosteriorBotellaA;
    bool fuegoPersisteTrasBotellaA;

    string pantallaEICAS;

    EstadoAlerta estado;

public:

    ComputadoraMotor(
        int idSensor1,
        int idSensor2,
        int idSensor3,
        int idSensor4
    )
        : loopA(
            "LOOP A",
            idSensor1,
            idSensor2
        ),

          loopB(
            "LOOP B",
            idSensor3,
            idSensor4
        ),

          valvulaSOV_Cerrada(false),

          botellaA_Detonada(false),
          botellaB_Detonada(false),

          masterWarning(false),
          masterCaution(false),

          fuegoConfirmado(false),
          overheatDetectado(false),

          monitoreoPosteriorBotellaA(false),
          fuegoPersisteTrasBotellaA(false),

          pantallaEICAS("SYSTEM NORMAL"),

          estado(EstadoAlerta::NORMAL)
    {
    }

    void generarLecturas(
        mt19937& generador,
        uniform_real_distribution<double>& distribucion
    )
    {
        loopA.leerSensores(
            generador,
            distribucion
        );

        loopB.leerSensores(
            generador,
            distribucion
        );
    }

    void actualizarEstado()
    {
        loopA.evaluarLoop();
        loopB.evaluarLoop();

        masterWarning = false;
        masterCaution = false;

        fuegoConfirmado = false;
        overheatDetectado = false;

        if (
            loopA.getEsFuego() &&
            loopB.getEsFuego()
        )
        {
            estado = EstadoAlerta::MASTER_WARNING;

            masterWarning = true;

            fuegoConfirmado = true;

            pantallaEICAS =
                "FIRE";

            valvulaSOV_Cerrada = true;
        }

        else if (
            loopA.getEsOverheat() ||
            loopB.getEsOverheat()
        )
        {
            estado = EstadoAlerta::MASTER_CAUTION;

            masterCaution = true;

            overheatDetectado = true;

            pantallaEICAS =
                "OVERHEAT";
        }

        else
        {
            estado = EstadoAlerta::NORMAL;

            pantallaEICAS =
                "SYSTEM NORMAL";
        }

        if (
            loopB.getEsFuego() &&
            !loopA.getEsFuego()
        )
        {
            pantallaEICAS =
                "LOOP B POSSIBLE DISCREPANCY";
        }

        if (botellaA_Detonada)
        {
            monitoreoPosteriorBotellaA = true;
            fuegoPersisteTrasBotellaA = fuegoConfirmado;
        }
    }

    void dispararBotella(char id)
    {
        if (!fuegoConfirmado)
        {
            throw ExcepcionSistema(
                "No se puede disparar una botella: "
                "no existe fuego confirmado."
            );
        }

        if (!valvulaSOV_Cerrada)
        {
            throw ExcepcionSistema(
                "No se puede disparar la botella: "
                "la valvula SOV no esta cerrada."
            );
        }

        id = static_cast<char>(
            toupper(static_cast<unsigned char>(id))
        );

        if (id == 'A')
        {
            if (botellaA_Detonada)
            {
                throw ExcepcionSistema(
                    "La botella SQUIB A ya fue utilizada."
                );
            }

            botellaA_Detonada = true;
            monitoreoPosteriorBotellaA = false;
            fuegoPersisteTrasBotellaA = false;

            return;
        }

        if (id == 'B')
        {
            if (!botellaA_Detonada)
            {
                throw ExcepcionSistema(
                    "La botella SQUIB B solo puede utilizarse "
                    "despues de la botella SQUIB A."
                );
            }

            if (!monitoreoPosteriorBotellaA)
            {
                throw ExcepcionSistema(
                    "Debe realizar un nuevo monitoreo despues de "
                    "activar SQUIB A."
                );
            }

            if (!fuegoPersisteTrasBotellaA)
            {
                throw ExcepcionSistema(
                    "SQUIB B no disponible: el fuego ya no persiste."
                );
            }

            if (botellaB_Detonada)
            {
                throw ExcepcionSistema(
                    "La botella SQUIB B ya fue utilizada."
                );
            }

            botellaB_Detonada = true;

            return;
        }

        throw ExcepcionSistema(
            "Botella no valida. Seleccione A o B."
        );
    }

    void establecerTemperaturasPrueba(
        double temperaturaA1,
        double temperaturaA2,
        double temperaturaB1,
        double temperaturaB2
    )
    {
        loopA.establecerTemperaturasPrueba(temperaturaA1, temperaturaA2);
        loopB.establecerTemperaturasPrueba(temperaturaB1, temperaturaB2);
        actualizarEstado();
    }

    bool autoTest() const
    {
        bool sensoresOperativos = false;
        bool loopAOperativo = false;
        bool loopBOperativo = false;
        bool masterCautionOperativo = false;
        bool masterWarningOperativo = false;
        bool logicaANDOperativa = false;
        bool valvulaOperativa = false;
        bool circuitoSquibA = false;
        bool circuitoSquibB = false;

        random_device dispositivoPrueba;
        mt19937 generadorPrueba(dispositivoPrueba());
        uniform_real_distribution<double> distribucionPrueba(
            Configuracion::TEMPERATURA_MINIMA,
            Configuracion::TEMPERATURA_MAXIMA
        );

        Sensor sensorPrueba(999);
        sensorPrueba.leer(generadorPrueba, distribucionPrueba);
        sensoresOperativos =
            sensorPrueba.getTemperatura() >= Configuracion::TEMPERATURA_MINIMA &&
            sensorPrueba.getTemperatura() <= Configuracion::TEMPERATURA_MAXIMA;

        ComputadoraMotor motorPrueba(101, 102, 103, 104);

        loopAOperativo = (motorPrueba.loopA.getNombre() == "LOOP A");
        loopBOperativo = (motorPrueba.loopB.getNombre() == "LOOP B");

        motorPrueba.establecerTemperaturasPrueba(400.0, 410.0, 300.0, 310.0);
        masterCautionOperativo =
            motorPrueba.hayMasterCaution() &&
            !motorPrueba.hayMasterWarning();

        motorPrueba.establecerTemperaturasPrueba(650.0, 660.0, 300.0, 310.0);
        bool fuegoEnUnSoloLoopNoActivaAlarma =
            !motorPrueba.hayMasterWarning();

        motorPrueba.establecerTemperaturasPrueba(650.0, 660.0, 670.0, 680.0);
        masterWarningOperativo = motorPrueba.hayMasterWarning();

        valvulaOperativa =
            motorPrueba.hayMasterWarning() &&
            motorPrueba.estaSOVCerrada();

        try
        {
            motorPrueba.dispararBotella('A');
            circuitoSquibA = motorPrueba.botellaADetonada();

            motorPrueba.establecerTemperaturasPrueba(
                650.0, 660.0, 670.0, 680.0
            );

            motorPrueba.dispararBotella('B');
            circuitoSquibB = motorPrueba.botellaBDetonada();
        }
        catch (const ExcepcionSistema&)
        {
            circuitoSquibA = false;
            circuitoSquibB = false;
        }

        logicaANDOperativa = fuegoEnUnSoloLoopNoActivaAlarma;

        bool pruebaCorrecta =
            sensoresOperativos &&
            loopAOperativo &&
            loopBOperativo &&
            masterCautionOperativo &&
            masterWarningOperativo &&
            logicaANDOperativa &&
            valvulaOperativa &&
            circuitoSquibA &&
            circuitoSquibB;

        cout << "\n";
        cout << Interfaz::CYAN
             << "==============================================\n"
             << "                 AUTO-TEST\n"
             << "==============================================\n"
             << Interfaz::RESET;

        cout << "Sensores............... " << (sensoresOperativos ? "OPERATIVOS" : "FALLA") << "\n";
        cout << "Loop A................. " << (loopAOperativo ? "OPERATIVO" : "FALLA") << "\n";
        cout << "Loop B................. " << (loopBOperativo ? "OPERATIVO" : "FALLA") << "\n";
        cout << "Master Caution......... " << (masterCautionOperativo ? "OPERATIVO" : "FALLA") << "\n";
        cout << "Master Warning......... " << (masterWarningOperativo ? "OPERATIVO" : "FALLA") << "\n";
        cout << "Logica AND............. " << (logicaANDOperativa ? "OPERATIVA" : "FALLA") << "\n";
        cout << "Valvula SOV............ " << (valvulaOperativa ? "OPERATIVA" : "FALLA") << "\n";
        cout << "Circuito SQUIB A....... " << (circuitoSquibA ? "OPERATIVO" : "FALLA") << "\n";
        cout << "Circuito SQUIB B....... " << (circuitoSquibB ? "OPERATIVO" : "FALLA") << "\n";
        cout << "Estado real SQUIB A.... " << (botellaA_Detonada ? "DESCARGADA" : "ARMADA") << "\n";
        cout << "Estado real SQUIB B.... " << (botellaB_Detonada ? "DESCARGADA" : "ARMADA") << "\n";

        cout << "\nResultado del AUTO-TEST: ";
        if (pruebaCorrecta)
        {
            cout << Interfaz::VERDE << "PASS" << Interfaz::RESET;
        }
        else
        {
            cout << Interfaz::ROJO << "FAIL" << Interfaz::RESET;
        }
        cout << "\n";

        return pruebaCorrecta;
    }

    void resetSistema()
    {
        if (
            loopA.getEsFuego() ||
            loopB.getEsFuego() ||
            loopA.getEsOverheat() ||
            loopB.getEsOverheat()
        )
        {
            throw ExcepcionSistema(
                "RESET no permitido: aun existe "
                "una condicion anormal."
            );
        }

        masterWarning = false;
        masterCaution = false;

        fuegoConfirmado = false;
        overheatDetectado = false;

        monitoreoPosteriorBotellaA = false;
        fuegoPersisteTrasBotellaA = false;

        valvulaSOV_Cerrada = false;

        botellaA_Detonada = false;
        botellaB_Detonada = false;

        pantallaEICAS =
            "SYSTEM NORMAL";

        estado = EstadoAlerta::NORMAL;
    }

    bool hayMasterWarning() const
    {
        return masterWarning;
    }

    bool hayMasterCaution() const
    {
        return masterCaution;
    }

    bool hayFuego() const
    {
        return fuegoConfirmado;
    }

    bool hayOverheat() const
    {
        return overheatDetectado;
    }

    bool estaSOVCerrada() const
    {
        return valvulaSOV_Cerrada;
    }

    bool botellaADetonada() const
    {
        return botellaA_Detonada;
    }

    bool botellaBDetonada() const
    {
        return botellaB_Detonada;
    }

    string getMensajeEICAS() const
    {
        return pantallaEICAS;
    }

    Loop& getLoopA()
    {
        return loopA;
    }

    Loop& getLoopB()
    {
        return loopB;
    }

    void mostrarEICAS(int numeroMotor) const
    {
        cout << "\n";
        cout << Interfaz::CYAN
             << "==================================================\n"
             << "                 EICAS - MOTOR "
             << numeroMotor
             << "\n"
             << "==================================================\n"
             << Interfaz::RESET;

        cout << "\n";

        loopA.mostrar();

        loopB.mostrar();

        cout << "\n";
        cout << "INDICACION EICAS: ";

        if (masterWarning)
        {
            cout << Interfaz::ROJO
                 << "MASTER WARNING"
                 << Interfaz::RESET;
        }
        else if (masterCaution)
        {
            cout << Interfaz::AMARILLO
                 << "MASTER CAUTION"
                 << Interfaz::RESET;
        }
        else
        {
            cout << Interfaz::VERDE
                 << "NORMAL"
                 << Interfaz::RESET;
        }

        cout << "\n";

        cout << "Mensaje: ";

        if (masterWarning)
        {
            cout << "ENG " << numeroMotor << " FIRE";
        }
        else if (masterCaution)
        {
            cout << "ENG " << numeroMotor << " OVERHEAT";
        }
        else
        {
            cout << pantallaEICAS;
        }

        cout << "\n";

        cout << "Fuel SOV: ";

        if (valvulaSOV_Cerrada)
        {
            cout << Interfaz::ROJO
                 << "CLOSED"
                 << Interfaz::RESET;
        }
        else
        {
            cout << Interfaz::VERDE
                 << "OPEN"
                 << Interfaz::RESET;
        }

        cout << "\n";

        cout << "SQUIB A: ";

        if (botellaA_Detonada)
        {
            cout << "DESCARGADA";
        }
        else
        {
            cout << "ARMADA";
        }

        cout << "\n";

        cout << "SQUIB B: ";

        if (botellaB_Detonada)
        {
            cout << "DESCARGADA";
        }
        else
        {
            cout << "ARMADA";
        }

        cout << "\n";
    }
};

class SistemaAvion
{
private:

    ComputadoraMotor motor1;
    ComputadoraMotor motor2;

    string pilaHistorial[
        Configuracion::MAX_HISTORIAL
    ];

    int tope;

    random_device dispositivoAleatorio;
    mt19937 generador;

    uniform_real_distribution<double>
        distribucionTemperatura;

public:

    SistemaAvion()
        : motor1(1, 2, 3, 4),

          motor2(5, 6, 7, 8),

          tope(-1),

          generador(dispositivoAleatorio()),

          distribucionTemperatura(
              Configuracion::TEMPERATURA_MINIMA,
              Configuracion::TEMPERATURA_MAXIMA
          )
    {
    }

    void apilarEvento(const string& evento)
    {
        if (
            tope >=
            Configuracion::MAX_HISTORIAL - 1
        )
        {
            throw ExcepcionSistema(
                "Desbordamiento de pila: "
                "el historial alcanzo su limite."
            );
        }

        ofstream archivo("historial_eventos.txt", ios::app);

        if (!archivo.is_open())
        {
            throw ExcepcionSistema(
                "No se pudo abrir el archivo de historial."
            );
        }

        archivo << evento << endl;
        archivo.close();

        tope++;
        pilaHistorial[tope] = evento;
    }

    void mostrarHistorialPila() const
    {
        cout << "\n";
        cout << Interfaz::CYAN
             << "==================================================\n"
             << "              HISTORIAL DE EVENTOS\n"
             << "==================================================\n"
             << Interfaz::RESET;

        if (tope == -1)
        {
            cout << "\nNo existen eventos registrados.\n";
            return;
        }

        for (int i = tope;
             i >= 0;
             i--)
        {
            cout << "\n"
                 << (tope - i + 1)
                 << ". "
                 << pilaHistorial[i];
        }

        cout << "\n";
    }

    void monitorear()
    {
        cout << "\n";
        cout << Interfaz::AZUL
             << "==================================================\n"
             << "             CICLO DE MONITOREO\n"
             << "==================================================\n"
             << Interfaz::RESET;

        motor1.generarLecturas(
            generador,
            distribucionTemperatura
        );

        motor1.actualizarEstado();

        motor1.mostrarEICAS(1);

        registrarEstadoMotor(
            motor1,
            1
        );

        motor2.generarLecturas(
            generador,
            distribucionTemperatura
        );

        motor2.actualizarEstado();

        motor2.mostrarEICAS(2);

        registrarEstadoMotor(
            motor2,
            2
        );
    }

    void registrarEstadoMotor(
        const ComputadoraMotor& motor,
        int numeroMotor
    )
    {
        string evento;

        if (motor.hayMasterWarning())
        {
            evento =
                "Motor " +
                to_string(numeroMotor) +
                ": MASTER WARNING - FIRE confirmado.";

            apilarEvento(evento);

            evento =
                "Motor " +
                to_string(numeroMotor) +
                ": Fuel SOV cerrada automaticamente.";

            apilarEvento(evento);
        }
        else if (motor.hayMasterCaution())
        {
            evento =
                "Motor " +
                to_string(numeroMotor) +
                ": MASTER CAUTION - OVERHEAT.";

            apilarEvento(evento);
        }
        else
        {
            evento =
                "Motor " +
                to_string(numeroMotor) +
                ": estado NORMAL.";

            apilarEvento(evento);
        }
    }

    void mostrarMotor()
    {
        int numero;

        cout << "\nSeleccione motor:\n";
        cout << "1. Motor 1\n";
        cout << "2. Motor 2\n";
        cout << "Seleccione: ";

        cin >> numero;

        if (cin.fail())
        {
            cin.clear();

            cin.ignore(
                numeric_limits<streamsize>::max(),
                '\n'
            );

            throw ExcepcionSistema(
                "La seleccion debe ser numerica."
            );
        }

        if (numero == 1)
        {
            motor1.mostrarEICAS(1);
        }
        else if (numero == 2)
        {
            motor2.mostrarEICAS(2);
        }
        else
        {
            throw ExcepcionSistema(
                "El motor seleccionado no existe."
            );
        }
    }

    void dispararBotella(char botella)
    {
        int motor;

        cout << "\nMotor: ";
        cin >> motor;

        if (cin.fail())
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            throw ExcepcionSistema(
                "El numero de motor debe ser numerico."
            );
        }

        if (motor != 1 && motor != 2)
        {
            throw ExcepcionSistema(
                "El motor seleccionado no existe."
            );
        }

        if (motor == 1)
        {
            motor1.dispararBotella(botella);
        }
        else
        {
            motor2.dispararBotella(botella);
        }

        apilarEvento(
            "Motor " + to_string(motor) + ": SQUIB " +
            string(1, toupper(static_cast<unsigned char>(botella))) +
            " activado."
        );

        cout << "\n"
             << Interfaz::VERDE
             << "BOTELLA ACTIVADA CORRECTAMENTE."
             << Interfaz::RESET
             << "\n";
    }

    void dispararBotella()
    {
        char botella;

        cout << "\nBotella [A/B]: ";
        cin >> botella;

        if (cin.fail())
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            throw ExcepcionSistema(
                "La seleccion de botella no es valida."
            );
        }

        dispararBotella(botella);
    }

    void ejecutarAutoTest()
    {
        bool resultadoMotor1 = motor1.autoTest();

        cout << "\n";

        bool resultadoMotor2 = motor2.autoTest();

        if (resultadoMotor1 && resultadoMotor2)
        {
            apilarEvento(
                "AUTO-TEST ejecutado: ambos motores aprobaron."
            );
        }
        else
        {
            apilarEvento(
                "AUTO-TEST ejecutado: se detecto una falla."
            );
        }
    }

    void resetSistema()
    {

        motor1.resetSistema();

        motor2.resetSistema();

        apilarEvento(
            "RESET manual realizado. "
            "Sistema restablecido."
        );

        cout << "\n"
             << Interfaz::VERDE
             << "SISTEMA RESTABLECIDO CORRECTAMENTE."
             << Interfaz::RESET
             << "\n";
    }

    void menuPrincipal()
    {
        int opcion = -1;

        do
        {
            cout << "\n";

            cout << Interfaz::AZUL
                 << "==================================================\n"
                 << "       SISTEMA DE DETECCION Y EXTINCION\n"
                 << "                    DE MOTOR\n"
                 << "==================================================\n"
                 << Interfaz::RESET;

            cout << "\n";
            cout << "1. Iniciar / monitorear simulacion\n";
            cout << "2. Mostrar estado actual del motor\n";
            cout << "3. Disparar Botella A (SQUIB A)\n";
            cout << "4. Disparar Botella B (SQUIB B)\n";
            cout << "5. Ejecutar Auto-Test\n";
            cout << "6. Realizar RESET manual\n";
            cout << "7. Ver historial de eventos\n";
            cout << "8. Salir\n";

            cout << "\nSeleccione una opcion: ";

            try
            {
                cin >> opcion;

                if (cin.fail())
                {
                    cin.clear();
                    cin.ignore(
                        numeric_limits<streamsize>::max(),
                        '\n'
                    );

                    throw ExcepcionSistema(
                        "Opcion no valida. Debe introducir un numero entero."
                    );
                }

                switch (opcion)
                {
                    case 1:
                        monitorear();
                        break;

                    case 2:
                        mostrarMotor();
                        break;

                    case 3:
                        dispararBotella('A');
                        break;

                    case 4:
                        dispararBotella('B');
                        break;

                    case 5:
                        ejecutarAutoTest();
                        break;

                    case 6:
                        resetSistema();
                        break;

                    case 7:
                        mostrarHistorialPila();
                        break;

                    case 8:
                        cout << "\n"
                             << Interfaz::VERDE
                             << "Sistema finalizado correctamente."
                             << Interfaz::RESET
                             << "\n";
                        break;

                    default:
                        throw ExcepcionSistema(
                            "Opcion no valida."
                        );
                }
            }
            catch (const ExcepcionSistema& e)
            {
                cout << "\n"
                     << Interfaz::ROJO
                     << "ERROR: "
                     << e.what()
                     << Interfaz::RESET
                     << "\n";
            }

        } while (opcion != 8);
    }

};

int main()
{
    try
    {
        SistemaAvion sistema;

        sistema.menuPrincipal();
    }
    catch (const ExcepcionSistema& e)
    {
        cout << "\n";
        cout << Interfaz::ROJO
             << "ERROR CRITICO DEL SISTEMA: "
             << e.what()
             << Interfaz::RESET
             << "\n";
    }
    catch (const exception& e)
    {
        cout << "\n";
        cout << Interfaz::ROJO
             << "ERROR INESPERADO: "
             << e.what()
             << Interfaz::RESET
             << "\n";
    }

    return 0;
}