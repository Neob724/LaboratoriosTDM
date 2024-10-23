import serial as s
import matplotlib.pyplot as plt
import time

# Configuración del puerto serial
port = 'COM5'
baudrate = 9600

# Listas para almacenar los datos
adc_motor = []
adc_referencia = []
pwm_pb1 = []
pwm_pb2 = []
timestamps = []

# Tiempo máximo de captura de datos en segundos
tiempo_max = 20
inicio = time.time()

# Conexión al puerto serial
try:
    with s.Serial(port, baudrate, timeout=1) as ser:
        print(f"Conectado al puerto {port} a {baudrate} baudios")
        while time.time() - inicio < tiempo_max:
            if ser.in_waiting > 0:
                # Leer los datos del puerto serial
                data = ser.readline().decode('utf-8', errors='ignore').strip()
                print(data)
                
                # Dividir los valores recibidos separados por coma
                valores = data.split(',')
                
                # Asegurarse de que tenemos 4 valores
                if len(valores) == 4:
                    try:
                        # Almacenar los valores en las listas correspondientes
                        adc_motor.append(float(valores[0]))
                        adc_referencia.append(float(valores[1]))
                        pwm_pb1.append(float(valores[2]))
                        pwm_pb2.append(float(valores[3]))
                        timestamps.append(time.time() - inicio)  # Tiempo relativo
                    except ValueError:
                        print("Error al convertir los valores a flotante.")
        
except s.SerialException as e:
    print(f"Error al abrir el puerto serial: {e}")

# Graficar los datos recibidos
plt.figure(figsize=(10, 6))

# Gráfico de adc_motor
plt.subplot(2, 2, 1)
plt.plot(timestamps, adc_motor, label="ADC Motor", color='b')
plt.title("ADC Motor")
plt.xlabel("Tiempo (s)")
plt.ylabel("Valor")
plt.grid(True)

# Gráfico de adc_referencia
plt.subplot(2, 2, 2)
plt.plot(timestamps, adc_referencia, label="ADC Referencia", color='g')
plt.title("ADC Referencia")
plt.xlabel("Tiempo (s)")
plt.ylabel("Valor")
plt.grid(True)

# Gráfico de pwm_pb1
plt.subplot(2, 2, 3)
plt.plot(timestamps, pwm_pb1, label="PWM PB1", color='r')
plt.title("PWM PB1")
plt.xlabel("Tiempo (s)")
plt.ylabel("Valor")
plt.grid(True)

# Gráfico de pwm_pb2
plt.subplot(2, 2, 4)
plt.plot(timestamps, pwm_pb2, label="PWM PB2", color='m')
plt.title("PWM PB2")
plt.xlabel("Tiempo (s)")
plt.ylabel("Valor")
plt.grid(True)

# Ajustar el espacio entre los gráficos
plt.tight_layout()
plt.show()