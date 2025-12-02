set -euxo pipefail

gcc -pthread src/main.c -o program
./program