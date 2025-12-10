set -euxo pipefail

gcc -pthread -o program src/controller.c src/serialport.c 
./program