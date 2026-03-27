# Instrukcja kompilacji (Build Instructions)

Poniżej znajdują się polecenia niezbędne do skompilowania plików źródłowych projektu. Projekt wymaga linkowania biblioteki czasu rzeczywistego (`librt`) oraz biblioteki matematycznej (`libm`).

### Wymagania
* Kompilator GCC

### Kompilacja vSensor (Producent)

Aby skompilować program producenta (`vSensor`), użyj poniższego polecenia:

```bash
gcc -o vSensor vSensor.c -lrt -lm
```

a następnie

```bash
./vSensor <f1> <fs> <buffer_size>
```

### Kompilacja monitorów (Konsumenci)

Aby skompilować programy konsumentów (monitor_avg, monitor_f), użyj poniższych poleceń:

```bash
gcc -o monitor_avg monitor_avg.c -lrt -lm
```

oraz

```bash
gcc -o monitor_f monitor_f.c -lrt -lm
```

a następnie w nowych terminalach

```bash 
./monitor_avg <nf> <np>
```

lub

```bash
./monitor_f <nf> <np>
```
