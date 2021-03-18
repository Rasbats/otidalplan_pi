#!/bin/sh
#

python ./test_build.py
chmod u+x test_build.py
./test_build.py


./gradlew asciidoctor

