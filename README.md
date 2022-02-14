# SPECIFIKACE
Vytvoření jednoduchého a lightweigh HTTP serveru v jazyce C, který má minimum závislostí. 

# Funkce serveru
## GET
Server přijímá GET requesty na zadaném portu.
Přijímané GET requesty : /hostname, /cpu-name, /load
## Instalace
```
$ make
```
## Použití
```
$ ./hinfosvc PORT
```
### Příklad využití
```
$ make
$ ./hinfosvc 12345 &
$ curl http://localhost:12345/hostname
$ curl http://localhost:12345/cpu-name
$ curl http://localhost:12345/load
```
